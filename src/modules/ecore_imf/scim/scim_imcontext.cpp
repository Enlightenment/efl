#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define Uses_SCIM_DEBUG
#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_PANEL_CLIENT

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <pthread.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>
#include <Ecore.h>
#include <Evas.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <scim.h>
#pragma GCC diagnostic pop

#include "scim_imcontext.h"

using namespace scim;

struct _EcoreIMFContextISFImpl
{
    EcoreIMFContextISF      *parent;
    IMEngineInstancePointer  si;
    Ecore_X_Window           client_window;
    Evas                    *client_canvas;
    Ecore_IMF_Input_Mode     input_mode;
    WideString               preedit_string;
    AttributeList            preedit_attrlist;
    Ecore_IMF_Autocapital_Type autocapital_type;
    int                      preedit_caret;
    int                      cursor_x;
    int                      cursor_y;
    int                      cursor_pos;
    bool                     use_preedit;
    bool                     is_on;
    bool                     shared_si;
    bool                     preedit_started;
    bool                     preedit_updating;
    bool                     prediction_allow;

    EcoreIMFContextISFImpl  *next;
};

/* Input Context handling functions. */
static EcoreIMFContextISFImpl *new_ic_impl              (EcoreIMFContextISF     *parent);
static void                    delete_ic_impl           (EcoreIMFContextISFImpl *impl);
static void                    delete_all_ic_impl       (void);

static EcoreIMFContextISF     *find_ic                  (int                     id);


/* private functions */
static void     panel_slot_reload_config                (int                     context);
static void     panel_slot_exit                         (int                     context);
static void     panel_slot_update_lookup_table_page_size(int                     context,
                                                         int                     page_size);
static void     panel_slot_lookup_table_page_up         (int                     context);
static void     panel_slot_lookup_table_page_down       (int                     context);
static void     panel_slot_trigger_property             (int                     context,
                                                         const String           &property);
static void     panel_slot_process_helper_event         (int                     context,
                                                         const String           &target_uuid,
                                                         const String           &helper_uuid,
                                                         const Transaction      &trans);
static void     panel_slot_move_preedit_caret           (int                     context,
                                                         int                     caret_pos);
static void     panel_slot_select_candidate             (int                     context,
                                                         int                     cand_index);
static void     panel_slot_process_key_event            (int                     context,
                                                         const KeyEvent         &key);
static void     panel_slot_commit_string                (int                     context,
                                                         const WideString       &wstr);
static void     panel_slot_forward_key_event            (int                     context,
                                                         const KeyEvent         &key);
static void     panel_slot_request_help                 (int                     context);
static void     panel_slot_request_factory_menu         (int                     context);
static void     panel_slot_change_factory               (int                     context,
                                                         const String           &uuid);

static void     panel_req_focus_in                      (EcoreIMFContextISF     *ic);
static void     panel_req_update_factory_info           (EcoreIMFContextISF     *ic);
static void     panel_req_update_spot_location          (EcoreIMFContextISF     *ic);
static void     panel_req_show_help                     (EcoreIMFContextISF     *ic);
static void     panel_req_show_factory_menu             (EcoreIMFContextISF     *ic);

/* Panel iochannel handler*/
static bool     panel_initialize                        (void);
static void     panel_finalize                          (void);
static Eina_Bool panel_iochannel_handler                (void                   *data,
                                                         Ecore_Fd_Handler       *fd_handler);

/* utility functions */
static bool     filter_hotkeys                          (EcoreIMFContextISF     *ic,
                                                         const KeyEvent         &key);
static void     turn_on_ic                              (EcoreIMFContextISF     *ic);
static void     turn_off_ic                             (EcoreIMFContextISF     *ic);
static void     set_ic_capabilities                     (EcoreIMFContextISF     *ic);

static void     initialize                              (void);
static void     finalize                                (void);

static void     open_next_factory                       (EcoreIMFContextISF     *ic);
static void     open_previous_factory                   (EcoreIMFContextISF     *ic);
static void     open_specific_factory                   (EcoreIMFContextISF     *ic,
                                                         const String           &uuid);
static void     initialize_modifier_bits                (Display *display);
static unsigned int scim_x11_keymask_scim_to_x11        (Display *display, uint16 scimkeymask);
static XKeyEvent createKeyEvent                         (Display *display, Window &win,
                                                         Window &winRoot, bool press,
                                                         int keysym, int modifiers);
static void     _x_send_key_event                       (const KeyEvent &key);

static void     attach_instance                         (const IMEngineInstancePointer &si);

/* slot functions */
static void     slot_show_preedit_string                (IMEngineInstanceBase   *si);
static void     slot_show_aux_string                    (IMEngineInstanceBase   *si);
static void     slot_show_lookup_table                  (IMEngineInstanceBase   *si);

static void     slot_hide_preedit_string                (IMEngineInstanceBase   *si);
static void     slot_hide_aux_string                    (IMEngineInstanceBase   *si);
static void     slot_hide_lookup_table                  (IMEngineInstanceBase   *si);

static void     slot_update_preedit_caret               (IMEngineInstanceBase   *si,
                                                         int                     caret);
static void     slot_update_preedit_string              (IMEngineInstanceBase   *si,
                                                         const WideString       &str,
                                                         const AttributeList    &attrs);
static void     slot_update_aux_string                  (IMEngineInstanceBase   *si,
                                                         const WideString       &str,
                                                         const AttributeList    &attrs);
static void     slot_commit_string                      (IMEngineInstanceBase   *si,
                                                         const WideString       &str);
static void     slot_forward_key_event                  (IMEngineInstanceBase   *si,
                                                         const KeyEvent         &key);
static void     slot_update_lookup_table                (IMEngineInstanceBase   *si,
                                                         const LookupTable      &table);

static void     slot_register_properties                (IMEngineInstanceBase   *si,
                                                         const PropertyList     &properties);
static void     slot_update_property                    (IMEngineInstanceBase   *si,
                                                         const Property         &property);
static void     slot_beep                               (IMEngineInstanceBase   *si);
static void     slot_start_helper                       (IMEngineInstanceBase   *si,
                                                         const String           &helper_uuid);
static void     slot_stop_helper                        (IMEngineInstanceBase   *si,
                                                         const String           &helper_uuid);
static void     slot_send_helper_event                  (IMEngineInstanceBase   *si,
                                                         const String           &helper_uuid,
                                                         const Transaction      &trans);
static bool     slot_get_surrounding_text               (IMEngineInstanceBase   *si,
                                                         WideString             &text,
                                                         int                    &cursor,
                                                         int                     maxlen_before,
                                                         int                     maxlen_after);
static bool     slot_delete_surrounding_text            (IMEngineInstanceBase   *si,
                                                         int                     offset,
                                                         int                     len);

static void     reload_config_callback                  (const ConfigPointer    &config);

static void     fallback_commit_string_cb               (IMEngineInstanceBase   *si,
                                                         const WideString       &str);

/* Local variables declaration */
static String                                           _language;
static EcoreIMFContextISFImpl                          *_used_ic_impl_list          = 0;
static EcoreIMFContextISFImpl                          *_free_ic_impl_list          = 0;
static EcoreIMFContextISF                              *_ic_list                    = 0;

static KeyboardLayout                                   _keyboard_layout            = SCIM_KEYBOARD_Default;
static int                                              _valid_key_mask             = SCIM_KEY_AllMasks;

static FrontEndHotkeyMatcher                            _frontend_hotkey_matcher;
static IMEngineHotkeyMatcher                            _imengine_hotkey_matcher;

static IMEngineInstancePointer                          _default_instance;

static ConfigModule                                    *_config_module              = 0;
static ConfigPointer                                    _config;
static BackEndPointer                                   _backend;

static EcoreIMFContextISF                              *_focused_ic                 = 0;

static bool                                             _scim_initialized           = false;

static int                                              _instance_count             = 0;
static int                                              _context_count              = 0;

static IMEngineFactoryPointer                           _fallback_factory;
static IMEngineInstancePointer                          _fallback_instance;
static PanelClient                                      _panel_client;

static Ecore_Fd_Handler                                *_panel_iochannel_read_handler = 0;
static Ecore_Fd_Handler                                *_panel_iochannel_err_handler  = 0;

static Ecore_X_Window                                  _client_window               = 0;

static bool                                             _on_the_spot                = true;
static bool                                             _shared_input_method        = false;

static Display *__current_display      = 0;
static int      __current_alt_mask     = Mod1Mask;
static int      __current_meta_mask    = 0;
static int      __current_super_mask   = 0;
static int      __current_hyper_mask   = 0;
static int      __current_numlock_mask = Mod2Mask;

// A hack to shutdown the immodule cleanly even if im_module_exit() is not called when exiting.
class FinalizeHandler
{
public:
   FinalizeHandler()
     {
        SCIM_DEBUG_FRONTEND(1) << "FinalizeHandler::FinalizeHandler()\n";
     }
   ~FinalizeHandler()
     {
        SCIM_DEBUG_FRONTEND(1) << "FinalizeHandler::~FinalizeHandler()\n";
        isf_imf_context_shutdown();
     }
};

static FinalizeHandler                                  _finalize_handler;

static unsigned int
utf8_offset_to_index(const char *str, int offset)
{
   int index = 0;
   int i;
   for (i = 0; i < offset; i++)
     {
        eina_unicode_utf8_next_get(str, &index);
     }

   return index;
}

static unsigned int
get_time(void)
{
   unsigned int tint;
   struct timeval tv;
   struct timezone tz;           /* is not used since ages */
   gettimeofday(&tv, &tz);
   tint = tv.tv_sec * 1000;
   tint = tint / 1000 * 1000;
   tint = tint + tv.tv_usec / 1000;
   return tint;
}

