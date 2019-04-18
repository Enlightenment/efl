#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_color_class.h"
#include "elm_colorselector_eo.h"

//static Eldbus_Proxy *cc_proxy;
//static Ecore_Event_Handler *h1;
//static Ecore_Event_Handler *h2;

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
   EINA_INLIST;
   Evas_Object *ly;
   Evas_Object *gl;
   Evas_Object *reset;
   Evas_Object *cs;

   //uint64_t winid;
   Ecore_Timer *timer;

   Colorclass *current; //actually Elm_Color_Overlay
   unsigned int num; //color[num]
   Eina_Bool changed : 1;
   Eina_Bool change_reset : 1;
   Eina_Bool exist : 1;
} Colorclass_UI;
#if 0
typedef enum
{
   COLORCLASS_SIGNAL_EDIT,
   COLORCLASS_SIGNAL_CHANGED,
   COLORCLASS_SIGNAL_RESET,
} Colorclass_Signals;

static const Eldbus_Signal colorclass_editor_signals[] =
{
   [COLORCLASS_SIGNAL_EDIT] = {"Edit", ELDBUS_ARGS({"t", "Window ID"}), 0},
   [COLORCLASS_SIGNAL_CHANGED] = {"Changed", ELDBUS_ARGS({"t", "Window ID"}, {"s", "Color class name"}, {"a(iiii)", "Colors"}), 0},
   [COLORCLASS_SIGNAL_RESET] = {"Reset", ELDBUS_ARGS({"t", "Window ID"}, {"b", "Color class name"}), 0},
   {NULL, NULL, 0}
};

static Eina_Inlist *remote_ccuis;
static Eldbus_Service_Interface *remote_iface;
#endif
static Elm_Color_Class_Name_Cb tl_cb;
static Elm_Color_Class_List_Cb list_cb;

static int
_colorclass_sort(Elm_Object_Item *a, Elm_Object_Item *b)
{
   Edje_Color_Class *a1, *b1;

   a1 = elm_object_item_data_get(a);
   b1 = elm_object_item_data_get(b);

   return strcmp(a1->name, b1->name);
}

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
#if 0
static void
_dbus_signal_changed(Colorclass_UI *cc)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *array, *struc;
   int x;

   msg = eldbus_service_signal_new(remote_iface, COLORCLASS_SIGNAL_CHANGED);
   eldbus_message_arguments_append(msg, "t", cc->winid);
   eldbus_message_arguments_append(msg, "s", cc->current->name);
   iter = eldbus_message_iter_get(msg);
   array = eldbus_message_iter_container_new(iter, 'a', "(iiii)");
   for (x = 0; x < 3; x++)
     {
        eldbus_message_iter_arguments_append(array, "(iiii)", &struc);
        eldbus_message_iter_arguments_append(struc, "iiii",
          cc->current->color[x].r, cc->current->color[x].g, cc->current->color[x].b, cc->current->color[x].a);
        eldbus_message_iter_container_close(array, struc);
     }
   eldbus_message_iter_container_close(iter, array);
   eldbus_service_signal_send(remote_iface, msg);
}
#endif
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
_colorclass_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Colorclass_UI *cc = data;

   elm_colorselector_color_get(cc->cs, (int*)&cc->current->color[cc->num].r, (int*)&cc->current->color[cc->num].g,
                               (int*)&cc->current->color[cc->num].b, (int*)&cc->current->color[cc->num].a);
   //if (cc->winid && remote_iface)
     //_dbus_signal_changed(cc);
   //else
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
_colorclass_reset(void *data, const Efl_Event *event EINA_UNUSED)
{
   Colorclass_UI *cc = data;
   Colorclass color;

   if (!cc->current)
     {
        efl_event_callback_stop(event->object);
        return;
     }
   //if (cc->winid && remote_iface)
     //{
        //Eldbus_Message *msg;

        //msg = eldbus_service_signal_new(remote_iface, COLORCLASS_SIGNAL_RESET);
        //eldbus_message_arguments_append(msg, "t", cc->winid);
        //eldbus_message_arguments_append(msg, "s", cc->current->name);
        //eldbus_service_signal_send(remote_iface, msg);
        //cc->change_reset = 1;
        //efl_event_callback_stop(event->object);
        //return;
     //}
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
_colorclass_activate(void *data, const Efl_Event *event)
{
   Colorclass_UI *cc = data;
   Elm_Object_Item *it = event->info;
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
        cc->current = calloc(1, sizeof(Colorclass)); //actually Elm_Color_Overlay
        if (!cc->current) return;

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
   if ((cc->changed && (!cc->change_reset)) || cc->exist)
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
          {
             elm_config_save();
             elm_config_all_flush();
          }
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
_colorclass_dismiss(void *data, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   Colorclass_UI *cc = data;

   evas_object_smart_callback_call(cc->ly, "dismissed", NULL);
}

static void
_colorclass_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Colorclass_UI *cc = data;

   _colorclass_save(cc);
   //if (cc->winid)
     //remote_ccuis = eina_inlist_remove(remote_ccuis, EINA_INLIST_GET(cc));
   //if (cc->winid && remote_iface && (!remote_ccuis))
     //{
        //Eldbus_Connection *conn;

        //conn = eldbus_service_connection_get(remote_iface);
        //eldbus_name_release(conn, ELM_COLOR_CLASS_METHOD_BASE, NULL, NULL);
        //ELM_SAFE_FREE(remote_iface, eldbus_service_interface_unregister);
        //eldbus_connection_unref(conn);
     //}
   free(cc);
}

static char *
_colorclass_text_get(Edje_Color_Class *ecc, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   if (ecc->desc && ecc->desc[0]) return strdup(ecc->desc);
   return strdup(ecc->name ?: "");
}

static void
_colorclass_item_del(Edje_Color_Class *ecc, Evas_Object *obj EINA_UNUSED)
{
   eina_stringshare_del(ecc->name);
   eina_stringshare_del(ecc->desc);
   free(ecc);
}
#if 0
static void
_dbus_request_name_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   unsigned int flag;
   Colorclass_UI *cc = data;

   if (eldbus_message_error_get(msg, NULL, NULL) ||
       (!eldbus_message_arguments_get(msg, "u", &flag)) ||
       (!(flag & ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)))
     {
        /* FIXME: translate */
        elm_object_text_set(cc->ly, "The color editor could not assume ownership of the DBus interface.");
        elm_object_signal_emit(cc->ly, "elm,state,info", "elm");
        return;
     }
   if (cc->winid && remote_ccuis)
     eldbus_service_signal_emit(remote_iface, COLORCLASS_SIGNAL_EDIT, cc->winid);
}

