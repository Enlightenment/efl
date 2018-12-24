#ifndef _EXACTNESS_H
#define _EXACTNESS_H

#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EXACTNESS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EXACTNESS_BUILD */
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
#endif /* ! _WIN32 */

/**
 * @page exactness_main Exactness
 *
 * @date 2018 (created)
 *
 * This page describes the public structures and APIs available for Exactness.
 *
 * @addtogroup Exactness
 * @{
 */

/**
 * @typedef Exactness_Action_Type
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
 * @typedef Exactness_Action_Mouse_Wheel
 * The type for the Exactness Mouse Wheel action.
 */
typedef struct
{
   int direction;
   int z;
} Exactness_Action_Mouse_Wheel;

/**
 * @typedef Exactness_Action_Key_Down_Up
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
 * @typedef Exactness_Action_Multi_Event
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
 * @typedef Exactness_Action_Multi_Move
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
 * @typedef Exactness_Action_Efl_Event
 * The type for the Exactness EFL Event action.
 */
typedef struct
{
   char *wdg_name;
   char *event_name;
} Exactness_Action_Efl_Event;

/**
 * @typedef Exactness_Action_Click_On
 * The type for the Exactness Click on (widget) action.
 */
typedef struct
{
   char *wdg_name;
} Exactness_Action_Click_On;

/**
 * @typedef Exactness_Action
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
 * @typedef Exactness_Object
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
 * @typedef Exactness_Objects
 * The type for the Exactness objects list.
 */
typedef struct
{
   Eina_List *objs;        /**< List of all the objects */
   /* main_objs not in EET */
   Eina_List *main_objs;   /**< List of the main objects */
} Exactness_Objects;

/**
 * @typedef Exactness_Image
 * The type for the Exactness Image.
 */
typedef struct
{
   unsigned int w;   /**< Width of the image */
   unsigned int h;   /**< Height of the image */
   void *pixels;     /**< Pixels of the image */
} Exactness_Image;

typedef struct
{
   char *language; /**< String describing the language of the content e.g "C"...*/
   char *content; /**< Content used as source */
   char *command; /**< Command needed to generate the application from the content */
} Exactness_Source_Code;

typedef struct
{
   Eina_List *actions;  /**< List of Exactness_Action */
   /* imgs not in EET */
   Eina_List *imgs;     /**< List of Exactness_Image */
   Eina_List *objs;     /**< List of Exactness_Objects */
   Eina_List *codes;    /**< List of Exactness_Source_Code */
   const char *fonts_path; /**< Path to the fonts to use, relative to the fonts dir given in parameter to the player/recorder */
   int nb_shots;        /**< The number of shots present in the unit */
} Exactness_Unit;

/**
 * @brief Read an unit from a given file
 *
 * @param filename Name of the file containing the unit
 *
 * @return the unit
 */
EAPI Exactness_Unit *exactness_unit_file_read(const char *filename);

/**
 * @brief Write an unit into the given file
 *
 * @param unit Unit to store
 * @param filename Name of the file containing the unit
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 */
EAPI Eina_Bool exactness_unit_file_write(Exactness_Unit *unit, const char *filename);

/**
 * @brief Compare two images
 *
 * @param img1 first image
 * @param img2 second image
 * @param imgO pointer for the diff image. Can be NULL
 *
 * @return EINA_TRUE if the images are different, EINA_FALSE otherwise
 */
EAPI Eina_Bool exactness_image_compare(Exactness_Image *img1, Exactness_Image *img2, Exactness_Image **imgO);

/**
 * @brief Free the given image
 *
 * @param img the image
 *
 */
EAPI void exactness_image_free(Exactness_Image *img);

/**
 * @brief Read a legacy file and convert it to an unit
 *
 * @param filename Name of the legacy file
 *
 * @return the unit
 */
EAPI Exactness_Unit *legacy_rec_file_read(const char *filename);

/**
 * @}
 */

#endif /* _EXACTNESS_H */
