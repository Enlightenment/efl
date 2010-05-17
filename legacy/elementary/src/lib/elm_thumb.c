#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Thumb Thumb
 *
 * A thumb object is used for displaying the thumbnail of an image or video.
 * You must have compiled Elementary with Ethumb_Client support and the DBus
 * service must be present and auto-activated in order to have thumbnails to
 * be generated.
 *
 * Signals that you can add callbacks for are:
 *
 * clicked - This is called when a user has clicked the thumb without dragging
 * around.
 *
 * clicked,double - This is called when a user has double-clicked the thumb.
 *
 * press - This is called when a user has pressed down the thumb.
 *
 * generate,start - The thumbnail generation started.
 *
 * generate,stop - The generation process stopped.
 *
 * generate,error - The generation failed.
 *
 * load,error - The thumbnail image loading failed.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *self;
   struct {
	Evas_Object *frm;
	Evas_Object *view;
	struct {
	     float x, y;
	} align;
   } children;
   const char *file;
   const char *key;
   Ecore_Event_Handler *eeh;
   int id;
   Elm_Thumb_Animation_Setting anim_setting;
   Eina_Bool on_hold : 1;
   Eina_Bool is_video : 1;
   Eina_Bool is_generating : 1;
   Eina_Bool keep_aspect : 1;
};

static const char *widtype = NULL;

static const char SIG_CLICKED[] = "clicked";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_GENERATE_ERROR[] = "generate,error";
static const char SIG_GENERATE_START[] = "generate,start";
static const char SIG_GENERATE_STOP[] = "generate,stop";
static const char SIG_LOAD_ERROR[] = "load,error";
static const char SIG_PRESS[]= "press";
static const Evas_Smart_Cb_Description _signals[] = {
  {SIG_CLICKED, ""},
  {SIG_CLICKED_DOUBLE, ""},
  {SIG_GENERATE_ERROR, ""},
  {SIG_GENERATE_START, ""},
  {SIG_GENERATE_STOP, ""},
  {SIG_LOAD_ERROR, ""},
  {SIG_PRESS, ""},
  {NULL, NULL}
};

static const char EDJE_SIGNAL_GENERATE_START[] = "elm,thumb,generate,start";
static const char EDJE_SIGNAL_GENERATE_STOP[] = "elm,thumb,generate,stop";
static const char EDJE_SIGNAL_GENERATE_ERROR[] = "elm,thumb,generate,error";
static const char EDJE_SIGNAL_LOAD_ERROR[] = "elm,thumb,load,error";
static const char EDJE_SIGNAL_PULSE_START[] = "elm,state,pulse,start";
static const char EDJE_SIGNAL_PULSE_STOP[] = "elm,state,pulse,stop";

#ifdef HAVE_ELEMENTARY_ETHUMB
Ethumb_Client *_elm_ethumb_client = NULL;
#endif
Eina_Bool _elm_ethumb_connected = EINA_FALSE;

EAPI int ELM_ECORE_EVENT_ETHUMB_CONNECT = 0;

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   eina_stringshare_del(wd->file);
   eina_stringshare_del(wd->key);
   if (wd->eeh)
     ecore_event_handler_del(wd->eeh);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _elm_theme_object_set(obj, wd->children.frm, "thumb", "base", elm_widget_style_get(obj));
}

#ifdef HAVE_ELEMENTARY_ETHUMB
static void
_mouse_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = data;
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->button != 1)
     return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     wd->on_hold = EINA_TRUE;
   else
     wd->on_hold = EINA_FALSE;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(data, SIG_CLICKED_DOUBLE, NULL);
   else
     evas_object_smart_callback_call(data, SIG_PRESS, NULL);
}

static void
_mouse_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{

   Widget_Data *wd = data;
   Evas_Event_Mouse_Up *ev = event_info;
   if (ev->button != 1)
     return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     wd->on_hold = EINA_TRUE;
   else
     wd->on_hold = EINA_FALSE;
   if (!wd->on_hold)
     evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
   wd->on_hold = EINA_FALSE;
}

