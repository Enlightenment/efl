#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>

static Eina_Value
_deliverty_cb(void *data, const Eina_Value value, const Eina_Future *dead_future EINA_UNUSED)
{
   Ecore_Evas_Selection_Buffer buffer = (intptr_t)data;
   Eina_Content *content = eina_value_to_content(&value);

   printf("Got Content of selection %d with type %s\n", buffer, eina_content_type_get(content));
   if (!strncmp(eina_content_type_get(content), "text", strlen("text")))
     {
        printf("Content: %s\n", (char*)eina_content_data_get(content).mem);
     }

   return EINA_VALUE_EMPTY;
}

static void
_selection_changed(Ecore_Evas *ee, unsigned int seat EINA_UNUSED, Ecore_Evas_Selection_Buffer selection)
{
   printf("Selection %d of %p has changed\n", selection, ee);
}

static void
_motion_cb(Ecore_Evas *ee, unsigned int seat EINA_UNUSED, Eina_Position2D p)
{
   printf("Drag and Drop has moved on the window %p (%d, %d)\n", ee, p.x, p.y);
}

static void
_enter_state_change_cb(Ecore_Evas *ee, unsigned int sea EINA_UNUSED, Eina_Position2D p, Eina_Bool inside)
{
   if (inside)
     printf("Drag and Drop has entered the window %p (%d, %d)\n", ee, p.x, p.y);
   else
     printf("Drag and Drop has left the window %p (%d, %d)\n", ee, p.x, p.y);
}

static void
_drop_cb(Ecore_Evas *ee, unsigned int seat EINA_UNUSED, Eina_Position2D p, const char *action EINA_UNUSED)
{
   printf("Drag and Drop has droped on the window %p (%d, %d)\n", ee, p.x, p.y);
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   const char *goal;
   if (argc < 2)
     {
        printf("Error, argc must be 2\n");
        return -1;
     }
   goal = argv[1];

   ecore_evas_init();

   Ecore_Evas *ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 0, 0);

   if (eina_streq(goal, "--monitor"))
     {
        ecore_evas_callback_selection_changed_set(ee, _selection_changed);
        ecore_evas_callback_drop_drop_set(ee, _drop_cb);
        ecore_evas_callback_drop_motion_set(ee, _motion_cb);
        ecore_evas_callback_drop_state_changed_set(ee, _enter_state_change_cb);
     }
   else if (eina_streq(goal, "--show-selections"))
     {
        const char *types[] = {eina_stringshare_add("text/plain")};
        for (int i = 0; i < ECORE_EVAS_SELECTION_BUFFER_LAST; ++i)
          {
             Eina_Future *future = ecore_evas_selection_get(ee, 0,  i, EINA_C_ARRAY_ITERATOR_NEW(types));
             eina_future_then(future, _deliverty_cb, .data = ((void*)(intptr_t)i));
          }
     }
   else if (eina_streq(goal, "--set-selection"))
     {
        if (argc != 3)
          {
             printf("Error, --set-selection only requires exactly 1 keyword (The selection to set).\n");
          }
        Eina_Content *content = eina_content_new((Eina_Slice)EINA_SLICE_STR_FULL(argv[2]), "text/plain");
        ecore_evas_selection_set(ee, 0, ECORE_EVAS_SELECTION_BUFFER_COPY_AND_PASTE_BUFFER, content);
     }
   else if (eina_streq(goal, "--show-owner"))
     {
        for (int i = 0; i < ECORE_EVAS_SELECTION_BUFFER_LAST; ++i)
          {
             printf("Selection buffer %d : %d\n", i, ecore_evas_selection_exists(ee, 0, i));
          }
     }
   else
     {
        printf("Error, goal %s not found\n", goal);
     }

   ecore_main_loop_begin();

   ecore_evas_shutdown();
}
