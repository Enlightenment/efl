#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Eina.h>
#include <Eet.h>
#include <Evas.h>

#include "Exactness.h"

#define CACHE_FILE_ENTRY "cache"

enum _Tsuite_Event_Type
{  /*  Add any supported events here */
   TSUITE_EVENT_NOT_SUPPORTED = 0,
   TSUITE_EVENT_MOUSE_IN,
   TSUITE_EVENT_MOUSE_OUT,
   TSUITE_EVENT_MOUSE_DOWN,
   TSUITE_EVENT_MOUSE_UP,
   TSUITE_EVENT_MOUSE_MOVE,
   TSUITE_EVENT_MOUSE_WHEEL,
   TSUITE_EVENT_MULTI_DOWN,
   TSUITE_EVENT_MULTI_UP,
   TSUITE_EVENT_MULTI_MOVE,
   TSUITE_EVENT_KEY_DOWN,
   TSUITE_EVENT_KEY_UP,
   TSUITE_EVENT_KEY_DOWN_WITH_KEYCODE,
   TSUITE_EVENT_KEY_UP_WITH_KEYCODE,
   TSUITE_EVENT_TAKE_SHOT
};
typedef enum _Tsuite_Event_Type Tsuite_Event_Type;

struct _eet_event_type_mapping
{
   Tsuite_Event_Type t;
   const char *name;
};
typedef struct _eet_event_type_mapping eet_event_type_mapping;

struct _Variant_Type_st
{
   const char *type;
   Eina_Bool   unknow : 1;
};
typedef struct _Variant_Type_st Variant_Type_st;

struct _Variant_st
{
   Variant_Type_st t;
   void                *data; /* differently than the union type, we
                               * don't need to pre-allocate the memory
                               * for the field*/
};
typedef struct _Variant_st Variant_st;

struct _Timer_Data
{
   unsigned int recent_event_time;
   Eina_List *current_event;
};
typedef struct _Timer_Data Timer_Data;

struct _Tsuite_Data
{
   int serial;    /**< Serial number of current-file */
   Timer_Data *td;
};
typedef struct _Tsuite_Data Tsuite_Data;

struct _mouse_in_mouse_out
{
   unsigned int timestamp;
   int n_evas;
};

struct _mouse_down_mouse_up
{
   int b;
   Evas_Button_Flags flags;
   unsigned int timestamp;
   int n_evas;
};

struct _mouse_move
{
   int x;
   int y;
   unsigned int timestamp;
   int n_evas;
};

struct _mouse_wheel
{
   int direction;
   int z;
   unsigned int timestamp;
   int n_evas;
};

struct _key_down_key_up
{
   unsigned int timestamp;
   const char *keyname;
   const char *key;
   const char *string;
   const char *compose;
   int n_evas;
};

struct _key_down_key_up_with_keycode
{
   unsigned int timestamp;
   const char *keyname;
   const char *key;
   const char *string;
   const char *compose;
   int n_evas;
   unsigned int keycode;
};

struct _multi_event
{
   int d;
   int b; /* In case of simple mouse down/up, corresponds to the button */
   int x;
   int y;
   double rad;
   double radx;
   double rady;
   double pres;
   double ang;
   double fx;
   double fy;
   Evas_Button_Flags flags;
   unsigned int timestamp;
   int n_evas;
};

struct _multi_move
{
   int d;
   int x;
   int y;
   double rad;
   double radx;
   double rady;
   double pres;
   double ang;
   double fx;
   double fy;
   unsigned int timestamp;
   int n_evas;
};

typedef struct _mouse_in_mouse_out mouse_in_mouse_out;
typedef struct _mouse_down_mouse_up mouse_down_mouse_up;
typedef struct _mouse_move mouse_move;
typedef struct _mouse_wheel mouse_wheel;
typedef struct _multi_event multi_event;
typedef struct _multi_move multi_move;
typedef struct _key_down_key_up key_down_key_up;
typedef struct _key_down_key_up_with_keycode key_down_key_up_with_keycode;
typedef struct _mouse_in_mouse_out take_screenshot;