static void
_finished_thumb(Widget_Data *wd, int id, const char *thumb_path, const char *thumb_key)
{
   int r;
   Evas *evas;

   evas = evas_object_evas_get(wd->self);
   if (wd->children.view)
     evas_object_del(wd->children.view);
   wd->children.view = NULL;
   wd->id = id;

   if (wd->is_video &&
       ethumb_client_format_get(_elm_ethumb_client) == ETHUMB_THUMB_EET)
     {
	wd->children.view = edje_object_add(evas);
	if (!wd->children.view)
	  {
	     ERR("could not create edje object");
	     goto err;
	  }
	if (!edje_object_file_set(wd->children.view, thumb_path, "movie/thumb"))
	  {
	     ERR("could not set file=%s key=%s for %s", thumb_path, thumb_key,
	         wd->file);
	     goto view_err;
	  }
     }
   else
     {
	wd->children.view = evas_object_image_filled_add(evas);
	if (!wd->children.view)
	  {
	     ERR("could not create image object");
	     goto err;
	  }
	evas_object_image_file_set(wd->children.view, thumb_path, thumb_key);
	r = evas_object_image_load_error_get(wd->children.view);
	if (r != EVAS_LOAD_ERROR_NONE)
	  {
	     ERR("%s: %s", thumb_path, evas_load_error_str(r));
	     goto view_err;
	  }
     }

   elm_widget_sub_object_add(wd->self, wd->children.view);
   edje_object_part_swallow(wd->children.frm, "elm.swallow.content",
			    wd->children.view);
   edje_object_signal_emit(wd->children.frm, EDJE_SIGNAL_GENERATE_STOP, "elm");
   evas_object_smart_callback_call(wd->self, SIG_GENERATE_STOP, NULL);
   return;

view_err:
   evas_object_del(wd->children.view);
   wd->children.view = NULL;
err:
   edje_object_signal_emit(wd->children.frm, EDJE_SIGNAL_LOAD_ERROR, "elm");
   evas_object_smart_callback_call(wd->self, SIG_LOAD_ERROR, NULL);
}

static void
_finished_thumb_cb(void *data, Ethumb_Client *c __UNUSED__, int id, const char *file, const char *key, const char *thumb_path, const char *thumb_key, Eina_Bool success)
{
   Widget_Data *wd = data;

   edje_object_signal_emit(wd->children.frm, EDJE_SIGNAL_PULSE_STOP, "elm");
   wd->is_generating = EINA_FALSE;

   if (success)
     {
	_finished_thumb(wd, id, thumb_path, thumb_key);
	return;
     }

   ERR("could not generate thumbnail for %s (key: %s)", file, key ? key : "");
   edje_object_signal_emit(wd->children.frm, EDJE_SIGNAL_GENERATE_ERROR, "elm");
   evas_object_smart_callback_call(wd->self, SIG_GENERATE_ERROR, NULL);
}

static void
_thumb_apply(Widget_Data *wd)
{
   ethumb_client_file_set(_elm_ethumb_client, wd->file, wd->key);
   if (ethumb_client_thumb_exists(_elm_ethumb_client))
     {
	const char *thumb_path, *thumb_key;
	ethumb_client_thumb_path_get(_elm_ethumb_client, &thumb_path,
				     &thumb_key);
	_finished_thumb(wd, 0, thumb_path, thumb_key);
	return;
     }
   else if (ethumb_client_generate(_elm_ethumb_client, _finished_thumb_cb, wd,
				   NULL) != -1)
     {
	edje_object_signal_emit(wd->children.frm, EDJE_SIGNAL_PULSE_START,
				"elm");
	edje_object_signal_emit(wd->children.frm, EDJE_SIGNAL_GENERATE_START,
				"elm");
	evas_object_smart_callback_call(wd->self, SIG_GENERATE_START, NULL);
     }
   else
     {
	edje_object_signal_emit(wd->children.frm, EDJE_SIGNAL_GENERATE_ERROR,
				"elm");
	evas_object_smart_callback_call(wd->self, SIG_GENERATE_ERROR, NULL);
     }
   wd->is_generating = EINA_FALSE;
}

static int
_thumb_apply_cb(void *data, int type __UNUSED__, void *ev __UNUSED__)
{
   _thumb_apply(data);
   return ECORE_CALLBACK_RENEW;
}

