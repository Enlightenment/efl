#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Input.h>
#include <Ecore_IMF.h>
#include <Ecore_X.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <langinfo.h>
#include <assert.h>

static int _ecore_imf_xim_log_dom = -1;

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_imf_xim_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_imf_xim_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_imf_xim_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_imf_xim_log_dom, __VA_ARGS__)


static Eina_List *open_ims = NULL;

#define FEEDBACK_MASK (XIMReverse | XIMUnderline | XIMHighlight)

typedef struct _XIM_Im_Info XIM_Im_Info;

typedef struct _Ecore_IMF_Context_Data Ecore_IMF_Context_Data;

struct _XIM_Im_Info
{
   Ecore_X_Window win;
   Ecore_IMF_Context_Data *user;
   char          *locale;
   XIM            im;
   Eina_List     *ics;
   Eina_Bool      reconnecting;
   XIMStyles     *xim_styles;
   Eina_Bool      supports_string_conversion : 1;
   Eina_Bool      supports_cursor : 1;
};

struct _Ecore_IMF_Context_Data
{
   Ecore_X_Window win;
   long           mask;
   XIC            ic; /* Input context for composed characters */
   char          *locale;
   XIM_Im_Info   *im_info;
   int            preedit_length;
   int            preedit_cursor;
   Eina_Unicode  *preedit_chars;
   Eina_Bool      use_preedit;
   Eina_Bool      finalizing;
   Eina_Bool      has_focus;
   Eina_Bool      in_toplevel;
   XIMFeedback   *feedbacks;

   XIMCallback    destroy_cb;

   XIMCallback    preedit_start_cb;
   XIMCallback    preedit_done_cb;
   XIMCallback    preedit_draw_cb;
   XIMCallback    preedit_caret_cb;
};

/* prototype */
static Ecore_IMF_Context_Data *_ecore_imf_xim_context_data_new(void);
static void                    _ecore_imf_xim_context_data_destroy(Ecore_IMF_Context_Data *imf_context_data);

static void          _ecore_imf_xim_feedback_attr_add(Eina_List **attrs,
                                                     const char *str,
                                                     XIMFeedback feedback,
                                                     int start_pos,
                                                     int end_pos);

static void          _ecore_imf_xim_ic_reinitialize(Ecore_IMF_Context *ctx);
static void          _ecore_imf_xim_ic_client_window_set(Ecore_IMF_Context *ctx,
                                                        Ecore_X_Window window);
static int           _ecore_imf_xim_preedit_start_call(XIC xic,
                                                      XPointer client_data,
                                                      XPointer call_data);
static void          _ecore_imf_xim_preedit_done_call(XIC xic,
                                                     XPointer client_data,
                                                     XPointer call_data);
static void          _ecore_imf_xim_preedit_draw_call(XIC xic,
                                                     XPointer client_data,
                                                     XIMPreeditDrawCallbackStruct *call_data);
static void          _ecore_imf_xim_preedit_caret_call(XIC xic,
                                                      XPointer client_data,
                                                      XIMPreeditCaretCallbackStruct *call_data);

static int           _ecore_imf_xim_text_to_utf8(Ecore_IMF_Context *ctx,
                                                XIMText *xim_text,
                                                char **text);

static XVaNestedList _ecore_imf_xim_preedit_callback_set(Ecore_IMF_Context *ctx);
static XIC           _ecore_imf_xim_ic_get(Ecore_IMF_Context *ctx);
static XIM_Im_Info  *_ecore_imf_xim_im_get(Ecore_X_Window window,
                                          char *locale);
static void          _ecore_imf_xim_info_im_init(XIM_Im_Info *info);
static void          _ecore_imf_xim_info_im_shutdown(Ecore_X_Display *display,
                                             int is_error,
                                             XIM_Im_Info *info);
static void          _ecore_imf_xim_instantiate_cb(Display *display,
                                                  XPointer client_data,
                                                  XPointer call_data);
static void          _ecore_imf_xim_destroy_cb(XIM xim,
                                              XPointer client_data,
                                              XPointer call_data);
static void          _ecore_imf_xim_im_setup(XIM_Im_Info *info);

static unsigned int
_ecore_imf_xim_utf8_offset_to_index(const char *str, int offset)
{
   int idx = 0;
   int i;
   for (i = 0; i < offset; i++)
     {
        eina_unicode_utf8_next_get(str, &idx);
     }

   return idx;
}

static void
_ecore_imf_context_xim_add(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data = _ecore_imf_xim_context_data_new();
   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   imf_context_data->use_preedit = EINA_TRUE;
   imf_context_data->finalizing = EINA_FALSE;
   imf_context_data->has_focus = EINA_FALSE;
   imf_context_data->in_toplevel = EINA_FALSE;

   ecore_imf_context_data_set(ctx, imf_context_data);
}

static void
_ecore_imf_context_xim_del(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   imf_context_data->finalizing = EINA_TRUE;
   if (imf_context_data->im_info && !imf_context_data->im_info->ics->next)
     {
        if (imf_context_data->im_info->reconnecting == EINA_TRUE)
          {
             Ecore_X_Display *dsp;
             dsp = ecore_x_display_get();
             if (dsp)
               XUnregisterIMInstantiateCallback(dsp,
                                                NULL, NULL, NULL,
                                                _ecore_imf_xim_instantiate_cb,
                                                (XPointer)imf_context_data->im_info);
          }
        else if (imf_context_data->im_info->im)
          {
             if (ecore_x_display_get())
               {
                  XIMCallback im_destroy_callback;
                  im_destroy_callback.client_data = NULL;
                  im_destroy_callback.callback = NULL;
                  XSetIMValues(imf_context_data->im_info->im,
                               XNDestroyCallback, &im_destroy_callback,
                               NULL);
               }
          }
     }

   _ecore_imf_xim_ic_client_window_set(ctx, 0);

   _ecore_imf_xim_context_data_destroy(imf_context_data);
}

