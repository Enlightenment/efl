#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eet.h>

#include "Exactness.h"
#include "exactness_private.h"

typedef struct _Dummy
{
} _Dummy;
/*
static Eet_Data_Descriptor *
_mouse_in_out_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Action_Mouse_In_Out);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Mouse_In_Out, "n_evas", n_evas, EET_T_INT);

   return _d;
}
*/

static Eet_Data_Descriptor *
_mouse_wheel_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Action_Mouse_Wheel);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Mouse_Wheel, "direction", direction, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Mouse_Wheel, "z", z, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_key_down_up_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Action_Key_Down_Up);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Key_Down_Up, "keyname", keyname, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Key_Down_Up, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Key_Down_Up, "string", string, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Key_Down_Up, "compose", compose, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Key_Down_Up, "keycode", keycode, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_multi_event_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Action_Multi_Event);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "d", d, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "b", b, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "x", x, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "y", y, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "rad", rad, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "radx", radx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "rady", rady, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "pres", pres, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "ang", ang, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "fx", fx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "fy", fy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Event, "flags", flags, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_multi_move_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Action_Multi_Move);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "d", d, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "x", x, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "y", y, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "rad", rad, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "radx", radx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "rady", rady, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "pres", pres, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "ang", ang, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "fx", fx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Multi_Move, "fy", fy, EET_T_DOUBLE);

   return _d;
}

static Eet_Data_Descriptor *
_efl_event_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Action_Efl_Event);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Efl_Event, "wdg_name", wdg_name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Efl_Event, "event_name", event_name, EET_T_STRING);

   return _d;
}

static Eet_Data_Descriptor *
_click_on_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Action_Click_On);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, Exactness_Action_Click_On, "wdg_name", wdg_name, EET_T_STRING);

   return _d;
}

static Eet_Data_Descriptor *
_dummy_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, _Dummy);
   _d = eet_data_descriptor_stream_new(&eddc);

   return _d;
}

/* !!! SAME ORDER AS Exactness_Action_Type */
static const char *_mapping[] =
{
   "",
   "exactness_action_mouse_in",
   "exactness_action_mouse_out",
   "exactness_action_mouse_wheel",
   "exactness_action_multi_down",
   "exactness_action_multi_up",
   "exactness_action_multi_move",
   "exactness_action_key_down",
   "exactness_action_key_up",
   "exactness_action_take_shot",
   "exactness_action_efl_event",
   "exactness_action_click_on",
   "exactness_action_stabilize"
};

const char *
_exactness_action_type_to_string_get(Exactness_Action_Type type)
{
   if (type <= EXACTNESS_ACTION_LAST) return _mapping[type];
   return NULL;
}

static const char *
_variant_type_get(const void *data, Eina_Bool  *unknow)
{
   const Exactness_Action *act = data;

   if (unknow) *unknow = EINA_FALSE;
   if (act->type <= EXACTNESS_ACTION_LAST) return _mapping[act->type];

   return NULL;
}

static Eina_Bool
_variant_type_set(const char *type,
                  void       *data,
                  Eina_Bool   unknow EINA_UNUSED)
{
   int i;
   Exactness_Action *act = data;
   for (i = 0; i <= EXACTNESS_ACTION_LAST; i++)
     {
        if (!strcmp(_mapping[i], type)) act->type = i;
     }
   return EINA_TRUE;
}