static void
_thumb_show(Widget_Data *wd)
{
   evas_object_show(wd->children.frm);

   if (elm_thumb_ethumb_client_connected())
     {
	_thumb_apply(wd);
	return;
     }

   if (!wd->eeh)
     wd->eeh = ecore_event_handler_add(ELM_ECORE_EVENT_ETHUMB_CONNECT,
				       _thumb_apply_cb, wd);
}

static void
_thumb_show_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _thumb_show(data);
}

static void
_cancel_cb(void *data, Eina_Bool success)
{
   Widget_Data *wd = data;

   if (success)
     {
	wd->is_generating = EINA_FALSE;
	edje_object_signal_emit(wd->children.frm, EDJE_SIGNAL_GENERATE_STOP,
				"elm");
	evas_object_smart_callback_call(wd->self, SIG_GENERATE_STOP, NULL);
     }
}

static void
_thumb_hide_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = data;

   evas_object_hide(wd->children.frm);

   if (wd->is_generating)
     ethumb_client_generate_cancel(_elm_ethumb_client, wd->id, _cancel_cb, wd, NULL);
   else if (wd->eeh)
     {
	ecore_event_handler_del(wd->eeh);
	wd->eeh = NULL;
     }
}

#endif

#ifdef ELM_ETHUMB
static Eina_Bool _elm_need_ethumb = 0;

static void _on_die_cb(void *, Ethumb_Client *);

static void
_connect_cb(void *data __UNUSED__, Ethumb_Client *c, Eina_Bool success)
{
   if (success)
     {
	ethumb_client_on_server_die_callback_set(c, _on_die_cb, NULL, NULL);
	_elm_ethumb_connected = EINA_TRUE;
	ecore_event_add(ELM_ECORE_EVENT_ETHUMB_CONNECT, NULL, NULL, NULL);
     }
   else
     _elm_ethumb_client = NULL;
}

static void
_on_die_cb(void *data __UNUSED__, Ethumb_Client *c __UNUSED__)
{
   ethumb_client_disconnect(_elm_ethumb_client);
   _elm_ethumb_client = NULL;
   _elm_ethumb_connected = EINA_FALSE;
   _elm_ethumb_client = ethumb_client_connect(_connect_cb, NULL, NULL);
}
#endif

void
_elm_unneed_ethumb(void)
{
#ifdef ELM_ETHUMB
   if (_elm_need_ethumb)
     {
	_elm_need_ethumb = 0;
	ethumb_client_disconnect(_elm_ethumb_client);
	_elm_ethumb_client = NULL;
	ethumb_client_shutdown();
	ELM_ECORE_EVENT_ETHUMB_CONNECT = 0;
     }
#endif
}

/**
 * This must be called before any other function that handle with
 * elm_thumb objects or ethumb_client instances.
 *
 * @ingroup Thumb
 */
EAPI void
elm_need_ethumb(void)
{
#ifdef ELM_ETHUMB
   if (_elm_need_ethumb)
     return;
   ELM_ECORE_EVENT_ETHUMB_CONNECT = ecore_event_type_new();
   _elm_need_ethumb = 1;
   ethumb_client_init();
   _elm_ethumb_client = ethumb_client_connect(_connect_cb, NULL, NULL);
#endif
}

/**
 * Add a new thumb object to the parent.
 *
 * @param parent The parent object.
 * @return The new object or NULL if it cannot be created.
 *
 * @see elm_thumb_file_set()
 * @see elm_thumb_ethumb_client_get()
 * @see elm_thumb_keep_aspect_set()
 * @see elm_thumb_align_set()
 * @see elm_thumb_align_get()
 *
 * @ingroup Thumb
 */
