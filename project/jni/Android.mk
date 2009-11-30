LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libventrilo3
LOCAL_SRC_FILES := libventrilo3.c libventrilo3_message.c ventrilo3_handshake.c jni_wrappers.c
LOCAL_LDFLAGS	:= -DANDROID -fpack-structs
LOCAL_LDLIBS	:= -llog

include $(BUILD_SHARED_LIBRARY)
