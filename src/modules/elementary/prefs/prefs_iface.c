#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"

#ifndef EFL_BUILD
# define EFL_BUILD
#endif
#undef ELM_MODULE_HELPER_H
#include "private.h"

/* including declaration of each prefs item implementation iface struct */
#define PREFS_ADD(w_name) \
  extern const Elm_Prefs_Item_Iface prefs_##w_name##_impl;

#include "item_widgets.inc"

#undef PREFS_ADD

int _elm_prefs_log_dom = -1;

/* now building on array of those, to be put on a hash for lookup */
static Elm_Prefs_Item_Iface_Info _elm_prefs_item_widgets[] =
{
#define PREFS_ADD(w_name) \
  {"elm/" #w_name, &prefs_##w_name##_impl},

#include "item_widgets.inc"

#undef PREFS_ADD
   {NULL, NULL}
};

/* including declaration of each prefs page implementation iface struct */
#define PREFS_ADD(w_name) \
  extern const Elm_Prefs_Page_Iface prefs_##w_name##_impl;

#include "page_widgets.inc"

#undef PREFS_ADD

/* now building on array of those, to be put on a hash for lookup */
static Elm_Prefs_Page_Iface_Info _elm_prefs_page_widgets[] =
{
#define PREFS_ADD(w_name) \
  {"elm/" #w_name, &prefs_##w_name##_impl},

#include "page_widgets.inc"

#undef PREFS_ADD
   {NULL, NULL}
};

Eina_Bool
elm_prefs_page_item_value_set(Evas_Object *it,
                              const Elm_Prefs_Item_Iface *iface,
                              Eina_Bool val)
{
   Eina_Value value;

   if (!iface->value_set) return EINA_FALSE;

   if ((!eina_value_setup(&value, EINA_VALUE_TYPE_UCHAR)) ||
       (!eina_value_set(&value, val)))
     return EINA_FALSE;

   return iface->value_set(it, &value);
}

static Evas_Object *
_elm_prefs_page_box_add(Evas_Object *obj,
                        Evas_Object *it)
{
   Evas_Object *sbx;
   double align_x, align_y;

   evas_object_size_hint_align_get(it, &align_x, &align_y);

   sbx = elm_box_add(obj);
   elm_box_horizontal_set(sbx, EINA_TRUE);
   evas_object_size_hint_weight_set(sbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sbx, align_x, align_y);
   evas_object_data_set(obj, "sub_box", sbx);
   evas_object_show(sbx);

   return sbx;
}

static void
_elm_prefs_page_item_hints_set(Evas_Object *it,
                          const Elm_Prefs_Item_Iface *iface)
{
   if (iface && iface->expand_want && iface->expand_want(it))
     evas_object_size_hint_align_set(it, EVAS_HINT_FILL, EVAS_HINT_FILL);

   evas_object_size_hint_weight_set(it, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
}

static void
_elm_prefs_page_pack_setup(Evas_Object *it,
                               Evas_Object *obj,
                               const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l, *i, *sbx;

   _elm_prefs_page_item_hints_set(it, iface);

   l = evas_object_data_get(it, "label_widget");
   i = evas_object_data_get(it, "icon_widget");
   if (i)
     {
        sbx = _elm_prefs_page_box_add(obj, it);
        elm_box_pack_end(obj, sbx);

        evas_object_size_hint_align_set(it, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_align_set(i, 0.0, EVAS_HINT_FILL);
        elm_box_pack_end(sbx, i);
        elm_box_pack_end(sbx, it);

        if (l) elm_box_pack_before(obj, l, sbx);
     }
   else
     {
        elm_box_pack_end(obj, it);
        if (l) elm_box_pack_before(obj, l, it);
     }
}

static void
_elm_prefs_page_pack_before_setup(Evas_Object *it,
                                      Evas_Object *it_before,
                                      Evas_Object *obj,
                                      const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l, *i, *sbx;

   _elm_prefs_page_item_hints_set(it, iface);

   l = evas_object_data_get(it, "label_widget");
   i = evas_object_data_get(it, "icon_widget");
   if (i)
     {
        sbx = _elm_prefs_page_box_add(obj, it);
        elm_box_pack_before(obj, sbx, it_before);

        evas_object_size_hint_align_set(i, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(sbx, i);
        elm_box_pack_end(sbx, it);

        if (l) elm_box_pack_before(obj, l, sbx);
     }
   else
     {
        elm_box_pack_before(obj, it, it_before);
        if (l) elm_box_pack_before(obj, l, it);
     }
}

static void
_elm_prefs_page_pack_after_setup(Evas_Object *it,
                                     Evas_Object *it_after,
                                     Evas_Object *obj,
                                     const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l, *i, *sbx;

   _elm_prefs_page_item_hints_set(it, iface);

   l = evas_object_data_get(it, "label_widget");
   i = evas_object_data_get(it, "icon_widget");
   if (i)
     {
        sbx = _elm_prefs_page_box_add(obj, it);
        elm_box_pack_after(obj, sbx, it_after);

        evas_object_size_hint_align_set(i, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(sbx, i);
        elm_box_pack_end(sbx, it);

        if (l) elm_box_pack_before(obj, l, sbx);
     }
   else
     {
        elm_box_pack_after(obj, it, it_after);
        if (l) elm_box_pack_before(obj, l, it);
     }
}

void
elm_prefs_horizontal_page_common_pack(Evas_Object *it,
                                      Evas_Object *obj,
                                      const Elm_Prefs_Item_Iface *iface)
{
   evas_object_size_hint_align_set(it, 0.5, EVAS_HINT_FILL);
   _elm_prefs_page_pack_setup(it, obj, iface);
}

void
elm_prefs_horizontal_page_common_pack_before(Evas_Object *it,
                                             Evas_Object *it_before,
                                             Evas_Object *obj,
                                             const Elm_Prefs_Item_Iface *iface)
{
   evas_object_size_hint_align_set(it, 0.5, EVAS_HINT_FILL);
   _elm_prefs_page_pack_before_setup(it, it_before, obj, iface);
}

void
elm_prefs_horizontal_page_common_pack_after(Evas_Object *it,
                                            Evas_Object *it_after,
                                            Evas_Object *obj,
                                            const Elm_Prefs_Item_Iface *iface)
{
   evas_object_size_hint_align_set(it, 0.5, EVAS_HINT_FILL);
   _elm_prefs_page_pack_after_setup(it, it_after, obj, iface);
}

void
elm_prefs_vertical_page_common_pack(Evas_Object *it,
                                    Evas_Object *obj,
                                    const Elm_Prefs_Item_Iface *iface)
{
   evas_object_size_hint_align_set(it, EVAS_HINT_FILL, 0.5);
   _elm_prefs_page_pack_setup(it, obj, iface);
}

void
elm_prefs_vertical_page_common_pack_before(Evas_Object *it,
                                           Evas_Object *it_before,
                                           Evas_Object *obj,
                                           const Elm_Prefs_Item_Iface *iface)
{
   evas_object_size_hint_align_set(it, EVAS_HINT_FILL, 0.5);
   _elm_prefs_page_pack_before_setup(it, it_before, obj, iface);
}

void
elm_prefs_vertical_page_common_pack_after(Evas_Object *it,
                                          Evas_Object *it_after,
                                          Evas_Object *obj,
                                          const Elm_Prefs_Item_Iface *iface)
{
   evas_object_size_hint_align_set(it, EVAS_HINT_FILL, 0.5);
   _elm_prefs_page_pack_after_setup(it, it_after, obj, iface);
}

void
elm_prefs_page_common_unpack(Evas_Object *it,
                             Evas_Object *obj)
{
   Evas_Object *l, *i, *sbx;

   l = evas_object_data_get(it, "label_widget");
   if (l) elm_box_unpack(obj, l);

   sbx = evas_object_data_get(it, "sub_box");
   i = evas_object_data_get(it, "icon_widget");

   if (i && sbx)
     {
        elm_box_unpack_all(sbx);
        elm_box_unpack(obj, sbx);
        evas_object_del(sbx);
     }
   else
     elm_box_unpack(obj, it);
}

EAPI int
elm_modapi_init(void *m EINA_UNUSED)
{
   _elm_prefs_log_dom = eina_log_domain_register
       ("elm-prefs", EINA_COLOR_YELLOW);

   elm_prefs_item_iface_register(_elm_prefs_item_widgets);
   elm_prefs_page_iface_register(_elm_prefs_page_widgets);

   return 1; // succeed always
}

EAPI int
elm_modapi_shutdown(void *m EINA_UNUSED)
{
   elm_prefs_item_iface_unregister(_elm_prefs_item_widgets);
   elm_prefs_page_iface_unregister(_elm_prefs_page_widgets);

   if (_elm_prefs_log_dom >= 0) eina_log_domain_unregister(_elm_prefs_log_dom);
   _elm_prefs_log_dom = -1;

   return 1; // succeed always
}