EAPI Evas_Object *
elm_thumb_add(Evas_Object *parent)
{
   Evas *evas;
   Widget_Data *wd;
   Evas_Object *obj;
   Evas_Coord minw, minh;

   wd = ELM_NEW(Widget_Data);
   evas = evas_object_evas_get(parent);
   obj = elm_widget_add(evas);
   ELM_SET_WIDTYPE(widtype, "thumb");
   elm_widget_type_set(obj, "thumb");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->children.frm = edje_object_add(evas);
   _elm_theme_object_set(obj, wd->children.frm, "thumb", "base", "default");
   elm_widget_resize_object_set(obj, wd->children.frm);

   edje_object_size_min_calc(obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   wd->self = obj;
   wd->children.view = NULL;
   wd->file = NULL;
   wd->key = NULL;
   wd->eeh = NULL;
   wd->children.align.x = 0.5;
   wd->children.align.y = 0.5;
   wd->on_hold = EINA_FALSE;
   wd->is_video = EINA_FALSE;
   wd->is_generating = EINA_FALSE;
   wd->keep_aspect = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_ETHUMB
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
				  _mouse_down_cb, wd);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
				  _mouse_up_cb, wd);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW,
				  _thumb_show_cb, wd);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE,
				  _thumb_hide_cb, wd);
#endif

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * Set the file that will be used as thumbnail.
 *
 * The file can be an image or a video (in that case, acceptable extensions are:
 * avi, mp4, ogv, mov, mpg and wmv). To start the video animation, use the
 * function elm_thumb_animate().
 *
 * @param obj The thumb object.
 * @param file The path to file that will be used as thumb.
 * @param key The key used in case of an EET file.
 *
 * @see elm_thumb_file_get()
 * @see elm_thumb_animate()
 *
 * @ingroup Thumb
 */
EAPI void
elm_thumb_file_set(Evas_Object *obj, const char *file, const char *key)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_Bool file_replaced, key_replaced;
   Widget_Data *wd = elm_widget_data_get(obj);

   file_replaced = eina_stringshare_replace(&(wd->file), file);
   key_replaced = eina_stringshare_replace(&(wd->key), key);

   if (file_replaced)
     {
	int prefix_size;
	const char **ext, *ptr;
	static const char *extensions[] = { ".avi", ".mp4", ".ogv", ".mov",
					    ".mpg", ".wmv", NULL };

	prefix_size = eina_stringshare_strlen(wd->file) - 4;
	if (prefix_size >= 0)
	  {
	     ptr = wd->file + prefix_size;
	     wd->is_video = EINA_FALSE;
	     for (ext = extensions; *ext; ext++)
	       if (!strcasecmp(ptr, *ext))
		 {
		    wd->is_video = EINA_TRUE;
		    break;
		 }
	  }
     }

#ifdef HAVE_ELEMENTARY_ETHUMB
   if ((file_replaced || key_replaced) && evas_object_visible_get(obj))
     _thumb_show(wd);
#endif
}

/**
 * Get the image or video path and key used to generate the thumbnail.
 *
 * @param obj The thumb object.
 * @param file Pointer to filename.
 * @param key Pointer to key.
 *
 * @see elm_thumb_file_set()
 * @see elm_thumb_animate()
 *
 * @ingroup Thumb
 */
EAPI void
elm_thumb_file_get(const Evas_Object *obj, const char **file, const char **key)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (file)
     *file = wd->file;
   if (key)
     *key = wd->key;
}

/**
 * Set the animation state for the thumb object. If its content is an animated
 * video, you may start/stop the animation or tell it to play continuously and
 * looping.
 *
 * @param obj The thumb object.
 * @param setting The animation setting.
 *
 * @see elm_thumb_file_set()
 *
 * @ingroup Thumb
 */
EAPI void
elm_thumb_animate_set(Evas_Object *obj, Elm_Thumb_Animation_Setting setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (setting < ELM_THUMB_ANIMATION_START ||
       setting >= ELM_THUMB_ANIMATION_LAST)
     {
        return;
     }

   wd->anim_setting = setting;
   if (setting == ELM_THUMB_ANIMATION_LOOP)
     edje_object_signal_emit(wd->children.view, "animate_loop", "");
   else if (setting == ELM_THUMB_ANIMATION_START)
     edje_object_signal_emit(wd->children.view, "animate", "");
   else if (setting == ELM_THUMB_ANIMATION_STOP)
     edje_object_signal_emit(wd->children.view, "animate_stop", "");
}

/**
 * Get the animation state for the thumb object.
 *
 * @param obj The thumb object.
 * @return getting The animation setting or @c ELM_THUMB_ANIMATION_LAST,
 * on errors.
 *
 * @see elm_thumb_file_get()
 *
 * @ingroup Thumb
 */
