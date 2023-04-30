#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

typedef char* Cstr;

int KillRunning();
void WriteToFile(int id);
void ProcesTreeCreator();
void proc1();
void proc2();
void proc6();
Cstr IntToKomKom(int input);
Cstr StrLow(Cstr input);
void Error(Cstr message) {
	printf("\e[1;31mERROR: \e[1;37m%s\e[0;37m\n", message);
	exit(1);
}

#define BROJ_PROCESA 8

int RANDOM = 0;				// Paljenje i gašenje randomizacije virualnih poslova
Cstr LOG_PATH = 0;			// "out" za stdout i "err" za stderr 
Cstr FILE_PATH = 0;
int procPipe[BROJ_PROCESA*2][2];	// 2 stanja (0 za citanje i 1 za pisanje)
int VRIJEME_PISANJA = 0;		// Vrijeme pisanja u kriticnom odsjecku
int BROJ_PISANJA_PROCESA0 = 2;
time_t POCETAK_IZVODJENJA = 0;		// Vrijeme pocetka izvodjenja programa
int VRIJEME_IZMEDJU_PISANJA_MSEC[BROJ_PROCESA] = {1300, 2300, 1450, 1000, 1300, 2300, 1000, 1300};

enum KomunikacijskeKomande {
	EXIT = -1, WAIT, READY, GO, DONE
};

