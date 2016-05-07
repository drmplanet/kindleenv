LOCAL_PATH := $(call my-dir)
TOP_LOCAL_PATH := $(LOCAL_PATH)

MUPDF_ROOT := ../../..

ifdef NDK_PROFILER
include android-ndk-profiler.mk
endif

include $(TOP_LOCAL_PATH)/Core.mk
include $(TOP_LOCAL_PATH)/ThirdParty.mk

include $(CLEAR_VARS)

LOCAL_MODULE := xdrm_client    
LOCAL_SRC_FILES := libxdrm_client.so  
include $(PREBUILT_SHARED_LIBRARY)
#LOCAL_SHARED_LIBRARIES := xdrm_client

include $(CLEAR_VARS)
LOCAL_MODULE := xdrm_jni    
LOCAL_SRC_FILES := libxdrm_jni.so   
include $(PREBUILT_SHARED_LIBRARY)
#LOCAL_SHARED_LIBRARIES := xdrm_jni

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	jni/andprof \
	$(MUPDF_ROOT)/include \
	$(MUPDF_ROOT)/source/fitz \
	$(MUPDF_ROOT)/source/pdf
LOCAL_CFLAGS :=
LOCAL_MODULE    := mupdf
LOCAL_SRC_FILES := mupdf.c
LOCAL_STATIC_LIBRARIES := mupdfcore mupdfthirdparty
LOCAL_SHARED_LIBRARIES := xdrm_client xdrm_jni
#LOCAL_LDFLAGS   := $(TOP_LOCAL_PATH)/libxdrm_client.so


ifdef NDK_PROFILER
LOCAL_CFLAGS += -pg -DNDK_PROFILER
LOCAL_STATIC_LIBRARIES += andprof
endif
ifdef SUPPORT_GPROOF
LOCAL_CFLAGS += -DSUPPORT_GPROOF
endif

LOCAL_LDLIBS    := -lm -llog -ljnigraphics
ifdef SSL_BUILD
LOCAL_LDLIBS	+= -L$(MUPDF_ROOT)/thirdparty/openssl/android -lcrypto -lssl
endif

include $(BUILD_SHARED_LIBRARY)
