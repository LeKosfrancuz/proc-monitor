#!/bin/bash
CFlags=" -Wall -Wextra -pedantic -lm "

gcc src/semafor.c -o Semafor.out $CFlags && echo "Finished Building" || echo "Build Failed" &&
rm file.txt &&
rm log.txt

