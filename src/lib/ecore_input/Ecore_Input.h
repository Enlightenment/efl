#ifndef _ECORE_INPUT_H
#define _ECORE_INPUT_H

#ifdef _WIN32
# include <stddef.h>
#else
# include <inttypes.h>
#endif

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_INPUT_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_INPUT_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

   EAPI extern int ECORE_EVENT_KEY_DOWN;
   EAPI extern int ECORE_EVENT_KEY_UP;
   EAPI extern int ECORE_EVENT_MOUSE_BUTTON_DOWN;
   EAPI extern int ECORE_EVENT_MOUSE_BUTTON_UP;
   EAPI extern int ECORE_EVENT_MOUSE_MOVE;
   EAPI extern int ECORE_EVENT_MOUSE_WHEEL;
   EAPI extern int ECORE_EVENT_MOUSE_IN;
   EAPI extern int ECORE_EVENT_MOUSE_OUT;

#define ECORE_EVENT_MODIFIER_SHIFT      0x0001
#define ECORE_EVENT_MODIFIER_CTRL       0x0002
#define ECORE_EVENT_MODIFIER_ALT        0x0004
#define ECORE_EVENT_MODIFIER_WIN        0x0008
#define ECORE_EVENT_MODIFIER_SCROLL     0x0010
#define ECORE_EVENT_MODIFIER_NUM        0x0020
#define ECORE_EVENT_MODIFIER_CAPS       0x0040
#define ECORE_EVENT_LOCK_SCROLL         0x0080
#define ECORE_EVENT_LOCK_NUM            0x0100
#define ECORE_EVENT_LOCK_CAPS           0x0200
#define ECORE_EVENT_LOCK_SHIFT          0x0300
#define ECORE_EVENT_MODIFIER_ALTGR      0x0400 /**< @since 1.7 */

#ifndef _ECORE_WINDOW_PREDEF
   typedef uintptr_t                        Ecore_Window;
#define _ECORE_WINDOW_PREDEF 1
#endif

   typedef struct _Ecore_Event_Key          Ecore_Event_Key;
   typedef struct _Ecore_Event_Mouse_Button Ecore_Event_Mouse_Button;
   typedef struct _Ecore_Event_Mouse_Wheel  Ecore_Event_Mouse_Wheel;
   typedef struct _Ecore_Event_Mouse_Move   Ecore_Event_Mouse_Move;
   typedef struct _Ecore_Event_Mouse_IO     Ecore_Event_Mouse_IO;
   typedef struct _Ecore_Event_Modifiers    Ecore_Event_Modifiers;
   
   typedef enum _Ecore_Event_Modifier
     {
        ECORE_NONE,
        ECORE_SHIFT,
        ECORE_CTRL,
        ECORE_ALT,
        ECORE_WIN,
        ECORE_SCROLL,
        ECORE_CAPS,
        ECORE_MODE, /**< @since 1.7 */
        ECORE_LAST
     } Ecore_Event_Modifier;

   typedef enum _Ecore_Event_Press
     {
        ECORE_DOWN,
        ECORE_UP
     } Ecore_Event_Press;

   typedef enum _Ecore_Event_IO
     {
        ECORE_IN,
        ECORE_OUT
     } Ecore_Event_IO;

   typedef enum _Ecore_Compose_State
     {   
        ECORE_COMPOSE_NONE,
        ECORE_COMPOSE_MIDDLE,
        ECORE_COMPOSE_DONE
     } Ecore_Compose_State;

   struct _Ecore_Event_Key
     {
        const char      *keyname;
        const char      *key;
        const char      *string;
        const char      *compose;
        Ecore_Window     window;
        Ecore_Window     root_window;
        Ecore_Window     event_window;
        
        unsigned int     timestamp;
        unsigned int     modifiers;
        
        int              same_screen;
     };

   struct _Ecore_Event_Mouse_Button
     {
        Ecore_Window     window;
        Ecore_Window     root_window;
        Ecore_Window     event_window;
        
        unsigned int     timestamp;
        unsigned int     modifiers;
        unsigned int     buttons;
        unsigned int     double_click;
        unsigned int     triple_click;
        int              same_screen;
        
        int              x;
        int              y;
        struct {
           int           x;
           int           y;
        } root;
        
        struct {
           int           device; /* 0 if normal mouse, 1+ for other mouse-devices (eg multi-touch - other fingers) */
           double        radius, radius_x, radius_y; /* radius of press point - radius_x and y if its an ellipse (radius is the average of the 2) */
           double        pressure; /* pressure - 1.0 == normal, > 1.0 == more, 0.0 == none */
           double        angle; /* angle relative to perpendicular (0.0 == perpendicular), in degrees */
           double        x, y; /* same as x, y root.x, root.y, but with sub-pixel precision, if available */
           struct {
              double     x, y;
           } root;
        } multi;
     };
   
   struct _Ecore_Event_Mouse_Wheel
     {
        Ecore_Window     window;
        Ecore_Window     root_window;
        Ecore_Window     event_window;
        
        unsigned int     timestamp;
        unsigned int     modifiers;
        
        int              same_screen;
        int              direction;
        int              z;
        
        int              x;
        int              y;
        struct {
           int           x;
           int           y;
        } root;
     };
   
   struct _Ecore_Event_Mouse_Move
     {
        Ecore_Window     window;
        Ecore_Window     root_window;
        Ecore_Window     event_window;
        
        unsigned int     timestamp;
        unsigned int     modifiers;
        
        int              same_screen;
        
        int              x;
        int              y;
        struct {
           int           x;
           int           y;
        } root;
        
        struct {
           int           device; /* 0 if normal mouse, 1+ for other mouse-devices (eg multi-touch - other fingers) */
           double        radius, radius_x, radius_y; /* radius of press point - radius_x and y if its an ellipse (radius is the average of the 2) */
           double        pressure; /* pressure - 1.0 == normal, > 1.0 == more, 0.0 == none */
           double        angle; /* angle relative to perpendicular (0.0 == perpendicular), in degrees */
           double        x, y; /* same as x, y root.x, root.y, but with sub-pixel precision, if available */
           struct {
              double     x, y;
           } root;
        } multi;
     };
   
   struct _Ecore_Event_Mouse_IO
     {
        Ecore_Window     window;
        Ecore_Window     event_window;
        
        unsigned int     timestamp;
        unsigned int     modifiers;
        
        int              x;
        int              y;
     };

   struct _Ecore_Event_Modifiers
     {
        unsigned int size;
        unsigned int array[ECORE_LAST];
     };
   
   EAPI int                  ecore_event_init(void);
   EAPI int                  ecore_event_shutdown(void);
   
   EAPI unsigned int         ecore_event_modifier_mask(Ecore_Event_Modifier modifier);
   EAPI Ecore_Event_Modifier ecore_event_update_modifier(const char *key, Ecore_Event_Modifiers *modifiers, int inc);

   /**
    * @since 1.7
    */
   EAPI Ecore_Compose_State  ecore_compose_get(const Eina_List *seq, char **seqstr_ret);
   
#ifdef __cplusplus
}
#endif

#endif
