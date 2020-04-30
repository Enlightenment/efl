#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eet.h>
#include <Evas.h>
#include <Elementary.h>

typedef struct
{
   Eina_Debug_Session *session;
   int srcid;
   void *buffer;
   unsigned int size;
} _Main_Loop_Info;

#define WRAPPER_TO_XFER_MAIN_LOOP(foo) \
static void \
_intern_main_loop ## foo(void *data) \
{ \
   _Main_Loop_Info *info = data; \
   _main_loop ## foo(info->session, info->srcid, info->buffer, info->size); \
   free(info->buffer); \
   free(info); \
} \
static Eina_Bool \
foo(Eina_Debug_Session *session, int srcid, void *buffer, int size) \
{ \
   _Main_Loop_Info *info = calloc(1, sizeof(*info)); \
   info->session = session; \
   info->srcid = srcid; \
   info->size = size; \
   if (info->size) \
     { \
        info->buffer = malloc(info->size); \
        memcpy(info->buffer, buffer, info->size); \
     } \
   ecore_main_loop_thread_safe_call_async(_intern_main_loop ## foo, info); \
   return EINA_TRUE; \
}

#ifndef WORDS_BIGENDIAN
#define SWAP_64(x) x
#define SWAP_32(x) x
#define SWAP_16(x) x
#define SWAP_DBL(x) x
#else
#define SWAP_64(x) eina_swap64(x)
#define SWAP_32(x) eina_swap32(x)
#define SWAP_16(x) eina_swap16(x)
#define SWAP_DBL(x) SWAP_64(x)
#endif

#define EXTRACT_INT(_buf) \
({ \
   int __i; \
   memcpy(&__i, _buf, sizeof(int)); \
   _buf += sizeof(int); \
   SWAP_32(__i); \
})

#define EXTRACT_DOUBLE(_buf) \
({ \
   double __d; \
   memcpy(&__d, _buf, sizeof(double)); \
   _buf += sizeof(double); \
   SWAP_DBL(__d); \
})

#define EXTRACT_STRING(_buf) \
({ \
   char *__s = _buf ? strdup(_buf) : NULL; \
   int __len = (__s ? strlen(__s) : 0) + 1; \
   _buf += __len; \
   __s; \
})

#define STORE_INT(_buf, __i) \
({ \
   int __si = SWAP_32(__i); \
   memcpy(_buf, &__si, sizeof(int)); \
   _buf += sizeof(int); \
})

#define STORE_DOUBLE(_buf, __d) \
{ \
   double __d2 = SWAP_DBL(__d); \
   memcpy(_buf, &__d2, sizeof(double)); \
   _buf += sizeof(double); \
}

#define STORE_STRING(_buf, __s) \
{ \
   int __len = (__s ? strlen(__s) : 0) + 1; \
   if (__s) memcpy(_buf, __s, __len); \
   else *_buf = '\0'; \
   _buf += __len; \
}

#define SHOT_DELIMITER '+'

/**
 * The type values for an Exactness action.
 */
typedef enum
{
   EXACTNESS_ACTION_UNKNOWN = 0,
   EXACTNESS_ACTION_MOUSE_IN,
   EXACTNESS_ACTION_MOUSE_OUT,
   EXACTNESS_ACTION_MOUSE_WHEEL,
   EXACTNESS_ACTION_MULTI_DOWN,
   EXACTNESS_ACTION_MULTI_UP,
   EXACTNESS_ACTION_MULTI_MOVE,
   EXACTNESS_ACTION_KEY_DOWN,
   EXACTNESS_ACTION_KEY_UP,
   EXACTNESS_ACTION_TAKE_SHOT,
   EXACTNESS_ACTION_EFL_EVENT,
   EXACTNESS_ACTION_CLICK_ON,
   EXACTNESS_ACTION_STABILIZE,
   EXACTNESS_ACTION_LAST = EXACTNESS_ACTION_STABILIZE
   /* Add any supported actions here and update _LAST */
} Exactness_Action_Type;

/**
 * The type for the Exactness Mouse Wheel action.
 */
typedef struct
{
   int direction;
   int z;
} Exactness_Action_Mouse_Wheel;

/**
 * The type for the Exactness Key Down Up action.
 */
typedef struct
{
   const char *keyname;
   const char *key;
   const char *string;
   const char *compose;
   unsigned int keycode;
} Exactness_Action_Key_Down_Up;

/**
 * The type for the Exactness Multi Event action.
 */
typedef struct
{
   int d;
   int b; /* In case of simple mouse down/up, corresponds to the button */
   int x;
   int y;
   double rad;
   double radx;
   double rady;
   double pres;
   double ang;
   double fx;
   double fy;
   Evas_Button_Flags flags;
} Exactness_Action_Multi_Event;

/**
 * The type for the Exactness Multi Move action.
 */
typedef struct
{
   int d;
   int x;
   int y;
   double rad;
   double radx;
   double rady;
   double pres;
   double ang;
   double fx;
   double fy;
} Exactness_Action_Multi_Move;

/**
 * The type for the Exactness EFL Event action.
 */
typedef struct
{
   char *wdg_name; /**< Name of the widget */
   char *event_name; /**< Name of the event */
} Exactness_Action_Efl_Event;

/**
 * The type for the Exactness Click on (widget) action.
 */
typedef struct
{
   char *wdg_name;	/**< Name of the widget */
} Exactness_Action_Click_On;

/**
 * The type for the Exactness action.
 */
typedef struct
{
   Exactness_Action_Type type;   /**< The action type */
   unsigned int n_evas;          /**< The evas number on which the action has to be applied */
   unsigned int delay_ms;        /**< The delay (in ms) to wait for this action */
   void *data;                   /**< The specific action data */
} Exactness_Action;

/**
 * The type for the Exactness object.
 */
typedef struct
{
   long long id;                 /**< The Eo pointer */
   long long parent_id;          /**< The Eo parent pointer */
   const char *kl_name;          /**< The class name */

   Eina_List *children; /* NOT EET */

   /* Evas stuff */
   int x;   /**< The X coordinate */
   int y;   /**< The Y coordinate */
   int w;   /**< The object width */
   int h;   /**< The object height */
} Exactness_Object;

/**
 * The type for the Exactness objects list.
 */
typedef struct
{
   Eina_List *objs;        /**< List of all the objects */
   /* main_objs not in EET */
   Eina_List *main_objs;   /**< List of the main objects */
} Exactness_Objects;

/**
 * The type for the Exactness Image.
 */
typedef struct
{
   unsigned int w;   /**< Width of the image */
   unsigned int h;   /**< Height of the image */
   void *pixels;     /**< Pixels of the image */
} Exactness_Image;

/**
 * An Exactness test unit, including the list of tested actions and produced images.
 */
typedef struct
{
   Eina_List *actions;  /**< List of Exactness_Action */
   /* imgs not in EET */
   Eina_List *imgs;     /**< List of Exactness_Image */
   Eina_List *objs;     /**< List of Exactness_Objects */
   const char *fonts_path; /**< Path to the fonts to use, relative to the fonts dir given in parameter to the player/recorder */
   int nb_shots;        /**< The number of shots present in the unit */
} Exactness_Unit;

const char *_exactness_action_type_to_string_get(Exactness_Action_Type type);

Eina_Bool ex_is_original_app(void);
void ex_set_original_envvar(void);
Eina_Bool exactness_image_compare(Exactness_Image *img1, Exactness_Image *img2, Exactness_Image **diff_img);
Exactness_Unit *exactness_unit_file_read(const char *filename);
Eina_Bool exactness_unit_file_write(Exactness_Unit *unit, const char *filename);
void exactness_image_free(Exactness_Image *img);

void ex_prepare_elm_overlay(void);
