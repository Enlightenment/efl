#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Input.h>
#include <Ecore_IMF.h>
#include <Evas.h>
#include <Ecore_X.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <langinfo.h>
#include <assert.h>
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define CLAMP(x, low, high) (x > high) ? high : (x < low) ? low : x
#define _(x)                x

#ifdef ENABLE_XIM
static Eina_List *open_ims = NULL;
#endif

typedef struct _XIM_Im_Info XIM_Im_Info;
struct _XIM_Im_Info
{
   Ecore_X_Window win;
   char          *locale;
   XIM            im;
   Eina_List     *ics;
   Eina_Bool      reconnecting;
   XIMStyles     *xim_styles;
   Eina_Bool      supports_string_conversion : 1;
   Eina_Bool      supports_cursor : 1;
};

typedef struct _Ecore_IMF_Context_Data Ecore_IMF_Context_Data;
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

   XIMCallback    preedit_start_cb;
   XIMCallback    preedit_done_cb;
   XIMCallback    preedit_draw_cb;
   XIMCallback    preedit_caret_cb;
};

/* prototype */
Ecore_IMF_Context_Data *imf_context_data_new();
void                    imf_context_data_destroy(Ecore_IMF_Context_Data *imf_context_data);

#ifdef ENABLE_XIM
static void reinitialize_ic(Ecore_IMF_Context *ctx);
static void reinitialize_all_ics(XIM_Im_Info *info);
static void set_ic_client_window(Ecore_IMF_Context *ctx,
                                 Ecore_X_Window     window);
static int  preedit_start_callback(XIC      xic,
                                   XPointer client_data,
                                   XPointer call_data);
static void preedit_done_callback(XIC      xic,
                                  XPointer client_data,
                                  XPointer call_data);
static int xim_text_to_utf8(Ecore_IMF_Context *ctx,
                            XIMText           *xim_text,
                            char             **text);
static void preedit_draw_callback(XIC                           xic,
                                  XPointer                      client_data,
                                  XIMPreeditDrawCallbackStruct *call_data);
static void preedit_caret_callback(XIC                            xic,
                                   XPointer                       client_data,
                                   XIMPreeditCaretCallbackStruct *call_data);
static XVaNestedList preedit_callback_set(Ecore_IMF_Context *ctx);
static XIC           get_ic(Ecore_IMF_Context *ctx);
static XIM_Im_Info  *get_im(Ecore_X_Window window,
                            char          *locale);
static void          xim_info_try_im(XIM_Im_Info *info);
static void          xim_info_display_closed(Ecore_X_Display *display,
                                             int              is_error,
                                             XIM_Im_Info     *info);
static void xim_instantiate_callback(Display *display,
                                     XPointer client_data,
                                     XPointer call_data);
static void setup_im(XIM_Im_Info *info);
static void xim_destroy_callback(XIM      xim,
                                 XPointer client_data,
                                 XPointer call_data);
#endif

static void
_ecore_imf_context_xim_add(Ecore_IMF_Context *ctx)
{
   EINA_LOG_DBG("in");
#ifdef ENABLE_XIM
   Ecore_IMF_Context_Data *imf_context_data = NULL;

   imf_context_data = imf_context_data_new();
   if(!imf_context_data) return;

   imf_context_data->use_preedit = EINA_TRUE;
   imf_context_data->finalizing = EINA_FALSE;
   imf_context_data->has_focus = EINA_FALSE;
   imf_context_data->in_toplevel = EINA_FALSE;

   ecore_imf_context_data_set(ctx, imf_context_data);
#endif
}

static void
_ecore_imf_context_xim_del(Ecore_IMF_Context *ctx)
{
   EINA_LOG_DBG("in");
#ifdef ENABLE_XIM
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);

   imf_context_data->finalizing = EINA_TRUE;
   if(imf_context_data->im_info && !imf_context_data->im_info->ics->next)
     {
        if(imf_context_data->im_info->reconnecting == EINA_TRUE)
          {
             Ecore_X_Display *dsp;
             dsp = ecore_x_display_get();
             XUnregisterIMInstantiateCallback (dsp,
                                               NULL, NULL, NULL,
                                               xim_instantiate_callback,
                                               (XPointer)imf_context_data->im_info);
          }
        else if(imf_context_data->im_info->im)
          {
             XIMCallback im_destroy_callback;
             im_destroy_callback.client_data = NULL;
             im_destroy_callback.callback = NULL;
             XSetIMValues (imf_context_data->im_info->im,
                           XNDestroyCallback, &im_destroy_callback,
                           NULL);
          }
     }

   set_ic_client_window(ctx, 0);

   imf_context_data_destroy(imf_context_data);
