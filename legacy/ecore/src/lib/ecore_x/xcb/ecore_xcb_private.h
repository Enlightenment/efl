#ifndef __ECORE_XCB_PRIVATE_H__
# define __ECORE_XCB_PRIVATE_H__

//# define LOGFNS 1

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include <unistd.h> // included for close & gethostname functions

/* generic xcb includes */
# include <xcb/xcb.h>
# include <xcb/bigreq.h>
# include <xcb/shm.h>
# include <xcb/xcb_image.h>

/* EFL includes */
# include "Ecore.h"
# include "Ecore_Input.h"
# include "Ecore_X.h"

/* logging */
extern int _ecore_xcb_log_dom;

# ifdef ECORE_XCB_DEFAULT_LOG_COLOR
#  undef ECORE_XCB_DEFAULT_LOG_COLOR
# endif
# define ECORE_XCB_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_xcb_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_xcb_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_xcb_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_xcb_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_xcb_log_dom, __VA_ARGS__)

# ifdef LOGFNS
#  include <stdio.h>
#  define LOGFN(fl, ln, fn) printf("-ECORE-XCB: %25s: %5i - %s\n", fl, ln, fn);
# else
#  define LOGFN(fl, ln, fn)
# endif

# ifndef MAXHOSTNAMELEN
#  define MAXHOSTNAMELEN 256
# endif

# ifndef MIN
#  define MIN(x, y) (((x) > (y)) ? (y) : (x))
# endif

# ifndef MAX
#  define MAX(a, b) ((a < b) ? b : a)
# endif

/* enums */
typedef enum _Ecore_Xcb_Encoding_Style Ecore_Xcb_Encoding_Style;

enum _Ecore_Xcb_Encoding_Style 
{
   XcbStringStyle, 
   XcbCompoundTextStyle, 
   XcbTextStyle,
   XcbStdICCTextStyle, 
   XcbUTF8StringStyle
};

/* structures */
typedef struct _Ecore_X_DND_Source Ecore_X_DND_Source;
typedef struct _Ecore_X_DND_Target Ecore_X_DND_Target;
typedef struct _Ecore_X_Selection_Intern Ecore_X_Selection_Intern;
typedef struct _Ecore_X_Selection_Converter Ecore_X_Selection_Converter;
typedef struct _Ecore_X_Selection_Parser Ecore_X_Selection_Parser;
typedef struct _Ecore_Xcb_Textproperty Ecore_Xcb_Textproperty;

struct _Ecore_X_DND_Source 
{
   int version;
   Ecore_X_Window win, dest;

   enum 
     {
        ECORE_X_DND_SOURCE_IDLE, 
        ECORE_X_DND_SOURCE_DRAGGING, 
        ECORE_X_DND_SOURCE_DROPPED, 
        ECORE_X_DND_SOURCE_CONVERTING
     } state;

   struct 
     {
        short x, y;
        unsigned short width, height;
     } rectangle;

   struct 
     {
        Ecore_X_Window window;
        int x, y;
     } prev;

   Ecore_X_Time time;

   Ecore_X_Atom action, accepted_action;

   int will_accept, suppress;
   int await_status;
};

struct _Ecore_X_DND_Target 
{
   int version;
   Ecore_X_Window win, source;

   enum 
     {
        ECORE_X_DND_TARGET_IDLE, 
        ECORE_X_DND_TARGET_ENTERED
     } state;

   struct 
     {
        int x, y;
     } pos;

   Ecore_X_Time time;

   Ecore_X_Atom action, accepted_action;
   int will_accept;
};

struct _Ecore_X_Selection_Intern 
{
   Ecore_X_Window win;
   Ecore_X_Atom selection;
   unsigned char *data;
   int length;
   Ecore_X_Time time;
};

struct _Ecore_X_Selection_Converter 
{
   Ecore_X_Atom target;
   Eina_Bool (*convert) (char *target, void *data, int size, void **data_ret, 
                         int *size_ret, Ecore_X_Atom *type, int *size_type);
   Ecore_X_Selection_Converter *next;
};

struct _Ecore_X_Selection_Parser 
{
   char *target;
   void *(*parse) (const char *target, void *data, int size, int format);
   Ecore_X_Selection_Parser *next;
};

struct _Ecore_Xcb_Textproperty 
{
   char *value;
   Ecore_X_Atom encoding;
   unsigned int format, nitems;
};

/* external variables */
extern Ecore_X_Connection *_ecore_xcb_conn;
extern Ecore_X_Screen *_ecore_xcb_screen;
extern double _ecore_xcb_double_click_time;
extern int16_t _ecore_xcb_event_last_root_x;
extern int16_t _ecore_xcb_event_last_root_y;

