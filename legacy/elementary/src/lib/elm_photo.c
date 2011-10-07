#include <Elementary.h>
#include "elm_priv.h"
#include "els_icon.h"
#include "els_scroller.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *img;
   int size;
   Eina_Bool fill;
   Ecore_Timer *longtimer;

#ifdef HAVE_ELEMENTARY_ETHUMB
   struct {
      int id;

      struct {
         const char *path;
         const char *key;
      } file, thumb;

      Ethumb_Exists *exists;

      Ecore_Event_Handler *eeh;

      Ethumb_Thumb_Format format;

      Eina_Bool retry : 1;
   } thumb;
#endif

};

#ifdef HAVE_ELEMENTARY_ETHUMB
static Eina_List *_elm_icon_retry = NULL;
static int _icon_pending_request = 0;

static void _icon_thumb_exists(Ethumb_Client *client __UNUSED__, Ethumb_Exists *thread, Eina_Bool exists, void *data);
static void _icon_thumb_stop(Widget_Data *wd, void *ethumbd);
#endif

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);

static const char SIG_CLICKED[] = "clicked";
static const char SIG_DRAG_START[] = "drag,start";
static const char SIG_DRAG_END[] = "drag,end";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CLICKED, ""},
   {SIG_DRAG_START, ""},
   {SIG_DRAG_END, ""},
   {NULL, NULL}
};


static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
#ifdef HAVE_ELEMENTARY_ETHUMB
   Ethumb_Client *ethumbd;
#endif
   if (!wd) return;

#ifdef HAVE_ELEMENTARY_ETHUMB
   ethumbd = elm_thumb_ethumb_client_get();
   _icon_thumb_stop(wd, ethumbd);

   eina_stringshare_del(wd->thumb.file.path);
   eina_stringshare_del(wd->thumb.file.key);
   eina_stringshare_del(wd->thumb.thumb.path);
   eina_stringshare_del(wd->thumb.thumb.key);

   if (wd->thumb.eeh)
     ecore_event_handler_del(wd->thumb.eeh);
#endif

   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->frm, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(wd->frm, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->frm, "photo", "base",
                         elm_widget_style_get(obj));
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
   edje_object_scale_set(wd->frm, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   if (!wd) return;
   if (wd->size > 0)
     {
        double scale = 0.0;

        scale = (wd->size * elm_widget_scale_get(obj) * _elm_config->scale);
        evas_object_size_hint_min_set(wd->img, scale, scale);
        edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
        elm_coords_finger_size_adjust(1, &minw, 1, &minh);
        edje_object_size_min_restricted_calc(wd->frm, &minw, &minh, minw, minh);
        elm_coords_finger_size_adjust(1, &minw, 1, &minh);
        maxw = minw;
        maxh = minh;
        evas_object_size_hint_min_set(obj, minw, minh);
        evas_object_size_hint_max_set(obj, maxw, maxh);
     }
}

static void
_icon_move_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Coord w, h;
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;
   if (wd->fill)
     {
        Edje_Message_Int_Set *msg;
        Evas_Object *icon = _els_smart_icon_object_get(wd->img);

        evas_object_geometry_get(icon, NULL, NULL, &w, &h);
        msg = alloca(sizeof(Edje_Message_Int_Set) + (sizeof(int)));
        msg->count=2;
        msg->val[0] = (int)w;
        msg->val[1] = (int)h;

        edje_object_message_send(wd->frm, EDJE_MESSAGE_INT_SET, 0, msg);
     }

#ifdef HAVE_ELEMENTARY_ETHUMB
   if (wd->thumb.file.path)
     elm_photo_thumb_set(data, wd->thumb.file.path, wd->thumb.file.key);
#endif
}


static void
_drag_done_cb(void *unused __UNUSED__, Evas_Object *obj)
{
   elm_object_scroll_freeze_pop(obj);
   evas_object_smart_callback_call(obj, SIG_DRAG_END, NULL);
}

