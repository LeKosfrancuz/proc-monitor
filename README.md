# Semafor za procese
Mali program koji zapocne nekoliko procesa i onda upravlja njima u kriticnom odsječku kod pisanja u file


## Setup
Potrebno je prvo build-at program za vlastiti os. To se može napraviti skriptom: `$ ./build.sh` ili komandom:

```bash
clang semafor.c -o Semafor.out -lm -std=gnu99 -Wall -Wextra
```

Potrebno je koristiti `-lm` zbog koristenja `math.h` library-a.

>***Napomena:** najranija verzija C-a je `gnu99` zbog koristenja funkcija za mjerenje vremena.*


## Korištenje
Posto nisam dodao -help, sljede jednostavne upute za koristenje

### **__Nasumicna vremena cekanja__**
Moguće je koristiti opcije `-r` ili `--random` za nasumicna vremena prije pisanja.
```bash
# Upaliti će randomiser za vremena prije kriticnog odsjecka
./Semafor.out --random on

# Ponovno će pokrenuti program, ali ovaj put će random biti ugašen
./Semafor.out --random off
```
>***__Napomena:__** nije potrebno ugasiti randomiser jer je ugašen po *default*-u*

Opcija ce dodati na postojece *default* vrijeme nasumican broj u intervalu **[0 - 1.96]**


### **__Prekid nakon n zapisa__**
Ukoliko treba prekinuti izvođenje programa nakon određenog broja zapisa (*default*: 2) moguće je koristiti opcije `-c` ili `-count`.
```bash
# Program će se prekinuti nakod 5 zapisa u file (koje napravi proc0)
./Semafor.out --count 5
```
Zapisi se ne broje zajedno već će se program prekinuti kada proc0 napravi n zapisa.


### **__Vrijeme u kritičnom odsječku__**
Moguće je simulirati procese koji jako puno vremena provode u kritičnom odsječku i tako usporavaju sve ostale. To se radi opcijom `-t` ili `-time-of-writing` (*default*: 0).
```bash
# Svi procesi provesti će 1s više u kriticnom odsjeku usporavajuci program.
./Semafor.out --time-Of-Writing 1000
```
Argument opciji je vrijeme u milisekundama.


### **__Vrijeme prije kritičnog odsječka__**
Kako bi se procesi činili stvarnijima moraju raditi stvari izvan kritičnog odsječka. To se može napraviti s opcijom `-tbw` ili `--time-before-write` (*default*: [Tablica procesa](https://github.com/LeKosfrancuz/proc-semaphore/edit/main/README.md#tablica-procesa))
```bash
# Proces 4 će sada čekati 1.5s izvan kriticnog odsjecka.
./Semafor.out --time-before-write 4 1500
```
Moguće je staviti isto vrijeme svim procesima pomoću argumenta `all`:
```bash
# Proces 2 čekati će 4s izvan kritičnog odsječka, a ostali 1.2s
./Semafor.out -tbw all 1200 -tbw 2 4000
```
Argumenti opciji su broj procesa i vrijeme u milisekundama.


### **__Spremanje dnevničkih zapisa__**
Promjenu zadane lokacije i imena dnevničkog zapisa moguće je promjeniti opcijom `-log` ili `--log-path` te navođenjem nove lokacije. (*default*: "log.txt")
```bash
# Program sprema dnevničke zapise na lokaciju "~/Documents/log file.txt"
./Semafor.out --log-path "~/Documents/log file.txt"
# ili
./Semafor.out --log-path ~/Documents/log\ file.txt
```
Moguće je slati dnevnicke zapise na *standardni izlaz* specificiranjem `-log out` ili `-log stdout` i na *standardni error* s opcijom `-log err` ili `-log stderr`
```bash
# Program će ispisati dnevničke zapise na standardni izlaz
./Semafor.out --log-path out
```

```bash
# Primjer ne prikazivanja niti spremanja dnevničkog zapisa
./Semafor.out --log-path err 2> /dev/null
```


### **__Spremanje datoteke u koju procesi upisuju__**
Promjenu zadane lokacije i imena te datoteke moguće je promjeniti opcijom `-o` ili `--file-path` te navođenjem nove lokacije. (*default*: "file.txt")
```bash
# Program sprema dnevničke zapise na lokaciju "~/Documents/jedna datoteka.txt"
./Semafor.out --file-path "~/Documents/jedna datoteka.txt"
# ili
./Semafor.out --file-path ~/Documents/jedna\ datoteka.txt
```
Kao i dnevnicke zapise, moguće je poslati sadržaj te datoteke na *standardni izlaz* specificiranjem `-o out` ili `-o stdout` i na *standardni error* s opcijom `-o err` ili `-o stderr`
```bash
# Program će ispisati što procesi pišu na standardni izlaz
./Semafor.out --file-path out
```
>***__Napomena:__**  standardni izlaz poprilično je spor pa se možda neće svi zapisi prikazati ako su kratka vremena prije i tijekom kritičnog odsječka. Promjena tih vremena pokazana je [ovdje za izvan k.o.](https://github.com/LeKosfrancuz/proc-semaphore/edit/main/README.md#vrijeme-prije-kritičnog-odsječka) i [ovdje za unutar k.o.](https://github.com/LeKosfrancuz/proc-semaphore/edit/main/README.md#vrijeme-u-kritičnom-odsječku)*

```bash
# Primjer ne prikazivanja niti spremanja datoteke sa zapisima
./Semafor.out --log-path err 2> /dev/null
```


## Tablica procesa
```
                    GLAVNI (PROC 0)
                          |
           (PROC 1) -------------- (PROC 2)
              |                       |
   (PROC 3) --|-- (PROC 4) (PROC 5) --|-- (PROC 6)	
                                            |
                                            |-- (PROC 7)	

   IME PROCESA     VRIJEME IZMEĐU PISANJA (sec)
      PROC0                 1.30	
      PROC1                 2.30	
      PROC2                 1.45	
      PROC3                 1.00	
      PROC4                 1.30	
      PROC5                 2.30	
      PROC6                 1.00	
      PROC7                 1.30	
```
>***Napomena:** Moguće je promjeniti default vremena s opcijama*

## Licenca
Copyright (c) Mateo Kos. All rights reserved. Licensed under the MIT License.
