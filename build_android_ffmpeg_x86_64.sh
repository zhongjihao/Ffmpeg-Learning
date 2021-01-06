#!/bin/bash

NDK_PATH=/opt/android/android-ndk-r14b
SYSROOT=$NDK_PATH/platforms/android-21/arch-x86_64
TOOLCHAIN=$NDK_PATH/toolchains/x86_64-4.9/prebuilt/linux-x86_64
CPU=x86_64
mkdir -p $(pwd)/android_build_out/$CPU
PREFIX=$(pwd)/android_build_out/$CPU

function build_android {

    ./configure \
    --prefix=$PREFIX \
    --target-os=android \
    --enable-shared \
    --enable-cross-compile \
    --enable-small \
    --disable-static \
    --disable-programs \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-doc \
    --disable-symver \
    --disable-asm \
    --arch=x86_64 \
    --cpu=x86-64 \
    --cc="${TOOLCHAIN}/bin/x86_64-linux-android-gcc" \
    --cross-prefix="${TOOLCHAIN}/bin/x86_64-linux-android-" \
    --sysroot="${SYSROOT}/" \
    --extra-cflags="-march=x86-64 -mtune=intel -m64 -DANDROID -Wfatal-errors -Wno-deprecated" \
    --extra-ldexeflags=-pie

};

build_android
