#ifndef ELM_TEST_FOCUS_COMMON_H
#define ELM_TEST_FOCUS_COMMON_H
#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "focus_test.eo.h"
#include "focus_manager_test.eo.h"

#define TEST_OBJ_NEW(name, x, y, w, h) \
  Efl_Ui_Focus_Object* name; \
  name = focus_test_object_new("" #name "",x, y, w, h); \


Efl_Ui_Focus_Object* focus_test_object_new(const char *name, int x, int y, int w, int h);

void focus_test_setup_cross(Efl_Ui_Focus_Object **middle,
                                Efl_Ui_Focus_Object **south,
                                Efl_Ui_Focus_Object **north,
                                Efl_Ui_Focus_Object **east,
                                Efl_Ui_Focus_Object **west);

Efl_Ui_Focus_Manager *focus_test_manager_new(Efl_Ui_Focus_Object **middle, Eina_Bool);
#endif
