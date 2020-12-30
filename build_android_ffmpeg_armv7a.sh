#!/bin/bash

NDK_PATH=/opt/android/android-ndk-r17c
SYSROOT=$NDK_PATH/platforms/android-21/arch-arm
TOOLCHAIN=$NDK_PATH/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
CPU=armabi-v7a
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
    --nm=${TOOLCHAIN}/bin/arm-linux-androideabi-nm \
    --cross-prefix="${TOOLCHAIN}/bin/arm-linux-androideabi-" \
    --extra-ldflags="-Wl,-rpath-link=${SYSROOT}/usr/lib -L${SYSROOT}/usr/lib -nostdlib" \
    --sysroot=${NDK_PATH}/sysroot/ \
    --extra-cflags="-DANDROID -I${NDK_PATH}/sysroot/usr/include/arm-linux-androideabi/ -march=armv7-a" \
    --extra-ldexeflags=-pie

};

build_android
