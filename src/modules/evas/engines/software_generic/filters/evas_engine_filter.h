#ifndef EVAS_ENGINE_FILTER_H
#define EVAS_ENGINE_FILTER_H

#include "evas_filter_private.h"
#include "draw.h"

typedef Eina_Bool (* Software_Filter_Func) (Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_blend_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_blur_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_bump_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_curve_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_displace_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_fill_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_mask_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_transform_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_grayscale_func_get(Evas_Filter_Command *cmd);
Software_Filter_Func eng_filter_inverse_color_func_get(Evas_Filter_Command *cmd);

#endif // EVAS_ENGINE_FILTER_H
