#!/bin/sh

if [ ! -f grumpy.icns ];then
    echo "Preparing the icon"
    ./mkicon.sh
fi

# Point to the Qt clang_64 directory, for example ~/Qt/5.9.1/clang_64/
if [ x"$1" != x ];then
  QTDIR="$1"
else
  QTDIR=~/Qt/5.10.1/clang_64/
fi

if [ ! -d "$QTDIR" ];then
    echo "$QTDIR not found"
    exit 1
fi

# Get Qt version and split on dots.
# TODO: Perhaps we can do this in configure instead?
QTVERSION=`${QTDIR}/bin/qtpaths --qt-version`
version_array=( ${QTVERSION//./ } )
EXTRA_FLAGS=""
if [ ${version_array[0]} -eq 5 ]; then
    EXTRA_FLAGS="--qt5"
elif [ ${version_array[0]} -eq 4 ];then
    EXTRA_FLAGS="--qt4"
else
    echo "Unsupported Qt version ${QTVERSION}, must be 4.x or 5.x"
    exit 1
fi
echo "Checking sanity of system..."
of=`pwd`
if [ -d release ];then
    echo "Release folder is already in folder, you need to delete it"
    exit 1
fi
cd .. || exit 1
./configure ${EXTRA_FLAGS} --qtpath "$QTDIR" --extension || exit 1
cd release || exit 1
make || exit 1
cd "$of"
cp -r ../release "$of/release" || exit 1
mkdir package || exit 1
mkdir package/grumpychat.app
cd package/grumpychat.app || exit 1
mkdir Contents
mkdir Contents/Frameworks
mkdir Contents/MacOS
mkdir Contents/PlugIns
mkdir Contents/Resources
mkdir Contents/SharedFrameworks
cd "$of"

echo "Copying the binaries to package"
cp info.plist package/grumpychat.app/Contents || exit 1
cp release/bin/* package/grumpychat.app/Contents/MacOS || exit 1
mkdir package/grumpychat.app/Contents/MacOS/scripts
cp -r ../doc/examples/ package/grumpychat.app/Contents/MacOS/scripts/examples/ || exit 1
cp grumpy.icns package/grumpychat.app/Contents/Resources || exit 1
cd package
$QTDIR/bin/macdeployqt grumpychat.app -dmg
