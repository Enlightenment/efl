#include <Elementary.h>
#include "elm_priv.h"
#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define DATETIME_FIELD_COUNT    6
#define FIELD_FORMAT_LEN        3
#define DISKSELECTOR_MIN_ITEMS  4
#define BUFF_SIZE               1024

typedef struct _Ctxpopup_Module_Data Ctxpopup_Module_Data;
typedef struct _DiskItem_Data DiskItem_Data;

struct _Ctxpopup_Module_Data
{
   Elm_Datetime_Module_Data mod_data;
   Evas_Object *ctxpopup;
};

struct _DiskItem_Data
{
   Ctxpopup_Module_Data *ctx_mod;
   Elm_Datetime_Field_Type  sel_field_type;
   unsigned int sel_field_value;
};

static void
_diskselector_item_free_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (data) free(data);
}

static void
_ctxpopup_dismissed_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__ )
{
   Evas_Object *diskselector;

   diskselector = elm_object_content_unset(obj);
   if (diskselector) evas_object_del(diskselector);
}

static void
_datetime_resize_cb(void *data, Evas *e __UNUSED__,Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Ctxpopup_Module_Data *ctx_mod;

   ctx_mod = (Ctxpopup_Module_Data *)data;
   if (!ctx_mod) return;

   evas_object_hide(ctx_mod->ctxpopup);
}

static void
_datetime_move_cb(void *data, Evas *e __UNUSED__,Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Ctxpopup_Module_Data *ctx_mod;

   ctx_mod = (Ctxpopup_Module_Data *)data;
   if (!ctx_mod) return;

   evas_object_hide(ctx_mod->ctxpopup);
}

static void
_field_value_set(struct tm *tim, Elm_Datetime_Field_Type  field_type, int val)
{
   if (field_type >= DATETIME_FIELD_COUNT - 1) return;

   int *timearr[]= { &tim->tm_year, &tim->tm_mon, &tim->tm_mday, &tim->tm_hour, &tim->tm_min };
   *timearr[field_type] = val;
}

static int
_field_value_get(struct tm *tim, Elm_Datetime_Field_Type  field_type)
{
   if (field_type >= DATETIME_FIELD_COUNT - 1) return -1;

   int *timearr[]= { &tim->tm_year, &tim->tm_mon, &tim->tm_mday, &tim->tm_hour, &tim->tm_min };
   return (*timearr[field_type]);
}

static void
_diskselector_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   DiskItem_Data *disk_data;
   struct tm curr_time;
   const char *fmt;

   disk_data = (DiskItem_Data *)elm_object_item_data_get(event_info);
   if (!disk_data || !(disk_data->ctx_mod)) return;

   elm_datetime_value_get(disk_data->ctx_mod->mod_data.base, &curr_time);
   fmt = disk_data->ctx_mod->mod_data.field_format_get(disk_data->ctx_mod->mod_data.base, disk_data->sel_field_type);
   if ((disk_data->sel_field_type == ELM_DATETIME_HOUR) && ((!strncmp(fmt, "%I", FIELD_FORMAT_LEN)) ||
        (!strncmp(fmt, "%l", FIELD_FORMAT_LEN))) && (curr_time.tm_hour >= 12))
     disk_data->sel_field_value += 12;
   _field_value_set(&curr_time, disk_data->sel_field_type, disk_data->sel_field_value);
   elm_datetime_value_set(disk_data->ctx_mod->mod_data.base, &curr_time);
   evas_object_hide(disk_data->ctx_mod->ctxpopup);
}

static void
_ampm_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ctxpopup_Module_Data *ctx_mod;
   struct tm curr_time;

   ctx_mod = (Ctxpopup_Module_Data *)data;
   if (!ctx_mod) return;

   elm_datetime_value_get(ctx_mod->mod_data.base, &curr_time);
   if (curr_time.tm_hour >= 12) curr_time.tm_hour -= 12;
   else curr_time.tm_hour += 12;
   elm_datetime_value_set(ctx_mod->mod_data.base, &curr_time);
}

