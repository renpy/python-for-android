#!/bin/bash

VERSION_ffmpeg=
URL_ffmpeg=
DEPS_ffmpeg=()
MD5_ffmpeg=
BUILD_ffmpeg=$BUILD_PATH/ffmpeg/ffmpeg
RECIPE_ffmpeg=$RECIPES_PATH/ffmpeg

function prebuild_ffmpeg() {
  cd $BUILD_PATH/ffmpeg

  # rm -rf ffmpeg
  if [ ! -d ffmpeg ]; then
    try tar xf $RECIPE_ffmpeg/ffmpeg-3.0.tar.bz2
    try mv ffmpeg-3.0 ffmpeg
  fi
}

function shouldbuild_ffmpeg() {
    true
}

function build_ffmpeg() {
  cd $BUILD_ffmpeg

  push_arm

  echo $CC
  echo $LD
  echo $CFLAGS
  echo $LDFLAGS

if [ ! -e config.mak ]; then

  try ./configure --prefix="$BUILD_PATH/ffmpeg-install" \
  --cc="$CC" \
  --ld="$CC" \
  --target-os=android \
  --arch=arm \
  --cpu=armv6 \
  --enable-avresample \
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
  --disable-decoders \
  --enable-decoder=flac \
  --enable-decoder=mp2 \
  --enable-decoder=mp3 \
  --enable-decoder=mp3on4 \
  --enable-decoder=pcm_dvd \
  --enable-decoder=pcm_s16be \
  --enable-decoder=pcm_s16le \
  --enable-decoder=pcm_s8 \
  --enable-decoder=pcm_u16be \
  --enable-decoder=pcm_u16le \
  --enable-decoder=pcm_u8 \
  --enable-decoder=vorbis \
  --enable-decoder=opus \
  --disable-parsers \
  --enable-parser=mpegaudio \
  --disable-protocols \
  --enable-protocol=file \
  --disable-devices \
  --disable-vdpau \
  --disable-filters \
  --disable-programs \
  --disable-bsfs # 2>&1 >/dev/null

fi

  try make
  try make install

  set -x

  try pushd "$BUILD_PATH/ffmpeg-install/lib"
  try ranlib libswresample.a
  try cp -a libavcodec.a libavformat.a libavresample.a libavutil.a libswscale.a libswresample.a "$LIBS_PATH/"
  try popd

  pop_arm
}

function postbuild_ffmpeg() {
  true
}
