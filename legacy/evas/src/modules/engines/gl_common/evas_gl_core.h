#ifndef EVAS_GL_CORE_H
#define EVAS_GL_CORE_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef EVAS_GL_STANDALONE
#  define ERR(...) \
   fprintf(stderr, __VA_ARGS__);

#  ifdef EVAS_GL_DEBUG
#     define DBG(...) \
         fprintf(stderr, __VA_ARGS__);
#  endif
#else
#  include "config.h"
#  include "evas_common.h"

#  define EVAS_GL_NAME_MANGLE 1
#endif

// Name mangling used when built with Evas
#ifdef EVAS_GL_NAME_MANGLE
#  define GL(name) glsym_##name
#else
#  define GL(name) name
#endif


/*
 * This document is licensed under the SGI Free Software B License Version
 * 2.0. For details, see http://oss.sgi.com/projects/FreeB/ .
 */

/*-------------------------------------------------------------------------
 * Data type definitions
 *-----------------------------------------------------------------------*/

typedef void             GLvoid;
typedef unsigned int     GLenum;
typedef unsigned char    GLboolean;
typedef unsigned int     GLbitfield;
typedef signed char      GLbyte;       // Changed khronos_int8_t
typedef short            GLshort;
typedef int              GLint;
typedef int              GLsizei;
typedef unsigned char    GLubyte;      // Changed khronos_uint8_t
typedef unsigned short   GLushort;
typedef unsigned int     GLuint;
typedef float            GLfloat;      // Changed khronos_float_t
typedef float            GLclampf;     // Changed khronos_float_t
typedef signed int       GLfixed;      // Changed khronos_int32_t

/* GL types for handling large vertex buffer objects */
typedef signed long int  GLintptr;     // Changed khronos_intptr_t
typedef signed long int  GLsizeiptr;   // Changed khronos_ssize_t

#if (!defined(__gl2_h_) && !defined(__gl_h_))
# define __gl_h_
# define __gl2_h_

/* OpenGL ES core versions */
//#define GL_ES_VERSION_2_0                 1

/* ClearBufferMask */
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000

/* Boolean */
#define GL_FALSE                          0
#define GL_TRUE                           1

/* BeginMode */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006

/* AlphaFunction (not supported in ES20) */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* BlendingFactorDest */
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */

/* BlendEquationSeparate */
#define GL_FUNC_ADD                       0x8006
#define GL_BLEND_EQUATION                 0x8009
#define GL_BLEND_EQUATION_RGB             0x8009    /* same as BLEND_EQUATION */
#define GL_BLEND_EQUATION_ALPHA           0x883D

/* BlendSubtract */
#define GL_FUNC_SUBTRACT                  0x800A
#define GL_FUNC_REVERSE_SUBTRACT          0x800B

/* Separate Blend Functions */
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GL_BLEND_COLOR                    0x8005

/* Buffer Objects */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895

#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765

#define GL_CURRENT_VERTEX_ATTRIB          0x8626

/* CullFaceMode */
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408

/* DepthFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* EnableCap */
#define GL_TEXTURE_2D                     0x0DE1
#define GL_CULL_FACE                      0x0B44
#define GL_BLEND                          0x0BE2
#define GL_DITHER                         0x0BD0
#define GL_STENCIL_TEST                   0x0B90
#define GL_DEPTH_TEST                     0x0B71
#define GL_SCISSOR_TEST                   0x0C11
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_COVERAGE                0x80A0

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505

/* FrontFaceDirection */
#define GL_CW                             0x0900
#define GL_CCW                            0x0901

/* GetPName */
#define GL_LINE_WIDTH                     0x0B21
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
#define GL_VIEWPORT                       0x0BA2
#define GL_SCISSOR_BOX                    0x0C10
/*      GL_SCISSOR_TEST */
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_POLYGON_OFFSET_UNITS           0x2A00
/*      GL_POLYGON_OFFSET_FILL */
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB

/* GetTextureParameter */
/*      GL_TEXTURE_MAG_FILTER */
/*      GL_TEXTURE_MIN_FILTER */
/*      GL_TEXTURE_WRAP_S */
/*      GL_TEXTURE_WRAP_T */

#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3

/* HintMode */
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

/* HintTarget */
#define GL_GENERATE_MIPMAP_HINT            0x8192

/* DataType */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_FIXED                          0x140C

/* PixelFormat */
#define GL_DEPTH_COMPONENT                0x1902
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A

/* PixelType */
/*      GL_UNSIGNED_BYTE */
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_5_6_5           0x8363

/* Shaders */
#define GL_FRAGMENT_SHADER                  0x8B30
#define GL_VERTEX_SHADER                    0x8B31
#define GL_MAX_VERTEX_ATTRIBS               0x8869
#define GL_MAX_VERTEX_UNIFORM_VECTORS       0x8DFB
#define GL_MAX_VARYING_VECTORS              0x8DFC
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS   0x8B4C
#define GL_MAX_TEXTURE_IMAGE_UNITS          0x8872
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS     0x8DFD
#define GL_SHADER_TYPE                      0x8B4F
#define GL_DELETE_STATUS                    0x8B80
#define GL_LINK_STATUS                      0x8B82
#define GL_VALIDATE_STATUS                  0x8B83
#define GL_ATTACHED_SHADERS                 0x8B85
#define GL_ACTIVE_UNIFORMS                  0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH        0x8B87
#define GL_ACTIVE_ATTRIBUTES                0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH      0x8B8A
#define GL_SHADING_LANGUAGE_VERSION         0x8B8C
#define GL_CURRENT_PROGRAM                  0x8B8D

/* StencilFunction */
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

/* StencilOp */
/*      GL_ZERO */
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_INVERT                         0x150A
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508

/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

/* TextureMagFilter */
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

/* TextureMinFilter */
/*      GL_NEAREST */
/*      GL_LINEAR */
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

/* TextureParameterName */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

/* TextureTarget */
/*      GL_TEXTURE_2D */
#define GL_TEXTURE                        0x1702

#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C

/* TextureUnit */
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0

/* TextureWrapMode */
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_MIRRORED_REPEAT                0x8370

/* Uniform Types */
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_CUBE                   0x8B60

/* Vertex Arrays */
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED        0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE           0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE         0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE           0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED     0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER        0x8645
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F

/* Read Format */
#define GL_IMPLEMENTATION_COLOR_READ_TYPE   0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B

/* Shader Source */
#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_SHADER_COMPILER                0x8DFA

/* Shader Binary */
#define GL_SHADER_BINARY_FORMATS          0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9

/* Shader Precision-Specified Types */
#define GL_LOW_FLOAT                      0x8DF0
#define GL_MEDIUM_FLOAT                   0x8DF1
#define GL_HIGH_FLOAT                     0x8DF2
#define GL_LOW_INT                        0x8DF3
#define GL_MEDIUM_INT                     0x8DF4
#define GL_HIGH_INT                       0x8DF5

