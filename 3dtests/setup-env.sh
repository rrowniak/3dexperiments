#!/bin/bash

# do some tests, download & build all necessary stuff

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
LIB_DIR=$CWD/lib

# ubuntu specific
#sudo apt install libncurses-dev

mkdir -p $LIB_DIR
cd $LIB_DIR

# download (if not exists) and install SDL2
SDL2_NAME=SDL2
SDL2_VERSION=2.0.12
SDL2_FILE=$SDL2_NAME-$SDL2_VERSION.tar.gz
SDL2_FILE_PATH=$LIB_DIR/$SDL2_FILE
SDL_URL=https://www.libsdl.org/release/$SDL2_FILE
#https://www.libsdl.org/release/SDL2-2.0.12.tar.gz

if [ ! -f $SDL2_FILE_PATH ]; then
    echo "SDL library not found. Downloading..."
    wget $SDL_URL
    if [ $? -ne 0 ]; then
        echo "Unable to download SDL2 from $SDL_URL. Aborting."
        rm $SDL2_FILE_PATH
        exit 1
    fi
fi

tar xfz $SDL2_FILE_PATH
cd $SDL2_NAME-$SDL2_VERSION

./configure --prefix $LIB_DIR 
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make -j4
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make install
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

#install SDL2 GFX
cd $LIB_DIR

SDL2_GFX_NAME=SDL2_gfx
SDL2_GFX_VERSION=1.0.4
SDL2_GFX_FILE=$SDL2_GFX_NAME-$SDL2_GFX_VERSION.tar.gz
SDL2_GFX_FILE_PATH=$LIB_DIR/$SDL2_GFX_FILE
SDL2_GFX_URL=http://www.ferzkopp.net/Software/SDL2_gfx/$SDL2_GFX_FILE
# http://www.ferzkopp.net/Software/SDL2_gfx/SDL2_gfx-1.0.4.tar.gz

if [ ! -f $SDL2_GFX_FILE_PATH ]; then
    echo "SDL GFX library not found. Downloading..."
    wget $SDL2_GFX_URL
    if [ $? -ne 0 ]; then
        echo "Unable to download SDL2 GFX from $SDL2_GFX_URL. Aborting."
        rm $SDL2_GFX_FILE_PATH
        exit 1
    fi
fi

tar xfz $SDL2_GFX_FILE_PATH
cd $SDL2_GFX_NAME-$SDL2_GFX_VERSION

./configure --prefix $LIB_DIR 
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make -j4
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make install
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

#install SDL2 IMAGE
cd $LIB_DIR

SDL2_IMAGE_NAME=SDL2_image
SDL2_IMAGE_VERSION=2.0.5
SDL2_IMAGE_FILE=$SDL2_IMAGE_NAME-${SDL2_IMAGE_VERSION}.tar.gz
SDL2_IMAGE_FILE_PATH=$LIB_DIR/$SDL2_IMAGE_FILE
SDL2_IMAGE_URL=https://www.libsdl.org/projects/SDL_image/release/$SDL2_IMAGE_FILE
# https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz

if [ ! -f $SDL2_IMAGE_FILE_PATH ]; then
    echo "SDL IMAGE library not found. Downloading..."
    wget $SDL2_IMAGE_URL
    if [ $? -ne 0 ]; then
        echo "Unable to download SDL2 IMAGE from $SDL2_IMAGE_URL. Aborting."
        rm $SDL2_IMAGE_FILE_PATH
        exit 1
    fi
fi

tar xfz $SDL2_IMAGE_FILE_PATH
cd $SDL2_IMAGE_NAME-$SDL2_IMAGE_VERSION

./configure --prefix $LIB_DIR 
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make -j4
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make install
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

#install SDL2 TTF
#prerequtisite
# sudo apt install libfreetype6-dev
cd $LIB_DIR

SDL2_TTF_NAME=SDL2_ttf
SDL2_TTF_VERSION=2.0.15
SDL2_TTF_FILE=$SDL2_TTF_NAME-${SDL2_TTF_VERSION}.tar.gz
SDL2_TTF_FILE_PATH=$LIB_DIR/$SDL2_TTF_FILE
SDL2_TTF_URL=https://www.libsdl.org/projects/SDL_ttf/release/$SDL2_TTF_FILE
# https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.15.tar.gz

if [ ! -f $SDL2_TTF_FILE_PATH ]; then
    echo "SDL TTF library not found. Downloading..."
    wget $SDL2_TTF_URL
    if [ $? -ne 0 ]; then
        echo "Unable to download SDL2 TTF from $SDL2_TTF_URL. Aborting."
        rm $SDL2_TTF_FILE_PATH
        exit 1
    fi
fi

tar xfz $SDL2_TTF_FILE_PATH
cd $SDL2_TTF_NAME-$SDL2_TTF_VERSION

./configure --prefix $LIB_DIR 
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make -j4
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make install
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

#install SDL2 SDL_FontCache
cd $LIB_DIR
FONT_CACHE_DIR=SDL_FontCache

if [ ! -d $FONT_CACHE_DIR ]; then
    git clone https://github.com/grimfang4/SDL_FontCache.git
    if [ $? -ne 0 ]; then
        echo "Aborting"
        exit 1
    fi
fi

cd $FONT_CACHE_DIR
mkdir -p build
cd build

cmake .. -DCMAKE_LIBRARY_PATH=$LIB_DIR/lib -DCMAKE_INCLUDE_PATH=$LIB_DIR/include/SDL2/

if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

make FontCache
if [ $? -ne 0 ]; then
    echo "Aborting"
    exit 1
fi

cp libFontCache.a $LIB_DIR/lib
cp ../SDL_FontCache.h $LIB_DIR/include

echo "-------------------------------------------------------"
echo "              DONE - SUCCESS"
echo "-------------------------------------------------------"