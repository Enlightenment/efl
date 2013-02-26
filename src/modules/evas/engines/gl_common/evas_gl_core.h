#ifndef _EVAS_GL_CORE_H
#define _EVAS_GL_CORE_H
#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

typedef void *EVGLNative_Display;
typedef void *EVGLNative_Surface;
typedef void *EVGLNative_Context;
typedef struct _EVGL_Engine         EVGL_Engine;
typedef struct _EVGL_Interface      EVGL_Interface;
typedef struct _EVGL_Surface        EVGL_Surface;
typedef struct _EVGL_Native_Surface EVGL_Native_Surface;
typedef struct _EVGL_Context        EVGL_Context;
typedef struct _EVGL_Resource       EVGL_Resource;
typedef struct _EVGL_Cap            EVGL_Cap;
typedef struct _EVGL_Surface_Cap    EVGL_Surface_Cap;
typedef struct _EVGL_Surface_Format EVGL_Surface_Format;
typedef struct _Native_Surface      Native_Surface;


extern EVGL_Engine *evgl_engine_create(EVGL_Interface *efunc, void *engine_data);
extern int          evgl_engine_destroy(EVGL_Engine *ee);

extern void        *evgl_surface_create(EVGL_Engine *ee, Evas_GL_Config *cfg, int w, int h);
extern int          evgl_surface_destroy(EVGL_Engine *ee, EVGL_Surface *sfc);
extern void        *evgl_context_create(EVGL_Engine *ee, EVGL_Context *share_ctx);
extern int          evgl_context_destroy(EVGL_Engine *ee, EVGL_Context *ctx);
extern int          evgl_make_current(EVGL_Engine *ee, EVGL_Surface *sfc, EVGL_Context *ctx);
extern const char  *evgl_string_query(EVGL_Engine *ee, int name);
extern void        *evgl_proc_address_get(const char *name);
extern int          evgl_native_surface_get(EVGL_Engine *ee, EVGL_Surface *sfc, Evas_Native_Surface *ns);
extern Evas_GL_API *evgl_api_get(EVGL_Engine *ee);

extern int          evgl_direct_rendered(EVGL_Engine *ee);
extern void         evgl_direct_img_obj_set(EVGL_Engine *ee, Evas_Object *img);
extern Evas_Object *evgl_direct_img_obj_get(EVGL_Engine *ee);


#endif //_EVAS_GL_CORE_H
