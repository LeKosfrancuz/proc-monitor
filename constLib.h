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

#define BROJ_PROCESA 8
#define RED_CC "\x1B[1;31m"
#define WHITE_CC "\x1B[1;37m"
#define LWHITE_CC "\x1B[0;37m"
#define GRN_CC "\x1B[1;32m"
#define CYN_CC "\x1B[1;36m"
#define MAG_CC "\x1B[1;35m"
#define YELW_CC "\x1B[1;33m"

extern int KillRunning();
extern void WriteToFile(int id);
extern void ProcesTreeCreator();
extern void proc1();
extern void proc2();
extern void proc6();
extern Cstr IntToKomKom(int input);
extern Cstr StrLow(Cstr input);
void Error(Cstr message) {
	printf("%sERROR: %s%s%s\n", RED_CC, WHITE_CC, message, LWHITE_CC);
	exit(1);
}


#endif
