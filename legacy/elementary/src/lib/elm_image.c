#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Image Image
 *
 * A standard image that may be provided by the theme (delete, edit,
 * arrows etc.) or a custom file (PNG, JPG, EDJE etc.) used for an
 * icon. The Icon may scale or not and of course... support alpha
 * channels.
 *
 * Signals that you can add callbacks for are:
 *
 * "clicked" - This is called when a user has clicked the image
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *img;
   Eina_Bool scale_up : 1;
   Eina_Bool scale_down : 1;
   Eina_Bool smooth : 1;
   Eina_Bool fill_outside : 1;
   Eina_Bool no_scale : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CLICKED, ""},
   {NULL, NULL}
};


static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   free(wd);
}

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   evas_object_del(wd->img);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   int w, h;

   if (!wd) return;
   _els_smart_icon_size_get(wd->img, &w, &h);
   _els_smart_icon_scale_up_set(wd->img, wd->scale_up);
   _els_smart_icon_scale_down_set(wd->img, wd->scale_down);
   _els_smart_icon_smooth_scale_set(wd->img, wd->smooth);
   _els_smart_icon_fill_inside_set(wd->img, !(wd->fill_outside));
   if (wd->no_scale) _els_smart_icon_scale_set(wd->img, 1.0);
   else
     {
        _els_smart_icon_scale_set(wd->img, elm_widget_scale_get(obj) * _elm_config->scale);
        _els_smart_icon_size_get(wd->img, &w, &h);
     }
   if (!wd->scale_down)
     {
        minw = w;
        minh = h;
     }
   if (!wd->scale_up)
     {
        maxw = w;
        maxh = h;
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

/**
 * Add a new image to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Image
 */
EAPI Evas_Object *
elm_image_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "image");
   elm_widget_type_set(obj, "image");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->img = _els_smart_icon_add(e);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);
   evas_object_repeat_events_set(wd->img, EINA_TRUE);
   elm_widget_resize_object_set(obj, wd->img);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   wd->smooth = EINA_TRUE;
   wd->scale_up = EINA_TRUE;
   wd->scale_down = EINA_TRUE;

   _els_smart_icon_scale_size_set(wd->img, 0);

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the file that will be used as image
 *
 * @param obj The image object
 * @param file The path to file that will be used as image
 * @param group The group that the image belongs in edje file
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Image
 */
EAPI Eina_Bool
elm_image_file_set(Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool ret;
   const char *p;

   if (!wd) return EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     ret = _els_smart_icon_file_edje_set(wd->img, file, group);
   else
     ret = _els_smart_icon_file_key_set(wd->img, file, group);
   _sizing_eval(obj);
   return ret;
}

/**
 * Get the file that will be used as image
 *
 * @param obj The image object
 * @param file The path to file
 * @param group The group that the image belongs in edje file
 *
 * @ingroup Image
 */
EAPI void
elm_image_file_get(const Evas_Object *obj, const char **file, const char **group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _els_smart_icon_file_get(wd->img, file, group);
}

/**
 * Set the smooth effect for a image
 *
 * @param obj The image object
 * @param smooth A bool to set (or no) smooth effect
 * (1 = smooth, 0 = not smooth)
 *
 * @ingroup Image
 */
EAPI void
elm_image_smooth_set(Evas_Object *obj, Eina_Bool smooth)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->smooth = smooth;
   _sizing_eval(obj);
}

/**
 * Get the smooth effect for a image
 *
 * @param obj The image object
 * @return If setted smooth effect
 *
 * @ingroup Image
 */
EAPI Eina_Bool
elm_image_smooth_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->smooth;
}

/**
 * Gets the current size of the image.
 *
 * Either width or height (or both) may be NULL.
 *
 * On error, neither w or h will be written too.
 *
 * @param obj The image object.
 * @param w Pointer to store width, or NULL.
 * @param h Pointer to store height, or NULL.
 *
 * @ingroup Image
 */
EAPI void
elm_image_object_size_get(const Evas_Object *obj, int *w, int *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _els_smart_icon_size_get(wd->img, w, h);
}

/**
 * Set if the object are scalable
 *
 * @param obj The image object.
 * @param no_scale A bool to set scale (or no).
 * (1 = no_scale, 0 = scale)
 *
 * @ingroup Image
 */
EAPI void
elm_image_no_scale_set(Evas_Object *obj, Eina_Bool no_scale)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->no_scale = no_scale;
   _sizing_eval(obj);

}

/**
 * Get if the object isn't scalable
 *
 * @param obj The image object
 * @return If isn't scalable
 *
 * @ingroup Image
 */
EAPI Eina_Bool
elm_image_no_scale_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->no_scale;
}

/**
 * Set if the object is (up/down) scalable
 *
 * @param obj The image object
 * @param scale_up A bool to set if the object is scalable up
 * @param scale_down A bool to set if the object is scalable down
 *
 * @ingroup Image
 */
EAPI void
elm_image_scale_set(Evas_Object *obj, Eina_Bool scale_up, Eina_Bool scale_down)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->scale_up = scale_up;
   wd->scale_down = scale_down;
   _sizing_eval(obj);
}

