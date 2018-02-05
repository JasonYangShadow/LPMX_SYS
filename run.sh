#!/bin/bash

rm -rf *.o *.lo *.so
gcc -c -Wall -fPIC lpm.c log.c hashmap.c hmappriv.c ipc_rcv.c
gcc -shared -o liblpm.so lpm.o log.o hashmap.o hmappriv.o ipc_rcv.o -pthread -lm -ldl
LD_PRELOAD=./liblpm.so /usr/bin/touch file
LD_PRELOAD=./liblpm.so /bin/ln -s file file_ln
LD_PRELOAD=./liblpm.so /bin/mv file file.log
LD_PRELOAD=./liblpm.so /bin/rm file_ln
LD_PRELOAD=./liblpm.so /bin/rm file.log
LD_PRELOAD=./liblpm.so /bin/mkdir a 
LD_PRELOAD=./liblpm.so /bin/rmdir a 
