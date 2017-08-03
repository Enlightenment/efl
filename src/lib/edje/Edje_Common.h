#include "edje_types.eot.h"

/**
 * @internal
 * @ingroup Edje_General_Group
 *
 * @typedef Edje_Version
 * Type for edje version
 *
 */
typedef struct _Edje_Version
{
   int major; /**< The major number of Edje version. */
   int minor; /**< The minor number of Edje version. */
   int micro; /**< The micro number of Edje version. */
   int revision; /**< The revision of Edje version) */
} Edje_Version;

EAPI extern Edje_Version *edje_version;

#ifndef _EDJE_OBJECT_EO_CLASS_TYPE
#define _EDJE_OBJECT_EO_CLASS_TYPE

typedef Eo Edje_Object;

#endif

/**
 * @typedef Edje_Color_Class
 * Type for edje color class
 */
typedef struct _Edje_Color_Class Edje_Color_Class;

struct _Edje_Color_Class
{
   const char       *name;

   unsigned char  r, g, b, a;
   unsigned char  r2, g2, b2, a2;
   unsigned char  r3, g3, b3, a3;
   Eina_Stringshare *desc;
};

/**
 * @typedef Edje_Text_Class
 * Type for edje text class
 *
 * @since 1.17
 */
typedef struct _Edje_Text_Class Edje_Text_Class;

struct _Edje_Text_Class
{
   const char     *name;
   const char     *font;
   Evas_Font_Size  size;
};

/**
 * @typedef Edje_Size_Class
 * Type for edje size class
 *
 * @since 1.17
 */
typedef struct _Edje_Size_Class Edje_Size_Class;

struct _Edje_Size_Class
{
   Eina_Stringshare *name;
   Evas_Coord        minw;
   Evas_Coord        minh;
   Evas_Coord        maxw;
   Evas_Coord        maxh;
};

/**
 * @defgroup Edje_Object_Communication_Interface_Signal Edje Communication Interface: Signal
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with signals.
 *
 * Edje has two communication interfaces between @b code and @b theme.
 * Signals and messages.
 *
 * This group has functions that deal with signals. One can either
 * emit a signal from @b code to a @b theme or create handles for
 * the ones emitted from @b themes. Signals are identified by strings.
 *
 * @{
 */

typedef void (*Efl_Signal_Cb) (void *data, Evas_Object *obj, const char *emission, const char *source); /**< Edje signal callback functions's prototype definition. @c data will have the auxiliary data pointer set at the time the callback registration. @c obj will be a pointer the Edje object where the signal comes from. @c emission will identify the exact signal's emission string and @c source the exact signal's source one. */
typedef Efl_Signal_Cb Edje_Signal_Cb;

/**
 * @brief Gets extra data passed to callbacks.
 *
 * @return The extra data for that callback.
 *
 * Some callbacks pass extra information. This function gives access to that
 * extra information. It's somehow like event_info in smart callbacks.
 *
 * @see edje_object_signal_callback_add() for more on Edje signals.
 * @since 1.1.0
 */
EAPI void *       edje_object_signal_callback_extra_data_get(void);

/**
 * @}
 */

/**
 * @defgroup Edje_General_Group Edje General
 * @ingroup Edje
 *
 * @brief This group discusses functions that have general purposes or affect Edje as a whole.
 *
 * Besides containing the initialize and shutdown functions of the library, which should
 * always be called when we are using Edje, this module contains some other utilities that
 * could be used in many contexts or should do their jobs independent of the context inside Edje.
 *
 * @{
 */

/**
 * @brief Initializes the Edje library.
 *
 * @return The new init count. The initial value is zero.
 *
 * This function initializes the Edje library, making the proper calls
 * to internal initialization functions. It will also initialize its
 * @b dependencies, making calls to @c eina_init(), @c ecore_init(),
 * @c embryo_init() and @c eet_init(). So, there is no need to call
 * those functions again, in your code. To shutdown Edje there is the
 * function edje_shutdown().
 *
 * @see edje_shutdown()
 * @see eina_init()
 * @see ecore_init()
 * @see embryo_init()
 * @see eet_init()
 *
 */
EAPI int          edje_init                       (void);

/**
 * @brief Shuts down the Edje library.
 *
 * @return The number of times the library has been initialized
 *         without being shutdown.
 *
 * This function shuts down the Edje library. It will also call the
 * shutdown functions of its @b dependencies, which are @c
 * eina_shutdown(), @c ecore_shutdown(), @c embryo_shutdown() and @c
 * eet_shutdown(), so there is no need to call these functions again,
 * in your code.
 *
 * @see edje_init()
 * @see eina_shutdown()
 * @see ecore_shutdown()
 * @see embryo_shutdown()
 * @see eet_shutdown()
 *
 */
EAPI int		edje_shutdown			(void);

/**
 * @brief Sets the edje append fontset.
 *
 * @param fonts The fontset to append.
 *
 * This function sets the edje append fontset.
 *
 */
EAPI void		edje_fontset_append_set		(const char *fonts);

/**
 * @brief Gets data from the file level data block of an edje mapped file.
 * @param f The mapped edje file
 * @param key The data key
 * @return The string value of the data, or @c NULL if no key is found.
 * Must be freed by the user when no longer needed.
 *
 * If an edje file test.edj is built from the following edc:
 *
 * data {
 *   item: "key1" "value1";
 *   item: "key2" "value2";
 * }
 * collections { ... }
 *
 * Then, edje_file_data_get("test.edj", "key1") will return "value1"
 */
EAPI char             *edje_mmap_data_get(const Eina_File *f, const char *key);

/**
 * @brief Gets data from the file level data block of an edje file.
 * @param file The path to the .edj file
 * @param key The data key
 * @return The string value of the data, or @c NULL if no key is found.
 * Must be freed by the user when no longer needed.
 *
 * If an edje file test.edj is built from the following edc:
 *
 * data {
 *   item: "key1" "value1";
 *   item: "key2" "value2";
 * }
 * collections { ... }
 *
 * Then, edje_file_data_get("test.edj", "key1") will return "value1"
 *
 * @see edje_mmap_data_get()
 */
EAPI char        *edje_file_data_get              (const char *file, const char *key);

/**
 * @brief Loads a new module in Edje.
 * @param module The name of the module that will be added to Edje.
 * @return @c EINA_TRUE if the module was successfully loaded. Otherwise, @c EINA_FALSE.
 *
 * Modules are used to add functionality to Edje.
 * So, when a module is loaded, its functionality should be available for use.
 *
 */
EAPI Eina_Bool               edje_module_load                (const char *module);

/**
 * @brief Retrieves all modules that can be loaded.
 * @return A list of all loadable modules.
 *
 * This function retrieves all modules that can be loaded by edje_module_load().
 *
 * @see edje_module_load().
 *
 */
EAPI const Eina_List        *edje_available_modules_get      (void);

/**
 * @brief Gets the edje append fontset.
 *
 * @return The edje append fontset.
 *
 * This function returns the edje append fontset set by
 * edje_fontset_append_set() function.
 *
 * @see edje_fontset_append_set().
 *
 */
EAPI const char		*edje_fontset_append_get	(void);

/**
 * @brief Sets the file cache size.
 *
 * @param count The file cache size in edje file units. Default is 16.
 *
 * This function sets the file cache size. Edje keeps this cache in
 * order to prevent duplicates of edje file entries in memory. The
 * file cache size can be retrieved with edje_file_cache_get().
 *
 * @see edje_file_cache_get()
 * @see edje_file_cache_flush()
 *
 */
EAPI void         edje_file_cache_set             (int count);

/**
 * @brief Returns the file cache size.
 *
 * @return The file cache size in edje file units. Default is 16.
 *
 * This function returns the file cache size set by
 * edje_file_cache_set().
 *
 * @see edje_file_cache_set()
 * @see edje_file_cache_flush()
 *
 */
EAPI int          edje_file_cache_get             (void);

/**
 * @brief Cleans the file cache.
 *
 * This function cleans the file cache entries, but keeps this cache's
 * size to the last value set.
 *
 * @see edje_file_cache_set()
 * @see edje_file_cache_get()
 *
 */
EAPI void         edje_file_cache_flush           (void);

/**
 * @brief Sets the collection cache size.
 *
 * @param count The collection cache size, in edje object units. Default is 16.
 *
 * This function sets the collection cache size. Edje keeps this cache
 * in order to prevent duplicates of edje {collection,group,part}
 * entries in memory. The collection cache size can be retrieved with
 * edje_collection_cache_get().
 *
 * @see edje_collection_cache_get()
 * @see edje_collection_cache_flush()
 *
 */
EAPI void         edje_collection_cache_set       (int count);

/**
 * @brief Returns the collection cache size.
 *
 * @return The collection cache size, in edje object units. Default is 16.
 *
 * This function returns the collection cache size set by
 * edje_collection_cache_set().
 *
 * @see edje_collection_cache_set()
 * @see edje_collection_cache_flush()
 *
 */
EAPI int          edje_collection_cache_get       (void);

/**
 * @brief Cleans the collection cache.
 *
 * This function cleans the collection cache, but keeps this cache's
 * size to the last value set.
 *
 * @see edje_collection_cache_set()
 * @see edje_collection_cache_get()
 *
 */
EAPI void         edje_collection_cache_flush     (void);

/**
 * @}
 */

/**
 * @defgroup Edje_External_Group Edje External
 * @ingroup Edje
 *
 * @brief This group discusses functions of the external section of Edje.
 *
 * The programmer can create new types for parts, that will be called generically
 * EXTERNALS as they are not native of Edje. The developer must also create
 * plugins that will define the meaning of each extra properties carried by
 * these parts of type EXTERNAL.
 *
 * As long as there are new types properly registered with the plugins created,
 * the user can use the parts of type EXTERNAL as all the parts of native types.
 *
 * @{
 */

