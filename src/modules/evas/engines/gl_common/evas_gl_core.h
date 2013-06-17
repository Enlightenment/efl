#ifndef _EVAS_GL_CORE_H
#define _EVAS_GL_CORE_H
#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

typedef void *EVGLNative_Display;
typedef void *EVGLNative_Window;
typedef void *EVGLNative_Surface;
typedef void *EVGLNative_Context;
typedef struct _EVGL_Engine         EVGL_Engine;
typedef struct _EVGL_Interface      EVGL_Interface;
typedef struct _EVGL_Surface        EVGL_Surface;
typedef struct _EVGL_Native_Window  EVGL_Native_Window;
typedef struct _EVGL_Context        EVGL_Context;
typedef struct _EVGL_Resource       EVGL_Resource;
typedef struct _EVGL_Cap            EVGL_Cap;
typedef struct _EVGL_Surface_Cap    EVGL_Surface_Cap;
typedef struct _EVGL_Surface_Format EVGL_Surface_Format;


extern EVGL_Engine *evgl_engine_init(void *eng_data, EVGL_Interface *efunc);
extern void         evgl_engine_shutdown(void *eng_data);

extern void        *evgl_surface_create(void *eng_data, Evas_GL_Config *cfg, int w, int h);
extern int          evgl_surface_destroy(void *eng_data, EVGL_Surface *sfc);
extern void        *evgl_context_create(void *eng_data, EVGL_Context *share_ctx);
extern int          evgl_context_destroy(void *eng_data, EVGL_Context *ctx);
extern int          evgl_make_current(void *eng_data, EVGL_Surface *sfc, EVGL_Context *ctx);

extern const char  *evgl_string_query(int name);
extern void        *evgl_proc_address_get(const char *name);
extern int          evgl_native_surface_get(EVGL_Surface *sfc, Evas_Native_Surface *ns);
extern Evas_GL_API *evgl_api_get();
extern int          evgl_direct_rendered();
extern void         evgl_direct_img_obj_set(Evas_Object *img, int rot);
extern Evas_Object *evgl_direct_img_obj_get();

extern void         evgl_direct_img_clip_set(int c, int x, int y, int w, int h);
extern void         evgl_direct_override_get(int *override, int *force_off);

#endif //_EVAS_GL_CORE_H
