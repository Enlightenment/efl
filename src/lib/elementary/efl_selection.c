#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_SELECTION_MANAGER_BETA

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_SELECTION_MIXIN
#define MY_CLASS_NAME "Efl.Selection"

static inline Eo*
_selection_manager_get(Eo *obj)
{
   Eo *top = elm_widget_top_get(obj);
   if (!top)
     {
        top = obj;
     }
   Eo *sel_man = efl_key_data_get(top, "__selection_manager");
   if (!sel_man)
     {
        sel_man = efl_add(EFL_SELECTION_MANAGER_CLASS, top);
        efl_key_data_set(top, "__selection_manager", sel_man);
     }
   return sel_man;
}

EOLIAN static void
_efl_selection_selection_get(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Type type, Efl_Selection_Format format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb, unsigned int seat)
{
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_selection_get(sel_man, obj, type, format,
                                       data_func_data, data_func,
                                       data_func_free_cb, seat);
}

EOLIAN static Eina_Future *
_efl_selection_selection_set(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Type type, Efl_Selection_Format format, Eina_Slice data, unsigned int seat)
{
   Eo *sel_man = _selection_manager_get(obj);
   return efl_selection_manager_selection_set(sel_man, obj, type, format, data, seat);
}

EOLIAN static void
_efl_selection_selection_clear(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Type type, unsigned int seat)
{
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_selection_clear(sel_man, obj, type, seat);
}

EOLIAN static Eina_Bool
_efl_selection_has_owner(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Type type, unsigned int seat)
{
    Eo *sel_man = _selection_manager_get(obj);
    return efl_selection_manager_selection_has_owner(sel_man, obj, type, seat);
}


////////// Support legacy APIs

//TODO: Clear this list (when sel_man is deleted)
Eina_List *lost_cb_list = NULL;

#ifdef HAVE_ELEMENTARY_WL2
static Ecore_Evas *
_wl_is_wl(const Evas_Object *obj)
{
   Ecore_Evas *ee;
   Evas *evas;
   const char *engine_name;

   if (!(evas = evas_object_evas_get(obj)))
     return NULL;
   if (!(ee = ecore_evas_ecore_evas_get(evas)))
     return NULL;

   engine_name = ecore_evas_engine_name_get(ee);
   if (!strcmp(engine_name, ELM_BUFFER))
     {
        ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
        if (!ee) return NULL;
        engine_name = ecore_evas_engine_name_get(ee);
     }
   if (!strncmp(engine_name, "wayland", sizeof("wayland") - 1))
     return ee;
   return NULL;
}

static Ecore_Wl2_Window *
_wl_window_get(const Evas_Object *obj)
{
   Evas_Object *top;
   Ecore_Wl2_Window *win = NULL;

   if (elm_widget_is(obj))
     {
        top = elm_widget_top_get(obj);
        if (!top) top = elm_widget_top_get(elm_widget_parent_widget_get(obj));
        if (top && (efl_isa(top, EFL_UI_WIN_CLASS)))
            win = elm_win_wl_window_get(top);
     }
   if (!win)
     {
        Ecore_Evas *ee = _wl_is_wl(obj);

        if (ee)
          {
             /* In case the engine is not a buffer, we want to check once. */
             win = ecore_evas_wayland2_window_get(ee);
             if (!win) return NULL;
          }
     }

   return win;
}

int
_wl_default_seat_id_get(Evas_Object *obj)
{
   Ecore_Wl2_Window *win = _wl_window_get(obj);
   Eo *seat, *parent2, *ewin;
   Eina_Bool is_wl = EINA_FALSE;

   if (obj)
     {
        if (_wl_is_wl(obj)) is_wl = EINA_TRUE;
        if (efl_isa(obj, EFL_UI_WIDGET_CLASS))
          {
             Eo *top = elm_widget_top_get(obj);
             if (efl_isa(top, EFL_UI_WIN_INLINED_CLASS))
               {
                  parent2 = efl_ui_win_inlined_parent_get(top);
                  if (parent2) obj = elm_widget_top_get(parent2) ?: parent2;
               }
             /* fake win means canvas seat id will not match protocol seat id */
             ewin = elm_win_get(obj);
             if (elm_win_type_get(ewin) == ELM_WIN_FAKE) obj = NULL;
          }
     }

   if (!obj)
     {
        if (is_wl)
          {
             Ecore_Wl2_Input *input;
             Eina_Iterator *it;

             it = ecore_wl2_display_inputs_get(ecore_wl2_window_display_get(win));
             EINA_ITERATOR_FOREACH(it, input) break;
             eina_iterator_free(it);
             if (input)
               return ecore_wl2_input_seat_id_get(input);
          }
     }

   seat = evas_default_device_get(evas_object_evas_get(obj), EFL_INPUT_DEVICE_TYPE_SEAT);
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, 1);
   return evas_device_seat_id_get(seat);
}
#endif

typedef struct _Cnp_Data_Cb_Wrapper Cnp_Data_Cb_Wrapper;
struct _Cnp_Data_Cb_Wrapper
{
   void *udata;
   Elm_Drop_Cb datacb;
};