static void
_ecore_imf_context_xim_client_window_set(Ecore_IMF_Context *ctx,
                                         void *window)
{
   DBG("ctx=%p, window=%p", ctx, window);
   _ecore_imf_xim_ic_client_window_set(ctx, (Ecore_X_Window)((unsigned long)window));
}

static void
_ecore_imf_context_xim_preedit_string_get(Ecore_IMF_Context *ctx,
                                          char **str,
                                          int *cursor_pos)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   char *utf8;
   int len;

   DBG("ctx=%p, imf_context_data=%p, str=%p, cursor_pos=%p",
       ctx, imf_context_data, str, cursor_pos);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   if (imf_context_data->preedit_chars)
     {
        utf8 = eina_unicode_unicode_to_utf8(imf_context_data->preedit_chars,
                                            &len);
        if (str)
          *str = utf8;
        else
          free(utf8);
     }
   else
     {
        if (str)
          *str = NULL;
     }

   if (cursor_pos)
     *cursor_pos = imf_context_data->preedit_cursor;
}

static void
_ecore_imf_context_xim_preedit_string_with_attributes_get(Ecore_IMF_Context *ctx,
                                                          char **str,
                                                          Eina_List **attrs,
                                                          int *cursor_pos)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);

   DBG("ctx=%p, imf_context_data=%p, str=%p, attrs=%p, cursor_pos=%p",
       ctx, imf_context_data, str, attrs, cursor_pos);

   _ecore_imf_context_xim_preedit_string_get(ctx, str, cursor_pos);

   if (!attrs) return;
   if (!imf_context_data || !imf_context_data->feedbacks) return;

   int i = 0;
   XIMFeedback last_feedback = 0;
   int start = -1;

   for (i = 0; i < imf_context_data->preedit_length; i++)
     {
        XIMFeedback new_feedback = imf_context_data->feedbacks[i] & FEEDBACK_MASK;

        if (new_feedback != last_feedback)
          {
             if (start >= 0)
               _ecore_imf_xim_feedback_attr_add(attrs, *str, last_feedback, start, i);

             last_feedback = new_feedback;
             start = i;
          }
     }

   if (start >= 0)
     _ecore_imf_xim_feedback_attr_add(attrs, *str, last_feedback, start, i);
}

static void
_ecore_imf_context_xim_focus_in(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data  = ecore_imf_context_data_get(ctx);
   XIC ic;

   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   ic = imf_context_data->ic;
   imf_context_data->has_focus = EINA_TRUE;

   if (ecore_imf_context_input_panel_enabled_get(ctx))
     ecore_imf_context_input_panel_show(ctx);

   if (ic)
     {
        char *str;

#ifdef X_HAVE_UTF8_STRING
        if ((str = Xutf8ResetIC(ic)))
#else
        if ((str = XmbResetIC(ic)))
#endif
          XFree(str);

        XSetICFocus(ic);
     }
}

static void
_ecore_imf_context_xim_focus_out(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   XIC ic;

   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   if (imf_context_data->has_focus == EINA_TRUE)
     {
        imf_context_data->has_focus = EINA_FALSE;
        ic = imf_context_data->ic;
        if (ic)
          XUnsetICFocus(ic);

        if (ecore_imf_context_input_panel_enabled_get(ctx))
          ecore_imf_context_input_panel_hide(ctx);
     }
}

static void
_ecore_imf_context_xim_reset(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   XIC ic;
   char *result;
   /* restore conversion state after resetting ic later */
   XIMPreeditState preedit_state = XIMPreeditUnKnown;
   XVaNestedList preedit_attr;
   Eina_Bool have_preedit_state = EINA_FALSE;

   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   ic = imf_context_data->ic;
   if (!ic)
     return;

   if (imf_context_data->preedit_length == 0)
     return;

   preedit_attr = XVaCreateNestedList(0,
                                      XNPreeditState, &preedit_state,
                                      NULL);
   if (!XGetICValues(ic,
                     XNPreeditAttributes, preedit_attr,
                     NULL))
     have_preedit_state = EINA_TRUE;

   XFree(preedit_attr);

   result = XmbResetIC(ic);

   preedit_attr = XVaCreateNestedList(0,
                                      XNPreeditState, preedit_state,
                                      NULL);
   if (have_preedit_state)
     XSetICValues(ic,
                  XNPreeditAttributes, preedit_attr,
                  NULL);

   XFree(preedit_attr);

   if (imf_context_data->feedbacks)
     {
        free(imf_context_data->feedbacks);
        imf_context_data->feedbacks = NULL;
     }

   if (imf_context_data->preedit_length)
     {
        imf_context_data->preedit_length = 0;
        free(imf_context_data->preedit_chars);
        imf_context_data->preedit_chars = NULL;

        ecore_imf_context_preedit_changed_event_add(ctx);
        ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
     }

   if (result)
     {
        char *result_utf8 = strdup(result);
        if (result_utf8)
          {
             ecore_imf_context_commit_event_add(ctx, result_utf8);
             ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_COMMIT, result_utf8);
             free(result_utf8);
          }
     }

   XFree(result);
}

