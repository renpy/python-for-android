#!/bin/bash

VERSION_pyjnius= # ${VERSION_pyjnius:-master}
URL_pyjnius= # https://github.com/kivy/pyjnius/archive/$VERSION_pyjnius.zip
DEPS_pyjnius=(python sdl)
MD5_pyjnius=
BUILD_pyjnius=$BUILD_PATH/pyjnius # /$(get_directory $URL_pyjnius)
RECIPE_pyjnius=$RECIPES_PATH/pyjnius

CHECKOUT="$BUILD_pyjnius/pyjnius/src"

function prebuild_pyjnius() {
	if [ ! -e $CHECKOUT ] ; then
        try git clone https://github.com/renpy/pyjnius.git $CHECKOUT
    fi
}

function shouldbuild_pyjnius() {
    true;

	# if [ -d "$SITEPACKAGES_PATH/jnius" ]; then
		# 	DO_BUILD=0
	# fi
}

function build_pyjnius() {
	try cd $CHECKOUT

	push_arm

	export LDFLAGS="$LDFLAGS -L$LIBS_PATH"
	export LDSHARED="$LIBLINK"
    export PYJNIUS_SDL_VERSION=2

	# fake try to be able to cythonize generated files
	$HOSTPYTHON setup.py build_ext
	try find . -iname '*.pyx' -exec $CYTHON {} \;
	try $HOSTPYTHON setup.py build_ext -v
	try find build/lib.* -name "*.o" -exec $STRIP {} \;
	try $HOSTPYTHON setup.py install -O2
	try cp -a jnius/src/org $JAVACLASS_PATH

	unset LDSHARED
	pop_arm
}

function postbuild_pyjnius() {
	true
}
