#ifndef EVAS_ENGINE_FILTER_H
#define EVAS_ENGINE_FILTER_H

#include "evas_filter_private.h"
#include "draw.h"

Evas_Filter_Apply_Func eng_filter_blend_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func eng_filter_bump_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func eng_filter_curve_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func eng_filter_displace_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func eng_filter_fill_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func eng_filter_mask_func_get(Evas_Filter_Command *cmd);
Evas_Filter_Apply_Func eng_filter_transform_func_get(Evas_Filter_Command *cmd);

#endif // EVAS_ENGINE_FILTER_H
