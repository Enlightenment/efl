#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Eo.h>
#include <Evas.h>

#include "Elementary.h"

#include "elm_code_private.h"

EAPI const Efl_Event_Description ELM_CODE_EVENT_LINE_LOAD_DONE =
    EFL_EVENT_DESCRIPTION("line,load,done");
EAPI const Efl_Event_Description ELM_CODE_EVENT_FILE_LOAD_DONE =
    EFL_EVENT_DESCRIPTION("file,load,done");


EAPI Elm_Code *
elm_code_create(void)
{
   Elm_Code *ret;

   ret = calloc(1, sizeof(Elm_Code));
   if (!ret) return NULL;
   ret->config.indent_style_efl = EINA_TRUE;

   // create an in-memory backing for this elm_code by default
   elm_code_file_new(ret);
   return ret;
}

EAPI void
elm_code_free(Elm_Code *code)
{
   Evas_Object *widget;
   Elm_Code_Parser *parser;

   if (code->file)
     elm_code_file_free(code->file);

   EINA_LIST_FREE(code->widgets, widget)
     {
        evas_object_hide(widget);
        evas_object_del(widget);
     }

   EINA_LIST_FREE(code->parsers, parser)
     {
        _elm_code_parser_free(parser);
     }

   free(code);
}

EAPI void
elm_code_callback_fire(Elm_Code *code, const Efl_Event_Description *signal, void *data)
{
   Eina_List *item;
   Eo *widget;

   EINA_LIST_FOREACH(code->widgets, item, widget)
     {
        efl_event_callback_legacy_call(widget, signal, data);
     }
}

