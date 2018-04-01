#! /bin/bash

#if [ "$QTTYPE" = "4" ]; then
#	sudo apt-get install -y libqt4-webkit libqtwebkit-dev qt4-qmake qt4-dev-tools cmake
#fi

if [ "$QTTYPE" = "5" ]; then
	sudo apt-get install -y qtbase5-dev qtdeclarative5-dev qt5-default qttools5-dev-tools qtscript5-dev
fi

#sudo apt-get install g++-4.7
#sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 20 --slave /usr/bin/g++ g++ /usr/bin/g++-4.6
#sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.7 80 --slave /usr/bin/g++ g++ /usr/bin/g++-4.7
#sudo update-alternatives --config gcc