#endif
}

static void
_ecore_imf_context_xim_client_window_set(Ecore_IMF_Context *ctx,
                                         void              *window)
{
   EINA_LOG_DBG("in");
#ifdef ENABLE_XIM
   set_ic_client_window(ctx, (Ecore_X_Window)((Ecore_Window)window));
#endif
}

static void
_ecore_imf_context_xim_preedit_string_get(Ecore_IMF_Context *ctx,
                                          char             **str,
                                          int               *cursor_pos)
{
   EINA_LOG_DBG("in");
#ifdef ENABLE_XIM
   Ecore_IMF_Context_Data *imf_context_data;
   char *utf8;
   int len;
   imf_context_data = ecore_imf_context_data_get(ctx);
   if (imf_context_data->preedit_chars)
     {
        utf8 = eina_unicode_unicode_to_utf8(imf_context_data->preedit_chars,
                                            &len);
        if(str)
           *str = utf8;
        else
           free(utf8);
     }
   else
     {
        if(str)
           *str = NULL;
        if(cursor_pos)
           *cursor_pos = 0;
     }

   if(cursor_pos)
     *cursor_pos = imf_context_data->preedit_cursor;
#else
   if(str)
     *str = NULL;
   if(cursor_pos)
     *cursor_pos = 0;
#endif
}

static void
_ecore_imf_context_xim_focus_in(Ecore_IMF_Context *ctx)
{
   EINA_LOG_DBG("in");
#ifdef ENABLE_XIM
   XIC ic;
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);
   ic = imf_context_data->ic;
   imf_context_data->has_focus = EINA_TRUE;
   if(ic)
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
#endif
}

static void
_ecore_imf_context_xim_focus_out(Ecore_IMF_Context *ctx)
{
   EINA_LOG_DBG("%s in", __FUNCTION__);
#ifdef ENABLE_XIM
   XIC ic;
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);
   if(imf_context_data->has_focus == EINA_TRUE)
     {
        imf_context_data->has_focus = EINA_FALSE;
        ic = imf_context_data->ic;
        if(ic)
          XUnsetICFocus(ic);
     }
#endif
}

static void
_ecore_imf_context_xim_reset(Ecore_IMF_Context *ctx)
{
   EINA_LOG_DBG("%s in", __FUNCTION__);
#ifdef ENABLE_XIM
   XIC ic;
   Ecore_IMF_Context_Data *imf_context_data;
   char *result;

   /* restore conversion state after resetting ic later */
   XIMPreeditState preedit_state = XIMPreeditUnKnown;
   XVaNestedList preedit_attr;
   Eina_Bool have_preedit_state = EINA_FALSE;

   imf_context_data = ecore_imf_context_data_get(ctx);
   ic = imf_context_data->ic;
   if(!ic)
     return;

   if(imf_context_data->preedit_length == 0)
     return;

   preedit_attr = XVaCreateNestedList(0,
                                      XNPreeditState, &preedit_state,
                                      NULL);
   if(!XGetICValues(ic,
                    XNPreeditAttributes, preedit_attr,
                    NULL))
     have_preedit_state = EINA_TRUE;

   XFree(preedit_attr);

   result = XmbResetIC(ic);

   preedit_attr = XVaCreateNestedList(0,
                                      XNPreeditState, preedit_state,
                                      NULL);
   if(have_preedit_state)
     XSetICValues(ic,
                  XNPreeditAttributes, preedit_attr,
                  NULL);

   XFree(preedit_attr);

   if(imf_context_data->preedit_length)
     {
        imf_context_data->preedit_length = 0;
        free(imf_context_data->preedit_chars);
        imf_context_data->preedit_chars = NULL;
        ecore_imf_context_preedit_changed_event_add(ctx);
     }

   if(result)
     {
         char *result_utf8 = strdup(result);
         if(result_utf8)
           {
              ecore_imf_context_commit_event_add(ctx, result_utf8);
              free(result_utf8);
           }
     }

   XFree (result);
#endif
}

