#!/bin/bash

set -e

# x86_64 build on CentOS 7.7
  # gcc 7 installed
  # sudo yum install -y centos-release-scl
  # sudo yum install -y devtoolset-7-gcc*
  # run below command before build
  # scl enable devtoolset-7 bash

  # newer cmake is required than one included in CentOS 7
  # download from http://www.cmake.org/download
  # sudo mkdir /opt/cmake
  # sudo sh cmake-$version.$build-Linux-x86_64.sh --prefix=/opt/cmake

  if [ $(arch) = "x86_64" ]; then
    CMAKE="/opt/cmake/bin/cmake"
  fi

# i686 build on Ubuntu 16.04 LTS

# armv7l build on raspbian stretch

# Qt 6 path and flags set in env e.g.:
# export PATH="/opt/Qt/6.11.1/gcc_64/bin/:$PATH"
# export CPPFLAGS="-I/opt/Qt/6.11.1/gcc_64/include/"
# export LDFLAGS="-L/opt/Qt/6.11.1/gcc_64/lib/"
# export LD_LIBRARY_PATH="/opt/Qt/6.11.1/gcc_64/lib/:$LD_LIBRARY_PATH"
#
# NOTE: linuxdeploy-plugin-qt must be a recent build that supports Qt 6.
# Qt 6 requires a reasonably modern base distribution (glibc/GL); building the
# AppImage on an older LTS than the Qt 6 minimum will not work.

# for x86_64 platform
# Qt 6 links against OpenSSL 3.x; build it from source when the host distro
# ships an older OpenSSL, e.g.:
# ./config shared --prefix=/opt/openssl-3/ && make --jobs=`nproc --all` && sudo make install
# and add to build env
# export LD_LIBRARY_PATH="/opt/openssl-3/lib/:$LD_LIBRARY_PATH"

if [ "$1" = "SIGN" ]; then
  export SIGN="1"
fi

# check gcc version on Centos
if [ $(arch) = "x86_64" ]; then
  currentver="$(gcc -dumpversion)"
  if [ "${currentver:0:1}" -lt "7"  ]; then
    echo "gcc version 7 or newer required"
    echo "on Cetos 7 run"
    echo "scl enable devtoolset-7 bash"
    exit
 fi
fi

# building AppImage in temporary directory to keep system clean
# use RAM disk if possible (as in: not building on CI system like Travis, and RAM disk is available)
if [ "$CI" == "" ] && [ -d /dev/shm ]; then
  TEMP_BASE=/dev/shm
else
  TEMP_BASE=/tmp
fi

# we run it from our project scripts folder
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"/..
VERSION=$(cat "$ROOT"/VERSION)-$(git rev-parse --short HEAD)
# linuxdeploy uses $VERSION env variable for AppImage name
export VERSION=$VERSION
BUILD="$ROOT"/build
TARGET=rclone-browser-$VERSION.AppImage

# clean AppImage temporary folder
if [ -d "$TEMP_BASE/$TARGET" ]; then
  rm -rf "$TEMP_BASE/$TARGET"
fi
mkdir "$TEMP_BASE/$TARGET"

# clean build folder
if [ -d "$BUILD" ]; then
  rm -rf "$BUILD"
fi
mkdir "$BUILD"

# create release folder if does not exist
mkdir -p "$ROOT"/release

# clean current version previous build
if [ $(arch) = "armv7l" ] && [ -f "$ROOT"/release/rclone-browser-"$VERSION"-armhf.AppImage ]; then
  rm "$ROOT"/release/rclone-browser-"$VERSION"-raspberrypi-armhf.AppImage
fi

if [ $(arch) = "i686" ] && [ -f "$ROOT"/release/rclone-browser-"$VERSION"-i386.AppImage ]; then
  rm "$ROOT"/release/rclone-browser-"$VERSION"-linux-i386.AppImage
fi

if [ $(arch) = "x86_64" ] && [ -f "$ROOT"/release/rclone-browser-"$VERSION"-x86_64.AppImage ]; then
  rm "$ROOT"/release/rclone-browser-"$VERSION"-linux-x86_64.AppImage
fi

# build and install to temporary AppDir folder
cd "$BUILD"

if [ $(arch) = "armv7l" ]; then
  # more threads need swap on 1GB RAM RPi
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr
  make -j 2
fi

if [ $(arch) = "x86_64" ]; then
  "$CMAKE" .. -DCMAKE_INSTALL_PREFIX=/usr
  make --jobs=$(nproc --all)
fi

if [ $(arch) = "i686" ]; then
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr
  make --jobs=$(nproc --all)
fi

make install DESTDIR="$TEMP_BASE"/"$TARGET"/AppDir

# prepare AppImage
cd "$TEMP_BASE/$TARGET"

# metainfo file
#mkdir $TEMP_BASE/$TARGET/AppDir/usr/share/metainfo
#cp $ROOT/assets/rclone-browser.appdata.xml $TEMP_BASE/$TARGET/AppDir/usr/share/metainfo/

# copy info files to AppImage
cp "$ROOT"/README.md "$TEMP_BASE"/"$TARGET"/AppDir/Readme.md
cp "$ROOT"/CHANGELOG.md "$TEMP_BASE"/"$TARGET"/AppDir/Changelog.md
cp "$ROOT"/LICENSE "$TEMP_BASE"/"$TARGET"/AppDir/License.txt

# https://github.com/linuxdeploy/linuxdeploy
# https://github.com/linuxdeploy/linuxdeploy-plugin-qt
linuxdeploy --appdir AppDir --desktop-file=AppDir/usr/share/applications/rclone-browser.desktop --plugin qt
#linuxdeploy-plugin-qt --appdir AppDir

# bundle OpenSSL 3.x libs (Qt 6 TLS backend) for distros without OpenSSL 3
cp /opt/openssl-3/lib/libssl.so.3 ./AppDir/usr/bin/
cp /opt/openssl-3/lib/libcrypto.so.3 ./AppDir/usr/bin/

# https://github.com/linuxdeploy/linuxdeploy-plugin-appimage
linuxdeploy-plugin-appimage --appdir=AppDir

# raspberry pi build
if [ $(arch) = "armv7l" ]; then
  rename 's/armhf/raspberrypi-armhf/' Rclone_Browser*
  rename 's/Rclone_Browser/rclone-browser/' Rclone_Browser*
fi

# x86 build
if [ $(arch) = "i686" ]; then
  rename 's/i386/linux-i386/' Rclone_Browser*
  rename 's/Rclone_Browser/rclone-browser/' Rclone_Browser*
fi

# x86_64 build
if [ $(arch) = "x86_64" ]; then
  rename x86_64 linux-x86_64 Rclone_Browser*
  rename Rclone_Browser rclone-browser Rclone_Browser*
fi

cp ./*AppImage "$ROOT"/release/

# clean AppImage temporary folder
cd ..
rm -rf "$TARGET"
