#!/bin/bash

rm -rf *.o *.lo *.so
gcc -c -Wall -fPIC lpm.c log.c hmappriv.c memcached_client.c
gcc -shared -o liblpm.so lpm.o log.o hmappriv.o memcached_client.o -lmemcached -lm -ldl
LD_PRELOAD=./liblpm.so /usr/bin/touch file
#LD_PRELOAD=./liblpm.so /bin/ln -s file file_ln
#LD_PRELOAD=./liblpm.so /bin/mv file file.log
#LD_PRELOAD=./liblpm.so /bin/rm file_ln
#LD_PRELOAD=./liblpm.so /bin/rm file.log
#LD_PRELOAD=./liblpm.so /bin/mkdir a 
#LD_PRELOAD=./liblpm.so /bin/rmdir a 