int main(int argc, Cstr* argv) {
	time(&POCETAK_IZVODJENJA);
	
	LOG_PATH = malloc(10);
	strcpy(LOG_PATH, "log.txt");
	FILE_PATH = malloc(10);
	strcpy(FILE_PATH, "file.txt");

	for (int i = 1; i < argc; i++) {
		char argName[250];
		if (!strcmp(argv[i], "-help")) {
			Error("Help not implemented");
		}
		else if (!strcmp(StrLow(argv[i]), "-r") || !strcmp(StrLow(argv[i]), "--random")) {
			if (i + 1 == argc) {
				snprintf(argName, 150, "Opcija %s koristi se kao \"%s %s [on/off]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
			RANDOM = (!strcmp(StrLow(argv[++i]), "on")) ? 1 : 0;
		}
		else if (!strcmp(StrLow(argv[i]), "-c") || !strcmp(StrLow(argv[i]), "--count")) {
			if (i + 1 == argc) {
				snprintf(argName, 150, "Opcija %s koristi se kao \"%s %s [broj pisanja procesa #0]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
			BROJ_PISANJA_PROCESA0 = atoi(argv[++i]);
		}
		else if (!strcmp(StrLow(argv[i]), "-t") || !strcmp(StrLow(argv[i]), "--timeOfWriting")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [vrijeme pisanja svakog procesa]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
		 	VRIJEME_PISANJA = atoi(argv[++i]);
		}
		else if (!strcmp(StrLow(argv[i]), "-log") || !strcmp(StrLow(argv[i]), "--logPath")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [putanja do lokacije za spremanje log datoteke]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
			LOG_PATH = realloc(LOG_PATH, strlen(argv[++i]) + 1);
			strcpy(LOG_PATH, argv[i]);
		}
		else if (!strcmp(StrLow(argv[i]), "-o") || !strcmp(StrLow(argv[i]), "--filePath")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [putanja do lokacije za spremanje datoteke]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
			FILE_PATH = realloc(FILE_PATH, strlen(argv[++i]) + 1);
			strcpy(FILE_PATH, argv[i]);
		}
		else if (!strcmp(StrLow(argv[i]), "-tbw") || !strcmp(StrLow(argv[i]), "--timeBeforeWrite")) {
			if (i + 2 >= argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [#procesa/all] [vrijeme u ms]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
			if (!strcmp(StrLow(argv[++i]), "all")) {
				i++;
				for (int j = 0; j < BROJ_PROCESA; j++)
					VRIJEME_IZMEDJU_PISANJA_MSEC[j] = atoi(argv[i]);
			} else {
				int id = atoi(argv[i]);
				if (id < 0 || id > 7) Error("Broj procesa mora biti od 0 do 7");
				VRIJEME_IZMEDJU_PISANJA_MSEC[id] = atoi(argv[++i]);
			}
		}
		else if (argc != 1) {
			snprintf(argName, 150, "Argumenti nisu prepoznati\n\e[0;37mUpiši \"%s -help\" za pomoć!", argv[0]);
			Error(argName);
		}
	}


// ***** POSTAVLJANJE KOMUNIKACIJE S PROCESIMA *****
	for (int i = 0; i < BROJ_PROCESA*2; i++)
		if (pipe(procPipe[i]) == -1) Error("Komunikacija preko \"Pipe-a\" nije uspjela");

	int flags = fcntl(procPipe[0][0], F_GETFL, 0) | O_NONBLOCK;  // spremanje ostalih zastavica pipe-a i dodavanje NONBLOCK
	for (int i = 0; i < BROJ_PROCESA*2; i++) 
		fcntl(procPipe[i][0], F_SETFL, flags);	// gasenje blokiranja programa ako je read() prazan
	
	int childPid = fork();	

	if (childPid == 0)
		ProcesTreeCreator();

// ***** UPRAVLJANJE PROCESIMA: SEMAFOR *****	

	for (int i = 0; i < BROJ_PROCESA*2; i++) {
		if (i < BROJ_PROCESA) close(procPipe[i][0]);	//Sa doljnjih ce samo pisati pa treba zabraniti citanje
		else close(procPipe[i][1]);			// Sa gornjih (id*2) ce samo dobivati info pa ne smije pisati
	}
	 
	FILE *log;
	if (!strcmp(LOG_PATH, "out")) log = stdout;
	else if (!strcmp(LOG_PATH, "err")) log = stderr;
	else log = fopen(LOG_PATH, "w");

	int komanda = GO;
	while (komanda != EXIT) {
		for (int i = 0; i < BROJ_PROCESA && komanda != EXIT; i++) {
			int dobivenaPoruka;
			if (read(procPipe[i+BROJ_PROCESA][0], &dobivenaPoruka, 4) == -1) continue;
			else if (dobivenaPoruka == EXIT) { komanda = EXIT; fprintf(log, "Id %d poslao zahtjev o gašenju programa\n", i); continue; }
			else if (dobivenaPoruka != READY) continue;
			
			komanda = GO;
			write(procPipe[i][1], &komanda, 4);

			do {
				read(procPipe[i+BROJ_PROCESA][0], &dobivenaPoruka, 4);
				fprintf(log, "Proces #%d odgovorio sa %s\n", i, IntToKomKom(dobivenaPoruka));
			} while (dobivenaPoruka != DONE);
			fprintf(log, "Id %d pisao u file!\n", i);
		}
	}
	
	fclose(log);

	if (KillRunning()) Error("Nije uspjela terminacija svih child procesa!");

	sleep(3);

	fprintf(log, "Main zavrsio!\n");
	exit(0);
}

int KillRunning() {
	FILE* log;
	if (!strcmp(LOG_PATH, "out")) log = stdout;
	else if (!strcmp(LOG_PATH, "err")) log = stderr;
	else log = fopen(LOG_PATH, "a");

	fprintf(log, "Pripremanje za gašenja procesa\n");

	int kill = EXIT;
	for (int i = 0; i < BROJ_PROCESA; i++) {
		write(procPipe[i][1], &kill, 4);
		fprintf(log, "Poslan KILL procesu #%d\n", i);
	}
	
	int returnVal = 0;
	for (int i = BROJ_PROCESA; i < BROJ_PROCESA*2; i++) {
		const int maxRetry = 10;
		int retry = maxRetry;
		do {
			read(procPipe[i][0], &kill, 4);
			fprintf(log, "Primljen %5s od procesa #%d (preostali broj pokusaja %d)\n", IntToKomKom(kill), i-BROJ_PROCESA, retry);
			if (kill != EXIT) retry--;
			sleep(0.01 * pow(2, maxRetry-retry));  // Da proces ima vremena odgovoriti ukoliko zatvara FILE ili slicno
		} while (kill != EXIT && retry > 0);
		if (retry <= 0) { fprintf(log, "Proces #%d nije odgovorio na zahtjev KILL\n", i-BROJ_PROCESA); returnVal = -1; }
	}

	fclose(log);
	return returnVal;
}

void WriteToFile(int id) {
	close(procPipe[id+BROJ_PROCESA][0]);  // Zabrana citanja s id*2 pipe
	close(procPipe[id][1]);   //  Zabrana pisanja na id pipe
	int poruka = WAIT;
	srand(id*time(NULL));

	float vrijemeIzmedjuPisanjaMs = VRIJEME_IZMEDJU_PISANJA_MSEC[id] / 1000.0;
	int count = 0;

	while(poruka != EXIT) {
		float pravoVrijemeIzmedjuPisanja = vrijemeIzmedjuPisanjaMs + (float)rand()/(float)RAND_MAX*1.96*RANDOM;
										 // Daje random broj od 0 - 1.96       ^Omogućuje randomizaciju
		sleep(pravoVrijemeIzmedjuPisanja);

		poruka = READY;
		write(procPipe[id+BROJ_PROCESA][1], &poruka, 4);
		
		struct timespec pocetakCekanja, krajCekanja;
		clock_gettime(CLOCK_MONOTONIC_RAW, &pocetakCekanja);

		// Cekanje odobrenja za ulazak u kriticni odsjecak
		int readFail = 0;
		do {
			readFail = read(procPipe[id][0], &poruka, 4); // Citanje trenutne komande
		} while (readFail == -1 || (poruka == WAIT && poruka != GO && poruka != EXIT));
		if (poruka == EXIT) continue;
		
		clock_gettime(CLOCK_MONOTONIC_RAW, &krajCekanja);

		// ***** KRITICNI ODSJECAK *****

		FILE *fp;
		if (!strcmp(FILE_PATH, "out")) fp = stdout;
		else if (!strcmp(FILE_PATH, "err")) fp = stderr;
		else fp = fopen(FILE_PATH, "a");
		count++;

		if (VRIJEME_PISANJA) sleep(VRIJEME_PISANJA / 1000.0); // Emulacija programa koji dugo piše unutar kritičnog odsječka

		long int vrijemeCekanjaMicroSec = (krajCekanja.tv_sec - pocetakCekanja.tv_sec) * 1000000 + (krajCekanja.tv_nsec - pocetakCekanja.tv_nsec) / 1000;
		float vrijemeCekanjaMs = (vrijemeCekanjaMicroSec) / 1000.0;

		fprintf(fp, "[%2ld] Zapis #%02d procesa #%02d (%d) i cekao je %9.4fms", time(NULL)-POCETAK_IZVODJENJA, count, id, getpid(), vrijemeCekanjaMs);
		fprintf(fp, " | Vrijeme između pisanja:  %1.2fs", vrijemeIzmedjuPisanjaMs);
		if (RANDOM) fprintf(fp, " (+%fs)", pravoVrijemeIzmedjuPisanja - vrijemeIzmedjuPisanjaMs); 
		fprintf(fp, "\n");

		fclose(fp);

		poruka = DONE;
		write(procPipe[id+BROJ_PROCESA][1], &poruka, 4);
		poruka = WAIT;

		// ***** KRAJ KRITICNOG ODSJECKA *****

		if (id == 0 && count >= BROJ_PISANJA_PROCESA0) { poruka = EXIT; write(procPipe[id+BROJ_PROCESA][1], &poruka, 4); }
	}

	poruka = EXIT; //Potvrda o zavrsetku
	write(procPipe[id+BROJ_PROCESA][1], &poruka, 4);

	printf("#%d je zavrsio!\n", id);
	exit(0);
}

void ProcesTreeCreator() {
/*
			
			GLAVNI (PROC 0)
			  |
		(PROC 1) --- (PROC 2)
		   |		|
	(PROC 3) --|  (PROC 5) --- (PROC 6)	
	(PROC 4) --|	 	      |
				      |-- (PROC 7)	

	IME PROCESA	VRIJEME IZMEĐU PISANJA (sec)
	PROC0			1.30	
	PROC1			2.30	
	PROC2			1.45	
	PROC3			1.00	
	PROC4			1.30	
	PROC5			2.30	
	PROC6			1.00	
	PROC7			1.30	


	Vremena je moguce promjeniti komandama!
							*/

	int id = fork();
	if (!id) proc1();
	
	id = fork();
	if (!id) proc2();

	WriteToFile(0);
}

void proc1() {
	int id = fork();
	if (!id) WriteToFile(3);

	id = fork();
	if (!id) WriteToFile(4);
	
	WriteToFile(1);
}

void proc2() {
	int id = fork();
	if (!id) WriteToFile(5);

	id = fork();
	if (!id) proc6();

	WriteToFile(2);
}

void proc6() {
	int id = fork();
	if (!id) WriteToFile(7);

	WriteToFile(6);
}

Cstr IntToKomKom(int input) {
	switch (input) {
		case EXIT: return "EXIT";
		case WAIT: return "WAIT";
		case READY: return "READY";
		case GO: return "GO";
		case DONE: return "DONE";
		default: { Cstr errout = malloc(50);
			   sprintf(errout, "Nepoznat Komunikacijski Kod %d", input);
			   Error(errout); }
	}

	Error("Unreachable");
	return "";
}

Cstr StrLow(Cstr input) {
	
	int inLen = strlen(input);
	Cstr output = malloc(inLen + 1);
	
	int i;
	for (i = 0; i < inLen && input[i] != '\0'; i++) 
		output[i] = tolower(input[i]);
	for (; i < inLen; i++) 
		output[i] = '\0';

	return output;
}
