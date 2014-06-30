#!/bin/bash

VERSION_renpy=6.15.0
URL_renpy=
DEPS_renpy=(pygame)
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

	# try cd "$RENPYROOT"
	# ./run.sh the_question compile
	try cd "$RENPYROOT/module"

	push_arm

    CFLAGS="$CFLAGS -DANDROID"
	CFLAGS="$CFLAGS -I$JNI_PATH/sdl/include"
	CFLAGS="$CFLAGS -I$JNI_PATH/png -I$JNI_PATH/freetype/include"
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

    # try cp -a "$RENPYROOT/renpy" "$SP"
    try cp -a "$B"/* "$SP"

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
