#!/bin/zsh

set -ex

CFLAGS=-std=c++0x

apxs2 -i -a -c -nsass -I/usr/include/apache2 -I/usr/include/apr-1.0 -I../libsass -L../libsass -lapr-1 -lsass -lstdc++ mod_sass.c