static void
_ecore_imf_context_xim_use_preedit_set(Ecore_IMF_Context *ctx,
                                       Eina_Bool          use_preedit)
{
   EINA_LOG_DBG("in");
#ifdef ENABLE_XIM
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);

   use_preedit = use_preedit != EINA_FALSE;

   if(imf_context_data->use_preedit != use_preedit)
     {
        imf_context_data->use_preedit = use_preedit;
        reinitialize_ic(ctx);
     }
#endif
}

static void
_ecore_imf_context_xim_cursor_location_set (Ecore_IMF_Context   *ctx,
                                            int x, int y, int w, int h)
{
   EINA_LOG_DBG("%s in", __FUNCTION__);

#ifdef ENABLE_XIM
   Ecore_IMF_Context_Data *imf_context_data;
   XIC ic;
   XVaNestedList preedit_attr;
   XPoint        spot;

   imf_context_data = ecore_imf_context_data_get(ctx);
   ic = imf_context_data->ic;
   if (!ic)
     return;

   spot.x = x;
   spot.y = y + h;
   
   preedit_attr = XVaCreateNestedList (0,
                                       XNSpotLocation, &spot,
                                       NULL);
   XSetICValues (ic,
                 XNPreeditAttributes, preedit_attr,
                 NULL);

   XFree(preedit_attr);
#endif
   (void)(w); // yes w is unused, but only a bi-product of the algorithm
}

#ifdef ENABLE_XIM
static unsigned int
_ecore_x_event_reverse_modifiers(unsigned int state)
{
   unsigned int modifiers = 0;

   /**< "Control" is pressed */
   if(state & ECORE_IMF_KEYBOARD_MODIFIER_CTRL)
     modifiers |= ControlMask;

   /**< "Alt" is pressed */
   if(state & ECORE_IMF_KEYBOARD_MODIFIER_ALT)
     modifiers |= Mod1Mask;

   /**< "Shift" is pressed */
   if(state & ECORE_IMF_KEYBOARD_MODIFIER_SHIFT)
     modifiers |= ShiftMask;

   /**< "Win" (between "Ctrl" and "A */
   if(state & ECORE_IMF_KEYBOARD_MODIFIER_WIN)
     modifiers |= Mod5Mask;

   return modifiers;
}

static unsigned int
_ecore_x_event_reverse_locks(unsigned int state)
{
   unsigned int locks = 0;

   /**< "Num" lock is active */
   if(state & ECORE_IMF_KEYBOARD_LOCK_NUM)
     locks |= Mod3Mask;

   if(state & ECORE_IMF_KEYBOARD_LOCK_CAPS)
     locks |= LockMask;

#if 0                           /* FIXME: add mask. */
   if(state & ECORE_IMF_KEYBOARD_LOCK_SCROLL)
     ;
#endif

   return locks;
}

static KeyCode
_keycode_get(Ecore_X_Display *dsp,
             const char      *keyname)
{
   KeyCode keycode;

   // EINA_LOG_DBG("keyname:%s keysym:%lu", keyname, XStringToKeysym(keyname));
   if(strcmp(keyname, "Keycode-0") == 0)
     {
        keycode = 0;
     }
   else {
        keycode = XKeysymToKeycode(dsp, XStringToKeysym(keyname));
     }

   return keycode;
}

#endif