static Evas_Object *
_colorclass_find_win(uint64_t winid)
{
   Evas_Object *win;
   Eina_List *l;
   pid_t pid = getpid();

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        Ecore_Evas *ee;
        const char *engine_name;
        uint64_t id = elm_win_window_id_get(win);

        ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win));
        engine_name = ecore_evas_engine_name_get(ee);
        if (!strncmp(engine_name, "wayland", sizeof("wayland") - 1))
          id = ((uint64_t)id << 32) + pid;
        if (id == winid) return win;
     }
   return NULL;
}

static void
_dbus_edit_helper(Eldbus_Message_Iter *array, Colorclass *lcc)
{
   Eldbus_Message_Iter *struc, *array2, *struc2;
   int x;
   char *desc = "";

   eldbus_message_iter_arguments_append(array, "(ssa(iiii))", &struc);
   if (lcc)
     {
        if (lcc->desc && lcc->desc[0])
          desc = tl_cb ? tl_cb((char*)lcc->desc) : (char*)lcc->desc;
        eldbus_message_iter_arguments_append(struc, "ss", lcc->name, desc);
     }
   else
     eldbus_message_iter_arguments_append(struc, "ss", "", desc);
   array2 = eldbus_message_iter_container_new(struc, 'a', "(iiii)");
   for (x = 0; x < 3; x++)
     {
        eldbus_message_iter_arguments_append(array2, "(iiii)", &struc2);
        if (lcc)
          eldbus_message_iter_arguments_append(struc2, "iiii",
            lcc->color[x].r, lcc->color[x].g, lcc->color[x].b, lcc->color[x].a);
        else
          eldbus_message_iter_arguments_append(struc2, "iiii", 0, 0, 0, 0);
        eldbus_message_iter_container_close(array2, struc2);
     }
   eldbus_message_iter_container_close(struc, array2);
   eldbus_message_iter_container_close(array, struc);
}

