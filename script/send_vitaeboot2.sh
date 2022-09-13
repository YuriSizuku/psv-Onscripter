#!/bin/sh 
# sh ./send_vitaeboot.sh [vitadir]

# config informations here
vitadir=$1
if [ -z $vitadir ]; then vitadir="/d/game/#emulator/PSV/data"; fi
titleid=VITAONSJH
cp -f ./../build/src/onsjh/onsjh.self $vitadir/ux0/app/$titleid/eboot.bin
cp -f ./../build/src/onsjh/onsjh.self $vitadir/ux0/app/$titleid/module/onsjh.self
