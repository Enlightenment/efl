#include "private.h"
#include <sys/types.h>
#include <regex.h>

#define BLINK_INTERVAL 0.1

static Elm_Prefs_Item_Type supported_types[] =
{
   ELM_PREFS_TYPE_TEXT,
   ELM_PREFS_TYPE_TEXTAREA,
   ELM_PREFS_TYPE_UNKNOWN
};

static Eina_Bool
_color_change_do(void *data)
{
   Evas_Object *obj = data;
   int color;

   color = (int)(uintptr_t)evas_object_data_get(obj, "current_color");
   switch (color)
     {
      case 0:
        evas_object_data_set(obj, "current_color", (void *)1);
        evas_object_color_set(obj, 255, 0, 0, 255);    /* 1st red */
        goto renew;

      case 1:
        evas_object_data_set(obj, "current_color", (void *)2);
        evas_object_color_set(obj, 255, 255, 255, 255);    /* 2nd white */
        goto renew;

      case 2:
        evas_object_data_set(obj, "current_color", (void *)3);
        evas_object_color_set(obj, 255, 0, 0, 255);    /* 2nd red */
        goto renew;

      case 3:
      default:
        evas_object_data_set(obj, "current_color", (void *)0);
        evas_object_color_set(obj, 255, 255, 255, 255);    /* back to white */
        goto end;
     }

renew:
   return ECORE_CALLBACK_RENEW;

end:
   evas_object_data_del(obj, "timer");
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
elm_prefs_entry_value_validate(Evas_Object *obj)
{
   Ecore_Timer *timer;
   const char *val;
   regex_t *regex;
   size_t min;

   val = elm_entry_entry_get(obj);

   regex = evas_object_data_get(obj, "accept_regex");
   if (regex)
     {
        if (regexec(regex, val, 0, NULL, 0)) goto mismatch;
     }

   regex = evas_object_data_get(obj, "deny_regex");
   if (regex)
     {
        /* we want tokens *out* of the deny language */
        if (!regexec(regex, val, 0, NULL, 0)) goto mismatch;
     }

   min = (size_t) evas_object_data_get(obj, "min_size");
   if (min)
     {
        if (strlen(val) < min) goto mismatch;
     }

   return EINA_TRUE;

mismatch:
   evas_object_color_set(obj, 255, 0, 0, 255);

   timer = evas_object_data_get(obj, "timer");
   if (timer) ecore_timer_del(timer);

   evas_object_data_set
     (obj, "timer", ecore_timer_add(BLINK_INTERVAL, _color_change_do, obj));

   return EINA_FALSE;
}

static void
_item_changed_cb(void *data,
                 Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   Elm_Prefs_Item_Changed_Cb prefs_it_changed_cb = data;

   prefs_it_changed_cb(obj);
}

static void
_entry_del_cb(void *data EINA_UNUSED,
              Evas *evas EINA_UNUSED,
              Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   regex_t *regex;
   Ecore_Timer *timer;

   regex = evas_object_data_del(obj, "accept_regex");
   if (regex) regfree(regex);

   regex = evas_object_data_del(obj, "deny_regex");
   if (regex) regfree(regex);

   timer = evas_object_data_del(obj, "timer");
   if (timer) ecore_timer_del(timer);

   evas_object_data_del(obj, "min_size");
}

static Evas_Object *
elm_prefs_entry_add(const Elm_Prefs_Item_Iface *iface EINA_UNUSED,
                    Evas_Object *prefs,
                    const Elm_Prefs_Item_Type type,
                    const Elm_Prefs_Item_Spec spec,
                    Elm_Prefs_Item_Changed_Cb cb)
{
   Evas_Object *obj = elm_entry_add(prefs);
   regex_t *regex = NULL;
   char buf[256];

   Elm_Entry_Filter_Limit_Size limit = {
        .max_char_count = spec.s.length.max
   };

   evas_object_data_set(obj, "prefs_type", (void *)type);

   /* FIXME: have this warning animation on the theme, later */

   /* 0: orig. white; 1: 1st red; 2: 2nd white; 3: 2o red */
   evas_object_data_set(obj, "current_color", 0);

   /* FIXME: is it worth to ERR with the item's name, too, here? */

   evas_object_smart_callback_add(obj, "activated", _item_changed_cb, cb);
   evas_object_smart_callback_add(obj, "unfocused", _item_changed_cb, cb);

   if (spec.s.accept)
     {
        int ret;

        regex = calloc(1, sizeof(regex_t));
        ret = regcomp(regex, spec.s.accept, REG_EXTENDED | REG_NOSUB);

        if (ret)
          {
             regerror(ret, regex, buf, sizeof(buf));
             regfree(regex);
             ERR("bad regular expression (%s) on item's 'accept' tag (%s)."
                 " Because of that, the 'accept' tag will be dropped for the "
                 "item.", spec.s.accept, buf);
          }
        else
          evas_object_data_set(obj, "accept_regex", regex);
     }

   if (spec.s.deny)
     {
        int ret;

        regex = calloc(1, sizeof(regex_t));
        ret = regcomp(regex, spec.s.deny, REG_EXTENDED | REG_NOSUB);

        if (ret)
          {
             regerror(ret, regex, buf, sizeof(buf));
             regfree(regex);
             ERR("bad regular expression (%s) on item's 'deny' tag (%s)."
                 " Because of that, the 'deny' tag will be dropped for the "
                 "item.", spec.s.deny, buf);
          }
        else
          evas_object_data_set(obj, "deny_regex", regex);
     }

   if (spec.s.length.min) /* zero makes no sense */
     {
        size_t min = (size_t) spec.s.length.min;
        evas_object_data_set(obj, "min_size", (void *) min);
     }

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_DEL, _entry_del_cb, NULL);

   elm_entry_scrollable_set(obj, EINA_TRUE);

   if (type == ELM_PREFS_TYPE_TEXT)
     elm_entry_single_line_set(obj, EINA_TRUE);

   elm_entry_markup_filter_append(obj, elm_entry_filter_limit_size, &limit);
   elm_layout_text_set(obj, NULL, spec.s.placeholder);

   return obj;
}

