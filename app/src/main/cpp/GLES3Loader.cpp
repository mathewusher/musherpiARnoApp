#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include "GLES3Loader.h"
#include <dlfcn.h>
#include <android/log.h>

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#define LOG_TAG "GLES3Loader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Function pointer definitions
PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR = nullptr;
PFNGLDEBUGMESSAGECONTROLKHRPROC  glDebugMessageControlKHR  = nullptr;
PFNGLDEBUGMESSAGEINSERTKHRPROC   glDebugMessageInsertKHR   = nullptr;
PFNGLGETDEBUGMESSAGELOGKHRPROC   glGetDebugMessageLogKHR   = nullptr;
PFNGLGETUNIFORMBLOCKINDEXPROC    glGetUniformBlockIndexPtr = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC     glUniformBlockBindingPtr  = nullptr;
PFNGLGENVERTEXARRAYSPROC         glGenVertexArraysPtr    = nullptr;
PFNGLBINDVERTEXARRAYPROC         glBindVertexArrayPtr    = nullptr;
PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArraysPtr = nullptr;
PFNGLTEXSTORAGE3DPROC           glTexStorage3DPtr           = nullptr;
PFNGLINVALIDATEFRAMEBUFFERPROC  glInvalidateFramebufferPtr  = nullptr;
PFNGLMAPBUFFERRANGEPROC         glMapBufferRangePtr         = nullptr;
PFNGLUNMAPBUFFERPROC            glUnmapBufferPtr            = nullptr;
PFNGLBINDBUFFERBASEPROC         glBindBufferBasePtr         = nullptr;

// Debug callback function
void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam) {
    LOGI("GL DEBUG MESSAGE:\nSource: 0x%x\nType: 0x%x\nID: %u\nSeverity: 0x%x\nMessage: %s",
         source, type, id, severity, message);
}

bool LoadGLES3Extensions() {
    void* libGLES = dlopen("libGLESv2.so", RTLD_LAZY);
    if (!libGLES) {
        LOGE("Failed to open libGLESv2.so");
        return false;
    }

    glDebugMessageCallbackKHR = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKKHRPROC>(
        dlsym(libGLES, "glDebugMessageCallbackKHR"));
    glDebugMessageControlKHR = reinterpret_cast<PFNGLDEBUGMESSAGECONTROLKHRPROC>(
        dlsym(libGLES, "glDebugMessageControlKHR"));
    glDebugMessageInsertKHR = reinterpret_cast<PFNGLDEBUGMESSAGEINSERTKHRPROC>(
        dlsym(libGLES, "glDebugMessageInsertKHR"));
    glGetDebugMessageLogKHR = reinterpret_cast<PFNGLGETDEBUGMESSAGELOGKHRPROC>(
        dlsym(libGLES, "glGetDebugMessageLogKHR"));

    // Uniform blocks
    glGetUniformBlockIndexPtr = reinterpret_cast<PFNGLGETUNIFORMBLOCKINDEXPROC>(
        dlsym(libGLES, "glGetUniformBlockIndex"));
    glUniformBlockBindingPtr = reinterpret_cast<PFNGLUNIFORMBLOCKBINDINGPROC>(
        dlsym(libGLES, "glUniformBlockBinding"));
    glGenVertexArraysPtr = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(
        dlsym(libGLES, "glGenVertexArrays"));
    glBindVertexArrayPtr = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(
        dlsym(libGLES, "glBindVertexArray"));
    glDeleteVertexArraysPtr = reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(
        dlsym(libGLES, "glDeleteVertexArrays"));

    // Inside LoadGLES3Extensions() after other dlsym() calls:
    glTexStorage3DPtr = reinterpret_cast<PFNGLTEXSTORAGE3DPROC>(
        dlsym(libGLES, "glTexStorage3D"));
    glInvalidateFramebufferPtr = reinterpret_cast<PFNGLINVALIDATEFRAMEBUFFERPROC>(
        dlsym(libGLES, "glInvalidateFramebuffer"));
    glMapBufferRangePtr = reinterpret_cast<PFNGLMAPBUFFERRANGEPROC>(
        dlsym(libGLES, "glMapBufferRange"));
    glUnmapBufferPtr = reinterpret_cast<PFNGLUNMAPBUFFERPROC>(
        dlsym(libGLES, "glUnmapBuffer"));
    glBindBufferBasePtr = reinterpret_cast<PFNGLBINDBUFFERBASEPROC>(
        dlsym(libGLES, "glBindBufferBase"));

    dlclose(libGLES);

    if (glDebugMessageCallbackKHR && glDebugMessageControlKHR &&
        glDebugMessageInsertKHR && glGetDebugMessageLogKHR) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallbackKHR(DebugCallback, nullptr);
        glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        LOGI("KHR_debug extension loaded and enabled.");
    } else {
        LOGE("Failed to load one or more KHR_debug extension functions.");
    }

    if (!glGetUniformBlockIndexPtr || !glUniformBlockBindingPtr) {
        LOGE("Failed to load uniform block functions.");
    }

    return glDebugMessageCallbackKHR && glDebugMessageControlKHR &&
           glDebugMessageInsertKHR && glGetDebugMessageLogKHR &&
           glGetUniformBlockIndexPtr && glUniformBlockBindingPtr &&
           glGenVertexArraysPtr && glBindVertexArrayPtr && glDeleteVertexArraysPtr &&
           glTexStorage3DPtr && glInvalidateFramebufferPtr &&
           glMapBufferRangePtr && glUnmapBufferPtr && glBindBufferBasePtr;

}