/* external variables for extension events */
extern int _ecore_xcb_event_damage;
extern int _ecore_xcb_event_randr;
extern int _ecore_xcb_event_screensaver;
extern int _ecore_xcb_event_shape;
extern int _ecore_xcb_event_sync;
extern int _ecore_xcb_event_xfixes;
extern int _ecore_xcb_event_input;

extern int ECORE_X_MODIFIER_SHIFT;
extern int ECORE_X_MODIFIER_CTRL;
extern int ECORE_X_MODIFIER_ALT;
extern int ECORE_X_MODIFIER_WIN;
extern int ECORE_X_LOCK_SCROLL;
extern int ECORE_X_LOCK_NUM;
extern int ECORE_X_LOCK_CAPS;
extern int ECORE_X_LOCK_SHIFT;

extern Ecore_X_Atom _ecore_xcb_atoms_wm_protocol[ECORE_X_WM_PROTOCOL_NUM];

extern int _ecore_xcb_button_grabs_num;
extern int _ecore_xcb_key_grabs_num;
extern Ecore_X_Window *_ecore_xcb_button_grabs;
extern Ecore_X_Window *_ecore_xcb_key_grabs;
extern Eina_Bool (*_ecore_xcb_window_grab_replay_func)(void *data, int type, void *event);
extern void *_ecore_xcb_window_grab_replay_data;

/* private function prototypes */
void _ecore_xcb_error_handler_init(void);
void _ecore_xcb_error_handler_shutdown(void);

void _ecore_xcb_atoms_init(void);
void _ecore_xcb_atoms_finalize(void);

void _ecore_xcb_extensions_init(void);
void _ecore_xcb_extensions_finalize(void);

void _ecore_xcb_shape_init(void);
void _ecore_xcb_shape_finalize(void);

void _ecore_xcb_screensaver_init(void);
void _ecore_xcb_screensaver_finalize(void);

void _ecore_xcb_sync_init(void);
void _ecore_xcb_sync_finalize(void);
void _ecore_xcb_sync_magic_send(int val, Ecore_X_Window win);

void _ecore_xcb_render_init(void);
void _ecore_xcb_render_finalize(void);
Eina_Bool _ecore_xcb_render_argb_get(void);
Eina_Bool _ecore_xcb_render_anim_get(void);
Eina_Bool _ecore_xcb_render_avail_get(void);

Eina_Bool _ecore_xcb_render_visual_supports_alpha(Ecore_X_Visual visual);
uint32_t _ecore_xcb_render_find_visual_id(int type, Eina_Bool check_alpha);
Ecore_X_Visual *_ecore_xcb_render_visual_get(int visual_id);

void _ecore_xcb_randr_init(void);
void _ecore_xcb_randr_finalize(void);

void _ecore_xcb_xfixes_init(void);
void _ecore_xcb_xfixes_finalize(void);
Eina_Bool _ecore_xcb_xfixes_avail_get(void);

void _ecore_xcb_damage_init(void);
void _ecore_xcb_damage_finalize(void);

void _ecore_xcb_composite_init(void);
void _ecore_xcb_composite_finalize(void);

void _ecore_xcb_dpms_init(void);
void _ecore_xcb_dpms_finalize(void);

void _ecore_xcb_cursor_init(void);
void _ecore_xcb_cursor_finalize(void);

void _ecore_xcb_xinerama_init(void);
void _ecore_xcb_xinerama_finalize(void);

void _ecore_xcb_dnd_init(void);
void _ecore_xcb_dnd_shutdown(void);
Ecore_X_DND_Source *_ecore_xcb_dnd_source_get(void);
Ecore_X_DND_Target *_ecore_xcb_dnd_target_get(void);
void _ecore_xcb_dnd_drag(Ecore_X_Window root, int x, int y);

void _ecore_xcb_selection_init(void);
void _ecore_xcb_selection_shutdown(void);
void *_ecore_xcb_selection_parse(const char *target, void *data, int size, int format);
char *_ecore_xcb_selection_target_get(Ecore_X_Atom target);
Ecore_X_Selection_Intern *_ecore_xcb_selection_get(Ecore_X_Atom selection);

# ifdef HAVE_ICONV
Eina_Bool _ecore_xcb_utf8_textlist_to_textproperty(char **list, int count, Ecore_Xcb_Encoding_Style style, Ecore_Xcb_Textproperty *ret);
# endif
Eina_Bool _ecore_xcb_mb_textlist_to_textproperty(char **list, int count, Ecore_Xcb_Encoding_Style style, Ecore_Xcb_Textproperty *ret);
Eina_Bool _ecore_xcb_textlist_to_textproperty(const char *type, char **list, int count, Ecore_Xcb_Encoding_Style style, Ecore_Xcb_Textproperty *ret);