/* Function Implementations */
static EcoreIMFContextISFImpl *
new_ic_impl(EcoreIMFContextISF *parent)
{
   EcoreIMFContextISFImpl *impl = NULL;

   if (_free_ic_impl_list != NULL)
     {
        impl = _free_ic_impl_list;
        _free_ic_impl_list = _free_ic_impl_list->next;
     }
   else
     {
        impl = new EcoreIMFContextISFImpl;
        if (impl == NULL)
          return NULL;
     }

   impl->autocapital_type = ECORE_IMF_AUTOCAPITAL_TYPE_NONE;
   impl->next = _used_ic_impl_list;
   _used_ic_impl_list = impl;

   impl->parent = parent;

   return impl;
}

static void
delete_ic_impl(EcoreIMFContextISFImpl *impl)
{
   EcoreIMFContextISFImpl *rec = _used_ic_impl_list, *last = 0;

   for (; rec != 0; last = rec, rec = rec->next)
     {
        if (rec == impl)
          {
             if (last != 0)
               last->next = rec->next;
             else
               _used_ic_impl_list = rec->next;

             rec->next = _free_ic_impl_list;
             _free_ic_impl_list = rec;

             rec->parent = 0;
             rec->si.reset();
             rec->client_window = 0;
             rec->preedit_string = WideString();
             rec->preedit_attrlist.clear();

             return;
          }
     }
}

static void
delete_all_ic_impl(void)
{
   EcoreIMFContextISFImpl *it = _used_ic_impl_list;

   while (it != 0)
     {
        _used_ic_impl_list = it->next;
        delete it;
        it = _used_ic_impl_list;
     }

   it = _free_ic_impl_list;
   while (it != 0)
     {
        _free_ic_impl_list = it->next;
        delete it;
        it = _free_ic_impl_list;
     }
}

static EcoreIMFContextISF *
find_ic(int id)
{
   EcoreIMFContextISFImpl *rec = _used_ic_impl_list;

   while (rec != 0)
     {
        if (rec->parent && rec->parent->id == id)
          return rec->parent;
        rec = rec->next;
     }

   return 0;
}

static void
feed_key_event(Evas *evas, const char *str, Eina_Bool fake)
{
   char key_string[128] = {0};
   unsigned int timestamp = 0;

   if (!fake)
     timestamp = get_time();

   if (strncmp(str, "KeyRelease+", 11) == 0)
     {
        strncpy(key_string, str + 11, strlen(str)-11);
        evas_event_feed_key_up(evas, key_string, key_string, NULL, NULL, timestamp, NULL);
        SCIM_DEBUG_FRONTEND(1) << "    evas_event_feed_key_up()...\n";
     }
   else
     {
	if (strlen(str) + 1 > 128) return;
        strncpy(key_string, str, strlen(str));
        evas_event_feed_key_down(evas, key_string, key_string, NULL, NULL, timestamp, NULL);
        SCIM_DEBUG_FRONTEND(1) << "    evas_event_feed_key_down()...\n";
     }
}

static void
window_to_screen_geometry_get(Ecore_X_Window client_win, int *x, int *y)
{
   Ecore_X_Window root_window, win;
   int win_x, win_y;
   int sum_x = 0, sum_y = 0;

   root_window = ecore_x_window_root_get(client_win);
   win = client_win;

   while (root_window != win)
     {
        ecore_x_window_geometry_get(win, &win_x, &win_y, NULL, NULL);
        sum_x += win_x;
        sum_y += win_y;
        win = ecore_x_window_parent_get(win);
     }

   if (x)
     *x = sum_x;
   if (y)
     *y = sum_y;
}

static unsigned int
_ecore_imf_modifier_to_scim_mask(unsigned int modifiers)
{
   unsigned int mask = 0;

   /**< "Control" is pressed */
   if (modifiers & ECORE_IMF_KEYBOARD_MODIFIER_CTRL)
     mask |= SCIM_KEY_ControlMask;

   /**< "Alt" is pressed */
   if (modifiers & ECORE_IMF_KEYBOARD_MODIFIER_ALT)
     mask |= SCIM_KEY_AltMask;

   /**< "Shift" is pressed */
   if (modifiers & ECORE_IMF_KEYBOARD_MODIFIER_SHIFT)
     mask |= SCIM_KEY_ShiftMask;

   /**< "Win" (between "Ctrl" and "Alt") is pressed */
   if (modifiers & ECORE_IMF_KEYBOARD_MODIFIER_WIN)
     mask |= SCIM_KEY_SuperMask;

   /**< "AltGr" is pressed */
   if (modifiers & ECORE_IMF_KEYBOARD_MODIFIER_ALTGR)
     mask |= SCIM_KEY_Mod5Mask;

   return mask;
}

static unsigned int
_ecore_imf_lock_to_scim_mask(unsigned int locks)
{
   unsigned int mask = 0;

   if (locks & ECORE_IMF_KEYBOARD_LOCK_CAPS)
     mask |= SCIM_KEY_CapsLockMask;

   if (locks & ECORE_IMF_KEYBOARD_LOCK_NUM)
     mask |= SCIM_KEY_NumLockMask;

   return mask;
}

/* Public functions */
/**
 * isf_imf_context_new
 *
 * This function will be called by Ecore IMF.
 * Create a instance of type EcoreIMFContextISF.
 *
 * Return value: A pointer to the newly created EcoreIMFContextISF instance
 */
EAPI EcoreIMFContextISF *
isf_imf_context_new(void)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = new EcoreIMFContextISF;
   if (context_scim == NULL)
     {
        std::cerr << "memory allocation failed in " << __FUNCTION__ << "\n";
        return NULL;
     }

   context_scim->id = _context_count++;

   if (!_scim_initialized)
     {
        initialize();
        _scim_initialized = true;
     }

   return context_scim;
}

/**
 * isf_imf_context_shutdown
 *
 * It will be called when the scim im module is unloaded by ecore. It will do some
 * cleanup job.
 */
EAPI void
isf_imf_context_shutdown(void)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   if (_scim_initialized)
     {
        _scim_initialized = false;
        finalize();
     }
}

EAPI void
isf_imf_context_add(Ecore_IMF_Context *ctx)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);

   context_scim->impl = NULL;

   if (_backend.null())
     return;

   IMEngineInstancePointer si;

   // Use the default instance if "shared input method" mode is enabled.
   if (_shared_input_method && !_default_instance.null())
     {
        si = _default_instance;
        SCIM_DEBUG_FRONTEND(2) << "use default instance: " << si->get_id() << " " << si->get_factory_uuid() << "\n";
     }

   // Not in "shared input method" mode, or no default instance, create an instance.
   if (si.null())
     {
        IMEngineFactoryPointer factory = _backend->get_default_factory(_language, "UTF-8");
        if (factory.null()) return;
        si = factory->create_instance("UTF-8", _instance_count++);
        if (si.null()) return;
        attach_instance(si);
        SCIM_DEBUG_FRONTEND(2) << "create new instance: " << si->get_id() << " " << si->get_factory_uuid() << "\n";
     }

   // If "shared input method" mode is enabled, and there is no default instance,
   // then store this instance as default one.
   if (_shared_input_method && _default_instance.null())
     {
        SCIM_DEBUG_FRONTEND(2) << "update default instance.\n";
        _default_instance = si;
     }

   context_scim->ctx                       = ctx;
   context_scim->impl                      = new_ic_impl(context_scim);
   if (context_scim->impl == NULL)
     {
        std::cerr << "memory allocation failed in " << __FUNCTION__ << "\n";
        return;
     }

   context_scim->impl->si                  = si;
   context_scim->impl->client_window       = 0;
   context_scim->impl->client_canvas       = NULL;
   context_scim->impl->preedit_caret       = 0;
   context_scim->impl->cursor_x            = 0;
   context_scim->impl->cursor_y            = 0;
   context_scim->impl->cursor_pos          = -1;
   context_scim->impl->is_on               = false;
   context_scim->impl->shared_si           = _shared_input_method;
   context_scim->impl->use_preedit         = _on_the_spot;
   context_scim->impl->preedit_started     = false;
   context_scim->impl->preedit_updating    = false;

   if (!_ic_list)
     context_scim->next = NULL;
   else
     context_scim->next = _ic_list;
   _ic_list = context_scim;

   if (_shared_input_method)
     context_scim->impl->is_on = _config->read(String(SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), context_scim->impl->is_on);

   _panel_client.prepare(context_scim->id);
   _panel_client.register_input_context(context_scim->id, si->get_factory_uuid());
   set_ic_capabilities(context_scim);
   _panel_client.send();

   SCIM_DEBUG_FRONTEND(2) << "input context created: id = " << context_scim->id << "\n";
}

EAPI void
isf_imf_context_del(Ecore_IMF_Context *ctx)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   if (!_ic_list) return;

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim->id != _ic_list->id)
     {
        EcoreIMFContextISF *pre = _ic_list;
        EcoreIMFContextISF *cur = _ic_list->next;
        while (cur != NULL)
          {
             if (cur->id == context_scim->id)
               {
                  pre->next = cur->next;
                  break;
               }
             pre = cur;
             cur = cur->next;
          }
     }
   else
     _ic_list = _ic_list->next;

   _panel_client.prepare(context_scim->id);

   if (context_scim == _focused_ic)
     context_scim->impl->si->focus_out();

   // Delete the instance.
   EcoreIMFContextISF *old_focused = _focused_ic;
   _focused_ic = context_scim;
   context_scim->impl->si.reset();
   _focused_ic = old_focused;

   if (context_scim == _focused_ic)
     {
        _panel_client.turn_off(context_scim->id);
        _panel_client.focus_out(context_scim->id);
     }

   _panel_client.remove_input_context(context_scim->id);
   _panel_client.send();

   if (context_scim->impl->client_window)
     isf_imf_context_client_window_set(ctx, NULL);

   delete_ic_impl(context_scim->impl);
   context_scim->impl = 0;

   if (context_scim == _focused_ic)
     _focused_ic = 0;

   delete context_scim;
   context_scim = 0;
}

