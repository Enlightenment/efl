#ifndef EFL_UI_VIEW_PRIVATE_H
#define EFL_UI_VIEW_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_View_Data Efl_Ui_View_Data;

struct _Efl_Ui_View_Data
{
   Efl_Model *model;
};
#endif
