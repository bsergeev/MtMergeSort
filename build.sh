#!/bin/bash

# This Bash script works in Linux and Windows (Git Bash).

# The script accepts an optional parameter for build configuration, Debug or Release.
# If no parameter is passed, Debug gets built.
BUILD_CONFIGURATION=Release
if [ "$1" = "Release" ] || [ "$1" = "Debug" ]; then
  BUILD_CONFIGURATION="$1"
fi

# Detect the OS
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)   OS_NAME=Linux;;
    Darwin*)  OS_NAME=Mac;;
    CYGWIN*)  OS_NAME=Windows;;
    MINGW*)   OS_NAME=Windows;;
    *)        OS_NAME="UNKNOWN:${unameOut}"
esac
echo Building in ${OS_NAME}

BUILD_DIRECTORY=build.$OS_NAME
BINARY_LOCATION=$BUILD_DIRECTORY
if [ "$OS_NAME" = "Windows" ]; then
  BINARY_LOCATION="$BUILD_DIRECTORY/$BUILD_CONFIGURATION"
fi

if [ ! -d "$BUILD_DIRECTORY" ]; then
  mkdir $BUILD_DIRECTORY
fi
cd $BUILD_DIRECTORY

if [ "$OS_NAME" = "Linux" ]; then
  cmake -G "Unix Makefiles" ..
  make -j4
elif [ "$OS_NAME" = "Windows" ]; then
  cmake -G "Visual Studio 15 2017 Win64" ..
  cmake --build . --target ALL_BUILD --config $BUILD_CONFIGURATION
fi

cd ..
$BINARY_LOCATION/c