/**
 * isf_imf_context_client_canvas_set
 * @ctx: a #Ecore_IMF_Context
 * @canvas: the client canvas
 *
 * This function will be called by Ecore IMF.
 *
 * Set the client canvas for the Input Method Context; this is the canvas
 * in which the input appears.
 *
 * The canvas type can be determined by using the context canvas type.
 * Actually only canvas with type "evas" (Evas *) is supported. This canvas
 * may be used in order to correctly position status windows, and may also
 * be used for purposes internal to the Input Method Context.
 */
EAPI void
isf_imf_context_client_canvas_set(Ecore_IMF_Context *ctx, void *canvas)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim->impl->client_canvas != (Evas*) canvas)
     context_scim->impl->client_canvas = (Evas*)canvas;
}

/**
 * isf_imf_context_client_window_set
 * @ctx: a #Ecore_IMF_Context
 * @window: the client window
 *
 * This function will be called by Ecore IMF.
 *
 * Set the client window for the Input Method Context; this is the Ecore_X_Window
 * when using X11, Ecore_Win32_Window when using Win32, etc.
 *
 * This window is used in order to correctly position status windows,
 * and may also be used for purposes internal to the Input Method Context.
 */
EAPI void
isf_imf_context_client_window_set(Ecore_IMF_Context *ctx, void *window)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim->impl->client_window != (Ecore_X_Window)((Ecore_Window)window))
     {
        context_scim->impl->client_window = (Ecore_X_Window)((Ecore_Window)window);

        if ((context_scim->impl->client_window != 0) &&
            (context_scim->impl->client_window != _client_window))
          _client_window = context_scim->impl->client_window;
     }
}

/**
 * isf_imf_context_reset
 * @ctx: a #Ecore_IMF_Context
 *
 * This function will be called by Ecore IMF.
 *
 * Notify the Input Method Context that a change such as a change in cursor
 * position has been made. This will typically cause the Input Method Context
 * to clear the preedit state.
 */
EAPI void
isf_imf_context_reset(Ecore_IMF_Context *ctx)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim != _focused_ic)
     return;

   WideString wstr = context_scim->impl->preedit_string;

   _panel_client.prepare(context_scim->id);
   context_scim->impl->si->reset();
   _panel_client.send();
}

/**
 * isf_imf_context_focus_in
 * @ctx: a #Ecore_IMF_Context
 *
 * This function will be called by Ecore IMF.
 *
 * Notify the Input Method Context that the widget to which its correspond has gained focus.
 */
EAPI void
isf_imf_context_focus_in(Ecore_IMF_Context *ctx)
{
   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__<< "(" << context_scim->id << ")...\n";

   if (_focused_ic)
     {
        if (_focused_ic == context_scim)
          {
             SCIM_DEBUG_FRONTEND(1) << "It's already focused.\n";
             return;
          }
        SCIM_DEBUG_FRONTEND(1) << "Focus out previous IC first: " << _focused_ic->id << "\n";
        if (_focused_ic->ctx)
          isf_imf_context_focus_out(_focused_ic->ctx);
     }

   bool need_cap   = false;
   bool need_reset = false;
   bool need_reg   = false;

   _focused_ic = context_scim;
   _panel_client.prepare(context_scim->id);

   // Handle the "Shared Input Method" mode.
   if (_shared_input_method)
     {
        SCIM_DEBUG_FRONTEND(2) << "shared input method.\n";
        IMEngineFactoryPointer factory = _backend->get_default_factory(_language, "UTF-8");
        if (!factory.null())
          {
             if (_default_instance.null() || _default_instance->get_factory_uuid() != factory->get_uuid())
               {
                  _default_instance = factory->create_instance("UTF-8", _default_instance.null() ? _instance_count++ : _default_instance->get_id());
                  attach_instance(_default_instance);
                  SCIM_DEBUG_FRONTEND(2) << "create new default instance: " << _default_instance->get_id() << " " << _default_instance->get_factory_uuid() << "\n";
               }

             context_scim->impl->shared_si = true;
             context_scim->impl->si = _default_instance;

             context_scim->impl->is_on = _config->read(String(SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), context_scim->impl->is_on);
             context_scim->impl->preedit_string.clear();
             context_scim->impl->preedit_attrlist.clear();
             context_scim->impl->preedit_caret = 0;
             context_scim->impl->preedit_started = false;
             need_cap = true;
             need_reset = true;
             need_reg = true;
          }
     }
   else if (context_scim->impl->shared_si)
     {
        SCIM_DEBUG_FRONTEND(2) << "exit shared input method.\n";
        IMEngineFactoryPointer factory = _backend->get_default_factory(_language, "UTF-8");
        if (!factory.null())
          {
             context_scim->impl->si = factory->create_instance("UTF-8", _instance_count++);
             context_scim->impl->preedit_string.clear();
             context_scim->impl->preedit_attrlist.clear();
             context_scim->impl->preedit_caret = 0;
             context_scim->impl->preedit_started = false;
             attach_instance(context_scim->impl->si);
             need_cap = true;
             need_reg = true;
             context_scim->impl->shared_si = false;
             SCIM_DEBUG_FRONTEND(2) << "create new instance: " << context_scim->impl->si->get_id() << " " << context_scim->impl->si->get_factory_uuid() << "\n";
          }
     }

   context_scim->impl->si->set_frontend_data(static_cast <void*>(context_scim));

   if (need_reg) _panel_client.register_input_context(context_scim->id, context_scim->impl->si->get_factory_uuid());
   if (need_cap) set_ic_capabilities(context_scim);
   if (need_reset) context_scim->impl->si->reset();

   panel_req_focus_in(context_scim);
   panel_req_update_spot_location(context_scim);
   panel_req_update_factory_info(context_scim);

   if (context_scim->impl->is_on)
     {
        _panel_client.turn_on(context_scim->id);
        _panel_client.hide_preedit_string(context_scim->id);
        _panel_client.hide_aux_string(context_scim->id);
        _panel_client.hide_lookup_table(context_scim->id);
        context_scim->impl->si->focus_in();
     }
   else
     {
        _panel_client.turn_off(context_scim->id);
     }

   _panel_client.send();

   if (ecore_imf_context_input_panel_enabled_get(ctx))
     ecore_imf_context_input_panel_show(ctx);
}

/**
 * isf_imf_context_focus_out
 * @ctx: a #Ecore_IMF_Context
 *
 * This function will be called by Ecore IMF.
 *
 * Notify the Input Method Context that the widget to which its correspond has lost focus.
 */
EAPI void
isf_imf_context_focus_out(Ecore_IMF_Context *ctx)
{
   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "(" << context_scim->id << ")...\n";

   if (context_scim == _focused_ic)
     {
        WideString wstr = context_scim->impl->preedit_string;

        _panel_client.prepare(context_scim->id);
        context_scim->impl->si->focus_out();
        context_scim->impl->si->reset();
        _panel_client.turn_off(context_scim->id);
        _panel_client.focus_out(context_scim->id);
        _panel_client.send();
        _focused_ic = 0;
     }

   if (ecore_imf_context_input_panel_enabled_get(ctx))
     ecore_imf_context_input_panel_hide(ctx);
}

/**
 * isf_imf_context_cursor_location_set
 * @ctx: a #Ecore_IMF_Context
 * @x: x position of New cursor.
 * @y: y position of New cursor.
 * @w: the width of New cursor.
 * @h: the height of New cursor.
 *
 * This function will be called by Ecore IMF.
 *
 * Notify the Input Method Context that a change in the cursor location has been made.
 */
EAPI void
isf_imf_context_cursor_location_set(Ecore_IMF_Context *ctx, int cx, int cy, int cw, int ch)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   Ecore_Evas *ee;
   int canvas_x, canvas_y;
   int new_cursor_x, new_cursor_y;

   if (cw == 0 && ch == 0)
     return;

   if (context_scim != _focused_ic)
     return;

   if (context_scim->impl->client_canvas)
     {
        ee = ecore_evas_ecore_evas_get(context_scim->impl->client_canvas);
        if (!ee) return;

        ecore_evas_geometry_get(ee, &canvas_x, &canvas_y, NULL, NULL);
     }
   else
     {
        if (context_scim->impl->client_window)
          window_to_screen_geometry_get(context_scim->impl->client_window, &canvas_x, &canvas_y);
        else
          return;
     }

   new_cursor_x = canvas_x + cx;
   new_cursor_y = canvas_y + cy + ch;

   // Don't update spot location while updating preedit string.
   if (context_scim->impl->preedit_updating && (context_scim->impl->cursor_y == new_cursor_y))
     return;

   if (context_scim->impl->cursor_x != new_cursor_x || context_scim->impl->cursor_y != new_cursor_y)
     {
        context_scim->impl->cursor_x     = new_cursor_x;
        context_scim->impl->cursor_y     = new_cursor_y;
        _panel_client.prepare(context_scim->id);
        panel_req_update_spot_location(context_scim);
        _panel_client.send();
        SCIM_DEBUG_FRONTEND(2) << "new cursor location = " << context_scim->impl->cursor_x << "," << context_scim->impl->cursor_y << "\n";
     }
}

/**
 * isf_imf_context_use_preedit_set
 * @ctx: a #Ecore_IMF_Context
 * @use_preedit: Whether the IM context should use the preedit string.
 *
 * This function will be called by Ecore IMF.
 *
 * Set whether the IM context should use the preedit string to display feedback.
 * If is 0 (default is 1), then the IM context may use some other method to
 * display feedback, such as displaying it in a child of the root window.
 */