static void
_field_clicked_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Ctxpopup_Module_Data *ctx_mod;
   Evas_Object *diskselector;
   Elm_Object_Item *item;
   DiskItem_Data *disk_data;
   Elm_Datetime_Field_Type  field_type;
   time_t t;
   struct tm time1;
   char buf[BUFF_SIZE], label[BUFF_SIZE];
   const char *fmt;
   int idx, min, max, val;
   unsigned int display_item_num, text_len = 0;
   Evas_Coord x = 0, y = 0, w = 0, h = 0, width;

   ctx_mod = (Ctxpopup_Module_Data *)data;
   if (!ctx_mod || !ctx_mod->ctxpopup) return;

   // because of the diskselector behaviour, it is being recreated
   diskselector = elm_diskselector_add(elm_widget_top_get(ctx_mod->mod_data.base));
   snprintf(buf, sizeof(buf), "datetime/%s", elm_object_style_get(obj));
   elm_object_style_set(diskselector, buf);
   elm_object_content_set(ctx_mod->ctxpopup, diskselector);

   t = time(NULL);
   localtime_r(&t, &time1);

   field_type = (Elm_Datetime_Field_Type )evas_object_data_get(obj, "_field_type");
   fmt = ctx_mod->mod_data.field_format_get(ctx_mod->mod_data.base, field_type);
   elm_datetime_value_get(ctx_mod->mod_data.base, &time1);
   val = _field_value_get(&time1, field_type);
   ctx_mod->mod_data.field_limit_get(ctx_mod->mod_data.base, field_type, &min, &max);

   time1.tm_mday = 1;   // To avoid month wrapping, set the first day of the month to start with.

   if ((field_type == ELM_DATETIME_HOUR) && ((!strncmp(fmt, "%I", FIELD_FORMAT_LEN)) ||
        (!strncmp(fmt, "%l", FIELD_FORMAT_LEN))))
     {
        if (max >= 12) max -= 12;
        if (val >= 12) val -= 12;
        if (min >= 12) min -= 12;
     }
   for (idx = min; idx <= max; idx++)
     {
        _field_value_set(&time1, field_type, idx);
        strftime(label, BUFF_SIZE, fmt, &time1);
        if (strlen(label) > text_len) text_len = strlen(label);
        if (idx == val)
          {
             item = elm_diskselector_item_append(diskselector, label, NULL, NULL, NULL);
             elm_diskselector_item_selected_set(item, EINA_TRUE);
          }
        else
          {
             disk_data = (DiskItem_Data *) malloc (sizeof(DiskItem_Data));
             disk_data->ctx_mod = ctx_mod;
             disk_data->sel_field_type = field_type;
             disk_data->sel_field_value = idx;
             item = elm_diskselector_item_append(diskselector, label, NULL, NULL, disk_data);
             evas_object_smart_callback_add(diskselector, "clicked", _diskselector_cb, NULL);
             elm_object_item_del_cb_set(item, _diskselector_item_free_cb);
          }
     }
   elm_diskselector_side_text_max_length_set(diskselector, text_len);

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_geometry_get(elm_widget_top_get(ctx_mod->mod_data.base), NULL, NULL, &width, NULL);
   evas_object_size_hint_min_set(ctx_mod->ctxpopup, width, -1);
   display_item_num = width / (w + elm_config_finger_size_get());
   // always display even number of items to avoid autoselection
   if (display_item_num % 2) display_item_num -= 1;
   if (display_item_num < DISKSELECTOR_MIN_ITEMS)
     display_item_num = DISKSELECTOR_MIN_ITEMS;
   elm_diskselector_display_item_num_set(diskselector, display_item_num);
   elm_diskselector_round_enabled_set(diskselector, EINA_TRUE);

   elm_ctxpopup_direction_priority_set(ctx_mod->ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN,
                                       ELM_CTXPOPUP_DIRECTION_UP, -1, -1);
   evas_object_move(ctx_mod->ctxpopup, (x+w/2), (y+h));

   // if the direction of Ctxpopup is upwards, move it to the top of datetime
   if (elm_ctxpopup_direction_get (ctx_mod->ctxpopup) == ELM_CTXPOPUP_DIRECTION_UP)
     {
        elm_ctxpopup_direction_priority_set(ctx_mod->ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
                                            ELM_CTXPOPUP_DIRECTION_DOWN, -1, -1);
        evas_object_move(ctx_mod->ctxpopup, (x+w/2), y);
     }
   evas_object_show(ctx_mod->ctxpopup);
}

static void
_access_set(Evas_Object *obj, Elm_Datetime_Field_Type field_type)
{
   const char* type = NULL;

   switch (field_type)
     {
		 case ELM_DATETIME_YEAR:
        type = "datetime field, year";
        break;

      case ELM_DATETIME_MONTH:
        type = "datetime field, month";
        break;

	  case ELM_DATETIME_DATE:
        type = "datetime field, date";
        break;

      case ELM_DATETIME_HOUR:
        type = "datetime field, hour";
        break;

      case ELM_DATETIME_MINUTE:
        type = "datetime field, minute";
        break;

      case ELM_DATETIME_AMPM:
        type = "datetime field, AM PM";
        break;

      default:
        break;
     }

   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, type);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, NULL, NULL);
}

