#! /bin/bash


echo Testing QTTYPE $QTTYPE
cd /home/travis/build/grumpy-irc/grumpy/test/bin || exit 1
./core_ut || exit 1
./grumpyd_ut || exit 1