EAPI void
isf_imf_context_use_preedit_set(Ecore_IMF_Context* ctx, Eina_Bool use_preedit)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " = " << (use_preedit ? "true" : "false") << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (!_on_the_spot) return;

   bool old = context_scim->impl->use_preedit;
   context_scim->impl->use_preedit = use_preedit;
   if (context_scim == _focused_ic)
     {
        _panel_client.prepare(context_scim->id);

        if (old != use_preedit)
          set_ic_capabilities(context_scim);

        if (context_scim->impl->preedit_string.length())
          slot_show_preedit_string(context_scim->impl->si);

        _panel_client.send();
     }
}

EAPI void
isf_imf_context_preedit_string_with_attributes_get(Ecore_IMF_Context *ctx, char** str, Eina_List **attrs, int *cursor_pos)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim->impl->is_on)
     {
        String mbs = utf8_wcstombs(context_scim->impl->preedit_string);

        if (str)
          {
             if (mbs.length())
               *str = strdup(mbs.c_str());
             else
               *str = strdup("");
          }

        if (cursor_pos)
          {
             *cursor_pos = context_scim->impl->preedit_caret;
          }

        if (attrs)
          {
             if (mbs.length())
               {
                  int start_index, end_index;
                  int wlen = context_scim->impl->preedit_string.length();

                  Ecore_IMF_Preedit_Attr *attr = NULL;
                  AttributeList::const_iterator i;
                  bool *attrs_flag = new bool [mbs.length()];
                  memset(attrs_flag, 0, mbs.length() *sizeof(bool));

                  for (i = context_scim->impl->preedit_attrlist.begin();
                       i != context_scim->impl->preedit_attrlist.end(); ++i)
                    {
                       start_index = i->get_start();
                       end_index = i->get_end();

                       if (end_index <= wlen && start_index < end_index && i->get_type() != SCIM_ATTR_DECORATE_NONE)
                         {
                            start_index = utf8_offset_to_index(mbs.c_str(), i->get_start());
                            end_index = utf8_offset_to_index(mbs.c_str(), i->get_end());

                            if (i->get_type() == SCIM_ATTR_DECORATE)
                              {
                                 attr = (Ecore_IMF_Preedit_Attr *)calloc(1, sizeof(Ecore_IMF_Preedit_Attr));
                                 if (attr == NULL)
                                   continue;
                                 attr->start_index = start_index;
                                 attr->end_index = end_index;

                                 if (i->get_value() == SCIM_ATTR_DECORATE_UNDERLINE)
                                   {
                                      attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB1;
                                      *attrs = eina_list_append(*attrs, (void *)attr);
                                   }
                                 else if (i->get_value() == SCIM_ATTR_DECORATE_REVERSE)
                                   {
                                      attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB2;
                                      *attrs = eina_list_append(*attrs, (void *)attr);
                                   }
                                 else if (i->get_value() == SCIM_ATTR_DECORATE_HIGHLIGHT)
                                   {
                                      attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB3;
                                      *attrs = eina_list_append(*attrs, (void *)attr);
                                   }
                                 else
                                   {
                                      free(attr);
                                   }

                                 switch(i->get_value())
                                   {
                                    case SCIM_ATTR_DECORATE_UNDERLINE:
                                    case SCIM_ATTR_DECORATE_REVERSE:
                                    case SCIM_ATTR_DECORATE_HIGHLIGHT:
                                       // Record which character has attribute.
                                       for (int pos = start_index; pos < end_index; ++pos)
                                         attrs_flag [pos] = 1;
                                       break;
                                    default:
                                       break;
                                   }
                            }
                            else if (i->get_type() == SCIM_ATTR_FOREGROUND)
                              {
                                 SCIM_DEBUG_FRONTEND(4) << "SCIM_ATTR_FOREGROUND\n";
                              }
                            else if (i->get_type() == SCIM_ATTR_BACKGROUND)
                              {
                                 SCIM_DEBUG_FRONTEND(4) << "SCIM_ATTR_BACKGROUND\n";
                              }
                         }
                    }

                  // Add underline for all characters which don't have attribute.
                  for (unsigned int pos = 0; pos < mbs.length(); ++pos)
                    {
                       if (!attrs_flag [pos])
                         {
                            int begin_pos = pos;

                            while (pos < mbs.length() && !attrs_flag[pos])
                              ++pos;

                            // use REVERSE style as default
                            attr = (Ecore_IMF_Preedit_Attr *)calloc(1, sizeof(Ecore_IMF_Preedit_Attr));
                            if (attr == NULL)
                              continue;
                            attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB2;
                            attr->start_index = begin_pos;
                            attr->end_index = pos;
                            *attrs = eina_list_append(*attrs, (void *)attr);
                         }
                    }

                  delete [] attrs_flag;
               }
          }
     }
   else
     {
        if (str)
          *str = strdup("");

        if (cursor_pos)
          *cursor_pos = 0;

        if (attrs)
          *attrs = NULL;
     }
}

/**
 * isf_imf_context_preedit_string_get
 * @ctx: a #Ecore_IMF_Context
 * @str: the preedit string
 * @cursor_pos: the cursor position
 *
 * This function will be called by Ecore IMF.
 *
 * To get the preedit string of the input method.
 */
EAPI void
isf_imf_context_preedit_string_get(Ecore_IMF_Context *ctx, char** str, int *cursor_pos)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim->impl->is_on)
     {
        String mbs = utf8_wcstombs(context_scim->impl->preedit_string);

        if (str)
          {
             if (mbs.length())
               *str = strdup(mbs.c_str());
             else
               *str = strdup("");
          }

        if (cursor_pos)
          *cursor_pos = context_scim->impl->preedit_caret;
     }
   else
     {
        if (str)
          *str = strdup("");

        if (cursor_pos)
          *cursor_pos = 0;
     }
}

/**
 * isf_imf_context_cursor_position_set
 * @ctx: a #Ecore_IMF_Context
 * @cursor_pos: New cursor position in characters.
 *
 * This function will be called by Ecore IMF.
 *
 * Notify the Input Method Context that a change in the cursor position has been made.
 */
EAPI void
isf_imf_context_cursor_position_set(Ecore_IMF_Context *ctx, int cursor_pos)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim != _focused_ic)
     return;

   // Don't update spot location while updating preedit string.
   if (context_scim->impl->preedit_updating)
     return;

   if (context_scim->impl->cursor_pos != cursor_pos)
     context_scim->impl->cursor_pos = cursor_pos;
}

/**
 * isf_imf_context_input_mode_set
 * @ctx: a #Ecore_IMF_Context
 * @input_mode: the input mode
 *
 * This function will be called by Ecore IMF.
 *
 * To set the input mode of input method. The definition of Ecore_IMF_Input_Mode
 * is in Ecore_IMF.h.
 */
EAPI void
isf_imf_context_input_mode_set(Ecore_IMF_Context *ctx, Ecore_IMF_Input_Mode input_mode)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   context_scim->impl->input_mode = input_mode;
}

/**
 * isf_imf_context_prediction_allow_set
 * @ctx: a #Ecore_IMF_Context
 * @use_prediction: Whether the IM context should use the prediction.
 *
 * This function will be called by Ecore IMF.
 *
 * Set whether the IM context should use the prediction.
 */
EAPI void
isf_imf_context_prediction_allow_set(Ecore_IMF_Context* ctx, Eina_Bool prediction)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " = " << (prediction ? "true" : "false") << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim->impl->prediction_allow != prediction)
     context_scim->impl->prediction_allow = prediction;
}

EAPI void
isf_imf_context_autocapital_type_set(Ecore_IMF_Context* ctx, Ecore_IMF_Autocapital_Type autocapital_type)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " = " << autocapital_type << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF *)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   if (context_scim->impl->autocapital_type != autocapital_type)
     context_scim->impl->autocapital_type = autocapital_type;
}

/**
 * isf_imf_context_filter_event
 * @ctx: a #Ecore_IMF_Context
 * @type: The type of event defined by Ecore_IMF_Event_Type.
 * @event: The event itself.
 * Return value: %TRUE if the input method handled the key event.
 *
 * This function will be called by Ecore IMF.
 *
 * Allow an Ecore Input Context to internally handle an event. If this function
 * returns 1, then no further processing should be done for this event. Input
 * methods must be able to accept all types of events (simply returning 0 if
 * the event was not handled), but there is no obligation of any events to be
 * submitted to this function.
 */
EAPI Eina_Bool
isf_imf_context_filter_event(Ecore_IMF_Context *ctx, Ecore_IMF_Event_Type type, Ecore_IMF_Event *event)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ic, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ic->impl, EINA_FALSE);

   Eina_Bool ret = EINA_FALSE;
   KeyEvent key;

   if (type == ECORE_IMF_EVENT_KEY_DOWN)
     {
        Ecore_IMF_Event_Key_Down *ev = (Ecore_IMF_Event_Key_Down *)event;
        scim_string_to_key(key, ev->key);
        key.mask |= _ecore_imf_modifier_to_scim_mask(ev->modifiers);
        key.mask |= _ecore_imf_lock_to_scim_mask(ev->locks);
     }
   else if (type == ECORE_IMF_EVENT_KEY_UP)
     {
        Ecore_IMF_Event_Key_Up *ev = (Ecore_IMF_Event_Key_Up *)event;
        scim_string_to_key(key, ev->key);
        key.mask = SCIM_KEY_ReleaseMask;
        key.mask |= _ecore_imf_modifier_to_scim_mask(ev->modifiers);
        key.mask |= _ecore_imf_lock_to_scim_mask(ev->locks);
     }
   else
     {
        return ret;
     }

   key.mask &= _valid_key_mask;

   _panel_client.prepare(ic->id);

   ret = EINA_TRUE;
   if (!filter_hotkeys(ic, key))
     {
        if (!_focused_ic || !_focused_ic->impl->is_on ||
            !_focused_ic->impl->si->process_key_event(key))
          ret = EINA_FALSE;
     }

   _panel_client.send();

   return ret;
}

EAPI void
isf_imf_context_input_panel_show(Ecore_IMF_Context *ctx)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   ecore_x_e_virtual_keyboard_state_set
        (context_scim->impl->client_window, ECORE_X_VIRTUAL_KEYBOARD_STATE_ON);
}

