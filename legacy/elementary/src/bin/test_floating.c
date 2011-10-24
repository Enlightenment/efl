#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
typedef struct _Testitem
{
   Elm_Genlist_Item *item;
   int mode;
   int onoff;
} Testitem;


static Elm_Genlist_Item_Class itc1;
static char *glf_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(long)data);
   return strdup(buf);
}

static Evas_Object *glf_content_get(void *data __UNUSED__, Evas_Object *obj, const char *part __UNUSED__)
{
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}
static Eina_Bool glf_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}
static void glf_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

static void
gl_sel(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on genlist obj [%p], item pointer [%p]\n", data, obj, event_info);
}

static Eina_Bool
anim(void *data)
{
   Evas_Object *gl = data;
   Evas_Coord x, y;

   y = 0;
   x = (sin(ecore_loop_time_get()) * 500);
   evas_object_move(gl, x, y);
   return ECORE_CALLBACK_RENEW;
}

static void
_del(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Animator *ani = data;

   ecore_animator_del(ani);
}

void
test_floating(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl;
   int i;
   Ecore_Animator *ani;

   win = elm_win_add(NULL, "floating", ELM_WIN_BASIC);
   elm_win_title_set(win, "Floating");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   gl = elm_genlist_add(win);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_move(gl, 800, 0);
   evas_object_resize(gl, 480, 800);
   evas_object_show(gl);

   itc1.item_style     = "default";
   itc1.func.label_get = glf_label_get;
   itc1.func.content_get  = glf_content_get;
   itc1.func.state_get = glf_state_get;
   itc1.func.del       = glf_del;

   for (i = 0; i < 20; i++)
     {
        elm_genlist_item_append(gl, &itc1,
                                (void *)(long)i/* item data */,
                                NULL/* parent */,
                                ELM_GENLIST_ITEM_NONE,
                                gl_sel/* func */,
                                (void *)(long)(i * 10)/* func data */);
     }
   evas_object_resize(win, 480, 800);
   evas_object_show(win);

   ani = ecore_animator_add(anim, gl);
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _del, ani);
}
#endif
