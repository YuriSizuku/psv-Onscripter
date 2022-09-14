#!/bin/sh 
# sh ./send_vitaeboot.sh vitaip

# config informations here
vitaip=$1
titleid=VITAONSJH

curl -T ./../build/src/onsjh_vitagui/onsjh_vitagui.self ftp://$vitaip:1337/ux0:/app/$titleid/eboot.bin
curl -T ./../build/src/onsjh/onsjh.self ftp://$vitaip:1337/ux0:/app/$titleid/onsjh.bin
