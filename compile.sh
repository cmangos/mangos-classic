#!/bin/bash

export DEBIAN_FRONTEND=noninteractive
export BUILD_TYPE=Release
export CXX=g++
export CC=gcc
gcc --version
if [ $CC = "gcc" ] ; then export CC=gcc-4.8 CXX=g++-4.8 ; fi
mkdir _build
mkdir _install
cd _build
cmake -DCMAKE_INSTALL_PREFIX=../_install -BUILD_PLAYERBOT=ON -DBUILD_SCRIPTDEV=ON -DBUILD_GAME_SERVER=ON -BUILD_LOGIN_SERVER=ON -DPCH=OFF ..
make -j4
make install

