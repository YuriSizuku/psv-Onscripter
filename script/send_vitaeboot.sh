#!/bin/sh 
# sh ./send_vitaeboot.sh vitaip

# config informations here
vitaip=$1
titleid=VITAONSJH
# curl -T ./../build/src/onsjh_vitagui/onsjh_vitagui.self ftp://$vitaip:1337/ux0:/app/PSVONSEMU/eboot.bin
# curl -T ./../build/src/onsjh/onsjh.suprx ftp://$vitaip:1337/ux0:/app/PSVONSEMU/module/onsjh.suprx
curl -T ./../build/src/onsjh/onsjh.self ftp://$vitaip:1337/ux0:/app/$titleid/eboot.bin
curl -T ./../build/src/onsjh/onsjh.self ftp://$vitaip:1337/ux0:/app/$titleid/module/onsjh.self