EAPI void
isf_imf_context_input_panel_hide(Ecore_IMF_Context *ctx)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *context_scim = (EcoreIMFContextISF*)ecore_imf_context_data_get(ctx);
   EINA_SAFETY_ON_NULL_RETURN(context_scim);
   EINA_SAFETY_ON_NULL_RETURN(context_scim->impl);

   ecore_x_e_virtual_keyboard_state_set
        (context_scim->impl->client_window, ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF);
}

/* Panel Slot functions */
static void
panel_slot_reload_config(int context EINA_UNUSED)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";
   _config->reload();
}

static void
panel_slot_exit(int /* context */)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   finalize();
}

static void
panel_slot_update_lookup_table_page_size(int context, int page_size)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " page_size=" << page_size << " ic=" << ic << "\n";

   _panel_client.prepare(ic->id);
   ic->impl->si->update_lookup_table_page_size(page_size);
   _panel_client.send();
}

static void
panel_slot_lookup_table_page_up(int context)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " ic=" << ic << "\n";

   _panel_client.prepare(ic->id);
   ic->impl->si->lookup_table_page_up();
   _panel_client.send();
}

static void
panel_slot_lookup_table_page_down(int context)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " ic=" << ic << "\n";

   _panel_client.prepare(ic->id);
   ic->impl->si->lookup_table_page_down();
   _panel_client.send();
}

static void
panel_slot_trigger_property(int context, const String &property)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " property=" << property << " ic=" << ic << "\n";

   _panel_client.prepare(ic->id);
   ic->impl->si->trigger_property(property);
   _panel_client.send();
}

static void
panel_slot_process_helper_event(int context, const String &target_uuid, const String &helper_uuid, const Transaction &trans)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " target=" << target_uuid
      << " helper=" << helper_uuid << " ic=" << ic << " ic->impl=" << (ic ? ic->impl : 0) << " ic-uuid="
      << ((ic && ic->impl) ? ic->impl->si->get_factory_uuid() : "" ) << "\n";

   if (ic->impl->si->get_factory_uuid() == target_uuid)
     {
        _panel_client.prepare(ic->id);
        SCIM_DEBUG_FRONTEND(2) << "call process_helper_event\n";
        ic->impl->si->process_helper_event(helper_uuid, trans);
        _panel_client.send();
     }
}

static void
panel_slot_move_preedit_caret(int context, int caret_pos)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " caret=" << caret_pos << " ic=" << ic << "\n";

   _panel_client.prepare(ic->id);
   ic->impl->si->move_preedit_caret(caret_pos);
   _panel_client.send();
}

static void
panel_slot_select_candidate(int context, int cand_index)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " candidate=" << cand_index << " ic=" << ic << "\n";

   _panel_client.prepare(ic->id);
   ic->impl->si->select_candidate(cand_index);
   _panel_client.send();
}

static void
panel_slot_process_key_event(int context, const KeyEvent &key)
{
   EcoreIMFContextISF *ic = find_ic(context);
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " key=" << key.get_key_string() << " ic=" << ic << "\n";

   if (key.is_key_press())
     ecore_x_test_fake_key_press(key.get_key_string().c_str());
}

static void
panel_slot_commit_string(int context, const WideString &wstr)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " str=" << utf8_wcstombs(wstr) << " ic=" << ic << "\n";

   if (_focused_ic != ic)
     return;

   ecore_imf_context_commit_event_add(ic->ctx, utf8_wcstombs(wstr).c_str());
   ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_COMMIT, (void *)utf8_wcstombs(wstr).c_str());
}

static void
panel_slot_forward_key_event(int context, const KeyEvent &key)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " key=" << key.get_key_string() << " ic=" << ic << "\n";

   if (ic->impl->client_canvas)
     feed_key_event(ic->impl->client_canvas, key.get_key_string().c_str(), EINA_TRUE);
}

static void
panel_slot_request_help(int context)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " ic=" << ic << "\n";

   _panel_client.prepare(ic->id);
   panel_req_show_help(ic);
   _panel_client.send();
}

static void
panel_slot_request_factory_menu(int context)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " ic=" << ic << "\n";

   _panel_client.prepare(ic->id);
   panel_req_show_factory_menu(ic);
   _panel_client.send();
}

static void
panel_slot_change_factory(int context, const String &uuid)
{
   EcoreIMFContextISF *ic = find_ic(context);
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " context=" << context << " factory=" << uuid << " ic=" << ic << "\n";

   ic->impl->si->reset();
   _panel_client.prepare(ic->id);
   open_specific_factory(ic, uuid);
   _panel_client.send();
}

/* Panel Requestion functions. */
static void
panel_req_show_help(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   String help;

   help =  String("Smart Common Input Method platform ") +
      //String(SCIM_VERSION) +
      String("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>\n\n");

   IMEngineFactoryPointer sf = _backend->get_factory(ic->impl->si->get_factory_uuid());
   if (sf)
     {
        help += utf8_wcstombs(sf->get_name());
        help += String(":\n\n");

        help += utf8_wcstombs(sf->get_help());
        help += String("\n\n");

        help += utf8_wcstombs(sf->get_credits());
     }
   _panel_client.show_help(ic->id, help);
}

static void
panel_req_show_factory_menu(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   std::vector<IMEngineFactoryPointer> factories;
   std::vector <PanelFactoryInfo> menu;

   _backend->get_factories_for_encoding(factories, "UTF-8");

   for (size_t i = 0; i < factories.size(); ++ i)
     {
        menu.push_back(PanelFactoryInfo(
              factories [i]->get_uuid(),
              utf8_wcstombs(factories [i]->get_name()),
              factories [i]->get_language(),
              factories [i]->get_icon_file()));
     }

   if (menu.size())
     _panel_client.show_factory_menu(ic->id, menu);
}

static void
panel_req_update_factory_info(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   if (ic != _focused_ic)
     return;

   PanelFactoryInfo info;
   if (ic->impl->is_on)
     {
        IMEngineFactoryPointer sf = _backend->get_factory(ic->impl->si->get_factory_uuid());
        if (sf)
          info = PanelFactoryInfo(sf->get_uuid(), utf8_wcstombs(sf->get_name()), sf->get_language(), sf->get_icon_file());
     }
   else
     {
        info = PanelFactoryInfo(String(""), String("English/Keyboard"), String("C"), "");
     }
   _panel_client.update_factory_info(ic->id, info);
}

static void
panel_req_focus_in(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   _panel_client.focus_in(ic->id, ic->impl->si->get_factory_uuid());
}

static void
panel_req_update_spot_location(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   _panel_client.update_spot_location(ic->id, ic->impl->cursor_x, ic->impl->cursor_y);
}

static bool
filter_hotkeys(EcoreIMFContextISF *ic, const KeyEvent &key)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   bool ret = false;

   _frontend_hotkey_matcher.push_key_event(key);
   _imengine_hotkey_matcher.push_key_event(key);

   FrontEndHotkeyAction hotkey_action = _frontend_hotkey_matcher.get_match_result();

   if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER)
     {
        if (!ic->impl->is_on)
          turn_on_ic(ic);
        else
          turn_off_ic(ic);
        ret = true;
     }
   else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON)
     {
        if (!ic->impl->is_on)
          turn_on_ic(ic);
        ret = true;
     }
   else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF)
     {
        if (ic->impl->is_on)
          turn_off_ic(ic);
        ret = true;
     }
   else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY)
     {
        open_next_factory(ic);
        ret = true;
     }
   else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY)
     {
        open_previous_factory(ic);
        ret = true;
     }
   else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU)
     {
        panel_req_show_factory_menu(ic);
        ret = true;
     }
   else if (_imengine_hotkey_matcher.is_matched())
     {
        String sfid = _imengine_hotkey_matcher.get_match_result();
        open_specific_factory(ic, sfid);
        ret = true;
     }
   return ret;
}

static bool
panel_initialize(void)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   String display_name;
     {
        const char *p = getenv("DISPLAY");
        if (p) display_name = String(p);
     }

   if (_panel_client.open_connection(_config->get_name(), display_name) >= 0)
     {
        int fd = _panel_client.get_connection_number();

        _panel_iochannel_read_handler = ecore_main_fd_handler_add(fd, ECORE_FD_READ, panel_iochannel_handler, NULL, NULL, NULL);

        SCIM_DEBUG_FRONTEND(2) << " Panel FD= " << fd << "\n";

        return true;
     }
   std::cerr << "panel_initialize() failed!!!\n";
   return false;
}

static void
panel_finalize(void)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   _panel_client.close_connection();

   if (_panel_iochannel_read_handler)
     {
        ecore_main_fd_handler_del(_panel_iochannel_read_handler);
        _panel_iochannel_read_handler = 0;
     }

   if (_panel_iochannel_err_handler)
     {
        ecore_main_fd_handler_del(_panel_iochannel_err_handler);
        _panel_iochannel_err_handler = 0;
     }
}

static Eina_Bool
panel_iochannel_handler(void *data EINA_UNUSED, Ecore_Fd_Handler *fd_handler)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   if (fd_handler == _panel_iochannel_read_handler)
     {
        if (!_panel_client.filter_event())
          {
             panel_finalize();
             panel_initialize();
             return ECORE_CALLBACK_CANCEL;
          }
     }
   else if (fd_handler == _panel_iochannel_err_handler)
     {
        panel_finalize();
        panel_initialize();
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
turn_on_ic(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   if (!ic->impl->is_on)
     {
        ic->impl->is_on = true;

        if (ic == _focused_ic)
          {
             panel_req_focus_in(ic);
             panel_req_update_spot_location(ic);
             panel_req_update_factory_info(ic);
             _panel_client.turn_on(ic->id);
             _panel_client.hide_preedit_string(ic->id);
             _panel_client.hide_aux_string(ic->id);
             _panel_client.hide_lookup_table(ic->id);
             ic->impl->si->focus_in();
          }

        //Record the IC on/off status
        if (_shared_input_method)
          _config->write(String(SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), true);

        if (ic->impl->use_preedit && ic->impl->preedit_string.length())
          {
             ecore_imf_context_preedit_start_event_add(ic->ctx);
             ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_START, NULL);
             ecore_imf_context_preedit_changed_event_add(ic->ctx);
             ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
             ic->impl->preedit_started = true;
          }
     }
}

