#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"



typedef struct Colorclass
{
   Eina_Stringshare *name;
   struct {
     unsigned char r, g, b, a;
   } color[3];
   Eina_Stringshare *desc;
} Colorclass;

typedef struct Colorclass_UI
{
   Evas_Object *ly;
   Evas_Object *gl;
   Evas_Object *reset;
   Evas_Object *cs;
   Elm_Color_Class_Name_Cb cb;

   Colorclass *current; //actually Elm_Color_Overlay
   unsigned int num; //color[num]
   Eina_Bool changed : 1;
   Eina_Bool change_reset : 1;
   Eina_Bool exist : 1;
} Colorclass_UI;

static void
_colorclass_cc_update(Colorclass_UI *cc, int num)
{
   char buf[128];

   snprintf(buf, sizeof(buf), "elm_colorclass_color%d", num + 1);
   edje_color_class_set(buf,
                        cc->current->color[num].r, cc->current->color[num].g, cc->current->color[num].b, cc->current->color[num].a,
                        0, 0, 0, 0,
                        0, 0, 0, 0);
}

static void
_colorclass_select(void *data, Evas_Object *obj EINA_UNUSED, const char *sig, const char *src EINA_UNUSED)
{
   Colorclass_UI *cc = data;
   const char *s;

   s = sig + sizeof("elm,colorclass,select,") - 1;
   switch (s[0])
     {
      case '1':
        cc->num = 0;
        break;
      case '2':
        cc->num = 1;
        break;
      case '3':
        cc->num = 2;
        break;
     }
   elm_colorselector_color_set(cc->cs, cc->current->color[cc->num].r, cc->current->color[cc->num].g,
                               cc->current->color[cc->num].b, cc->current->color[cc->num].a);
}

static void
_colorclass_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Colorclass_UI *cc = data;

   elm_colorselector_color_get(cc->cs, (int*)&cc->current->color[cc->num].r, (int*)&cc->current->color[cc->num].g,
                               (int*)&cc->current->color[cc->num].b, (int*)&cc->current->color[cc->num].a);
   edje_color_class_set(cc->current->name,
                        cc->current->color[0].r, cc->current->color[0].g, cc->current->color[0].b, cc->current->color[0].a,
                        cc->current->color[1].r, cc->current->color[1].g, cc->current->color[1].b, cc->current->color[1].a,
                        cc->current->color[2].r, cc->current->color[2].g, cc->current->color[2].b, cc->current->color[2].a);
   edje_color_class_set("elm_colorclass_text",
                        cc->current->color[0].r, cc->current->color[0].g, cc->current->color[0].b, cc->current->color[0].a,
                        cc->current->color[1].r, cc->current->color[1].g, cc->current->color[1].b, cc->current->color[1].a,
                        cc->current->color[2].r, cc->current->color[2].g, cc->current->color[2].b, cc->current->color[2].a);
   _colorclass_cc_update(cc, cc->num);
   cc->change_reset = 0;
   cc->changed = 1;
}

static void
_colorclass_reset(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Colorclass_UI *cc = data;
   Colorclass color;

   if (!cc->current) return;
   edje_color_class_del(cc->current->name);
   edje_color_class_get(cc->current->name,
                        (int*)&color.color[0].r, (int*)&color.color[0].g, (int*)&color.color[0].b, (int*)&color.color[0].a,
                        (int*)&color.color[1].r, (int*)&color.color[1].g, (int*)&color.color[1].b, (int*)&color.color[1].a,
                        (int*)&color.color[2].r, (int*)&color.color[2].g, (int*)&color.color[2].b, (int*)&color.color[2].a);
   cc->changed = !!memcmp(color.color, cc->current->color, sizeof(color.color));
   cc->change_reset = 1;
   memcpy(cc->current->color, color.color, sizeof(color.color));
   edje_color_class_set("elm_colorclass_text",
                        color.color[0].r, color.color[0].g, color.color[0].b, color.color[0].a,
                        color.color[1].r, color.color[1].g, color.color[1].b, color.color[1].a,
                        color.color[2].r, color.color[2].g, color.color[2].b, color.color[2].a);
   elm_colorselector_color_set(cc->cs, color.color[cc->num].r, color.color[cc->num].g,
                               color.color[cc->num].b, color.color[cc->num].a);
   _colorclass_cc_update(cc, 0);
   _colorclass_cc_update(cc, 1);
   _colorclass_cc_update(cc, 2);
}

static void
_colorclass_activate(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Colorclass_UI *cc = data;
   Elm_Object_Item *it = event_info;
   Edje_Color_Class *ecc;
   Colorclass *lcc;
   Eina_List *l;

   ecc = elm_object_item_data_get(it);
   EINA_LIST_FOREACH(_elm_config->color_overlays, l, lcc)
     {
        if (!eina_streq(lcc->name, ecc->name)) continue;
        cc->current = lcc;
        break;
     }
   if (cc->current)
     {
        _elm_config->color_overlays = eina_list_remove_list(_elm_config->color_overlays, l);
        cc->exist = 1;
     }
   else
     {
        cc->current = malloc(sizeof(Elm_Color_Overlay));
        memcpy(cc->current, ecc, sizeof(Elm_Color_Overlay));
        cc->current->name = eina_stringshare_ref(ecc->name);
     }
   _colorclass_cc_update(cc, 0);
   _colorclass_cc_update(cc, 1);
   _colorclass_cc_update(cc, 2);
   edje_color_class_set("elm_colorclass_text",
                        cc->current->color[0].r, cc->current->color[0].g, cc->current->color[0].b, cc->current->color[0].a,
                        cc->current->color[1].r, cc->current->color[1].g, cc->current->color[1].b, cc->current->color[1].a,
                        cc->current->color[2].r, cc->current->color[2].g, cc->current->color[2].b, cc->current->color[2].a);
   elm_colorselector_color_set(cc->cs, cc->current->color[0].r, cc->current->color[0].g,
                               cc->current->color[0].b, cc->current->color[0].a);
   elm_layout_signal_emit(cc->ly, "elm,colors,show", "elm");
}

