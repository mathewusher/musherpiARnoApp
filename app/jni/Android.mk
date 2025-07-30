LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

include ../../../../cflags.mk
LOCAL_CPPFLAGS += -std=c++17

LOCAL_MODULE := xrpassthrough


LOCAL_C_INCLUDES := \
                $(LOCAL_PATH)/../../../../../SampleCommon/Src \
                $(LOCAL_PATH)/../../../../../SampleXrFramework/Src \
                $(LOCAL_PATH)/../../../../../1stParty/OVR/Include \
                $(LOCAL_PATH)/../../../../../1stParty/utilities/include \
                $(LOCAL_PATH)/../../../../../3rdParty/stb/src \
                $(LOCAL_PATH)/../../../../../3rdParty/khronos/openxr/OpenXR-SDK/include \
		$(LOCAL_PATH)/External/curl/include \
		$(LOCAL_PATH)/../../../../../OpenXR/Include \


LOCAL_SRC_FILES := ../../../Src/XrPassthrough.cpp \
    ../../../Src/XrPassthroughGl.cpp \
    ../../../Src/XrPassthroughInput.cpp \
    ../../../Src/Engine.cpp \
    ../../../Src/Piarno.cpp \
    ../../../Src/Object.cpp \
    ../../../Src/midi/Binasc.cpp \
    ../../../Src/midi/MidiEvent.cpp \
    ../../../Src/midi/MidiEventList.cpp \
    ../../../Src/midi/MidiFile.cpp \
    ../../../Src/midi/MidiMessage.cpp \
    ../../../Src/GLES3Loader.cpp \

LOCAL_LDLIBS 			:= -llog -landroid -lGLESv2 -lEGL -lz -lm
LOCAL_STATIC_LIBRARIES 	:= samplexrframework curl
LOCAL_SHARED_LIBRARIES := openxr_loader

include $(BUILD_SHARED_LIBRARY)

$(call import-module,SampleXrFramework/Projects/Android/jni)
$(call import-module,OpenXR/Projects/AndroidPrebuilt/jni)

# --- Link against libcurl ---
include $(CLEAR_VARS)
LOCAL_MODULE := curl
LOCAL_SRC_FILES := External/curl/libcurl.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/External/curl/include
include $(PREBUILT_STATIC_LIBRARY)
