#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
   Evas_Coord   x, y, w, h;
   Evas_Object *obj;
   Evas_Object *prev;
   int          size;
   double       scale;
   Eina_Bool fill_inside : 1;
   Eina_Bool scale_up : 1;
   Eina_Bool scale_down : 1;
   Eina_Bool preloading : 1;
   Eina_Bool show : 1;
   Eina_Bool edit : 1;
   Eina_Bool edje : 1;
   Eina_Bool aspect_ratio_retained: 1;
   Elm_Image_Orient orient;
};

/* local subsystem functions */
static void _smart_reconfigure(Smart_Data *sd);
static void _smart_init(void);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object *obj);

static void _els_smart_icon_flip_horizontal(Smart_Data *sd);
static void _els_smart_icon_flip_vertical(Smart_Data *sd);
static void _els_smart_icon_rotate_180(Smart_Data *sd);
static Eina_Bool _els_smart_icon_dropcb(void *,Evas_Object *, Elm_Selection_Data *);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
_els_smart_icon_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

static void
_preloaded(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   Smart_Data *sd = data;

   sd->preloading = EINA_FALSE;
   if (sd->show)
     evas_object_show(sd->obj);
   evas_object_del(sd->prev);
   sd->prev = NULL;
}

Eina_Bool
_els_smart_icon_file_key_set(Evas_Object *obj, const char *file, const char *key)
{
   Smart_Data *sd;
   Evas_Object *pclip;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;
   /* smart code here */
   if (sd->prev) evas_object_del(sd->prev);
   if (sd->obj) sd->prev = sd->obj;
   sd->obj = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_IMAGE_PRELOADED,
                                  _preloaded, sd);
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_image_scale_hint_set(sd->obj, EVAS_IMAGE_SCALE_HINT_STATIC);

   if (sd->edje)
     {
        pclip = evas_object_clip_get(sd->obj);
        evas_object_clip_set(sd->obj, pclip);
        sd->edje = EINA_FALSE;
     }
   if (!sd->size)
     evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
   evas_object_image_file_set(sd->obj, file, key);
   sd->preloading = EINA_TRUE;
   sd->show = EINA_TRUE;
   evas_object_hide(sd->obj);
   evas_object_show(sd->obj);
   evas_object_image_preload(sd->obj, EINA_FALSE);
   if (evas_object_image_load_error_get(sd->obj) != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for '%s' (%p)", file, sd->obj);
        return EINA_FALSE;
     }
   _smart_reconfigure(sd);
   return EINA_TRUE;
}

Eina_Bool
_els_smart_icon_file_edje_set(Evas_Object *obj, const char *file, const char *part)
{
   Smart_Data *sd;
   Evas_Object *pclip;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;
   /* smart code here */
   if (!sd->edje)
     {
        pclip = evas_object_clip_get(sd->obj);
        if (sd->obj) evas_object_del(sd->obj);
        sd->obj = edje_object_add(evas_object_evas_get(obj));
        evas_object_smart_member_add(sd->obj, obj);
        if (sd->show) evas_object_show(sd->obj);
        evas_object_clip_set(sd->obj, pclip);
     }
   sd->edje = EINA_TRUE;
   if (!edje_object_file_set(sd->obj, file, part))
     return EINA_FALSE;
   _smart_reconfigure(sd);
   return EINA_TRUE;
}

void
_els_smart_icon_file_get(const Evas_Object *obj, const char **file, const char **key)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->edje)
     edje_object_file_get(sd->obj, file, key);
   else
     evas_object_image_file_get(sd->obj, file, key);
}

void
_els_smart_icon_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->edje)
     return;
   evas_object_image_smooth_scale_set(sd->obj, smooth);
}

Eina_Bool
_els_smart_icon_smooth_scale_get(const Evas_Object *obj)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;
   if (sd->edje)
     return EINA_FALSE;
   return evas_object_image_smooth_scale_get(sd->obj);
}

Evas_Object *
_els_smart_icon_object_get(const Evas_Object *obj)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   return sd->obj;
}

