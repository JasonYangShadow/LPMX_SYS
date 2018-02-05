#!/bin/bash

rm -rf memcache
gcc -c memcached.c memcached_client.c log.c
gcc -o memcache memcached.o memcached_client.o log.o -lmemcached