/* already expects an EINA_VALUE_TYPE_STRINGSHARE one */
static Eina_Bool
elm_prefs_entry_value_set(Evas_Object *obj,
                          Eina_Value *value)
{
   const char *val;

   eina_value_get(value, &val);

   return elm_layout_text_set(obj, NULL, val);
}

static Eina_Bool
elm_prefs_entry_value_get(Evas_Object *obj,
                          Eina_Value *value)
{
   const char *val;

   val = elm_layout_text_get(obj, NULL);

   if (!eina_value_setup(value, EINA_VALUE_TYPE_STRINGSHARE))
     return EINA_FALSE;
   if (!eina_value_set(value, val)) return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_entry_icon_set(Evas_Object *obj,
                         const char *icon)
{
   Evas_Object *ic = elm_icon_add(obj);

   elm_icon_standard_set(ic, icon);

   return elm_layout_content_set(obj, "icon", ic);
}

static Eina_Bool
elm_prefs_entry_editable_set(Evas_Object *obj,
                             Eina_Bool editable)
{
   elm_entry_editable_set(obj, editable);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_entry_editable_get(Evas_Object *obj)
{
   return elm_entry_editable_get(obj);
}

static Eina_Bool
elm_prefs_entry_expand_want(Evas_Object *obj EINA_UNUSED)
{
   return EINA_TRUE;
}

PREFS_ITEM_WIDGET_ADD(entry,
                      supported_types,
                      elm_prefs_entry_value_set,
                      elm_prefs_entry_value_get,
                      elm_prefs_entry_value_validate,
                      NULL,
                      elm_prefs_entry_icon_set,
                      elm_prefs_entry_editable_set,
                      elm_prefs_entry_editable_get,
                      elm_prefs_entry_expand_want);
