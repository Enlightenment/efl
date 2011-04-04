#ifndef _EVAS_GL_H
#define _EVAS_GL_H

#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Evas_GL               Evas_GL;
typedef struct _Evas_GL_Surface       Evas_GL_Surface;
typedef struct _Evas_GL_Context       Evas_GL_Context;
typedef struct _Evas_GL_Config        Evas_GL_Config;
typedef void*                         Evas_GL_Func;

typedef enum _Evas_GL_Color_Format
{
    EVAS_GL_RGB_8,      // 8 bits per channel
    EVAS_GL_RGBA_8,
    EVAS_GL_RGB_32,     // 32-bits per channel
    EVAS_GL_RGBA_32,
} Evas_GL_Color_Format;

typedef enum _Evas_GL_Depth_Bits
{
    EVAS_GL_DEPTH_BIT_8,
    EVAS_GL_DEPTH_BIT_16,
    EVAS_GL_DEPTH_BIT_24,
    EVAS_GL_DEPTH_BIT_32,
    EVAS_GL_DEPTH_NONE
} Evas_GL_Depth_Bits;

typedef enum _Evas_GL_Stencil_Bits
{
    EVAS_GL_STENCIL_BIT_1,
    EVAS_GL_STENCIL_BIT_2,
    EVAS_GL_STENCIL_BIT_4,
    EVAS_GL_STENCIL_BIT_8,
    EVAS_GL_STENCIL_BIT_16,
    EVAS_GL_STENCIL_NONE
} Evas_GL_Stencil_Bits;

struct _Evas_GL_Config
{
    Evas_GL_Color_Format     color_format;
    Evas_GL_Depth_Bits       depth_bits;
    Evas_GL_Stencil_Bits     stencil_bits;
};

/**
 * @defgroup Evas_GL group for rendering GL on Evas
 *
 * Functions that are used to do GL rendering on Evas.
 *
 * @ingroup Evas_Canvas
 */

   EAPI Evas_GL                 *evas_gl_new                (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
   EAPI void                     evas_gl_free               (Evas_GL *evas_gl) EINA_ARG_NONNULL(1, 2);;
   EAPI Evas_GL_Surface         *evas_gl_surface_create     (Evas_GL *evas_gl, Evas_GL_Config *cfg, int w, int h) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1,2);
   EAPI void                     evas_gl_surface_destroy    (Evas_GL *evas_gl, Evas_GL_Surface *surf) EINA_ARG_NONNULL(1,2); 
   EAPI Evas_GL_Context         *evas_gl_context_create     (Evas_GL *evas_gl, Evas_GL_Context *share_ctx) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1); 
   EAPI void                     evas_gl_context_destroy    (Evas_GL *evas_gl, Evas_GL_Context *ctx) EINA_ARG_NONNULL(1,2); 
   EAPI Eina_Bool                evas_gl_make_current       (Evas_GL *evas_gl, Evas_GL_Surface *surf, Evas_GL_Context *ctx) EINA_ARG_NONNULL(1,2);
   EAPI Evas_GL_Func             evas_gl_proc_address_get   (Evas_GL *evas_gl, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1,2) EINA_PURE;

   EAPI Eina_Bool                evas_gl_native_surface_get (Evas_GL *evas_gl, Evas_GL_Surface *surf, Evas_Native_Surface *ns) EINA_ARG_NONNULL(1,2,3);
 
//   EAPI Evas_GL                 *evas_gl_api_get            (Evas *e, Evas_GL_API *glapi) EINA_ARG_NONNULL(1, 2);

#ifdef __cplusplus
}
#endif

#endif
