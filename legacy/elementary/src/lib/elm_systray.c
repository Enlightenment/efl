#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"

#include "Eo.h"

EAPI Eo_Op ELM_OBJ_SYSTRAY_BASE_ID = EO_NOOP;

#include "elm_systray_watcher.h"

EAPI int ELM_EVENT_SYSTRAY_READY = 0;

typedef struct _Elm_Systray_Private_Data
{
   Elm_Systray_Category cat;
   Elm_Systray_Status   status;

   const char           *att_icon_name;
   const char           *icon_name;
   const char           *icon_theme_path;
   const char           *id;
   const char           *title;
   const char           *menu;
   const Eo             *menu_obj;
} Elm_Systray_Private_Data;

static Elm_Systray_Private_Data _item = {
   .cat             = ELM_SYSTRAY_CATEGORY_APP_STATUS,
   .status          = ELM_SYSTRAY_STATUS_ACTIVE,
   .att_icon_name   = NULL,
   .icon_name       = NULL,
   .icon_theme_path = NULL,
   .id              = NULL,
   .title           = NULL,
   .menu            = NULL,
   .menu_obj        = NULL
};

#define OBJ_PATH  "/org/ayatana/NotificationItem/StatusNotifierItem"
#define INTERFACE "org.kde.StatusNotifierItem"

static Eina_Bool _elm_need_systray = EINA_FALSE;

static Eldbus_Connection        *_conn  = NULL;
static Eldbus_Service_Interface *_iface = NULL;

static const char *_Elm_Systray_Cat_Str[] = {
   [ELM_SYSTRAY_CATEGORY_APP_STATUS]     = "ApplicationStatus",
   [ELM_SYSTRAY_CATEGORY_COMMUNICATIONS] = "Communications",
   [ELM_SYSTRAY_CATEGORY_SYS_SERVICES]   = "SystemServices",
   [ELM_SYSTRAY_CATEGORY_HARDWARE]       = "Hardware",
   [ELM_SYSTRAY_CATEGORY_OTHER]          = "Other"
};

static const char *_Elm_Systray_Status_Str[] = {
   [ELM_SYSTRAY_STATUS_ACTIVE]    = "Active",
   [ELM_SYSTRAY_STATUS_PASSIVE]   = "Passive",
   [ELM_SYSTRAY_STATUS_ATTENTION] = "NeedsAttention"
};

// =============================================================================
//                     org.kde.StatusNotifierItem Service
// =============================================================================
// =============================================================================
// Methods
// =============================================================================
static Eldbus_Message *
_empty_method(const Eldbus_Service_Interface *iface EINA_UNUSED,
        const Eldbus_Message *msg)
{
   return eldbus_message_method_return_new(msg);
}

static const Eldbus_Method methods[] = {
      {
         "Scroll",
         ELDBUS_ARGS({"i", "delta"}, {"s", "orientation"}),
         NULL,
         _empty_method,
         0
      },
      {
         "SecondaryActivate",
         ELDBUS_ARGS({"i", "x"}, {"i", "y"}),
         NULL,
         _empty_method,
         0
      },
      {
         "XAyatanaSecondaryActivate",
         ELDBUS_ARGS({"u", "timestamp"}),
         NULL,
         _empty_method,
         0
      },
      { NULL, NULL, NULL, NULL, 0 }
};

// =============================================================================
// Signals
// =============================================================================
typedef enum _Elm_Systray_Service_Signals
{
   ELM_SYSTRAY_SIGNAL_NEWATTENTIONICON,
   ELM_SYSTRAY_SIGNAL_NEWICON,
   ELM_SYSTRAY_SIGNAL_NEWICONTHEMEPATH,
   ELM_SYSTRAY_SIGNAL_NEWSTATUS,
   ELM_SYSTRAY_SIGNAL_NEWTITLE,
   ELM_SYSTRAY_SIGNAL_XAYATANANEWLABEL
} Elm_Systray_Service_Signals;

#define _elm_systray_signal_emit(sig, ...)            \
   eldbus_service_signal_emit(_iface, sig, __VA_ARGS__)

static const Eldbus_Signal signals[] = {
     [ELM_SYSTRAY_SIGNAL_NEWATTENTIONICON] = {
          "NewAttentionIcon", NULL, 0
     },
     [ELM_SYSTRAY_SIGNAL_NEWICON] = {
          "NewIcon", NULL, 0
     },
     [ELM_SYSTRAY_SIGNAL_NEWICONTHEMEPATH] = {
          "NewIconThemePath", ELDBUS_ARGS({"s", "icon_theme_path"}), 0
     },
     [ELM_SYSTRAY_SIGNAL_NEWSTATUS] = {
          "NewStatus", ELDBUS_ARGS({"s", "status"}), 0
     },
     [ELM_SYSTRAY_SIGNAL_NEWTITLE] = {
          "NewTitle", NULL, 0
     },
     [ELM_SYSTRAY_SIGNAL_XAYATANANEWLABEL] = {
          "XAyatanaNewLabel", ELDBUS_ARGS({"s", "label"}, {"s", "guide"}), 0
     },
     { NULL, NULL, 0 }
};