/** The possible types the parameters of an EXTERNAL part can be.
 *
 * @ingroup Edje_External
 */
typedef enum
{
  EDJE_EXTERNAL_PARAM_TYPE_INT = 0, /**< Parameter value is an integer. */
  EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, /**< Parameter value is a double. */
  EDJE_EXTERNAL_PARAM_TYPE_STRING, /**< Parameter value is a string. */
  EDJE_EXTERNAL_PARAM_TYPE_BOOL, /**< Parameter value is boolean. */
  EDJE_EXTERNAL_PARAM_TYPE_CHOICE, /**< Parameter value is one of a set of predefined string choices. */
  EDJE_EXTERNAL_PARAM_TYPE_MAX /**< Sentinel value to indicate last enum field during iteration */
} Edje_External_Param_Type;

/**
 * Flags that determine how a parameter may be accessed in different
 * circumstances.
 */
typedef enum _Edje_External_Param_Flags
{
   EDJE_EXTERNAL_PARAM_FLAGS_NONE        = 0, /**< Property is incapable of operations, this is used to catch bogus flags. */
   EDJE_EXTERNAL_PARAM_FLAGS_GET         = (1 << 0), /**< Property can be read/get. */
   EDJE_EXTERNAL_PARAM_FLAGS_SET         = (1 << 1), /**< Property can be written/set. This only enables edje_object_part_external_param_set() and Embryo scripts. To enable the parameter being set from state description whenever it changes state, use #EDJE_EXTERNAL_PARAM_FLAGS_STATE. */
   EDJE_EXTERNAL_PARAM_FLAGS_STATE       = (1 << 2), /**< Property can be set from state description. */
   EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR = (1 << 3), /**< This property is only set once when the object is constructed using its value from "default" 0.0 state description. Setting this overrides #EDJE_EXTERNAL_PARAM_FLAGS_STATE. */
   EDJE_EXTERNAL_PARAM_FLAGS_REGULAR     = (EDJE_EXTERNAL_PARAM_FLAGS_GET |
                                            EDJE_EXTERNAL_PARAM_FLAGS_SET |
                                            EDJE_EXTERNAL_PARAM_FLAGS_STATE) /**< Convenience flag that sets property as GET, SET and STATE. */
} Edje_External_Param_Flags;

/** Struct that holds parameters for parts of type EXTERNAL.
 */
typedef struct _Edje_External_Param
{
  const char *name; /**< The name of the parameter. */
  Edje_External_Param_Type type; /**< The type of the parameter. This defines
                                  * which of the next three variables holds the
                                  * value for it. */
  int i; /**< Used by both integer and boolean */
  double d; /**< Used by double */
  const char *s; /**< Used by both string and choice */
} Edje_External_Param;

/**
 * @brief Converts type identifier to string nicer representation.
 *
 * This may be used to debug or other informational purposes.
 *
 * @param type the identifier to convert.
 * @return the string with the string representation, or @c "(unknown)".
 */
EAPI const char *edje_external_param_type_str(Edje_External_Param_Type type) EINA_PURE;

/**
 * Helper macro to indicate an EXTERNAL's integer parameter is undefined.
 */
#define EDJE_EXTERNAL_INT_UNSET INT_MAX
/**
 * Helper macro to indicate an EXTERNAL's double parameter is undefined.
 */
#define EDJE_EXTERNAL_DOUBLE_UNSET DBL_MAX

/**
 * Struct holding information about an EXTERNAL part's parameters.
 *
 * When creating types to use with EXTERNAL parts, an array of this type is
 * used to describe the different parameters the object uses.
 *
 * This struct holds the name, type and flags that define how and when the
 * parameter is used, as well as information specific to each type, like the
 * maximum or minimum value, that can be used by editors to restrict the
 * range of values to set for each parameter.
 */
typedef struct _Edje_External_Param_Info Edje_External_Param_Info;

/**
 * Struct holding information about an EXTERNAL part's parameters.
 *
 * When creating types to use with EXTERNAL parts, an array of this type is
 * used to describe the different parameters the object uses.
 *
 * This struct holds the name, type and flags that define how and when the
 * parameter is used, as well as information specific to each type, like the
 * maximum or minimum value, that can be used by editors to restrict the
 * range of values to set for each parameter.
 */
struct _Edje_External_Param_Info
{
   const char               *name; /**< Name of the parameter. */
   Edje_External_Param_Type  type; /**< Type of the parameter. */
   Edje_External_Param_Flags flags; /**< Flags indicating how this parameter is
                                      used. */
   union {
      struct {
         int                 def, /**< Default value for the parameter. */
                             min, /**< Minimum value it can have. */
                             max, /**< Maximum value it can have. */
                             step; /**< Values will be a multiple of this. */
      } i; /**< Info about integer type parameters. Use #EDJE_EXTERNAL_INT_UNSET
             on any of them to indicate they are not defined.*/
      struct {
         double              def, /**< Default value for the parameter. */
                             min, /**< Minimum value it can have. */
                             max, /**< Maximum value it can have. */
                             step; /**< Values will be a multiple of this. */
      } d; /**< Info about double type parameters. Use
#EDJE_EXTERNAL_DOUBLE_UNSET on any of them to indicate they are not defined.*/
      struct {
         const char         *def; /**< Default value. */
         const char         *accept_fmt; /**< Not implemented. */
         const char         *deny_fmt; /**< Not implemented */
      } s; /**< Info about string type parameters. NULL indicates undefined. */
      struct {
         int                 def; /**< Default value. */
         const char         *false_str; /**< String shown by editors to indicate the false state. */
         const char         *true_str; /**< String shown by editors to indicate the true state. */
      } b; /**< Info about boolean type parameters.*/
      struct {
         const char         *def; /**< Default value. */
         const char        **choices; /* Array of strings, each represents a
                                         valid value for this parameter. The
                                         last element of the array must be
                                         NULL. */
         char               *(*def_get)(void *data, const Edje_External_Param_Info *info); /** return malloc() memory with the default choice, should be used if def is NULL. First parameter is Edje_External_Type::data */
         char              **(*query)(void *data, const Edje_External_Param_Info *info); /** NULL terminated array of strings, memory is dynamically allocated and should be freed with free() for array and each element. First parameter is Edje_External_Type::data */
      } c; /**< Info about choice type parameters. */
   } info;
};

#define EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, min, max, step, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_INT, flags, {.i = {def, min, max, step}}}
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, min, max, step, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, flags, {.d = {def, min, max, step}}}
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, accept, deny, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_STRING, flags, {.s = {def, accept, deny}}}
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, false_str, true_str, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_BOOL, flags, {.b = {def, false_str, true_str}}}
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL_FLAGS(name, def, choices, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_CHOICE, flags, {.c = {def, choices, NULL, NULL}}}
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL_FLAGS(name, def_get, query, flags) \
  {name, EDJE_EXTERNAL_PARAM_TYPE_CHOICE, flags, {.c = {NULL, NULL, def_get, query}}}

#define EDJE_EXTERNAL_PARAM_INFO_INT_FULL(name, def, min, max, step) \
  EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, min, max, step, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL(name, def, min, max, step) \
  EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, min, max, step, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FULL(name, def, accept, deny) \
  EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, accept, deny, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL(name, def, false_str, true_str) \
  EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, false_str, true_str, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL(name, def, choices) \
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL_FLAGS(name, def, choices, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)
#define EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL(name, def_get, query) \
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_DYNAMIC_FULL_FLAGS(name, def_get, query, EDJE_EXTERNAL_PARAM_FLAGS_REGULAR)

#define EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_INT_FULL(name, def, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL(name, def, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_FULL(name, def, NULL, NULL)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT(name, def) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL(name, def, "false", "true")

#define EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT_FLAGS(name, def, flags)    \
  EDJE_EXTERNAL_PARAM_INFO_INT_FULL_FLAGS(name, def, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, EDJE_EXTERNAL_INT_UNSET, flags)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT_FLAGS(name, def, flags) \
  EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FULL_FLAGS(name, def, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, EDJE_EXTERNAL_DOUBLE_UNSET, flags)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT_FLAGS(name, def, flags) \
  EDJE_EXTERNAL_PARAM_INFO_STRING_FULL_FLAGS(name, def, NULL, NULL, flags)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT_FLAGS(name, def, flags)   \
  EDJE_EXTERNAL_PARAM_INFO_BOOL_FULL_FLAGS(name, def, "false", "true", flags)

#define EDJE_EXTERNAL_PARAM_INFO_INT(name) \
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT(name, 0)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE(name) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT(name, 0.0)
#define EDJE_EXTERNAL_PARAM_INFO_STRING(name) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT(name, NULL)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL(name) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT(name, 0)

#define EDJE_EXTERNAL_PARAM_INFO_INT_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT_FLAGS(name, 0, flags)
#define EDJE_EXTERNAL_PARAM_INFO_DOUBLE_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT_FLAGS(name, 0.0, flags)
#define EDJE_EXTERNAL_PARAM_INFO_STRING_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT_FLAGS(name, NULL, flags)
#define EDJE_EXTERNAL_PARAM_INFO_BOOL_FLAGS(name, flags) \
   EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT_FLAGS(name, 0, flags)

#define EDJE_EXTERNAL_PARAM_INFO_SENTINEL {NULL, 0, 0, {.s = {NULL, NULL, NULL}}}