static Eina_Bool
_dbus_edit(void *d EINA_UNUSED, int t EINA_UNUSED, Elementary_Colorclass_Edit_Data *ev)
{
   Evas_Object *win;
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *array;
   Eina_Iterator *it;
   Eina_List *eccs = NULL;

   win = _colorclass_find_win(ev->winid);
   if (!win) return ECORE_CALLBACK_RENEW;

   msg = eldbus_proxy_method_call_new(cc_proxy, "SendCC");
   iter = eldbus_message_iter_get(msg);
   eldbus_message_arguments_append(msg, "t", ev->winid);
   eldbus_message_arguments_append(msg, "b", 0);
   iter = eldbus_message_iter_get(msg);
   array = eldbus_message_iter_container_new(iter, 'a', "(ssa(iiii))");
   if (list_cb)
     eccs = list_cb();
   it = edje_color_class_active_iterator_new();
   if (it || eccs)
     {
        Edje_Color_Class *ecc;
        Eina_Hash *test = NULL;

        if (it && eccs)
          test = eina_hash_string_superfast_new(NULL);
        if (it)
          {
             EINA_ITERATOR_FOREACH(it, ecc)
               {
                  if (test)
                    eina_hash_add(test, ecc->name, ecc);
                  _dbus_edit_helper(array, (Colorclass*)ecc);
               }
          }
        EINA_LIST_FREE(eccs, ecc)
          {
             if (test && (!eina_hash_find(test, ecc->name)))
               {
                  eina_hash_add(test, ecc->name, ecc);
                  _dbus_edit_helper(array, (Colorclass*)ecc);
               }
          }
        eina_hash_free(test);
     }
   else
     _dbus_edit_helper(array, NULL);
   eldbus_message_iter_container_close(iter, array);
   eina_iterator_free(it);
   eldbus_proxy_send(cc_proxy, msg, NULL, NULL, -1);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_dbus_changed(void *d EINA_UNUSED, int t EINA_UNUSED, Elementary_Colorclass_Changed_Data *ev)
{
   Eina_Value idv, array;
   uint64_t id;
   char *name;
   Evas_Object *win;
   unsigned int i;
   int color[3][4];

   eina_value_struct_value_get(ev->value, "arg0", &idv);
   eina_value_get(&idv, &id);
   win = _colorclass_find_win(id);
   eina_value_flush(&idv);
   if (!win) return ECORE_CALLBACK_RENEW;
   eina_value_struct_value_get(ev->value, "arg1", &idv);
   eina_value_get(&idv, &name);
   eina_value_struct_value_get(ev->value, "arg2", &array);
   if (eina_value_array_count(&array) != 3)
     {
        ERR("Someone is failing at sending color class data over dbus!");
        goto out;
     }
   for (i = 0; i < eina_value_array_count(&array); i++)
     {
        Eina_Value struc2, c;

        eina_value_array_value_get(&array, i, &struc2);
        eina_value_struct_value_get(&struc2, "arg0", &c);
        eina_value_get(&c, &color[i][0]);
        eina_value_struct_value_get(&struc2, "arg1", &c);
        eina_value_get(&c, &color[i][1]);
        eina_value_struct_value_get(&struc2, "arg2", &c);
        eina_value_get(&c, &color[i][2]);
        eina_value_struct_value_get(&struc2, "arg3", &c);
        eina_value_get(&c, &color[i][3]);
        eina_value_flush(&c);
        eina_value_flush(&struc2);
     }
   edje_color_class_set(name, color[0][0], color[0][1], color[0][2], color[0][3],
                        color[1][0], color[1][1], color[1][2], color[1][3],
                        color[2][0], color[2][1], color[2][2], color[2][3]);
out:
   eina_value_flush(&idv);
   eina_value_flush(&array);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_dbus_reset(void *d EINA_UNUSED, int t EINA_UNUSED, Elementary_Colorclass_Reset_Data *ev)
{
   Evas_Object *win;
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *array;
   Colorclass color;

   win = _colorclass_find_win(ev->winid);
   if (!win) return ECORE_CALLBACK_RENEW;

   msg = eldbus_proxy_method_call_new(cc_proxy, "SendCC");
   iter = eldbus_message_iter_get(msg);
   eldbus_message_arguments_append(msg, "t", ev->winid);
   eldbus_message_arguments_append(msg, "b", 1);
   iter = eldbus_message_iter_get(msg);
   array = eldbus_message_iter_container_new(iter, 'a', "(ssa(iiii))");
   color.name = ev->name;
   color.desc = NULL;
   edje_color_class_del(ev->name);
   edje_color_class_get(ev->name,
                        (int*)&color.color[0].r, (int*)&color.color[0].g, (int*)&color.color[0].b, (int*)&color.color[0].a,
                        (int*)&color.color[1].r, (int*)&color.color[1].g, (int*)&color.color[1].b, (int*)&color.color[1].a,
                        (int*)&color.color[2].r, (int*)&color.color[2].g, (int*)&color.color[2].b, (int*)&color.color[2].a);
   _dbus_edit_helper(array, &color);
   eldbus_message_iter_container_close(iter, array);
   eldbus_proxy_send(cc_proxy, msg, NULL, NULL, -1);
   return ECORE_CALLBACK_RENEW;
}
#endif
static Elm_Genlist_Item_Class itc =
{
   .item_style = "default",
   .func = {
        .text_get = (Elm_Genlist_Item_Text_Get_Cb)_colorclass_text_get,
        .del = (Elm_Genlist_Item_Del_Cb)_colorclass_item_del
   },
   .version = ELM_GENLIST_ITEM_CLASS_VERSION
};
#if 0
static Colorclass_UI *
_dbus_ccui_find(uint64_t winid)
{
   Colorclass_UI *cc;

   EINA_INLIST_FOREACH(remote_ccuis, cc)
     if (cc->winid == winid)
       return cc;
   return NULL;
}

static Eldbus_Message *
_dbus_close(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Colorclass_UI *cc;
   uint64_t winid;

   if (!eldbus_message_arguments_get(msg, "t", &winid))
     return eldbus_message_method_return_new(msg);
   cc = _dbus_ccui_find(winid);
   if (cc)
     {
        evas_object_smart_callback_call(cc->ly, "application_closed", NULL);
        ELM_SAFE_FREE(cc->timer, ecore_timer_del);
     }
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
_dbus_send_cc(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Colorclass_UI *cc;
   Eldbus_Message_Iter *array, *struc;
   uint64_t winid;
   Eina_Bool reset;

   if (!eldbus_message_arguments_get(msg, "tba(ssa(iiii))", &winid, &reset, &array))
     return eldbus_message_method_return_new(msg);
   cc = _dbus_ccui_find(winid);
   if (!cc)
     return eldbus_message_method_return_new(msg);
   ELM_SAFE_FREE(cc->timer, ecore_timer_del);
   while (eldbus_message_iter_get_and_next(array, 'r', &struc))
     {
        Colorclass *ecc;
        const char *name, *desc;
        Eldbus_Message_Iter *array2, *struc2;
        int i = 0;

        eldbus_message_iter_arguments_get(struc, "ssa(iiii)", &name, &desc, &array2);
        if ((!name[0]) && (!desc[0]) && (!elm_genlist_items_count(cc->gl)))
          break;
        ecc = malloc(sizeof(Colorclass));
        ecc->name = eina_stringshare_add(name);
        ecc->desc = eina_stringshare_add(desc);
        while (eldbus_message_iter_get_and_next(array2, 'r', &struc2))
          {
             if (i > 2)
               {
                  ERR("Someone failed at trying to send color class data!");
                  break;
               }
             eldbus_message_iter_arguments_get(struc2, "iiii",
               &ecc->color[i].r, &ecc->color[i].g, &ecc->color[i].b, &ecc->color[i].a);
             i++;
          }
        if (reset)
          {
             cc->changed = !!memcmp(ecc->color, cc->current->color, sizeof(ecc->color));
             cc->change_reset = 1;
             memcpy(cc->current->color, ecc->color, sizeof(ecc->color));
             edje_color_class_set("elm_colorclass_text",
                                  ecc->color[0].r, ecc->color[0].g, ecc->color[0].b, ecc->color[0].a,
                                  ecc->color[1].r, ecc->color[1].g, ecc->color[1].b, ecc->color[1].a,
                                  ecc->color[2].r, ecc->color[2].g, ecc->color[2].b, ecc->color[2].a);
             elm_colorselector_color_set(cc->cs, ecc->color[cc->num].r, ecc->color[cc->num].g,
                                         ecc->color[cc->num].b, ecc->color[cc->num].a);
             _colorclass_cc_update(cc, 0);
             _colorclass_cc_update(cc, 1);
             _colorclass_cc_update(cc, 2);
             free(ecc);
             return eldbus_message_method_return_new(msg);
          }
        else
          elm_genlist_item_sorted_insert(cc->gl, &itc, ecc, NULL, 0, (Eina_Compare_Cb)_colorclass_sort, NULL, NULL);
     }
   if (elm_genlist_items_count(cc->gl))
     elm_object_signal_emit(cc->ly, "elm,state,loaded", "elm");
   else
     elm_object_text_set(cc->ly, "No color scheme available!");

   return eldbus_message_method_return_new(msg);
}

static Eina_Bool
_dbus_timeout(Colorclass_UI *cc)
{
   ELM_SAFE_FREE(cc->timer, ecore_timer_del);
   evas_object_smart_callback_call(cc->ly, "timeout", NULL);
   /* FIXME: translate */
   elm_object_text_set(cc->ly, "Application was unable to provide color scheme info");
   return EINA_FALSE;
}
#endif
/* internal */ void
elm_color_class_init(void)
{
   //eldbus_init();
   //cc_proxy = elementary_colorclass_proxy_get(eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION), ELM_COLOR_CLASS_METHOD_BASE, NULL);
   //h1 = ecore_event_handler_add(ELEMENTARY_COLORCLASS_EDIT_EVENT, (Ecore_Event_Handler_Cb)_dbus_edit, NULL);
   //h2 = ecore_event_handler_add(ELEMENTARY_COLORCLASS_CHANGED_EVENT, (Ecore_Event_Handler_Cb)_dbus_changed, NULL);
   //h2 = ecore_event_handler_add(ELEMENTARY_COLORCLASS_RESET_EVENT, (Ecore_Event_Handler_Cb)_dbus_reset, NULL);
}

void
elm_color_class_shutdown(void)
{
   //Eldbus_Connection *conn = eldbus_object_connection_get(eldbus_proxy_object_get(cc_proxy));
   //ELM_SAFE_FREE(cc_proxy, eldbus_proxy_unref);
   //eldbus_connection_unref(conn);
   //ecore_event_handler_del(h1);
   //ecore_event_handler_del(h2);
   //h1 = h2 = NULL;
   //eldbus_shutdown();
}
#if 0
static const Eldbus_Method colorclass_editor_methods[] =
{
      { "SendCC", ELDBUS_ARGS({"t", "Window ID"}, {"b", "reset"}, {"a(ssa(iiii))", "Array of color classes"}), NULL, _dbus_send_cc, 0},
      { "Close", ELDBUS_ARGS({"t", "Window ID"}), NULL, _dbus_close, 0},
      {NULL, NULL, NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc base_desc =
{
   ELM_COLOR_CLASS_METHOD_BASE, colorclass_editor_methods, colorclass_editor_signals, NULL, NULL, NULL
};
#endif
EAPI Evas_Object *
elm_color_class_editor_add(Evas_Object *obj)
{
   Evas_Object *ly, *bt, *gl, *cs;
   Colorclass_UI *cc;
   Edje_Color_Class *ecc, *ecc2;
   Eina_Iterator *it;
   Eina_Hash *test = NULL;
   Eina_List *ccs = NULL;

   cc = calloc(1, sizeof(Colorclass_UI));
   if (!cc) return NULL;
   //if (!winid)
     {
        it = edje_color_class_active_iterator_new();
        EINA_ITERATOR_FOREACH(it, ecc)
          {
             Colorclass *lcc;

             ecc2 = malloc(sizeof(Edje_Color_Class));
             if (!ecc2) continue;

             memcpy(ecc2, ecc, sizeof(Edje_Color_Class));
             ecc2->name = eina_stringshare_add(ecc->name);
             if (tl_cb)
               ecc2->desc = eina_stringshare_add(tl_cb((char*)ecc->desc));
             else
               ecc2->desc = eina_stringshare_add(ecc->desc);
             if (list_cb && (!test))
               test = eina_hash_string_superfast_new(NULL);
             if (test)
               eina_hash_add(test, ecc->name, ecc2);
             lcc = (Colorclass*)ecc;
             edje_color_class_set(lcc->name,
                             lcc->color[0].r, lcc->color[0].g, lcc->color[0].b, lcc->color[0].a,
                             lcc->color[1].r, lcc->color[1].g, lcc->color[1].b, lcc->color[1].a,
                             lcc->color[2].r, lcc->color[2].g, lcc->color[2].b, lcc->color[2].a);
             ccs = eina_list_append(ccs, ecc2);
          }
        eina_iterator_free(it);
     }
#if 0
   else
     {
        if (!remote_iface)
          {
             Eldbus_Connection *dbus_conn;

             dbus_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
             eldbus_name_request(dbus_conn, ELM_COLOR_CLASS_METHOD_BASE, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                                 _dbus_request_name_cb, cc);
             remote_iface = eldbus_service_interface_register(dbus_conn, "/", &base_desc);
          }
        cc->timer = ecore_timer_add(1.5, (Ecore_Task_Cb)_dbus_timeout, cc);
        remote_ccuis = eina_inlist_append(remote_ccuis, EINA_INLIST_GET(cc));
     }
   cc->winid = winid;
#endif
   cc->ly = ly = elm_layout_add(obj);
   if (!elm_layout_theme_set(ly, "layout", "colorclass", "base"))
     CRI("Failed to set layout!");
   elm_layout_signal_callback_add(ly, "elm,colorclass,select,*", "elm", _colorclass_select, cc);
   elm_layout_signal_callback_add(ly, "elm,colorclass,deactivate", "elm", _colorclass_deactivate, cc);
   elm_layout_signal_callback_add(ly, "elm,colorclass,dismiss", "elm", _colorclass_dismiss, cc);
   evas_object_event_callback_add(ly, EVAS_CALLBACK_DEL, _colorclass_del, cc);

   cc->gl = gl = elm_genlist_add(ly);
   elm_genlist_homogeneous_set(gl, 1);
   elm_scroller_bounce_set(gl, 0, 0);
   elm_scroller_policy_set(gl, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
   elm_genlist_mode_set(gl, ELM_LIST_COMPRESS);
   elm_object_part_content_set(ly, "elm.swallow.list", gl);
   efl_event_callback_add
     (gl, EFL_UI_EVENT_ITEM_SELECTED, _colorclass_activate, cc);

   cc->reset = bt = elm_button_add(ly);
   elm_object_style_set(bt, "colorclass");
   /* FIXME: translate */
   elm_object_text_set(bt, "Reset");
   elm_object_part_content_set(ly, "elm.swallow.reset", bt);
   efl_event_callback_add
     (bt, EFL_UI_EVENT_CLICKED, _colorclass_reset, cc);

   cc->cs = cs = elm_colorselector_add(ly);
   elm_colorselector_mode_set(cs, ELM_COLORSELECTOR_COMPONENTS);
   elm_object_part_content_set(ly, "elm.swallow.colors", cs);
   efl_event_callback_add
     (cs, ELM_COLORSELECTOR_EVENT_CHANGED_USER, _colorclass_changed, cc);

   EINA_LIST_FREE(ccs, ecc)
     elm_genlist_item_append(gl, &itc, ecc, NULL, 0, NULL, NULL);

   //if (winid)
     //{
        ///* FIXME: translate */
        //elm_object_text_set(ly, "Loading color scheme...");
        //elm_object_signal_emit(ly, "elm,state,info", "elm");
        //elm_object_signal_emit(ly, "elm,state,remote", "elm");
     //}
   //else
   if (list_cb)
     {
        ccs = list_cb();
        EINA_LIST_FREE(ccs, ecc)
          {
             if (test)
               {
                  if (eina_hash_find(test, ecc->name))
                    {
                       eina_stringshare_del(ecc->name);
                       eina_stringshare_del(ecc->desc);
                       free(ecc);
                       continue;
                    }
                  eina_hash_add(test, ecc->name, ecc);
               }
             elm_genlist_item_sorted_insert(gl, &itc, ecc, NULL, 0, (Eina_Compare_Cb)_colorclass_sort, NULL, NULL);
          }
     }
   eina_hash_free(test);

   return ly;
}

EAPI void
elm_color_class_translate_cb_set(Elm_Color_Class_Name_Cb cb)
{
   tl_cb = cb;
}

EAPI void
elm_color_class_list_cb_set(Elm_Color_Class_List_Cb cb)
{
   list_cb = cb;
}

EAPI Eina_List *
elm_color_class_util_edje_file_list(Eina_File *f)
{
   Edje_Color_Class *ecc, *ecc2;
   Eina_Iterator *it;
   Eina_List *ret = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(f, NULL);

   it = edje_mmap_color_class_iterator_new(f);
   if (!it) return NULL;
   EINA_ITERATOR_FOREACH(it, ecc)
     {
        ecc2 = malloc(sizeof(Edje_Color_Class));
        if (!ecc2) continue;

        memcpy(ecc2, ecc, sizeof(Edje_Color_Class));
        ecc2->name = eina_stringshare_add(ecc->name);
        if (tl_cb)
          ecc2->desc = eina_stringshare_add(tl_cb((char*)ecc->desc));
        else
          ecc2->desc = eina_stringshare_add(ecc->desc);
        ret = eina_list_append(ret, ecc2);
     }
   eina_iterator_free(it);
   return ret;
}
