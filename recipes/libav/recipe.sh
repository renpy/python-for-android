#!/bin/bash

VERSION_libav=
URL_libav=
DEPS_libav=()
MD5_libav=
BUILD_libav=$BUILD_PATH/libav/libav
RECIPE_libav=$RECIPES_PATH/libav

function prebuild_libav() {
  cd $BUILD_PATH/libav

  # rm -rf libav
  if [ ! -d libav ]; then
    try tar xvf $RECIPE_libav/libav-11.1.tar.gz
    try mv libav-11.1 libav
  fi
}

function shouldbuild_libav() {
    true
}

function build_libav() {
  cd $BUILD_libav

  push_arm

  echo $CC
  echo $LD
  echo $CFLAGS
  echo $LDFLAGS

if [ ! -e Makefile ]; then

  try ./configure --prefix="$BUILD_PATH/libav-install" \
  --cc="$CC" \
  --ld="$CC" \
  --target-os=android \
  --arch=arm \
  --cpu=armv6 \
  --extra-cflags="$CFLAGS" \
  --extra-ldflags="$LDFLAGS" \
  --extra-ldexeflags=-pie \
  --enable-cross-compile \
  --disable-shared \
  --enable-static \
  --enable-memalign-hack \
  --enable-runtime-cpudetect \
  --disable-encoders \
  --disable-muxers \
  --disable-bzlib \
  --disable-demuxers \
  --enable-demuxer=au \
  --enable-demuxer=avi \
  --enable-demuxer=flac \
  --enable-demuxer=m4v \
  --enable-demuxer=matroska \
  --enable-demuxer=mov \
  --enable-demuxer=mp3 \
  --enable-demuxer=mpegps \
  --enable-demuxer=mpegts \
  --enable-demuxer=mpegtsraw \
  --enable-demuxer=mpegvideo \
  --enable-demuxer=ogg \
  --enable-demuxer=wav \
  --enable-demuxer=webm \
  --disable-decoders \
  --enable-decoder=flac \
  --enable-decoder=mp2 \
  --enable-decoder=mp3 \
  --enable-decoder=mp3on4 \
  --enable-decoder=mpeg1video \
  --enable-decoder=mpeg2video \
  --enable-decoder=mpegvideo \
  --enable-decoder=msmpeg4v1 \
  --enable-decoder=msmpeg4v2 \
  --enable-decoder=msmpeg4v3 \
  --enable-decoder=mpeg4 \
  --enable-decoder=pcm_dvd \
  --enable-decoder=pcm_s16be \
  --enable-decoder=pcm_s16le \
  --enable-decoder=pcm_s8 \
  --enable-decoder=pcm_u16be \
  --enable-decoder=pcm_u16le \
  --enable-decoder=pcm_u8 \
  --enable-decoder=theora \
  --enable-decoder=vorbis \
  --enable-decoder=vp3 \
  --enable-decoder=vp8 \
  --disable-parsers \
  --enable-parser=mpegaudio \
  --enable-parser=mpegvideo \
  --enable-parser=mpeg4video \
  --enable-parser=vp3 \
  --enable-parser=vp8 \
  --disable-protocols \
  --enable-protocol=file \
  --disable-devices \
  --disable-vdpau \
  --disable-filters \
  --disable-bsfs # 2>&1 >/dev/null

fi

  try make
  try make install

  set -x

  try pushd "$BUILD_PATH/libav-install/lib"
  try cp -a libavcodec.a libavformat.a libavresample.a libavutil.a libswscale.a "$LIBS_PATH/"
  try popd

  pop_arm
}

function postbuild_android() {
  true
}