static Eina_Bool
_longpress(void *objv)
{
   Widget_Data *wd = elm_widget_data_get(objv);
   Evas_Object *tmp;
   const char *file;
   char *buf;

   DBG("Long press: start drag!");
   wd->longtimer = NULL; /* clear: must return NULL now */
   evas_object_event_callback_del(objv, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move);

   tmp = _els_smart_icon_object_get(wd->img);
   file = NULL;
   evas_object_image_file_get(tmp,&file,NULL);
   if (file)
     {
        /* FIXME: Deal with relative paths */
        buf = malloc(strlen(file) + strlen("file://") + 1);
        sprintf(buf, "%s%s","file://",file);
        elm_drag_start(objv, ELM_SEL_FORMAT_IMAGE, buf, _drag_done_cb, NULL);
        free(buf);
     }
   elm_object_scroll_freeze_push(objv);

   evas_object_smart_callback_call(objv, SIG_DRAG_START, NULL);

   return 0; /* Don't call again */
}

static void
_mouse_move(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *move = event;

   /* Sanity */
   if (!wd->longtimer)
     {
        evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move);
        return;
     }

   /* if the event is held, stop waiting */
   if (move->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        /* Moved too far: No longpress for you! */
        ecore_timer_del(wd->longtimer);
        wd->longtimer = NULL;
        evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                       _mouse_move);
     }
}

static void
_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->longtimer) ecore_timer_del(wd->longtimer);

   /* FIXME: Hard coded timeout */
   wd->longtimer = ecore_timer_add(0.7, _longpress, data);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move, data);
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if ((wd) && (wd->longtimer))
     {
        ecore_timer_del(wd->longtimer);
        wd->longtimer = NULL;
     }

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static inline int
_icon_size_min_get(Evas_Object *icon)
{
   int size;
   _els_smart_icon_size_get(icon, &size, NULL);
   return (size < 32) ? 32 : size;
}


#ifdef HAVE_ELEMENTARY_ETHUMB
static void
_icon_thumb_stop(Widget_Data *wd, void *ethumbd)
{
   if (wd->thumb.id >= 0)
     {
        ethumb_client_generate_cancel(ethumbd, wd->thumb.id, NULL, NULL, NULL);
        wd->thumb.id = -1;
        _icon_pending_request--;
     }

   if (wd->thumb.exists)
     {
        ethumb_client_thumb_exists_cancel(wd->thumb.exists, _icon_thumb_exists, wd);
        wd->thumb.exists = NULL;
        _icon_pending_request--;
     }

   if (wd->thumb.retry)
     {
        _elm_icon_retry = eina_list_remove(_elm_icon_retry, wd);
        wd->thumb.retry = EINA_FALSE;
     }
}

static Eina_Bool
_icon_thumb_display(Widget_Data *wd)
{
   Eina_Bool ret = EINA_FALSE;

   if (wd->thumb.format == ETHUMB_THUMB_EET)
     {
        static const char *extensions[] = {
          ".avi", ".mp4", ".ogv", ".mov", ".mpg", ".wmv", NULL
        };
        const char **ext, *ptr;
        int prefix_size;
        Eina_Bool video = EINA_FALSE;

        prefix_size = eina_stringshare_strlen(wd->thumb.file.path) - 4;
        if (prefix_size >= 0)
          {
             ptr = wd->thumb.file.path + prefix_size;
             for (ext = extensions; *ext; ++ext)
               if (!strcasecmp(ptr, *ext))
                 {
                    video = EINA_TRUE;
                    break;
                 }
          }

        if (video)
          ret = _els_smart_icon_file_edje_set(wd->img, wd->thumb.thumb.path, wd->thumb.thumb.key);
     }

   if (!ret)
     ret = _els_smart_icon_file_key_set(wd->img, wd->thumb.thumb.path, wd->thumb.thumb.key);

   return ret;
}