static void
_colorclass_apply(Colorclass_UI *cc)
{
   if (cc->changed && (!cc->change_reset))
     _elm_config->color_overlays = eina_list_prepend(_elm_config->color_overlays, cc->current);
   else
     {
        eina_stringshare_del(cc->current->name);
        free(cc->current);
     }
}

static void
_colorclass_save(Colorclass_UI *cc)
{
   if (cc->current)
     {
        _colorclass_apply(cc);
        if (cc->changed || (cc->exist && cc->change_reset))
          elm_config_save();
     }
}

static void
_colorclass_deactivate(void *data, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   Colorclass_UI *cc = data;

   _colorclass_save(cc);
   cc->exist = cc->change_reset = cc->changed = 0;
   cc->current = NULL;
   cc->num = 0;
   elm_layout_signal_emit(cc->ly, "elm,colors,hide", "elm");
   elm_genlist_item_selected_set(elm_genlist_selected_item_get(cc->gl), 0);
}

static void
_colorclass_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Colorclass_UI *cc = data;

   _colorclass_save(cc);
   free(cc);
}

static char *
_colorclass_text_get(Edje_Color_Class *ecc, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   if (ecc->desc) return strdup(ecc->desc);
   return strdup(ecc->name ?: "");
}

static void
_colorclass_item_del(Edje_Color_Class *ecc, Evas_Object *obj EINA_UNUSED)
{
   eina_stringshare_del(ecc->name);
   eina_stringshare_del(ecc->desc);
   free(ecc);
}

EAPI Evas_Object *
elm_color_class_editor_add(Evas_Object *obj, Elm_Color_Class_Name_Cb cb)
{
   Evas_Object *ly, *bt, *gl, *cs;
   Colorclass_UI *cc;
   Edje_Color_Class *ecc, *ecc2;
   Eina_Iterator *it;
   Eina_List *ccs = NULL;
   static Elm_Genlist_Item_Class itc =
   {
      .item_style = "default",
      .func = {
           .text_get = (Elm_Genlist_Item_Text_Get_Cb)_colorclass_text_get,
           .del = (Elm_Genlist_Item_Del_Cb)_colorclass_item_del
      },
      .version = ELM_GENLIST_ITEM_CLASS_VERSION
   };

   cc = calloc(1, sizeof(Colorclass_UI));
   if (!cc) return NULL;
   it = edje_color_class_active_iterator_new();
   EINA_ITERATOR_FOREACH(it, ecc)
     {
        Colorclass *lcc;

        ecc2 = malloc(sizeof(Edje_Color_Class));
        memcpy(ecc2, ecc, sizeof(Edje_Color_Class));
        ecc2->name = eina_stringshare_add(ecc->name);
        if (cb)
          ecc2->desc = eina_stringshare_add(cb((char*)ecc->desc));
        else
          ecc2->desc = eina_stringshare_add(ecc->desc);
        lcc = (Colorclass*)ecc;
        edje_color_class_set(lcc->name,
                        lcc->color[0].r, lcc->color[0].g, lcc->color[0].b, lcc->color[0].a,
                        lcc->color[1].r, lcc->color[1].g, lcc->color[1].b, lcc->color[1].a,
                        lcc->color[2].r, lcc->color[2].g, lcc->color[2].b, lcc->color[2].a);
        ccs = eina_list_append(ccs, ecc2);
     }
   eina_iterator_free(it);
   cc->ly = ly = elm_layout_add(obj);
   elm_layout_theme_set(ly, "layout", "colorclass", "base");
   elm_layout_signal_callback_add(ly, "elm,colorclass,select,*", "elm", _colorclass_select, cc);
   elm_layout_signal_callback_add(ly, "elm,colorclass,deactivate", "elm", _colorclass_deactivate, cc);
   evas_object_event_callback_add(ly, EVAS_CALLBACK_DEL, _colorclass_del, cc);

   cc->gl = gl = elm_genlist_add(ly);
   elm_genlist_homogeneous_set(gl, 1);
   elm_scroller_bounce_set(gl, 0, 0);
   elm_scroller_policy_set(gl, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
   elm_genlist_mode_set(gl, ELM_LIST_COMPRESS);
   elm_object_part_content_set(ly, "elm.swallow.list", gl);
   evas_object_smart_callback_add(gl, "selected", _colorclass_activate, cc);

   cc->reset = bt = elm_button_add(ly);
   elm_object_style_set(bt, "colorclass");
   elm_object_text_set(bt, "Reset");
   elm_object_part_content_set(ly, "elm.swallow.reset", bt);
   evas_object_smart_callback_add(bt, "clicked", _colorclass_reset, cc);

   cc->cs = cs = elm_colorselector_add(ly);
   elm_colorselector_mode_set(cs, ELM_COLORSELECTOR_COMPONENTS);
   elm_object_part_content_set(ly, "elm.swallow.colors", cs);
   evas_object_smart_callback_add(cs, "changed,user", _colorclass_changed, cc);

   EINA_LIST_FREE(ccs, ecc)
     elm_genlist_item_append(gl, &itc, ecc, NULL, 0, NULL, NULL);

   return ly;
}
