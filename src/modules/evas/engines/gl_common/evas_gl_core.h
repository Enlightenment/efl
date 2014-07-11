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

EAPI void         evgl_engine_shutdown(void *eng_data);

typedef void (*EVGL_Engine_Call)(void *eng_data);

EVGL_Engine *evgl_engine_init(void *eng_data, const EVGL_Interface *efunc);

void        *evgl_surface_create(void *eng_data, Evas_GL_Config *cfg, int w, int h);
int          evgl_surface_destroy(void *eng_data, EVGL_Surface *sfc);
void        *evgl_context_create(void *eng_data, EVGL_Context *share_ctx);
int          evgl_context_destroy(void *eng_data, EVGL_Context *ctx);
int          evgl_make_current(void *eng_data, EVGL_Surface *sfc, EVGL_Context *ctx);

const char  *evgl_string_query(int name);
void        *evgl_proc_address_get(const char *name);
int          evgl_native_surface_get(EVGL_Surface *sfc, Evas_Native_Surface *ns);
Evas_GL_API *evgl_api_get();

int          evgl_direct_rendered();
void         evgl_direct_override_get(int *override, int *force_off);
void         evgl_direct_info_set(int win_w, int win_h, int rot,
                                  int img_x, int img_y, int img_w, int img_h,
                                  int clip_x, int clip_y, int clip_w, int clip_h);
void         evgl_direct_info_clear();

void         evgl_direct_partial_info_set(int pres);
void         evgl_direct_partial_info_clear();
void         evgl_direct_partial_render_start();
void         evgl_direct_partial_render_end();
#endif //_EVAS_GL_CORE_H
