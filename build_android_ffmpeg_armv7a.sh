#!/bin/bash

NDK_PATH=/opt/android/android-ndk-r14b
SYSROOT=$NDK_PATH/platforms/android-21/arch-arm
TOOLCHAIN=$NDK_PATH/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
CPU=armeabi-v7a
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
    --arch=arm \
    --cpu=armv7-a \
    --cc="${TOOLCHAIN}/bin/arm-linux-androideabi-gcc" \
    --cross-prefix="${TOOLCHAIN}/bin/arm-linux-androideabi-" \
    --sysroot="${SYSROOT}/" \
    --extra-cflags="-march=armv7-a -mfloat-abi=softfp -mfpu=neon" \
    --extra-ldexeflags=-pie

};

build_android