static Eet_Data_Descriptor *
_unit_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   static Eet_Data_Descriptor *unit_d = NULL;
   static Eet_Data_Descriptor *action_d = NULL, *action_variant_d = NULL;
   static Eet_Data_Descriptor *objs_d = NULL;
   static Eet_Data_Descriptor *obj_d = NULL;
   if (!obj_d)
     {
        EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Object);
        obj_d = eet_data_descriptor_stream_new(&eddc);
        EET_DATA_DESCRIPTOR_ADD_BASIC(obj_d, Exactness_Object, "kl_name", kl_name, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(obj_d, Exactness_Object, "id", id, EET_T_ULONG_LONG);
        EET_DATA_DESCRIPTOR_ADD_BASIC(obj_d, Exactness_Object, "parent_id", parent_id, EET_T_ULONG_LONG);
        /* Evas stuff */
        EET_DATA_DESCRIPTOR_ADD_BASIC(obj_d, Exactness_Object, "x", x, EET_T_INT);
        EET_DATA_DESCRIPTOR_ADD_BASIC(obj_d, Exactness_Object, "y", y, EET_T_INT);
        EET_DATA_DESCRIPTOR_ADD_BASIC(obj_d, Exactness_Object, "w", w, EET_T_INT);
        EET_DATA_DESCRIPTOR_ADD_BASIC(obj_d, Exactness_Object, "h", h, EET_T_INT);

        EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Objects);
        objs_d = eet_data_descriptor_stream_new(&eddc);
        EET_DATA_DESCRIPTOR_ADD_LIST(objs_d, Exactness_Objects, "objs", objs, obj_d);
     }
   if (!unit_d)
     {
        Eet_Data_Descriptor *code_d = NULL;
        EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Source_Code);
        code_d = eet_data_descriptor_stream_new(&eddc);
        EET_DATA_DESCRIPTOR_ADD_BASIC(code_d, Exactness_Source_Code, "language", language, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(code_d, Exactness_Source_Code, "content", content, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(code_d, Exactness_Source_Code, "command", command, EET_T_STRING);

        EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Action);
        action_d = eet_data_descriptor_stream_new(&eddc);

        eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
        eddc.func.type_get = _variant_type_get;
        eddc.func.type_set = _variant_type_set;
        action_variant_d = eet_data_descriptor_stream_new(&eddc);

        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_MOUSE_IN], _dummy_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_MOUSE_OUT], _dummy_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_MOUSE_WHEEL], _mouse_wheel_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_MULTI_DOWN], _multi_event_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_MULTI_UP], _multi_event_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_MULTI_MOVE], _multi_move_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_KEY_DOWN], _key_down_up_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_KEY_UP], _key_down_up_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_TAKE_SHOT], _dummy_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_EFL_EVENT], _efl_event_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_CLICK_ON], _click_on_desc_make());
        EET_DATA_DESCRIPTOR_ADD_MAPPING(action_variant_d,
              _mapping[EXACTNESS_ACTION_STABILIZE], _dummy_desc_make());

        EET_DATA_DESCRIPTOR_ADD_BASIC(action_d, Exactness_Action, "n_evas", n_evas, EET_T_UINT);
        EET_DATA_DESCRIPTOR_ADD_BASIC(action_d, Exactness_Action, "delay_ms", delay_ms, EET_T_UINT);
        EET_DATA_DESCRIPTOR_ADD_VARIANT(action_d, Exactness_Action, "data", data, type, action_variant_d);

        /* Exactness_Unit */
        EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Exactness_Unit);
        unit_d = eet_data_descriptor_stream_new(&eddc);
        EET_DATA_DESCRIPTOR_ADD_LIST(unit_d, Exactness_Unit, "actions", actions, action_d);
        EET_DATA_DESCRIPTOR_ADD_LIST(unit_d, Exactness_Unit, "objs", objs, objs_d);
        EET_DATA_DESCRIPTOR_ADD_LIST(unit_d, Exactness_Unit, "codes", codes, code_d);
        EET_DATA_DESCRIPTOR_ADD_BASIC(unit_d, Exactness_Unit, "fonts_path", fonts_path, EET_T_STRING);
        EET_DATA_DESCRIPTOR_ADD_BASIC(unit_d, Exactness_Unit, "nb_shots", nb_shots, EET_T_UINT);
     }

   return unit_d;
}
/* END   Event struct descriptors */

EAPI Exactness_Unit *
exactness_unit_file_read(const char *filename)
{
   int i;
   Eina_List *itr, *itr2;
   Exactness_Objects *e_objs;
   Exactness_Object *e_obj, *e_parent;
   Exactness_Unit *unit = NULL;
   Eet_File *file;
   eet_init();
   file = eet_open(filename, EET_FILE_MODE_READ);
   if (!file)
     {
        fprintf(stderr, "Impossible to extract EET from %s\n", filename);
        return NULL;
     }
   unit = eet_data_read(file, _unit_desc_make(), "cache");
   for (i = 0; i < unit->nb_shots; i++)
     {
        char entry[32];
        Exactness_Image *ex_img = malloc(sizeof(*ex_img));
        sprintf(entry, "images/%d", i + 1);
        ex_img->pixels = eet_data_image_read(file, entry,
              &ex_img->w, &ex_img->h, NULL,
              NULL, NULL, NULL);
        unit->imgs = eina_list_append(unit->imgs, ex_img);
     }
   EINA_LIST_FOREACH(unit->objs, itr, e_objs)
     {
        Eina_Hash *hash = eina_hash_pointer_new(NULL);
        EINA_LIST_FOREACH(e_objs->objs, itr2, e_obj)
          {
             eina_hash_set(hash, &(e_obj->id), e_obj);
          }
        EINA_LIST_FOREACH(e_objs->objs, itr2, e_obj)
          {
             if (!e_obj->parent_id)
                e_objs->main_objs = eina_list_append(e_objs->main_objs, e_obj);
             else
               {
                  e_parent = eina_hash_find(hash, &(e_obj->parent_id));
                  if (e_parent) e_parent->children = eina_list_append(e_parent->children, e_obj);
               }
          }
        eina_hash_free(hash);
     }
   eet_close(file);
   eet_shutdown();
   return unit;
}

