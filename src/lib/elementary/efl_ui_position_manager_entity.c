#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Efl_Ui.h>
#include "elm_priv.h"

#define MY_CLASS      EFL_UI_POSITION_MANAGER_ENTITY_CLASS

#include "efl_ui_position_manager_entity.eo.c"
#include "efl_ui_position_manager_data_access_v1.eo.c"
