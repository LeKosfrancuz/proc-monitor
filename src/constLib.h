#ifndef CONSTLIB_H
#define CONSTLIB_H

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

typedef struct {
	int read;
	int write;
} Pipe;

#define BROJ_PROCESA 8
#define RED_CC "\x1B[1;31m"
#define WHITE_CC "\x1B[1;37m"
#define LWHITE_CC "\x1B[0;37m"
#define GRN_CC "\x1B[1;32m"
#define CYN_CC "\x1B[1;36m"
#define MAG_CC "\x1B[1;35m"
#define YELW_CC "\x1B[1;33m"

#if defined(DEBUG)
	#define DEBUGLN printf("%s:%d func %s\n", __FILE__, __LINE__, __func__);
	#define DEBUGINT(num) printf("%s:%d func %s ... %d\n", __FILE__, __LINE__, __func__, num);
	#define	DEBUGSTR(msg) printf("%s:%d func %s ... %s\n", __FILE__, __LINE__, __func__, msg);
#endif

extern int KillRunning(Cstr logPath, Pipe procPipes[BROJ_PROCESA]);
extern void WriteToFile(int id, Cstr filePath, Pipe pipe);
extern void ProcesTreeCreator(Cstr filePath, int procPipe[BROJ_PROCESA*2][2]);
extern void proc1(Cstr filePath, Pipe pipes[BROJ_PROCESA]);
extern void proc2(Cstr filePath, Pipe pipes[BROJ_PROCESA]);
extern void proc6(Cstr filePath, Pipe pipes[BROJ_PROCESA]);
extern Cstr IntToKomKom(int input);
extern Cstr StrLow(Cstr input);
void Error(Cstr message) {
	printf("%sERROR: %s%s%s\n", RED_CC, WHITE_CC, message, LWHITE_CC);
	exit(1);
}


#endif
