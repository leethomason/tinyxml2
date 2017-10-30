LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := tinyxml2

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_SRC_FILES := ./tinyxml2.cpp

include $(BUILD_STATIC_LIBRARY)

			
