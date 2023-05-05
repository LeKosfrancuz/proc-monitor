// #define DEBUG
#include "constLib.h"		// Definicije funkcija i konstanti

int RANDOM = 0;				// Paljenje i gašenje randomizacije virualnih poslova
int VRIJEME_PISANJA = 0;		// Vrijeme pisanja u kriticnom odsjecku
int BROJ_PISANJA_PROCESA0 = 2;
time_t POCETAK_IZVODJENJA = 0;		// Vrijeme pocetka izvodjenja programa
int VRIJEME_IZMEDJU_PISANJA_MSEC[BROJ_PROCESA] = {1300, 2300, 1450, 1000, 1300, 2300, 1000, 1300};

enum KomunikacijskeKomande {
	EXIT = -1, WAIT, READY, GO, DONE
};

#include "CLIProcesor.c"	// Procesiranje argumenata komandne linije
#include "processHandler.c"	// Kreacija i terminacija procesa

int main(int argc, Cstr* argv) {
	time(&POCETAK_IZVODJENJA);

	int procPipe[BROJ_PROCESA*2][2];	// 2 stanja (0 za citanje i 1 za pisanje)
	
	Cstr logPath = malloc(10);			// "out" za stdout i "err" za stderr 
	Cstr filePath = malloc(10);

	strcpy(logPath, "log.txt");
	strcpy(filePath, "file.txt");

	ApplyCommandLineArgs(argc, argv, logPath, filePath);

// ***** POSTAVLJANJE KOMUNIKACIJE S PROCESIMA *****
	for (int i = 0; i < BROJ_PROCESA*2; i++)
		if (pipe(procPipe[i]) == -1) Error("Komunikacija preko \"Pipe-a\" nije uspjela");

	int flags = fcntl(procPipe[0][0], F_GETFL, 0) | O_NONBLOCK;  // spremanje ostalih zastavica pipe-a i dodavanje NONBLOCK
	for (int i = 0; i < BROJ_PROCESA*2; i++) 
		fcntl(procPipe[i][0], F_SETFL, flags);	// gašenje blokiranja programa ako je read() prazan
	
	int childPid = fork();	

	if (childPid == 0)
		ProcesTreeCreator(filePath, procPipe);

// ***** UPRAVLJANJE PROCESIMA: SEMAFOR *****	

	Pipe procPipes[BROJ_PROCESA];

	for (int i = 0; i < BROJ_PROCESA*2; i++) {
		/* Proces nesmije imat pristup citati sa pipe-a na koji pise, 
		 ovdje taj kraj zatvaramo pa to nije moguće, a drugi kraj dodjelit pipeHandler-u	*/
		if (i < BROJ_PROCESA) {
			close(procPipe[i][0]);

			procPipes[i].write = dup(procPipe[i][1]);
			close(procPipe[i][1]);
		}
		/* Sa gornjih (id*2) ce samo dobivati info pa ne smije pisati */
		else {
			close(procPipe[i][1]);

			procPipes[i - BROJ_PROCESA].read = dup(procPipe[i][0]);
			close(procPipe[i][0]);
		}
	}
	 
	FILE *log;
	if (!strcmp(logPath, "out")) log = stdout;
	else if (!strcmp(logPath, "err")) log = stderr;
	else log = fopen(logPath, "w");

	int komanda = GO;
	while (komanda != EXIT) {
		for (int i = 0; i < BROJ_PROCESA && komanda != EXIT; i++) {
			int dobivenaPoruka;
			if (read(procPipes[i].read, &dobivenaPoruka, 4) == -1) continue;
			else if (dobivenaPoruka == EXIT) { komanda = EXIT; fprintf(log, "Id %d poslao zahtjev o gašenju programa\n", i); continue; }
			else if (dobivenaPoruka != READY) continue;
			
			komanda = GO;
			write(procPipes[i].write, &komanda, 4);

			do {
				if (read(procPipes[i].read, &dobivenaPoruka, 4) == -1 ) { dobivenaPoruka = WAIT; continue; }
				fprintf(log, "Proces #%d odgovorio sa %s\n", i, IntToKomKom(dobivenaPoruka));
			} while (dobivenaPoruka != DONE);
			fprintf(log, "Proces #%d završio pisanje u file!\n", i);
		}
	}
	
	fclose(log);

	if (KillRunning(logPath, procPipes)) Error("Nije uspjela terminacija svih child procesa!");

	sleep(1);

	log = fopen(logPath, "a");
	fprintf(log, "Main završio!\n");
	printf("Main završio!\n");
	fclose(log);

	exit(0);
}