/* START - EET support typedefs */
#define TSUITE_EVENT_MOUSE_IN_STR "tsuite_event_mouse_in"
#define TSUITE_EVENT_MOUSE_OUT_STR "tsuite_event_mouse_out"
#define TSUITE_EVENT_MOUSE_DOWN_STR "tsuite_event_mouse_down"
#define TSUITE_EVENT_MOUSE_UP_STR "tsuite_event_mouse_up"
#define TSUITE_EVENT_MOUSE_MOVE_STR "tsuite_event_mouse_move"
#define TSUITE_EVENT_MOUSE_WHEEL_STR "tsuite_event_mouse_wheel"
#define TSUITE_EVENT_MULTI_DOWN_STR "tsuite_event_multi_down"
#define TSUITE_EVENT_MULTI_UP_STR "tsuite_event_multi_up"
#define TSUITE_EVENT_MULTI_MOVE_STR "tsuite_event_multi_move"
#define TSUITE_EVENT_KEY_DOWN_STR "tsuite_event_key_down"
#define TSUITE_EVENT_KEY_UP_STR "tsuite_event_key_up"
#define TSUITE_EVENT_KEY_DOWN_WITH_KEYCODE_STR "tsuite_event_key_down_with_keycode"
#define TSUITE_EVENT_KEY_UP_WITH_KEYCODE_STR "tsuite_event_key_up_with_keycode"
#define TSUITE_EVENT_TAKE_SHOT_STR "tsuite_event_take_shot"

struct _Lists_st
{
   Eina_List *variant_list;
   unsigned int first_timestamp;
};
typedef struct _Lists_st Lists_st;

struct _data_desc
{
   Eet_Data_Descriptor *take_screenshot;
   Eet_Data_Descriptor *mouse_in_mouse_out;
   Eet_Data_Descriptor *mouse_down_mouse_up;
   Eet_Data_Descriptor *mouse_move;
   Eet_Data_Descriptor *mouse_wheel;
   Eet_Data_Descriptor *multi_event;
   Eet_Data_Descriptor *multi_move;
   Eet_Data_Descriptor *key_down_key_up;
   Eet_Data_Descriptor *key_down_key_up_with_keycode;

   /* list, variant EET desc support */
   Eet_Data_Descriptor *lists_descriptor;
   Eet_Data_Descriptor *variant_descriptor;
   Eet_Data_Descriptor *variant_unified_descriptor;
};
typedef struct _data_desc data_desc;
/* END   - EET support typedefs */

static data_desc *_desc = NULL; /* this struct holds descs (alloc on init) */

static eet_event_type_mapping eet_mapping[] = {
       { TSUITE_EVENT_MOUSE_IN, TSUITE_EVENT_MOUSE_IN_STR },
       { TSUITE_EVENT_MOUSE_OUT, TSUITE_EVENT_MOUSE_OUT_STR },
       { TSUITE_EVENT_MOUSE_DOWN, TSUITE_EVENT_MOUSE_DOWN_STR },
       { TSUITE_EVENT_MOUSE_UP, TSUITE_EVENT_MOUSE_UP_STR },
       { TSUITE_EVENT_MOUSE_MOVE, TSUITE_EVENT_MOUSE_MOVE_STR },
       { TSUITE_EVENT_MOUSE_WHEEL, TSUITE_EVENT_MOUSE_WHEEL_STR },
       { TSUITE_EVENT_MULTI_DOWN, TSUITE_EVENT_MULTI_DOWN_STR },
       { TSUITE_EVENT_MULTI_UP, TSUITE_EVENT_MULTI_UP_STR },
       { TSUITE_EVENT_MULTI_MOVE, TSUITE_EVENT_MULTI_MOVE_STR },
       { TSUITE_EVENT_KEY_DOWN, TSUITE_EVENT_KEY_DOWN_STR },
       { TSUITE_EVENT_KEY_UP, TSUITE_EVENT_KEY_UP_STR },
       { TSUITE_EVENT_KEY_DOWN_WITH_KEYCODE, TSUITE_EVENT_KEY_DOWN_WITH_KEYCODE_STR },
       { TSUITE_EVENT_KEY_UP_WITH_KEYCODE, TSUITE_EVENT_KEY_UP_WITH_KEYCODE_STR },
       { TSUITE_EVENT_TAKE_SHOT, TSUITE_EVENT_TAKE_SHOT_STR },
       { TSUITE_EVENT_NOT_SUPPORTED, NULL }
};

