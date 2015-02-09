#!/bin/bash

VERSION_renpy=6.19.0
URL_renpy=
DEPS_renpy=(pygame_sdl2 sdl python fribidi)
MD5_renpy=
BUILD_renpy=$BUILD_PATH/renpy/
RECIPE_renpy=$RECIPES_PATH/renpy

function strip_renpy() {
    rm -f "$1"/*.py
    rm -f "$1"/*.pyc
    rm -f "$1"/*.pyx
    rm -f "$1"/*.pxd
    rm -f "$1"/*~
    rm -f "$1"/*.rpy
    rm -f "$1"/*.rpym
    rm -f "$1"/*.so.o
    rm -f "$1"/*.so.libs

}

function prebuild_renpy() {
    true
}

function build_renpy() {

    try cd "$RENPYROOT/module"

    push_arm

    CFLAGS="$CFLAGS -DANDROID -I$JNI_PATH/sdl2/include -I$JNI_PATH/sdl2_image -I$JNI_PATH/png -I$JNI_PATH/freetype/include -I$BUILD_PATH/libav-install/include -I$BUILD_PATH/fribidi-install/include"
    export CFLAGS="$CFLAGS"
    export LDFLAGS="$LDFLAGS -L$LIBS_PATH -L$SRC_PATH/obj/local/$ARCH/ -lm -lz"
    export LDSHARED="$LIBLINK"
    try $BUILD_PATH/python-install/bin/python.host setup.py clean -b build/lib.android -t build/tmp.android
    try $BUILD_PATH/python-install/bin/python.host setup.py build_ext -b build/lib.android -t build/tmp.android

    unset LDSHARED

    pop_arm

    #site-packages
    SP="$BUILD_PATH/python-install/lib/python2.7/site-packages"
    B="$RENPYROOT/module/build/lib.android"

    try python -O -m compileall "$RENPYROOT/module/pysdlsound"

    # try cp -a "$RENPYROOT/renpy" "$SP"
    try cp -a "$B"/* "$SP"
    try cp "$RENPYROOT/module/pysdlsound/__init__.pyo" "$SP/pysdlsound"

    echo $SP/pysdlsound
    ls $SP/pysdlsound

  # try rm -Rf "$SP/renpy/common"

    strip_renpy "$SP/renpy"
    strip_renpy "$SP/renpy"
    strip_renpy "$SP/renpy/display"
    strip_renpy "$SP/renpy/audio"
    strip_renpy "$SP/renpy/tools"
    strip_renpy "$SP/renpy/gl"
    strip_renpy "$SP/renpy/text"

}

function postbuild_renpy() {
	true
}