// module fucns for the specific module type
EAPI void
field_value_display(Elm_Datetime_Module_Data *module_data, Evas_Object *obj)
{
   Ctxpopup_Module_Data *ctx_mod;
   Elm_Datetime_Field_Type  field_type;
   struct tm tim;
   char buf[BUFF_SIZE];
   const char *fmt;

   ctx_mod = (Ctxpopup_Module_Data *)module_data;
   if (!ctx_mod || !obj) return;

   elm_datetime_value_get(ctx_mod->mod_data.base, &tim);
   field_type = (Elm_Datetime_Field_Type )evas_object_data_get(obj, "_field_type");
   fmt = ctx_mod->mod_data.field_format_get(ctx_mod->mod_data.base, field_type);
   buf[0] = 0;
   strftime(buf, sizeof(buf), fmt, &tim);
   if ((!buf[0]) && ((!strcmp(fmt, "%p")) || (!strcmp(fmt, "%P"))))
     {
        // yes BUFF_SIZE is more than 2 bytes!
        if (tim.tm_hour < 12) strcpy(buf, "AM");
        else strcpy(buf, "PM");
     }
   elm_object_text_set(obj, buf);
}

EAPI Evas_Object *
field_create(Elm_Datetime_Module_Data *module_data, Elm_Datetime_Field_Type  field_type)
{
   Ctxpopup_Module_Data *ctx_mod;
   Evas_Object *field_obj;

   ctx_mod = (Ctxpopup_Module_Data *)module_data;
   if (!ctx_mod) return NULL;

   if (field_type == ELM_DATETIME_AMPM)
     {
        field_obj = elm_button_add(ctx_mod->mod_data.base);
        evas_object_smart_callback_add(field_obj, "clicked", _ampm_clicked_cb, ctx_mod);
     }
   else
     {
        field_obj = elm_entry_add(ctx_mod->mod_data.base);
        elm_entry_single_line_set(field_obj, EINA_TRUE);
        elm_entry_editable_set(field_obj, EINA_FALSE);
        elm_entry_input_panel_enabled_set(field_obj, EINA_FALSE);
        elm_entry_context_menu_disabled_set(field_obj, EINA_TRUE);
        evas_object_smart_callback_add(field_obj, "clicked", _field_clicked_cb, ctx_mod);
     }
   evas_object_data_set(field_obj, "_field_type", (void *)field_type);

   // ACCESS
   _access_set(field_obj, field_type);

   return field_obj;
}

EAPI Elm_Datetime_Module_Data *
obj_hook(Evas_Object *obj)
{
   Ctxpopup_Module_Data *ctx_mod;
   char buf[BUFF_SIZE];

   ctx_mod = ELM_NEW(Ctxpopup_Module_Data);
   if (!ctx_mod) return NULL;

   ctx_mod->ctxpopup = elm_ctxpopup_add(elm_widget_top_get(obj));
   snprintf(buf, sizeof(buf), "datetime/%s", elm_object_style_get(obj));
   elm_object_style_set(ctx_mod->ctxpopup, buf);
   elm_ctxpopup_horizontal_set(ctx_mod->ctxpopup, EINA_TRUE);
   evas_object_size_hint_weight_set(ctx_mod->ctxpopup, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ctx_mod->ctxpopup, EVAS_HINT_FILL, 0.5);
   evas_object_smart_callback_add(ctx_mod->ctxpopup, "dismissed",
                                  _ctxpopup_dismissed_cb, ctx_mod);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                  _datetime_resize_cb, ctx_mod);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE,
                                  _datetime_move_cb, ctx_mod);

   return ((Elm_Datetime_Module_Data*)ctx_mod);
}

EAPI void
obj_unhook(Elm_Datetime_Module_Data *module_data)
{
   Ctxpopup_Module_Data *ctx_mod;

   ctx_mod = (Ctxpopup_Module_Data *)module_data;
   if (!ctx_mod) return;

   if (ctx_mod->ctxpopup)
     evas_object_del(ctx_mod->ctxpopup);

   if (ctx_mod)
     {
          free(ctx_mod);
          ctx_mod = NULL;
      }
}

// module api funcs needed
EAPI int
elm_modapi_init(void *m __UNUSED__)
{
   return 1; // succeed always
}

EAPI int
elm_modapi_shutdown(void *m __UNUSED__)
{
   return 1; // succeed always
}
