#!/bin/bash

NDK_LOCATION=$ANDROID_NDK
ARM_ABI=armeabi-v7a
if [[ $1 ]] 
then 
    NDK_LOCATION=$1 
fi

if [[ $2 ]] 
then 
    ABI=$2 
fi

cmake -DANDROID_ABI=$ABI -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain/android.toolchain.cmake -DANDROID_NDK=$NDK_LOCATION -DANDROID_NATIVE_API_LEVEL=android-17 .. 

