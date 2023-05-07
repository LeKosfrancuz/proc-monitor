#ifndef CONSTLIB_H
#define CONSTLIB_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifdef __STRICT_ANSI__
    #define timespec linux_timespec
    #include <stdlib.h>
    #undef timespec
    struct timespec { long tv_sec; long tv_nsec; };    //Redefinicija timespec strukture zbog kolizija sa drugim .h file-ovima
    #define linux_timespec timespec

    int clock_gettime(int f, struct timespec *spec);
#else
    #include <stdlib.h>
#endif


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
extern Cstr StrLow(Cstr input); // Vraca input, ali sve malim slovima [non_destructive]
void Error(Cstr message) {
	printf("%sERROR: %s%s%s\n", RED_CC, WHITE_CC, message, LWHITE_CC);
	exit(1);
}

/* Windows support */
#ifdef _WIN32
    #include <windows.h>
    #define UNIX_OFFSET 116444736000000000ULL
    #define TO_SEC      10000000ULL
    #define CLOCK_MONOTONIC_RAW 4
    // struct timespec { long tv_sec; long tv_nsec; };    //header part

    /* Funcija clock_gettime za windows src:
     * https://stackoverflow.com/a/31335254 */
    int clock_gettime(int f, struct timespec *spec)      //C-file part
    {  __int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
       wintime      -=  UNIX_OFFSET;			//1jan1601 to 1jan1970
       spec->tv_sec  =wintime / TO_SEC;		//seconds
       spec->tv_nsec =wintime % TO_SEC * 100;		//nano-seconds
       return 0;
    }

    void sleep(int seconds) {
        Sleep(seconds * 1000);
    }

/* Linux support */
#elif __linux__
	#ifdef _POSIX_TIMERS  //ANSI verzije nemaju support za clock_gettime; ovim se to omogucuje
		#define _XOPEN_SOURCE 700
		#define _XOPEN_SOURCE_EXTENDED
		#define _POSIX_C_SOURCE 200809L
		#define _TIME_BITS 64
		#define CLOCK_MONOTONIC_RAW 4
	#endif
        #ifndef __STRICT_ANSI__
            #include <time.h>
        #else  // Ukoliko je ANSI verzija, potrebno je ne definirati timespec direktno jer se inace definira vise puta
            #define timespec time_H_timespec
            #include <time.h>
            #undef timespec
        #endif
#endif

void printStdCVersion() {
	#ifdef __STDC_VERSION__ // Verzije nakon C90 definiraju __STDC_VERSION
        switch (__STDC_VERSION__) {
            case 199409:
                printf ("C version: C94 (%ld)", __STDC_VERSION__);
                break;
            case 199901:
                printf ("C version: C99 (%ld)", __STDC_VERSION__);
                break;
            case 201112:
                printf ("C version: C11 (%ld)", __STDC_VERSION__);
                break;
            case 201710:
                printf ("C version: C17 (%ld)", __STDC_VERSION__);
                break;
            default:
                printf ("C version: ?? (%ld)", __STDC_VERSION__);
                break;
        }
    #else
        printf ("C(89), C(90)");
    #endif

    #ifdef __STRICT_ANSI__  // Verzije prije C90 (ukljucen) definirale su __STRICT_ANSI__  (0 -> C89, 1 -> C90+) [npr. gnu11 ce biti C11, ali ne ANSI]
        printf (" (ANSI %d)\n", __STDC__);
    #else
        printf("\n");
    #endif

    return;
}

#endif
