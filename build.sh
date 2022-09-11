#!/bin/sh 
# sh ./build_vitasdk target [SDKDIR]

# prepare env
if [ -z $1 ]; then
    TARGET=all
else
    TARGET=$1
fi

if [ -z $2 ]; then
    export VITASDK=/d/AppExtend/SDK/psvsdk 
else
    export VITASDK=$1
fi
export PATH=$VITASDK/bin:$PATH


make $TARGET