static Eina_Bool
_ecore_imf_context_xim_filter_event(Ecore_IMF_Context   *ctx,
                                    Ecore_IMF_Event_Type type,
                                    Ecore_IMF_Event     *event)
{
   EINA_LOG_DBG("%s in", __FUNCTION__);
#ifdef ENABLE_XIM
   Ecore_IMF_Context_Data *imf_context_data;
   XIC ic;

   Ecore_X_Display *dsp;
   Ecore_X_Window win;

   int val;
   char compose_buffer[256];
   KeySym sym;
   char *compose = NULL;
   char *tmp = NULL;
   Eina_Bool result = EINA_FALSE;

   imf_context_data = ecore_imf_context_data_get(ctx);
   ic = imf_context_data->ic;
   if(!ic)
     {
        ic = get_ic(ctx);
     }

   if(type == ECORE_IMF_EVENT_KEY_DOWN)
     {
        XKeyPressedEvent xev;
        Ecore_IMF_Event_Key_Down *ev = (Ecore_IMF_Event_Key_Down *)event;
        EINA_LOG_DBG("ECORE_IMF_EVENT_KEY_DOWN");

        dsp = ecore_x_display_get();
        win = imf_context_data->win;

        xev.type = KeyPress;
        xev.serial = 0; /* hope it doesn't matter */
        xev.send_event = 0;
        xev.display = dsp;
        xev.window = win;
        xev.root = ecore_x_window_root_get(win);
        xev.subwindow = win;
        xev.time = ev->timestamp;
        xev.x = xev.x_root = 0;
        xev.y = xev.y_root = 0;
        xev.state = 0;
        xev.state |= _ecore_x_event_reverse_modifiers(ev->modifiers);
        xev.state |= _ecore_x_event_reverse_locks(ev->locks);
        xev.keycode = _keycode_get(dsp, ev->keyname);
        xev.same_screen = True;

        if(ic)
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
                    {
                       return EINA_FALSE;
                    }

                  compose = tmp;

#ifdef X_HAVE_UTF8_STRING
                  val = Xutf8LookupString(ic,
                                          (XKeyEvent *)&xev,
                                          tmp,
                                          val,
                                          &sym,
                                          &mbstatus);
#else /* ifdef X_HAVE_UTF8_STRING */
                  val = XmbLookupString(ic,
                                        (XKeyEvent *)&xev,
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
        else {
             XComposeStatus status;
             val = XLookupString(&xev,
                                 compose_buffer,
                                 sizeof(compose_buffer),
                                 &sym,
                                 &status);
             if (val > 0)
               {
                  compose_buffer[val] = '\0';
                  compose = eina_str_convert(nl_langinfo(CODESET),
                                             "UTF-8", compose_buffer);
               }
          }

        if(compose)
          {
             Eina_Unicode *unicode;
             int len;
             unicode = eina_unicode_utf8_to_unicode(compose, &len);
             if(!unicode) abort();
             if(unicode[0] >= 0x20 && unicode[0] != 0x7f)
               {
                  ecore_imf_context_commit_event_add(ctx, compose);
                  result = EINA_TRUE;
               }
             free(compose);
             free(unicode);
          }
     }

   return result;
#else
   return EINA_FALSE;
#endif
}

static const Ecore_IMF_Context_Info xim_info = {
   .id = "xim",
   .description = _("X input method"),
   .default_locales = "ko:ja:th:zh",
   .canvas_type = "evas",
   .canvas_required = 1,
};

static Ecore_IMF_Context_Class xim_class = {
   .add = _ecore_imf_context_xim_add,
   .del = _ecore_imf_context_xim_del,
   .client_window_set = _ecore_imf_context_xim_client_window_set,
   .client_canvas_set = NULL,
   .show = NULL,
   .hide = NULL,
   .preedit_string_get = _ecore_imf_context_xim_preedit_string_get,
   .focus_in = _ecore_imf_context_xim_focus_in,
   .focus_out = _ecore_imf_context_xim_focus_out,
   .reset = _ecore_imf_context_xim_reset,
   .cursor_position_set = NULL,
   .use_preedit_set = _ecore_imf_context_xim_use_preedit_set,
   .input_mode_set = NULL,
   .filter_event = _ecore_imf_context_xim_filter_event,
   .preedit_string_with_attributes_get = NULL,
   .prediction_allow_set = NULL,
   .autocapital_type_set = NULL,
   .control_panel_show = NULL,
   .control_panel_hide = NULL,
   .input_panel_layout_set = NULL,
   .input_panel_layout_get = NULL,
   .input_panel_language_set = NULL,
   .input_panel_language_get = NULL,
   .cursor_location_set = _ecore_imf_context_xim_cursor_location_set,
};

static Ecore_IMF_Context *
xim_imf_module_create(void)
{
   EINA_LOG_DBG("%s in", __FUNCTION__);
   Ecore_IMF_Context *ctx = NULL;

   ctx = ecore_imf_context_new(&xim_class);
   if(!ctx)
     goto error;

   return ctx;

error:
   free(ctx);
   return NULL;
}

static Ecore_IMF_Context *
xim_imf_module_exit(void)
{
   return NULL;
}

Eina_Bool
ecore_imf_xim_init(void)
{
   EINA_LOG_DBG("%s in", __FUNCTION__);
   eina_init();
   ecore_x_init(NULL);
   ecore_imf_module_register(&xim_info,
                             xim_imf_module_create,
                             xim_imf_module_exit);

   return EINA_TRUE;
}

void
ecore_imf_xim_shutdown(void)
{
#ifdef ENABLE_XIM
   while (open_ims) {
        XIM_Im_Info *info = open_ims->data;
        Ecore_X_Display *display = ecore_x_display_get();

        xim_info_display_closed(display, EINA_FALSE, info);
     }
#endif

   ecore_x_shutdown();
   eina_shutdown();
}

EINA_MODULE_INIT(ecore_imf_xim_init);
EINA_MODULE_SHUTDOWN(ecore_imf_xim_shutdown);

#ifdef ENABLE_XIM
/*
 * internal functions
 */
Ecore_IMF_Context_Data *
imf_context_data_new()
{
   Ecore_IMF_Context_Data *imf_context_data = NULL;
   char *locale;

   locale = setlocale(LC_CTYPE, "");
   if(!locale) return NULL;

   if(!XSupportsLocale()) return NULL;

   imf_context_data = calloc(1, sizeof(Ecore_IMF_Context_Data));
   if(!imf_context_data) return NULL;

   imf_context_data->locale = strdup(locale);
   if(!imf_context_data->locale) goto error;

   return imf_context_data;
error:
   imf_context_data_destroy(imf_context_data);
   return NULL;
}

void
imf_context_data_destroy(Ecore_IMF_Context_Data *imf_context_data)
{
   if(!imf_context_data)
     return;

   if(imf_context_data->ic)
     XDestroyIC(imf_context_data->ic);

   free(imf_context_data->preedit_chars);
   free(imf_context_data->locale);
   free(imf_context_data);
}

static int
preedit_start_callback(XIC      xic __UNUSED__,
                       XPointer client_data,
                       XPointer call_data __UNUSED__)
{
   EINA_LOG_DBG("in");
   Ecore_IMF_Context *ctx = (Ecore_IMF_Context *)client_data;
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);

   if(imf_context_data->finalizing == EINA_FALSE)
     ecore_imf_context_preedit_start_event_add(ctx);

   return -1;
}

