#!/bin/bash -
phpize --clean && phpize
aclocal && libtoolize --force && autoreconf
./configure --enable-ev --enable-ev-debug
make clean
make -j3
