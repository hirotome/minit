#!/bin/sh
gcc -static -o2 -s -o minit minit.c
sleep 1
strip --strip-all minit