void WriteToFile(int id, Cstr filePath, Pipe pipe) {
	int poruka = WAIT;
	srand(id*time(NULL));

	float vrijemeIzmedjuPisanjaSec = VRIJEME_IZMEDJU_PISANJA_MSEC[id] / 1000.0;
	int count = 0;

	while(poruka != EXIT) {
		float pravoVrijemeIzmedjuPisanjaSec = vrijemeIzmedjuPisanjaSec + (float)rand()/(float)RAND_MAX*1.96*RANDOM;
										 // Daje random broj od 0 - 1.96       ^Omogućuje randomizaciju
		sleep(pravoVrijemeIzmedjuPisanjaSec);

		poruka = READY;
		write(pipe.write, &poruka, 4);
		
		struct timespec pocetakCekanja, krajCekanja;
		clock_gettime(CLOCK_MONOTONIC_RAW, &pocetakCekanja);

		// Cekanje odobrenja za ulazak u kriticni odsjecak
		int readFail = 0;
		do {
			readFail = read(pipe.read, &poruka, 4); // Citanje trenutne komande
		} while (readFail == -1 || (poruka == WAIT && poruka != GO && poruka != EXIT));
		if (poruka == EXIT) continue;
		
		clock_gettime(CLOCK_MONOTONIC_RAW, &krajCekanja);

		// ***** KRITICNI ODSJECAK *****

		FILE *fp;
		if (!strcmp(filePath, "out")) fp = stdout;
		else if (!strcmp(filePath, "err")) fp = stderr;
		else fp = fopen(filePath, "a");
		count++;

		if (VRIJEME_PISANJA) sleep(VRIJEME_PISANJA / 1000.0); // Emulacija programa koji dugo piše unutar kritičnog odsječka

		long int vrijemeCekanjaMicroSec = (krajCekanja.tv_sec - pocetakCekanja.tv_sec) * 1000000 + (krajCekanja.tv_nsec - pocetakCekanja.tv_nsec) / 1000;
		float vrijemeCekanjaMs = (vrijemeCekanjaMicroSec) / 1000.0;

		fprintf(fp, "["CYN_CC"%2ld"LWHITE_CC"] Zapis "GRN_CC"#%02d"LWHITE_CC" procesa "RED_CC"#%02d"LWHITE_CC" (%d) i cekao je "MAG_CC"%9.4fms"LWHITE_CC, 
			time(NULL)-POCETAK_IZVODJENJA, count, id, getpid(), vrijemeCekanjaMs);
		fprintf(fp, " | Vrijeme između pisanja:  "MAG_CC"%1.2fs"LWHITE_CC, vrijemeIzmedjuPisanjaSec);
		if (RANDOM) fprintf(fp, " ("YELW_CC"+%fs"LWHITE_CC")", pravoVrijemeIzmedjuPisanjaSec - vrijemeIzmedjuPisanjaSec); 
		fprintf(fp, "\n");

		fclose(fp);

		poruka = DONE;
		write(pipe.write, &poruka, 4);
		poruka = WAIT;

		// ***** KRAJ KRITICNOG ODSJECKA *****

		if (id == 0 && count >= BROJ_PISANJA_PROCESA0) { poruka = EXIT; write(pipe.write, &poruka, 4); }
	}

	poruka = EXIT; //Potvrda o završetku
	write(pipe.write, &poruka, 4);

	printf("#%d je završio!\n", id);
	exit(0);
}


Cstr IntToKomKom(int input) {
	switch (input) {
		case EXIT: return RED_CC"EXIT"LWHITE_CC;
		case WAIT: return WHITE_CC"WAIT"LWHITE_CC;
		case READY: return CYN_CC"READY"LWHITE_CC;
		case GO: return GRN_CC"GO"LWHITE_CC;
		case DONE: return GRN_CC"DONE"LWHITE_CC;
		default: { Cstr errout = malloc(50);
			   sprintf(errout, "Nepoznat Komunikacijski Kod %d", input);
			   Error(errout); }
	}

	Error("Unreachable");
	return "";
}