/**
 * Get if the object is (up/down) scalable
 *
 * @param obj The image object
 * @param scale_up A bool to set if the object is scalable up
 * @param scale_down A bool to set if the object is scalable down
 *
 * @ingroup Image
 */
EAPI void
elm_image_scale_get(const Evas_Object *obj, Eina_Bool *scale_up, Eina_Bool *scale_down)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (scale_up) *scale_up = wd->scale_up;
   if (scale_down) *scale_down = wd->scale_down;
}

/**
 * Set if the object is filled outside
 *
 * @param obj The image object
 * @param fill_outside A bool to set if the object is filled outside
 * (1 = filled, 0 = no filled)
 *
 * @ingroup Image
 */
EAPI void
elm_image_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->fill_outside = fill_outside;
   _sizing_eval(obj);
}

/**
 * Get if the object is filled outside
 *
 * @param obj The image object
 * @return If the object is filled outside
 *
 * @ingroup Image
 */
EAPI Eina_Bool
elm_image_fill_outside_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->fill_outside;
}

/**
 * Set the prescale size for the image
 *
 * @param obj The image object
 * @param size The prescale size
 *
 * @ingroup Image
 */
EAPI void
elm_image_prescale_set(Evas_Object *obj, int size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _els_smart_icon_scale_size_set(wd->img, size);
}

/**
 * Get the prescale size for the image
 *
 * @param obj The image object
 * @return The prescale size
 *
 * @ingroup Image
 */
EAPI int
elm_image_prescale_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return 0;
   return _els_smart_icon_scale_size_get(wd->img);
}

/**
 * Set the image orient
 *
 * @param obj The image object
 * @param orient The image orient
 * (ELM_IMAGE_ORIENT_NONE, ELM_IMAGE_ROTATE_90_CW,
 *  ELM_IMAGE_ROTATE_180_CW, ELM_IMAGE_ROTATE_90_CCW,
 *  ELM_IMAGE_FLIP_HORIZONTAL,ELM_IMAGE_FLIP_VERTICAL,
 *  ELM_IMAGE_FLIP_TRANSPOSE, ELM_IMAGE_FLIP_TRANSVERSE)
 *
 * @ingroup Image
 */
EAPI void
elm_image_orient_set(Evas_Object *obj, Elm_Image_Orient orient)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _els_smart_icon_orient_set(wd->img, orient);
}

/**
 * Get the image orient
 *
 * @param obj The image object
 * @return The image orient
 * (ELM_IMAGE_ORIENT_NONE, ELM_IMAGE_ROTATE_90_CW,
 *  ELM_IMAGE_ROTATE_180_CW, ELM_IMAGE_ROTATE_90_CCW,
 *  ELM_IMAGE_FLIP_HORIZONTAL,ELM_IMAGE_FLIP_VERTICAL,
 *  ELM_IMAGE_FLIP_TRANSPOSE, ELM_IMAGE_FLIP_TRANSVERSE)
 *
 * @ingroup Image
 */
EAPI Elm_Image_Orient
elm_image_orient_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_IMAGE_ORIENT_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_IMAGE_ORIENT_NONE;
   return _els_smart_icon_orient_get(wd->img);
}

/**
 * Make the image 'editable'.
 *
 * This means the image is a valid drag target for drag and drop, and can be
 * cut or pasted too.
 *
 * @param obj Image object.
 * @param set Turn on or off editability.
 *
 * @ingroup Image
 */
EAPI void
elm_image_editable_set(Evas_Object *obj, Eina_Bool set)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _els_smart_icon_edit_set(wd->img, set, obj);
}

/**
 * Make the image 'editable'.
 *
 * This means the image is a valid drag target for drag and drop, and can be
 * cut or pasted too.
 *
 * @param obj Image object.
 * @return Editability.
 */
EAPI Eina_Bool
elm_image_editable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return _els_smart_icon_edit_get(wd->img);
}

/**
 * Get the image object
 *
 * When you create a image with elm_image_add(). You can get this object (be
 * careful to not manipulate it as it is under control of elementary), and use
 * it to do things like get pixel data, save the image to a file, etc.
 *
 * @param obj The image object to get the inlined image from
 * @return The inlined image object, or NULL if none exists
 *
 * @ingroup Image
 */
EAPI Evas_Object *
elm_image_object_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return _els_smart_icon_object_get(wd->img);
}

/**
 * Enable/disable retaining up the aspect ratio of the image. 
 *
 * @param obj The image object.
 * @param retained Retaining or Non retaining.
 *
 * @ingroup Image
 */
EAPI void
elm_image_aspect_ratio_retained_set(Evas_Object *obj, Eina_Bool retained)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   return _els_smart_icon_aspect_ratio_retained_set(wd->img, retained);
}

/**
 * Get if the object retains the aspect ratio.
 *
 * @param obj The image object.
 * @return If the object retains the aspect ratio.
 *
 * @ingroup Image
 */
EAPI Eina_Bool
elm_image_aspect_ratio_retained_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return _els_smart_icon_aspect_ratio_retained_get(wd->img);
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0 :*/
