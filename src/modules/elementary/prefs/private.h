#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include "Elementary.h"

#undef CRI
#undef ERR
#undef WRN
#undef INF
#undef DBG

#define CRI(...)      EINA_LOG_DOM_CRIT(_elm_prefs_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR (_elm_prefs_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_elm_prefs_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_elm_prefs_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG (_elm_prefs_log_dom, __VA_ARGS__)

extern int _elm_prefs_log_dom;

#define PREFS_ITEM_WIDGET_ADD(_w_name,                                     \
                              _types,                                      \
                              _value_set_fn,                               \
                              _value_get_fn,                               \
                              _value_validate_fn,                          \
                              _label_set_fn,                               \
                              _icon_set_fn,                                \
                              _editable_set_fn,                            \
                              _editable_get_fn,                            \
                              _expand_want_fn)                             \
  static Evas_Object *                                                     \
  _prefs_##_w_name##_add(const Elm_Prefs_Item_Iface * iface,               \
                         Evas_Object * prefs,                              \
                         const Elm_Prefs_Item_Type type,                   \
                         const Elm_Prefs_Item_Spec spec,                   \
                         Elm_Prefs_Item_Changed_Cb it_changed_cb)          \
  {                                                                        \
     Evas_Object *obj =                                                    \
       elm_prefs_##_w_name##_add(iface, prefs, type, spec, it_changed_cb); \
     if (!obj) return NULL;                                                \
     if (!elm_prefs_item_widget_common_add(prefs, obj))                    \
       {                                                                   \
          evas_object_del(obj);                                            \
          return NULL;                                                     \
       }                                                                   \
                                                                           \
     return obj;                                                           \
  }                                                                        \
                                                                           \
  const Elm_Prefs_Item_Iface prefs_##_w_name##_impl =                      \
  {                                                                        \
     .abi_version = ELM_PREFS_ITEM_IFACE_ABI_VERSION,                      \
     .add = _prefs_##_w_name##_add,                                        \
     .types = _types,                                                      \
     .value_set = _value_set_fn,                                           \
     .value_get = _value_get_fn,                                           \
     .value_validate = _value_validate_fn,                                 \
     .label_set = _label_set_fn,                                           \
     .icon_set = _icon_set_fn,                                             \
     .editable_set = _editable_set_fn,                                     \
     .editable_get = _editable_get_fn,                                     \
     .expand_want = _expand_want_fn                                        \
  }

#define PREFS_PAGE_WIDGET_ADD(_w_name,                       \
                              _title_set_fn,                 \
                              _sub_title_set_fn,             \
                              _icon_set_fn,                  \
                              _item_pack_fn,                 \
                              _item_unpack_fn,               \
                              _item_pack_before_fn,          \
                              _item_pack_after_fn)           \
  static Evas_Object *                                       \
  _prefs_##_w_name##_add(const Elm_Prefs_Page_Iface * iface, \
                         Evas_Object * prefs)                \
  {                                                          \
     Evas_Object *obj =                                      \
       elm_prefs_##_w_name##_add(iface, prefs);              \
     if (!obj) return NULL;                                  \
     if (!elm_prefs_page_widget_common_add(prefs, obj))      \
       {                                                     \
          evas_object_del(obj);                              \
          return NULL;                                       \
       }                                                     \
                                                             \
     return obj;                                             \
  }                                                          \
                                                             \
  const Elm_Prefs_Page_Iface prefs_##_w_name##_impl =        \
  {                                                          \
     .abi_version = ELM_PREFS_PAGE_IFACE_ABI_VERSION,        \
     .add = _prefs_##_w_name##_add,                          \
     .title_set = _title_set_fn,                             \
     .sub_title_set = _sub_title_set_fn,                     \
     .icon_set = _icon_set_fn,                               \
     .item_pack = _item_pack_fn,                             \
     .item_unpack = _item_unpack_fn,                         \
     .item_pack_before = _item_pack_before_fn,               \
     .item_pack_after = _item_pack_after_fn                  \
  }

Eina_Bool elm_prefs_page_item_value_set(Evas_Object *it,
                                        const Elm_Prefs_Item_Iface *iface,
                                        Eina_Bool val);
void elm_prefs_horizontal_page_common_pack(Evas_Object *,
                                           Evas_Object *,
                                           const Elm_Prefs_Item_Iface *);
void elm_prefs_horizontal_page_common_pack_before(Evas_Object *,
                                                  Evas_Object *,
                                                  Evas_Object *,
                                                  const Elm_Prefs_Item_Iface *);
void elm_prefs_horizontal_page_common_pack_after(Evas_Object *,
                                                 Evas_Object *,
                                                 Evas_Object *,
                                                 const Elm_Prefs_Item_Iface *);
void elm_prefs_vertical_page_common_pack(Evas_Object *,
                                         Evas_Object *,
                                         const Elm_Prefs_Item_Iface *);
void elm_prefs_vertical_page_common_pack_before(Evas_Object *,
                                                Evas_Object *,
                                                Evas_Object *,
                                                const Elm_Prefs_Item_Iface *);
void elm_prefs_vertical_page_common_pack_after(Evas_Object *,
                                               Evas_Object *,
                                               Evas_Object *,
                                               const Elm_Prefs_Item_Iface *);
void elm_prefs_page_common_unpack(Evas_Object *,
                                  Evas_Object *);
