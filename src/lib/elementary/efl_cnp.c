#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

//#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
//#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
//#define ELM_INTERFACE_ATSPI_TEXT_EDITABLE_PROTECTED
//#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
//#include <Elementary_Cursor.h>
#include "elm_priv.h"

#include "efl_cnp_private.h"

//#define MY_CLASS EFL_CNP_CLASS
#define MY_CLASS EFL_CNP_MIXIN
#define MY_CLASS_NAME "Efl.Cnp"
//#define MY_CLASS_NAME_LEGACY "elm_cnp"

static Ecore_X_Window
_x11_elm_widget_xwin_get(Evas_Object *obj)
{
   if (!obj) return 0;

   Ecore_X_Window xwin = 0;
   //get top
   Evas_Object *top = obj;
   Evas_Object *parent = obj;
   while(parent)
     {
        top = parent;
        parent = efl_parent_get(parent);
     }
   if (efl_isa(top, EFL_UI_WIN_CLASS))
     {
        xwin = elm_win_xwindow_get(top);
     }
   if (!xwin)
     {
        Ecore_Evas *ee;
        Evas *evas = evas_object_evas_get(obj);
        if (!evas) return 0;
        ee = ecore_evas_ecore_evas_get(evas);
        if (!ee) return 0;

        while(!xwin)
          {
             const char *engine_name = ecore_evas_engine_name_get(ee);
             if (!strcmp(engine_name, ELM_BUFFER))
               {
                  ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
                  if (!ee) return 0;
                  xwin = _elm_ee_xwin_get(ee);
               }
             else
               {
                  xwin = _elm_ee_xwin_get(ee);
                  if (!xwin) return 0;
               }
          }
     }

   return xwin;
}


static Eina_Bool
_efl_cnp_x11_target_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
{
   ERR("In:: target: %s, data: %s, size: %d", target, (char *)data, size);
   if (data_ret) *data_ret = strdup(data);
   if (*size_ret) *size_ret = size;

   return EINA_TRUE;
}

static Eina_Bool
_x11_notify_text(Ecore_X_Event_Selection_Notify *notify, Elm_Selection_Data *ddata)
{
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = ELM_SEL_FORMAT_TEXT;
   ddata->data = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->len = data->length;

   return EINA_TRUE;
}

//#define EFL_LOOP_FUTURE_CLASS efl_loop_future_class_get()


static Eina_Bool
_efl_cnp_x11_selection_notify(void *udata, int type EINA_UNUSED, void *event)
{
   ERR("in");
   Efl_Cnp_Data *pd = udata;
   Ecore_X_Event_Selection_Notify *ev = event;
   Ecore_X_Selection_Data *data = ev->data;
   ERR("data: %s (%d), target: %s", (char *)data->data, data->length, ev->target);
   Efl_Cnp_Selection *sd = malloc(sizeof(Efl_Cnp_Selection));
   if (!sd) ERR("failed to allocate sd memory");
   sd->data = malloc(data->length);
   if (!sd->data) ERR("failed to allocate sd->data memory");
   memcpy(sd->data, data->data, data->length);
   sd->length = data->length;

   if (pd->data_func)
     {
        ERR("call function data pointer");
        pd->data_func(pd->data_func_data, pd->obj, sd->data, sd->length);
        pd->data_func_data = NULL;
        pd->data_func = NULL;
        pd->data_func_free_cb = NULL;
     }
   ecore_event_handler_del(pd->notify_handler);
   pd->notify_handler = NULL;

   return EINA_TRUE;
}

EOLIAN static void
_efl_cnp_efl_selection_selection_get(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type, Efl_Selection_Format format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb, Efl_Input_Device *seat)
{
   ERR("in");

   pd->atom.name = "TARGETS";
   pd->atom.x_atom = ecore_x_atom_get(pd->atom.name);
   ecore_x_selection_converter_atom_add(pd->atom.x_atom, _efl_cnp_x11_target_converter);
   pd->notify_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, _efl_cnp_x11_selection_notify, pd);

   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   ERR("xwin: %d", xwin);
   ecore_x_selection_primary_request(xwin, ECORE_X_SELECTION_TARGET_TARGETS);

   pd->obj = obj;
   pd->data_func_data = data_func_data;
   pd->data_func = data_func;
   pd->data_func_free_cb = data_func_free_cb;
}

static void
_local_efl_cnp_selection_set(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type, Efl_Selection_Format format, const void *buf, int len, Efl_Input_Device *seat)
{
   ERR("In");
}