static void
preedit_done_callback(XIC      xic __UNUSED__,
                      XPointer client_data,
                      XPointer call_data __UNUSED__)
{
   EINA_LOG_DBG("in");
   Ecore_IMF_Context *ctx = (Ecore_IMF_Context *)client_data;
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);

   if(imf_context_data->preedit_length)
     {
        imf_context_data->preedit_length = 0;
        free(imf_context_data->preedit_chars);
        imf_context_data->preedit_chars = NULL;
        ecore_imf_context_preedit_changed_event_add(ctx);
     }

   if(imf_context_data->finalizing == EINA_FALSE)
     ecore_imf_context_preedit_end_event_add(ctx);
}

/* FIXME */
static int
xim_text_to_utf8(Ecore_IMF_Context *ctx __UNUSED__,
                 XIMText           *xim_text,
                 char             **text)
{
   int text_length = 0;
   char *result = NULL;

   if(xim_text && xim_text->string.multi_byte)
     {
        if(xim_text->encoding_is_wchar)
          {
             EINA_LOG_WARN("Wide character return from Xlib not currently supported");
             *text = NULL;
             return 0;
          }

        /* XXX Convert to UTF-8 */
        result = strdup(xim_text->string.multi_byte);
        if(result)
          {
             text_length = eina_unicode_utf8_get_len(result);
             if (text_length != xim_text->length)
               {
                  EINA_LOG_WARN("Size mismatch when converting text from input method: supplied length = %d\n, result length = %d", xim_text->length, text_length);
               }
          }
        else {
             EINA_LOG_WARN("Error converting text from IM to UCS-4");
             *text = NULL;
             return 0;
          }

        *text = result;
        return text_length;
     }
   else {
        *text = NULL;
        return 0;
     }
}