// =============================================================================
// Properties
// =============================================================================
static Eina_Bool
_prop_str_empty_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
                    const char *propname EINA_UNUSED,
                    Eldbus_Message_Iter *iter,
                    const Eldbus_Message *request_msg EINA_UNUSED,
                    Eldbus_Message **error EINA_UNUSED)
{
   eldbus_message_iter_basic_append(iter, 's', "");

   return EINA_TRUE;
}

static Eina_Bool
_prop_attention_icon_name_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
                              const char *propname EINA_UNUSED,
                              Eldbus_Message_Iter *iter,
                              const Eldbus_Message *request_msg EINA_UNUSED,
                              Eldbus_Message **error EINA_UNUSED)
{
   const char *s = _item.att_icon_name ? _item.att_icon_name : "";

   eldbus_message_iter_basic_append(iter, 's', s);

   return EINA_TRUE;
}

static Eina_Bool
_prop_category_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
                   const char *propname EINA_UNUSED,
                   Eldbus_Message_Iter *iter,
                   const Eldbus_Message *request_msg EINA_UNUSED,
                   Eldbus_Message **error EINA_UNUSED)
{
   eldbus_message_iter_basic_append(iter, 's', _Elm_Systray_Cat_Str[_item.cat]);

   return EINA_TRUE;
}

static Eina_Bool
_prop_icon_name_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
                    const char *propname EINA_UNUSED,
                    Eldbus_Message_Iter *iter,
                    const Eldbus_Message *request_msg EINA_UNUSED,
                    Eldbus_Message **error EINA_UNUSED)
{
   const char *s = _item.icon_name ? _item.icon_name : "";

   eldbus_message_iter_basic_append(iter, 's', s);

   return EINA_TRUE;
}

static Eina_Bool
_prop_icon_theme_path_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
                          const char *propname EINA_UNUSED,
                          Eldbus_Message_Iter *iter,
                          const Eldbus_Message *request_msg EINA_UNUSED,
                          Eldbus_Message **error EINA_UNUSED)
{
   const char *s = _item.icon_theme_path ? _item.icon_theme_path : "";

   eldbus_message_iter_basic_append(iter, 's', s);

   return EINA_TRUE;
}

static Eina_Bool
_prop_id_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
             const char *propname EINA_UNUSED,
             Eldbus_Message_Iter *iter,
             const Eldbus_Message *request_msg EINA_UNUSED,
             Eldbus_Message **error EINA_UNUSED)
{
   const char *s = _item.id ? _item.id : "";

   eldbus_message_iter_basic_append(iter, 's', s);

   return EINA_TRUE;
}

static Eina_Bool
_prop_menu_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
               const char *propname EINA_UNUSED,
               Eldbus_Message_Iter *iter,
               const Eldbus_Message *request_msg EINA_UNUSED,
               Eldbus_Message **error EINA_UNUSED)
{
   const char *s = _item.menu ? _item.menu : "/";

   eldbus_message_iter_basic_append(iter, 'o', s);

   return EINA_TRUE;
}

static Eina_Bool
_prop_status_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
                 const char *propname EINA_UNUSED,
                 Eldbus_Message_Iter *iter,
                 const Eldbus_Message *request_msg EINA_UNUSED,
                 Eldbus_Message **error EINA_UNUSED)
{
   eldbus_message_iter_basic_append(iter, 's',
                                   _Elm_Systray_Status_Str[_item.status]);

   return EINA_TRUE;
}

static Eina_Bool
_prop_title_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
                const char *propname EINA_UNUSED,
                Eldbus_Message_Iter *iter,
                const Eldbus_Message *request_msg EINA_UNUSED,
                Eldbus_Message **error EINA_UNUSED)
{
   const char *s = _item.title ? _item.title : "";

   eldbus_message_iter_basic_append(iter, 's', s);

   return EINA_TRUE;
}

static Eina_Bool
_prop_xayatana_orderindex_get(const Eldbus_Service_Interface *iface EINA_UNUSED,
                              const char *propname EINA_UNUSED,
                              Eldbus_Message_Iter *iter,
                              const Eldbus_Message *request_msg EINA_UNUSED,
                              Eldbus_Message **error EINA_UNUSED)
{
   eldbus_message_iter_basic_append(iter, 'u', 0);

   return EINA_TRUE;
}