/* Framebuffer Object. */
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41

#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGB565                         0x8D62
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_STENCIL_INDEX                  0x1901
#define GL_STENCIL_INDEX8                 0x8D48

#define GL_RENDERBUFFER_WIDTH             0x8D42
#define GL_RENDERBUFFER_HEIGHT            0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT   0x8D44
#define GL_RENDERBUFFER_RED_SIZE          0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE        0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE         0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE        0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE        0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE      0x8D55

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3

#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20

#define GL_NONE                           0

#define GL_FRAMEBUFFER_COMPLETE                      0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS         0x8CD9
#define GL_FRAMEBUFFER_UNSUPPORTED                   0x8CDD

#define GL_FRAMEBUFFER_BINDING            0x8CA6
#define GL_RENDERBUFFER_BINDING           0x8CA7
#define GL_MAX_RENDERBUFFER_SIZE          0x84E8

#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506

//---------------------------//
// GLES extension defines

/* GL_OES_get_program_binary */
#ifndef GL_OES_get_program_binary
#define GL_PROGRAM_BINARY_LENGTH_OES                            0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS_OES                       0x87FE
#define GL_PROGRAM_BINARY_FORMATS_OES                           0x87FF
#endif

#ifndef GL_ARB_get_program_binary
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#define GL_PROGRAM_BINARY_LENGTH          0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS     0x87FE
#define GL_PROGRAM_BINARY_FORMATS         0x87FF
#endif

/* GL_EXT_read_format_bgra */
#ifndef GL_EXT_read_format_bgra
#define GL_BGRA_EXT                                             0x80E1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT                       0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT                       0x8366
#endif

/* GL_EXT_texture_filter_anisotropic */
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT                           0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                       0x84FF
#endif

/* GL_EXT_texture_format_BGRA8888 */
#ifndef GL_EXT_texture_format_BGRA8888
#define GL_BGRA_EXT                                             0x80E1
#endif

#endif

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
//------------------------------------------------------//
//                         EGL
//------------------------------------------------------//

/* EGL Types */
/* EGLint is defined in eglplatform.h */

//!!!! Should be in 
typedef int EGLint;     // Should properly handle 64bit machine... 

//!!!! X11 Dependent...
typedef Display *EGLNativeDisplayType;
typedef Pixmap   EGLNativePixmapType;
typedef Window   EGLNativeWindowType;

typedef unsigned int EGLBoolean;
typedef unsigned int EGLenum;
typedef void *EGLConfig;
typedef void *EGLContext;
typedef void *EGLDisplay;
typedef void *EGLSurface;
typedef void *EGLClientBuffer;

/* EGL Versioning */
#define EGL_VERSION_1_0			1
#define EGL_VERSION_1_1			1
#define EGL_VERSION_1_2			1
#define EGL_VERSION_1_3			1
#define EGL_VERSION_1_4			1

/* EGL Enumerants. Bitmasks and other exceptional cases aside, most
 * enums are assigned unique values starting at 0x3000.
 */

/* EGL aliases */
#define EGL_FALSE			0
#define EGL_TRUE			1

/* Out-of-band handle values */
#define EGL_DEFAULT_DISPLAY		((EGLNativeDisplayType)0)
#define EGL_NO_CONTEXT			((EGLContext)0)
#define EGL_NO_DISPLAY			((EGLDisplay)0)
#define EGL_NO_SURFACE			((EGLSurface)0)

/* Out-of-band attribute value */
#define EGL_DONT_CARE			((EGLint)-1)

/* Errors / GetError return values */
#define EGL_SUCCESS			0x3000
#define EGL_NOT_INITIALIZED		0x3001
#define EGL_BAD_ACCESS			0x3002
#define EGL_BAD_ALLOC			0x3003
#define EGL_BAD_ATTRIBUTE		0x3004
#define EGL_BAD_CONFIG			0x3005
#define EGL_BAD_CONTEXT			0x3006
#define EGL_BAD_CURRENT_SURFACE		0x3007
#define EGL_BAD_DISPLAY			0x3008
#define EGL_BAD_MATCH			0x3009
#define EGL_BAD_NATIVE_PIXMAP		0x300A
#define EGL_BAD_NATIVE_WINDOW		0x300B
#define EGL_BAD_PARAMETER		0x300C
#define EGL_BAD_SURFACE			0x300D
#define EGL_CONTEXT_LOST		0x300E	/* EGL 1.1 - IMG_power_management */

/* Reserved 0x300F-0x301F for additional errors */

/* Config attributes */
#define EGL_BUFFER_SIZE			0x3020
#define EGL_ALPHA_SIZE			0x3021
#define EGL_BLUE_SIZE			0x3022
#define EGL_GREEN_SIZE			0x3023
#define EGL_RED_SIZE			0x3024
#define EGL_DEPTH_SIZE			0x3025
#define EGL_STENCIL_SIZE		0x3026
#define EGL_CONFIG_CAVEAT		0x3027
#define EGL_CONFIG_ID			0x3028
#define EGL_LEVEL			0x3029
#define EGL_MAX_PBUFFER_HEIGHT		0x302A
#define EGL_MAX_PBUFFER_PIXELS		0x302B
#define EGL_MAX_PBUFFER_WIDTH		0x302C
#define EGL_NATIVE_RENDERABLE		0x302D
#define EGL_NATIVE_VISUAL_ID		0x302E
#define EGL_NATIVE_VISUAL_TYPE		0x302F
#define EGL_PRESERVED_RESOURCES		0x3030
#define EGL_SAMPLES			0x3031
#define EGL_SAMPLE_BUFFERS		0x3032
#define EGL_SURFACE_TYPE		0x3033
#define EGL_TRANSPARENT_TYPE		0x3034
#define EGL_TRANSPARENT_BLUE_VALUE	0x3035
#define EGL_TRANSPARENT_GREEN_VALUE	0x3036
#define EGL_TRANSPARENT_RED_VALUE	0x3037
#define EGL_NONE			0x3038	/* Attrib list terminator */
#define EGL_BIND_TO_TEXTURE_RGB		0x3039
#define EGL_BIND_TO_TEXTURE_RGBA	0x303A
#define EGL_MIN_SWAP_INTERVAL		0x303B
#define EGL_MAX_SWAP_INTERVAL		0x303C
#define EGL_LUMINANCE_SIZE		0x303D
#define EGL_ALPHA_MASK_SIZE		0x303E
#define EGL_COLOR_BUFFER_TYPE		0x303F
#define EGL_RENDERABLE_TYPE		0x3040
#define EGL_MATCH_NATIVE_PIXMAP		0x3041	/* Pseudo-attribute (not queryable) */
#define EGL_CONFORMANT			0x3042