# ifdef HAVE_ICONV
Eina_Bool _ecore_xcb_utf8_textproperty_to_textlist(const Ecore_Xcb_Textproperty *text_prop, char ***list_ret, int *count_ret);
# endif
Eina_Bool _ecore_xcb_mb_textproperty_to_textlist(const Ecore_Xcb_Textproperty *text_prop, char ***list_ret, int *count_ret);
Eina_Bool _ecore_xcb_textproperty_to_textlist(const Ecore_Xcb_Textproperty *text_prop, const char *type, char ***list_ret, int *count_ret);

void _ecore_xcb_events_init(void);
void _ecore_xcb_events_shutdown(void);
void _ecore_xcb_events_handle(xcb_generic_event_t *ev);
Ecore_X_Time _ecore_xcb_events_last_time_get(void);
unsigned int _ecore_xcb_events_modifiers_get(unsigned int state);
void _ecore_xcb_event_mouse_move(uint16_t timestamp, uint16_t modifiers, int16_t x, int16_t y, int16_t root_x, int16_t root_y, xcb_window_t event_win, xcb_window_t win, xcb_window_t root_win, uint8_t same_screen, int dev, double radx, double rady, double pressure, double angle, int16_t mx, int16_t my, int16_t mrx, int16_t mry);
Ecore_Event_Mouse_Button *_ecore_xcb_event_mouse_button(int event, uint16_t timestamp, uint16_t modifiers, xcb_button_t buttons, int16_t x, int16_t y, int16_t root_x, int16_t root_y, xcb_window_t event_win, xcb_window_t win, xcb_window_t root_win, uint8_t same_screen, int dev, double radx, double rady, double pressure, double angle, int16_t mx, int16_t my, int16_t mrx, int16_t mry);

void _ecore_xcb_keymap_init(void);
void _ecore_xcb_keymap_finalize(void);
void _ecore_xcb_keymap_shutdown(void);
void _ecore_xcb_keymap_refresh(xcb_mapping_notify_event_t *event);
xcb_keysym_t _ecore_xcb_keymap_keycode_to_keysym(xcb_keycode_t keycode, int col);
xcb_keycode_t *_ecore_xcb_keymap_keysym_to_keycode(xcb_keysym_t keysym);
char *_ecore_xcb_keymap_keysym_to_string(xcb_keysym_t keysym);
xcb_keycode_t _ecore_xcb_keymap_string_to_keycode(const char *key);
int _ecore_xcb_keymap_lookup_string(xcb_keycode_t keycode, int state, char *buffer, int bytes, xcb_keysym_t *sym);

void _ecore_xcb_input_init(void);
void _ecore_xcb_input_finalize(void);
void _ecore_xcb_input_shutdown(void);
# ifdef ECORE_XCB_XINPUT
void _ecore_xcb_input_handle_event(xcb_generic_event_t *event);
# else
void _ecore_xcb_input_handle_event(xcb_generic_event_t *event __UNUSED__);
# endif

void _ecore_xcb_dri_init(void);
void _ecore_xcb_dri_finalize(void);

void _ecore_xcb_xtest_init(void);
void _ecore_xcb_xtest_finalize(void);

Ecore_X_Window _ecore_xcb_window_root_of_screen_get(int screen);
void _ecore_xcb_window_prop_string_utf8_set(Ecore_X_Window win, Ecore_X_Atom atom, const char *str);
Ecore_X_Visual _ecore_xcb_window_visual_get(Ecore_X_Window win);
void _ecore_xcb_window_button_grab_remove(Ecore_X_Window win);
void _ecore_xcb_window_key_grab_remove(Ecore_X_Window win);
void _ecore_xcb_window_grab_allow_events(Ecore_X_Window event_win, Ecore_X_Window child_win, int type, void *event, Ecore_X_Time timestamp);

int _ecore_xcb_netwm_startup_info_begin(Ecore_X_Window win __UNUSED__, uint8_t data __UNUSED__);
int _ecore_xcb_netwm_startup_info(Ecore_X_Window win __UNUSED__, uint8_t data __UNUSED__);
Ecore_X_Window_State _ecore_xcb_netwm_window_state_get(Ecore_X_Atom atom);

int _ecore_xcb_error_handle(xcb_generic_error_t *err);
int _ecore_xcb_io_error_handle(xcb_generic_error_t *err);

xcb_image_t *_ecore_xcb_image_create_native(int w, int h, xcb_image_format_t format, uint8_t depth, void *base, uint32_t bytes, uint8_t *data);

void _ecore_xcb_xdefaults_init(void);
void _ecore_xcb_xdefaults_shutdown(void);
char *_ecore_xcb_xdefaults_string_get(const char *prog, const char *param);
int _ecore_xcb_xdefaults_int_get(const char *prog, const char *param);

#endif
