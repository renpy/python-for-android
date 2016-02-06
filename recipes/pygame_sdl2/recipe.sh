#!/bin/bash

VERSION_pygame_sdl2=1.0
URL_pygame_sdl2=
DEPS_pygame_sdl2=(sdl python)
MD5_pygame_sdl2=
BUILD_pygame_sdl2=$BUILD_PATH/pygame_sdl2/
RECIPE_pygame_sdl2=$RECIPES_PATH/pygame_sdl2

function strip_pygame_sdl2() {
    # rm -f "$1"/*.py
    # rm -f "$1"/*.pyc
    # rm -f "$1"/*.pyx
    # rm -f "$1"/*.pxd
    # rm -f "$1"/*~
    # rm -f "$1"/*.rpy
    # rm -f "$1"/*.rpym
    # rm -f "$1"/*.so.o
    # rm -f "$1"/*.so.libs
    echo -n
}


function prebuild_pygame_sdl2() {
    true
}

function build_pygame_sdl2() {

	try cd "$PYGAME_SDL2_ROOT"

	push_arm

    CFLAGS="$CFLAGS -DANDROID -I$JNI_PATH/sdl2/include -I$JNI_PATH/sdl2_image -I$JNI_PATH/sdl2_gfx -I$JNI_PATH/sdl2_ttf -I$JNI_PATH/sdl2_mixer -I$JNI_PATH/jpeg -I$JNI_PATH/png -I$JNI_PATH/freetype/include"

	export CFLAGS="$CFLAGS"
	export LDFLAGS="$LDFLAGS -L$LIBS_PATH -L$SRC_PATH/obj/local/$ARCH/ -lm -lz"
	export LDSHARED="$LIBLINK"

	# export PYGAME_SDL2_EXCLUDE="pygame_sdl2.mixer pygame_sdl2.mixer_music"
    export PYGAME_SDL2_INSTALL_HEADERS=1

	rm -R build/lib.android build/tmp.android
    try $BUILD_PATH/python-install/bin/python.host setup.py build_ext -b build/lib.android -t build/tmp.android install --prefix "$BUILD_PATH/python-install/"

    unset LDSHARED

	pop_arm

    #site-packages
    SP="$BUILD_PATH/python-install/lib/python2.7/site-packages"
    B="$PYGAME_SDL2_ROOT/build/lib.android"

    try cp -a "$B"/* "$SP"

    strip_pygame_sdl2 "$SP/pygame_sdl2"
}

function postbuild_pygame_sdl2() {
	true
}