static void
preedit_draw_callback(XIC                           xic __UNUSED__,
                      XPointer                      client_data,
                      XIMPreeditDrawCallbackStruct *call_data)
{
   EINA_LOG_DBG("in");
   Eina_Bool ret = EINA_FALSE;
   Ecore_IMF_Context *ctx = (Ecore_IMF_Context *)client_data;
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   XIMText *t = call_data->text;
   char *tmp;
   Eina_Unicode *new_text = NULL;
   Eina_UStrbuf *preedit_bufs = NULL;
   int new_text_length;

   preedit_bufs = eina_ustrbuf_new();
   if(imf_context_data->preedit_chars) {
      ret = eina_ustrbuf_append(preedit_bufs, imf_context_data->preedit_chars);
      if(ret == EINA_FALSE) goto done;
   }

   new_text_length = xim_text_to_utf8(ctx, t, &tmp);
   if(tmp)
     {
        int tmp_len;
        new_text = eina_unicode_utf8_to_unicode((const char *)tmp, &tmp_len);
        free(tmp);
     }

   if(t == NULL) {
      /* delete string */
      ret = eina_ustrbuf_remove(preedit_bufs,
                                call_data->chg_first, call_data->chg_length);
   } else if(call_data->chg_length == 0) {
      /* insert string */
      ret = eina_ustrbuf_insert(preedit_bufs, new_text, call_data->chg_first);
   } else if(call_data->chg_length > 0) {
      /* replace string */
      ret = eina_ustrbuf_remove(preedit_bufs,
                                call_data->chg_first, call_data->chg_length);
      if(ret == EINA_FALSE) goto done;

      ret = eina_ustrbuf_insert_n(preedit_bufs, new_text, 
                                  new_text_length, call_data->chg_first);
      if(ret == EINA_FALSE) goto done;
   } else {
      ret = EINA_FALSE;
   }

 done:
   if(ret == EINA_TRUE) {
      free(imf_context_data->preedit_chars);
      imf_context_data->preedit_chars = 
          eina_ustrbuf_string_steal(preedit_bufs);
      imf_context_data->preedit_length =
          eina_unicode_strlen(imf_context_data->preedit_chars);

      ecore_imf_context_preedit_changed_event_add(ctx);
   }

   free(new_text);
   eina_ustrbuf_free(preedit_bufs);
}

static void
preedit_caret_callback(XIC                            xic __UNUSED__,
                       XPointer                       client_data,
                       XIMPreeditCaretCallbackStruct *call_data)
{
   EINA_LOG_DBG("in");
   Ecore_IMF_Context *ctx = (Ecore_IMF_Context *)client_data;
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);

   if(call_data->direction == XIMAbsolutePosition)
     {
        // printf("call_data->position:%d\n", call_data->position);
          imf_context_data->preedit_cursor = call_data->position;
          if(imf_context_data->finalizing == EINA_FALSE)
            ecore_imf_context_preedit_changed_event_add(ctx);
     }
}

