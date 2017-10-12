#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_selection_manager_private.h"

#define MY_CLASS EFL_SELECTION_MANAGER_CLASS
//#define MY_CLASS_NAME "Efl.Selection_Manager"

static Ecore_X_Window
_x11_xwin_get(Evas_Object *obj)
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
_efl_sel_manager_x11_target_converter(char *target, void *data, int size,
                                      void **data_ret, int *size_ret,
                                      Ecore_X_Atom *ttype EINA_UNUSED,
                                      int *typesize EINA_UNUSED)
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

static Eina_Bool
_efl_sel_manager_x11_selection_notify(void *udata, int type EINA_UNUSED, void *event)
{
   ERR("in");
   Efl_Selection_Manager_Data *pd = udata;
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
   //ecore_event_handler_del(pd->notify_handler);
   //pd->notify_handler = NULL;

   return EINA_TRUE;
}


#ifdef HAVE_ELEMENTARY_X
static void
_x11_efl_sel_manager_selection_set(Eo *obj, Efl_Selection_Manager_Data *pd,
                                   Efl_Selection_Type type, Efl_Selection_Format format,
                                   const void *buf, int len, Efl_Input_Device *seat)
{
   Ecore_X_Window xwin = _x11_xwin_get(obj);

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

static void
_selection_loss_data_clear_cb(void *data)
{
   Efl_Selection_Type *lt = data;
   free(lt);
}


EOLIAN static void
_efl_selection_manager_selection_set(Eo *obj, Efl_Selection_Manager_Data *pd,
                                     Efl_Object *owner, Efl_Selection_Type type,
                                     Efl_Selection_Format format,
                                     const void *buf, int len, Efl_Input_Device *seat)
{
   ERR("In");
   if (type > EFL_SELECTION_TYPE_CLIPBOARD)
     {
        ERR("Not supported format: %d", type);
        return;
     }

#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_xwin_get(owner);
#endif
   //check if owner is changed
   if (pd->sel_owner != NULL &&
       pd->sel_owner != owner
#ifdef HAVE_ELEMENTARY_X
       //support 1 app with multiple window, 1 selection manager
       && pd->xwin == xwin
#endif
      )
     {
        /*//call selection_loss callback: should we include prev owner??
        Efl_Selection_Type *lt = malloc(sizeof(Efl_Selection_Type));
        *lt = pd->loss_type;
        efl_promise_value_set(pd->promise, lt, _selection_loss_data_clear_cb);*/

        efl_event_callback_call(pd->sel_owner, EFL_SELECTION_EVENT_SELECTION_LOSS, NULL);
     }

   pd->sel_owner = owner;
#ifdef HAVE_ELEMENTARY_X
   pd->xwin = xwin;

   return _x11_efl_sel_manager_selection_set(obj, pd, type, format, buf, len, seat);
#endif
#ifdef HAVE_ELEMENTARY_WL2
#endif
#ifdef HAVE_ELEMENTARY_COCOA
#endif
#ifdef HAVE_ELEMENTARY_WIN32
#endif
}


EOLIAN static void
_efl_selection_manager_selection_get(Eo *obj, Efl_Selection_Manager_Data *pd,
                                     Efl_Object *owner, Efl_Selection_Type type,
                                     Efl_Selection_Format format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb,
                                     Efl_Input_Device *seat)
{
   ERR("In");

   pd->atom.name = "TARGETS";
   pd->atom.x_atom = ecore_x_atom_get(pd->atom.name);
   ecore_x_selection_converter_atom_add(pd->atom.x_atom, _efl_sel_manager_x11_target_converter);

   Ecore_X_Window xwin = _x11_xwin_get(obj);
   ERR("xwin: %d", xwin);
   ecore_x_selection_primary_request(xwin, ECORE_X_SELECTION_TARGET_TARGETS);

   pd->obj = obj;
   pd->data_func_data = data_func_data;
   pd->data_func = data_func;
   pd->data_func_free_cb = data_func_free_cb;
}


EOLIAN static void
_efl_selection_manager_selection_clear(Eo *obj, Efl_Selection_Manager_Data *pd,
                                       Efl_Object *owner, Efl_Selection_Type type, Efl_Input_Device *seat)
{
   //no need to call loss cb here: it will be called from WM
   ERR("In");
   if (type > EFL_SELECTION_TYPE_CLIPBOARD)
     {
        ERR("Not supported type: %d", type);
        return;
     }
   if (pd->sel_owner != owner)
     {
        return;
     }
   pd->sel_owner = NULL;
   if (type == EFL_SELECTION_TYPE_PRIMARY)
     {
        ecore_x_selection_primary_clear();
     }
   else if (type == EFL_SELECTION_TYPE_SECONDARY)
     {
        ecore_x_selection_secondary_clear();
     }
   else if (type == EFL_SELECTION_TYPE_DND)
     {
     }
   else if (type == EFL_SELECTION_TYPE_CLIPBOARD)
     {
        ecore_x_selection_clipboard_clear();
     }
}

static Eina_Bool
_x11_selection_clear(void *data, int type EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = data;
   ERR("In");
   /*if (pd->promise)
     {
        Efl_Selection_Type *lt = malloc(sizeof(Efl_Selection_Type));
        *lt = pd->loss_type;
        efl_promise_value_set(pd->promise, lt, _selection_loss_data_clear_cb);
        pd->promise = NULL;
     }*/
   efl_event_callback_call(pd->sel_owner, EFL_SELECTION_EVENT_SELECTION_LOSS, NULL);
   //clear the buffer
   //

   return ECORE_CALLBACK_PASS_ON;
}

/*
EOLIAN static Efl_Future *
_efl_selection_manager_selection_loss_feedback(Eo *obj, Efl_Selection_Manager_Data *pd,
                                               Efl_Object *owner, Efl_Selection_Type type)
{
   ERR("In");
   Efl_Promise *p;
   Eo *loop = efl_loop_get(obj);

   pd->promise = NULL;
   p = efl_add(EFL_PROMISE_CLASS, loop);
   if (!p) return NULL;
   pd->promise = p;
   pd->loss_type = type;

   return efl_promise_future_get(p);
}*/

static Efl_Object *
_efl_selection_manager_efl_object_constructor(Eo *obj, Efl_Selection_Manager_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->notify_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,
                                                _efl_sel_manager_x11_selection_notify, pd);
   pd->clear_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR,
                                               _x11_selection_clear, pd);
   return obj;
}


static void
_efl_selection_manager_efl_object_destructor(Eo *obj, Efl_Selection_Manager_Data *pd)
{
   ecore_event_handler_del(pd->notify_handler);
   ecore_event_handler_del(pd->clear_handler);

   efl_destructor(efl_super(obj, MY_CLASS));
}



#include "efl_selection_manager.eo.c"
