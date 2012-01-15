#!/bin/bash
mkdir -p ./lib
mkdir -p ./graphs

tar -zxvf libev-4.04.tar.gz
tar -zxvf libevent-2.0.16-stable.tar.gz

cd ./libev-4.04/
make clean
./configure --prefix=`pwd`/../lib/
make install

cd ../libevent-2.0.16-stable/
./configure --prefix=`pwd`/../lib/
make install
cd ../