/* Reserved 0x3041-0x304F for additional config attributes */

/* Config attribute values */
#define EGL_SLOW_CONFIG			0x3050	/* EGL_CONFIG_CAVEAT value */
#define EGL_NON_CONFORMANT_CONFIG	0x3051	/* EGL_CONFIG_CAVEAT value */
#define EGL_TRANSPARENT_RGB		0x3052	/* EGL_TRANSPARENT_TYPE value */
#define EGL_RGB_BUFFER			0x308E	/* EGL_COLOR_BUFFER_TYPE value */
#define EGL_LUMINANCE_BUFFER		0x308F	/* EGL_COLOR_BUFFER_TYPE value */

/* More config attribute values, for EGL_TEXTURE_FORMAT */
#define EGL_NO_TEXTURE			0x305C
#define EGL_TEXTURE_RGB			0x305D
#define EGL_TEXTURE_RGBA		0x305E
#define EGL_TEXTURE_2D			0x305F

/* Config attribute mask bits */
#define EGL_PBUFFER_BIT			0x0001	/* EGL_SURFACE_TYPE mask bits */
#define EGL_PIXMAP_BIT			0x0002	/* EGL_SURFACE_TYPE mask bits */
#define EGL_WINDOW_BIT			0x0004	/* EGL_SURFACE_TYPE mask bits */
#define EGL_VG_COLORSPACE_LINEAR_BIT	0x0020	/* EGL_SURFACE_TYPE mask bits */
#define EGL_VG_ALPHA_FORMAT_PRE_BIT	0x0040	/* EGL_SURFACE_TYPE mask bits */
#define EGL_MULTISAMPLE_RESOLVE_BOX_BIT 0x0200	/* EGL_SURFACE_TYPE mask bits */
#define EGL_SWAP_BEHAVIOR_PRESERVED_BIT 0x0400	/* EGL_SURFACE_TYPE mask bits */

#define EGL_OPENGL_ES_BIT		0x0001	/* EGL_RENDERABLE_TYPE mask bits */
#define EGL_OPENVG_BIT			0x0002	/* EGL_RENDERABLE_TYPE mask bits */
#define EGL_OPENGL_ES2_BIT		0x0004	/* EGL_RENDERABLE_TYPE mask bits */
#define EGL_OPENGL_BIT			0x0008	/* EGL_RENDERABLE_TYPE mask bits */

/* QueryString targets */
#define EGL_VENDOR			0x3053
#define EGL_VERSION			0x3054
#define EGL_EXTENSIONS			0x3055
#define EGL_CLIENT_APIS			0x308D

/* QuerySurface / SurfaceAttrib / CreatePbufferSurface targets */
#define EGL_HEIGHT			0x3056
#define EGL_WIDTH			0x3057
#define EGL_LARGEST_PBUFFER		0x3058
#define EGL_TEXTURE_FORMAT		0x3080
#define EGL_TEXTURE_TARGET		0x3081
#define EGL_MIPMAP_TEXTURE		0x3082
#define EGL_MIPMAP_LEVEL		0x3083
#define EGL_RENDER_BUFFER		0x3086
#define EGL_VG_COLORSPACE		0x3087
#define EGL_VG_ALPHA_FORMAT		0x3088
#define EGL_HORIZONTAL_RESOLUTION	0x3090
#define EGL_VERTICAL_RESOLUTION		0x3091
#define EGL_PIXEL_ASPECT_RATIO		0x3092
#define EGL_SWAP_BEHAVIOR		0x3093
#define EGL_MULTISAMPLE_RESOLVE		0x3099

/* EGL_RENDER_BUFFER values / BindTexImage / ReleaseTexImage buffer targets */
#define EGL_BACK_BUFFER			0x3084
#define EGL_SINGLE_BUFFER		0x3085

/* OpenVG color spaces */
#define EGL_VG_COLORSPACE_sRGB		0x3089	/* EGL_VG_COLORSPACE value */
#define EGL_VG_COLORSPACE_LINEAR	0x308A	/* EGL_VG_COLORSPACE value */

/* OpenVG alpha formats */
#define EGL_VG_ALPHA_FORMAT_NONPRE	0x308B	/* EGL_ALPHA_FORMAT value */
#define EGL_VG_ALPHA_FORMAT_PRE		0x308C	/* EGL_ALPHA_FORMAT value */

/* Constant scale factor by which fractional display resolutions &
 * aspect ratio are scaled when queried as integer values.
 */
#define EGL_DISPLAY_SCALING		10000

/* Unknown display resolution/aspect ratio */
#define EGL_UNKNOWN			((EGLint)-1)

/* Back buffer swap behaviors */
#define EGL_BUFFER_PRESERVED		0x3094	/* EGL_SWAP_BEHAVIOR value */
#define EGL_BUFFER_DESTROYED		0x3095	/* EGL_SWAP_BEHAVIOR value */

/* CreatePbufferFromClientBuffer buffer types */
#define EGL_OPENVG_IMAGE		0x3096

/* QueryContext targets */
#define EGL_CONTEXT_CLIENT_TYPE		0x3097

/* CreateContext attributes */
#define EGL_CONTEXT_CLIENT_VERSION	0x3098

/* Multisample resolution behaviors */
#define EGL_MULTISAMPLE_RESOLVE_DEFAULT 0x309A	/* EGL_MULTISAMPLE_RESOLVE value */
#define EGL_MULTISAMPLE_RESOLVE_BOX	0x309B	/* EGL_MULTISAMPLE_RESOLVE value */

/* BindAPI/QueryAPI targets */
#define EGL_OPENGL_ES_API		0x30A0
#define EGL_OPENVG_API			0x30A1
#define EGL_OPENGL_API			0x30A2

/* GetCurrentSurface targets */
#define EGL_DRAW			0x3059
#define EGL_READ			0x305A

/* WaitNative engines */
#define EGL_CORE_NATIVE_ENGINE		0x305B

/* EGL 1.2 tokens renamed for consistency in EGL 1.3 */
#define EGL_COLORSPACE			EGL_VG_COLORSPACE
#define EGL_ALPHA_FORMAT		EGL_VG_ALPHA_FORMAT
#define EGL_COLORSPACE_sRGB		EGL_VG_COLORSPACE_sRGB
#define EGL_COLORSPACE_LINEAR		EGL_VG_COLORSPACE_LINEAR
#define EGL_ALPHA_FORMAT_NONPRE		EGL_VG_ALPHA_FORMAT_NONPRE
#define EGL_ALPHA_FORMAT_PRE		EGL_VG_ALPHA_FORMAT_PRE


#else 

//------------------------------------------------------//
//                         GLX
//------------------------------------------------------//

