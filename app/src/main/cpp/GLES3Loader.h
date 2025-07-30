#ifndef GLES3LOADER_H
#define GLES3LOADER_H

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifndef GL_DEBUG_OUTPUT
#define GL_DEBUG_OUTPUT 0x92E0
#endif

#ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#endif

// Define GLDEBUGPROC if not available
#ifndef GLDEBUGPROC
typedef void (GL_APIENTRYP GLDEBUGPROC)(GLenum source, GLenum type, GLuint id,
                                        GLenum severity, GLsizei length,
                                        const GLchar* message, const void* userParam);
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Ensure KHR_debug types are defined even if GL_KHR_debug macro is missing
#ifndef GL_KHR_debug
#define GL_KHR_debug 1
typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECALLBACKKHRPROC)(GLDEBUGPROC callback, const void *userParam);
typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECONTROLKHRPROC)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGEINSERTKHRPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
typedef GLuint (GL_APIENTRYP PFNGLGETDEBUGMESSAGELOGKHRPROC)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
// Uniform block function pointer typedefs
typedef GLuint (GL_APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC)(GLuint program, const GLchar *uniformBlockName);
typedef void (GL_APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void (GL_APIENTRYP PFNGLTEXSTORAGE3DPROC)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
typedef void (GL_APIENTRYP PFNGLINVALIDATEFRAMEBUFFERPROC)(GLenum, GLsizei, const GLenum*);
typedef void* (GL_APIENTRYP PFNGLMAPBUFFERRANGEPROC)(GLenum, GLintptr, GLsizeiptr, GLbitfield);
typedef GLboolean (GL_APIENTRYP PFNGLUNMAPBUFFERPROC)(GLenum);
typedef void (GL_APIENTRYP PFNGLBINDBUFFERBASEPROC)(GLenum, GLuint, GLuint);

#endif

// Function pointer declarations
extern PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR;
extern PFNGLDEBUGMESSAGECONTROLKHRPROC  glDebugMessageControlKHR;
extern PFNGLDEBUGMESSAGEINSERTKHRPROC   glDebugMessageInsertKHR;
extern PFNGLGETDEBUGMESSAGELOGKHRPROC   glGetDebugMessageLogKHR;
// Uniform block function pointers
extern PFNGLGETUNIFORMBLOCKINDEXPROC    glGetUniformBlockIndexPtr;
extern PFNGLUNIFORMBLOCKBINDINGPROC     glUniformBlockBindingPtr;
// VAO function pointers
extern PFNGLGENVERTEXARRAYSPROC    glGenVertexArraysPtr;
extern PFNGLBINDVERTEXARRAYPROC    glBindVertexArrayPtr;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysPtr;
extern PFNGLTEXSTORAGE3DPROC           glTexStorage3DPtr;
extern PFNGLINVALIDATEFRAMEBUFFERPROC  glInvalidateFramebufferPtr;
extern PFNGLMAPBUFFERRANGEPROC         glMapBufferRangePtr;
extern PFNGLUNMAPBUFFERPROC            glUnmapBufferPtr;
extern PFNGLBINDBUFFERBASEPROC         glBindBufferBasePtr;

// Macro wrappers to replace gl* calls with loaded function pointers
#define glGetUniformBlockIndex glGetUniformBlockIndexPtr
#define glUniformBlockBinding  glUniformBlockBindingPtr
#define glGenVertexArrays    glGenVertexArraysPtr
#define glBindVertexArray    glBindVertexArrayPtr
#define glDeleteVertexArrays glDeleteVertexArraysPtr
// Add macros for safe usage
#define glTexStorage3D           glTexStorage3DPtr
#define glInvalidateFramebuffer  glInvalidateFramebufferPtr
#define glMapBufferRange         glMapBufferRangePtr
#define glUnmapBuffer            glUnmapBufferPtr
#define glBindBufferBase         glBindBufferBasePtr

// Initialization function
bool LoadGLES3Extensions();

#ifdef __cplusplus
}
#endif

#endif // GLES3LOADER_H

