#include "elm_test_focus_common.h"

#define Q(o,_x,_y,_w,_h) \
  do {\
   Eina_Rectangle rect = EINA_RECTANGLE_INIT; \
   rect.x = _x; \
   rect.y = _y; \
   rect.w = _w; \
   rect.h = _h; \
   focus_test_size(o, rect); \
  } while (0)

Efl_Ui_Focus_Object*
elm_focus_test_object_new(const char *name, int x, int y, int w, int h)
{
   Efl_Ui_Focus_Object *ret;

   ret = efl_add(FOCUS_TEST_CLASS, NULL,
    efl_name_set(efl_added, name)
   );
   Q(ret, x, y, w, h);

   return ret;
}

void
elm_focus_test_setup_cross(Efl_Ui_Focus_Object **middle,
    Efl_Ui_Focus_Object **south,
    Efl_Ui_Focus_Object **north,
    Efl_Ui_Focus_Object **east,
    Efl_Ui_Focus_Object **west)
  {

   *middle = elm_focus_test_object_new("middle", 40, 40, 20, 20);
   *south = elm_focus_test_object_new("south", 40, 80, 20, 20);
   *north = elm_focus_test_object_new("north", 40, 0, 20, 20);
   *east = elm_focus_test_object_new("east", 80, 40, 20, 20);
   *west = elm_focus_test_object_new("west", 0, 40, 20, 20);
}


Efl_Ui_Focus_Manager*
elm_focus_test_manager_new(Efl_Ui_Focus_Object **middle)
{
   Efl_Ui_Focus_Object *root;
   Efl_Ui_Focus_Manager *m;

   root = elm_focus_test_object_new("middle", 40, 40, 20, 20);
   m = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, NULL,
     efl_ui_focus_manager_root_set(efl_added, root)
   );
   if (middle)
     *middle = root;

   return m;
}

//Test class implementation

typedef struct {
    Eina_Rectangle rect;
    Eina_Bool focus;
} Focus_Test_Data;

EOLIAN static Efl_Object*
_focus_test_efl_object_constructor(Eo *obj, Focus_Test_Data *pd)
{
   Eo *eo;

   eo = efl_constructor(efl_super(obj, FOCUS_TEST_CLASS));
   eina_rectangle_coords_from(&pd->rect, 0, 0, 0, 0);
   return eo;
}

EOLIAN static void
_focus_test_efl_ui_focus_object_focus_set(Eo *obj, Focus_Test_Data *pd, Eina_Bool focus)
{
   pd->focus = focus;
   printf("Object %p now focused\n", obj);
}

EOLIAN static void
_focus_test_efl_ui_focus_object_geometry_get(Eo *obj EINA_UNUSED, Focus_Test_Data *pd, Eina_Rectangle *rect)
{
   if (!rect) return;

   *rect = pd->rect;
}

EOLIAN static void
_focus_test_size(Eo *obj EINA_UNUSED, Focus_Test_Data *pd, Eina_Rectangle rect)
{
   pd->rect = rect;
}

#include "focus_test.eo.c"
