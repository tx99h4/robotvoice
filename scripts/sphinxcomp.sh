#!/bin/sh
# script de compilation de la librairie pocketSphinx
# pour utilisation dans la carte colibri
# 2009 Cherik Muhiy-eddine

# répertoire contenant le résultat de la compilation
mkdir $HOME/colibri/src/arm/devdep
cd    $HOME/colibri/src/arm/devdep

# télécharger les archives nécessaires
wget http://switch.dl.sourceforge.net/sourceforge/cmusphinx/sphinxbase-0.4.1.tar.bz2
wget http://switch.dl.sourceforge.net/sourceforge/cmusphinx/pocketsphinx-0.5.1.tar.bz2

# décompresser les archives
tar xjvf sphinxbase-0.4.1.tar.bz2
tar xjvf pocketsphinx-0.5.1.tar.bz2

mv sphinxbase-0.4.1   sphinxbase
mv pocketsphinx-0.5.1 pocketsphinx

# compiler sphinxbase
cd sphinxbase

./configure --prefix=$HOME/colibri/src/arm/devdep --enable-fixed --enable-shared CC=/usr/local/arm/oe/bin/arm-linux-gnueabi-gcc CPP=/usr/local/arm/oe/bin/arm-linux-gnueabi-cpp CPPFLAGS=-I/usr/local/arm/oe/include/ LDFLAGS=-L/usr/local/arm/oe/lib  --target=arm-linux-gnueabi --host=i386-pc-linux-gnu --without-python

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
make install

# compiler pocketsphinx
cd ../pocketsphinx

./configure --prefix=$HOME/colibri/src/arm/devdep --with-sphinxbase=`pwd`/../sphinxbase --enable-shared CC=/usr/local/arm/oe/bin/arm-linux-gnueabi-gcc CPP=/usr/local/arm/oe/bin/arm-linux-gnueabi-cpp CPPFLAGS=-I/usr/local/arm/oe/include/ LDFLAGS=-L/usr/local/arm/oe/lib --target=arm-linux-gnueabi --host=i386-pc-linux-gnu --without-python

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
make install

