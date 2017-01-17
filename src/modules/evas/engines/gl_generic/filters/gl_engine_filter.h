#ifndef GL_ENGINE_FILTER_H
#define GL_ENGINE_FILTER_H

#include "evas_filter_private.h"
#include "evas_gl_common.h"
#include "evas_ector_buffer.eo.h"
#include "../Evas_Engine_GL_Generic.h"

typedef Eina_Bool (* GL_Filter_Apply_Func) (Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd);
GL_Filter_Apply_Func gl_filter_blend_func_get(Evas_Filter_Command *cmd);
//Software_Filter_Func gl_filter_blur_func_get(Evas_Filter_Command *cmd);
//Software_Filter_Func gl_filter_bump_func_get(Evas_Filter_Command *cmd);
//Software_Filter_Func gl_filter_curve_func_get(Evas_Filter_Command *cmd);
//Software_Filter_Func gl_filter_displace_func_get(Evas_Filter_Command *cmd);
//Software_Filter_Func gl_filter_fill_func_get(Evas_Filter_Command *cmd);
//Software_Filter_Func gl_filter_mask_func_get(Evas_Filter_Command *cmd);
//Software_Filter_Func gl_filter_transform_func_get(Evas_Filter_Command *cmd);

#endif // GL_ENGINE_FILTER_H