static void
turn_off_ic(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   if (ic->impl->is_on)
     {
        ic->impl->is_on = false;

        if (ic == _focused_ic)
          {
             ic->impl->si->focus_out();

             panel_req_update_factory_info(ic);
             _panel_client.turn_off(ic->id);
          }

        //Record the IC on/off status
        if (_shared_input_method)
          _config->write(String(SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), false);

        if (ic->impl->use_preedit && ic->impl->preedit_string.length())
          {
             ecore_imf_context_preedit_changed_event_add(ic->ctx);
             ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
             ecore_imf_context_preedit_end_event_add(ic->ctx);
             ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_END, NULL);
             ic->impl->preedit_started = false;
          }
     }
}

static void
set_ic_capabilities(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   unsigned int cap = SCIM_CLIENT_CAP_ALL_CAPABILITIES;

   if (!_on_the_spot || !ic->impl->use_preedit)
     cap -= SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT;

   ic->impl->si->update_client_capabilities(cap);
}

static bool
check_socket_frontend(void)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   SocketAddress address;
   SocketClient client;

   uint32 magic;

   address.set_address(scim_get_default_socket_frontend_address());

   if (!client.connect(address))
     return false;

   if (!scim_socket_open_connection(magic,
                                    String("ConnectionTester"),
                                    String("SocketFrontEnd"),
                                    client,
                                    1000))
        return false;

   return true;
}

void
initialize(void)
{
   std::vector<String>     config_list;
   std::vector<String>     engine_list;
   std::vector<String>     load_engine_list;

   std::vector<String>::iterator it;

   bool                    manual = false;

   bool                    socket = true;

   String                  config_module_name = "simple";

   SCIM_DEBUG_FRONTEND(1) << "Initializing Ecore SCIM IMModule...\n";

   // Get system language.
   _language = scim_get_locale_language(scim_get_current_locale());

   if (socket)
     {
        // If no Socket FrontEnd is running, then launch one.
        // And set manual to false.
        bool check_result = check_socket_frontend();
        if (!check_result)
          {
             std::cerr << "Launching a SCIM daemon with Socket FrontEnd...\n";
             //get modules list
             scim_get_imengine_module_list(engine_list);

             for (it = engine_list.begin(); it != engine_list.end(); it++)
               {
                  if (*it != "socket")
                    load_engine_list.push_back(*it);
               }

             const char *new_argv [] = { "--no-stay", 0 };
             scim_launch(true,
                         config_module_name,
                         (load_engine_list.size() ? scim_combine_string_list(load_engine_list, ',') : "none"),
                         "socket",
                         (char **)new_argv);
             manual = false;
          }

        // If there is one Socket FrontEnd running and it's not manual mode,
        // then just use this Socket Frontend.
        if (!manual)
          {
             for (int i = 0; i < 200; ++i)
               {
                  if (check_result)
                    {
                       config_module_name = "socket";
                       load_engine_list.clear();
                       load_engine_list.push_back("socket");
                       break;
                    }
                  scim_usleep(50000);
                  check_result = check_socket_frontend();
               }
          }
     }

   if (config_module_name != "dummy")
     {
        //load config module
        SCIM_DEBUG_FRONTEND(1) << "Loading Config module: " << config_module_name << "...\n";
        _config_module = new ConfigModule(config_module_name);

        //create config instance
        if (_config_module != NULL && _config_module->valid())
          _config = _config_module->create_config();
     }

   if (_config.null())
     {
        SCIM_DEBUG_FRONTEND(1) << "Config module cannot be loaded, using dummy Config.\n";

        if (_config_module) delete _config_module;
        _config_module = NULL;

        _config = new DummyConfig();
        config_module_name = "dummy";
     }

   reload_config_callback(_config);
   _config->signal_connect_reload(slot(reload_config_callback));

   // create backend
   _backend = new CommonBackEnd(_config, load_engine_list.size() ? load_engine_list : engine_list);

   if (_backend.null())
     std::cerr << "Cannot create BackEnd Object!\n";
   else
     _fallback_factory = _backend->get_factory(SCIM_COMPOSE_KEY_FACTORY_UUID);

   if (_fallback_factory.null())
     _fallback_factory = new DummyIMEngineFactory();

   _fallback_instance = _fallback_factory->create_instance(String("UTF-8"), 0);
   _fallback_instance->signal_connect_commit_string(slot(fallback_commit_string_cb));

   // Attach Panel Client signal.
   _panel_client.signal_connect_reload_config                (slot(panel_slot_reload_config));
   _panel_client.signal_connect_exit                         (slot(panel_slot_exit));
   _panel_client.signal_connect_update_lookup_table_page_size(slot(panel_slot_update_lookup_table_page_size));
   _panel_client.signal_connect_lookup_table_page_up         (slot(panel_slot_lookup_table_page_up));
   _panel_client.signal_connect_lookup_table_page_down       (slot(panel_slot_lookup_table_page_down));
   _panel_client.signal_connect_trigger_property             (slot(panel_slot_trigger_property));
   _panel_client.signal_connect_process_helper_event         (slot(panel_slot_process_helper_event));
   _panel_client.signal_connect_move_preedit_caret           (slot(panel_slot_move_preedit_caret));
   _panel_client.signal_connect_select_candidate             (slot(panel_slot_select_candidate));
   _panel_client.signal_connect_process_key_event            (slot(panel_slot_process_key_event));
   _panel_client.signal_connect_commit_string                (slot(panel_slot_commit_string));
   _panel_client.signal_connect_forward_key_event            (slot(panel_slot_forward_key_event));
   _panel_client.signal_connect_request_help                 (slot(panel_slot_request_help));
   _panel_client.signal_connect_request_factory_menu         (slot(panel_slot_request_factory_menu));
   _panel_client.signal_connect_change_factory               (slot(panel_slot_change_factory));

   if (!panel_initialize())
     std::cerr << "Ecore IM Module: Cannot connect to Panel!\n";
}

static void
finalize(void)
{
   SCIM_DEBUG_FRONTEND(1) << "Finalizing Ecore ISF IMModule...\n";

   // Reset this first so that the shared instance could be released correctly afterwards.
   _default_instance.reset();

   SCIM_DEBUG_FRONTEND(2) << "Finalize all IC partially.\n";
   while (_used_ic_impl_list)
     {
        // In case in "shared input method" mode,
        // all contexts share only one instance,
        // so we need point the reference pointer correctly before finalizing.
        _used_ic_impl_list->si->set_frontend_data(static_cast <void*>(_used_ic_impl_list->parent));
        isf_imf_context_del(_used_ic_impl_list->parent->ctx);
     }

   delete_all_ic_impl();

   _fallback_instance.reset();
   _fallback_factory.reset();

   SCIM_DEBUG_FRONTEND(2) << " Releasing BackEnd...\n";
   _backend.reset();

   SCIM_DEBUG_FRONTEND(2) << " Releasing Config...\n";
   _config.reset();

   if (_config_module)
     {
        SCIM_DEBUG_FRONTEND(2) << " Deleting _config_module...\n";
        delete _config_module;
        _config_module = 0;
     }

   _focused_ic = NULL;
   _ic_list = NULL;

   _scim_initialized = false;

   panel_finalize();
}

static void
open_next_factory(EcoreIMFContextISF *ic)
{
   SCIM_DEBUG_FRONTEND(2) << __FUNCTION__ << " context=" << ic->id << "\n";
   IMEngineFactoryPointer sf = _backend->get_next_factory("", "UTF-8", ic->impl->si->get_factory_uuid());

   if (!sf.null())
     {
        turn_off_ic(ic);
        ic->impl->si = sf->create_instance("UTF-8", ic->impl->si->get_id());
        ic->impl->si->set_frontend_data(static_cast <void*>(ic));
        ic->impl->preedit_string = WideString();
        ic->impl->preedit_caret = 0;
        attach_instance(ic->impl->si);
        _backend->set_default_factory(_language, sf->get_uuid());
        _panel_client.register_input_context(ic->id, sf->get_uuid());
        set_ic_capabilities(ic);
        turn_on_ic(ic);

        if (_shared_input_method)
          {
             _default_instance = ic->impl->si;
             ic->impl->shared_si = true;
          }
     }
}

static void
open_previous_factory(EcoreIMFContextISF *ic)
{
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(2) << __FUNCTION__ << " context=" << ic->id << "\n";
   IMEngineFactoryPointer sf = _backend->get_previous_factory("", "UTF-8", ic->impl->si->get_factory_uuid());

   if (!sf.null())
     {
        turn_off_ic(ic);
        ic->impl->si = sf->create_instance("UTF-8", ic->impl->si->get_id());
        ic->impl->si->set_frontend_data(static_cast <void*>(ic));
        ic->impl->preedit_string = WideString();
        ic->impl->preedit_caret = 0;
        attach_instance(ic->impl->si);
        _backend->set_default_factory(_language, sf->get_uuid());
        _panel_client.register_input_context(ic->id, sf->get_uuid());
        set_ic_capabilities(ic);
        turn_on_ic(ic);

        if (_shared_input_method)
          {
             _default_instance = ic->impl->si;
             ic->impl->shared_si = true;
          }
     }
}