static const Eldbus_Property properties[] = {
       { "AttentionAcessibleDesc", "s", _prop_str_empty_get, NULL, 0 },
       { "AttentionIconName", "s", _prop_attention_icon_name_get, NULL, 0 },
       { "Category", "s", _prop_category_get, NULL, 0 },
       { "IconAcessibleDesc", "s", _prop_str_empty_get, NULL, 0 },
       { "IconName", "s", _prop_icon_name_get, NULL, 0 },
       { "IconThemePath", "s", _prop_icon_theme_path_get, NULL, 0 },
       { "Id", "s", _prop_id_get, NULL, 0 },
       { "Menu", "o", _prop_menu_get, NULL, 0 },
       { "Status", "s", _prop_status_get, NULL, 0 },
       { "Title", "s", _prop_title_get, NULL, 0 },
       { "XAyatanaLabelGuide", "s", _prop_str_empty_get, NULL, 0 },
       { "XAyatanaLabel", "s", _prop_str_empty_get, NULL, 0 },
       { "XAyatanaOrderingIndex", "u", _prop_xayatana_orderindex_get, NULL, 0 },
       { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc _iface_desc = {
     INTERFACE, methods, signals, properties, NULL, NULL
};
// =============================================================================

static void
_menu_died(void *data EINA_UNUSED,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   _item.menu_obj = NULL;

   eina_stringshare_replace(&(_item.menu), NULL);

   eldbus_service_property_changed(_iface, "Menu");
}

static void
_category_set(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   Elm_Systray_Category cat = va_arg(*args, Elm_Systray_Category);

   if (_item.cat == cat) return;

   _item.cat = cat;
   eldbus_service_property_changed(_iface, "Category");
}

static void
_category_get(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   Elm_Systray_Category *ret = va_arg(*args, Elm_Systray_Category *);

   *ret = _item.cat;
}

static void
_status_set(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   Elm_Systray_Status st = va_arg(*args, Elm_Systray_Status);

   if (_item.status == st) return;

   _item.status = st;
   eldbus_service_property_changed(_iface, "Status");
   _elm_systray_signal_emit(ELM_SYSTRAY_SIGNAL_NEWSTATUS,
                            _Elm_Systray_Status_Str[_item.status]);
}

static void
_status_get(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   Elm_Systray_Status *ret = va_arg(*args, Elm_Systray_Status *);
   *ret = _item.status;
}

static void
_att_icon_name_set(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char *att_icon_name = va_arg(*args, const char *);

   if (!eina_stringshare_replace(&(_item.att_icon_name), att_icon_name)) return;

   eldbus_service_property_changed(_iface, "AttentionIconName");
   _elm_systray_signal_emit(ELM_SYSTRAY_SIGNAL_NEWATTENTIONICON, NULL);
}

static void
_att_icon_name_get(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char **ret = va_arg(*args, const char **);
   *ret = _item.att_icon_name;
}

static void
_icon_name_set(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char *icon_name = va_arg(*args, const char *);

   if (!eina_stringshare_replace(&(_item.icon_name), icon_name)) return;

   eldbus_service_property_changed(_iface, "IconName");
   _elm_systray_signal_emit(ELM_SYSTRAY_SIGNAL_NEWICON, NULL);
}

static void
_icon_name_get(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char **ret = va_arg(*args, const char **);
   *ret = _item.icon_name;
}

static void
_icon_theme_path_set(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char *icon_theme_path = va_arg(*args, const char *);

   if (!eina_stringshare_replace(&(_item.icon_theme_path), icon_theme_path))
     return;

   eldbus_service_property_changed(_iface, "IconThemePath");
   _elm_systray_signal_emit(ELM_SYSTRAY_SIGNAL_NEWICONTHEMEPATH,
                            _item.icon_theme_path);
}

static void
_icon_theme_path_get(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char **ret = va_arg(*args, const char **);
   *ret = _item.icon_theme_path;
}

static void
_id_set(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char *id = va_arg(*args, const char *);

   if (!eina_stringshare_replace(&(_item.id), id)) return;

   eldbus_service_property_changed(_iface, "Id");
}

static void
_id_get(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char **ret = va_arg(*args, const char **);
   *ret = _item.id;
}

static void
_title_set(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char *title = va_arg(*args, const char *);

   if (!eina_stringshare_replace(&(_item.title), title)) return;

   eldbus_service_property_changed(_iface, "Title");
   _elm_systray_signal_emit(ELM_SYSTRAY_SIGNAL_NEWTITLE, NULL);
}

static void
_title_get(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const char **ret = va_arg(*args, const char **);
   *ret = _item.title;
}

static void
_menu_set(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const Eo *menu_obj = va_arg(*args, const Eo *);
   const char *menu = NULL;

   if (_item.menu_obj == menu_obj) return;

   if (menu_obj)
     {
        menu = _elm_dbus_menu_register((Eo *) menu_obj);
        evas_object_event_callback_add((Evas_Object *) menu_obj,
                                       EVAS_CALLBACK_DEL, _menu_died, NULL);
     }

   eina_stringshare_replace(&(_item.menu), menu);

   if (_item.menu_obj)
     evas_object_event_callback_del_full((Evas_Object *) _item.menu_obj,
                                         EVAS_CALLBACK_DEL, _menu_died, NULL);

   _item.menu_obj = menu_obj;

   eldbus_service_property_changed(_iface, "Menu");
}

static void
_menu_get(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   const Eo **ret = va_arg(*args, const Eo **);
   *ret = _item.menu_obj;
}

static void
_register(Eo *obj EINA_UNUSED, void *priv EINA_UNUSED, va_list *args)
{
   Eina_Bool *ret = va_arg(*args, Eina_Bool *);
   if (!_elm_need_systray) goto err;

   *ret = _elm_systray_watcher_status_notifier_item_register(OBJ_PATH);
   return;

err:
   *ret = EINA_FALSE;
}

EAPI Eina_Bool
elm_need_systray(void)
{
   if (_elm_need_systray) return EINA_TRUE;

   if (!elm_need_eldbus()) return EINA_FALSE;

   if (!ELM_EVENT_SYSTRAY_READY)
     ELM_EVENT_SYSTRAY_READY = ecore_event_type_new();

   if (!_elm_systray_watcher_init()) return EINA_FALSE;

   _conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   if (!_conn) goto err;

   _iface = eldbus_service_interface_register(_conn, OBJ_PATH, &_iface_desc);
   if (!_iface) goto err;

   _elm_need_systray = EINA_TRUE;
   return EINA_TRUE;

err:
   if (_conn)
     {
        eldbus_connection_unref(_conn);
        _conn = NULL;
     }

   _elm_systray_watcher_shutdown();
   return EINA_FALSE;
}

void
_elm_unneed_systray(void)
{
   if (!_elm_need_systray) return;

   _elm_need_systray = EINA_FALSE;

   eldbus_service_interface_unregister(_iface);

   eldbus_connection_unref(_conn);

   _elm_systray_watcher_shutdown();

   eina_stringshare_del(_item.att_icon_name);
   eina_stringshare_del(_item.icon_name);
   eina_stringshare_del(_item.icon_theme_path);
   eina_stringshare_del(_item.id);
   eina_stringshare_del(_item.title);
   eina_stringshare_del(_item.menu);

   if (_item.menu_obj)
     {
        evas_object_event_callback_del_full((Evas_Object *) _item.menu_obj,
                                            EVAS_CALLBACK_DEL, _menu_died,
                                            NULL);
        _item.menu_obj = NULL;
     }
}

// =============================================================================
// EObject Stuff
// =============================================================================
static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_CATEGORY_SET),
                   _category_set),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_CATEGORY_GET),
                   _category_get),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_STATUS_SET),
                   _status_set),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_STATUS_GET),
                   _status_get),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ATT_ICON_NAME_SET),
                   _att_icon_name_set),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ATT_ICON_NAME_GET),
                   _att_icon_name_get),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ICON_NAME_SET),
                   _icon_name_set),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ICON_NAME_GET),
                   _icon_name_get),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ICON_THEME_PATH_SET),
                   _icon_theme_path_set),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ICON_THEME_PATH_GET),
                   _icon_theme_path_get),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ID_SET),
                   _id_set),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ID_GET),
                   _id_get),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_TITLE_SET),
                   _title_set),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_TITLE_GET),
                   _title_get),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_MENU_SET),
                   _menu_set),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_MENU_GET),
                   _menu_get),
        EO_OP_FUNC(ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_REGISTER),
                   _register),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_CATEGORY_SET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_CATEGORY_GET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_STATUS_SET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_STATUS_GET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_ATT_ICON_NAME_SET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_ATT_ICON_NAME_GET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_ICON_NAME_SET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_ICON_NAME_GET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_ICON_THEME_PATH_SET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_ICON_THEME_PATH_GET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_ID_SET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_ID_GET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_TITLE_SET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_TITLE_GET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_MENU_SET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_MENU_GET, ""),
     EO_OP_DESCRIPTION(ELM_OBJ_SYSTRAY_SUB_ID_REGISTER, ""),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Elm_Systray",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS
     (&ELM_OBJ_SYSTRAY_BASE_ID, op_desc, ELM_OBJ_SYSTRAY_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL//_class_destructor,
};

EO_DEFINE_CLASS(elm_obj_systray_class_get, &class_desc, EO_BASE_CLASS, NULL);
