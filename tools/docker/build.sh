#!/bin/bash

if [ ! -d ../../release ];then
    echo "You must build grumpy first"
    exit 1
fi

if [ -d ../../release/grumpy ]
then
    echo "../../release/grumpy exists"
    exit 1
fi
cd ../../release || exit 1
mkdir grumpy
cp ../tools/docker/Dockerfile grumpy/
cp libirc/libirc/*.so grumpy/
cp libirc/libircclient/*.so grumpy/
cp libgp/*.so grumpy/
cp libcore/*.so grumpy/
cp bin/grumpyd grumpy/
cp ../tools/docker/grumpy.ini grumpy/

cd grumpy
openssl req  -nodes -new -x509  -keyout server.key -out server.crt
cd ..


docker build grumpy -t grumpy/grumpyd
