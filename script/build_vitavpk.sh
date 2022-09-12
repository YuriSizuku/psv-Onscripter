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

# prepare for build
pushd ..
if ! [  -d build ]; then mkdir build; fi

pushd build
cmake .. -G "Unix Makefiles"  \
    -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"
make $TARGET
popd
popd