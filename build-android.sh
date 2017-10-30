#!/bin/bash
ANDROID_NDK_HOME=${ANDROID_NDK_HOME:=}
NUM_CORES=$(sysctl -n hw.ncpu)
CORE_DIRECTIVE="-j $NUM_CORES"
if [ -z "$ANDROID_NDK_HOME" ]; then
	if [ -z "$ANDROID_SDK_HOME" ]; then
	    echo "Need to set ANDROID_SDK_HOME or ANDROID_ANDROID_NDK_HOME"
	    exit 1
	fi
	ANDROID_NDK_HOME="$ANDROID_SDK_HOME/ndk-bundle"
fi  
	echo $ANDROID_NDK_HOME

if [ -z "$ANDROID_NDK_HOME" ]; then
	echo "Need to set ANDROID_NDK_HOME"
	exit 1
fi
if [ ! -f "$ANDROID_NDK_HOME/ndk-build" ]; then
    echo "Does not look like a valid NDK path"
fi

$ANDROID_NDK_HOME/ndk-build NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=./  APP_BUILD_SCRIPT=./Android.mk clean $CORE_DIRECTIVE

$ANDROID_NDK_HOME/ndk-build NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=./  APP_BUILD_SCRIPT=./Android.mk $CORE_DIRECTIVE

