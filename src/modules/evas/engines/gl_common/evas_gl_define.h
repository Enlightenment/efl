#ifndef EVAS_GL_DEFINE_H_
# define EVAS_GL_DEFINE_H_

/*
Some of the remainder of this file is lifted from Khronos' public
EGL/glext headers.  The copyright notice from a recent version of the
headers is reproduced here.

Much of it is from Mesa, so their copyright is included as well.
*/

/*
** Copyright (c) 2013-2016 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/
/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef GL_TEXTURE_RECTANGLE_NV
# define GL_TEXTURE_RECTANGLE_NV 0x84F5
#endif
#ifndef GL_ABGR
# define GL_ABGR 0x8000
#endif
#ifndef GL_BGRA
# define GL_BGRA 0x80E1
#endif
#ifndef GL_RGBA4
# define GL_RGBA4 0x8056
#endif
#ifndef GL_RGBA8
# define GL_RGBA8 0x8058
#endif
#ifndef GL_RGBA12
# define GL_RGBA12 0x805A
#endif
#ifndef GL_RGBA16
# define GL_RGBA16 0x805B
#endif
#ifndef GL_R3_G3_B2
# define GL_R3_G3_B2 0x2A10
#endif
#ifndef GL_RGB4
# define GL_RGB4 0x804F
#endif
#ifndef GL_RGB5
# define GL_RGB5 0x8050
#endif
#ifndef GL_RGB8
# define GL_RGB8 0x8051
#endif
#ifndef GL_RGB10
# define GL_RGB10 0x8052
#endif
#ifndef GL_RGB12
# define GL_RGB12 0x8053
#endif
#ifndef GL_RGB16
# define GL_RGB16 0x8054
#endif
#ifndef GL_ALPHA4
# define GL_ALPHA4 0x803B
#endif
#ifndef GL_ALPHA8
# define GL_ALPHA8 0x803C
#endif
#ifndef GL_ALPHA12
# define GL_ALPHA12 0x803D
#endif
#ifndef GL_ALPHA16
# define GL_ALPHA16 0x803E
#endif
#ifndef GL_LUMINANCE4
# define GL_LUMINANCE4 0x803F
#endif
#ifndef GL_LUMINANCE8
# define GL_LUMINANCE8 0x8040
#endif
#ifndef GL_LUMINANCE12
# define GL_LUMINANCE12 0x8041
#endif
#ifndef GL_LUMINANCE16
# define GL_LUMINANCE16 0x8042
#endif
#ifndef GL_LUMINANCE4_ALPHA4
# define GL_LUMINANCE4_ALPHA4 0x8043
#endif
#ifndef GL_LUMINANCE8_ALPHA8
# define GL_LUMINANCE8_ALPHA8 0x8045
#endif
#ifndef GL_LUMINANCE12_ALPHA12
# define GL_LUMINANCE12_ALPHA12 0x8047
#endif
#ifndef GL_LUMINANCE16_ALPHA16
# define GL_LUMINANCE16_ALPHA16 0x8048
#endif
#ifndef GL_ETC1_RGB8_OES
# define GL_ETC1_RGB8_OES 0x8D64
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
# define GL_COMPRESSED_RGB8_ETC2 0x9274
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
# define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#endif
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
# define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
# define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
# define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
# define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_TEXTURE_EXTERNAL_OES
# define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif
#ifndef GL_UNPACK_ROW_LENGTH
# define GL_UNPACK_ROW_LENGTH 0x0CF2
#endif
#ifndef GL_DEPTH_COMPONENT24
# define GL_DEPTH_COMPONENT24 0x81A6
#endif
#ifndef EGL_NO_DISPLAY
# define EGL_NO_DISPLAY 0
#endif
#ifndef EGL_NO_CONTEXT
# define EGL_NO_CONTEXT 0
#endif
#ifndef EGL_NONE
# define EGL_NONE 0x3038
#endif
#ifndef EGL_TRUE
# define EGL_TRUE 1
#endif
#ifndef EGL_FALSE
# define EGL_FALSE 0
#endif

#ifndef EGL_MAP_GL_TEXTURE_2D_SEC
# define EGL_MAP_GL_TEXTURE_2D_SEC 0x3201
#endif
#ifndef  EGL_MAP_GL_TEXTURE_HEIGHT_SEC
# define EGL_MAP_GL_TEXTURE_HEIGHT_SEC 0x3202
#endif
#ifndef EGL_MAP_GL_TEXTURE_WIDTH_SEC
# define EGL_MAP_GL_TEXTURE_WIDTH_SEC 0x3203
#endif
#ifndef EGL_MAP_GL_TEXTURE_FORMAT_SEC
# define EGL_MAP_GL_TEXTURE_FORMAT_SEC 0x3204
#endif
#ifndef EGL_MAP_GL_TEXTURE_RGB_SEC
# define EGL_MAP_GL_TEXTURE_RGB_SEC 0x3205
#endif
#ifndef EGL_MAP_GL_TEXTURE_RGBA_SEC
# define EGL_MAP_GL_TEXTURE_RGBA_SEC 0x3206
#endif
#ifndef EGL_MAP_GL_TEXTURE_BGRA_SEC
# define EGL_MAP_GL_TEXTURE_BGRA_SEC 0x3207
#endif
#ifndef EGL_MAP_GL_TEXTURE_LUMINANCE_SEC
# define EGL_MAP_GL_TEXTURE_LUMINANCE_SEC 0x3208
#endif
#ifndef EGL_MAP_GL_TEXTURE_LUMINANCE_ALPHA_SEC
# define EGL_MAP_GL_TEXTURE_LUMINANCE_ALPHA_SEC 0x3209
#endif
#ifndef EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC
# define EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC 0x320a
#endif
#ifndef EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC
# define EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC 0x320b
#endif
#ifndef EGL_MAP_GL_TEXTURE_STRIDE_IN_BYTES_SEC
# define EGL_MAP_GL_TEXTURE_STRIDE_IN_BYTES_SEC 0x320c
#endif
#ifndef EGL_IMAGE_PRESERVED_KHR
# define EGL_IMAGE_PRESERVED_KHR 0x30D2
#endif
#ifndef EGL_NATIVE_SURFACE_TIZEN
#define EGL_NATIVE_SURFACE_TIZEN 0x32A1
#endif
#ifndef GL_PROGRAM_BINARY_LENGTH
# define GL_PROGRAM_BINARY_LENGTH 0x8741
#endif
#ifndef GL_NUM_PROGRAM_BINARY_FORMATS
# define GL_NUM_PROGRAM_BINARY_FORMATS 0x87FE
#endif
#ifndef GL_PROGRAM_BINARY_FORMATS
# define GL_PROGRAM_BINARY_FORMATS 0x87FF
#endif
#ifndef GL_PROGRAM_BINARY_RETRIEVABLE_HINT
# define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#endif
#ifndef GL_MAX_SAMPLES_IMG
#define GL_MAX_SAMPLES_IMG 0x9135
#endif
#ifndef GL_MAX_SAMPLES_EXT
#define GL_MAX_SAMPLES_EXT 0x8D57
#endif
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9
#endif
#ifndef GL_NUM_EXTENSIONS
#define GL_NUM_EXTENSIONS 0x821D
#endif
#ifndef EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC
#define EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC 1
#endif
#ifndef EGL_MAP_GL_TEXTURE_DEVICE_G2D_SEC
#define EGL_MAP_GL_TEXTURE_DEVICE_G2D_SEC 2
#endif
#ifndef EGL_MAP_GL_TEXTURE_OPTION_READ_SEC
#define EGL_MAP_GL_TEXTURE_OPTION_READ_SEC (1<<0)
#endif
#ifndef EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC
#define EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC (1<<1)
#endif
#ifndef EGL_GL_TEXTURE_2D_KHR
#define EGL_GL_TEXTURE_2D_KHR 0x30B1
#endif
#ifndef EGL_GL_TEXTURE_LEVEL_KHR
#define EGL_GL_TEXTURE_LEVEL_KHR 0x30BC
#endif
#ifndef EGL_IMAGE_PRESERVED_KHR
#define EGL_IMAGE_PRESERVED_KHR 0x30D2
#endif
#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR            0x00000040
#endif
#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif
#ifndef EGL_Y_INVERTED_NOK
# define EGL_Y_INVERTED_NOK 0x307F
#endif
#ifndef EGL_OPENGL_ES3_BIT
# define EGL_OPENGL_ES3_BIT 0x00000040
#endif
#ifndef EGL_WAYLAND_Y_INVERTED_WL
# define EGL_WAYLAND_Y_INVERTED_WL 0x31DB
#endif
#ifndef EGL_PLATFORM_X11_KHR
# define EGL_PLATFORM_X11_KHR 0x31D5
#endif
#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif

#ifndef GL_COLOR_BUFFER_BIT0_QCOM
// if GL_COLOR_BUFFER_BIT0_QCOM  just assume the rest arent... saves fluff
#define GL_COLOR_BUFFER_BIT0_QCOM                     0x00000001
#define GL_COLOR_BUFFER_BIT1_QCOM                     0x00000002
#define GL_COLOR_BUFFER_BIT2_QCOM                     0x00000004
#define GL_COLOR_BUFFER_BIT3_QCOM                     0x00000008
#define GL_COLOR_BUFFER_BIT4_QCOM                     0x00000010
#define GL_COLOR_BUFFER_BIT5_QCOM                     0x00000020
#define GL_COLOR_BUFFER_BIT6_QCOM                     0x00000040
#define GL_COLOR_BUFFER_BIT7_QCOM                     0x00000080
#define GL_DEPTH_BUFFER_BIT0_QCOM                     0x00000100
#define GL_DEPTH_BUFFER_BIT1_QCOM                     0x00000200
#define GL_DEPTH_BUFFER_BIT2_QCOM                     0x00000400
#define GL_DEPTH_BUFFER_BIT3_QCOM                     0x00000800
#define GL_DEPTH_BUFFER_BIT4_QCOM                     0x00001000
#define GL_DEPTH_BUFFER_BIT5_QCOM                     0x00002000
#define GL_DEPTH_BUFFER_BIT6_QCOM                     0x00004000
#define GL_DEPTH_BUFFER_BIT7_QCOM                     0x00008000
#define GL_STENCIL_BUFFER_BIT0_QCOM                   0x00010000
#define GL_STENCIL_BUFFER_BIT1_QCOM                   0x00020000
#define GL_STENCIL_BUFFER_BIT2_QCOM                   0x00040000
#define GL_STENCIL_BUFFER_BIT3_QCOM                   0x00080000
#define GL_STENCIL_BUFFER_BIT4_QCOM                   0x00100000
#define GL_STENCIL_BUFFER_BIT5_QCOM                   0x00200000
#define GL_STENCIL_BUFFER_BIT6_QCOM                   0x00400000
#define GL_STENCIL_BUFFER_BIT7_QCOM                   0x00800000
#define GL_MULTISAMPLE_BUFFER_BIT0_QCOM               0x01000000
#define GL_MULTISAMPLE_BUFFER_BIT1_QCOM               0x02000000
#define GL_MULTISAMPLE_BUFFER_BIT2_QCOM               0x04000000
#define GL_MULTISAMPLE_BUFFER_BIT3_QCOM               0x08000000
#define GL_MULTISAMPLE_BUFFER_BIT4_QCOM               0x10000000
#define GL_MULTISAMPLE_BUFFER_BIT5_QCOM               0x20000000
#define GL_MULTISAMPLE_BUFFER_BIT6_QCOM               0x40000000
#define GL_MULTISAMPLE_BUFFER_BIT7_QCOM               0x80000000
#endif

#ifndef GL_EXT_discard_framebuffer
#define GL_COLOR_EXT   0x1800
#define GL_DEPTH_EXT   0x1801
#define GL_STENCIL_EXT 0x1802
#endif

#if defined(GL_GLES) && !defined(EGL_VERSION_1_5)
typedef intptr_t EGLAttrib;
#endif

#endif