#define EDJE_EXTERNAL_TYPE_ABI_VERSION (3)

/**
 * @struct _Edje_External_Type
 *
 * @brief Information about an external type to be used.
 *
 * This structure provides information on how to display and modify a
 * third party Evas_Object in Edje.
 *
 * Some function pointers are not really used by Edje, but provide
 * means for Edje users to better interact with such objects. For
 * instance, an editor may use label_get() and icon_get() to list all
 * registered external types.
 *
 * @note The function pointers provided in this structure must check
 *       for errors and invalid or out-of-range values as for
 *       performance reasons Edje will not enforce hints provided as
 *       #Edje_External_Param_Info in the member parameters_info.
 */
struct _Edje_External_Type
{
  unsigned int  abi_version; /**< always use:
                              *  - #EDJE_EXTERNAL_TYPE_ABI_VERSION to declare.
                              *  - edje_external_type_abi_version_get() to check.
                              */
  const char    *module; /**< Name of the module that holds these definitions,
                           as used in the externals {} block of a theme
                           definition. */
  const char    *module_name; /**< Canonical name of the module, for displaying
                                in edition programs, for example. */
  Evas_Object *(*add) (void *data, Evas *evas, Evas_Object *parent, const Eina_List *params, const char *part_name); /**< Creates the object to be used by Edje as the part. @p part_name is the name of the part that holds the object and can be used to forward callbacks from the object as signals from Edje. @p params is the list of #Edje_External_Param, not parsed, from the default state of the part. Parameters of type #EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR should be set on
 the object here. */
  void         (*state_set) (void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos); /**< Called upon state changes, including the initial "default" 0.0 state. Parameters are the value returned by params_parse(). The @p pos parameter is a value between 0.0 and 1.0 indicating the position in time within the state transition. */
  void         (*signal_emit) (void *data, Evas_Object *obj, const char *emission, const char *source); /**< Feed a signal emitted with emission originally set as part_name:signal to this object (without the "part_name:" prefix) */
  Eina_Bool    (*param_set) (void *data, Evas_Object *obj, const Edje_External_Param *param); /**< Dynamically change a parameter of this external, called by scripts and user code. Returns @c EINA_TRUE on success */
  Eina_Bool    (*param_get) (void *data, const Evas_Object *obj, Edje_External_Param *param); /**< Dynamically fetch a parameter of this external, called by scripts and user code. Returns @c EINA_TRUE on success. (Must check parameter name and type!) */
  Evas_Object *(*content_get) (void *data, const Evas_Object *obj, const char *content); /**< Dynamically fetch a sub object of this external, called by scripts and user code. Returns @c Evas_Object * on success. (Must check parameter name and type!) */
  void        *(*params_parse) (void *data, Evas_Object *obj, const Eina_List *params); /**< Parses the list of parameters, converting into a friendly representation. Used with state_set() */
  void         (*params_free) (void *params); /**< Free parameters parsed with params_parse() */

  /* The following callbacks aren't used by Edje itself, but by UI design
     tools instead */
  const char  *(*label_get) (void *data); /**< Get a label to use to identify this EXTERNAL. (For editors) */
  const char  *(*description_get) (void *data); /**< Get a user friendly description of this EXTERNAL. (For editors) */
  Evas_Object *(*icon_add) (void *data, Evas *e); /**< Get an icon to use to identify this EXTERNAL. (For editors) */
  Evas_Object *(*preview_add) (void *data, Evas *e); /**< Get a preview of the EXTERNAL object in use. (For editors) */
  const char  *(*translate) (void *data, const char *orig); /**< called to translate parameters_info name properties for use in user interfaces that support internationalization (i18n) (For editors) */

  Edje_External_Param_Info *parameters_info; /**< An array of #Edje_External_Param_Info describing the different parameters this EXTERNAL may have. The last element in the array must be #EDJE_EXTERNAL_PARAM_INFO_SENTINEL. */
  void                     *data; /**< Private user data that will be passed to all of the class functions. */
};
typedef struct _Edje_External_Type Edje_External_Type;

/**
 * Convenience struct used to mass-register types of EXTERNAL objects.
 *
 * Used with edje_external_type_array_register().
 */
struct _Edje_External_Type_Info
{
   const char               *name; /**< The name of the type to register. */
   const Edje_External_Type *info; /**< The type definition. */
};

typedef struct _Edje_External_Type_Info Edje_External_Type_Info;

/**
 * @}
 */

/**
 * @defgroup Edje_External_Plugin_Development_Group Edje Development of External Plugins
 * @ingroup Edje_External_Group
 *
 * @brief Functions to register, unregister EXTERNAL types and develop the plugins.
 *
 * This group discusses functions useful for the development of new plugins.
 * These functions deal with the newly EXTERNAL types by registering, unregistering and manipulating them.
 *
 *
 * @{
 */

/**
 * @brief Registers a type to be used by EXTERNAL parts.
 *
 * Parts of type EXTERNAL will call user defined functions
 * to create and manipulate the object that's allocated in that part. This is
 * done by expecifying in the @c source property of the part the name of the
 * external to use, which must be one registered with this function.
 *
 * @param type_name Name to register and be known by edje's "source:"
 *        parameter of "type: EXTERNAL" parts.
 * @param type_info Meta-information describing how to interact with it.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type already registered).
 *
 * @see edje_external_type_array_register()
 */
EAPI Eina_Bool    edje_external_type_register             (const char *type_name, const Edje_External_Type *type_info);

/**
 * @brief Unregisters a previously registered EXTERNAL type.
 *
 * @param type_name name to unregister. It should have been registered with
 *        edje_external_type_register() before.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type_name did not exist).
 *
 * @see edje_external_type_array_unregister()
 */
EAPI Eina_Bool    edje_external_type_unregister           (const char *type_name);

/**
 * @brief Registers a batch of types and their information.
 *
 * When several types will be registered it is recommended to use this
 * function instead of several calls to edje_external_type_register(), as it
 * is faster.
 *
 * @note The contents of the array will be referenced directly for as long as
 * the type remains registered, so both the @c name and @c info in the
 * @p array must be kept alive during all this period (usually, the entire
 * program lifetime). The most common case would be to keep the array as a
 * @c static @c const type anyway.
 *
 * @param array @c NULL terminated array with type name and
 *        information. Note that type name or information are
 *        referenced directly, so they must be kept alive after
 *        this function returns!
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type already registered).
 *
 * @see edje_external_type_register()
 */
EAPI void         edje_external_type_array_register       (const Edje_External_Type_Info *array);

/**
 * @brief Unregisters a batch of given external type previously registered.
 *
 * @param array @c NULL terminated array, should be the same as the
 *        one used to register with edje_external_type_array_register()
 *
 * @see edje_external_type_unregister()
 */
EAPI void         edje_external_type_array_unregister     (const Edje_External_Type_Info *array);

/**
 * @brief Returns the current ABI version for Edje_External_Type structure.
 *
 * Always check this number before accessing Edje_External_Type in
 * your own software. If the number is not the same, your software may
 * access invalid memory and crash, or just get garbage values.
 *
 * @warning @b NEVER, EVER define your own Edje_External_Type using the
 *          return of this function as it will change as Edje library
 *          (libedje.so) changes, but your type definition will
 *          not. Instead, use #EDJE_EXTERNAL_TYPE_ABI_VERSION.
 *
 * Summary:
 *   - use edje_external_type_abi_version_get() to check.
 *   - use #EDJE_EXTERNAL_TYPE_ABI_VERSION to define/declare.
 *
 * @return The external ABI version the Edje library was compiled with. That
 * is, the value #EDJE_EXTERNAL_TYPE_ABI_VERSION had at that moment.
 */
EAPI unsigned int edje_external_type_abi_version_get      (void) EINA_CONST;

/**
 *
 * @return An iterator of all the registered EXTERNAL types.
 *
 * Each item in the iterator is an @c Eina_Hash_Tuple which has the type
 * of the external in the @c key and #Edje_External_Type as @c data.
 *
 * @code
 * const Eina_Hash_Tuple *tuple;
 * Eina_Iterator *itr;
 * const Eina_List *l, *modules;
 * const char *s;
 *
 * modules = edje_available_modules_get();
 * EINA_LIST_FOREACH(modules, l, s)
 *   {
 *      if (!edje_module_load(s))
 *        printf("Error loading edje module: %s\n", s);
 *   }
 *
 * itr = edje_external_iterator_get();
 * EINA_ITERATOR_FOREACH(itr, tuple)
 *   {
 *      const char *name = tuple->key;
 *      const Edje_External_Type *type = tuple->data;
 *
 *      if ((!type) ||
 *          (type->abi_version != edje_external_type_abi_version_get()))
 *        {
 *           printf("Error: invalid type %p (abi: %d, expected: %d)\n",
 *                   type, type ? type->abi_version : 0,
 *                   edje_external_type_abi_version_get());
 *           continue;
 *        }
 *
 *      printf("%s: %s (%s) label='%s' desc='%s'\n",
 *             name, type->module, type->module_name,
 *             type->label_get ? type->label_get(type->data) : "",
 *             type->description_get ? type->description_get(type->data) : "");
 *   }
 *
 * @endcode
 */
EAPI Eina_Iterator                  *edje_external_iterator_get     (void);

/**
 * @brief Convenience function to find a specific parameter in a list of them.
 *
 * @param params The list of parameters for the external
 * @param key The parameter to look for
 *
 * @return The matching #Edje_External_Param or NULL if it's not found.
 */
EAPI Edje_External_Param            *edje_external_param_find       (const Eina_List *params, const char *key);