static void
_ecore_imf_context_xim_use_preedit_set(Ecore_IMF_Context *ctx,
                                       Eina_Bool use_preedit)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);

   DBG("ctx=%p, imf_context_data=%p, use_preedit=%hhu", ctx, imf_context_data, use_preedit);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   use_preedit = use_preedit != EINA_FALSE;

   if (imf_context_data->use_preedit != use_preedit)
     {
        imf_context_data->use_preedit = use_preedit;
        _ecore_imf_xim_ic_reinitialize(ctx);
     }
}

static void
_ecore_imf_xim_feedback_attr_add(Eina_List **attrs,
                                const char *str,
                                XIMFeedback feedback,
                                int start_pos,
                                int end_pos)
{
   Ecore_IMF_Preedit_Attr *attr;
   unsigned int start_index = _ecore_imf_xim_utf8_offset_to_index(str, start_pos);
   unsigned int end_index = _ecore_imf_xim_utf8_offset_to_index(str, end_pos);

   if (feedback & FEEDBACK_MASK)
     {
        attr = calloc(1, sizeof(Ecore_IMF_Preedit_Attr));
        attr->start_index = start_index;
        attr->end_index = end_index;
        *attrs = eina_list_append(*attrs, attr);
     }
   else
     return;

   if (feedback & XIMUnderline)
     attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB1;

   if (feedback & XIMReverse)
     attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB2;

   if (feedback & XIMHighlight)
     attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB3;
}

static void
_ecore_imf_context_xim_cursor_location_set(Ecore_IMF_Context *ctx,
                                           int x, int y, int w, int h)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   XIC ic;
   XVaNestedList preedit_attr;
   XPoint spot;

   DBG("ctx=%p, imf_context_data=%p, location=(%d, %d, %d, %d)",
       ctx, imf_context_data, x, y, w, h);

   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);
   ic = imf_context_data->ic;
   if (!ic)
     return;

   spot.x = x;
   spot.y = y + h;

   preedit_attr = XVaCreateNestedList(0,
                                      XNSpotLocation, &spot,
                                      NULL);
   XSetICValues(ic,
                XNPreeditAttributes, preedit_attr,
                NULL);

   XFree(preedit_attr);
}

static void
_ecore_imf_context_xim_input_panel_show(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   if (!ecore_x_display_get()) return;
   ecore_x_e_virtual_keyboard_state_set
        (imf_context_data->win, ECORE_X_VIRTUAL_KEYBOARD_STATE_ON);
}

static void
_ecore_imf_context_xim_input_panel_hide(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   if (!ecore_x_display_get()) return;
   ecore_x_e_virtual_keyboard_state_set
        (imf_context_data->win, ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF);
}

static unsigned int
_ecore_x_event_reverse_modifiers(unsigned int state)
{
   unsigned int modifiers = 0;

   /**< "Control" is pressed */
   if (state & ECORE_IMF_KEYBOARD_MODIFIER_CTRL)
     modifiers |= ECORE_X_MODIFIER_CTRL;

   /**< "Alt" is pressed */
   if (state & ECORE_IMF_KEYBOARD_MODIFIER_ALT)
     modifiers |= ECORE_X_MODIFIER_ALT;

   /**< "Shift" is pressed */
   if (state & ECORE_IMF_KEYBOARD_MODIFIER_SHIFT)
     modifiers |= ECORE_X_MODIFIER_SHIFT;

   /**< "Win" (between "Ctrl" and "Alt") is pressed */
   if (state & ECORE_IMF_KEYBOARD_MODIFIER_WIN)
     modifiers |= ECORE_X_MODIFIER_WIN;

   /**< "AltGr" is pressed */
   if (state & ECORE_IMF_KEYBOARD_MODIFIER_ALTGR)
     modifiers |= ECORE_X_MODIFIER_ALTGR;

   return modifiers;
}

static unsigned int
_ecore_x_event_reverse_locks(unsigned int state)
{
   unsigned int locks = 0;

   /**< "Num" lock is active */
   if (state & ECORE_IMF_KEYBOARD_LOCK_NUM)
     locks |= ECORE_X_LOCK_NUM;

   if (state & ECORE_IMF_KEYBOARD_LOCK_CAPS)
     locks |= ECORE_X_LOCK_CAPS;

   if (state & ECORE_IMF_KEYBOARD_LOCK_SCROLL)
     locks |= ECORE_X_LOCK_SCROLL;

   return locks;
}

static KeyCode
_ecore_imf_xim_keycode_get(Ecore_X_Display *dsp,
                           const char *keyname)
{
   KeyCode keycode;

   //DBG("keyname=%s keysym=%lu", keyname, XStringToKeysym(keyname));
   if (strcmp(keyname, "Keycode-0") == 0)
     keycode = 0;
   else
     keycode = XKeysymToKeycode(dsp, XStringToKeysym(keyname));

   return keycode;
}