static XVaNestedList
preedit_callback_set(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data;
   imf_context_data = ecore_imf_context_data_get(ctx);

   imf_context_data->preedit_start_cb.client_data = (XPointer)ctx;
   imf_context_data->preedit_start_cb.callback = (XIMProc)preedit_start_callback;

   imf_context_data->preedit_done_cb.client_data = (XPointer)ctx;
   imf_context_data->preedit_done_cb.callback = (XIMProc)preedit_done_callback;

   imf_context_data->preedit_draw_cb.client_data = (XPointer)ctx;
   imf_context_data->preedit_draw_cb.callback = (XIMProc)preedit_draw_callback;

   imf_context_data->preedit_caret_cb.client_data = (XPointer)ctx;
   imf_context_data->preedit_caret_cb.callback = (XIMProc)preedit_caret_callback;

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
get_ic(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data;
   XIC ic;
   imf_context_data = ecore_imf_context_data_get(ctx);
   ic = imf_context_data->ic;
   if(!ic)
     {
        XIM_Im_Info *im_info = imf_context_data->im_info;
        XVaNestedList preedit_attr = NULL;
        XIMStyle im_style = 0;
        XPoint spot = { 0, 0 };
        char *name = NULL;
        
        // supported styles
#if 0
        int i;
        if (im_info->xim_styles)
          {
             for (i = 0; i < im_info->xim_styles->count_styles; i++)
               {
                  printf("%i: ", i);
                  if (im_info->xim_styles->supported_styles[i] & XIMPreeditCallbacks)
                     printf("XIMPreeditCallbacks | ");
                  if (im_info->xim_styles->supported_styles[i] & XIMPreeditPosition)
                     printf("XIMPreeditPosition | ");
                  if (im_info->xim_styles->supported_styles[i] & XIMPreeditArea)
                     printf("XIMPreeditArea | ");
                  if (im_info->xim_styles->supported_styles[i] & XIMPreeditNothing)
                     printf("XIMPreeditNothing | ");
                  if (im_info->xim_styles->supported_styles[i] & XIMPreeditNone)
                     printf("XIMPreeditNone | ");
                  if (im_info->xim_styles->supported_styles[i] & XIMStatusArea)
                     printf("XIMStatusArea | ");
                  if (im_info->xim_styles->supported_styles[i] & XIMStatusCallbacks)
                     printf("XIMStatusCallbacks | ");
                  if (im_info->xim_styles->supported_styles[i] & XIMStatusNothing)
                     printf("XIMStatusNothing | ");
                  if (im_info->xim_styles->supported_styles[i] & XIMStatusNone)
                     printf("XIMStatusNone | ");
                  printf("\n");
               }
          }
#endif
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
                  preedit_attr = preedit_callback_set(ctx);
               }
             name = XNPreeditAttributes;
          }
        else 
          {
             im_style |= XIMPreeditNothing;
             im_style |= XIMStatusNothing;
          }

        if (im_info->im)
          {
             ic = XCreateIC(im_info->im,
                            XNInputStyle, im_style,
                            XNClientWindow, imf_context_data->win,
                            name, preedit_attr, NULL);
          }
        XFree(preedit_attr);
        if(ic)
          {
             unsigned long mask = 0xaaaaaaaa;
             XGetICValues (ic,
                           XNFilterEvents, &mask,
                           NULL);
             imf_context_data->mask = mask;
             ecore_x_event_mask_set(imf_context_data->win, mask);
          }

        imf_context_data->ic = ic;
        if(ic && imf_context_data->has_focus == EINA_TRUE)
          XSetICFocus(ic);
     }

   return ic;
}

static void
reinitialize_ic(Ecore_IMF_Context *ctx)
{
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   XIC ic = imf_context_data->ic;
   if(ic)
     {
        XDestroyIC(ic);
        imf_context_data->ic = NULL;
        if(imf_context_data->preedit_length)
          {
             imf_context_data->preedit_length = 0;
             free(imf_context_data->preedit_chars);
             imf_context_data->preedit_chars = NULL;
             ecore_imf_context_preedit_changed_event_add(ctx);
          }
     }
}

static void
reinitialize_all_ics(XIM_Im_Info *info)
{
   Eina_List *tmp_list;
   Ecore_IMF_Context *ctx;

   EINA_LIST_FOREACH(info->ics, tmp_list, ctx)
     reinitialize_ic(ctx);
}

static void
set_ic_client_window(Ecore_IMF_Context *ctx,
                     Ecore_X_Window     window)
{
   EINA_LOG_DBG("in");
   Ecore_IMF_Context_Data *imf_context_data = ecore_imf_context_data_get(ctx);
   Ecore_X_Window old_win;

   /* reinitialize IC */
   reinitialize_ic(ctx);

   old_win = imf_context_data->win;
   EINA_LOG_DBG("old_win:%d window:%d ", old_win, window);
   if(old_win != 0 && old_win != window)   /* XXX how do check window... */
     {
        XIM_Im_Info *info;
        info = imf_context_data->im_info;
        info->ics = eina_list_remove(info->ics, imf_context_data);
        imf_context_data->im_info = NULL;
     }

   imf_context_data->win = window;

   if(window) /* XXX */
     {
        XIM_Im_Info *info = NULL;
        info = get_im(window, imf_context_data->locale);
        imf_context_data->im_info = info;
        imf_context_data->im_info->ics =
          eina_list_prepend(imf_context_data->im_info->ics,
                            imf_context_data);
     }
}