#define GLX_VERSION_1_1		1
#define GLX_VERSION_1_2		1
#define GLX_VERSION_1_3		1
#define GLX_VERSION_1_4		1

#define GLX_EXTENSION_NAME   "GLX"

/*
 * Tokens for glXChooseVisual and glXGetConfig:
 */
#define GLX_USE_GL		1
#define GLX_BUFFER_SIZE		2
#define GLX_LEVEL		3
#define GLX_RGBA		4
#define GLX_DOUBLEBUFFER	5
#define GLX_STEREO		6
#define GLX_AUX_BUFFERS		7
#define GLX_RED_SIZE		8
#define GLX_GREEN_SIZE		9
#define GLX_BLUE_SIZE		10
#define GLX_ALPHA_SIZE		11
#define GLX_DEPTH_SIZE		12
#define GLX_STENCIL_SIZE	13
#define GLX_ACCUM_RED_SIZE	14
#define GLX_ACCUM_GREEN_SIZE	15
#define GLX_ACCUM_BLUE_SIZE	16
#define GLX_ACCUM_ALPHA_SIZE	17


/*
 * Error codes returned by glXGetConfig:
 */
#define GLX_BAD_SCREEN		1
#define GLX_BAD_ATTRIBUTE	2
#define GLX_NO_EXTENSION	3
#define GLX_BAD_VISUAL		4
#define GLX_BAD_CONTEXT		5
#define GLX_BAD_VALUE       	6
#define GLX_BAD_ENUM		7


/*
 * GLX 1.1 and later:
 */
#define GLX_VENDOR		1
#define GLX_VERSION		2
#define GLX_EXTENSIONS 		3


/*
 * GLX 1.3 and later:
 */
#define GLX_CONFIG_CAVEAT		0x20
#define GLX_DONT_CARE			0xFFFFFFFF
#define GLX_X_VISUAL_TYPE		0x22
#define GLX_TRANSPARENT_TYPE		0x23
#define GLX_TRANSPARENT_INDEX_VALUE	0x24
#define GLX_TRANSPARENT_RED_VALUE	0x25
#define GLX_TRANSPARENT_GREEN_VALUE	0x26
#define GLX_TRANSPARENT_BLUE_VALUE	0x27
#define GLX_TRANSPARENT_ALPHA_VALUE	0x28
#define GLX_WINDOW_BIT			0x00000001
#define GLX_PIXMAP_BIT			0x00000002
#define GLX_PBUFFER_BIT			0x00000004
#define GLX_AUX_BUFFERS_BIT		0x00000010
#define GLX_FRONT_LEFT_BUFFER_BIT	0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT	0x00000002
#define GLX_BACK_LEFT_BUFFER_BIT	0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT	0x00000008
#define GLX_DEPTH_BUFFER_BIT		0x00000020
#define GLX_STENCIL_BUFFER_BIT		0x00000040
#define GLX_ACCUM_BUFFER_BIT		0x00000080
#define GLX_NONE			0x8000
#define GLX_SLOW_CONFIG			0x8001
#define GLX_TRUE_COLOR			0x8002
#define GLX_DIRECT_COLOR		0x8003
#define GLX_PSEUDO_COLOR		0x8004
#define GLX_STATIC_COLOR		0x8005
#define GLX_GRAY_SCALE			0x8006
#define GLX_STATIC_GRAY			0x8007
#define GLX_TRANSPARENT_RGB		0x8008
#define GLX_TRANSPARENT_INDEX		0x8009
#define GLX_VISUAL_ID			0x800B
#define GLX_SCREEN			0x800C
#define GLX_NON_CONFORMANT_CONFIG	0x800D
#define GLX_DRAWABLE_TYPE		0x8010
#define GLX_RENDER_TYPE			0x8011
#define GLX_X_RENDERABLE		0x8012
#define GLX_FBCONFIG_ID			0x8013
#define GLX_RGBA_TYPE			0x8014
#define GLX_COLOR_INDEX_TYPE		0x8015
#define GLX_MAX_PBUFFER_WIDTH		0x8016
#define GLX_MAX_PBUFFER_HEIGHT		0x8017
#define GLX_MAX_PBUFFER_PIXELS		0x8018
#define GLX_PRESERVED_CONTENTS		0x801B
#define GLX_LARGEST_PBUFFER		0x801C
#define GLX_WIDTH			0x801D
#define GLX_HEIGHT			0x801E
#define GLX_EVENT_MASK			0x801F
#define GLX_DAMAGED			0x8020
#define GLX_SAVED			0x8021
#define GLX_WINDOW			0x8022
#define GLX_PBUFFER			0x8023
#define GLX_PBUFFER_HEIGHT              0x8040
#define GLX_PBUFFER_WIDTH               0x8041
#define GLX_RGBA_BIT			0x00000001
#define GLX_COLOR_INDEX_BIT		0x00000002
#define GLX_PBUFFER_CLOBBER_MASK	0x08000000


/*
 * GLX 1.4 and later:
 */
#define GLX_SAMPLE_BUFFERS              0x186a0 /*100000*/
#define GLX_SAMPLES                     0x186a1 /*100001*/



typedef struct __GLXcontextRec *GLXContext;
typedef XID GLXPixmap;
typedef XID GLXDrawable;
/* GLX 1.3 and later */
typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef XID GLXFBConfigID;
typedef XID GLXContextID;
typedef XID GLXWindow;
typedef XID GLXPbuffer;



/*
 * #?. GLX_EXT_texture_from_pixmap
 * XXX not finished?
 */
#ifndef GLX_EXT_texture_from_pixmap
#define GLX_EXT_texture_from_pixmap 1

#define GLX_BIND_TO_TEXTURE_RGB_EXT        0x20D0
#define GLX_BIND_TO_TEXTURE_RGBA_EXT       0x20D1
#define GLX_BIND_TO_MIPMAP_TEXTURE_EXT     0x20D2
#define GLX_BIND_TO_TEXTURE_TARGETS_EXT    0x20D3
#define GLX_Y_INVERTED_EXT                 0x20D4

#define GLX_TEXTURE_FORMAT_EXT             0x20D5
#define GLX_TEXTURE_TARGET_EXT             0x20D6
#define GLX_MIPMAP_TEXTURE_EXT             0x20D7

#define GLX_TEXTURE_FORMAT_NONE_EXT        0x20D8
#define GLX_TEXTURE_FORMAT_RGB_EXT         0x20D9
#define GLX_TEXTURE_FORMAT_RGBA_EXT        0x20DA

#define GLX_TEXTURE_1D_BIT_EXT             0x00000001
#define GLX_TEXTURE_2D_BIT_EXT             0x00000002
#define GLX_TEXTURE_RECTANGLE_BIT_EXT      0x00000004

