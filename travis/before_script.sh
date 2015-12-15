#! /bin/bash

g++ --version

cd /home/travis/build/grumpy-irc/grumpy/src/sqlite
unzip *.zip
mv sqlite*/* .
cd /home/travis/build/grumpy-irc/grumpy/
git submodule init
git submodule update
mkdir temp
cd temp

if [ "$QTTYPE" = "4" ]; then
        cmake ../src -DUNIT_TESTS=true
	make || exit 1
fi

if [ "$QTTYPE" = "5" ]; then
        cmake ../src -DQT5_BUILD=TRUE -DUNIT_TESTS=TRUE
	make || exit 1
fi

