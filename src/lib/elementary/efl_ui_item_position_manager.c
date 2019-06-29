#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Efl_Ui.h>
#include "elm_priv.h"

#define MY_CLASS      EFL_UI_ITEM_POSITION_MANAGER_CLASS

#include "efl_ui_item_position_manager.eo.c"
