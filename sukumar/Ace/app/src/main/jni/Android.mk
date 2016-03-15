LOCAL_PATH := $(call my-dir)
OPENCV_PATH := F:/BE_Project/OpenCV-2.4.11-android-sdk/sdk/native/jni


include $(CLEAR_VARS)
LOCAL_MODULE    := nonfree
LOCAL_SRC_FILES := libnonfree.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gnustl_shared_mine
LOCAL_SRC_FILES := libgnustl_shared.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OPENCV_INSTALL_MODULES := on
OPENCV_CAMERA_MODULES  := off
include $(OPENCV_PATH)/OpenCV.mk

LOCAL_C_INCLUDES :=				\
	$(LOCAL_PATH)				\
	$(OPENCV_PATH)/include

LOCAL_SRC_FILES :=	AceJNI.cpp

LOCAL_MODULE := AceNativeLib
LOCAL_CFLAGS := -Werror -O3 -ffast-math
LOCAL_LDLIBS := -llog -ldl
LOCAL_SHARED_LIBRARIES += nonfree
LOCAL_SHARED_LIBRARIES += gnustl_shared_mine

include $(BUILD_SHARED_LIBRARY)
