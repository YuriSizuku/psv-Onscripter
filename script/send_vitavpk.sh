#!/bin/sh 
# sh ./send_vitasdk.sh vpkpath vitaip titleid

function ftpsenddir()
{
    srcdir=$1
    dstdir=$2
    for f in $(ls $srcdir); do
        echo $srcdir/$f
        if [ -d $srcdir/$f ]; then
            ftpsenddir $srcdir/$f $dstdir/$f
        else
            curl -T $srcdir/$f $dstdir/$f
        fi
    done  
}

vpkpath=${1//\\//} # replace to linux path
vitaip=$2
titleid=$3

vpkdir=${vpkpath%/*}
vpkname=${vpkpath##*/}
echo vpkdir=$vpkdir, vpkname=$vpkname

7z x -y -o$vpkdir/${vpkname}_out $vpkpath
ftpsenddir $vpkdir/${vpkname}_out ftp://$vitaip:1337/ux0:app/$titleid