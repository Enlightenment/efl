#ifndef GL_ENGINE_FILTER_H
#define GL_ENGINE_FILTER_H

#include "evas_filter_private.h"
#include "evas_gl_common.h"
#include "../evas_ector_gl.h"
#include "../Evas_Engine_GL_Generic.h"

extern int _evas_engine_GL_log_dom;

typedef Eina_Bool (* GL_Filter_Apply_Func) (Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_blend_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_blur_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
//GL_Filter_Apply_Func gl_filter_bump_func_get(Render_Engine_GL_Generic *reEvas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_curve_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_displace_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_fill_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_mask_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
//GL_Filter_Apply_Func gl_filter_transform_func_get(Evas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_grayscale_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_inverse_color_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);

#undef DBG
#undef INF
#undef WRN
#undef ERR
#undef CRI
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_GL_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_GL_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_GL_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_GL_log_dom, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_GL_log_dom, __VA_ARGS__)

#endif // GL_ENGINE_FILTER_H
