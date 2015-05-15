#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_clipper.h"
#include "elm_widget_container.h"

#define MY_CLASS ELM_CLIPPER_CLASS

#define MY_CLASS_NAME "Elm_Clipper"
#define MY_CLASS_NAME_LEGACY "elm_clipper"

static void
_calc_region(Eo *obj)
{
   int x, y, w, h;
   int calx1, caly1, calx2, caly2;

   ELM_CLIPPER_DATA_GET(obj, sd);
   evas_object_geometry_get(sd->content, &x, &y, &w, &h);

   calx1 = (int)(w * sd->region_x1 + x);
   caly1 = (int)(h * sd->region_y1 + y);

   if (sd->region_x2 < sd->region_x1) calx2 = 0;
   else calx2 = (int)(w * (sd->region_x2 - sd->region_x1));
   if (sd->region_y2 < sd->region_y1) caly2 = 0;
   else caly2 = (int)(h * (sd->region_y2 - sd->region_y1));

   evas_object_move(sd->clipper, calx1, caly1);
   evas_object_resize(sd->clipper, calx2, caly2);
}

EOLIAN static void
_elm_clipper_clip_set(Eo *obj, Elm_Clipper_Data *sd, Evas_Object *clip)
{
   if (sd->clipper != clip)
     {
        if (sd->clipper)
          {
             elm_widget_sub_object_del(obj, sd->clipper);
             if (sd->content) evas_object_clip_unset(sd->content);
             sd->clipper = NULL;
          }
        if (clip)
          {
             elm_widget_sub_object_add(obj, clip);
             evas_object_smart_member_add(clip, obj);
             _calc_region(obj);
             if (sd->content) evas_object_clip_set(sd->content, clip);
             sd->clipper = clip;
          }
     }
}

EOLIAN static Evas_Object *
_elm_clipper_clip_get(Eo *obj EINA_UNUSED, Elm_Clipper_Data *sd)
{
   return sd->clipper;
}

EOLIAN static void
_elm_clipper_region_set(Eo *obj, Elm_Clipper_Data *sd, double x1, double y1, double x2, double y2)
{
   if (x2 < x1  || y2 < y1)
     ERR("Clipper region x2/y2 should be greater than or equal to x1/y1!\n");

   if (sd->region_x1 != x1 || sd->region_y1 != y1 ||
       sd->region_x2 != x2 || sd->region_y2 != y2)
     {
        sd->region_x1 = x1;
        sd->region_y1 = y1;
        sd->region_x2 = x2;
        sd->region_y2 = y2;
        _calc_region(obj);
     }
}

EOLIAN static void
_elm_clipper_region_get(Eo *obj EINA_UNUSED, Elm_Clipper_Data *sd, double *x1, double *y1, double *x2, double *y2)
{
   if (x1) *x1 = sd->region_x1;
   if (y1) *y1 = sd->region_y1;
   if (x2) *x2 = sd->region_x2;
   if (y2) *y2 = sd->region_y2;
}

EOLIAN static Eina_Bool
_elm_clipper_elm_widget_sub_object_del(Eo *obj, Elm_Clipper_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_del(sobj));
   if (!int_ret) return EINA_FALSE;
   if (sobj == sd->content) evas_object_data_del(sobj, "_elm_leaveme");
   evas_object_smart_member_del(sobj);
   return EINA_TRUE;
}

EOLIAN static void
_elm_clipper_evas_object_smart_resize(Eo *obj, Elm_Clipper_Data *sd EINA_UNUSED, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));
   if(sd->content) evas_object_resize(sd->content, w, h);
   _calc_region(obj);
}

EOLIAN static void
_elm_clipper_evas_object_smart_move(Eo *obj, Elm_Clipper_Data *sd EINA_UNUSED, Evas_Coord x, Evas_Coord y)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));
   evas_object_move(sd->content, x, y);
   _calc_region(obj);
}

EOLIAN static void
_elm_clipper_evas_object_smart_show(Eo *obj, Elm_Clipper_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());
   if(sd->content) evas_object_show(sd->content);
   if(sd->clipper) evas_object_show(sd->clipper);

}

EOLIAN static void
_elm_clipper_evas_object_smart_hide(Eo *obj, Elm_Clipper_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());
   if(sd->content) evas_object_hide(sd->content);
   if(sd->clipper) evas_object_hide(sd->clipper);
}

EOLIAN static Eina_Bool
_elm_clipper_elm_container_content_set(Eo *obj, Elm_Clipper_Data *sd, const char *part, Evas_Object *content)
{
   if (part && strcmp(part, "default")) return EINA_FALSE;
   if (sd->content != content)
     {
        if (sd->content)
          {
             elm_widget_sub_object_del(obj, sd->content);
             sd->content = NULL;
          }
        if (content)
          {
             elm_widget_sub_object_add(content,obj);
             evas_object_data_set(content, "_elm_leaveme", (void *)1);
             evas_object_smart_member_add(content, obj);
             _calc_region(obj);
             if (sd->clipper) evas_object_clip_set(content, sd->clipper);
             sd->content = content;
          }
     }
   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_elm_clipper_elm_container_content_get(Eo *obj EINA_UNUSED, Elm_Clipper_Data *sd, const char *part)
{
   if (part && strcmp(part, "default")) return NULL;
   return sd->content;
}

EOLIAN static Evas_Object*
_elm_clipper_elm_container_content_unset(Eo *obj, Elm_Clipper_Data *sd, const char *part)
{
   Evas_Object *content;
   if (part && strcmp(part, "default")) return NULL;
   content = sd->content;
   elm_widget_sub_object_del(obj, sd->content);
   sd->content = NULL;
   return content;
}

EOLIAN static void
_elm_clipper_evas_object_smart_add(Eo *obj, Elm_Clipper_Data *priv)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   priv->content = NULL;
   priv->clipper = NULL;
   priv->region_x1 = 0;
   priv->region_y1 = 0;
   priv->region_x2 = 1;
   priv->region_y2 = 1;
}

EOLIAN static void
_elm_clipper_evas_object_smart_del(Eo *obj, Elm_Clipper_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
   ELM_SAFE_FREE(sd->content, evas_object_del);
   ELM_SAFE_FREE(sd->clipper, evas_object_del);
   sd->content = NULL;
   sd->clipper = NULL;
}


EAPI Evas_Object *
elm_clipper_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static void
_elm_clipper_eo_base_constructor(Eo *obj, Elm_Clipper_Data *sd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

static void
_elm_clipper_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_clipper.eo.c"
