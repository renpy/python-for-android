#!/bin/bash

VERSION_android=
URL_android=
DEPS_android=(python)
MD5_android=
BUILD_android=$BUILD_PATH/android/android
RECIPE_android=$RECIPES_PATH/android

function prebuild_android() {
	cd $BUILD_PATH/android

	rm -rf android
	if [ ! -d android ]; then
		try cp -a $RECIPE_android/src $BUILD_android
	fi
}

function shouldbuild_android() {
    true
}

function build_android() {
	cd $BUILD_android

	push_arm

	export LDFLAGS="$LDFLAGS -L$LIBS_PATH"
	export LDSHARED="$LIBLINK"

	# cythonize
	try find . -iname '*.pyx' -exec $CYTHON {} \;
	try $HOSTPYTHON setup.py build_ext -v
	try $HOSTPYTHON setup.py install -O2

	unset LDSHARED

	pop_arm
}

function postbuild_android() {
	true
}
