#ifndef ECORE_AUDIO_PRIVATE_H_
#define ECORE_AUDIO_PRIVATE_H_

#ifdef __linux__
#include <features.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "Ecore.h"
#include "ecore_private.h"

#include "Ecore_Audio.h"

extern int _ecore_audio_log_dom;

#ifdef ECORE_AUDIO_DEFAULT_LOG_COLOR
#undef ECORE_AUDIO_DEFAULT_LOG_COLOR
#endif
#define ECORE_AUDIO_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_audio_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_audio_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_audio_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_audio_log_dom, __VA_ARGS__)

#ifdef CRIT
#undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_audio_log_dom, __VA_ARGS__)

#define ECORE_MAGIC_AUDIO_MODULE 0xabba0001
#define ECORE_MAGIC_AUDIO_OUTPUT 0xabba0002
#define ECORE_MAGIC_AUDIO_INPUT  0xabba0003

/**
 * @addtogroup Ecore_Audio_Module_API_Group Ecore_Audio_Module_API - API for modules
 *
 * @{
 */

typedef struct _Ecore_Audio_Input Ecore_Audio_Input;
typedef struct _Ecore_Audio_Output Ecore_Audio_Output;

/**
 * @brief Functions for inputs
 */
struct input_api
{
   /**
    * @brief Create a new input
    *
    * @param in The input already initialized from @ref ecore_audio_input_new()
    *
    * @return The input, NULL on error
    */
   Ecore_Audio_Object *(*input_new)(Ecore_Audio_Object *in);

   /**
    * @brief Free an input
    *
    * @param in The input
    */
   void               (*input_del)(Ecore_Audio_Object *in);

   /**
    * @brief Read data from an input
    *
    * @param in The input to read from
    * @param data A pointer to a buffer where the data is put
    * @param len The amount of data to read in bytes
    */
   int                (*input_read)(Ecore_Audio_Object *in, void *data, int len);

   /**
    * @brief Seek within an input
    *
    * @param in The input
    * @param count The amount to seek in seconds
    * @param mode The mode
    *
    * @return The current position in seconds
    *
    */
   double              (*input_seek)(Ecore_Audio_Object *in, double count, int mode);

   /**
    * @brief Set the volume
    *
    * @param in the input
    * @param volume the volume
    */
   void                 (*input_volume_set)(Ecore_Audio_Object *in, double volume);

   /**
    * @brief Get the volume
    *
    * @param in the input
    * 
    * @return the volume
    */
   double               (*input_volume_get)(Ecore_Audio_Object *in);

   void                 (*input_looped_set)(Ecore_Audio_Object *in, Eina_Bool looped);
   Eina_Bool            (*input_looped_get)(Ecore_Audio_Object *in);

   void                 (*input_preloaded_set)(Ecore_Audio_Object *in, Eina_Bool preloaded);
};

/**
 * @brief Functions for outputs
 */
struct output_api
{
   /**
    * @brief Create a new output
    *
    * @param out The output already initialized from @ref ecore_audio_output_new()
    *
    * @return The output, NULL on error
    */
   Ecore_Audio_Object *(*output_new)(Ecore_Audio_Object * out);

   /**
    * @brief Free an output
    *
    * @param out the output
    */
   void                (*output_del)(Ecore_Audio_Object *out);

   /**
    * @brief Set the volume of the output
    *
    * @param out The output
    * @param vol The volume in the range of 0-255
    */
   void                (*output_volume_set)(Ecore_Audio_Object *out, double vol);

   /**
    * @brief Get the volume of the output
    *
    * @param out The output
    *
    * @return vol The volume
    */
   double              (*output_volume_get)(Ecore_Audio_Object *out);

   /**
    * @brief Set the paused state of the output
    *
    * @param out The output
    * @param paused The paused state
    */
   void                (*output_paused_set)(Ecore_Audio_Object *out, Eina_Bool paused);

   /**
    * @brief Add an input to an output
    *
    * @param out The output
    * @param in The input
    *
    * @return EINA_TRUE if the operation was successful, EINA_FALSE otherwise
    */
   Eina_Bool           (*output_add_input)(Ecore_Audio_Object *out, Ecore_Audio_Object *in);

   /**
    * @brief Delete an input from an output
    *
    * @param out The output
    * @param in The input
    *
    * @return EINA_TRUE if the operation was successful, EINA_FALSE otherwise
    */
   Eina_Bool           (*output_del_input)(Ecore_Audio_Object *out, Ecore_Audio_Object *in);

   void                (*output_update_input_format)(Ecore_Audio_Object *out, Ecore_Audio_Object *in);
};

/**
 * @brief The structure representing an Ecore_Audio module
 */
struct _Ecore_Audio_Module
{
   ECORE_MAGIC;
   Ecore_Audio_Type type;
   char              *name;
   Eina_List         *inputs;
   Eina_List         *outputs;

   void              *priv;

   struct input_api  *in_ops;
   struct output_api *out_ops;
};

/**
 * @brief A common structure, could be input or output
 */
struct _Ecore_Audio_Object
{
   ECORE_MAGIC;
   const char         *name;
   Ecore_Audio_Module *module;

   Eina_Bool           paused;

   void               *module_data;
   void               *obj_data;
   void               *user_data;

};

/**
 * @brief The structure representing an Ecore_Audio output
 */
struct _Ecore_Audio_Output
{
   ECORE_MAGIC;
   const char         *name;
   Ecore_Audio_Module *module;

   Eina_Bool           paused;

   void               *module_data;
   void               *obj_data;
   void               *user_data;

   Eina_List          *inputs; /**< The inputs that are connected to this output */
};

/**
 * @brief The structure representing an Ecore_Audio input
 */
struct _Ecore_Audio_Input
{
   ECORE_MAGIC;
   const char         *name;
   Ecore_Audio_Module *module;

   Eina_Bool           paused; /**< Is the input paused? */

   void               *module_data;
   void               *obj_data;
   void               *user_data;

   Ecore_Audio_Output *output; /**< The output this input is connected to */

   int                 samplerate;
   int                 channels;
   Eina_Bool           looped; /**< Loop the sound */
   double              length; /**< Length of the sound */
   Eina_Bool           preloaded;
   Eina_Bool           ended;
};

/**
 * @brief The structure representing an Ecore_Audio format
 */
struct _Ecore_Audio_Format
{
   unsigned int   rate;
   unsigned short channels;
};

struct _Ecore_Audio_Callback {
    Ecore_Audio_Read_Callback read_cb;
    void *data;
};

extern Eina_List *ecore_audio_modules;

/**
 * @}
 */
#endif
