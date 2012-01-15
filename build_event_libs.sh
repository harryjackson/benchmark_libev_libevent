#!/bin/bash
mkdir -p ./lib

cd ./libev-4.04/
make clean
./configure --prefix=`pwd`/../lib/
make install

cd ../libevent-2.0.16-stable/
./configure --prefix=`pwd`/../lib/
make install
cd ../