EAPI Elm_Thumb_Animation_Setting
elm_thumb_animate_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_THUMB_ANIMATION_LAST;
   Widget_Data *wd = elm_widget_data_get(obj);

   return wd->anim_setting;
}

/**
 * Set whether the thumbnail exhibition should keep the image or video aspect.
 *
 * For positioning the image/video within the object use the function
 * elm_thumb_align_set().
 *
 * @param obj The thumb object.
 * @param setting Whether keep or not the aspect.
 *
 * @see elm_thumb_file_set()
 * @see elm_thumb_align_set()
 *
 * @ingroup Thumb
 */
EAPI void
elm_thumb_keep_aspect_set(Evas_Object *obj, Eina_Bool setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->keep_aspect = setting;
}

/**
 * Get back the aspect info set with @c elm_thumb_keep_aspect_set().
 *
 * @param obj The thumb object.
 * @return Whether the thumb object keeps or not the aspect for its content.
 *
 * @see elm_thumb_file_get()
 * @see elm_thumb_align_get()
 *
 * @ingroup Thumb
 */
EAPI Eina_Bool
elm_thumb_keep_aspect_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->keep_aspect;
}

/**
 * Set image/video's alignment within the thumbnail.
 *
 * @param obj The thumb object.
 * @param x_align The x alignment (0 <= x <= 1).
 * @param y_align The y alignment (0 <= y <= 1).
 *
 * @see elm_thumb_keep_aspect_set()
 * @see elm_thumb_align_get()
 *
 * @ingroup Thumb
 */
EAPI void
elm_thumb_align_set(Evas_Object *obj, float x_align, float y_align)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (x_align > 1.0)
     x_align = 1.0;
   else if (x_align < 0.0)
     x_align = 0.0;
   wd->children.align.x = x_align;

   if (y_align > 1.0)
     y_align = 1.0;
   else if (y_align < 0.0)
     y_align = 0.0;
   wd->children.align.y = y_align;
}

/**
 * Get the alignenment set for the thumb object.
 *
 * @param obj The thumb object.
 * @param x Pointer to x alignenment.
 * @param y Pointer to y alignenment.
 *
 * @see elm_thumb_align_set()
 *
 * @ingroup Thumb
 */
EAPI void
elm_thumb_align_get(const Evas_Object *obj, float *x, float *y)
{
    ELM_CHECK_WIDTYPE(obj, widtype);
    Widget_Data *wd = elm_widget_data_get(obj);
    if (x) *x = wd->children.align.x;
    if (y) *y = wd->children.align.y;
}

/**
 * Get the ethumb_client handle so custom configuration can be made.
 * This must be called before the objects are created to be sure no object is
 * visible and no generation started.
 *
 * @return Ethumb_Client instance or NULL.
 *
 * Example of usage:
 *
 * @code
 * #include <Elementary.h>
 * #ifndef ELM_LIB_QUICKLAUNCH
 * EAPI int
 * elm_main(int argc, char **argv)
 * {
 *    Ethumb_Client *client;
 *
 *    elm_need_ethumb();
 *
 *    // ... your code
 *
 *    client = elm_thumb_ethumb_client_get();
 *    if (!client)
 *      {
 *         ERR("could not get ethumb_client");
 *         return 1;
 *      }
 *    ethumb_client_size_set(client, 100, 100);
 *    ethumb_client_crop_align_set(client, 0.5, 0.5);
 *    // ... your code
 *
 *    // Create elm_thumb objects here
 *
 *    elm_run();
 *    elm_shutdown();
 *    return 0;
 * }
 * #endif
 * ELM_MAIN()
 * @endcode
 *
 * @ingroup Thumb
 */
#ifdef ELM_ETHUMB
EAPI Ethumb_Client *
elm_thumb_ethumb_client_get(void)
{
   return _elm_ethumb_client;
}
#else
EAPI void *
elm_thumb_ethumb_client_get(void)
{
   return NULL;
}
#endif

/**
 * Get the ethumb_client connection state.
 *
 * @return EINA_TRUE if the client is connected to the server or
 *	   EINA_FALSE otherwise.
 */
EAPI Eina_Bool
elm_thumb_ethumb_client_connected(void)
{
   return _elm_ethumb_connected;
}