static void
open_specific_factory(EcoreIMFContextISF *ic,
                       const String     &uuid)
{
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(2) << __FUNCTION__ << " context=" << ic->id << "\n";

   // The same input method is selected, just turn on the IC.
   if (ic->impl->si->get_factory_uuid() == uuid)
     {
        turn_on_ic(ic);
        return;
     }

   IMEngineFactoryPointer sf = _backend->get_factory(uuid);

   if (uuid.length() && !sf.null())
     {
        turn_off_ic(ic);
        ic->impl->si = sf->create_instance("UTF-8", ic->impl->si->get_id());
        ic->impl->si->set_frontend_data(static_cast <void*>(ic));
        ic->impl->preedit_string = WideString();
        ic->impl->preedit_caret = 0;
        attach_instance(ic->impl->si);
        _backend->set_default_factory(_language, sf->get_uuid());
        _panel_client.register_input_context(ic->id, sf->get_uuid());
        set_ic_capabilities(ic);
        turn_on_ic(ic);

        if (_shared_input_method)
          {
             _default_instance = ic->impl->si;
             ic->impl->shared_si = true;
          }
     }
   else
     {
        // turn_off_ic comment out panel_req_update_factory_info()
        turn_off_ic(ic);
        if (ic->impl->is_on)
          {
             ic->impl->is_on = false;

             if (ic == _focused_ic)
               {
                  ic->impl->si->focus_out();

                  panel_req_update_factory_info(ic);
                  _panel_client.turn_off(ic->id);
               }

             //Record the IC on/off status
             if (_shared_input_method)
               _config->write(String(SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), false);

             if (ic->impl->use_preedit && ic->impl->preedit_string.length())
               {
                  ecore_imf_context_preedit_changed_event_add(ic->ctx);
                  ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
                  ecore_imf_context_preedit_end_event_add(ic->ctx);
                  ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_END, NULL);
                  ic->impl->preedit_started = false;
               }
          }
     }
}

static void initialize_modifier_bits(Display *display)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   if (__current_display == display)
     return;

   __current_display = display;

   if (display == 0)
     {
        __current_alt_mask     = Mod1Mask;
        __current_meta_mask    = ShiftMask | Mod1Mask;
        __current_super_mask   = 0;
        __current_hyper_mask   = 0;
        __current_numlock_mask = Mod2Mask;
        return;
     }

   XModifierKeymap *mods = NULL;

   ::KeyCode ctrl_l  = XKeysymToKeycode(display, XK_Control_L);
   ::KeyCode ctrl_r  = XKeysymToKeycode(display, XK_Control_R);
   ::KeyCode meta_l  = XKeysymToKeycode(display, XK_Meta_L);
   ::KeyCode meta_r  = XKeysymToKeycode(display, XK_Meta_R);
   ::KeyCode alt_l   = XKeysymToKeycode(display, XK_Alt_L);
   ::KeyCode alt_r   = XKeysymToKeycode(display, XK_Alt_R);
   ::KeyCode super_l = XKeysymToKeycode(display, XK_Super_L);
   ::KeyCode super_r = XKeysymToKeycode(display, XK_Super_R);
   ::KeyCode hyper_l = XKeysymToKeycode(display, XK_Hyper_L);
   ::KeyCode hyper_r = XKeysymToKeycode(display, XK_Hyper_R);
   ::KeyCode numlock = XKeysymToKeycode(display, XK_Num_Lock);

   int i, j;

   mods = XGetModifierMapping(display);
   if (mods == NULL)
     return;

   __current_alt_mask     = 0;
   __current_meta_mask    = 0;
   __current_super_mask   = 0;
   __current_hyper_mask   = 0;
   __current_numlock_mask = 0;

   /* We skip the first three sets for Shift, Lock, and Control.  The
      remaining sets are for Mod1, Mod2, Mod3, Mod4, and Mod5.  */
   for (i = 3; i < 8; i++)
     {
        for (j = 0; j < mods->max_keypermod; j++)
          {
             ::KeyCode code = mods->modifiermap [i * mods->max_keypermod + j];
             if (! code) continue;
             if (code == alt_l || code == alt_r)
               __current_alt_mask |= (1 << i);
             else if (code == meta_l || code == meta_r)
               __current_meta_mask |= (1 << i);
             else if (code == super_l || code == super_r)
               __current_super_mask |= (1 << i);
             else if (code == hyper_l || code == hyper_r)
               __current_hyper_mask |= (1 << i);
             else if (code == numlock)
               __current_numlock_mask |= (1 << i);
          }
     }

   /* Check whether there is a combine keys mapped to Meta */
   if (__current_meta_mask == 0)
     {
        char buf [32];
        XKeyEvent xkey;
        KeySym keysym_l, keysym_r;

        xkey.type = KeyPress;
        xkey.display = display;
        xkey.serial = 0L;
        xkey.send_event = False;
        xkey.x = xkey.y = xkey.x_root = xkey.y_root = 0;
        xkey.time = 0;
        xkey.same_screen = False;
        xkey.subwindow = None;
        xkey.window = None;
        xkey.root = DefaultRootWindow(display);
        xkey.state = ShiftMask;

        xkey.keycode = meta_l;
        XLookupString(&xkey, buf, 32, &keysym_l, 0);
        xkey.keycode = meta_r;
        XLookupString(&xkey, buf, 32, &keysym_r, 0);

        if ((meta_l == alt_l && keysym_l == XK_Meta_L) || (meta_r == alt_r && keysym_r == XK_Meta_R))
          __current_meta_mask = ShiftMask + __current_alt_mask;
        else if ((meta_l == ctrl_l && keysym_l == XK_Meta_L) || (meta_r == ctrl_r && keysym_r == XK_Meta_R))
          __current_meta_mask = ShiftMask + ControlMask;
     }

   XFreeModifiermap(mods);
}

static unsigned int scim_x11_keymask_scim_to_x11(Display *display, uint16 scimkeymask)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   unsigned int state = 0;

   initialize_modifier_bits(display);

   if (scimkeymask & SCIM_KEY_ShiftMask)    state |= ShiftMask;
   if (scimkeymask & SCIM_KEY_CapsLockMask) state |= LockMask;
   if (scimkeymask & SCIM_KEY_ControlMask)  state |= ControlMask;
   if (scimkeymask & SCIM_KEY_AltMask)      state |= __current_alt_mask;
   if (scimkeymask & SCIM_KEY_MetaMask)     state |= __current_meta_mask;
   if (scimkeymask & SCIM_KEY_SuperMask)    state |= __current_super_mask;
   if (scimkeymask & SCIM_KEY_HyperMask)    state |= __current_hyper_mask;
   if (scimkeymask & SCIM_KEY_NumLockMask)  state |= __current_numlock_mask;

   return state;
}

static XKeyEvent createKeyEvent(Display *display, Window &win,
                                Window &winRoot, bool press,
                                int keysym, int modifiers)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   XKeyEvent event;

   event.display     = display;
   event.window      = win;
   event.root        = winRoot;
   event.subwindow   = None;
   event.time        = CurrentTime;
   event.x           = 1;
   event.y           = 1;
   event.x_root      = 1;
   event.y_root      = 1;
   event.same_screen = EINA_TRUE;
   event.state       = modifiers;
   event.keycode     = XKeysymToKeycode(display, keysym);
   if (press)
     event.type = KeyPress;
   else
     event.type = KeyRelease;
   event.send_event  = EINA_FALSE;
   event.serial = 0;

   return event;
}

static void _x_send_key_event(const KeyEvent &key)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   // Obtain the X11 display.
   Display *display = (Display *)ecore_x_display_get();
   EINA_SAFETY_ON_NULL_RETURN(display);

   // Get the root window for the current display.
   Window winRoot = 0;

   // Find the window which has the current keyboard focus.
   Window winFocus = 0;
   int revert = RevertToParent;

   XGetInputFocus(display, &winFocus, &revert);

   unsigned int modifier = scim_x11_keymask_scim_to_x11(display, key.mask);
   XKeyEvent event;
   if (key.is_key_press())
     {
        event = createKeyEvent(display, winFocus, winRoot, true, key.code, modifier);
        XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
     }
   else
     {
        event = createKeyEvent(display, winFocus, winRoot, false, key.code, modifier);
        XSendEvent(event.display, event.window, True, KeyReleaseMask, (XEvent *)&event);
     }
}

static void
attach_instance(const IMEngineInstancePointer &si)
{
   si->signal_connect_show_preedit_string(
      slot(slot_show_preedit_string));
   si->signal_connect_show_aux_string(
      slot(slot_show_aux_string));
   si->signal_connect_show_lookup_table(
      slot(slot_show_lookup_table));

   si->signal_connect_hide_preedit_string(
      slot(slot_hide_preedit_string));
   si->signal_connect_hide_aux_string(
      slot(slot_hide_aux_string));
   si->signal_connect_hide_lookup_table(
      slot(slot_hide_lookup_table));

   si->signal_connect_update_preedit_caret(
      slot(slot_update_preedit_caret));
   si->signal_connect_update_preedit_string(
      slot(slot_update_preedit_string));
   si->signal_connect_update_aux_string(
      slot(slot_update_aux_string));
   si->signal_connect_update_lookup_table(
      slot(slot_update_lookup_table));

   si->signal_connect_commit_string(
      slot(slot_commit_string));

   si->signal_connect_forward_key_event(
      slot(slot_forward_key_event));

   si->signal_connect_register_properties(
      slot(slot_register_properties));

   si->signal_connect_update_property(
      slot(slot_update_property));

   si->signal_connect_beep(
      slot(slot_beep));

   si->signal_connect_start_helper(
      slot(slot_start_helper));

   si->signal_connect_stop_helper(
      slot(slot_stop_helper));

   si->signal_connect_send_helper_event(
      slot(slot_send_helper_event));

   si->signal_connect_get_surrounding_text(
      slot(slot_get_surrounding_text));

   si->signal_connect_delete_surrounding_text(
      slot(slot_delete_surrounding_text));
}