/**
 * @brief Gets the value of the given parameter of integer type.
 *
 * Look for the @p key parameter in the @p params list and return its value in
 * @p ret. If the parameter is found and is of type
 * #EDJE_EXTERNAL_PARAM_TYPE_INT, its value will be stored in the int pointed
 * by @p ret, returning EINA_TRUE. In any other case, the function returns
 * EINA_FALSE.
 *
 * @param params List of parameters where to look
 * @param key Name of the parameter to fetch
 * @param ret Int pointer where to store the value, must not be NULL.
 *
 * @return @c EINA_TRUE if the parameter was found and is of integer type,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_int_get    (const Eina_List *params, const char *key, int *ret);

/**
 * @brief Gets the value of the given parameter of double type.
 *
 * Look for the @p key parameter in the @p params list and return its value in
 * @p ret. If the parameter is found and is of type
 * #EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, its value will be stored in the double
 * pointed by @p ret, returning EINA_TRUE. In any other case, the function
 * returns EINA_FALSE.
 *
 * @param params List of parameters where to look
 * @param key Name of the parameter to fetch
 * @param ret Double pointer where to store the value, must not be NULL.
 *
 * @return @c EINA_TRUE if the parameter was found and is of double type,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_double_get (const Eina_List *params, const char *key, double *ret);

/**
 * @brief Gets the value of the given parameter of string type.
 *
 * Look for the @p key parameter in the @p params list and return its value in
 * @p ret. If the parameter is found and is of type
 * #EDJE_EXTERNAL_PARAM_TYPE_STRING, its value will be stored in the pointer
 * pointed by @p ret, returning EINA_TRUE. In any other case, the function
 * returns EINA_FALSE.
 *
 * The string stored in @p ret must not be freed or modified.
 *
 * @param params List of parameters where to look
 * @param key Name of the parameter to fetch
 * @param ret String pointer where to store the value, must not be NULL.
 *
 * @return @c EINA_TRUE if the parameter was found and is of string type,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_string_get (const Eina_List *params, const char *key, const char **ret);

/**
 * @brief Gets the value of the given parameter of boolean type.
 *
 * Look for the @p key parameter in the @p params list and return its value in
 * @p ret. If the parameter is found and is of type
 * #EDJE_EXTERNAL_PARAM_TYPE_BOOL, its value will be stored in the Eina_Bool
 * pointed by @p ret, returning EINA_TRUE. In any other case, the function
 * returns EINA_FALSE.
 *
 * @param params List of parameters where to look
 * @param key Name of the parameter to fetch
 * @param ret Eina_Bool pointer where to store the value, must not be NULL.
 *
 * @return @c EINA_TRUE if the parameter was found and is of boolean type,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_bool_get   (const Eina_List *params, const char *key, Eina_Bool *ret);

/**
 * @brief Gets the value of the given parameter of choice type.
 *
 * Look for the @p key parameter in the @p params list and return its value in
 * @p ret. If the parameter is found and is of type
 * #EDJE_EXTERNAL_PARAM_TYPE_CHOICE, its value will be stored in the string
 * pointed by @p ret, returning EINA_TRUE. In any other case, the function
 * returns EINA_FALSE.
 *
 * The string stored in @p ret must not be freed or modified.
 *
 * @param params List of parameters where to look
 * @param key Name of the parameter to fetch
 * @param ret String pointer where to store the value, must not be NULL.
 *
 * @return EINA_TRUE if the parameter was found and is of integer type,
 * EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_choice_get (const Eina_List *params, const char *key, const char **ret);

/**
 * @brief Gets the array of parameters information about a type given its name.
 *
 * @note the type names and other strings are static, that means they are
 *       @b NOT translated. One must use
 *       Edje_External_Type::translate() to translate those.
 *
 * @param type_name Edje external type name
 *
 * @return the NULL terminated array, or @c NULL if type is unknown or
 *         it does not have any parameter information.
 *
 * @see edje_external_type_get()
 */
EAPI const Edje_External_Param_Info *edje_external_param_info_get   (const char *type_name);

/**
 * @brief Gets the #Edje_External_Type that defines an EXTERNAL type registered with
 * the name @p type_name.
 */
