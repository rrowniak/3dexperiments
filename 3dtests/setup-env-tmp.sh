#!/bin/bash

# do some tests, download & build all necessary stuff

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
LIB_DIR=$CWD/lib


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