#ifdef HAVE_ELEMENTARY_X
static void
_x11_efl_cnp_selection_set(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type, Efl_Selection_Format format, const void *buf, int len, Efl_Input_Device *seat)
{
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);

   pd->has_sel = EINA_TRUE;
   pd->buf = malloc(len);
   if (!pd->buf)
     {
        ERR("failed to allocate memory");
     }
   pd->buf = memcpy(pd->buf, buf, len);
   pd->len = len;
   pd->type = type;
   pd->format = format;

   switch (type)
     {
      case EFL_SELECTION_TYPE_PRIMARY:
         ecore_x_selection_primary_set(xwin, pd->buf, pd->len);
         break;
      case EFL_SELECTION_TYPE_SECONDARY:
         ecore_x_selection_secondary_set(xwin, pd->buf, pd->len);
         break;
      case EFL_SELECTION_TYPE_DND: //FIXME: Check
         ecore_x_selection_xdnd_set(xwin, pd->buf, pd->len);
         break;
      case EFL_SELECTION_TYPE_CLIPBOARD:
         ecore_x_selection_clipboard_set(xwin, pd->buf, pd->len);
         break;
      default:
         break;
     }
}
#endif

EOLIAN static void
_efl_cnp_efl_selection_selection_set(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type, Efl_Selection_Format format, const void *buf, int len, Efl_Input_Device *seat)
{
   ERR("in");
   if (type > EFL_SELECTION_TYPE_CLIPBOARD)
     {
        ERR("Not supported format: %d", type);
        return;
     }
#ifdef HAVE_ELEMENTARY_X
   return _x11_efl_cnp_selection_set(obj, pd, type, format, buf, len, seat);
   /*Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   ERR("xwin: %d", xwin);
   char *data = malloc(len);
   if (!data) ERR("failed to allocate mem: %d", len);
   memcpy(data, buf, len);
   int ret = ecore_x_selection_primary_set(xwin, data, len);
   ERR("sel set ret: %d", ret);*/
#endif
#ifdef HAVE_ELEMENTARY_WL2
#endif
#ifdef HAVE_ELEMENTARY_COCOA
#endif
#ifdef HAVE_ELEMENTARY_WIN32
#endif
   _local_efl_cnp_selection_set(obj, pd, type, format, buf, len, seat);
}

EOLIAN static void
_efl_cnp_efl_selection_selection_clear(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type, Efl_Input_Device *seat)
{
    ERR("In");
}

//issue: selection clear only come with window-level
//if a window has two entries, selection moves from one entry to the other
//the selection clear does not come (still in that window)
//fix: make selection manager: 1 selection manager per one window
//it manages selection, calls selection loss even selection moves inside window.
//Implementation: has SelMan mixin, ui_win includes it.
//this cnp handles all converter, while SelMan manages selection requests.
static Eina_Bool
_x11_efl_cnp_selection_clear_cb(void *data, int type, void *event)
{
   ERR("In");
   Efl_Cnp_Data *pd = data;
   Ecore_X_Event_Selection_Clear *ev = event;

   if (pd->has_sel)
     {
        ERR("call selection clear");
        efl_promise_value_set(pd->promise, NULL, NULL);
        pd->has_sel = EINA_FALSE;
        pd->len = 0;
        ELM_SAFE_FREE(pd->buf, free);

        return ECORE_CALLBACK_PASS_ON;
     }
   return ECORE_CALLBACK_PASS_ON;
}

//Selection loss event callback:: name
//future or event???
EOLIAN static Efl_Future *
_efl_cnp_efl_selection_selection_loss_feedback(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type)
{
    ERR("In");
    Efl_Promise *p;

    Eo *loop = efl_loop_get(obj);
    p = efl_add(EFL_PROMISE_CLASS, loop);
    if (!p) return NULL;

    pd->promise = p;

    ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, _x11_efl_cnp_selection_clear_cb, pd);

    return efl_promise_future_get(p);
}


/*
EOLIAN static Eo*
_efl_cnp_efl_object_constructor(Eo *obj, Efl_Cnp_Data *_pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   _pd->atom.name = "TARGETS";
   _pd->atom.x_atom = ecore_x_atom_get(_pd->atom.name);
   ecore_x_selection_converter_atom_add(_pd->atom.x_atom, _efl_cnp_x11_target_converter);
   _pd->notify_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, _efl_cnp_x11_selection_notify, _pd);

   return obj;
}

EOLIAN static void
_efl_cnp_efl_object_destructor(Eo *obj, Efl_Cnp_Data *_pd)
{
   efl_destructor(efl_super(obj, MY_CLASS));

   ecore_event_handler_del(_pd->notify_handler);
}*/
#include "efl_cnp.eo.c"
