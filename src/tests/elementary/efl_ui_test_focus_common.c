#define EFL_UI_FOCUS_OBJECT_PROTECTED

#include "efl_ui_test_focus_common.h"

#define Q(o,_x,_y,_w,_h) \
  do {\
   Eina_Rect rect; \
   rect.x = _x; \
   rect.y = _y; \
   rect.w = _w; \
   rect.h = _h; \
   focus_test_size(o, rect); \
  } while (0)

Efl_Ui_Focus_Object*
focus_test_object_new(const char *name, int x, int y, int w, int h)
{
   Efl_Ui_Focus_Object *ret;

   ret = efl_add_ref(FOCUS_TEST_CLASS, NULL,
    efl_name_set(efl_added, name)
   );
   Q(ret, x, y, w, h);

   return ret;
}

void
focus_test_setup_cross(Efl_Ui_Focus_Object **middle,
    Efl_Ui_Focus_Object **south,
    Efl_Ui_Focus_Object **north,
    Efl_Ui_Focus_Object **east,
    Efl_Ui_Focus_Object **west)
  {

   *middle = focus_test_object_new("middle", 40, 40, 20, 20);
   *south = focus_test_object_new("south", 40, 80, 20, 20);
   *north = focus_test_object_new("north", 40, 0, 20, 20);
   *east = focus_test_object_new("east", 80, 40, 20, 20);
   *west = focus_test_object_new("west", 0, 40, 20, 20);
}


Efl_Ui_Focus_Manager*
focus_test_manager_new(Efl_Ui_Focus_Object **middle)
{
   Efl_Ui_Focus_Object *root;
   Efl_Ui_Focus_Manager *m;

   root = focus_test_object_new("middle", 40, 40, 20, 20);
   m = efl_add_ref(EFL_UI_FOCUS_MANAGER_CALC_CLASS, NULL,
     efl_ui_focus_manager_root_set(efl_added, root)
   );
   if (middle)
     *middle = root;

   return m;
}

//Test class implementation

typedef struct {
    Eina_Rect rect;
    Eina_Bool focus;
    Eo *manager;
} Focus_Test_Data;

EOLIAN static Efl_Object*
_focus_test_efl_object_constructor(Eo *obj, Focus_Test_Data *pd)
{
   Eo *eo;

   eo = efl_constructor(efl_super(obj, FOCUS_TEST_CLASS));
   eina_rectangle_coords_from(&pd->rect.rect, 0, 0, 0, 0);
   return eo;
}

EOLIAN static void
_focus_test_efl_ui_focus_object_focus_set(Eo *obj, Focus_Test_Data *pd, Eina_Bool focus)
{
   pd->focus = focus;
   printf("Object %p now focused\n", obj);
   efl_ui_focus_object_focus_set(efl_super(obj, FOCUS_TEST_CLASS), focus);
}

EOLIAN static Eina_Rect
_focus_test_efl_ui_focus_object_focus_geometry_get(const Eo *obj EINA_UNUSED, Focus_Test_Data *pd)
{
   return pd->rect;
}

EOLIAN static void
_focus_test_test_size(Eo *obj EINA_UNUSED, Focus_Test_Data *pd, Eina_Rect rect)
{
   pd->rect = rect;
}

EOLIAN static Eina_Rect
_focus_test_efl_gfx_entity_geometry_get(const Eo *obj EINA_UNUSED, Focus_Test_Data *pd)
{
   return pd->rect;
}

EOLIAN static Efl_Ui_Focus_Manager*
_focus_test_efl_ui_focus_object_focus_manager_get(const Eo *obj EINA_UNUSED, Focus_Test_Data *pd)
{
   return pd->manager;
}

EOLIAN static Efl_Ui_Focus_Object*
_focus_test_efl_ui_focus_object_focus_parent_get(const Eo *obj, Focus_Test_Data *pd EINA_UNUSED)
{
   return efl_parent_get(obj);
}

EOLIAN static void
_focus_test_manager_set(Eo *obj EINA_UNUSED, Focus_Test_Data *pd, Efl_Ui_Focus_Manager *manager)
{
  pd->manager = manager;
}


#include "focus_test.eo.c"
