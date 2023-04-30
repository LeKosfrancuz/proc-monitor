#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

typedef char* Cstr;

int KillRunning();
void WriteToFile(int id, int vrijemeCekanjaMs);
void ProcesTreeCreator();
void proc1();
void proc2();
void proc6();
void Error(Cstr message) {
	printf("\e[1;31mERROR: \e[1;37m%s\e[0;37m\n", message);
	exit(1);
}

#define BROJ_PROCESA 8
#define RANDOM 1

Cstr LOG_PATH = "log.txt";  // "out" za stdout i "err" za stderr 
Cstr FILE_PATH = "file.txt";
int procPipe[BROJ_PROCESA*2][2]; // 2 stanja (0 za citanje i 1 za pisanje)

enum KomunikacijskeKomande {
	EXIT = -1, WAIT, READY, GO, DONE
};

int main() {
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
	 

	int komanda = GO, count = 0;
	while (count < 3) {
		count++;
		for (int i = 0; i < BROJ_PROCESA; i++) {
			int dobivenaPoruka;
			if (read(procPipe[i+BROJ_PROCESA][0], &dobivenaPoruka, 4) == -1) continue;
			else if (dobivenaPoruka != READY) continue;
			
			komanda = GO;
			write(procPipe[i][1], &komanda, 4);

			do {
				read(procPipe[i+BROJ_PROCESA][0], &dobivenaPoruka, 4);
			} while (dobivenaPoruka != DONE);
		}
	}

	if (KillRunning()) Error("Nije uspjela terminacija svih child procesa!");

	sleep(3);

	printf("Main zavrsio!\n");
	exit(0);
}

int KillRunning() {
	FILE* log;
	if (!strcmp(LOG_PATH, "out")) log = stdout;
	else if (!strcmp(LOG_PATH, "err")) log = stderr;
	else log = fopen(LOG_PATH, "w");

	fprintf(log, "Pripremanje za gašenja procesa\n");

	int kill = EXIT;
	for (int i = 0; i < BROJ_PROCESA; i++) {
		write(procPipe[i][1], &kill, 4);
		fprintf(log, "Poslan KILL(%d) procesu #%d\n", EXIT, i);
	}

	for (int i = BROJ_PROCESA; i < BROJ_PROCESA*2; i++) {
		int retry = 9;
		do {
			read(procPipe[i][0], &kill, 4);
			fprintf(log, "Primljen %d od procesa #%d (preostali broj pokusaja %d)\n", kill, i, retry);
			retry--;
			sleep(0.1);  // Da proces ima vremena odgovoriti ukoliko zatvara FILE ili slicno
		} while (kill != EXIT && retry > 0);
		if (retry <= 0) return -1;
	}

	fclose(log);
	return 0;
}

void WriteToFile(int id, int vrijemeCekanjaMs) {
	close(procPipe[id+BROJ_PROCESA][0]);  // Zabrana citanja s id*2 pipe
	close(procPipe[id][1]);   //  Zabrana pisanja na id pipe
	int poruka = WAIT;
	srand(id*time(NULL));
	while(poruka != EXIT) {
		float pravoVrijemeCekanja = vrijemeCekanjaMs/1000.0 + (float)rand()/(float)RAND_MAX*1.96*RANDOM;
		sleep(pravoVrijemeCekanja);

		poruka = READY;
		write(procPipe[id+BROJ_PROCESA][1], &poruka, 4);

		while (poruka != EXIT && poruka != WAIT){
			do {
				read(procPipe[id][0], &poruka, 4); // Citanje trenutne komande
			} while (poruka == WAIT && poruka != GO && poruka != EXIT);
			if (poruka == EXIT) continue;

			printf("Ovo je proces #%d (%d) i ceka %1.2fs  (+%fs)\n", id, getpid(), vrijemeCekanjaMs/1000.0, 
									pravoVrijemeCekanja - vrijemeCekanjaMs/1000.0);
			//Error("WRITE to FILE not implemented");

			poruka = DONE;
			write(procPipe[id+BROJ_PROCESA][1], &poruka, 4);
			poruka = WAIT;
		}
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
							*/

	int id = fork();
	if (!id) proc1();
	
	id = fork();
	if (!id) proc2();

	WriteToFile(0, 1300);
}

void proc1() {
	int id = fork();
	if (!id) WriteToFile(3, 1000);

	id = fork();
	if (!id) WriteToFile(4, 1300);
	
	WriteToFile(1, 1300);
}

void proc2() {
	int id = fork();
	if (!id) WriteToFile(5, 2300);

	id = fork();
	if (!id) proc6();

	WriteToFile(2, 1450);
}

void proc6() {
	int id = fork();
	if (!id) WriteToFile(7, 1300);

	WriteToFile(6, 1000);
}

