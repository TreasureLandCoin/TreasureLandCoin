TreasureLandCoin
==========

[Treasure Land](https://treasurelandcoin.com/) is a virtual world where the player’s main objective is to explore, travel and mine valuable ores which contain the TreasureLand coins (TLC). Players travel through the Treasure Land by moving with their phones in the real world.
The purpose of the player in the Treasure Land is to find, mine and process valuable ores scattered around the world in order to receive the TreasureLand coins (TLC).
This Guthub contains the blockchain coin code behind TLC.
	
	Copyright (c) 2017 Treasure Land Coin
	License: GNU General Public License version 3, see COPYING

	Portions copyright (c) 2009-2016 The Bitcoin Core developers
	Portions copyright (c) 2014-2017 Coin Sciences Ltd

System requirements
-------------------

These compilation instructions have been tested on Ubuntu 14.04 x64 only.

C++ compilers are memory-hungry, so it is recommended to have at least 1 GB of memory available when compiling TreasureLandCoin. With less memory, compilation may take much longer due to swapfile thrashing.


Linux Build Notes (on Ubuntu 14.04 x64)
=================

Install dependencies
--------------------

    sudo apt-get update
    sudo apt-get install build-essential libtool autotools-dev automake pkg-config libssl-dev libevent-dev bsdmainutils
    sudo apt-get install libboost-all-dev
    sudo apt-get install git
    sudo apt-get install software-properties-common
    sudo add-apt-repository ppa:bitcoin/bitcoin
    sudo apt-get update
    sudo apt-get install libdb4.8-dev libdb4.8++-dev

Compile TreasureLandCoin for Ubuntu (64-bit)
-----------------------------

    ./autogen.sh
    ./configure
    make

Notes
-----

* This will build `treasurelandcoind`, `treasurelandcoin-cli` and `treasurelandcoin-util` in the `src` directory.

* The release is built with GCC after which `strip treasurelandcoind` strings the debug symbols, which reduces the executable size by about 90%.


Windows Build Notes (on Ubuntu 14.04 x64)
=====================

Install dependencies
--------------------

    sudo apt-get update
    sudo apt-get install build-essential libtool autotools-dev automake pkg-config libssl-dev libevent-dev bsdmainutils
    sudo apt-get install g++-mingw-w64-i686 mingw-w64-i686-dev g++-mingw-w64-x86-64 mingw-w64-x86-64-dev curl
    sudo apt-get install libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev
    sudo apt-get install git
    sudo add-apt-repository ppa:bitcoin/bitcoin
    sudo apt-get update
    sudo apt-get install libdb4.8-dev libdb4.8++-dev

Compile TreasureLandCoin for Windows (64-bit)
------------------------------

    ./autogen.sh
    cd depends
    make HOST=x86_64-w64-mingw32 -j4
    cd ..
    ./configure --prefix=`pwd`/depends/x86_64-w64-mingw32 --enable-cxx --disable-shared --enable-static --with-pic
    make

Notes
-----

* This will build `treasurelandcoind.exe`, `treasurelandcoin-cli.exe` and `treasurelandcoin-util.exe` in the `src` directory.