static Eina_Bool
_ecore_imf_context_xim_filter_event(Ecore_IMF_Context *ctx,
                                    Ecore_IMF_Event_Type type,
                                    Ecore_IMF_Event *event)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   XIC ic;
   Ecore_X_Display *dsp;
   Ecore_X_Window win;

   int val;
   char compose_buffer[256];
   KeySym sym;
   char *compose = NULL;
   char *tmp = NULL;
   Eina_Bool result = EINA_FALSE;

   DBG("ctx=%p, imf_context_data=%p, type=%d, event=%p",
       ctx, imf_context_data, type, event);
   EINA_SAFETY_ON_NULL_RETURN_VAL(imf_context_data, EINA_FALSE);
   ic = imf_context_data->ic;
   if (!ic)
     ic = _ecore_imf_xim_ic_get(ctx);

   if (type == ECORE_IMF_EVENT_KEY_DOWN)
     {
        XKeyPressedEvent xev;
        Ecore_IMF_Event_Key_Down *ev = (Ecore_IMF_Event_Key_Down *)event;
        DBG("ECORE_IMF_EVENT_KEY_DOWN");

        dsp = ecore_x_display_get();
        win = imf_context_data->win;

        xev.type = KeyPress;
        xev.serial = 0; /* hope it doesn't matter */
        xev.send_event = 0;
        xev.display = dsp;
        xev.window = win;
        xev.root = dsp ? ecore_x_window_root_get(win) : 0;
        xev.subwindow = win;
        xev.time = ev->timestamp;
        xev.x = xev.x_root = 0;
        xev.y = xev.y_root = 0;
        xev.state = 0;
        if (dsp)
          {
             xev.state |= _ecore_x_event_reverse_modifiers(ev->modifiers);
             xev.state |= _ecore_x_event_reverse_locks(ev->locks);
             xev.keycode = _ecore_imf_xim_keycode_get(dsp, ev->keyname);
          }
        xev.same_screen = True;

        if (ic)
          {
             Status mbstatus;
#ifdef X_HAVE_UTF8_STRING
             val = Xutf8LookupString(ic,
                                     &xev,
                                     compose_buffer,
                                     sizeof(compose_buffer) - 1,
                                     &sym,
                                     &mbstatus);
#else /* ifdef X_HAVE_UTF8_STRING */
             val = XmbLookupString(ic,
                                   &xev,
                                   compose_buffer,
                                   sizeof(compose_buffer) - 1,
                                   &sym,
                                   &mbstatus);
#endif /* ifdef X_HAVE_UTF8_STRING */
             if (mbstatus == XBufferOverflow)
               {
                  tmp = malloc(sizeof (char) * (val + 1));
                  if (!tmp)
                    return EINA_FALSE;

                  compose = tmp;

#ifdef X_HAVE_UTF8_STRING
                  val = Xutf8LookupString(ic,
                                          &xev,
                                          tmp,
                                          val,
                                          &sym,
                                          &mbstatus);
#else /* ifdef X_HAVE_UTF8_STRING */
                  val = XmbLookupString(ic,
                                        &xev,
                                        tmp,
                                        val,
                                        &sym,
                                        &mbstatus);
#endif /* ifdef X_HAVE_UTF8_STRING */
                  if (val > 0)
                    {
                       tmp[val] = '\0';
#ifndef X_HAVE_UTF8_STRING
                       compose = eina_str_convert(nl_langinfo(CODESET),
                                                  "UTF-8", tmp);
                       free(tmp);
                       tmp = compose;
#endif /* ifndef X_HAVE_UTF8_STRING */
                    }
                  else
                    compose = NULL;
               }
             else if (val > 0)
               {
                  compose_buffer[val] = '\0';
#ifdef X_HAVE_UTF8_STRING
                  compose = strdup(compose_buffer);
#else /* ifdef X_HAVE_UTF8_STRING */
                  compose = eina_str_convert(nl_langinfo(CODESET), "UTF-8",
                                             compose_buffer);
#endif /* ifdef X_HAVE_UTF8_STRING */
               }
          }
        else
          {
             compose = strdup(ev->compose);
          }

        if (compose)
          {
             Eina_Unicode *unicode;
             int len;
             unicode = eina_unicode_utf8_to_unicode(compose, &len);
             if (!unicode) abort();
             if (unicode[0] >= 0x20 && unicode[0] != 0x7f)
               {
                  ecore_imf_context_commit_event_add(ctx, compose);
                  ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_COMMIT, compose);
                  result = EINA_TRUE;
               }
             free(compose);
             free(unicode);
          }
     }

   return result;
}

static const Ecore_IMF_Context_Info xim_info = {
   .id = "xim",
   .description = "X input method",
   .default_locales = "ko:ja:th:zh",
   .canvas_type = "evas",
   .canvas_required = 1,
};

