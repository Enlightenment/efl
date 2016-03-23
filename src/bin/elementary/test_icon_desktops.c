#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static Elm_Genlist_Item_Class *it_desk = NULL;

static char *
desk_gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
#ifdef ELM_EFREET
   Efreet_Desktop *d = (Efreet_Desktop *)data;
   return strdup(d->name);
#else
   return NULL;
   (void)data;
#endif
}
static Evas_Object *
desk_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
#ifdef ELM_EFREET
   Efreet_Desktop *d = (Efreet_Desktop *)data;
   Evas_Object *ic;

   ic = elm_icon_add(obj);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   if (!(!strcmp(part, "elm.swallow.icon"))) return ic;
   if (!d->icon) return ic;
   elm_icon_standard_set(ic, d->icon);
   return ic;
#else
   return NULL;
   (void)data;
   (void)obj;
   (void)part;
#endif
}
static void
desk_gl_del(void *data, Evas_Object *obj EINA_UNUSED)
{
#ifdef ELM_EFREET
   Efreet_Desktop *d = (Efreet_Desktop *)data;
   efreet_desktop_free(d);
#else
   return;
   (void)data;
#endif
}

#ifdef ELM_EFREET
static void
desktop_sel(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efreet_Desktop *d = data;
   if (!d) return;
   printf("Selected Desktop Icon:\n");
   printf("\tname : %s\n", d->name);
   printf("\ticon : %s\n", d->icon);
   printf("\tgeneric_name : %s\n", d->generic_name);
}
#endif

void
test_icon_desktops(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *gl;
#ifdef ELM_EFREET
   Eina_List *desktops;
#endif

   win = elm_win_util_standard_add("icon-desktops", "Icon Desktops");
   elm_win_autodel_set(win, EINA_TRUE);

   it_desk = elm_genlist_item_class_new();
   it_desk->item_style     = "default";
   it_desk->func.text_get = desk_gl_text_get;
   it_desk->func.content_get  = desk_gl_content_get;
   it_desk->func.state_get = NULL;
   it_desk->func.del       = desk_gl_del;

   gl = elm_genlist_add(win);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gl);
   evas_object_show(gl);

#ifdef ELM_EFREET
   if (elm_need_efreet())
     {
       desktops = efreet_util_desktop_name_glob_list("*");
       if (desktops)
         {
           Efreet_Desktop *d;

           EINA_LIST_FREE(desktops, d)
             elm_genlist_item_append(gl, it_desk, d,
                                    NULL, ELM_GENLIST_ITEM_NONE,
                                    desktop_sel, d);
         }
     }
#endif

   elm_genlist_item_class_free(it_desk);
   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