void
_els_smart_icon_size_get(const Evas_Object *obj, int *w, int *h)
{
   Smart_Data *sd;
   int tw, th;
   int cw, ch;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     edje_object_size_min_get(sd->obj, &tw, &th);
   else
     evas_object_image_size_get(sd->obj, &tw, &th);
   evas_object_geometry_get(sd->obj, NULL, NULL, &cw, &ch);
   tw = tw > cw ? tw : cw;
   th = th > ch ? th : ch;
   tw = ((double)tw) * sd->scale;
   th = ((double)th) * sd->scale;
   if (w) *w = tw;
   if (h) *h = th;
}

void
_els_smart_icon_fill_inside_set(Evas_Object *obj, Eina_Bool fill_inside)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->fill_inside) && (fill_inside)) ||
       ((!sd->fill_inside) && (!fill_inside))) return;
   sd->fill_inside = fill_inside;
   _smart_reconfigure(sd);
}

Eina_Bool
_els_smart_icon_fill_inside_get(const Evas_Object *obj)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;
   return sd->fill_inside;
}

void
_els_smart_icon_scale_up_set(Evas_Object *obj, Eina_Bool scale_up)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->scale_up) && (scale_up)) ||
       ((!sd->scale_up) && (!scale_up))) return;
   sd->scale_up = scale_up;
   _smart_reconfigure(sd);
}

Eina_Bool
_els_smart_icon_scale_up_get(const Evas_Object *obj)
{
   Smart_Data *sd; sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;
   return sd->scale_up;
}

void
_els_smart_icon_scale_down_set(Evas_Object *obj, Eina_Bool scale_down)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->scale_down) && (scale_down)) ||
       ((!sd->scale_down) && (!scale_down))) return;
   sd->scale_down = scale_down;
   _smart_reconfigure(sd);
}

Eina_Bool
_els_smart_icon_scale_down_get(const Evas_Object *obj)
{
   Smart_Data *sd; sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;
   return sd->scale_up;
}

void
_els_smart_icon_scale_size_set(Evas_Object *obj, int size)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->size = size;
   if (!sd->obj) return;
   if (sd->edje)
     return;
   evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
}

int
_els_smart_icon_scale_size_get(const Evas_Object *obj)
{
   Smart_Data *sd; sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->size;
}

void
_els_smart_icon_scale_set(Evas_Object *obj, double scale)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->scale = scale;
   _smart_reconfigure(sd);
}

double
_els_smart_icon_scale_get(const Evas_Object *obj)
{
   Smart_Data *sd; sd = evas_object_smart_data_get(obj);
   if (!sd) return 0.0;
   return sd->scale;
}

void
_els_smart_icon_orient_set(Evas_Object *obj, Elm_Image_Orient orient)
{
   Smart_Data   *sd;
   Evas_Object  *tmp;
   unsigned int *data, *data2, *to, *from;
   int           x, y, w, hw, iw, ih;
   const char   *file, *key;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->edje)
     return;

   switch (orient)
     {
      case ELM_IMAGE_FLIP_HORIZONTAL:
         _els_smart_icon_flip_horizontal(sd);
         return;
      case ELM_IMAGE_FLIP_VERTICAL:
         _els_smart_icon_flip_vertical(sd);
         return;
      case ELM_IMAGE_ROTATE_180_CW:
         _els_smart_icon_rotate_180(sd);
         return;
      default:
         break;
     }

   evas_object_image_size_get(sd->obj, &iw, &ih);
   evas_object_image_file_get(sd->obj, &file, &key);
   tmp = evas_object_image_add(evas_object_evas_get(sd->obj));
   evas_object_image_file_set(tmp, file, key);
   data2 = evas_object_image_data_get(tmp, EINA_FALSE);

   w = ih;
   ih = iw;
   iw = w;
   hw = w * ih;

   evas_object_image_size_set(sd->obj, iw, ih);
   data = evas_object_image_data_get(sd->obj, EINA_TRUE);
   switch (orient)
     {
      case ELM_IMAGE_FLIP_TRANSPOSE:
         to = data;
         hw = -hw + 1;
         break;
      case ELM_IMAGE_FLIP_TRANSVERSE:
         to = data + hw - 1;
         w = -w;
         hw = hw - 1;
         break;
      case ELM_IMAGE_ROTATE_90_CW:
         to = data + w - 1;
         hw = -hw - 1;
         break;
      case ELM_IMAGE_ROTATE_90_CCW:
         to = data + hw - w;
         w = -w;
         hw = hw + 1;
         break;
      default:
         ERR("unknown orient %d", orient);
         evas_object_del(tmp);
         evas_object_image_data_set(sd->obj, data); // give it back
         return;
     }
   from = data2;
   for (x = iw; --x >= 0;)
     {
        for (y = ih; --y >= 0;)
          {
             *to = *from;
             from++;
             to += w;
          }
        to += hw;
     }
   sd->orient = orient;
   evas_object_del(tmp);
   evas_object_image_data_set(sd->obj, data);
   evas_object_image_data_update_add(sd->obj, 0, 0, iw, ih);
   _smart_reconfigure(sd);
}

