#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static Elm_Genlist_Item_Class it_desk;

static char *
desk_gl_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
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
desk_gl_icon_get(void *data, Evas_Object *obj, const char *part)
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
desk_gl_del(void *data, Evas_Object *obj __UNUSED__)
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
desktop_sel(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("sel\n");
}
#endif

void
test_icon_desktops(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl;
#ifdef ELM_EFREET
   Eina_List *desktops;
#endif

   win = elm_win_add(NULL, "icon-desktops", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon Desktops");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   it_desk.item_style     = "default";
   it_desk.func.label_get = desk_gl_label_get;
   it_desk.func.icon_get  = desk_gl_icon_get;
   it_desk.func.state_get = NULL;
   it_desk.func.del       = desk_gl_del;

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
             elm_genlist_item_append(gl, &it_desk, d,
                                    NULL, ELM_GENLIST_ITEM_NONE,
                                    desktop_sel, NULL);
         }
     }
#endif

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