static Ecore_IMF_Context_Class xim_class = {
   .add = _ecore_imf_context_xim_add,
   .del = _ecore_imf_context_xim_del,
   .client_window_set = _ecore_imf_context_xim_client_window_set,
   .client_canvas_set = NULL,
   .show = _ecore_imf_context_xim_input_panel_show,
   .hide = _ecore_imf_context_xim_input_panel_hide,
   .preedit_string_get = _ecore_imf_context_xim_preedit_string_get,
   .focus_in = _ecore_imf_context_xim_focus_in,
   .focus_out = _ecore_imf_context_xim_focus_out,
   .reset = _ecore_imf_context_xim_reset,
   .cursor_position_set = NULL,
   .use_preedit_set = _ecore_imf_context_xim_use_preedit_set,
   .input_mode_set = NULL,
   .filter_event = _ecore_imf_context_xim_filter_event,
   .preedit_string_with_attributes_get = _ecore_imf_context_xim_preedit_string_with_attributes_get,
   .prediction_allow_set = NULL,
   .autocapital_type_set = NULL,
   .control_panel_show = NULL,
   .control_panel_hide = NULL,
   .input_panel_layout_set = NULL,
   .input_panel_layout_get = NULL,
   .input_panel_language_set = NULL,
   .input_panel_language_get = NULL,
   .cursor_location_set = _ecore_imf_context_xim_cursor_location_set,
   .input_panel_imdata_set = NULL,
   .input_panel_imdata_get = NULL,
   .input_panel_return_key_type_set = NULL,
   .input_panel_return_key_disabled_set = NULL,
   .input_panel_caps_lock_mode_set = NULL
};

static Ecore_IMF_Context *
xim_imf_module_create(void)
{
   Ecore_IMF_Context *ctx;

   if (!ecore_x_init(NULL))
     return NULL;
   ctx = ecore_imf_context_new(&xim_class);
   DBG("ctx=%p", ctx);
   return ctx;
}

static Ecore_IMF_Context *
xim_imf_module_exit(void)
{
   ecore_x_shutdown();
   DBG(" ");
   return NULL;
}

static Eina_Bool
_ecore_imf_xim_init(void)
{
   const char *s;

   if (!getenv("DISPLAY")) return EINA_FALSE;
   if ((s = getenv("ELM_DISPLAY")))
     {
        if (strcmp(s, "x11")) return EINA_FALSE;
     }
   eina_init();

   _ecore_imf_xim_log_dom = eina_log_domain_register("ecore_imf_xim", NULL);
   if (_ecore_imf_xim_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ecore_imf_xim");
     }

   DBG(" ");

   ecore_imf_module_register(&xim_info,
                             xim_imf_module_create,
                             xim_imf_module_exit);

   return EINA_TRUE;
}

static void
_ecore_imf_xim_shutdown(void)
{
   while (open_ims)
     {
        XIM_Im_Info *info = open_ims->data;
        Ecore_X_Display *display = ecore_x_display_get();

        if (display)
          _ecore_imf_xim_info_im_shutdown(display, EINA_FALSE, info);
     }

   if (_ecore_imf_xim_log_dom >= 0)
     {
        eina_log_domain_unregister(_ecore_imf_xim_log_dom);
        _ecore_imf_xim_log_dom = -1;
     }

   eina_shutdown();
}

EINA_MODULE_INIT(_ecore_imf_xim_init);
EINA_MODULE_SHUTDOWN(_ecore_imf_xim_shutdown);

/*
 * internal functions
 */
static Ecore_IMF_Context_Data *
_ecore_imf_xim_context_data_new(void)
{
   Ecore_IMF_Context_Data *imf_context_data = NULL;
   char *locale;

   locale = setlocale(LC_CTYPE, "");
   if (!locale) return NULL;

   if (!XSupportsLocale()) return NULL;

   imf_context_data = calloc(1, sizeof(Ecore_IMF_Context_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(imf_context_data, NULL);

   imf_context_data->locale = strdup(locale);
   if (!imf_context_data->locale) goto error;

   return imf_context_data;
error:
   _ecore_imf_xim_context_data_destroy(imf_context_data);
   return NULL;
}

static void
_ecore_imf_xim_context_data_destroy(Ecore_IMF_Context_Data *imf_context_data)
{
   if (!imf_context_data)
     return;

   if (imf_context_data->ic)
     XDestroyIC(imf_context_data->ic);

   free(imf_context_data->preedit_chars);

   if (imf_context_data->feedbacks)
     {
        free(imf_context_data->feedbacks);
        imf_context_data->feedbacks = NULL;
     }

   free(imf_context_data->locale);
   free(imf_context_data);
}

static int
_ecore_imf_xim_preedit_start_call(XIC xic EINA_UNUSED,
                                 XPointer client_data,
                                 XPointer call_data EINA_UNUSED)
{
   Ecore_IMF_Context *ctx = (Ecore_IMF_Context *)client_data;
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);

   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(imf_context_data, -1);

   if (imf_context_data->finalizing == EINA_FALSE)
     {
        ecore_imf_context_preedit_start_event_add(ctx);
        ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_PREEDIT_START, NULL);
     }
   return -1;
}

static void
_ecore_imf_xim_preedit_done_call(XIC xic EINA_UNUSED,
                                XPointer client_data,
                                XPointer call_data EINA_UNUSED)
{
   Ecore_IMF_Context *ctx = (Ecore_IMF_Context *)client_data;
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);

   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   if (imf_context_data->preedit_length)
     {
        imf_context_data->preedit_length = 0;
        free(imf_context_data->preedit_chars);
        imf_context_data->preedit_chars = NULL;
        ecore_imf_context_preedit_changed_event_add(ctx);
        ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
     }

   if (imf_context_data->finalizing == EINA_FALSE)
     {
        ecore_imf_context_preedit_end_event_add(ctx);
        ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_PREEDIT_END, NULL);
     }
}