#define GLX_TEXTURE_1D_EXT                 0x20DB
#define GLX_TEXTURE_2D_EXT                 0x20DC
#define GLX_TEXTURE_RECTANGLE_EXT          0x20DD

#define GLX_FRONT_LEFT_EXT                 0x20DE
#define GLX_FRONT_RIGHT_EXT                0x20DF
#define GLX_BACK_LEFT_EXT                  0x20E0
#define GLX_BACK_RIGHT_EXT                 0x20E1
#define GLX_FRONT_EXT                      GLX_FRONT_LEFT_EXT
#define GLX_BACK_EXT                       GLX_BACK_LEFT_EXT
#define GLX_AUX0_EXT                       0x20E2
#define GLX_AUX1_EXT                       0x20E3 
#define GLX_AUX2_EXT                       0x20E4 
#define GLX_AUX3_EXT                       0x20E5 
#define GLX_AUX4_EXT                       0x20E6 
#define GLX_AUX5_EXT                       0x20E7 
#define GLX_AUX6_EXT                       0x20E8
#define GLX_AUX7_EXT                       0x20E9 
#define GLX_AUX8_EXT                       0x20EA 
#define GLX_AUX9_EXT                       0x20EB

#endif //GLX_EXT_texture_from_pixmap

#endif //EGL vs. GLX


//------------------------------------------------------//

#define MAX_TEXTURE_UNITS 32
#define MAX_VERTEX_ATTRIBS 16
typedef struct _EvasGlueContext     *EvasGlueContext;

typedef struct _GL_Texture_State
{
   GLint    tex_unit;
   GLuint   tex_id;
} GL_Texture_State;

typedef struct _GL_Vertex_Array_State
{
   GLboolean   modified;
   GLboolean   enabled;
   GLint       size;
   GLenum      type;
   GLboolean   normalized;
   GLsizei     stride;
   void       *pointer;
} GL_Vertex_Array_State;

typedef struct _GL_Vertex_Attrib
{
   GLboolean   modified;
   GLfloat     value[4]; 
} GL_Vertex_Attrib;


#define MAGIC_GLFAST   0x73777770

struct _EvasGlueContext 
{

   int            magic;

   // First time flag
   int            first_time;

   // Default Framebuffer and RenderBuffers
   GLuint         fb_zero;
   GLuint         rb_zero;
   GLuint         evasgl_enabled;

   GLint          num_tex_units;
   GLint          num_vertex_attribs;

   //----------------------------------------//
   // GL States
   // Bind Functions
   // glBind {Buffer, Framebuffer, Renderbuffer}
   // * Texture Binding is done with textures
   unsigned char _bind_flag;
   GLuint      gl_array_buffer_binding;               // 0
   GLuint      gl_element_array_buffer_binding;       // 0
   GLuint      gl_framebuffer_binding;                // 0
   GLuint      gl_renderbuffer_binding;               // 0

   //------------------//
   // Enable States
   // glEnable()
   unsigned char _enable_flag1;
   GLboolean   gl_blend;                              // GL_FALSE
   GLboolean   gl_cull_face;                          // GL_FALSE
   GLboolean   gl_depth_test;                         // GL_FALSE
   GLboolean   gl_dither;                             // GL_TRUE

   unsigned char _enable_flag2;
   GLboolean   gl_polygon_offset_fill;                // GL_FALSE
   GLboolean   gl_sample_alpha_to_coverage;           // GL_FALSE
   GLboolean   gl_sample_coverage;                    // GL_FALSE
   GLboolean   gl_scissor_test;                       // GL_FALSE
   GLboolean   gl_stencil_test;                       // GL_FALSE

   //------------------//
   unsigned char _clear_flag1;
   // Viewport
   GLint       gl_viewport[4];                        // (0,0,w,h)
   // Program (Shaders)
   GLuint      gl_current_program;                    // 0

   // Clear Color
   GLclampf    gl_color_clear_value[4];               // (0,0,0,0)

   //------------------//
   unsigned char _clear_flag2;
   GLboolean   gl_color_writemask[4];                 // (GL_TRUE x 4)
   // Depth
   GLclampf    gl_depth_range[2];                     // (0,1)
   GLclampf    gl_depth_clear_value;                  // 1
   GLenum      gl_depth_func;                         // GL_LESS
   GLboolean   gl_depth_writemask;                    // GL_TRUE
   GLenum      gl_cull_face_mode;                     // GL_FALSE

   //------------------//
   unsigned char _tex_flag1;
   // Texture
   GL_Texture_State  tex_state[MAX_TEXTURE_UNITS];
   GLenum            gl_active_texture;               // GL_TEXTURE0
   GLenum            gl_generate_mipmap_hint;         // GL_DONT_CARE
   GLuint            gl_texture_binding_2d;           // 0
   GLuint            gl_texture_binding_cube_map;     // 0

   //unsigned char _tex_flag2[4];

   //------------------//
   unsigned char _blend_flag;
   // Blending
   GLclampf    gl_blend_color[4];                 
   GLenum      gl_blend_src_rgb;                      // GL_ONE
   GLenum      gl_blend_src_alpha;                    // GL_ONE
   GLenum      gl_blend_dst_rgb;                      // GL_ZERO
   GLenum      gl_blend_dst_alpha;                    // GL_ZERO
   GLenum      gl_blend_equation_rgb;
   GLenum      gl_blend_equation_alpha;               

   //------------------//
   unsigned char _stencil_flag1;
   // Stencil
   GLenum      gl_stencil_func;                       // GL_ALWAYS
   GLint       gl_stencil_ref;
   GLuint      gl_stencil_value_mask;                 // 0xffffffff
   GLenum      gl_stencil_fail;                       // GL_KEEP
   GLenum      gl_stencil_pass_depth_fail;            // GL_KEEP
   GLenum      gl_stencil_pass_depth_pass;            // GL_KEEP
   GLuint      gl_stencil_writemask;                  // 0xffffffff

   unsigned char _stencil_flag2;
   GLenum      gl_stencil_back_func;                  // GL_ALWAYS
   GLint       gl_stencil_back_ref;
   GLuint      gl_stencil_back_value_mask;            // 0xffffffff
   GLenum      gl_stencil_back_fail;                  // GL_KEEP
   GLenum      gl_stencil_back_depth_fail;            // GL_KEEP
   GLenum      gl_stencil_back_depth_pass;            // GL_KEEP
   GLuint      gl_stencil_back_writemask;             // 0xffffffff

   GLint       gl_stencil_clear_value;                


   //------------------//


   //------------------//
   unsigned char _misc_flag1;
   GLenum      gl_front_face;                         // GL_CCW
   GLfloat     gl_line_width;                         // 1
   GLfloat     gl_polygon_offset_factor;              // 0
   GLfloat     gl_polygon_offset_units;               // 0
   GLclampf    gl_sample_coverage_value;
   GLboolean   gl_sample_coverage_invert;

