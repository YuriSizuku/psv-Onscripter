#!/bin/sh 
# sh ./build_vitasdk [vitasdkdir]

# install tools
if [ -n "$(uname -a | grep Linux)" ]; then
    echo In linux enviroment, prepare tools...
    sudo apt-get install git cmake make build-essential
fi

# prepare env
if [ -z $1 ]; then 
    export VITASDK=/opt/vitasdk
else
    export VITASDK=$1
fi
export PATH=$VITASDK/bin:$PATH
echo "config VITASDK=$VITASDK"

# install vitasdk
git clone https://github.com/vitasdk/vdpm ./../build/vpdm
pushd ./../build/vpdm
./bootstrap-vitasdk.sh
./install-all.sh
popd

# export env
if [ -n "$(uname -a | grep Linux)" ]; then
    echo "export VITASDK=$VITASDK" >> ~/.bashrc
    echo "export PATH=$VITASDK/bin:$PATH" >> ~/.bashrc
fi