/* FIXME */
static int
_ecore_imf_xim_text_to_utf8(Ecore_IMF_Context *ctx EINA_UNUSED,
                           XIMText *xim_text,
                           char **text)
{
   int text_length = 0;
   char *result = NULL;

   if (xim_text && xim_text->string.multi_byte)
     {
        if (xim_text->encoding_is_wchar)
          {
             WRN("Wide character return from Xlib not currently supported");
             *text = NULL;
             return 0;
          }

        /* XXX Convert to UTF-8 */
        result = strdup(xim_text->string.multi_byte);
        if (result)
          {
             text_length = eina_unicode_utf8_get_len(result);
             if (text_length != xim_text->length)
               {
                  WRN("Size mismatch when converting text from input method: supplied length = %d, result length = %d",
                      xim_text->length, text_length);
               }
          }
        else
          {
             WRN("Error converting text from IM to UCS-4");
             *text = NULL;
             return 0;
          }

        *text = result;
        return text_length;
     }
   else
     {
        *text = NULL;
        return 0;
     }
}

static void
_ecore_imf_xim_preedit_draw_call(XIC xic EINA_UNUSED,
                                XPointer client_data,
                                XIMPreeditDrawCallbackStruct *call_data)
{
   Eina_Bool ret = EINA_FALSE;
   Ecore_IMF_Context *ctx = (Ecore_IMF_Context *)client_data;
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   XIMText *t = call_data->text;
   char *tmp;
   Eina_Unicode *new_text = NULL;
   Eina_UStrbuf *preedit_bufs = NULL;
   int new_text_length;
   int i = 0;

   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   imf_context_data->preedit_cursor = call_data->caret;

   preedit_bufs = eina_ustrbuf_new();
   if (imf_context_data->preedit_chars)
     {
        ret = eina_ustrbuf_append(preedit_bufs, imf_context_data->preedit_chars);
        if (ret == EINA_FALSE) goto done;
     }

   new_text_length = _ecore_imf_xim_text_to_utf8(ctx, t, &tmp);
   if (tmp)
     {
        int tmp_len;
        new_text = eina_unicode_utf8_to_unicode(tmp, &tmp_len);
        free(tmp);
     }

   if (t == NULL)
     {
        /* delete string */
        ret = eina_ustrbuf_remove(preedit_bufs,
                                  call_data->chg_first, call_data->chg_length);
     }
   else if (call_data->chg_length == 0)
     {
        /* insert string */
        ret = eina_ustrbuf_insert(preedit_bufs, new_text, call_data->chg_first);
     }
   else if (call_data->chg_length > 0)
     {
        /* replace string */
        ret = eina_ustrbuf_remove(preedit_bufs,
                                  call_data->chg_first, call_data->chg_length);
        if (ret == EINA_FALSE) goto done;

        ret = eina_ustrbuf_insert_n(preedit_bufs, new_text,
                                    new_text_length, call_data->chg_first);
        if (ret == EINA_FALSE) goto done;
     }
   else
     {
        ret = EINA_FALSE;
     }

done:
   if (ret == EINA_TRUE)
     {
        free(imf_context_data->preedit_chars);
        imf_context_data->preedit_chars =
          eina_ustrbuf_string_steal(preedit_bufs);
        imf_context_data->preedit_length =
          eina_unicode_strlen(imf_context_data->preedit_chars);

        if (imf_context_data->feedbacks)
          {
             free(imf_context_data->feedbacks);
             imf_context_data->feedbacks = NULL;
          }

        if (imf_context_data->preedit_length > 0)
          {
             imf_context_data->feedbacks = calloc(imf_context_data->preedit_length, sizeof(XIMFeedback));

             for (i = 0; i < imf_context_data->preedit_length; i++)
               {
                  if (t)
                    imf_context_data->feedbacks[i] = t->feedback[i];
               }
          }

        ecore_imf_context_preedit_changed_event_add(ctx);
        ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
     }

   free(new_text);
   eina_ustrbuf_free(preedit_bufs);
}

static void
_ecore_imf_xim_preedit_caret_call(XIC xic EINA_UNUSED,
                                 XPointer client_data,
                                 XIMPreeditCaretCallbackStruct *call_data)
{
   Ecore_IMF_Context *ctx = (Ecore_IMF_Context *)client_data;
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);

   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   if (call_data->direction == XIMAbsolutePosition)
     {
        imf_context_data->preedit_cursor = call_data->position;
        if (imf_context_data->finalizing == EINA_FALSE)
          {
             ecore_imf_context_preedit_changed_event_add(ctx);
             ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
          }
     }
}

static XVaNestedList
_ecore_imf_xim_preedit_callback_set(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);
   if (!imf_context_data)
     return XVaCreateNestedList(0, NULL);

   imf_context_data->preedit_start_cb.client_data = (XPointer)ctx;
   imf_context_data->preedit_start_cb.callback = (XIMProc)_ecore_imf_xim_preedit_start_call;

   imf_context_data->preedit_done_cb.client_data = (XPointer)ctx;
   imf_context_data->preedit_done_cb.callback = (XIMProc)_ecore_imf_xim_preedit_done_call;

   imf_context_data->preedit_draw_cb.client_data = (XPointer)ctx;
   imf_context_data->preedit_draw_cb.callback = (XIMProc)_ecore_imf_xim_preedit_draw_call;

   imf_context_data->preedit_caret_cb.client_data = (XPointer)ctx;
   imf_context_data->preedit_caret_cb.callback = (XIMProc)_ecore_imf_xim_preedit_caret_call;

   return XVaCreateNestedList(0,
                              XNPreeditStartCallback,
                              &imf_context_data->preedit_start_cb,
                              XNPreeditDoneCallback,
                              &imf_context_data->preedit_done_cb,
                              XNPreeditDrawCallback,
                              &imf_context_data->preedit_draw_cb,
                              XNPreeditCaretCallback,
                              &imf_context_data->preedit_caret_cb,
                              NULL);
}

