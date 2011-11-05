#ifndef _ECORE_PSL1GHT_H
#define _ECORE_PSL1GHT_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

/**
 * @file
 * @brief Ecore PSL1GHT system functions.
 */

#ifdef __cplusplus
extern "C" {
#endif

EAPI extern int ECORE_PSL1GHT_EVENT_KEY_MODIFIERS;
EAPI extern int ECORE_PSL1GHT_EVENT_GOT_FOCUS;
EAPI extern int ECORE_PSL1GHT_EVENT_LOST_FOCUS;
EAPI extern int ECORE_PSL1GHT_EVENT_EXPOSE;

typedef struct _Ecore_Psl1ght_Event_Key_Modifiers Ecore_Psl1ght_Event_Key_Modifiers;
struct _Ecore_Psl1ght_Event_Key_Modifiers /** PSL1GHT Key Modifier event */
{
   unsigned int timestamp;
   unsigned int modifiers;
   int          shift_changed : 1;
   int          shift : 1;
   int          alt_changed : 1;
   int          alt : 1;
   int          ctrl_changed : 1;
   int          ctrl : 1;
   int          win_changed : 1;
   int          win : 1;
   int          num_lock_changed : 1;
   int          num_lock : 1;
   int          caps_lock_changed : 1;
   int          caps_lock : 1;
   int          scroll_lock_changed : 1;
   int          scroll_lock : 1;
};

typedef struct _Ecore_Psl1ght_Event_Key_Down Ecore_Psl1ght_Event_Key_Down;
struct _Ecore_Psl1ght_Event_Key_Down /** PSL1GHT Key Down event */
{
   const char  *keyname;     /**< The name of the key that was pressed */
   const char  *keycompose;     /**< The UTF-8 string conversion if any */
   unsigned int time;
};

typedef struct _Ecore_Psl1ght_Event_Key_Up Ecore_Psl1ght_Event_Key_Up;
struct _Ecore_Psl1ght_Event_Key_Up /** PSL1GHT Key Up event */
{
   const char  *keyname;     /**< The name of the key that was released */
   const char  *keycompose;     /**< The UTF-8 string conversion if any */
   unsigned int time;
};

typedef struct _Ecore_Psl1ght_Event_Mouse_Button_Down Ecore_Psl1ght_Event_Mouse_Button_Down;
struct _Ecore_Psl1ght_Event_Mouse_Button_Down /** PSL1GHT Mouse Down event */
{
   int          button;    /**< Mouse button that was pressed (1 - 32) */
   int          x;    /**< Mouse co-ordinates when mouse button was pressed */
   int          y;    /**< Mouse co-ordinates when mouse button was pressed */
   int          double_click : 1;    /**< Set if click was a double click */
   int          triple_click : 1;    /**< Set if click was a triple click  */
   unsigned int time;
};

typedef struct _Ecore_Psl1ght_Event_Mouse_Button_Up Ecore_Psl1ght_Event_Mouse_Button_Up;
struct _Ecore_Psl1ght_Event_Mouse_Button_Up /** PSL1GHT Mouse Up event */
{
   int          button;    /**< Mouse button that was released (1 - 32) */
   int          x;    /**< Mouse co-ordinates when mouse button was raised */
   int          y;    /**< Mouse co-ordinates when mouse button was raised */
   int          double_click : 1;    /**< Set if click was a double click */
   int          triple_click : 1;    /**< Set if click was a triple click  */
   unsigned int time;
};

typedef struct _Ecore_Psl1ght_Event_Mouse_Move Ecore_Psl1ght_Event_Mouse_Move;
struct _Ecore_Psl1ght_Event_Mouse_Move /** PSL1GHT Mouse Move event */
{
   int          x;    /**< Mouse co-ordinates where the mouse cursor moved to */
   int          y;    /**< Mouse co-ordinates where the mouse cursor moved to */
   unsigned int time;
};

typedef struct _Ecore_Psl1ght_Event_Mouse_Wheel Ecore_Psl1ght_Event_Mouse_Wheel;
struct _Ecore_Psl1ght_Event_Mouse_Wheel /** PSL1GHT Mouse Wheel event */
{
   int          x, y;
   int          direction;    /* 0 = vertical, 1 = horizontal */
   int          wheel;    /* value 1 (left/up), -1 (right/down) */
   unsigned int time;
};

EAPI int
          ecore_psl1ght_init(const char *name);
EAPI int
          ecore_psl1ght_shutdown(void);
EAPI void
          ecore_psl1ght_resolution_set(int width, int height);
EAPI void
          ecore_psl1ght_poll_events(void);

EAPI void ecore_psl1ght_screen_resolution_get(int *w, int *h);
EAPI void ecore_psl1ght_optimal_screen_resolution_get(int *w, int *h);

#ifdef __cplusplus
}
#endif

#endif