   unsigned char _misc_flag2;
   GLint       gl_scissor_box[4];                     // (0,0,w,h)
   GLint       gl_pack_alignment;                     // 4
   GLint       gl_unpack_alignment;                   // 4

   //------------------//
   // Vertex Attrib Array
   unsigned char _varray_flag;
   GL_Vertex_Array_State      vertex_array[MAX_VERTEX_ATTRIBS];
   GLint                      gl_current_vertex_attrib[4];

   // Verte Attribs
   unsigned char _vattrib_flag;
   GL_Vertex_Attrib           vertex_attrib[MAX_VERTEX_ATTRIBS];

};


#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
//------------------------------------------------------//

typedef void (*_eng_fn) (void);

//------------------------------------------------------//
// EGL APIs... Only ones that are being used.

// Standard EGL APIs
extern _eng_fn       (*GL(eglGetProcAddress))                     (const char* procname);

extern EGLint        (*GL(eglGetError))                  (void);
extern EGLDisplay    (*GL(eglGetDisplay))                (EGLNativeDisplayType display_id);
extern EGLBoolean    (*GL(eglInitialize))                (EGLDisplay dpy, EGLint* major, EGLint *minor);
extern EGLBoolean    (*GL(eglTerminate))                 (EGLDisplay dpy);
extern EGLBoolean    (*GL(eglChooseConfig))              (EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
extern EGLSurface    (*GL(eglCreateWindowSurface))       (EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list);
extern EGLSurface    (*GL(eglCreatePixmapSurface))       (EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint* attrib_list);
extern EGLBoolean    (*GL(eglDestroySurface))            (EGLDisplay dpy, EGLSurface surface);
extern EGLBoolean    (*GL(eglBindAPI))                   (EGLenum api);
extern EGLBoolean    (*GL(eglWaitClient))                (void);
extern EGLBoolean    (*GL(eglSurfaceAttrib))             (EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
extern void          (*GL(eglBindTexImage))              (EGLDisplay dpy, EGLSurface surface, EGLint buffer);
extern EGLBoolean    (*GL(eglReleaseTexImage))           (EGLDisplay dpy, EGLSurface surface, EGLint buffer);
extern EGLBoolean    (*GL(eglSwapInterval))              (EGLDisplay dpy, EGLint interval);
extern EGLContext    (*GL(eglCreateContext))             (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list);
extern EGLBoolean    (*GL(eglDestroyContext))            (EGLDisplay dpy, EGLContext ctx);
extern EGLBoolean    (*GL(eglMakeCurrent))               (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
extern EGLContext    (*GL(eglGetCurrentContext))         (void);
extern EGLSurface    (*GL(eglGetCurrentSurface))         (EGLint readdraw);
extern EGLDisplay    (*GL(eglGetCurrentDisplay))         (void);
extern EGLBoolean    (*GL(eglWaitGL))                    (void);
extern EGLBoolean    (*GL(eglWaitNative))                (EGLint engine);
extern EGLBoolean    (*GL(eglSwapBuffers))               (EGLDisplay dpy, EGLSurface surface);
extern EGLBoolean    (*GL(eglCopyBuffers))               (EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
extern char const   *(*GL(eglQueryString))               (EGLDisplay dpy, EGLint name);

// Extensions
extern void         *(*GL(eglCreateImage))               (void *a, void *b, GLenum c, void *d, const int *e);
extern unsigned int  (*GL(eglDestroyImage))              (void *a, void *b);
extern void          (*GL(glEGLImageTargetTexture2DOES)) (int a, void *b);
extern void          (*GL(glEGLImageTargetRenderbufferStorageOES)) (int a, void *b);
extern void         *(*GL(eglMapImageSEC))               (void *a, void *b);
extern unsigned int  (*GL(eglUnmapImageSEC))             (void *a, void *b);
extern unsigned int  (*GL(eglGetImageAttribSEC))         (void *a, void *b, int c, int *d);
//extern unsigned int   (*GL(eglLockSurface))              (EGLDisplay a, EGLSurface b, const int *attrib_list);
//extern unsigned int   (*GL(eglUnlockSurface))            (EGLDisplay a, EGLSurface b);


//------------------------------------------------------//

#else

//------------------------------------------------------//

typedef void (*_eng_fn) (void);

//------------------------------------------------------//
// GLX APIs... Only ones that are being used.

//--------//
extern _eng_fn      (*GL(glXGetProcAddress))            (const char* procName);

// Standard GL(glX) functions
extern XVisualInfo* (*GL(glXChooseVisual))              (Display* dpy, int screen, int* attribList);
extern GLXContext   (*GL(glXCreateContext))             (Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct);
extern void         (*GL(glXDestroyContext))            (Display* dpy, GLXContext ctx);
extern GLXContext   (*GL(glXGetCurrentContext))         (void);
extern GLXDrawable  (*GL(glXGetCurrentDrawable))        (void);
extern Bool         (*GL(glXMakeCurrent))               (Display* dpy, GLXDrawable draw, GLXContext ctx);
extern void         (*GL(glXSwapBuffers))               (Display* dpy, GLXDrawable draw);
extern void         (*GL(glXWaitX))                     (void);
extern void         (*GL(glXWaitGL))                    (void);
extern Bool         (*GL(glXQueryExtension))            (Display* dpy, int* errorb, int* event);
extern const char  *(*GL(glXQueryExtensionsString))     (Display *dpy, int screen);

//--------//
extern GLXFBConfig* (*GL(glXChooseFBConfig))            (Display* dpy, int screen, const int* attribList, int* nitems);
extern GLXFBConfig* (*GL(glXGetFBConfigs))              (Display* dpy, int screen, int* nelements);
extern int          (*GL(glXGetFBConfigAttrib))         (Display* dpy, GLXFBConfig config, int attribute, int* value);
extern XVisualInfo* (*GL(glXGetVisualFromFBConfig))     (Display* dpy, GLXFBConfig config);
extern void         (*GL(glXDestroyWindow))             (Display* dpy, GLXWindow window);
extern Bool         (*GL(glXMakeContextCurrent))        (Display* dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
//--------//

extern void         (*GL(glXBindTexImage))              (Display* dpy, GLXDrawable draw, int buffer, int* attribList);
extern void         (*GL(glXReleaseTexImage))           (Display* dpy, GLXDrawable draw, int buffer);
extern int          (*GL(glXGetVideoSync))              (unsigned int* count);
extern int          (*GL(glXWaitVideoSync))             (int divisor, int remainder, unsigned int* count);
extern XID          (*GL(glXCreatePixmap))              (Display* dpy, void* config, Pixmap pixmap, const int* attribList);
extern void         (*GL(glXDestroyPixmap))             (Display* dpy, XID pixmap);
extern void         (*GL(glXQueryDrawable))             (Display* dpy, XID draw, int attribute, unsigned int* value);
extern int          (*GL(glXSwapIntervalSGI))           (int interval);
extern void         (*GL(glXSwapIntervalEXT))           (Display* dpy, GLXDrawable draw, int interval);

//--------//

#endif // EGL vs. GLX

//------------------------------------------------------//

typedef enum _Evas_GL_Opt_Flag
{
   GL_NORMAL_PATH,
   GL_WRAPPED_PATH,
   GL_FAST_PATH
} Evas_GL_Opt_Flag;


//#define GL_ERRORS 1
   /*
   { \
      int __gl_err = _gl.GL(glGetError)(); \
      if (__gl_err != GL_NO_ERROR) glerr(__gl_err, fl, fn, ln, op); \
   }
   */
#ifdef GL_ERRORS
# define GLERR(fn, fl, ln, op)
#else
# define GLERR(fn, fl, ln, op)
#endif

//extern Evas_GL_API _gl;


extern int  init_gl();
extern void free_gl();

   /* version 1: */
extern   void         (*GL(glActiveTexture)) (GLenum texture);
extern   void         (*GL(glAttachShader)) (GLuint program, GLuint shader);
extern   void         (*GL(glBindAttribLocation)) (GLuint program, GLuint index, const char* name);
extern   void         (*GL(glBindBuffer)) (GLenum target, GLuint buffer);
extern   void         (*GL(glBindFramebuffer)) (GLenum target, GLuint framebuffer);
extern   void         (*GL(glBindRenderbuffer)) (GLenum target, GLuint renderbuffer);
extern   void         (*GL(glBindTexture)) (GLenum target, GLuint texture);
extern   void         (*GL(glBlendColor)) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern   void         (*GL(glBlendEquation)) ( GLenum mode );
extern   void         (*GL(glBlendEquationSeparate)) (GLenum modeRGB, GLenum modeAlpha);
extern   void         (*GL(glBlendFunc)) (GLenum sfactor, GLenum dfactor);
extern   void         (*GL(glBlendFuncSeparate)) (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern   void         (*GL(glBufferData)) (GLenum target, GLsizeiptr size, const void* data, GLenum usage);
extern   void         (*GL(glBufferSubData)) (GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
extern   GLenum       (*GL(glCheckFramebufferStatus)) (GLenum target);
extern   void         (*GL(glClear)) (GLbitfield mask);
extern   void         (*GL(glClearColor)) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern   void         (*GL(glClearDepthf)) (GLclampf depth);
extern   void         (*GL(glClearStencil)) (GLint s);
extern   void         (*GL(glColorMask)) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern   void         (*GL(glCompileShader)) (GLuint shader);
extern   void         (*GL(glCompressedTexImage2D)) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
extern   void         (*GL(glCompressedTexSubImage2D)) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
extern   void         (*GL(glCopyTexImage2D)) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern   void         (*GL(glCopyTexSubImage2D)) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern   GLuint       (*GL(glCreateProgram)) (void);
extern   GLuint       (*GL(glCreateShader)) (GLenum type);
extern   void         (*GL(glCullFace)) (GLenum mode);
extern   void         (*GL(glDeleteBuffers)) (GLsizei n, const GLuint* buffers);
extern   void         (*GL(glDeleteFramebuffers)) (GLsizei n, const GLuint* framebuffers);
extern   void         (*GL(glDeleteProgram)) (GLuint program);
extern   void         (*GL(glDeleteRenderbuffers)) (GLsizei n, const GLuint* renderbuffers);
extern   void         (*GL(glDeleteShader)) (GLuint shader);
extern   void         (*GL(glDeleteTextures)) (GLsizei n, const GLuint* textures);
extern   void         (*GL(glDepthFunc)) (GLenum func);
extern   void         (*GL(glDepthMask)) (GLboolean flag);
extern   void         (*GL(glDepthRangef)) (GLclampf zNear, GLclampf zFar);
extern   void         (*GL(glDetachShader)) (GLuint program, GLuint shader);
extern   void         (*GL(glDisable)) (GLenum cap);
extern   void         (*GL(glDisableVertexAttribArray)) (GLuint index);
extern   void         (*GL(glDrawArrays)) (GLenum mode, GLint first, GLsizei count);
extern   void         (*GL(glDrawElements)) (GLenum mode, GLsizei count, GLenum type, const void* indices);
extern   void         (*GL(glEnable)) (GLenum cap);
extern   void         (*GL(glEnableVertexAttribArray)) (GLuint index);
extern   void         (*GL(glFinish)) (void);
extern   void         (*GL(glFlush)) (void);
extern   void         (*GL(glFramebufferRenderbuffer)) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern   void         (*GL(glFramebufferTexture2D)) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern   void         (*GL(glFrontFace)) (GLenum mode);
extern   void         (*GL(glGenBuffers)) (GLsizei n, GLuint* buffers);
extern   void         (*GL(glGenerateMipmap)) (GLenum target);
extern   void         (*GL(glGenFramebuffers)) (GLsizei n, GLuint* framebuffers);
extern   void         (*GL(glGenRenderbuffers)) (GLsizei n, GLuint* renderbuffers);
extern   void         (*GL(glGenTextures)) (GLsizei n, GLuint* textures);
extern   void         (*GL(glGetActiveAttrib)) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name);
extern   void         (*GL(glGetActiveUniform)) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name);
extern   void         (*GL(glGetAttachedShaders)) (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
extern   int          (*GL(glGetAttribLocation)) (GLuint program, const char* name);
extern   void         (*GL(glGetBooleanv)) (GLenum pname, GLboolean* params);
extern   void         (*GL(glGetBufferParameteriv)) (GLenum target, GLenum pname, GLint* params);
extern   GLenum       (*GL(glGetError)) (void);
extern   void         (*GL(glGetFloatv)) (GLenum pname, GLfloat* params);
extern   void         (*GL(glGetFramebufferAttachmentParameteriv)) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
extern   void         (*GL(glGetIntegerv)) (GLenum pname, GLint* params);
extern   void         (*GL(glGetProgramiv)) (GLuint program, GLenum pname, GLint* params);
extern   void         (*GL(glGetProgramInfoLog)) (GLuint program, GLsizei bufsize, GLsizei* length, char* infolog);
extern   void         (*GL(glGetRenderbufferParameteriv)) (GLenum target, GLenum pname, GLint* params);
extern   void         (*GL(glGetShaderiv)) (GLuint shader, GLenum pname, GLint* params);
extern   void         (*GL(glGetShaderInfoLog)) (GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog);
extern   void         (*GL(glGetShaderPrecisionFormat)) (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
extern   void         (*GL(glGetShaderSource)) (GLuint shader, GLsizei bufsize, GLsizei* length, char* source);
extern   const GLubyte *(*GL(glGetString)) (GLenum name);
extern   void         (*GL(glGetTexParameterfv)) (GLenum target, GLenum pname, GLfloat* params);
extern   void         (*GL(glGetTexParameteriv)) (GLenum target, GLenum pname, GLint* params);
extern   void         (*GL(glGetUniformfv)) (GLuint program, GLint location, GLfloat* params);
extern   void         (*GL(glGetUniformiv)) (GLuint program, GLint location, GLint* params);
extern   int          (*GL(glGetUniformLocation)) (GLuint program, const char* name);
extern   void         (*GL(glGetVertexAttribfv)) (GLuint index, GLenum pname, GLfloat* params);
extern   void         (*GL(glGetVertexAttribiv)) (GLuint index, GLenum pname, GLint* params);
extern   void         (*GL(glGetVertexAttribPointerv)) (GLuint index, GLenum pname, void** pointer);
extern   void         (*GL(glHint)) (GLenum target, GLenum mode);
extern   GLboolean    (*GL(glIsBuffer)) (GLuint buffer);
extern   GLboolean    (*GL(glIsEnabled)) (GLenum cap);
extern   GLboolean    (*GL(glIsFramebuffer)) (GLuint framebuffer);
extern   GLboolean    (*GL(glIsProgram)) (GLuint program);
extern   GLboolean    (*GL(glIsRenderbuffer)) (GLuint renderbuffer);
extern   GLboolean    (*GL(glIsShader)) (GLuint shader);
extern   GLboolean    (*GL(glIsTexture)) (GLuint texture);
extern   void         (*GL(glLineWidth)) (GLfloat width);
extern   void         (*GL(glLinkProgram)) (GLuint program);
extern   void         (*GL(glPixelStorei)) (GLenum pname, GLint param);
extern   void         (*GL(glPolygonOffset)) (GLfloat factor, GLfloat units);
extern   void         (*GL(glReadPixels)) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
extern   void         (*GL(glReleaseShaderCompiler)) (void);
extern   void         (*GL(glRenderbufferStorage)) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern   void         (*GL(glSampleCoverage)) (GLclampf value, GLboolean invert);
extern   void         (*GL(glScissor)) (GLint x, GLint y, GLsizei width, GLsizei height);
extern   void         (*GL(glShaderBinary)) (GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length);
extern   void         (*GL(glShaderSource)) (GLuint shader, GLsizei count, const char** string, const GLint* length);
extern   void         (*GL(glStencilFunc)) (GLenum func, GLint ref, GLuint mask);
extern   void         (*GL(glStencilFuncSeparate)) (GLenum face, GLenum func, GLint ref, GLuint mask);
extern   void         (*GL(glStencilMask)) (GLuint mask);
extern   void         (*GL(glStencilMaskSeparate)) (GLenum face, GLuint mask);
extern   void         (*GL(glStencilOp)) (GLenum fail, GLenum zfail, GLenum zpass);
extern   void         (*GL(glStencilOpSeparate)) (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
extern   void         (*GL(glTexImage2D)) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
extern   void         (*GL(glTexParameterf)) (GLenum target, GLenum pname, GLfloat param);
extern   void         (*GL(glTexParameterfv)) (GLenum target, GLenum pname, const GLfloat* params);
extern   void         (*GL(glTexParameteri)) (GLenum target, GLenum pname, GLint param);
extern   void         (*GL(glTexParameteriv)) (GLenum target, GLenum pname, const GLint* params);
extern   void         (*GL(glTexSubImage2D)) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
extern   void         (*GL(glUniform1f)) (GLint location, GLfloat x);
extern   void         (*GL(glUniform1fv)) (GLint location, GLsizei count, const GLfloat* v);
extern   void         (*GL(glUniform1i)) (GLint location, GLint x);
extern   void         (*GL(glUniform1iv)) (GLint location, GLsizei count, const GLint* v);
extern   void         (*GL(glUniform2f)) (GLint location, GLfloat x, GLfloat y);
extern   void         (*GL(glUniform2fv)) (GLint location, GLsizei count, const GLfloat* v);
extern   void         (*GL(glUniform2i)) (GLint location, GLint x, GLint y);
extern   void         (*GL(glUniform2iv)) (GLint location, GLsizei count, const GLint* v);
extern   void         (*GL(glUniform3f)) (GLint location, GLfloat x, GLfloat y, GLfloat z);
extern   void         (*GL(glUniform3fv)) (GLint location, GLsizei count, const GLfloat* v);
extern   void         (*GL(glUniform3i)) (GLint location, GLint x, GLint y, GLint z);
extern   void         (*GL(glUniform3iv)) (GLint location, GLsizei count, const GLint* v);
extern   void         (*GL(glUniform4f)) (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern   void         (*GL(glUniform4fv)) (GLint location, GLsizei count, const GLfloat* v);
extern   void         (*GL(glUniform4i)) (GLint location, GLint x, GLint y, GLint z, GLint w);
extern   void         (*GL(glUniform4iv)) (GLint location, GLsizei count, const GLint* v);
extern   void         (*GL(glUniformMatrix2fv)) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern   void         (*GL(glUniformMatrix3fv)) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern   void         (*GL(glUniformMatrix4fv)) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern   void         (*GL(glUseProgram)) (GLuint program);
extern   void         (*GL(glValidateProgram)) (GLuint program);
extern   void         (*GL(glVertexAttrib1f)) (GLuint indx, GLfloat x);
extern   void         (*GL(glVertexAttrib1fv)) (GLuint indx, const GLfloat* values);
extern   void         (*GL(glVertexAttrib2f)) (GLuint indx, GLfloat x, GLfloat y);
extern   void         (*GL(glVertexAttrib2fv)) (GLuint indx, const GLfloat* values);
extern   void         (*GL(glVertexAttrib3f)) (GLuint indx, GLfloat x, GLfloat y, GLfloat z);
extern   void         (*GL(glVertexAttrib3fv)) (GLuint indx, const GLfloat* values);
extern   void         (*GL(glVertexAttrib4f)) (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern   void         (*GL(glVertexAttrib4fv)) (GLuint indx, const GLfloat* values);
extern   void         (*GL(glVertexAttribPointer)) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr);
extern   void         (*GL(glViewport)) (GLint x, GLint y, GLsizei width, GLsizei height);

/* Extensions */
extern   void         (*GL(glGetProgramBinary)) (GLuint program, GLsizei bufsize, GLsizei *length, GLenum *binaryFormat, void *binary);
extern   void         (*GL(glProgramBinary)) (GLuint program, GLenum binaryFormat, const void *binary, GLint length);
extern   void         (*GL(glProgramParameteri)) (GLuint a, GLuint b, GLint d);


#endif // EVAS_GL_CORE_H
