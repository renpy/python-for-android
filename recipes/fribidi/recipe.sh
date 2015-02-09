#!/bin/bash

VERSION_fribidi=0.19.2
URL_fribidi=
DEPS_fribidi=()
MD5_fribidi=
BUILD_fribidi=$BUILD_PATH/fribidi/fribidi
RECIPE_fribidi=$RECIPES_PATH/fribidi

function prebuild_fribidi() {
  cd $BUILD_PATH/fribidi

  if [ ! -d fribidi ]; then
    try tar xvf $RECIPE_fribidi/fribidi-$VERSION_fribidi.tar.gz
    try mv fribidi-$VERSION_fribidi fribidi
  fi
}

function shouldbuild_fribidi() {
    true
}

function build_fribidi() {
  cd $BUILD_fribidi

  push_arm

  # Configure.

  ./configure -host=arm-eabi --enable-static --disable-shared --prefix="$BUILD_PATH/fribidi-install/"

  try make
  try make install

  try pushd "$BUILD_PATH/fribidi-install/lib"
  try cp -a libfribidi.a "$LIBS_PATH/"
  try popd

  pop_arm
}

function postbuild_fribidi() {
  true
}
