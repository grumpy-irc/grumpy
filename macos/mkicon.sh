#!/bin/sh
mkdir grumpy.iconset
cp ../src/GrumpyIRC/lib/img/grumpy_logo.png .
sips -z 16 16     grumpy_logo.png --out grumpy.iconset/icon_16x16.png
sips -z 32 32     grumpy_logo.png --out grumpy.iconset/icon_16x16@2x.png
sips -z 32 32     grumpy_logo.png --out grumpy.iconset/icon_32x32.png
sips -z 64 64     grumpy_logo.png --out grumpy.iconset/icon_32x32@2x.png
sips -z 128 128   grumpy_logo.png --out grumpy.iconset/icon_128x128.png
sips -z 256 256   grumpy_logo.png --out grumpy.iconset/icon_128x128@2x.png
sips -z 256 256   grumpy_logo.png --out grumpy.iconset/icon_256x256.png
sips -z 512 512   grumpy_logo.png --out grumpy.iconset/icon_256x256@2x.png
sips -z 512 512   grumpy_logo.png --out grumpy.iconset/icon_512x512.png
cp grumpy_logo.png grumpy.iconset/icon_512x512@2x.png
iconutil -c icns grumpy.iconset
rm -R grumpy.iconset
