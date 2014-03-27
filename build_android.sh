#!/bin/bash

if [[ $* == *-f* ]]
then
	FORCE_INPUT=true
else
	FORCE_INPUT=false
fi

if [[ -z "$ANDROID_NATIVE_API_LEVEL" ]] || [[ "$FORCE_INPUT" = true ]]
then
	echo "Type the native api level (e.g. 19)"
	read ANDROID_NATIVE_API_LEVEL
	export ANDROID_NATIVE_API_LEVEL
else
	echo "Using android native api level $ANDROID_NATIVE_API_LEVEL"
fi

if [[ -z "$ANDROID_NDK" ]] || [[ "$FORCE_INPUT" = true ]]
then
	echo "Type the android ndk path"
	read -er ANDROID_NDK
	export ANDROID_NDK
else
	echo "Using android ndk path $ANDROID_NDK"
fi

if [[ -z "$ABI" ]] || [[ "$FORCE_INPUT" = true ]]
then
	echo "Type the android abi (e.g. armeabi or armeabi-v7a)"
	read ABI
	export ABI
else
	echo "Using abi $ABI"
fi


cmake -DANDROID_ABI=$ABI -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain/android.toolchain.cmake -DANDROID_NDK=$ANDROID_NDK -DANDROID_NATIVE_API_LEVEL=$ANDROID_NATIVE_API_LEVEL .. 