static Tsuite_Event_Type
_event_mapping_type_get(const char *name)
{
   int i;
   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (strcmp(name, eet_mapping[i].name) == 0)
       return eet_mapping[i].t;

   return TSUITE_EVENT_NOT_SUPPORTED;
}

static unsigned int
_evt_time_get(unsigned int tm, Variant_st *v)
{
   if (!v) return tm;
   switch(_event_mapping_type_get(v->t.type))
     {
      case TSUITE_EVENT_MOUSE_IN:
           {
              mouse_in_mouse_out *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_MOUSE_OUT:
           {
              mouse_in_mouse_out *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_MOUSE_DOWN:
           {
              mouse_down_mouse_up *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_MOUSE_UP:
           {
              mouse_down_mouse_up *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_MOUSE_MOVE:
           {
              mouse_move *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_MOUSE_WHEEL:
           {
              mouse_wheel *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_MULTI_DOWN:
           {
              multi_event *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_MULTI_UP:
           {
              multi_event *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_MULTI_MOVE:
           {
              multi_move *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_KEY_DOWN:
           {
              key_down_key_up *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_KEY_UP:
           {
              key_down_key_up *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_KEY_DOWN_WITH_KEYCODE:
           {
              key_down_key_up_with_keycode *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_KEY_UP_WITH_KEYCODE:
           {
              key_down_key_up_with_keycode *t = v->data;
              return t->timestamp;
           }
      case TSUITE_EVENT_TAKE_SHOT:
           {
              take_screenshot *t = v->data;
              return t->timestamp;
           }
      default: /* All non-input events are not handeled */
         return tm;
         break;
     }
}

static Lists_st *
_free_events(Lists_st *st)
{
   Variant_st *v;
   if (!st) goto end;
   EINA_LIST_FREE(st->variant_list, v)
     {
        free(v->data);
        free(v);
     }

   free(st);  /* Allocated when reading data from EET file */
end:
   return NULL;
}

static const char *
_variant_type_get(const void *data, Eina_Bool  *unknow)
{
   const Variant_Type_st *type = data;
   int i;

   if (unknow)
     *unknow = type->unknow;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (strcmp(type->type, eet_mapping[i].name) == 0)
       return eet_mapping[i].name;

   if (unknow)
     *unknow = EINA_FALSE;

   return type->type;
} /* _variant_type_get */

static Eina_Bool
_variant_type_set(const char *type,
                  void       *data,
                  Eina_Bool   unknow)
{
   Variant_Type_st *vt = data;

   vt->type = type;
   vt->unknow = unknow;
   return EINA_TRUE;
} /* _variant_type_set */

/* START Event struct descriptors */
static Eet_Data_Descriptor *
_take_screenshot_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, take_screenshot);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, take_screenshot, "timestamp",
         timestamp, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, take_screenshot, "n_evas",
         n_evas, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_mouse_in_mouse_out_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, mouse_in_mouse_out);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_in_mouse_out, "timestamp",
         timestamp, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_in_mouse_out, "n_evas",
         n_evas, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_mouse_down_mouse_up_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, mouse_down_mouse_up);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_down_mouse_up, "b", b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_down_mouse_up, "flags",
         flags, EET_T_INT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_down_mouse_up, "timestamp",
         timestamp, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_down_mouse_up, "n_evas",
         n_evas, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_mouse_move_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, mouse_move);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_move, "x", x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_move, "y", y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_move, "timestamp",
         timestamp, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_move, "n_evas",
         n_evas, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_mouse_wheel_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, mouse_wheel);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_wheel, "direction",
         direction, EET_T_INT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_wheel, "z", z, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_wheel, "timestamp",
         timestamp, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, mouse_wheel, "n_evas",
         n_evas, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_key_down_key_up_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, key_down_key_up);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up, "timestamp",
         timestamp, EET_T_UINT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up, "keyname",
         keyname, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up, "key",
         key, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up, "string",
         string, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up, "compose",
         compose, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up, "n_evas",
         n_evas, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_key_down_key_up_with_keycode_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, key_down_key_up_with_keycode);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up_with_keycode, "timestamp",
         timestamp, EET_T_UINT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up_with_keycode, "keyname",
         keyname, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up_with_keycode, "key",
         key, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up_with_keycode, "string",
         string, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up_with_keycode, "compose",
         compose, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up_with_keycode, "n_evas",
         n_evas, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, key_down_key_up_with_keycode, "keycode",
         keycode, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_multi_event_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, multi_event);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "d", d, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "b", b, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "x", x, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "y", y, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "rad", rad, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "radx", radx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "rady", rady, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "pres", pres, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "ang", ang, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "fx", fx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "fy", fy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "flags", flags, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "timestamp",
         timestamp, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_event, "n_evas",
         n_evas, EET_T_INT);

   return _d;
}

static Eet_Data_Descriptor *
_multi_move_desc_make(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *_d;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, multi_move);
   _d = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "d", d, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "x", x, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "y", y, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "rad", rad, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "radx", radx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "rady", rady, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "pres", pres, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "ang", ang, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "fx", fx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "fy", fy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "timestamp",
         timestamp, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_d, multi_move, "n_evas",
         n_evas, EET_T_INT);

   return _d;
}