static Eina_Bool
_icon_thumb_retry(Widget_Data *wd)
{
   return _icon_thumb_display(wd);
}

static void
_icon_thumb_cleanup(Ethumb_Client *ethumbd)
{
   Eina_List *l, *ll;
   Widget_Data *wd;

   EINA_LIST_FOREACH_SAFE(_elm_icon_retry, l, ll, wd)
     if (_icon_thumb_retry(wd))
       {
          _elm_icon_retry = eina_list_remove_list(_elm_icon_retry, l);
          wd->thumb.retry = EINA_FALSE;
       }

   if (_icon_pending_request == 0)
     EINA_LIST_FREE(_elm_icon_retry, wd)
       _icon_thumb_stop(wd, ethumbd);
}

static void
_icon_thumb_finish(Widget_Data *wd, Ethumb_Client *ethumbd)
{
   const char *file = NULL, *group = NULL;
   Eina_Bool ret;

   _els_smart_icon_file_get(wd->img, &file, &group);
   file = eina_stringshare_ref(file);
   group = eina_stringshare_ref(group);

   ret = _icon_thumb_display(wd);

   if (!ret && file)
     {
        const char *p;

        if (!wd->thumb.retry)
          {
             _elm_icon_retry = eina_list_append(_elm_icon_retry, wd);
             wd->thumb.retry = EINA_TRUE;
          }

        /* Back to previous image */
        if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
          _els_smart_icon_file_edje_set(wd->img, file, group);
        else
          _els_smart_icon_file_key_set(wd->img, file, group);
     }

   _icon_thumb_cleanup(ethumbd);

   eina_stringshare_del(file);
   eina_stringshare_del(group);
}

static void
_icon_thumb_cb(void *data,
               Ethumb_Client *ethumbd,
               int id,
               const char *file __UNUSED__,
               const char *key __UNUSED__,
               const char *thumb_path,
               const char *thumb_key,
               Eina_Bool success)
{
   Widget_Data *wd = data;

   EINA_SAFETY_ON_FALSE_RETURN(wd->thumb.id == id);
   wd->thumb.id = -1;

   _icon_pending_request--;

   if (success)
     {
        eina_stringshare_replace(&wd->thumb.thumb.path, thumb_path);
        eina_stringshare_replace(&wd->thumb.thumb.key, thumb_key);
        wd->thumb.format = ethumb_client_format_get(ethumbd);

        _icon_thumb_finish(wd, ethumbd);
     }
   else
     {
        ERR("could not generate thumbnail for %s (key: %s)", file, key);
        _icon_thumb_cleanup(ethumbd);
     }
}

static void
_icon_thumb_exists(Ethumb_Client *client __UNUSED__, Ethumb_Exists *thread, Eina_Bool exists, void *data)
{
   Widget_Data *wd = data;
   Ethumb_Client *ethumbd;

   if (ethumb_client_thumb_exists_check(thread))
     return ;

   wd->thumb.exists = NULL;

   ethumbd = elm_thumb_ethumb_client_get();

   if (exists)
     {
        const char *thumb_path, *thumb_key;

        _icon_pending_request--;
        ethumb_client_thumb_path_get(ethumbd, &thumb_path, &thumb_key);
        eina_stringshare_replace(&wd->thumb.thumb.path, thumb_path);
        eina_stringshare_replace(&wd->thumb.thumb.key, thumb_key);
        wd->thumb.format = ethumb_client_format_get(ethumbd);

        _icon_thumb_finish(wd, ethumbd);
     }
   else if ((wd->thumb.id = ethumb_client_generate(ethumbd, _icon_thumb_cb, wd, NULL)) == -1)
     {
        ERR("Generate was unable to start !");
        /* Failed to generate thumbnail */
        _icon_pending_request--;
     }
}