EAPI const Edje_External_Type       *edje_external_type_get         (const char *type_name);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Group Edje Object
 * @ingroup Edje
 *
 * @brief This group discusses functions that deal with Edje layouts and its components
 *
 * An important thing to know about this group is that there is no
 * Edje_Object in @b code. What we refer here as object are layouts (or themes)
 * defined by groups, and parts, both declared in EDC files. They are of
 * type Evas_Object as the other native objects of Evas, but they only exist
 * in Edje, so that is why we are calling them "edje objects".
 *
 * With the Edje Object Group functions we can deal with layouts by managing
 * its aspect, content, message and signal exchange and animation, among others.
 *
 * @{
 */
/**
 * @}
 */

/**
 * @defgroup Edje_Object_Scale Edje Scale
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with scaling objects
 *
 * Edje allows one to build scalable interfaces. Scaling factors,
 * which are set to neutral (@c 1.0) values by default (no scaling,
 * actual sizes), are of two types: @b global and @b individual.
 *
 * Scaling affects the values of minimum/maximum @b part sizes, which
 * are @b multiplied by it. Font sizes are scaled, too.
 *
 *
 * @{
 */

/**
 * @brief Sets Edje's global scaling factor.
 *
 * @param scale The global scaling factor (the default value is @c 1.0)
 *
 * Edje's global scaling factor will affect all its objects which
 * hadn't their individual scaling factors altered from the default
 * value (which is zero). If they had it set differently, by
 * edje_object_scale_set(), that factor will @b override the global
 * one.
 *
 * @warning Only parts which, at EDC level, had the @c "scale"
 * property set to @c 1, will be affected by this function. Check the
 * complete @ref edcref "syntax reference" for EDC files.
 *
 * @see edje_scale_get().
 */
EAPI void         edje_scale_set                  (double scale);

/**
 * @brief Retrieves Edje's global scaling factor.
 *
 * @return The global scaling factor.
 *
 * This function returns Edje's global scaling factor.
 *
 * @see edje_scale_set() for more details
 *
 */
EAPI double       edje_scale_get                  (void);

/**
 * @}
 */

/**
 * @defgroup Edje_Text_Entry Edje Text Entry
 * @ingroup Edje_Part_Text
 *
 * @brief Functions that deal with text entries
 *
 * In Edje it's possible to use a text part as a entry so the user is
 * able to make inputs of text. To do so, the text part must be set
 * with a input panel that will work as a virtual keyboard.
 *
 * Some of effects can be applied to the entered text and also plenty
 * actions can be performed after any input.
 *
 * Use the functions of this section to handle the user input of text.
 *
 *
 * @{
 */

struct _Edje_Entry_Change_Info
{
   union {
        struct {
             const char *content;
             size_t pos;
             size_t plain_length; /* Number of cursor positions represented
                                     in content. */
        } insert;
        struct {
             const char *content;
             size_t start, end;
        } del;
   } change;
   Eina_Bool insert : 1; /**< True if the "change" union's "insert" is valid */
   Eina_Bool merge : 1; /**< True if can be merged with the previous one. Used for example with insertion when something is already selected. */
};

/**
 * @since 1.1.0
 */
typedef struct _Edje_Entry_Change_Info        Edje_Entry_Change_Info;

/**
 * @typedef Edje_Text_Filter_Type
 *
 * All Edje text filters type values.
 */
typedef enum _Edje_Text_Filter_Type
{
   EDJE_TEXT_FILTER_TEXT = 0,   /**< Text type filter   */
   EDJE_TEXT_FILTER_FORMAT = 1, /**< Format type filter */
   EDJE_TEXT_FILTER_MARKUP = 2  /**< Markup type filter */
} Edje_Text_Filter_Type;

enum
{
   EDJE_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_NORMAL,         /**< The plain normal layout @since 1.12 */
   EDJE_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_FILENAME,       /**< Filename layout. Symbols such as '/' should be disabled. @since 1.12 */
   EDJE_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_PERSON_NAME     /**< The name of a person. @since 1.12 */
};

enum
{
   EDJE_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_NORMAL,             /**< The plain normal number layout @since 1.8 */
   EDJE_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED,             /**< The number layout to allow a positive or negative sign at the start @since 1.8 */
   EDJE_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_DECIMAL,            /**< The number layout to allow decimal point to provide fractional value @since 1.8 */
   EDJE_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL  /**< The number layout to allow decimal point and negative sign @since 1.8 */
};

enum
{
   EDJE_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NORMAL,       /**< The normal password layout @since 1.12 */
   EDJE_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NUMBERONLY    /**< The password layout to allow only number @since 1.12 */
};

/**
 * @typedef (*Edje_Text_Filter_Cb)
 * @brief Callback prototype for Edje_Text_Filter
 * @param obj The Evas_Object to filter.
 * @param type The filter type.
 * @param text The text of the filter.
 */
typedef void         (*Edje_Text_Filter_Cb)     (void *data, Evas_Object *obj, const char *part, Edje_Text_Filter_Type type, char **text);

/**
 * @typedef (*Edje_Markup_Filter_Cb)
 * @brief Callback prototype for Edje_Text_Filter
 * @param obj The Evas_Object to filter.
 * @param part Edje part name.
 * @param text The text of the filter.
 */
typedef void         (*Edje_Markup_Filter_Cb)   (void *data, Evas_Object *obj, const char *part, char **text);

/**
 * @typedef (*Edje_Item_Provider_Cb)
 * @brief Callback prototype for Edje_Item_Provider
 * @param data some data provided by user
 * @param obj The Evas_Object to filter.
 * @param part Edje part name
 * @param item Item of container
 * @return Must be an Evas_Object.
 */
typedef Evas_Object *(*Edje_Item_Provider_Cb)   (void *data, Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Shows last character in password mode.
 *
 * @param password_show_last If TRUE enable last character show in password mode.
 *
 * This function enables last input to be visible when in password mode for few seconds
 * or until the next input is entered.
 *
 * The time out value is obtained by edje_password_show_last_timeout_set function.
 *
 * @see edje_password_show_last_timeout_set().
 */
EAPI void edje_password_show_last_set(Eina_Bool password_show_last);

/**
 * @brief Sets the timeout value in last show password mode.
 *
 * @param password_show_last_timeout The timeout value.
 *
 * This functions sets the time out value for which the last input entered in password
 * mode will be visible. If the time out value is less than zero, the last input entered in password mode will be always visible.
 *
 * This value can be used only when last show mode is set in password mode.
 *
 * @see edje_password_show_last_set().
 *
 */
EAPI void edje_password_show_last_timeout_set(double password_show_last_timeout);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Color_Class Edje Class: Color
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with Color Classes
 *
 * Sometimes we want to change the color of two or more parts equally and
 * that's when we use color classes.
 *
 * If one or more parts are assigned with a color class, when we set color
 * values to this class it will cause all these parts to have their colors
 * multiplied by the values. Setting values to a color class at a process level
 * will affect all parts with that color class, while at a object level will
 * affect only the parts inside an specified object.
 *
 * @{
 */

/**
 * @brief Sets Edje color class.
 *
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * This function sets the color values for a process level color
 * class.  This will cause all edje parts in the current process that
 * have the specified color class to have their colors multiplied by
 * these values.  (Object level color classes set by
 * edje_object_color_class_set() will override the values set by this
 * function).
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * Setting color emits a signal "color_class,set" with source being
 * the given color class in all objects.
 *
 * @see edje_color_class_set().
 *
 * @note Unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 *
 * @return Eina_Bool, @c EINA_TRUE on success and @c EINA_FALSE on failure.
 */
EAPI Eina_Bool    edje_color_class_set            (const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Gets Edje color class.
 *
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * @return @c EINA_TRUE if found or @c EINA_FALSE if not found and all
 *         values are zeroed.
 *
 * This function gets the color values for a process level color
 * class. This value is the globally set and not per-object, that is,
 * the value that would be used by objects if they did not override with
 * edje_object_color_class_set().
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * @see edje_color_class_set().
 *
 * @note Unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_color_class_get            (const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Deletes edje color class.
 *
 * @param color_class
 *
 * This function deletes any values at the process level for the
 * specified color class.
 * @note Deleting the color class will revert it to the
 *       values defined in the theme file.
 *
 * Deleting the color class will emit the signal "color_class,del"
 * to all the Edje objects in the running program.
 */
EAPI void         edje_color_class_del            (const char *color_class);

/**
 * @brief Lists color classes.
 *
 * @return A list of color class names (strings). These strings and
 * the list must be free()'d by the caller.
 *
 * This function lists all color classes known about by the current
 * process.
 *
 */
EAPI Eina_List   *edje_color_class_list           (void);

/**
 * @brief Iterates over all the active class of an application.
 *
 * @return an iterator of Edje_Color_Class of the currently active color class
 *
 * This function only iterate over the Edje_Color_Class in use by
 * an application.
 *
 * @since 1.14
 */
EAPI Eina_Iterator *edje_color_class_active_iterator_new(void);

/**
 * @brief Iterates over all the color class provided by an Edje file.
 *
 * @return an iterator of Edje_Color_Class provided by the Edje file.
 *
 * @since 1.14
 */
EAPI Eina_Iterator *edje_mmap_color_class_iterator_new(Eina_File *f);


/**
 * @}
 */

/**
 * @defgroup Edje_Object_Part Edje Part
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with layout components
 *
 * Parts are layout components, but as a layout, they are objects too.
 *
 * There are several types of parts, these types can be divided into two
 * main categories, the first being containers. Containers are parts
 * that are in effect a group of elements. The second group is that of
 * the elements, these part types may not contain others.
 *
 * This section has some functions specific for some types and others that
 * could be applied to any type.
 *
 * @{
 */

/**
 * @typedef Edje_Part_Type
 *
 * All possible "part" types in Edje.
 */
typedef enum _Edje_Part_Type
{
   EDJE_PART_TYPE_NONE      = 0,  /**< None type value */
   EDJE_PART_TYPE_RECTANGLE = 1,  /**< Rectangle type value */
   EDJE_PART_TYPE_TEXT      = 2,  /**< Text type value */
   EDJE_PART_TYPE_IMAGE     = 3,  /**< Image type value */
   EDJE_PART_TYPE_SWALLOW   = 4,  /**< Swallow  type value */
   EDJE_PART_TYPE_TEXTBLOCK = 5,  /**< Text block type value */
   EDJE_PART_TYPE_GRADIENT  = 6,  /**< Gradient type value */
   EDJE_PART_TYPE_GROUP     = 7,  /**< Group type value */
   EDJE_PART_TYPE_BOX       = 8,  /**< Box type value */
   EDJE_PART_TYPE_TABLE     = 9,  /**< Table type value */
   EDJE_PART_TYPE_EXTERNAL  = 10, /**< External type value */
   EDJE_PART_TYPE_PROXY     = 11, /**< Proxy type value */
   EDJE_PART_TYPE_SPACER    = 12, /**< Spacer type value @since 1.7 */
   EDJE_PART_TYPE_MESH_NODE = 13,
   EDJE_PART_TYPE_LIGHT     = 14,
   EDJE_PART_TYPE_CAMERA    = 15,
   EDJE_PART_TYPE_SNAPSHOT  = 16, /**< Snapshot @since 1.16 */
   EDJE_PART_TYPE_VECTOR    = 17, /**< Vector @since 1.18 */
   EDJE_PART_TYPE_LAST      = 18  /**< Last type value */
} Edje_Part_Type;
/**
 * @}
 */

/**
 * @defgroup Edje_Part_Text Edje Text Part
 * @ingroup Edje_Object_Part
 *
 * @brief Functions that deal with parts of type text
 *
 * Text is an element type for parts. It's basic functionality is to
 * display a string on the layout, but a lot more things can be done
 * with texts, like string selection, setting the cursor and include
 * a input panel, where one can set a virtual keyboard to handle
 * keyboard entry easily.
 *
 * @{
 */

#define EDJE_TEXT_EFFECT_MASK_BASIC 0xf
#define EDJE_TEXT_EFFECT_BASIC_SET(x, s) \
   do { x = ((x) & ~EDJE_TEXT_EFFECT_MASK_BASIC) | (s); } while (0)

#define EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION (0x7 << 4)
#define EDJE_TEXT_EFFECT_SHADOW_DIRECTION_SET(x, s) \
   do { x = ((x) & ~EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION) | (s); } while (0)

/**
 * @typedef Edje_Text_Effect
 *
 * All possible text effects in Edje.
 */
typedef enum _Edje_Text_Effect
{
   EDJE_TEXT_EFFECT_NONE                = 0,  /**< None text effect value */
   EDJE_TEXT_EFFECT_PLAIN               = 1,  /**< Plain text effect value */
   EDJE_TEXT_EFFECT_OUTLINE             = 2,  /**< Outline text effect value */
   EDJE_TEXT_EFFECT_SOFT_OUTLINE        = 3,  /**< Soft outline text effect value */
   EDJE_TEXT_EFFECT_SHADOW              = 4,  /**< Shadown text effect value */
   EDJE_TEXT_EFFECT_SOFT_SHADOW         = 5,  /**< Soft shadow text effect value */
   EDJE_TEXT_EFFECT_OUTLINE_SHADOW      = 6,  /**< Outline shadow text effect value */
   EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW = 7,  /**< Outline soft shadow text effect value */
   EDJE_TEXT_EFFECT_FAR_SHADOW          = 8,  /**< Far shadow text effect value */
   EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW     = 9,  /**< Far soft shadow text effect value */
   EDJE_TEXT_EFFECT_GLOW                = 10, /**< Glow text effect value */

   EDJE_TEXT_EFFECT_LAST                = 11, /**< Last text effect value */

   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_RIGHT = (0x0 << 4), /**< Bottom right shadow direction value */
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM       = (0x1 << 4), /**< Bottom shadow direction value */
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_LEFT  = (0x2 << 4), /**< Bottom left shadow direction value */
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_LEFT         = (0x3 << 4), /**< Left shadow direction value */
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_LEFT     = (0x4 << 4), /**< Top left shadow direction value */
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP          = (0x5 << 4), /**< Top shadow direction value */
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_RIGHT    = (0x6 << 4), /**< Top right shadow direction value */
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_RIGHT        = (0x7 << 4)  /**< right shadow direction value */
} Edje_Text_Effect;

/**
 * @typedef (*Edje_Text_Change_Cb)
 *
 * Callback prototype for Edje_Text_Change.
 * @param data User provided data to pass to the callback
 * @param obj The Evas_Object
 * @param part The edje part
 */
typedef void         (*Edje_Text_Change_Cb)     (void *data, Evas_Object *obj, const char *part);
/**
 * @}
 */

/**
 * @defgroup Edje_Text_Selection Edje Text Selection
 * @ingroup Edje_Part_Text
 *
 * @brief Functions that deal with selection in text parts.
 *
 * Selection is a known functionality for texts in the whole computational
 * world. It is a block of text marked for further manipulation.
 *
 * Edje is responsible for handling this functionality through the
 * following functions.
 *
 * @{
 */
/**
 * @}
 */

/**
 * @defgroup Edje_Text_Cursor Edje Text Cursor
 * @ingroup Edje_Part_Text
 *
 * @brief Functions that deal with cursor in text parts.
 *
 * Cursor is a known functionality for texts in the whole computational
 * world. It marks a position in the text from where one may want
 * to make a insertion, deletion or selection.
 *
 * Edje is responsible for handling this functionality through the
 * following functions.
 *
 * @{
 */

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Swallow Edje Swallow Part
 * @ingroup Edje_Object_Part
 *
 * @brief Functions that deal with parts of type swallow and swallowed objects.
 *
 * A important feature of Edje is to be able to create Evas_Objects
 * in code and place them in a layout. And that is what swallowing
 * is all about.
 *
 * Swallow parts are place holders defined in the EDC file for
 * objects that one may want to include in the layout later, or for
 * objects that are not native of Edje. In this last case, Edje will
 * only treat the Evas_Object properties of the swallowed objects.
 *
 *
 * @{
 */

/**
 * @typedef Edje_Aspect_Control
 *
 * All Edje aspect control values.
 *
 */
typedef enum _Edje_Aspect_Control
{
   EDJE_ASPECT_CONTROL_NONE = 0,       /*< None aspect control value       */
   EDJE_ASPECT_CONTROL_NEITHER = 1,    /*< Neither aspect control value    */
   EDJE_ASPECT_CONTROL_HORIZONTAL = 2, /*< Horizontal aspect control value */
   EDJE_ASPECT_CONTROL_VERTICAL = 3,   /*< Vertical aspect control value   */
   EDJE_ASPECT_CONTROL_BOTH = 4        /*< Both aspect control value       */
} Edje_Aspect_Control;

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Geometry_Group Edje Object Geometry
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with object's geometry.
 *
 * By geometry we mean size and position. So in this groups there are
 * functions to manipulate object's geometry or retrieve information
 * about it.
 *
 * Keep in mind that by changing an object's geometry, it may affect
 * the appearance in the screen of the parts inside. Most times
 * that is what you want.
 *
 * @{
 */
/**
 * @}
 */

/**
 * @defgroup Edje_Part_Box Edje Box Part
 * @ingroup Edje_Object_Part
 *
 * @brief Functions that deal with parts of type box.
 *
 * Box is a container type for parts, that means it can contain
 * other parts.
 *
 * @{
 */

/**
 * @brief Registers a custom layout to be used in edje boxes.
 *
 * @param name The name of the layout
 * @param func The function defining the layout
 * @param layout_data_get This function gets the custom data pointer
 * for func
 * @param layout_data_free Passed to func to free its private data
 * when needed
 * @param free_data Frees data
 * @param data Private pointer passed to layout_data_get
 *
 * This function registers custom layouts that can be referred from
 * themes by the registered name. The Evas_Object_Box_Layout
 * functions receive two pointers for internal use, one being private
 * data, and the other the function to free that data when it's not
 * longer needed. From Edje, this private data will be retrieved by
 * calling layout_data_get, and layout_data_free will be the free
 * function passed to func. layout_data_get will be called with data
 * as its parameter, and this one will be freed by free_data whenever
 * the layout is unregistered from Edje.
 */
EAPI void         edje_box_layout_register        (const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Table Edje Table Part
 * @ingroup Edje_Object_Part
 *
 * @brief Functions that deal with parts of type table.
 *
 * Table is a container type for parts, that means it can contain
 * other parts.
 *
 * @{
 */

/**
 * @typedef Edje_Object_Table_Homogeneous_Mode
 *
 * Table homogeneous modes.
 *
 */
typedef enum _Edje_Object_Table_Homogeneous_Mode
{
   EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,  /*< None homogeneous mode  */
   EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1, /*< Table homogeneous mode */
   EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2   /*< Item homogeneous mode  */
} Edje_Object_Table_Homogeneous_Mode;

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Text_Class Edje Class: Text
 * @ingroup Edje_Object_Group
 * @brief Functions that deal with Text Classes.
 *
 * Sometimes we want to change the text of two or more parts equally and
 * that's when we use text classes.
 *
 * If one or more parts are assigned with a text class, when we set font
 * attributes to this class will update all these parts with the new font
 * attributes. Setting values to a text class at a process level will affect
 * all parts with that text class, while at object level will affect only
 * the parts inside an specified object.
 *
 * @{
 */

/**
 * @brief Sets the Edje text class.
 *
 * @param text_class The text class name
 * @param font The font name
 * @param size The font size
 *
 * @return @c EINA_TRUE on success, or @c EINA_FALSE on error
 *
 * This function updates all Edje members at the process level which
 * belong to this text class with the new font attributes.
 * If the @p size is 0 then the font size will be kept with the previous size.
 * If the @p size is less then 0 then the font size will be calculated in the
 * percentage. For example, if the @p size is -50, then the font size will be
 * scaled to half of the original size and if the @p size is -10 then the font
 * size will be scaled as much as 0.1x.
 *
 * @see edje_text_class_get().
 *
 */
EAPI Eina_Bool    edje_text_class_set             (const char *text_class, const char *font, Evas_Font_Size size);

/**
 * @brief Gets the font and the font size from Edje text class.
 *
 * @param text_class The text class name
 * @param font The font name
 * @param size The font size
 *
 * @return @c EINA_TRUE on success, or @c EINA_FALSE on error
 *
 * This function gets the font and the font name from the specified Edje
 * text class. The font string will only be valid until the text class is
 * changed or edje is shut down.
 * @see edje_text_class_set().
 *
 * @since 1.14
 */
EAPI Eina_Bool    edje_text_class_get             (const char *text_class, const char **font, Evas_Font_Size *size);

/**
 * @brief Deletes the text class.
 *
 * @param text_class The text class name string
 *
 * This function deletes any values at the process level for the
 * specified text class.
 *
 */
EAPI void         edje_text_class_del             (const char *text_class);

/**
 * @brief Lists text classes.
 *
 * @return A list of text class names (strings). These strings are
 * stringshares and the list must be free()'d by the caller.
 *
 * This function lists all text classes known about by the current
 * process.
 *
 */
EAPI Eina_List   *edje_text_class_list            (void);

/**
 * @brief Iterate over all active classes of an application.
 *
 * @return an iterator of Edje_Text_Class of the currently active text class
 *
 * This function only iterates over the Edje_Text_Class in use by
 * an application.
 *
 * @since 1.17
 *
 */
EAPI Eina_Iterator *edje_text_class_active_iterator_new(void);

/**
 * @brief Iterate over all text classes provided by an Edje file.
 *
 * @return an iterator of Edje_Text_Class provided by the Edje file.
 *
 * @since 1.17
 *
 */
EAPI Eina_Iterator *edje_mmap_text_class_iterator_new(Eina_File *f);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Size_Class Edje Class: Size
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with Size Classes
 *
 * Sometimes we want to change the size of two or more parts equally and
 * that's when we use size classes.
 *
 * If one or more parts are assigned with a size class, when we set attributes
 * (minw etc.) to this class will update all these parts with the new attributes.
 * Setting values to a size class at a process level will affect
 * all parts with that size class, while at object level will affect only
 * the parts inside an specified object.
 *
 * @{
 */

/**
 * @brief Sets the Edje size class.
 *
 * @param size_class The size class name
 * @param minw The min width
 * @param minh The min height
 * @param maxw The max width
 * @param maxh The max height
 *
 * @return @c EINA_TRUE on success, or @c EINA_FALSE on error
 *
 * This function updates all Edje members at the process level which
 * belong to this size class with the new min and max attributes.
 *
 * @see edje_size_class_get().
 *
 * @since 1.17
 */
EAPI Eina_Bool    edje_size_class_set             (const char *size_class, Evas_Coord minw, Evas_Coord minh, Evas_Coord maxw, Evas_Coord maxh);

/**
 * @brief Gets the Edje size class.
 *
 * @param size_class The size class name
 * @param minw The min width
 * @param minh The min height
 * @param maxw The max width
 * @param maxh The max height
 *
 * @return @c EINA_TRUE on success, or @c EINA_FALSE on error
 *
 * This function gets the min and max size from the specified Edje
 * size class.
 *
 * @since 1.17
 */
EAPI Eina_Bool    edje_size_class_get             (const char *size_class, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord *maxw, Evas_Coord *maxh);

/**
 * @brief Deletes the size class.
 *
 * @param size_class The size class name
 *
 * This function deletes any values at the process level for the
 * specified size class.
 *
 * @since 1.17
 */
EAPI void         edje_size_class_del             (const char *size_class);

/**
 * @brief Lists size classes.
 *
 * @return A list of size class names (strings). These strings are
 * stringshares and the list must be eina_stringshare_del()'ed by the caller.
 *
 * This function lists all size classes known about by the current
 * process.
 *
 * @since 1.17
 */
EAPI Eina_List   *edje_size_class_list            (void);

/**
 * @brief Iterates over all active classes of an application.
 *
 * @return An iterator of Edje_Size_Class of the currently active size class
 *
 * This function only iterates over the Edje_Size_Class in use by
 * an application.
 *
 * @since 1.17
 */
EAPI Eina_Iterator *edje_size_class_active_iterator_new(void);

/**
 * @brief Iterates over all size classes provided by an Edje file.
 *
 * @return an iterator of Edje_Size_Class provided by the Edje file.
 *
 * @since 1.17
 */
EAPI Eina_Iterator *edje_mmap_size_class_iterator_new(Eina_File *f);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_File Edje Object File
 * @ingroup Edje_Object_Group
 *
 * @brief Functions to deals with EDJ files.
 *
 * Layouts in Edje are usually called themes and they are
 * created using the EDC language. The EDC language is declarative
 * and must be compiled before being used. The output of this
 * compilation is an EDJ file, this file can be loaded by Edje,
 * and the result is a edje object.
 *
 * This groups of functions interact with these EDJ files,
 * either by loading them or retrieving information of the EDC
 * file about objects.
 *
 * @{
 */

/**
 * @brief Gets a list of groups in an edje mapped file.
 * @param f The mapped file
 *
 * @return The Eina_List of group names (char *)
 *
 * Note: the list must be freed using edje_mmap_collection_list_free()
 * when you are done with it.
 */
EAPI Eina_List        *edje_mmap_collection_list(Eina_File *f);

/**
 * @brief Frees file collection list.
 * @param lst The Eina_List of groups
 *
 * Frees the list returned by edje_mmap_collection_list().
 */
EAPI void              edje_mmap_collection_list_free(Eina_List *lst);

/**
 * @brief Determines whether a group matching glob exists in an edje mapped file.
 * @param f The mapped file
 * @param glob A glob to match on
 *
 * @return @c 1 if a match is found, @c 0 otherwise
 */
EAPI Eina_Bool         edje_mmap_group_exists(Eina_File *f, const char *glob);

/**
 * @brief Determines whether a group have 3D Scene.
 * @param f The mapped file
 * @param glob The group name
 *
 * @return @c 1 if a Scene is found is found, @c 0 otherwise
 *
 * @since 1.18
 */
EAPI Eina_Bool         edje_mmap_3d_has(Eina_File *f, const char *group);

/**
 * @brief Iterates over all the opened Edje file.
 *
 * @return an iterator of Eina_File currently opened Edje file.
 *
 * @since 1.14
 */
EAPI Eina_Iterator *edje_file_iterator_new(void);

/**
 * @brief Gets a list of groups in an edje file.
 * @param file The path to the edje file
 *
 * @return The Eina_List of group names (char *)
 * @see edje_mmap_collection_list()
 *
 * Note: the list must be freed using edje_file_collection_list_free()
 * when you are done with it.
 *
 * @see edje_mmap_group_exists()
 */
EAPI Eina_List        *edje_file_collection_list  (const char *file);

/**
 * @brief Frees file collection list.
 * @param lst The Eina_List of groups
 *
 * Frees the list returned by edje_file_collection_list().
 */
EAPI void             edje_file_collection_list_free (Eina_List *lst);

/**
 * @brief Determines whether a group matching glob exists in an edje file.
 * @param file The file path
 * @param glob A glob to match on
 *
 * @return @c 1 if a match is found, @c 0 otherwise
 */
EAPI Eina_Bool        edje_file_group_exists      (const char *file, const char *glob);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Animation Edje Object Animation
 * @ingroup Edje_Object_Group
 * @brief Functions that deal with animations.
 *
 * Edje has the ability to animate objects. One can start, stop, play,
 * pause, freeze, and thaw edje animations using the functions of this section.
 *
 * @{
 */

/**
 * @typedef Edje_Tween_Mode
 *
 * Available tween mode for edje animations.
 */
typedef enum _Edje_Tween_Mode
{
   EDJE_TWEEN_MODE_NONE              = 0,        /**< None tween mode value */
   EDJE_TWEEN_MODE_LINEAR            = 1,        /**< Linear tween mode value */
   EDJE_TWEEN_MODE_SINUSOIDAL        = 2,        /**< Sinusoidal tween mode value */
   EDJE_TWEEN_MODE_ACCELERATE        = 3,        /**< Accelerate tween mode value */
   EDJE_TWEEN_MODE_DECELERATE        = 4,        /**< Decelerate tween mode value */
   EDJE_TWEEN_MODE_ACCELERATE_FACTOR = 5,        /**< Accelerate factor tween mode value */
   EDJE_TWEEN_MODE_DECELERATE_FACTOR = 6,        /**< Decelerate factor tween mode value */
   EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR = 7,        /**< Sinusoidal factgor tween mode value */
   EDJE_TWEEN_MODE_DIVISOR_INTERP    = 8,        /**< Divisor iterp tween mode value */
   EDJE_TWEEN_MODE_BOUNCE            = 9,        /**< Bounce tween mode value */
   EDJE_TWEEN_MODE_SPRING            = 10,       /**< Spring tween mode value */
   EDJE_TWEEN_MODE_CUBIC_BEZIER      = 11,       /**< Cubic Bezier tween mode value */
   EDJE_TWEEN_MODE_LAST              = 12,       /**< Last tween mode value */
   EDJE_TWEEN_MODE_MASK              = 0xff,     /**< Mask tween mode value */
   EDJE_TWEEN_MODE_OPT_FROM_CURRENT  = (1 << 31) /**< Options from current tween mode value */
} Edje_Tween_Mode;

/**
 * @typedef Edje_Action_Type
 *
 * All actions available in Edje programs.
 *
 */
typedef enum _Edje_Action_Type
{
   EDJE_ACTION_TYPE_NONE                     = 0,  /**< None action value */
   EDJE_ACTION_TYPE_STATE_SET                = 1,  /**< State set action value */
   EDJE_ACTION_TYPE_ACTION_STOP              = 2,  /**< Action stop action value */
   EDJE_ACTION_TYPE_SIGNAL_EMIT              = 3,  /**< Signal emit action value */
   EDJE_ACTION_TYPE_DRAG_VAL_SET             = 4,  /**< Drag val set action value */
   EDJE_ACTION_TYPE_DRAG_VAL_STEP            = 5,  /**< Drag val step action value */
   EDJE_ACTION_TYPE_DRAG_VAL_PAGE            = 6,  /**< Drag val page action value */
   EDJE_ACTION_TYPE_SCRIPT                   = 7,  /**< Script action value */
   EDJE_ACTION_TYPE_FOCUS_SET                = 8,  /**< Focus set action value */
   EDJE_ACTION_TYPE_RESERVED00               = 9,  /**< Reversed do action value */
   EDJE_ACTION_TYPE_FOCUS_OBJECT             = 10, /**< Forcus object action value */
   EDJE_ACTION_TYPE_PARAM_COPY               = 11, /**< Param copy action value */
   EDJE_ACTION_TYPE_PARAM_SET                = 12, /**< Param set action value */
   EDJE_ACTION_TYPE_SOUND_SAMPLE             = 13, /**< @since 1.1 @brief Sound sample action value */
   EDJE_ACTION_TYPE_SOUND_TONE               = 14, /**< @since 1.1 @brief Sound tone action value */
   EDJE_ACTION_TYPE_PHYSICS_IMPULSE          = 15, /**< @since 1.8 @brief Physics impulse action value */
   EDJE_ACTION_TYPE_PHYSICS_TORQUE_IMPULSE   = 16, /**< @since 1.8 @brief Physics torque impulse action value */
   EDJE_ACTION_TYPE_PHYSICS_FORCE            = 17, /**< @since 1.8 @brief Physics force action value */
   EDJE_ACTION_TYPE_PHYSICS_TORQUE           = 18, /**< @since 1.8 @brief Physics torque action value */
   EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR     = 19, /**< @since 1.8 @brief Physics forces clear action value */
   EDJE_ACTION_TYPE_PHYSICS_VEL_SET          = 20, /**< @since 1.8 @brief Physics velocity set action value */
   EDJE_ACTION_TYPE_PHYSICS_ANG_VEL_SET      = 21, /**< @since 1.8 @brief Physics angle velocity set action value */
   EDJE_ACTION_TYPE_PHYSICS_STOP             = 22, /**< @since 1.8 @brief Physics stop action value */
   EDJE_ACTION_TYPE_PHYSICS_ROT_SET          = 23, /**< @since 1.8 @brief Physics rotation set action value */
   EDJE_ACTION_TYPE_VIBRATION_SAMPLE         = 24, /**< @since 1.10 @brief vibration sample action value */
   EDJE_ACTION_TYPE_MO                       = 25, /**< @since 1.15 @brief Mo action value */
   EDJE_ACTION_TYPE_LAST                     = 26  /**< Last action value */
} Edje_Action_Type;

/**
 * @brief Sets edje transitions' frame time.
 *
 * @param t The frame time, in seconds. Default value is 1/30.
 *
 * This function sets the edje built-in animations' frame time (thus,
 * affecting their resolution) by calling
 * ecore_animator_frametime_set(). This frame time can be retrieved
 * with edje_frametime_get().
 *
 * @see edje_frametime_get()
 *
 */
EAPI void         edje_frametime_set              (double t);

/**
 * @brief Gets edje transitions' frame time.
 *
 * @return The frame time, in seconds.
 *
 * This function returns the edje frame time set by
 * edje_frametime_set() or the default value 1/30.
 *
 * @see edje_frametime_set()
 *
 */
EAPI double       edje_frametime_get              (void);

/**
 * @brief Freezes Edje objects.
 *
 * This function freezes all Edje animations in the current process.
 *
 * @note: for freeze a specific object @see edje_object_freeze().
 *
 * @see edje_thaw()
 *
 */
EAPI void         edje_freeze                     (void);

/**
 * @brief Thaws Edje objects.
 *
 * This function thaws all Edje animations in the current process.
 *
 * @note for thaw a specific object @see edje_object_thaw().
 *
 * @see edje_freeze()
 *
 */
EAPI void         edje_thaw                       (void);

/**
 * @brief Sets Edje language.
 *
 * This function sets the given language.
 *
 * @note: emits signal edje,language,"locale".
 *
 * @since 1.15
 */
EAPI void         edje_language_set               (const char *locale);

/**
 * @brief Sets edje transition duration factor.
 *
 * @param scale The edje transition's duration factor (the default value is @c 1.0)
 *
 * This function sets the edje transition duration factor
 * It will affect the speed of transitions
 * which had the @c use_duration_factor property set to @1.
 * The default value of @c use_duration_factor property is @c zero,
 * but can be changed by @p "USE_DURATION_FACTOR 1" or @p "USE_DURATION_FACTOR 0"
 * as parameter of @c "TRANSITION" property at EDC level.
 * If the parameter is @p "USE_DURATION_FACTOR 0" or not mentioned about @p "USE_DURATION_FACTOR",
 * the duration of transition keeps original duration
 *
 * @warning The transition's duration factor cannot be set on each translation.
 * If you use this function, it will affect transitions globally
 *
 * @see edje_transition_duration_factor_get()
 *
 * @since 1.15
 */
EAPI void         edje_transition_duration_factor_set        (double scale);

/**
 * @brief Retrieves transitions duration factor.
 *
 * @return The edje transition duration factor
 *
 * This function returns the edje transition duration factor.
 *
 * @see edje_transition_duration_set() for more details
 *
 * @since 1.15
 *
 */
EAPI double       edje_transition_duration_factor_get                  (void);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Communication_Interface_Message Edje Communication Interface: Message
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with messages.
 *
 * Edje has two communication interfaces between @b code and @b theme.
 * Signals and messages.
 *
 * Edje messages are one of the communication interfaces between
 * @b code and a given Edje object's @b theme. With messages, one can
 * communicate values like strings, float numbers and integer
 * numbers. Moreover, messages can be identified by integer
 * numbers. See #Edje_Message_Type for the full list of message types.
 *
 * @note Messages must be handled by scripts.
 *
 * @{
 */

/**
 * @brief Processes all queued up edje messages.
 *
 * This function triggers the processing of messages addressed to any
 * (alive) edje objects.
 *
 */
EAPI void         edje_message_signal_process             (void);

/**
 * @}
 */

/**
 * @defgroup Edje_Perspective Edje Perspective
 * @ingroup Edje_Object_Group
 *
 * @brief Functions that deal with 3D projection of an 2D object.
 *
 * Perspective is a graphical tool that makes objets represented in 2D
 * look like they have a 3D appearance.
 *
 * Edje allows us to use perspective on any edje object. This group of
 * functions deal with the use of perspective, by creating and configuring
 * a perspective object that must set to a edje object or a canvas,
 * affecting all the objects inside that have no particular perspective
 * set already.
 *
 * @{
 */

/**
 * @brief Creates a new perspective in the given canvas.
 *
 * @param e The given canvas (Evas).
 * @return An @ref Edje_Perspective object for this canvas, or @c NULL on errors.
 *
 * This function creates a perspective object that can be set on an Edje
 * object, or globally to all Edje objects on this canvas.
 *
 * @see edje_perspective_set()
 * @see edje_perspective_free()
 */
EAPI Edje_Perspective       *edje_perspective_new            (Evas *e);
/**
 * @brief Deletes the given perspective object.
 *
 * @param ps A valid perspective object, or @c NULL.
 *
 * This function will delete the perspective object. If the perspective
 * effect was being applied to any Edje object or part, this effect won't be
 * applied anymore.
 *
 * @see edje_perspective_new()
 */
EAPI void                    edje_perspective_free           (Edje_Perspective *ps);
/**
 * @brief Sets up the transform for this perspective object.
 *
 * This sets the parameters of the perspective transformation. X, Y and Z
 * values are used. The px and py points specify the "infinite distance" point
 * in the 3D conversion (where all lines converge to like when artists draw
 * 3D by hand). The @p z0 value specifies the z value at which there is a 1:1
 * mapping between spatial coordinates and screen coordinates. Any points
 * on this z value will not have their X and Y values modified in the transform.
 * Those further away (Z value higher) will shrink into the distance, and
 * those less than this value will expand and become bigger. The @p foc value
 * determines the "focal length" of the camera. This is in reality the distance
 * between the camera lens plane itself (at or closer than this rendering
 * results are undefined) and the "z0" z value. This allows for some "depth"
 * control and @p foc must be greater than 0.
 *
 * @param ps The perspective object
 * @param px The perspective distance X coordinate
 * @param py The perspective distance Y coordinate
 * @param z0 The "0" z plane value
 * @param foc The focal distance
 */
EAPI void                    edje_perspective_set            (Edje_Perspective *ps, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);
/**
 * @brief Makes this perspective object be global for its canvas.
 *
 * @param ps The given perspective object
 * @param global @c EINA_TRUE if the perspective should be global, @c
 * EINA_FALSE otherwise.
 *
 * The canvas which this perspective object is being set as global is the one
 * given as argument upon the object creation (the @p evas parameter on the
 * function @c edje_perspective_new(evas) ).
 *
 * There can be only one global perspective object set per canvas, and if
 * a perspective object is set to global when there was already another
 * global perspective set, the old one will be set as non-global.
 *
 * A global perspective just affects a part if its Edje object doesn't have a
 * perspective object set to it, and if the part doesn't point to another
 * part to be used as perspective.
 *
 * @see edje_object_perspective_set()
 * @see edje_perspective_global_get()
 * @see edje_perspective_new()
 */
EAPI void                    edje_perspective_global_set     (Edje_Perspective *ps, Eina_Bool global);
/**
 * @brief Gets whether the given perspective object is global or not.
 *
 * @param ps The given perspective object.
 * @return @c EINA_TRUE if this perspective object is global, @c EINA_FALSE
 * otherwise.
 *
 * @see edje_perspective_global_set()
 */
EAPI Eina_Bool               edje_perspective_global_get     (const Edje_Perspective *ps);
/**
 * @brief Gets the global perspective object set for this canvas.
 *
 * @param e The given canvas (Evas).
 * @return The perspective object set as global for this canvas. Or @c NULL
 * if there is no global perspective set and on errors.
 *
 * This function will return the perspective object that was set as global
 * with edje_perspective_global_set().
 *
 * @see edje_perspective_global_set()
 * @see edje_perspective_global_get()
 */
EAPI const Edje_Perspective *edje_evas_global_perspective_get(const Evas *e);
/**
 * @}
 */

/**
 * @defgroup Edje_Audio Edje Audio
 * @ingroup Edje
 *
 * @brief Functions to manipulate audio abilities in edje.
 *
 * Perspective is a graphical tool that makes objets represented in 2D
 * look like they have a 3D appearance.
 *
 * Edje allows us to use perspective on any edje object. This group of
 * functions deal with the use of perspective, by creating and configuring
 * a perspective object that must set to a edje object or a canvas,
 * affecting all the objects inside that have no particular perspective
 * set already.
 *
 * @{
 */

/**
 * Identifiers of Edje message types, which can be sent back and forth
 * code and a given Edje object's theme file/group.
 *
 * @see edje_audio_channel_mute_set()
 * @see edje_audio_channel_mute_get()
 *
 * @since 1.9
 */
typedef enum _Edje_Channel
{
   EDJE_CHANNEL_EFFECT = 0, /**< Standard audio effects */
   EDJE_CHANNEL_BACKGROUND = 1, /**< Background audio sounds  */
   EDJE_CHANNEL_MUSIC = 2, /**< Music audio */
   EDJE_CHANNEL_FOREGROUND = 3, /**< Foreground audio sounds */
   EDJE_CHANNEL_INTERFACE = 4, /**< Sounds related to the interface */
   EDJE_CHANNEL_INPUT = 5, /**< Sounds related to regular input */
   EDJE_CHANNEL_ALERT = 6, /**< Sounds for major alerts */
   EDJE_CHANNEL_ALL = 7 /**< All audio channels (convenience) */
} Edje_Channel;

/**
 * @brief Sets the mute state of audio for the process as a whole.
 *
 * @param channel The channel to set the mute state of
 * @param mute The mute state
 *
 * This sets the mute (no output) state of audio for the given channel.
 *
 * @see edje_audio_channel_mute_get()
 *
 * @since 1.9
 */
EAPI void edje_audio_channel_mute_set(Edje_Channel channel, Eina_Bool mute);

/**
 * @brief Gets the mute state of the given channel.
 *
 * @param channel The channel to get the mute state of
 * @return The mute state of the channel
 *
 * @see edje_audio_channel_mute_set()
 *
 * @since 1.9
 */
EAPI Eina_Bool edje_audio_channel_mute_get(Edje_Channel channel);

/**
 * @brief Gets the part name of an edje part object.
 * @param obj An edje part object
 * @return The name of the part, if the object is an edje part, or @c NULL
 * @note If this function returns @c NULL, @p obj was not an Edje part object
 * @see edje_object_part_object_get()
 * @since 1.10
 */
EAPI const char *edje_object_part_object_name_get(const Evas_Object *obj);

#ifdef EFL_BETA_API_SUPPORT

/**
 * @brief Creates scene and root node which contains all 3D parts of edje object.
 * @param obj An edje part object
 * @param root node to collect all 3D parts
 * @param scene
 * @return scene and root node which contains all 3D parts of edje object
 * @note If this function returns @c EINA_FALSE, @p the scene or the root
 * node wasn't made
 * @since 1.18
 */
EAPI Eina_Bool edje_3d_object_add(Evas_Object *obj, Eo **root_node, Eo *scene);

#endif

/**
 * @}
 */