/* declaring types */
static data_desc *
_data_descriptors_init(void)
{
   if (_desc)  /* Was allocated */
     return _desc;

   _desc = calloc(1, sizeof(data_desc));

   Eet_Data_Descriptor_Class eddc;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Lists_st);
   _desc->lists_descriptor = eet_data_descriptor_file_new(&eddc);

   _desc->take_screenshot = _take_screenshot_desc_make();
   _desc->mouse_in_mouse_out = _mouse_in_mouse_out_desc_make();
   _desc->mouse_down_mouse_up = _mouse_down_mouse_up_desc_make();
   _desc->mouse_move = _mouse_move_desc_make();
   _desc->mouse_wheel = _mouse_wheel_desc_make();
   _desc->multi_event = _multi_event_desc_make();
   _desc->multi_move = _multi_move_desc_make();
   _desc->key_down_key_up = _key_down_key_up_desc_make();
   _desc->key_down_key_up_with_keycode = _key_down_key_up_with_keycode_desc_make();

   /* for variant */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Variant_st);
   _desc->variant_descriptor = eet_data_descriptor_file_new(&eddc);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _variant_type_get;
   eddc.func.type_set = _variant_type_set;
   _desc->variant_unified_descriptor = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MOUSE_IN_STR, _desc->mouse_in_mouse_out);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MOUSE_OUT_STR, _desc->mouse_in_mouse_out);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MOUSE_DOWN_STR, _desc->mouse_down_mouse_up);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MOUSE_UP_STR, _desc->mouse_down_mouse_up);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MOUSE_MOVE_STR, _desc->mouse_move);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MOUSE_WHEEL_STR, _desc->mouse_wheel);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MULTI_DOWN_STR, _desc->multi_event);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MULTI_UP_STR, _desc->multi_event);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_MULTI_MOVE_STR, _desc->multi_move);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_KEY_DOWN_STR, _desc->key_down_key_up);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_KEY_UP_STR, _desc->key_down_key_up);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_KEY_DOWN_WITH_KEYCODE_STR, _desc->key_down_key_up_with_keycode);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_KEY_UP_WITH_KEYCODE_STR, _desc->key_down_key_up_with_keycode);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_desc->variant_unified_descriptor,
         TSUITE_EVENT_TAKE_SHOT_STR, _desc->take_screenshot);


   EET_DATA_DESCRIPTOR_ADD_VARIANT(_desc->variant_descriptor,
         Variant_st, "data", data, t, _desc->variant_unified_descriptor);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_desc->lists_descriptor,
         Lists_st, "first_timestamp", first_timestamp, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_LIST(_desc->lists_descriptor,
         Lists_st, "variant_list", variant_list, _desc->variant_descriptor);

   return _desc;
}