static XIM_Im_Info *
get_im(Ecore_X_Window window,
       char          *locale)
{
   EINA_LOG_DBG("in");

   Eina_List *l;
   XIM_Im_Info *im_info = NULL;
   XIM_Im_Info *info = NULL;
   EINA_LIST_FOREACH(open_ims, l, im_info) {
        if(strcmp(im_info->locale, locale) == 0)
          {
             if(im_info->im)
               {
                  return im_info;
               }
             else {
                  info = im_info;
                  break;
               }
          }
     }

   if(!info)
     {
        info = calloc(1, sizeof(XIM_Im_Info));
        if(!info) return NULL;
        open_ims = eina_list_prepend(open_ims, info);
        info->win = window;
        info->locale = strdup(locale);
        info->reconnecting = EINA_FALSE;
     }

   xim_info_try_im(info);
   return info;
}

/* initialize info->im */
static void
xim_info_try_im(XIM_Im_Info *info)
{
   Ecore_X_Display *dsp;

   assert(info->im == NULL);
   if (info->reconnecting == EINA_TRUE)
     return;

   if(XSupportsLocale())
     {
        if (!XSetLocaleModifiers (""))
          EINA_LOG_WARN("Unable to set locale modifiers with XSetLocaleModifiers()");
        dsp = ecore_x_display_get();
        info->im = XOpenIM(dsp, NULL, NULL, NULL);
        if(!info->im)
          {
             XRegisterIMInstantiateCallback(dsp,
                                            NULL, NULL, NULL,
                                            xim_instantiate_callback,
                                            (XPointer)info);
             info->reconnecting = EINA_TRUE;
             return;
          }
        setup_im(info);
     }
}

static void
xim_info_display_closed(Ecore_X_Display *display __UNUSED__,
                        int              is_error __UNUSED__,
                        XIM_Im_Info     *info)
{
   Eina_List *ics, *tmp_list;
   Ecore_IMF_Context *ctx;

   open_ims = eina_list_remove(open_ims, info);

   ics = info->ics;
   info->ics = NULL;

   EINA_LIST_FOREACH(ics, tmp_list, ctx)
     set_ic_client_window(ctx, 0);

   EINA_LIST_FREE(ics, ctx) {
        Ecore_IMF_Context_Data *imf_context_data;
        imf_context_data = ecore_imf_context_data_get(ctx);
        imf_context_data_destroy(imf_context_data);
     }

   free (info->locale);

   if (info->im)
     XCloseIM (info->im);

   free (info);
}

static void
xim_instantiate_callback(Display *display,
                         XPointer client_data,
                         XPointer call_data __UNUSED__)
{
   XIM_Im_Info *info = (XIM_Im_Info *)client_data;
   XIM im = NULL;

   im = XOpenIM(display, NULL, NULL, NULL);

   if (!im)
     {
        fprintf(stderr, "Failed to connect to IM\n");
        return;
     }

   info->im = im;
   setup_im (info);

   XUnregisterIMInstantiateCallback (display, NULL, NULL, NULL,
                                     xim_instantiate_callback,
                                     (XPointer)info);
   info->reconnecting = EINA_FALSE;
}

static void
setup_im(XIM_Im_Info *info)
{
   XIMValuesList *ic_values = NULL;
   XIMCallback im_destroy_callback;

   if(!info->im)
     return;

   im_destroy_callback.client_data = (XPointer)info;
   im_destroy_callback.callback = (XIMProc)xim_destroy_callback;
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
#if 0
        printf("values........\n");
        for (i = 0; i < ic_values->count_values; i++)
           printf("%s\n", ic_values->supported_values[i]);
        printf("styles........\n");
        for (i = 0; i < info->xim_styles->count_styles; i++)
           printf("%lx\n", info->xim_styles->supported_styles[i]);
#endif
        XFree(ic_values);
     }
}

static void
xim_destroy_callback(XIM      xim __UNUSED__,
                     XPointer client_data,
                     XPointer call_data __UNUSED__)
{
   XIM_Im_Info *info = (XIM_Im_Info *)client_data;
   info->im = NULL;

   reinitialize_all_ics(info);
   xim_info_try_im(info);

   return;
}

#endif  /* ENABLE_XIM */
