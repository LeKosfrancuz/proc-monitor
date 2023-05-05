#ifndef PROC_HANDLE_C
#define PROC_HANDLE_C

#include "constLib.h"

int KillRunning(Cstr logPath) {
	FILE* log;
	if (!strcmp(logPath, "out")) log = stdout;
	else if (!strcmp(logPath, "err")) log = stderr;
	else log = fopen(logPath, "a");

	fprintf(log, "%sPripremanje za gašenja procesa%s\n", CYN_CC, LWHITE_CC);

	int kill = EXIT;
	for (int i = 0; i < BROJ_PROCESA; i++) {
		write(procPipe[i][1], &kill, 4);
		fprintf(log, "Poslan %sKILL%s procesu %s#%d%s\n", RED_CC, LWHITE_CC, CYN_CC, i, LWHITE_CC);
	}
	
	int returnVal = 0;
	for (int i = BROJ_PROCESA; i < BROJ_PROCESA*2; i++) {
		const int maxRetry = 10;
		int retry = maxRetry;
		do {
			read(procPipe[i][0], &kill, 4);
			fprintf(log, "Primljen %6s od procesa %s#%d%s (preostali broj pokušaja %d)\n", IntToKomKom(kill), CYN_CC, i-BROJ_PROCESA, LWHITE_CC, retry);
			if (kill != EXIT) retry--;
			sleep(0.01 * pow(2, maxRetry-retry));  // Da proces ima vremena odgovoriti ukoliko zatvara FILE ili slicno
		} while (kill != EXIT && retry > 0);
		if (retry <= 0) { fprintf(log, "Proces %s#%d%s nije odgovorio na zahtjev KILL\n", CYN_CC, i-BROJ_PROCESA, LWHITE_CC); returnVal = -1; }
	}

	fclose(log);
	return returnVal;
}

void ProcesTreeCreator(Cstr filePath) {
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
	if (!id) proc1(filePath);
	
	id = fork();
	if (!id) proc2(filePath);

	WriteToFile(0, filePath);
}

void proc1(Cstr filePath) {
	int id = fork();
	if (!id) WriteToFile(3, filePath);

	id = fork();
	if (!id) WriteToFile(4, filePath);
	
	WriteToFile(1, filePath);
}

void proc2(Cstr filePath) {
	int id = fork();
	if (!id) WriteToFile(5, filePath);

	id = fork();
	if (!id) proc6(filePath);

	WriteToFile(2, filePath);
}

void proc6(Cstr filePath) {
	int id = fork();
	if (!id) WriteToFile(7, filePath);

	WriteToFile(6, filePath);
}

#endif