static void
_data_descriptors_shutdown(void)
{
   if (_desc)
     {
        eet_data_descriptor_free(_desc->mouse_in_mouse_out);
        eet_data_descriptor_free(_desc->mouse_down_mouse_up);
        eet_data_descriptor_free(_desc->mouse_move);
        eet_data_descriptor_free(_desc->mouse_wheel);
        eet_data_descriptor_free(_desc->multi_event);
        eet_data_descriptor_free(_desc->multi_move);
        eet_data_descriptor_free(_desc->key_down_key_up);
        eet_data_descriptor_free(_desc->key_down_key_up_with_keycode);
        eet_data_descriptor_free(_desc->take_screenshot);
        eet_data_descriptor_free(_desc->lists_descriptor);
        eet_data_descriptor_free(_desc->variant_descriptor);
        eet_data_descriptor_free(_desc->variant_unified_descriptor);

        free(_desc);
        _desc = NULL;
        /* FIXME: Should probably only init and shutdown once */
     }
}

EAPI Exactness_Unit *
legacy_rec_file_read(const char *filename)
{
   Lists_st *vr_list;
   Eina_List *itr;
   Variant_st *v, *prev_v = NULL;
   Exactness_Unit *unit = NULL;
   Eet_File *fp = eet_open(filename, EET_FILE_MODE_READ);
   if (!fp)
     {
        printf("Failed to open input file <%s>.\n", filename);
        return NULL;
     }

   /* Read events list */
   _data_descriptors_init();
   vr_list = eet_data_read(fp, _desc->lists_descriptor, CACHE_FILE_ENTRY);
   eet_close(fp);
   _data_descriptors_shutdown();

   unit = calloc(1, sizeof(*unit));

   EINA_LIST_FOREACH(vr_list->variant_list, itr, v)
     {
        Exactness_Action *act = calloc(1, sizeof(*act));
        Tsuite_Event_Type old_type = _event_mapping_type_get(v->t.type);
        unsigned int vtm = _evt_time_get(0, v);
        if (!vtm) continue;
        switch (old_type)
          {
           case TSUITE_EVENT_MOUSE_IN:
                {
                   mouse_in_mouse_out *d_i = v->data;
                   act->type = EXACTNESS_ACTION_MOUSE_IN;
                   act->n_evas = d_i->n_evas;
                   break;
                }
           case TSUITE_EVENT_MOUSE_OUT:
                {
                   mouse_in_mouse_out *d_i = v->data;
                   act->type = EXACTNESS_ACTION_MOUSE_OUT;
                   act->n_evas = d_i->n_evas;
                   break;
                }
           case TSUITE_EVENT_MOUSE_DOWN:
           case TSUITE_EVENT_MOUSE_UP:
                {
                   mouse_down_mouse_up *d_i = v->data;
                   Exactness_Action_Multi_Event *d_o = calloc(1, sizeof(*d_o));
                   d_o->b = d_i->b;
                   d_o->flags = d_i->flags;
                   if (old_type == TSUITE_EVENT_MOUSE_DOWN)
                      act->type = EXACTNESS_ACTION_MULTI_DOWN;
                   else
                      act->type = EXACTNESS_ACTION_MULTI_UP;
                   act->n_evas = d_i->n_evas;
                   act->data = d_o;
                   break;
                }
           case TSUITE_EVENT_MOUSE_MOVE:
                {
                   mouse_move *d_i = v->data;
                   Exactness_Action_Multi_Move *d_o = calloc(1, sizeof(*d_o));
                   d_o->x = d_i->x;
                   d_o->y = d_i->y;
                   act->type = EXACTNESS_ACTION_MULTI_MOVE;
                   act->n_evas = d_i->n_evas;
                   act->data = d_o;
                   break;
                }
           case TSUITE_EVENT_MOUSE_WHEEL:
                {
                   mouse_wheel *d_i = v->data;
                   Exactness_Action_Mouse_Wheel *d_o = calloc(1, sizeof(*d_o));
                   d_o->direction = d_i->direction;
                   d_o->z = d_i->z;
                   act->type = EXACTNESS_ACTION_MOUSE_WHEEL;
                   act->n_evas = d_i->n_evas;
                   act->data = d_o;
                   break;
                }
           case TSUITE_EVENT_MULTI_DOWN:
           case TSUITE_EVENT_MULTI_UP:
                {
                   multi_event *d_i = v->data;
                   Exactness_Action_Multi_Event *d_o = calloc(1, sizeof(*d_o));
                   d_o->d = d_i->d;
                   d_o->b = d_i->b;
                   d_o->x = d_i->x;
                   d_o->y = d_i->y;
                   d_o->rad = d_i->rad;
                   d_o->radx = d_i->radx;
                   d_o->rady = d_i->rady;
                   d_o->pres = d_i->pres;
                   d_o->ang = d_i->ang;
                   d_o->fx = d_i->fx;
                   d_o->fy = d_i->fy;
                   d_o->flags = d_i->flags;
                   if (old_type == TSUITE_EVENT_MULTI_DOWN)
                      act->type = EXACTNESS_ACTION_MULTI_DOWN;
                   else
                      act->type = EXACTNESS_ACTION_MULTI_UP;
                   act->n_evas = d_i->n_evas;
                   act->data = d_o;
                   break;
                }
           case TSUITE_EVENT_MULTI_MOVE:
                {
                   multi_move *d_i = v->data;
                   Exactness_Action_Multi_Move *d_o = calloc(1, sizeof(*d_o));
                   d_o->d = d_i->d;
                   d_o->x = d_i->x;
                   d_o->y = d_i->y;
                   d_o->rad = d_i->rad;
                   d_o->radx = d_i->radx;
                   d_o->rady = d_i->rady;
                   d_o->pres = d_i->pres;
                   d_o->ang = d_i->ang;
                   d_o->fx = d_i->fx;
                   d_o->fy = d_i->fy;
                   act->type = EXACTNESS_ACTION_MULTI_MOVE;
                   act->n_evas = d_i->n_evas;
                   act->data = d_o;
                   break;
                }
           case TSUITE_EVENT_KEY_DOWN:
           case TSUITE_EVENT_KEY_UP:
                {
                   key_down_key_up *d_i = v->data;
                   Exactness_Action_Key_Down_Up *d_o = calloc(1, sizeof(*d_o));
                   d_o->keyname = d_i->keyname;
                   d_o->key = d_i->key;
                   d_o->string = d_i->string;
                   d_o->compose = d_i->compose;
                   if (old_type == TSUITE_EVENT_KEY_DOWN)
                      act->type = EXACTNESS_ACTION_KEY_DOWN;
                   else
                      act->type = EXACTNESS_ACTION_KEY_UP;
                   act->n_evas = d_i->n_evas;
                   act->data = d_o;
                   break;
                }
           case TSUITE_EVENT_KEY_DOWN_WITH_KEYCODE:
           case TSUITE_EVENT_KEY_UP_WITH_KEYCODE:
                {
                   key_down_key_up_with_keycode *d_i = v->data;
                   Exactness_Action_Key_Down_Up *d_o = calloc(1, sizeof(*d_o));
                   d_o->keyname = d_i->keyname;
                   d_o->key = d_i->key;
                   d_o->string = d_i->string;
                   d_o->compose = d_i->compose;
                   d_o->keycode = d_i->keycode;
                   if (old_type == TSUITE_EVENT_KEY_DOWN_WITH_KEYCODE)
                      act->type = EXACTNESS_ACTION_KEY_DOWN;
                   else
                      act->type = EXACTNESS_ACTION_KEY_UP;
                   act->n_evas = d_i->n_evas;
                   act->data = d_o;
                   break;
                }
           case TSUITE_EVENT_TAKE_SHOT:
                {
                   take_screenshot *d_i = v->data;
                   act->type = EXACTNESS_ACTION_TAKE_SHOT;
                   act->n_evas = d_i->n_evas;
                   break;
                }
           default: break;
          }
        if (!prev_v)
          {
             if (vr_list->first_timestamp)
                act->delay_ms = _evt_time_get(0, v) - vr_list->first_timestamp;
             else
                act->delay_ms = 0;
          }
        else
          {
             if (vtm > _evt_time_get(0, prev_v))
                act->delay_ms = vtm - _evt_time_get(0, prev_v);
             else act->delay_ms = 0;
          }
        unit->actions = eina_list_append(unit->actions, act);
        prev_v = v;
     }
#ifdef DEBUG_TSUITE
   printf("%s number of actions in the scenario <%d>\n", __func__, eina_list_count(unit->actions));
#endif
   _free_events(vr_list);

   return unit;
}

