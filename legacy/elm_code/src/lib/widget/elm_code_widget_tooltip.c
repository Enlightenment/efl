#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_widget_private.h"

void
_elm_code_widget_tooltip_text_set(Evas_Object *widget, const char *text)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (!text)
     elm_object_tooltip_hide(widget);
   else
     elm_object_tooltip_show(widget);

   if (pd->tooltip) // will have been created by the callback below...
     elm_object_text_set(pd->tooltip, text);
}

static Evas_Object *
_elm_code_widget_tooltip_cb(void *data EINA_UNUSED, Evas_Object *obj, Evas_Object *tooltip)
{
   Elm_Code_Widget_Data *pd;
   Evas_Object *label;

   pd = eo_data_scope_get(obj, ELM_CODE_WIDGET_CLASS);

   label = elm_label_add(tooltip);
   pd->tooltip = label;

   return label;
}

void
_elm_code_widget_tooltip_add(Evas_Object *widget)
{
   elm_object_tooltip_content_cb_set(widget, _elm_code_widget_tooltip_cb, NULL, NULL);
}

