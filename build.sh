#!/bin/bash
CFlags=" -Wall -Wextra -pedantic -lm "
Debug=""
argc=$#
if [ $argc -eq 1 ] && [ $1 = "dbg" ] 
then 
	Debug="-g" 
fi

gcc src/monitor.c -o Monitor.out $CFlags $Debug && echo "Finished Building" || echo "Build Failed" &&
rm file.txt &&
rm log.txt

