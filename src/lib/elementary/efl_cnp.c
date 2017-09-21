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
_efl_cnp_x11_selection_notify(void *udata EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   ERR("in");
   Ecore_X_Event_Selection_Notify *ev = event;
   Ecore_X_Selection_Data *data = ev->data;
   ERR("data: %s (%d), target: %s", (char *)data->data, data->length, ev->target);
   Efl_Cnp_Selection *sd = malloc(sizeof(Efl_Cnp_Selection));
   if (!sd) ERR("failed to allocate sd memory");
   sd->data = malloc(data->length);
   if (!sd->data) ERR("failed to allocate sd->data memory");
   memcpy(sd->data, data->data, data->length);
   //sd->data = strcpy((char *)sd->data, data->data);
   sd->length = data->length;

   Efl_Cnp_Data *pd = udata;
   if (pd->promise)
     {
        ERR("call future");
        efl_promise_value_set(pd->promise, sd, NULL);

        return EINA_TRUE;
     }
   else
     {
        ERR("promise does not exist");
     }

   return EINA_TRUE;
}

EOLIAN static Efl_Future *
_efl_cnp_efl_selection_selection_get(Eo *obj, Efl_Cnp_Data *pd, Efl_Cnp_Type type, Efl_Cnp_Format format, Efl_Input_Device *seat)
{
   ERR("in");
   Efl_Promise *p;

   pd->atom.name = "TARGETS";
   pd->atom.x_atom = ecore_x_atom_get(pd->atom.name);
   ecore_x_selection_converter_atom_add(pd->atom.x_atom, _efl_cnp_x11_target_converter);
   pd->notify_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, _efl_cnp_x11_selection_notify, pd);


   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   ERR("xwin: %d", xwin);
   ecore_x_selection_primary_request(xwin, ECORE_X_SELECTION_TARGET_TARGETS);

   //issue: ctrl-v many times continuously, promise cannot sastify it
   //1st set value to promise, 2nd set value for the same promise -> error
   //another bug case: 1st get selection type A, 2nd get selection type B
   if (!pd->promise)
     {
        Eo *loop = efl_loop_get(obj);
        p = efl_add(EFL_PROMISE_CLASS,loop);
        if (!p) return NULL;

        pd->promise = p;
     }

   return efl_promise_future_get(pd->promise);
}

EOLIAN static void
_efl_cnp_efl_selection_selection_set(Eo *obj, Efl_Cnp_Data *pd, Efl_Cnp_Type type, Efl_Cnp_Format format, const void *buf, int len, Efl_Input_Device *seat)
{
   ERR("in");
   //add implementation
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   ERR("xwin: %d", xwin);
   char *data = malloc(len);
   if (!data) ERR("failed to allocate mem: %d", len);
   memcpy(data, buf, len);
   int ret = ecore_x_selection_primary_set(xwin, data, len);
   ERR("sel set ret: %d", ret);
}

EOLIAN static void
_efl_cnp_efl_selection_selection_clear(Eo *obj, Efl_Cnp_Data *pd, Efl_Cnp_Type type, Efl_Input_Device *seat)
{
    ERR("In");
}

//Selection loss event callback:: name
//future or event???
EOLIAN static Efl_Future *
_efl_cnp_efl_selection_selection_loss_feedback(Eo *obj, Efl_Cnp_Data *pd, Efl_Cnp_Type type)
{
    ERR("In");
    Efl_Promise *p;

    Eo *loop = efl_loop_get(obj);
    p = efl_add(EFL_PROMISE_CLASS, loop);
    if (!p) return NULL;

    pd->promise = p;

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
