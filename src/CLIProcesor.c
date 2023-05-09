#ifndef CLI_PROC_C
#define CLI_PROC_C

#include "constLib.h"
#include "lev.c"	// Izracun slicnosti rijeci
#include <stdio.h>


void ApplyCommandLineArgs(int argc, Cstr* argv, Cstr* logPath, Cstr* filePath) {
	Cstr currentArg = 0;
	for (int i = 1; i < argc; i++) {
		char argName[250];
		if (strcmp(argName, "argReSet")) {
			const int lenArgi = strlen(argv[i]);
			currentArg = realloc(currentArg, lenArgi + 1);
			strncpy(currentArg, argv[i], lenArgi + 1);
			strcpy(argName, "argSet");
		}

		if (!strcmp(StrLow(currentArg), "--help") || !strcmp(StrLow(currentArg), "-h")) {
			printStdCVersion();

			Error("Help not implemented");
		}
		else if (!strcmp(StrLow(currentArg), "-r") || !strcmp(StrLow(currentArg), "--random")) {
			if (i + 1 == argc) {
				snprintf(argName, 150, "Opcija %s koristi se kao \"%s %s [on/off]\"", currentArg, argv[0], currentArg);
				Error(argName);
			}
			RANDOM = (!strcmp(StrLow(argv[++i]), "on")) ? 1 : 0;
		}
		else if (!strcmp(StrLow(currentArg), "-c") || !strcmp(StrLow(currentArg), "--count")) {
			if (i + 1 == argc) {
				snprintf(argName, 150, "Opcija %s koristi se kao \"%s %s [broj pisanja procesa #0]\"", currentArg, argv[0], currentArg);
				Error(argName);
			}
			BROJ_PISANJA_PROCESA0 = atoi(argv[++i]);
		}
		else if (!strcmp(StrLow(currentArg), "-t") || !strcmp(StrLow(currentArg), "--time-of-writing")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [vrijeme pisanja svakog procesa]\"", currentArg, argv[0], currentArg);
				Error(argName);
			}
		 	VRIJEME_PISANJA = atoi(argv[++i]);
		}
		else if (!strcmp(StrLow(currentArg), "-log") || !strcmp(StrLow(currentArg), "--log-path")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [putanja do lokacije za spremanje log datoteke]\"", currentArg, argv[0], currentArg);
				Error(argName);
			}
			*logPath = realloc(*logPath, strlen(argv[++i]) + 1);
			strcpy(*logPath, argv[i]);
		}
		else if (!strcmp(StrLow(currentArg), "-o") || !strcmp(StrLow(currentArg), "--file-path")) {
			if (i + 1 == argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [putanja do lokacije za spremanje datoteke]\"", currentArg, argv[0], currentArg);
				Error(argName);
			}
			*filePath = realloc(*filePath, strlen(argv[++i]) + 1);
			strcpy(*filePath, argv[i]);
		}
		else if (!strcmp(StrLow(currentArg), "-tbw") || !strcmp(StrLow(currentArg), "--time-before-write")) {
			if (i + 2 >= argc) {
				snprintf(argName, 250, "Opcija %s koristi se kao \"%s %s [#procesa/all] [vrijeme u ms]\"", currentArg, argv[0], currentArg);
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
			#define BROJ_KOMANDI 14
			Cstr komande[BROJ_KOMANDI] = {"-h", "--help", "-r", "--random", "-c", "--count", "-t", "--time-of-writing", "-log", "--log-path", "-o", "--file-path", "-tbw", "--time-before-write"};
			int index = indexNajSlicnijeRijeci(3, currentArg, komande, BROJ_KOMANDI);

			if (index == -1) {
				snprintf(argName, 150, "Argument (%s) nije prepoznat\n\x1B[0;37mUpiši \"%s -help\" za pomoć!", currentArg, argv[0]);
				Error(argName);
			} else {
				const int duljinaKomande = strlen(komande[index]);
				currentArg = (Cstr)realloc(currentArg, duljinaKomande + 1);
				strncpy(currentArg, komande[index], duljinaKomande + 1);
				strcpy(argName, "argReSet");
				--i;
			}

			
			#undef BROJ_KOMANDI	
		}
	}
	free(currentArg);
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