EAPI Eina_Bool
exactness_unit_file_write(Exactness_Unit *unit, const char *filename)
{
   Eina_List *itr;
   Exactness_Image *ex_img;
   Eet_File *file;
   int i = 1;
   eet_init();
   file = eet_open(filename, EET_FILE_MODE_WRITE);
   eet_data_write(file, _unit_desc_make(), "cache", unit, EINA_TRUE);
   EINA_LIST_FOREACH(unit->imgs, itr, ex_img)
     {
        char entry[32];
        sprintf(entry, "images/%d", i++);
        eet_data_image_write(file, entry,
              ex_img->pixels, ex_img->w, ex_img->h, 0xFF,
              9, 100, EET_IMAGE_LOSSLESS);
     }
   eet_close(file);
   eet_shutdown();
   return EINA_TRUE;
}

EAPI Eina_Bool
exactness_image_compare(Exactness_Image *img1, Exactness_Image *img2, Exactness_Image **imgO)
{
   unsigned int w, h;
   int *pxs1, *pxs2, *pxsO = NULL;
   unsigned int w1 = img1 ? img1->w : 0, h1 = img1 ? img1->h : 0;
   unsigned int w2 = img2 ? img2->w : 0, h2 = img2 ? img2->h : 0;
   unsigned int wO = MAX(w1, w2);
   unsigned int hO = MAX(h1, h2);
   Eina_Bool ret = EINA_FALSE;
   if (imgO) *imgO = NULL;
   if (!wO || !hO) return EINA_FALSE;

   pxs1 = img1 ? img1->pixels : NULL;
   pxs2 = img2 ? img2->pixels : NULL;
   if (imgO) pxsO = malloc(wO * hO * 4);

   for (w = 0; w < wO; w++)
     {
        for (h = 0; h < hO; h++)
          {
             Eina_Bool valid1 = img1 ? w < w1 && h < h1 : EINA_FALSE;
             Eina_Bool valid2 = img2 ? w < w2 && h < h2 : EINA_FALSE;
             int px1 = valid1 ? pxs1[h * w1 + w] : 0;
             int px2 = valid2 ? pxs2[h * w2 + w] : 0;
             int r1 = (px1 & 0x00FF0000) >> 16;
             int r2 = (px2 & 0x00FF0000) >> 16;
             int g1 = (px1 & 0x0000FF00) >> 8;
             int g2 = (px2 & 0x0000FF00) >> 8;
             int b1 = (px1 & 0x000000FF);
             int b2 = (px2 & 0x000000FF);
             int new_r, new_g, new_b;
             if (valid1 || valid2)
               {
                  if (px1 != px2)
                    {
                       new_r = 0xFF;
                       new_g = ((g1 + g2) >> 1) >> 2;
                       new_b = ((b1 + b2) >> 1) >> 2;
                       ret = EINA_TRUE;
                    }
                  else
                    {
                       new_r = (((r1 + r2) >> 1) >> 2) + 0xC0;
                       new_g = (((g1 + g2) >> 1) >> 2) + 0xC0;
                       new_b = (((b1 + b2) >> 1) >> 2) + 0xC0;
                    }
               }
             else
               {
                  new_r = new_g = new_b = 0x0;
               }
             if (pxsO) pxsO[h * wO + w] = 0xFF000000 | new_r << 16 | new_g << 8 | new_b;
          }
     }
   if (imgO)
     {
        Exactness_Image *imgR = calloc(1, sizeof(Exactness_Image));
        *imgO = imgR;
        imgR->w = wO;
        imgR->h = hO;
        imgR->pixels = pxsO;
     }
   return ret;
}

EAPI void exactness_image_free(Exactness_Image *img)
{
   if (!img) return;
   free(img->pixels);
   free(img);
}