Elm_Image_Orient
_els_smart_icon_orient_get(const Evas_Object *obj)
{
   Smart_Data *sd; sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->orient;
}

/**
 * Turns on editing through drag and drop and copy and paste.
 */
void
_els_smart_icon_edit_set(Evas_Object *obj, Eina_Bool edit, Evas_Object *parent)
{
   Smart_Data   *sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   if (sd->edje)
     {
        printf("No editing edje objects yet (ever)\n");
        return;
     }

   /* Unfortunately eina bool is not a bool, but a char */
   if (edit == sd->edit) return;

   sd->edit = edit;

   if (sd->edit)
     elm_drop_target_add(obj, ELM_SEL_FORMAT_IMAGE, _els_smart_icon_dropcb,
                         parent);
   else
     elm_drop_target_del(obj);
}

Eina_Bool
_els_smart_icon_edit_get(const Evas_Object *obj)
{
   Smart_Data *sd; sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;
   return sd->edit;
}

Evas_Object *
_els_smart_icon_edje_get(Evas_Object *obj)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   if (!sd->edje) return NULL;
   return sd->obj;
}

void
_els_smart_icon_aspect_ratio_retained_set(Evas_Object *obj, Eina_Bool retained)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   retained = !!retained;
   if (sd->aspect_ratio_retained == retained) return;
   sd->aspect_ratio_retained = retained;
   _smart_reconfigure(sd);
}

Eina_Bool
_els_smart_icon_aspect_ratio_retained_get(const Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;
   return sd->aspect_ratio_retained;
}

/* local subsystem globals */
static void
_smart_reconfigure(Smart_Data *sd)
{
   Evas_Coord x, y, w, h;

   if (!sd->obj) return;

   w = sd->w;
   h = sd->h;

   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     {
        x = sd->x;
        y = sd->y;
        evas_object_move(sd->obj, x, y);
        evas_object_resize(sd->obj, w, h);
     }
   else
     {
        int iw = 0, ih = 0;

        evas_object_image_size_get(sd->obj, &iw, &ih);

        iw = ((double)iw) * sd->scale;
        ih = ((double)ih) * sd->scale;

        if (iw < 1) iw = 1;
        if (ih < 1) ih = 1;

        if (sd->aspect_ratio_retained)
          {
             h = ((double)ih * w) / (double)iw;
             if (sd->fill_inside)
               {
                  if (h > sd->h)
                    {
                       h = sd->h;
                       w = ((double)iw * h) / (double)ih;
                    }
               }
             else
               {
                  if (h < sd->h)
                    {
                       h = sd->h;
                       w = ((double)iw * h) / (double)ih;
                    }
               }
          }
        if (!sd->scale_up)
          {
             if (w > iw) w = iw;
             if (h > ih) h = ih;
          }
        if (!sd->scale_down)
          {
             if (w < iw) w = iw;
             if (h < ih) h = ih;
          }
        x = sd->x + ((sd->w - w) / 2);
        y = sd->y + ((sd->h - h) / 2);
        evas_object_move(sd->obj, x, y);
        evas_object_image_fill_set(sd->obj, 0, 0, w, h);
        evas_object_resize(sd->obj, w, h);
     }
}

