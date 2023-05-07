#ifndef PROC_HANDLE_C
#define PROC_HANDLE_C

#include "constLib.h"

int KillRunning(Cstr logPath, Pipe procPipes[BROJ_PROCESA]) {
	FILE* log;
	if (!strcmp(logPath, "out")) log = stdout;
	else if (!strcmp(logPath, "err")) log = stderr;
	else log = fopen(logPath, "a");

	fprintf(log, "%sPripremanje za gašenja procesa%s\n", CYN_CC, LWHITE_CC);

	int kill = EXIT;
	for (int i = 0; i < BROJ_PROCESA; i++) {
		write(procPipes[i].write, &kill, 4);
		fprintf(log, "Poslan %sKILL%s procesu %s#%d%s\n", RED_CC, LWHITE_CC, CYN_CC, i, LWHITE_CC);
	}
	
	int returnVal = 0;
	for (int i = 0; i < BROJ_PROCESA; i++) {
		const int maxRetry = 10;
		int retry = maxRetry;
		do {
			int readRetVal = read(procPipes[i].read, &kill, 4);
			if (readRetVal == -1) kill = WAIT; 
			fprintf(log, "Primljen %6s od procesa %s#%d%s (preostali broj pokušaja %d)\n", IntToKomKom(kill), CYN_CC, i, LWHITE_CC, retry);
			if (kill != EXIT) retry--;
			sleep(0.01 * pow(2, maxRetry-retry));  // Da proces ima vremena odgovoriti ukoliko zatvara FILE ili slicno
		} while (kill != EXIT && retry > 0);
		if (retry <= 0) { fprintf(log, "Proces %s#%d%s nije odgovorio na zahtjev KILL\n", CYN_CC, i, LWHITE_CC); returnVal = -1; }
	}

	fclose(log);
	return returnVal;
}

void ProcesTreeCreator(Cstr filePath, int procPipe[BROJ_PROCESA*2][2]) {
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

	Pipe procPipes[BROJ_PROCESA];
	for (int i = 0; i < BROJ_PROCESA*2; i++) {
		if (i < BROJ_PROCESA) {
		/* Proces nesmije imat pristup pisati na pipe-a sa kojeg cita, 
		 ovdje taj kraj zatvaramo pa to nije moguće, a drugi kraj dodjelit pipeHandler-u	*/
			close(procPipe[i][1]);	

			procPipes[i].read = dup(procPipe[i][0]);
			close(procPipe[i][0]);
		}
		else {
		/* Sa gornjih (id*2) ce samo pisati info pa ne smije citati */
			close(procPipe[i][0]);
			
			procPipes[i - BROJ_PROCESA].write = dup(procPipe[i][1]);
			close(procPipe[i][1]);
		}
	}

	int id = fork();
	if (!id) proc1(filePath, procPipes);
	
	id = fork();
	if (!id) proc2(filePath, procPipes);

	WriteToFile(0, filePath, procPipes[0]);
}

void proc1(Cstr filePath, Pipe procPipes[BROJ_PROCESA]) {
	int id = fork();
	if (!id) WriteToFile(3, filePath, procPipes[3]);

	id = fork();
	if (!id) WriteToFile(4, filePath, procPipes[4]);
	
	WriteToFile(1, filePath, procPipes[id]);
}

void proc2(Cstr filePath, Pipe procPipes[BROJ_PROCESA]) {
	int id = fork();
	if (!id) WriteToFile(5, filePath, procPipes[5]);

	id = fork();
	if (!id) proc6(filePath, procPipes);

	WriteToFile(2, filePath, procPipes[2]);
}

void proc6(Cstr filePath, Pipe procPipes[BROJ_PROCESA]) {
	int id = fork();
	if (!id) WriteToFile(7, filePath, procPipes[7]);

	WriteToFile(6, filePath, procPipes[6]);
}

#endif