static XIC
_ecore_imf_xim_ic_get(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data;
   XIC ic;
   imf_context_data = ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN_VAL(imf_context_data, NULL);

   if (!ecore_x_display_get()) return NULL;
   ic = imf_context_data->ic;
   if (!ic)
     {
        XIM_Im_Info *im_info = imf_context_data->im_info;
        XVaNestedList preedit_attr = NULL;
        XIMStyle im_style = 0;
        XPoint spot = { 0, 0 };
        char *name = NULL;

        if (!im_info)
          {
             WRN("Could not open XIM.");
             return NULL;
          }

        // supported styles
        // "OverTheSpot" = XIMPreeditPosition | XIMStatusNothing
        // "OffTheSpot" = XIMPreeditArea | XIMStatusArea
        // "Root" = XIMPreeditNothing | XIMStatusNothing

        if (imf_context_data->use_preedit == EINA_TRUE)
          {
             if (im_info->supports_cursor)
               {
                  // kinput2 DOES do this...
                  XFontSet fs;
                  char **missing_charset_list;
                  int missing_charset_count;
                  char *def_string;

                  im_style |= XIMPreeditPosition;
                  im_style |= XIMStatusNothing;
                  fs = XCreateFontSet(ecore_x_display_get(),
                                      "fixed",
                                      &missing_charset_list,
                                      &missing_charset_count,
                                      &def_string);
                  preedit_attr = XVaCreateNestedList(0,
                                                     XNSpotLocation, &spot,
                                                     XNFontSet, fs,
                                                     NULL);
               }
             else
               {
                  im_style |= XIMPreeditCallbacks;
                  im_style |= XIMStatusNothing;
                  preedit_attr = _ecore_imf_xim_preedit_callback_set(ctx);
               }
             name = XNPreeditAttributes;
          }
        else
          {
             im_style |= XIMPreeditNothing;
             im_style |= XIMStatusNothing;
          }

        if (!im_info->xim_styles)
          {
             WRN("No XIM styles supported! Wanted %#llx",
                 (unsigned long long)im_style);
             im_style = 0;
          }
        else
          {
             XIMStyle fallback = 0;
             int i;

             for (i = 0; i < im_info->xim_styles->count_styles; i++)
               {
                  XIMStyle cur = im_info->xim_styles->supported_styles[i];
                  if (cur == im_style)
                    break;
                  else if (cur == (XIMPreeditNothing | XIMStatusNothing))
                    /* TODO: fallback is just that or the anyone? */
                    fallback = cur;
               }

             if (i == im_info->xim_styles->count_styles)
               {
                  if (fallback)
                    {
                       WRN("Wanted XIM style %#llx not found, using fallback %#llx instead.",
                           (unsigned long long)im_style,
                           (unsigned long long)fallback);
                       im_style = fallback;
                    }
                  else
                    {
                       WRN("Wanted XIM style %#llx not found, no fallback supported.",
                           (unsigned long long)im_style);
                       im_style = 0;
                    }
               }
          }

        if ((im_info->im) && (im_style))
          {
             ic = XCreateIC(im_info->im,
                            XNInputStyle, im_style,
                            XNClientWindow, imf_context_data->win,
                            name, preedit_attr, NULL);
          }
        XFree(preedit_attr);
        if (ic)
          {
             unsigned long mask = 0xaaaaaaaa;
             XGetICValues(ic,
                          XNFilterEvents, &mask,
                          NULL);
             imf_context_data->mask = mask;
             ecore_x_event_mask_set(imf_context_data->win, mask);
          }

        imf_context_data->ic = ic;
        if (ic && imf_context_data->has_focus == EINA_TRUE)
          XSetICFocus(ic);
     }

   return ic;
}

static void
_ecore_imf_xim_ic_reinitialize(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   XIC ic = imf_context_data->ic;
   if (ic)
     {
        XDestroyIC(ic);
        imf_context_data->ic = NULL;
        if (imf_context_data->preedit_length)
          {
             imf_context_data->preedit_length = 0;
             free(imf_context_data->preedit_chars);
             imf_context_data->preedit_chars = NULL;
             ecore_imf_context_preedit_changed_event_add(ctx);
             ecore_imf_context_event_callback_call(ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
          }
     }
}

static void
_ecore_imf_xim_ic_client_window_set(Ecore_IMF_Context *ctx,
                                    Ecore_X_Window window)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   Ecore_X_Window old_win;

   DBG("ctx=%p, imf_context_data=%p", ctx, imf_context_data);
   EINA_SAFETY_ON_NULL_RETURN(imf_context_data);

   /* reinitialize IC */
   _ecore_imf_xim_ic_reinitialize(ctx);

   old_win = imf_context_data->win;
   DBG("old_win=%#x, window=%#x", old_win, window);
   if (old_win != 0 && old_win != window)   /* XXX how do check window... */
     {
        XIM_Im_Info *info = imf_context_data->im_info;
        if (info)
          {
             info->ics = eina_list_remove(info->ics, imf_context_data);
             info->user = NULL;
             info = NULL;
          }
     }

   imf_context_data->win = window;

   if (window) /* XXX */
     {
        XIM_Im_Info *info = NULL;
        info = _ecore_imf_xim_im_get(window, imf_context_data->locale);
        imf_context_data->im_info = info;
        imf_context_data->im_info->ics =
          eina_list_prepend(imf_context_data->im_info->ics,
                            imf_context_data);
        if (imf_context_data->im_info)
          imf_context_data->im_info->user = imf_context_data;
     }
}