static void
_icon_thumb_apply(Widget_Data *wd)
{
   Ethumb_Client *ethumbd;

   ethumbd = elm_thumb_ethumb_client_get();

   _icon_thumb_stop(wd, ethumbd);

   if (!wd->thumb.file.path) return ;

   _icon_pending_request++;
   if (!ethumb_client_file_set(ethumbd, wd->thumb.file.path, wd->thumb.file.key)) return ;
   ethumb_client_size_set(ethumbd, _icon_size_min_get(wd->img), _icon_size_min_get(wd->img));
   wd->thumb.exists = ethumb_client_thumb_exists(ethumbd, _icon_thumb_exists, wd);
}

static Eina_Bool
_icon_thumb_apply_cb(void *data, int type __UNUSED__, void *ev __UNUSED__)
{
   Widget_Data *wd = data;

   _icon_thumb_apply(wd);
   return ECORE_CALLBACK_RENEW;
}
#endif

EAPI Evas_Object *
elm_photo_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Object *icon;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "photo");
   elm_widget_type_set(obj, "photo");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->frm = edje_object_add(e);
   _elm_theme_object_set(obj, wd->frm, "photo", "base", "default");
   elm_widget_resize_object_set(obj, wd->frm);

   wd->img = _els_smart_icon_add(e);
   _els_smart_icon_scale_up_set(wd->img, 1);
   _els_smart_icon_scale_down_set(wd->img, 1);
   _els_smart_icon_smooth_scale_set(wd->img, 1);
   _els_smart_icon_fill_inside_set(wd->img, 0);
   _els_smart_icon_scale_size_set(wd->img, 0);
   wd->fill = EINA_FALSE;
   _els_smart_icon_scale_set(wd->img,
                             elm_widget_scale_get(obj) * _elm_config->scale);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, obj);
   evas_object_repeat_events_set(wd->img, 1);
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
   evas_object_show(wd->img);
   elm_widget_sub_object_add(obj, wd->img);

   wd->longtimer = NULL;

   icon = _els_smart_icon_object_get(wd->img);
   evas_object_event_callback_add(icon, EVAS_CALLBACK_MOVE,
                                  _icon_move_resize, obj);
   evas_object_event_callback_add(icon, EVAS_CALLBACK_RESIZE,
                                  _icon_move_resize, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

#ifdef HAVE_ELEMENTARY_ETHUMB
   wd->thumb.id = -1;
#endif

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI Eina_Bool
elm_photo_file_set(Evas_Object *obj, const char *file)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   if (!_els_smart_icon_file_key_set(wd->img, file, NULL))
     return EINA_FALSE;

   _sizing_eval(obj);
   return EINA_TRUE;
}

EAPI void
elm_photo_size_set(Evas_Object *obj, int size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->size = (size > 0) ? size : 0;

   _els_smart_icon_scale_size_set(wd->img, wd->size);

   _sizing_eval(obj);
}

EAPI void
elm_photo_fill_inside_set(Evas_Object *obj, Eina_Bool fill)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _els_smart_icon_fill_inside_set(wd->img, fill);
   wd->fill = fill;
   _sizing_eval(obj);
}

EAPI void
elm_photo_editable_set(Evas_Object *obj, Eina_Bool set)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;;
   _els_smart_icon_edit_set(wd->img, set, obj);
}

EAPI void
elm_photo_thumb_set(const Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

#ifdef HAVE_ELEMENTARY_ETHUMB
   eina_stringshare_replace(&wd->thumb.file.path, file);
   eina_stringshare_replace(&wd->thumb.file.key, group);

   if (elm_thumb_ethumb_client_connected())
     {
        _icon_thumb_apply(wd);
        return ;
     }

   if (!wd->thumb.eeh)
     {
        wd->thumb.eeh = ecore_event_handler_add(ELM_ECORE_EVENT_ETHUMB_CONNECT, _icon_thumb_apply_cb, wd);
     }
#else
   (void) obj;
   (void) file;
   (void) group;
#endif
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0 :*/