// Implementation of slot functions
static void
slot_show_preedit_string(IMEngineInstanceBase *si)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   if (_focused_ic != ic)
     return;

   if (ic->impl->use_preedit)
     {
        if (!ic->impl->preedit_started)
          {
             ecore_imf_context_preedit_start_event_add(_focused_ic->ctx);
             ecore_imf_context_event_callback_call(_focused_ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_START, NULL);
             ic->impl->preedit_started = true;
          }
     }
   else
     _panel_client.show_preedit_string(ic->id);
}

static void
slot_show_aux_string(IMEngineInstanceBase *si)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     _panel_client.show_aux_string(ic->id);
}

static void
slot_show_lookup_table(IMEngineInstanceBase *si)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     _panel_client.show_lookup_table(ic->id);
}

static void
slot_hide_preedit_string(IMEngineInstanceBase *si)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   if (_focused_ic != ic)
     return;

   bool emit = false;
   if (ic->impl->preedit_string.length())
     {
        ic->impl->preedit_string = WideString();
        ic->impl->preedit_caret = 0;
        ic->impl->preedit_attrlist.clear();
        emit = true;
     }
   if (ic->impl->use_preedit)
     {
        if (emit)
          {
             ecore_imf_context_preedit_changed_event_add(ic->ctx);
             ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
          }
        if (ic->impl->preedit_started)
          {
             ecore_imf_context_preedit_end_event_add(ic->ctx);
             ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_END, NULL);
             ic->impl->preedit_started = false;
          }
     }
   else
     _panel_client.hide_preedit_string(ic->id);
}

static void
slot_hide_aux_string(IMEngineInstanceBase *si)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     _panel_client.hide_aux_string(ic->id);
}

static void
slot_hide_lookup_table(IMEngineInstanceBase *si)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     _panel_client.hide_lookup_table(ic->id);
}

static void
slot_update_preedit_caret(IMEngineInstanceBase *si, int caret)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   if (_focused_ic == ic && ic->impl->preedit_caret != caret)
     {
        ic->impl->preedit_caret = caret;
        if (ic->impl->use_preedit)
          {
             if (!ic->impl->preedit_started)
               {
                  ecore_imf_context_preedit_start_event_add(ic->ctx);
                  ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_START, NULL);
                  ic->impl->preedit_started = true;
               }
             ecore_imf_context_preedit_changed_event_add(ic->ctx);
             ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
          }
        else
          _panel_client.update_preedit_caret(ic->id, caret);
     }
}

static void
slot_update_preedit_string(IMEngineInstanceBase *si,
                            const WideString & str,
                            const AttributeList & attrs)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   if (_focused_ic == ic && (ic->impl->preedit_string != str || str.length()))
     {
        ic->impl->preedit_string   = str;
        ic->impl->preedit_attrlist = attrs;
        if (ic->impl->use_preedit)
          {
             if (!ic->impl->preedit_started)
               {
                  ecore_imf_context_preedit_start_event_add(_focused_ic->ctx);
                  ecore_imf_context_event_callback_call(_focused_ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_START, NULL);
                  ic->impl->preedit_started = true;
               }
             ic->impl->preedit_caret    = str.length();
             ic->impl->preedit_updating = true;
             ecore_imf_context_preedit_changed_event_add(ic->ctx);
             ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, NULL);
             ic->impl->preedit_updating = false;
          }
        else
          {
             _panel_client.update_preedit_string(ic->id, str, attrs);
          }
     }
}

static void
slot_update_aux_string(IMEngineInstanceBase *si,
                        const WideString & str,
                        const AttributeList & attrs)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     _panel_client.update_aux_string(ic->id, str, attrs);
}

static void
slot_commit_string(IMEngineInstanceBase *si,
                    const WideString & str)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->ctx);

   ecore_imf_context_commit_event_add(ic->ctx, utf8_wcstombs(str).c_str());
   ecore_imf_context_event_callback_call(ic->ctx, ECORE_IMF_CALLBACK_COMMIT, (void *)utf8_wcstombs(str).c_str());
}

static void
slot_forward_key_event(IMEngineInstanceBase *si,
                        const KeyEvent & key)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     {
        if (!_fallback_instance->process_key_event(key))
          _x_send_key_event(key);
     }
}

static void
slot_update_lookup_table(IMEngineInstanceBase *si,
                          const LookupTable & table)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     _panel_client.update_lookup_table(ic->id, table);
}

static void
slot_register_properties(IMEngineInstanceBase *si,
                         const PropertyList & properties)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     _panel_client.register_properties(ic->id, properties);
}

static void
slot_update_property(IMEngineInstanceBase *si,
                     const Property & property)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     _panel_client.update_property(ic->id, property);
}

static void
slot_beep(IMEngineInstanceBase *si)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   if (_focused_ic == ic)
     ecore_x_bell(0);
}

static void
slot_start_helper(IMEngineInstanceBase *si,
                  const String &helper_uuid)
{
   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " helper= " << helper_uuid << " context="
      << (ic ? ic->id : -1) << " ic=" << ic
      << " ic-uuid=" << ((ic ) ? ic->impl->si->get_factory_uuid() : "") << "...\n";

   _panel_client.start_helper(ic->id, helper_uuid);
}

static void
slot_stop_helper(IMEngineInstanceBase *si,
                 const String &helper_uuid)
{
   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " helper= " << helper_uuid << " context=" << (ic ? ic->id : -1) << " ic=" << ic << "...\n";

   _panel_client.stop_helper(ic->id, helper_uuid);
}

static void
slot_send_helper_event(IMEngineInstanceBase *si,
                       const String      &helper_uuid,
                       const Transaction &trans)
{
   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN(ic);
   EINA_SAFETY_ON_NULL_RETURN(ic->impl);

   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << " helper= " << helper_uuid << " context="
      << (ic ? ic->id : -1) << " ic=" << ic
      << " ic-uuid=" << ((ic) ? ic->impl->si->get_factory_uuid() : "") << "...\n";

   _panel_client.send_helper_event(ic->id, helper_uuid, trans);
}

static bool
slot_get_surrounding_text(IMEngineInstanceBase *si,
                          WideString            &text,
                          int                   &cursor,
                          int                    maxlen_before,
                          int                    maxlen_after)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN_VAL(ic, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ic->impl, false);

   if (_focused_ic != ic)
     return false;

   char *surrounding = NULL;
   int   cursor_index;
   if (ecore_imf_context_surrounding_get(_focused_ic->ctx, &surrounding, &cursor_index))
     {
        SCIM_DEBUG_FRONTEND(2) << "Surrounding text: " << surrounding <<"\n";
        SCIM_DEBUG_FRONTEND(2) << "Cursor Index    : " << cursor_index <<"\n";
        if (!surrounding)
          return false;

        if (cursor_index < 0)
          {
             free(surrounding);
             surrounding = NULL;
             return false;
          }

        WideString before(utf8_mbstowcs(String(surrounding, surrounding + cursor_index)));
        WideString after(utf8_mbstowcs(String(surrounding + cursor_index)));

        free(surrounding);
        surrounding = NULL;

        if (maxlen_before > 0 && ((unsigned int)maxlen_before) < before.length())
          before = WideString(before.begin() + (before.length() - maxlen_before), before.end());
        else if (maxlen_before == 0) before = WideString();
        if (maxlen_after > 0 && ((unsigned int)maxlen_after) < after.length())
          after = WideString(after.begin(), after.begin() + maxlen_after);
        else if (maxlen_after == 0) after = WideString();
        text = before + after;
        cursor = before.length();
        return true;
     }

   return false;
}

static bool
slot_delete_surrounding_text(IMEngineInstanceBase *si,
                             int                   offset,
                             int                   len)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   EcoreIMFContextISF *ic = static_cast<EcoreIMFContextISF *>(si->get_frontend_data());
   EINA_SAFETY_ON_NULL_RETURN_VAL(ic, false);

   if (_focused_ic != ic)
     return false;

   Ecore_IMF_Event_Delete_Surrounding ev;
   ev.ctx = _focused_ic->ctx;
   ev.n_chars = len;
   ev.offset = offset;
   ecore_imf_context_delete_surrounding_event_add(_focused_ic->ctx, offset, len);
   ecore_imf_context_event_callback_call(_focused_ic->ctx, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, &ev);

   return true;
}

static void
reload_config_callback(const ConfigPointer &config)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   _frontend_hotkey_matcher.load_hotkeys(config);
   _imengine_hotkey_matcher.load_hotkeys(config);

   KeyEvent key;

   scim_string_to_key(key,
                      config->read(String(SCIM_CONFIG_HOTKEYS_FRONTEND_VALID_KEY_MASK),
                                     String("Shift+Control+Alt+Lock")));

   _valid_key_mask = (key.mask > 0)?(key.mask):0xFFFF;
   _valid_key_mask |= SCIM_KEY_ReleaseMask;
   // Special treatment for two backslash keys on jp106 keyboard.
   _valid_key_mask |= SCIM_KEY_QuirkKanaRoMask;

   _on_the_spot = config->read(String(SCIM_CONFIG_FRONTEND_ON_THE_SPOT), _on_the_spot);
   _shared_input_method = config->read(String(SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD), _shared_input_method);

   // Get keyboard layout setting
   // Flush the global config first, in order to load the new configs from disk.
   scim_global_config_flush();

   _keyboard_layout = scim_get_default_keyboard_layout();
}

static void
fallback_commit_string_cb(IMEngineInstanceBase  *si EINA_UNUSED,
                          const WideString      &str)
{
   SCIM_DEBUG_FRONTEND(1) << __FUNCTION__ << "...\n";

   if (_focused_ic)
     {
        ecore_imf_context_commit_event_add(_focused_ic->ctx, utf8_wcstombs(str).c_str());
        ecore_imf_context_event_callback_call(_focused_ic->ctx, ECORE_IMF_CALLBACK_COMMIT, (void *)utf8_wcstombs(str).c_str());
     }
}