static XIM_Im_Info *
_ecore_imf_xim_im_get(Ecore_X_Window window,
                     char *locale)
{
   Eina_List *l;
   XIM_Im_Info *im_info = NULL;
   XIM_Im_Info *info = NULL;

   DBG(" ");
   EINA_LIST_FOREACH (open_ims, l, im_info)
     {
        if (strcmp(im_info->locale, locale) == 0)
          {
             if (im_info->im)
               {
                  return im_info;
               }
             else
               {
                  info = im_info;
                  break;
               }
          }
     }

   if (!info)
     {
        info = calloc(1, sizeof(XIM_Im_Info));
        if (!info) return NULL;
        open_ims = eina_list_prepend(open_ims, info);
        info->win = window;
        info->locale = strdup(locale);
        info->reconnecting = EINA_FALSE;
     }

   _ecore_imf_xim_info_im_init(info);
   return info;
}

/* initialize info->im */
static void
_ecore_imf_xim_info_im_init(XIM_Im_Info *info)
{
   Ecore_X_Display *dsp;

   assert(info->im == NULL);
   if (info->reconnecting == EINA_TRUE)
     return;

   if (XSupportsLocale())
     {
        if (!XSetLocaleModifiers(""))
          WRN("Unable to set locale modifiers with XSetLocaleModifiers()");
        dsp = ecore_x_display_get();
        if (dsp)
          {
             info->im = XOpenIM(dsp, NULL, NULL, NULL);
             if (!info->im)
               {
                  XRegisterIMInstantiateCallback(dsp,
                                                 NULL, NULL, NULL,
                                                 _ecore_imf_xim_instantiate_cb,
                                                 (XPointer)info);
                  info->reconnecting = EINA_TRUE;
                  return;
               }
             _ecore_imf_xim_im_setup(info);
          }
     }
}

static void
_ecore_imf_xim_info_im_shutdown(Ecore_X_Display *display EINA_UNUSED,
                               int is_error EINA_UNUSED,
                               XIM_Im_Info *info)
{
   Eina_List *ics, *tmp_list;
   Ecore_IMF_Context *ctx;

   open_ims = eina_list_remove(open_ims, info);

   ics = info->ics;
   info->ics = NULL;

   EINA_LIST_FOREACH (ics, tmp_list, ctx)
     _ecore_imf_xim_ic_client_window_set(ctx, 0);

   EINA_LIST_FREE (ics, ctx)
     {
        Ecore_IMF_Context_Data *imf_context_data;
        imf_context_data = ecore_imf_context_data_get(ctx);
        _ecore_imf_xim_context_data_destroy(imf_context_data);
     }

   free(info->locale);

   if (info->xim_styles)
     XFree(info->xim_styles);

   if (info->im)
     XCloseIM(info->im);

   free(info);
}

static void
_ecore_imf_xim_instantiate_cb(Display *display,
                             XPointer client_data,
                             XPointer call_data EINA_UNUSED)
{
   XIM_Im_Info *info = (XIM_Im_Info *)client_data;
   XIM im = XOpenIM(display, NULL, NULL, NULL);
   EINA_SAFETY_ON_NULL_RETURN(im);

   info->im = im;
   _ecore_imf_xim_im_setup(info);

   XUnregisterIMInstantiateCallback(display, NULL, NULL, NULL,
                                    _ecore_imf_xim_instantiate_cb,
                                    (XPointer)info);
   info->reconnecting = EINA_FALSE;
}

static void
_ecore_imf_xim_im_setup(XIM_Im_Info *info)
{
   XIMValuesList *ic_values = NULL;
   XIMCallback im_destroy_callback;

   if (!info->im)
     return;

   im_destroy_callback.client_data = (XPointer)info;
   im_destroy_callback.callback = (XIMProc)_ecore_imf_xim_destroy_cb;
   XSetIMValues(info->im,
                XNDestroyCallback, &im_destroy_callback,
                NULL);

   XGetIMValues(info->im,
                XNQueryInputStyle, &info->xim_styles,
                XNQueryICValuesList, &ic_values,
                NULL);

   if (ic_values)
     {
        int i;

        for (i = 0; i < ic_values->count_values; i++)
          {
             if (!strcmp(ic_values->supported_values[i],
                         XNStringConversionCallback))
               info->supports_string_conversion = EINA_TRUE;
             if (!strcmp(ic_values->supported_values[i],
                         XNCursor))
               info->supports_cursor = EINA_TRUE;
          }

        XFree(ic_values);
     }
}

static void
_ecore_imf_xim_destroy_cb(XIM xim EINA_UNUSED,
                         XPointer client_data,
                         XPointer call_data EINA_UNUSED)
{
   XIM_Im_Info *info = (XIM_Im_Info *)client_data;

   if (info->user) info->user->ic = NULL;
   info->im = NULL;
//   _ecore_imf_xim_ic_reinitialize(ctx);
   _ecore_imf_xim_info_im_init(info);

   return;
}