static void
_selection_data_ready_cb(void *data, Efl_Object *obj, Efl_Selection_Data *seldata)
{
   printf("obj: %p, data: [", obj);
   fwrite(seldata->content.mem, seldata->content.len, 1, stdout);
   printf("], length: %zd\n", seldata->content.len);

    Cnp_Data_Cb_Wrapper *wdata = data;
    if (!wdata) return;
    Elm_Selection_Data ddata;

    ddata.data = calloc(1, seldata->content.len + 1);
    if (!ddata.data) return;
    ddata.data = memcpy(ddata.data, seldata->content.mem, seldata->content.len);
    ddata.len = seldata->content.len;
    ddata.x = seldata->pos.x;
    ddata.y = seldata->pos.y;
    ddata.format = (Elm_Sel_Format)seldata->format;
    ddata.action = (Elm_Xdnd_Action)seldata->action;
    wdata->datacb(wdata->udata, obj, &ddata);
    free(ddata.data);
}

typedef struct _Sel_Lost_Data Sel_Lost_Data;
struct _Sel_Lost_Data
{
   const Evas_Object *obj;
   Elm_Sel_Type type;
   void *udata;
   Elm_Selection_Loss_Cb loss_cb;
};

static Eina_Value
_selection_lost_cb(void *data, const Eina_Value value)
{
   Eina_List *l, *l2;
   Sel_Lost_Data *ldata, *ldata2;

   ldata = data;
   EINA_LIST_FOREACH_SAFE(lost_cb_list, l, l2, ldata2)
     {
        if ((ldata->obj == ldata2->obj) &&
            (ldata->type == ldata2->type))
          {
             ldata2->loss_cb(ldata2->udata, ldata2->type);
             lost_cb_list = eina_list_remove(lost_cb_list, ldata2);
          }
     }
   free(ldata);

   return value;
}

EAPI Eina_Bool
elm_cnp_selection_get(const Evas_Object *obj, Elm_Sel_Type type,
                      Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata)
{
   int seatid = 1;
   Eo *sel_man = _selection_manager_get((Evas_Object *)obj);
   Cnp_Data_Cb_Wrapper *wdata = calloc(1, sizeof(Cnp_Data_Cb_Wrapper));

   if (!wdata) return EINA_FALSE;

#ifdef HAVE_ELEMENTARY_WL2
   
   seatid = _wl_default_seat_id_get((Evas_Object *)obj);
#endif
   wdata->udata = udata;
   wdata->datacb = datacb;
   efl_selection_manager_selection_get(sel_man, (Evas_Object *)obj, (Efl_Selection_Type)type,
                                       (Efl_Selection_Format)format,
                                       wdata, _selection_data_ready_cb, NULL, seatid);
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_cnp_selection_set(Evas_Object *obj, Elm_Sel_Type type,
                      Elm_Sel_Format format, const void *selbuf, size_t buflen)
{
   int seatid = 1;
   Eina_Future *f;
   Sel_Lost_Data *ldata;
   Eo *sel_man = _selection_manager_get(obj);
   Eina_Slice data;

   ldata = calloc(1, sizeof(Sel_Lost_Data));
   if (!ldata) return EINA_FALSE;
   data.mem = selbuf;
   data.len = buflen;
#ifdef HAVE_ELEMENTARY_WL2
   seatid = _wl_default_seat_id_get(obj);
#endif
   f = efl_selection_manager_selection_set(sel_man, obj, (Efl_Selection_Type)type,
                                           (Efl_Selection_Format)format, data, seatid);

   ldata->obj = obj;
   ldata->type = type;
   eina_future_then_easy(f, _selection_lost_cb, NULL, NULL, EINA_VALUE_TYPE_UINT, ldata);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_object_cnp_selection_clear(Evas_Object *obj, Elm_Sel_Type type)
{
   int seatid = 1;
   Eo *sel_man = _selection_manager_get((Evas_Object *)obj);

#ifdef HAVE_ELEMENTARY_WL2
   seatid = _wl_default_seat_id_get(obj);
#endif
   efl_selection_manager_selection_clear(sel_man, obj, (Efl_Selection_Type)type, seatid);

   return EINA_TRUE;
}

EAPI void
elm_cnp_selection_loss_callback_set(Evas_Object *obj, Elm_Sel_Type type,
                                    Elm_Selection_Loss_Cb func, const void *data)
{
   Sel_Lost_Data *ldata = calloc(1, sizeof(Sel_Lost_Data));
   if (!ldata) return;
   ldata->obj = obj;
   ldata->type = type;
   ldata->udata = (void *)data;
   ldata->loss_cb = func;
   lost_cb_list = eina_list_append(lost_cb_list, ldata);
}

EAPI Eina_Bool
elm_selection_selection_has_owner(Evas_Object *obj)
{
   int seatid = 1;
   Eo *sel_man = _selection_manager_get((Evas_Object *)obj);

#ifdef HAVE_ELEMENTARY_WL2
   seatid = _wl_default_seat_id_get(obj);
#endif

   return efl_selection_manager_selection_has_owner(sel_man, obj,
                                                    EFL_SELECTION_TYPE_CLIPBOARD, seatid);
}

EAPI Eina_Bool
elm_cnp_clipboard_selection_has_owner(Evas_Object *obj)
{
   int seatid = 1;
   Eo *sel_man = _selection_manager_get((Evas_Object *)obj);

#ifdef HAVE_ELEMENTARY_WL2
   seatid = _wl_default_seat_id_get(obj);
#endif
   return efl_selection_manager_selection_has_owner(sel_man, obj,
                                                    EFL_SELECTION_TYPE_CLIPBOARD, seatid);
}

#include "efl_selection.eo.c"