static void
_smart_init(void)
{
   if (_e_smart) return;
     {
        static const Evas_Smart_Class sc =
          {
             "e_icon",
             EVAS_SMART_CLASS_VERSION,
             _smart_add,
             _smart_del,
             _smart_move,
             _smart_resize,
             _smart_show,
             _smart_hide,
             _smart_color_set,
             _smart_clip_set,
             _smart_clip_unset,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL
          };
        _e_smart = evas_smart_class_new(&sc);
     }
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->obj = evas_object_image_add(evas_object_evas_get(obj));
   sd->prev = NULL;
   evas_object_image_scale_hint_set(sd->obj, EVAS_IMAGE_SCALE_HINT_STATIC);
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->fill_inside = EINA_TRUE;
   sd->scale_up = EINA_TRUE;
   sd->scale_down = EINA_TRUE;
   sd->aspect_ratio_retained = EINA_TRUE;
   sd->size = 64;
   sd->scale = 1.0;
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_smart_data_set(obj, sd);
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_IMAGE_PRELOADED,
                                  _preloaded, sd);
}

static void
_smart_del(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_del(sd->obj);
   if (sd->prev) evas_object_del(sd->prev);
   free(sd);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->x == x) && (sd->y == y)) return;
   sd->x = x;
   sd->y = y;
   _smart_reconfigure(sd);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->w == w) && (sd->h == h)) return;
   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);
}

static void
_smart_show(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->show = EINA_TRUE;
   if (!sd->preloading)
     evas_object_show(sd->obj);
   else if (sd->prev)
     evas_object_show(sd->prev);
}

static void
_smart_hide(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->show = EINA_FALSE;
   evas_object_hide(sd->obj);
   if (sd->prev) evas_object_hide(sd->prev);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_color_set(sd->obj, r, g, b, a);
   if (sd->prev) evas_object_color_set(sd->prev, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object * clip)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->obj, clip);
   if (sd->prev) evas_object_clip_set(sd->prev, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->obj);
   if (sd->prev) evas_object_clip_unset(sd->prev);
}

static void
_els_smart_icon_flip_horizontal(Smart_Data *sd)
{
   unsigned int   *data;
   unsigned int   *p1, *p2, tmp;
   int             x, y, iw, ih;

   evas_object_image_size_get(sd->obj, &iw, &ih);
   data = evas_object_image_data_get(sd->obj, EINA_TRUE);

   for (y = 0; y < ih; y++)
     {
        p1 = data + (y * iw);
        p2 = data + ((y + 1) * iw) - 1;
        for (x = 0; x < (iw >> 1); x++)
          {
             tmp = *p1;
             *p1 = *p2;
             *p2 = tmp;
             p1++;
             p2--;
          }
     }

   evas_object_image_data_set(sd->obj, data);
   evas_object_image_data_update_add(sd->obj, 0, 0, iw, ih);
   _smart_reconfigure(sd);
}

static void
_els_smart_icon_flip_vertical(Smart_Data *sd)
{
   unsigned int   *data;
   unsigned int   *p1, *p2, tmp;
   int             x, y, iw, ih;

   evas_object_image_size_get(sd->obj, &iw, &ih);
   data = evas_object_image_data_get(sd->obj, EINA_TRUE);

   for (y = 0; y < (ih >> 1); y++)
     {
        p1 = data + (y * iw);
        p2 = data + ((ih - 1 - y) * iw);
        for (x = 0; x < iw; x++)
          {
             tmp = *p1;
             *p1 = *p2;
             *p2 = tmp;
             p1++;
             p2++;
          }
     }

   evas_object_image_data_set(sd->obj, data);
   evas_object_image_data_update_add(sd->obj, 0, 0, iw, ih);
   _smart_reconfigure(sd);
}

static void
_els_smart_icon_rotate_180(Smart_Data *sd)
{
   unsigned int   *data;
   unsigned int   *p1, *p2, tmp;
   int             x, hw, iw, ih;

   evas_object_image_size_get(sd->obj, &iw, &ih);
   data = evas_object_image_data_get(sd->obj, 1);

   hw = iw * ih;
   x = (hw / 2);
   p1 = data;
   p2 = data + hw - 1;
   for (; --x > 0;)
     {
        tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
     }
   evas_object_image_data_set(sd->obj, data);
   evas_object_image_data_update_add(sd->obj, 0, 0, iw, ih);
   _smart_reconfigure(sd);
}

static Eina_Bool
_els_smart_icon_dropcb(void *elmobj,Evas_Object *obj, Elm_Selection_Data *drop)
{
   _els_smart_icon_file_key_set(obj, drop->data, NULL);
   evas_object_smart_callback_call(elmobj, "drop", drop->data);

   return EINA_TRUE;
}
/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0 :*/
