#!/bin/bash

NDK_PATH=/opt/android/android-ndk-r14b
SYSROOT=$NDK_PATH/platforms/android-21/arch-arm64
TOOLCHAIN=$NDK_PATH/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64
CPU=arm64-v8a
mkdir -p $(pwd)/android_build_out/$CPU
PREFIX=$(pwd)/android_build_out/$CPU

build_android(){
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
    --arch=aarch64 \
    --cpu=armv8-a \
    --cc="${TOOLCHAIN}/bin/aarch64-linux-android-gcc" \
    --cross-prefix="${TOOLCHAIN}/bin/aarch64-linux-android-" \
    --sysroot="${SYSROOT}/" \
    --extra-cflags="-march=armv8-a -DANDROID -Wfatal-errors -Wno-deprecated" \
    --extra-ldexeflags=-pie
}


build_android
