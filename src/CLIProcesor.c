#ifndef CLI_PROC_C
#define CLI_PROC_C

#include "constLib.h"
#include <stdio.h>


void ApplyCommandLineArgs(int argc, Cstr* argv, Cstr* logPath, Cstr* filePath) {

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
		else if (!strcmp(StrLow(argv[i]), "-t") || !strcmp(StrLow(argv[i]), "--time-of-writing")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [vrijeme pisanja svakog procesa]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
		 	VRIJEME_PISANJA = atoi(argv[++i]);
		}
		else if (!strcmp(StrLow(argv[i]), "-log") || !strcmp(StrLow(argv[i]), "--log-path")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [putanja do lokacije za spremanje log datoteke]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
			*logPath = realloc(*logPath, strlen(argv[++i]) + 1);
			strcpy(*logPath, argv[i]);
		}
		else if (!strcmp(StrLow(argv[i]), "-o") || !strcmp(StrLow(argv[i]), "--file-path")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [putanja do lokacije za spremanje datoteke]\"", argv[i], argv[0], argv[i]);
				Error(argName);
			}
			*filePath = realloc(*filePath, strlen(argv[++i]) + 1);
			strcpy(*filePath, argv[i]);
		}
		else if (!strcmp(StrLow(argv[i]), "-tbw") || !strcmp(StrLow(argv[i]), "--time-before-write")) {
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
			snprintf(argName, 150, "Argument (%s) nije prepoznat\n\x1B[0;37mUpiši \"%s -help\" za pomoć!", argv[i], argv[0]);
			Error(argName);
		}
	}
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

#endif
