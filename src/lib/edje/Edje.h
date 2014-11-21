/**
@defgroup Edje_Group Edje
@ingroup EFL_Group

@brief Edje Graphical Design Library

These routines are used for Edje.

See @ref edje_main for more details.

@page edje_main Edje

@date 2003 (created)

@section toc Table of Contents

@li @ref edje_main_intro
@li @ref edje_main_work
@li @ref edje_main_compiling
@li @ref edje_main_next_steps

@section edje_main_intro Introduction

Edje is a complex graphical design & layout library.

It doesn't intend to do a containing and regular layout like a widget
set, but it is the base for such components. Based on the requirements
of Enlightenment 0.17, Edje should serve all the purposes of creating
visual elements (borders of windows, buttons, scrollbars, etc.) and
allow the designer the ability to animate, layout, and control the look
and feel of any program using Edje as its basic GUI constructor. This
library allows for multiple collections of Layouts in one file,
sharing the same image and font database and thus allowing a whole
theme to be conveniently packaged into 1 file and shipped around.

Edje separates the layout and behavior logic. Edje files ship with an
image and font database that is used by all the parts of all the collections
to source graphical data. It has a directory of logical part names
pointing to the part collection entry ID in the file (thus allowing
for multiple logical names to point to the same part collection,
allowing for the sharing of data between display elements). Each part
collection consists of a list of visual parts, as well as a list of
programs. A program is a conditionally run program such that if a
particular event occurs (a button is pressed, a mouse enters or leaves
a part) it triggers an action that may affect other parts. In this
way a part collection can be "programmed" via its file to highlight
buttons when the mouse passes over them or show hidden parts when a
button is clicked somewhere, and so on. The actions performed in changing
from one state to another are also allowed to transition over a period
of time, allowing animation. Programs and animations can be run in
"parallel".

This separation and simplistic event driven style of programming can produce
almost any look and feel that one could want for basic visual elements. Anything
more complex is mostly the domain of an application or a widget set that may
use Edje as a convenient way to able to configure parts of the display.

For details on Edje's history, see the @ref edje_history section.

@subsection edje_main_work So how does this all work?

Edje internally holds a geometry state machine and state graph of what is
visible or not, where it is visible, at what size, with what colors, and so on. This is described
to Edje from an Edje .edj file containing this information. These files can
be produced by using edje_cc to take a text file (a .edc file) and "compile"
an output .edj file that contains this information, images, and any other
data that is needed.

The application using Edje then creates an object in its Evas
canvas and sets the bundle file to use, specifying the @b group name to
use. Edje loads such information and creates all the required
children objects with the specified properties as defined in each @b
part of the given group.

Before going into the depth of changing or creating your own edje source (edc)
files, read @ref edcref.

@subsection edje_history Edje History

It's a sequel to "Ebits" which has serviced the needs of Enlightenment
development for early version 0.17. The original design parameters under
which Ebits is built are a lot more restricted than the resulting
use of them, thus Edje is born.

Edje is a more complex layout engine compared to Ebits. It doesn't
pretend to do a containing and regular layout like a widget set. It
still inherits the more simplistic layout ideas behind Ebits, but it
now does them a lot more cleanly, allowing for easy expansion, and the
ability to cover much more ground than Ebits could. For the
purposes of Enlightenment 0.17, Edje is conceived to serve all the
purposes of creating visual elements (borders of windows, buttons,
scrollbars, etc.) and allow the designer the ability to animate,
layout, and control the look and feel of any program using Edje as its
basic GUI constructor.

Unlike Ebits, Edje separates the layout and behavior logic.

@section edje_main_compiling How to compile

@section edje_main_next_steps Next Steps

After you understood what Edje is and installed it in your system you
should proceed understanding the programming interface for all
objects, then see the specific for the most used elements. We'd
recommend you to take a while to learn @ref Ecore_Group, @ref Evas,
and @ref Eina_Group as they are the building blocks for Edje. There is a
widget set built on top of Edje providing high level elements such as
buttons, lists and selectors called Elementary
(http://docs.enlightenment.org/auto/elementary/) as they will likely
save you tons of work compared to using just Evas directly.

Recommended reading:
@li @ref edcref
@internal
@li @ref Edje_General_Group
@endinternal
@li @ref Edje_Object_Group
@internal
@li @ref Edje_External_Group
@endinternal
@li @ref luaref

*/

/**
 * @page edcref
 * See http://docs.enlightenment.org/auto/edje/edcref.html
 */

/**
 * @page luaref
 * See http://docs.enlightenment.org/auto/edje/luaref.html
 */

/**
 * @file Edje.h
 * @brief Edje Graphical Design Library
 *
 * These routines are used for Edje.
 */

/**
 * @internal
 * @defgroup Edje_General_Group Edje General
 * @ingroup Edje_Group
 *
 * @brief This group discusses the functions that have general purposes or affect Edje as a whole.
 *
 * Besides containing the initialize and shutdown functions of the library, which should
 * always be called when we are using Edje, this module contains some other utilities that
 * could be used in many contexts or should do their jobs independent of the context inside Edje.
 *
 * @{
 */

#ifndef _EDJE_H
#define _EDJE_H

#ifndef _MSC_VER
# include <stdint.h>
#endif
#include <math.h>
#include <float.h>
#include <limits.h>

#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef HAVE_ECORE_IMF
#include <Ecore_IMF.h>
#endif

#ifdef _WIN32
# ifdef EFL_EDJE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EDJE_BUILD */
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

#define EDJE_VERSION_MAJOR 1
#define EDJE_VERSION_MINOR 8

typedef struct _Edje_Version
{
   int major;
   int minor;
   int micro;
   int revision;
} Edje_Version;

EAPI extern Edje_Version *edje_version;

/**
 * @brief Initializes the edje library.
 *
 * @details This function initializes the Ejde library, making proper calls
 *          to internal initialization functions. It also initializes its
 *          @b dependencies, making calls to @c eina_init(), @c ecore_init(),
 *          @c embryo_init(), and @c eet_init(). So there is no need to call
 *          those functions again, in your code. To shutdown Edje you can call the
 *          function edje_shutdown().
 *
 * @return The new init count \n
 *         The initial value is zero.
 *
 * @see edje_shutdown()
 * @see eina_init()
 * @see ecore_init()
 * @see embryo_init()
 * @see eet_init()
 */
EAPI int          edje_init                       (void);

/**
 * @brief Shutsdown the edje library.
 *
 * @details This function shuts down the edje library. It also calls the
 *          shutdown functions of its @b dependencies, which are @c
 *          eina_shutdown(), @c ecore_shutdown(), @c embryo_shutdown(), and @c
 *          eet_shutdown(), so there is no need to call these functions again,
 *          in your code.
 *
 * @return The number of times the library has been initialised
 *         without being shutdown
 *
 * @see edje_init()
 * @see eina_shutdown()
 * @see ecore_shutdown()
 * @see embryo_shutdown()
 * @see eet_shutdown()
 *
 */
EAPI int          edje_shutdown          (void);

/**
 * @brief Sets the edje append fontset.
 *
 * @details This function sets the edje append fontset.
 *
 * @param[in] fonts The fontset to append
 *
 */
EAPI void          edje_fontset_append_set          (const char *fonts);

/**
 * @brief Gets the edje append fontset.
 *
 * @details This function returns the edje append fontset set by the
 *          edje_fontset_append_set() function.
 *
 * @return The edje append fontset
 *
 * @see edje_fontset_append_set()
 *
 */
EAPI const char          *edje_fontset_append_get          (void);

/**
 * @brief Loads a new module in Edje.
 *
 * @remarks Modules are used to add functionality to Edje.
 *          So when a module is loaded, its functionality should be available for use.
 *
 * @param[in] module The name of the module that is added to Edje
 * @return #EINA_TRUE if the module is successfully loaded, 
 *         otherwise #EINA_FALSE
 *
 */
EAPI Eina_Bool               edje_module_load                (const char *module);

/**
 * @brief Retrieves all modules that can be loaded.
 *
 * @details This function retrieves all the modules that can be loaded by edje_module_load().
 *
 * @return A list of all the loadable modules
 *
 * @see edje_module_load()
 *
 */
EAPI const Eina_List        *edje_available_modules_get      (void);

/**
 * @brief Sets the file cache size.
 *
 * @details This function sets the file cache size. Edje keeps this cache in
 *          order to prevent duplicate edje file entries in the memory. The
 *          file cache size can be retrieved using edje_file_cache_get().
 *
 * @param[in] count The file cache size in edje file units \n
 *              Default is 16.
 *
 * @see edje_file_cache_get()
 * @see edje_file_cache_flush()
 *
 */
EAPI void         edje_file_cache_set             (int count);

/**
 * @brief Gets the file cache size.
 *
 * @details This function returns the file cache size set by
 *          edje_file_cache_set().
 *
 * @return The file cache size in edje file units \n
 *         Default is 16.
 *
 * @see edje_file_cache_set()
 * @see edje_file_cache_flush()
 *
 */
EAPI int          edje_file_cache_get             (void);

/**
 * @brief Cleans the file cache.
 *
 * @details This function cleans the file cache entries, but keeps this cache's
 *          size to the last value set.
 *
 * @see edje_file_cache_set()
 * @see edje_file_cache_get()
 *
 */
EAPI void         edje_file_cache_flush           (void);

/**
 * @brief Sets the collection cache size.
 *
 * @details This function sets the collection cache size. Edje keeps this cache
 *          in order to prevent duplicate edje {collection,group,part}
 *          entries in the memory. The collection cache size can be retrieved using
 *          edje_collection_cache_get().
 *
 * @param[in] count The collection cache size, in edje object units \n
 *              Default is 16.
 *
 * @see edje_collection_cache_get()
 * @see edje_collection_cache_flush()
 *
 */
EAPI void         edje_collection_cache_set       (int count);

/**
 * @brief Gets the collection cache size.
 *
 * @details This function returns the collection cache size set by
 *          edje_collection_cache_set().
 *
 * @return The collection cache size, in edje object units \n
 *         Default is 16.
 *
 * @see edje_collection_cache_set()
 * @see edje_collection_cache_flush()
 *
 */
EAPI int          edje_collection_cache_get       (void);

/**
 * @brief Cleans the collection cache.
 *
 * @details This function cleans the collection cache, but keeps this cache's
 *          size to the last value set.
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
 * @internal
 * @defgroup Edje_External_Group Edje External
 * @ingroup Edje_Group
 *
 * @brief This group discusses functions of the external section of Edje.
 *
 * @remarks The programmer can create new types of parts, that are generically called
 *          EXTERNALS as they are not natives of Edje. The developer must also create
 *          plugins that define the meaning of each extra property carried by
 *          these parts of type EXTERNAL.
 *
 *          As long as there are new types that are properly registered with the plugins that are created,
 *          the user can use the parts of type EXTERNAL as all the parts of native types.
 *
 * @{
 */

/**
 * @brief Enumeration of the possible types of an EXTERNAL part parameter.
 */
typedef enum _Edje_External_Param_Type
{
   EDJE_EXTERNAL_PARAM_TYPE_INT, /**< Parameter value is an integer */
   EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, /**< Parameter value is a double */
   EDJE_EXTERNAL_PARAM_TYPE_STRING, /**< Parameter value is a string */
   EDJE_EXTERNAL_PARAM_TYPE_BOOL, /**< Parameter value is boolean */
   EDJE_EXTERNAL_PARAM_TYPE_CHOICE, /**< Parameter value is one from a set of
                                      predefined string choices */
   EDJE_EXTERNAL_PARAM_TYPE_MAX /**< Sentinel. Don't use */
} Edje_External_Param_Type;

/**
 * @brief Enumeration of the flags that determine how a parameter may be accessed in different
 *        circumstances.
 */
typedef enum _Edje_External_Param_Flags
{
   EDJE_EXTERNAL_PARAM_FLAGS_NONE        = 0, /**< Property is incapable of operations, this is used to catch bogus flags */
   EDJE_EXTERNAL_PARAM_FLAGS_GET         = (1 << 0), /**< Property can be read/obtained */
   EDJE_EXTERNAL_PARAM_FLAGS_SET         = (1 << 1), /**< Property can be written/set. This only enables edje_object_part_external_param_set() and Embryo scripts. To enable the parameter that is set from a state description whenever it changes state, use #EDJE_EXTERNAL_PARAM_FLAGS_STATE */
   EDJE_EXTERNAL_PARAM_FLAGS_STATE       = (1 << 2), /**< Property can be set from a state description */
   EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR = (1 << 3), /**< This property is only set once when the object is constructed using its value from the "default" 0.0 state description. Setting this overrides #EDJE_EXTERNAL_PARAM_FLAGS_STATE */
   EDJE_EXTERNAL_PARAM_FLAGS_REGULAR     = (EDJE_EXTERNAL_PARAM_FLAGS_GET |
                                            EDJE_EXTERNAL_PARAM_FLAGS_SET |
                                            EDJE_EXTERNAL_PARAM_FLAGS_STATE) /**< Convenience flag that sets the property as GET, SET, or STATE */
} Edje_External_Param_Flags;

/**
 * @brief Converts the type identifier to a string representation.
 *
 * @remarks This may be used to debug or for other informational purposes.
 *
 * @param[in] type The identifier to convert
 * @return The string with the string representation, otherwise @c "(unknown)"
 */
EAPI const char *edje_external_param_type_str(Edje_External_Param_Type type) EINA_PURE;

/**
 * @brief The structure type that holds parameters for the parts of type EXTERNAL.
 */
struct _Edje_External_Param
{
   const char               *name; /**< The name of the parameter */
   Edje_External_Param_Type  type; /**< The type of the parameter. This defines
                                     which of the next three variables holds
                                     the value for it */
   // XXX these could be in a union, but eet doesn't support them (or does it?)
   int                       i; /**< Used by both integer and boolean */
   double                    d; /**< Used by double */
   const char               *s; /**< Used by both string and choice */
};
/**
 * @brief The structure type that holds parameters for the parts of type EXTERNAL.
 */
typedef struct _Edje_External_Param Edje_External_Param;

/**
 * @brief Definition of the helper macro to indicate that an EXTERNAL integer parameter is undefined.
 */
#define EDJE_EXTERNAL_INT_UNSET INT_MAX
/**
 * @brief Definition of the helper macro to indicate that an EXTERNAL double parameter is undefined.
 */
#define EDJE_EXTERNAL_DOUBLE_UNSET DBL_MAX

/**
 * 
 * @brief The structure type holding information about an EXTERNAL part's parameters.
 *
 * @remarks When creating types to use with EXTERNAL parts, an array of this type is
 *          used to describe the different parameters that the object uses.
 *
 * @remarks This struct holds the name, type, and flags that define how and when the
 *          parameter is used, as well as information specific to each type, like the
 *          maximum or minimum value that can be used by editors to restrict the
 *          range of values to set for each parameter.
 */
typedef struct _Edje_External_Param_Info Edje_External_Param_Info;

/**
 * @internal
 * @brief The structure type holding information about an EXTERNAL part's parameters.
 *
 * @remarks When creating types to use with EXTERNAL parts, an array of this type is
 *          used to describe the different parameters that the object uses.
 *
 * @remarks This struct holds the name, type, and flags that define how and when the
 *          parameter is used, as well as information specific to each type, like the
 *          maximum or minimum value that can be used by editors to restrict the
 *          range of values to set for each parameter.
 */
struct _Edje_External_Param_Info
{
   const char               *name; /**< Name of the parameter */
   Edje_External_Param_Type  type; /**< Type of the parameter */
   Edje_External_Param_Flags flags; /**< Flags indicating how this parameter is
                                      used */
   union {
      struct {
         int                 def, /**< Default value for the parameter */
                             min, /**< Minimum value it can have */
                             max, /**< Maximum value it can have */
                             step; /**< Values are a multiple of this */
      } i; /**< Information about integer type parameters. Use #EDJE_EXTERNAL_INT_UNSET on any of them to indicate that they are not defined */
      struct {
         double              def, /**< Default value for the parameter */
                             min, /**< Minimum value it can have */
                             max, /**< Maximum value it can have */
                             step; /**< Values are a multiple of this */
      } d; /**< Information about double type parameters. Use #EDJE_EXTERNAL_DOUBLE_UNSET on any of them to indicate that they are not defined */
      struct {
         const char         *def; /**< Default value */
         const char         *accept_fmt; /**< Not implemented */
         const char         *deny_fmt; /**< Not implemented */
      } s; /**< Information about string type parameters. NULL indicates undefined */
      struct {
         int                 def; /**< Default value */
         const char         *false_str; /**< String shown by editors to indicate the false state */
         const char         *true_str; /**< String shown by editors to indicate the true state */
      } b; /**< Information about boolean type parameters */
      struct {
         const char         *def; /**< Default value */
         const char        **choices; /* Array of strings, each represents a
                                         valid value for this parameter. The
                                         last element of the array must be
                                         NULL */
         char               *(*def_get)(void *data, const Edje_External_Param_Info *info); /** return malloc() memory with the default choice, should be used if def is NULL. First parameter is Edje_External_Type::data */
         char              **(*query)(void *data, const Edje_External_Param_Info *info); /** NULL terminated array of strings, memory is dynamically allocated and should be freed with free() for each array and element. First parameter is Edje_External_Type::data */
      } c; /**< Information about choice type parameters */
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

/**
 * @brief definition of EDJE_EXTERNAL_PARAM_INFO_SENTINEL
 */
#define EDJE_EXTERNAL_PARAM_INFO_SENTINEL {NULL, 0, 0, {.s = {NULL, NULL, NULL}}}

/**
 * @brief definition of EDJE_EXTERNAL_TYPE_ABI_VERSION
 */
#define EDJE_EXTERNAL_TYPE_ABI_VERSION (3)

/**
 * @internal
 * @struct _Edje_External_Type
 *
 * @brief The structure type containing information about an external type to be used.
 *
 * @remarks This structure provides information on how to display and modify a
 *          third party Evas_Object in Edje.
 *
 * @remarks Some function pointers are not really used by Edje, but provide
 *          a means for edje users to interact with such objects in a better way. For
 *          instance, an editor may use label_get() and icon_get() to list all the
 *          registered external types.
 *
 * @remarks The function pointers provided in this structure must check
 *          for errors and invalid or out-of-range values. As for
 *          performance reasons, Edje does not enforce hints provided as
 *          #Edje_External_Param_Info in the member parameters_info.
 */
struct _Edje_External_Type
{
  unsigned int  abi_version; /**< Always use:
                              *  - #EDJE_EXTERNAL_TYPE_ABI_VERSION to declare.
                              *  - edje_external_type_abi_version_get() to check.
                              */
  const char    *module; /**< Name of the module that holds these definitions,
                           as used in the externals {} block of a theme
                           definition */
  const char    *module_name; /**< Canonical name of the module, for displaying
                                in edition programs, for example */
  Evas_Object *(*add) (void *data, Evas *evas, Evas_Object *parent, const Eina_List *params, const char *part_name); /**< Creates the object to be used by Edje as a part. @a part_name is the name of the part that holds the object and can be used to forward callbacks from the object as signals from Edje. @a params is the list of #Edje_External_Param, not parsed, from the default state of the part. Parameters of type #EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR should be set on
 the object here */
  void         (*state_set) (void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos); /**< Called upon state changes, including the initial "default" 0.0 state. Parameters are the value returned by params_parse(). The @a pos parameter is a value between 0.0 and 1.0 indicating the position in time within the state transition */
  void         (*signal_emit) (void *data, Evas_Object *obj, const char *emission, const char *source); /**< Feeds a signal emitted with emission originally set as part_name:signal to this object (without the "part_name:" prefix) */
  Eina_Bool    (*param_set) (void *data, Evas_Object *obj, const Edje_External_Param *param); /**< Dynamically changes a parameter of this external, called by scripts and user code. Returns #EINA_TRUE on success */
  Eina_Bool    (*param_get) (void *data, const Evas_Object *obj, Edje_External_Param *param); /**< Dynamically fetches a parameter of this external, called by scripts and user code. Returns #EINA_TRUE on success (Must check the parameter name and type) */
  Evas_Object *(*content_get) (void *data, const Evas_Object *obj, const char *content); /**< Dynamically fetches a sub object of this external, called by scripts and user code. Returns @c Evas_Object * on success (Must check the parameter name and type) */
  void        *(*params_parse) (void *data, Evas_Object *obj, const Eina_List *params); /**< Parses the list of parameters, converting them into a friendly representation. Used with state_set() */
  void         (*params_free) (void *params); /**< Frees parameters parsed with params_parse() */

  /* The following callbacks aren't used by Edje itself, but by UI design
     tools instead */
  const char  *(*label_get) (void *data); /**< Gets a label to use to identify this EXTERNAL (For editors) */
  const char  *(*description_get) (void *data); /**< Gets a user friendly description of this EXTERNAL (For editors) */
  Evas_Object *(*icon_add) (void *data, Evas *e); /**< Gets an icon to use to identify this EXTERNAL (For editors) */
  Evas_Object *(*preview_add) (void *data, Evas *e); /**< Gets a preview of the EXTERNAL object in use (For editors) */
  const char  *(*translate) (void *data, const char *orig); /**< Called to translate parameters_info name properties for use in user interfaces that support internationalization (i18n) (For editors) */

  Edje_External_Param_Info *parameters_info; /**< An array of #Edje_External_Param_Info describing the different parameters that EXTERNAL may have. The last element in the array must be #EDJE_EXTERNAL_PARAM_INFO_SENTINEL */
  void                     *data; /**< Private user data that is passed to all the class functions */
};

/**
 * @brief typedef to struct #_Edje_External_Type
 */
typedef struct _Edje_External_Type Edje_External_Type;

/**
 * @brief The convenience structure type used to mass-register types of EXTERNAL objects.
 *
 * @remarks Used with edje_external_type_array_register().
 */
struct _Edje_External_Type_Info
{
   const char               *name; /**< The name of the type to register */
   const Edje_External_Type *info; /**< The type definition */
};

typedef struct _Edje_External_Type_Info Edje_External_Type_Info;

/**
 * @}
 */

/**
 * @internal
 * @defgroup Edje_External_Part_Group Edje Use of External Parts
 * @ingroup Edje_External_Group
 *
 * @brief This group discusses functions to manipulate parts of type EXTERNAL.
 *
 * @remarks Edje supports parts of type EXTERNAL, which call plugins defined by the user
 *          to create and manipulate the object that's allocated in that part.
 *
 *          Parts of type external may carry extra properties that have meanings defined
 *          by the external plugin. For instance, it may be a string that defines a button
 *          label and setting this property changes that label on the fly.
 *
 * @{
 */

/**
 * @brief Gets the object created by this external part.
 *
 * @remarks Parts of type external create the part object using the information
 *          provided by external plugins. It's somehow like "swallow"
 *          (edje_object_part_swallow()), but it's set automatically.
 *
 * @remarks This function returns the part created by such external plugins and
 * 	        being currently managed by this Edje.
 *
 * @remarks Almost all swallow rules apply: you should not move, resize,
 *          hide, show, or set the color or clipper of such a part. It's a bit
 *          more restrictive as one must @b never delete this object.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return The externally created object, otherwise @c NULL if there is no such object or
 *         the part is not an external
 *
 */
EAPI Evas_Object              *edje_object_part_external_object_get     (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the parameter for the external part.
 *
 * @remarks Parts of type external may carry extra properties that have their
 *          meanings defined by the external plugin. For instance, it may be a
 *          string that defines a button label and setting this property 
 *          changes that label on the fly.
 *
 * @remarks External parts have their parameters set when they change
 *          states. Those parameters are never changed by this
 *          function. The interpretation of how the state_set parameters and
 *          param_set interact is up to the external plugin.
 *
 * @remarks This function does not check if the parameter value is valid
 *          using #Edje_External_Param_Info minimum, maximum, valid
 *          choices, and others. However, these should be checked by the
 *          underlying implementation provided by the external
 *          plugin. This is done for performance reasons.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] param The parameter details, including its name, type, and
 *              actual value \n
 *              This pointer should be valid, and the
 *              parameter must exist in
 *              #Edje_External_Type::parameters_info, with the exact type,
 *              otherwise the operation fails and #EINA_FALSE is
 *              returned.
 *
 * @return #EINA_TRUE if everything works fine, 
 *         otherwise #EINA_FALSE on errors
 *
 */
EAPI Eina_Bool                 edje_object_part_external_param_set      (Evas_Object *obj, const char *part, const Edje_External_Param *param);

/**
 * @brief Gets the parameter for the external part.
 *
 * @remarks Parts of type external may carry extra properties that have
 *          meanings defined by the external plugin. For instance, it may be a
 *          string that defines a button label. This property can be modified by
 *          the state parameters, by making explicit calls to
 *          edje_object_part_external_param_set() or by getting the actual object
 *          using edje_object_part_external_object_get() and calling native
 *          functions.
 *
 * @remarks This function asks the external plugin about the current value,
 *          independent of how it is set.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[out] param The parameter details \n
 *              It is used as both the input and output variable \n
 *              This pointer should be valid and the
 *              parameter must exist in
 *              #Edje_External_Type::parameters_info, with the exact type,
 *              otherwise the operation fails and #EINA_FALSE is
 *              returned.
 *
 * @return #EINA_TRUE if everything works fine and @a param members
 *         are filled with information, otherwise #EINA_FALSE on errors
 *         and if @a param member values are not set or valid
 */
EAPI Eina_Bool                 edje_object_part_external_param_get      (const Evas_Object *obj, const char *part, Edje_External_Param *param);

/**
 * @brief Gets an object contained in a part of type EXTERNAL.
 *
 * @remarks The @a content string must not be @c NULL. Its actual value depends on the
 *          code providing the type EXTERNAL.
 *
 * @param[in] obj The edje object
 * @param[in] part The name of the part holding the type EXTERNAL
 * @param[out] content A string identifying the content to get from the type EXTERNAL
 */
EAPI Evas_Object              *edje_object_part_external_content_get    (const Evas_Object *obj, const char *part, const char *content);

/**
 * @brief Gets the type of the given parameter of the given part.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[out] param The parameter name to use
 *
 * @return @c EDJE_EXTERNAL_PARAM_TYPE_MAX on errors, otherwise another value
 *         from #Edje_External_Param_Type on success
 */
EAPI Edje_External_Param_Type  edje_object_part_external_param_type_get (const Evas_Object *obj, const char *part, const char *param);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Edje_External_Plugin_Development_Group Edje Development of External Plugins
 * @ingroup Edje_External_Group
 *
 * @brief This group discusses functions to register and unregister EXTERNAL types as well as develop plugins.
 *
 * @remarks This group dicusses the functions that are useful for the development of new plugins.
 *          These functions deal with the new EXTERNAL types by registering, unregistering, and manipulating them.
 *
 * @{
 */

/**
 * @brief Registers a type to be used by EXTERNAL parts.
 *
 * @remarks Parts of type EXTERNAL call user defined functions
 *          to create and manipulate the object that's allocated in that part. This is
 *          done by specifying the name of the external to use
 *          in the @c source property of that part.
 *          This property must be registered with this function.
 *
 * @param[in] type_name The name to register and be known by edje's "source:"
 *            parameter of "type: EXTERNAL" parts
 * @param[in] type_info The meta-information describing how to interact with the @c source parameter
 *
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE on failure (like
 *         type already registered)
 *
 * @see edje_external_type_array_register()
 */
EAPI Eina_Bool    edje_external_type_register             (const char *type_name, const Edje_External_Type *type_info);

/**
 * @brief Unregisters a previously registered EXTERNAL type.
 *
 * @param[in] type_name The name to unregister \n
 *                  It should have been registered with
 *                  edje_external_type_register() earlier.
 *
 * @return #EINA_TRUE on success,
 *         otherwise #EINA_FALSE on failure (like type_name did not exist)
 *
 * @see edje_external_type_array_unregister()
 */
EAPI Eina_Bool    edje_external_type_unregister           (const char *type_name);

/**
 * @brief Registers a batch of types and their information.
 *
 * @remarks When several types are registered, it is recommended to use this
 *          function instead of several calls to edje_external_type_register(), as it
 *          is faster.
 *
 * @remarks The contents of the array are referenced directly for as long as
 *          the type remains registered, so both the @c name and @c info in the
 *          @a array must be kept alive during this period (usually, the entire
 *          program lifetime). The most common case would be to keep the array as a
 *          @c static @c const type.
 *
 * @param[in] array The @c NULL terminated array with type name and
 *              information \n
 *              Note that type name or information is
 *              referenced directly, so they must be kept alive after
 *              this function returns.
 *
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE on failure (like
 *         type already registered)
 *
 * @see edje_external_type_register()
 */
EAPI void         edje_external_type_array_register       (const Edje_External_Type_Info *array);

/**
 * @brief Unregisters a batch of given external type arrays that were previously registered.
 *
 * @param[in] array The @c NULL terminated array, should be same as the
 *              one registered with edje_external_type_array_register()
 *
 * @see edje_external_type_unregister()
 */
EAPI void         edje_external_type_array_unregister     (const Edje_External_Type_Info *array);

/**
 * @brief Gets the current ABI version for the #Edje_External_Type structure.
 *
 * @remarks Always check this number before accessing #Edje_External_Type in
 *          your own software. If the number is not the same, your software may
 *          access invalid memory and crash, or just get garbage values.
 *
 * @remarks @b NEVER, define your own #Edje_External_Type using the
 *          return of this function as it changes as the edje library
 *          (libedje.so) changes, but your type definition does
 *          not. Instead, use #EDJE_EXTERNAL_TYPE_ABI_VERSION.
 *
 * Summary:
 *   - Use edje_external_type_abi_version_get() to check.
 *   - Use #EDJE_EXTERNAL_TYPE_ABI_VERSION to define/declare.
 *
 * @return The external ABI version that the edje library is compiled with,
 *         that is, the value #EDJE_EXTERNAL_TYPE_ABI_VERSION had at that moment
 *
 */
EAPI unsigned int edje_external_type_abi_version_get      (void) EINA_CONST;

/**
 * @brief Gets an iterator of all the registered EXTERNAL types.
 *
 * @remarks Each item in the iterator is an @c Eina_Hash_Tuple which has the type
 *          of the external in the @c key and #Edje_External_Type as the @c data.
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
 * @brief Finds a specific parameter from a list of external parameters.
 *
 * @param[in] params The list of parameters
 * @param[in] key The parameter to look for
 *
 * @return The matching #Edje_External_Param, 
 *         otherwise @c NULL if it's not found
 */
EAPI Edje_External_Param            *edje_external_param_find       (const Eina_List *params, const char *key);

/**
 * @brief Gets the value of the given parameter of integer type.
 *
 * @remarks Look for the @a key parameter in the @a params list and return its value in
 *          @a ret. If the parameter is found and is of type
 *          #EDJE_EXTERNAL_PARAM_TYPE_INT, its value is stored in the int pointed
 *          by @a ret, returning #EINA_TRUE. In any other case, the function returns
 *          #EINA_FALSE.
 *
 * @param[in] params The list of parameters to look at
 * @param[in] key The name of the parameter to fetch
 * @param[out] ret An Int pointer to store the value, must not be NULL
 *
 * @return #EINA_TRUE if the parameter is found and is of integer type,
 *         otherwise #EINA_FALSE
 */
EAPI Eina_Bool                       edje_external_param_int_get    (const Eina_List *params, const char *key, int *ret);

/**
 * @brief Gets the value of the given parameter of double type.
 *
 * @remarks Look for the @a key parameter in the @a params list and return its value in
 *          @a ret. If the parameter is found and is of type
 *          #EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, its value is stored in the double
 *          pointed by @a ret, returning #EINA_TRUE. In any other case, the function
 *          returns #EINA_FALSE.
 *
 * @param[in] params The list of parameters to look at
 * @param[in] key The name of the parameter to fetch
 * @param[out] ret A double pointer to store the value, must not be NULL
 *
 * @return #EINA_TRUE if the parameter is found and is of double type,
 *         otherwise #EINA_FALSE
 */
EAPI Eina_Bool                       edje_external_param_double_get (const Eina_List *params, const char *key, double *ret);

/**
 * @brief Gets the value of the given parameter of string type.
 *
 * @remarks Look for the @a key parameter in the @a params list and return its value in
 *          @a ret. If the parameter is found and is of type
 *          #EDJE_EXTERNAL_PARAM_TYPE_STRING, its value is stored in the pointer
 *          pointed by @a ret, returning #EINA_TRUE. In any other case, the function
 *          returns #EINA_FALSE.
 *
 * @remarks The string stored in @a ret must not be freed or modified.
 *
 * @param[in] params The list of parameters to look at
 * @param[in] key The name of the parameter to fetch
 * @param[out] ret A string pointer to store the value, must not be NULL
 *
 * @return #EINA_TRUE if the parameter is found and is of string type,
 *         otherwise #EINA_FALSE
 */
EAPI Eina_Bool                       edje_external_param_string_get (const Eina_List *params, const char *key, const char **ret);

/**
 * @brief Gets the value of the given parameter of boolean type.
 *
 * @remarks Look for the @a key parameter in the @a params list and return its value in
 *          @a ret. If the parameter is found and is of type
 *          #EDJE_EXTERNAL_PARAM_TYPE_BOOL, its value is stored in the Eina_Bool
 *          pointed by @a ret, returning #EINA_TRUE. In any other case, the function
 *          returns #EINA_FALSE.
 *
 * @param[in] params The list of parameters to look at
 * @param[in] key The name of the parameter to fetch
 * @param[out] ret An Eina_Bool pointer to store the value, must not be NULL
 *
 * @return #EINA_TRUE if the parameter is found and is of boolean type,
 *         otherwise #EINA_FALSE
 */
EAPI Eina_Bool                       edje_external_param_bool_get   (const Eina_List *params, const char *key, Eina_Bool *ret);

/**
 * @brief Gets the value of the given parameter of choice type.
 *
 * @remarks Look for the @a key parameter in the @a params list and return its value in
 *          @a ret. If the parameter is found and is of type
 *          #EDJE_EXTERNAL_PARAM_TYPE_CHOICE, its value is stored in the string
 *          pointed by @a ret, returning #EINA_TRUE. In any other case, the function
 *          returns #EINA_FALSE.
 *
 * @remarks The string stored in @a ret must not be freed or modified.
 *
 * @param[in] params The list of parameters to look at
 * @param[in] key The name of the parameter to fetch
 * @param[out] ret A string pointer to store the value, must not be NULL
 *
 * @return #EINA_TRUE if the parameter is found and is of integer type,
 *         otherwise #EINA_FALSE
 */
EAPI Eina_Bool                       edje_external_param_choice_get (const Eina_List *params, const char *key, const char **ret);

/**
 * @brief Gets the array of parameter information about a type, given that its name is provided.
 *
 * @remarks The type names and other strings are static, that means they are
 *          @b NOT translated. One must use
 *          Edje_External_Type::translate() to translate them.
 *
 * @return The NULL terminated array, otherwise @c NULL if the type is unknown or
 *         it does not have any parameter information
 *
 * @see edje_external_type_get()
 */
EAPI const Edje_External_Param_Info *edje_external_param_info_get   (const char *type_name);

/**
 * @brief Gets #Edje_External_Type that defines an EXTERNAL type registered with
 *        the name @a type_name.
 */
EAPI const Edje_External_Type       *edje_external_type_get         (const char *type_name);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Group Edje Object
 * @ingroup Edje_Group
 *
 * @brief This group discusses functions that deal with edje layouts and its components.
 *
 * @remarks An important thing to know about this group is that there is no
 *          Edje_Object in the @b code. What we refer to as an object are actually layouts (or themes)
 *          defined by groups, and parts, both declared in EDC files. They are of
 *          type Evas_Object as the other native objects of Evas, but they only exist
 *          in Edje, so that is why we are calling them "edje objects".
 *
 * @remarks With the Edje Object Group functions we can deal with layouts by managing
 *          its aspect, content, message, and signal exchange and animation, among others.
 *
 * @{
 */

/**
 * @brief Instantiates a new edje object.
 *
 * @details This function creates a new edje smart object, returning its @c
 *          Evas_Object handle. An edje object is useless without a (source)
 *          file set to it, so you would most probably call edje_object_file_set()
 *          afterwards, like in:
 *
 * @since_tizen 2.3
 *
 * @code
 * Evas_Object *edje;
 *
 * edje = edje_object_add(canvas);
 * if (!edje)
 *   {
 *      fprintf(stderr, "could not create edje object!\n");
 *      return NULL;
 *   }
 *
 * if (!edje_object_file_set(edje, "theme.edj", "group_name"))
 *   {
 *      int err = edje_object_load_error_get(edje);
 *      const char *errmsg = edje_load_error_str(err);
 *      fprintf(stderr, "could not load 'group_name' from theme.edj: %s",
 *      errmsg);
 *
 *      evas_object_del(edje);
 *      return NULL;
 *   }
 *
 * @endcode
 *
 * @remarks You can get a callback every time edje re-calculates the object
 *          (either due to animation or some kind of signal or input). This is called
 *          in-line just after the recalculation has occurred. It is a good idea to not
 *          go and delete or alter the object inside this callback, simply make
 *          a note that the recalculation has taken place and then do something about
 *          it outside the callback. To register a callback use a code like:
 *
 * @code
 *    evas_object_smart_callback_add(edje_obj, "recalc", my_cb, my_cb_data);
 * @endcode
 *
 * @remarks Before creating the first edje object in your code, remember
 *          to initialize the library, with edje_init(), or unexpected behavior
 *          might occur.
 *
 * @param[in] evas A valid Evas handle, the canvas to place the new object in
 * @return A handle to the new object created, otherwise @c NULL on errors
 *
 * @see evas_object_smart_callback_add()
 */
EAPI Evas_Object *edje_object_add                 (Evas *evas);

/**
 * @brief Preloads the images on the edje Object in the background.
 *
 * @details This function requests the preload of data images (on the given
 *          object) in the background. The work is queued before being processed
 *          (because there might be other pending requests of this type).
 *          It emits a signal "preload,done" when finished.
 *
 * @since_tizen 2.3
 *
 * @remarks Use #EINA_TRUE in scenarios where you don't need
 *          the image data to be preloaded anymore.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] cancel If #EINA_FALSE it is added to the preloading work queue,
 *               otherwise #EINA_TRUE to remove it (if it is issued before)
 * @return @c EINA_FASLE if @a obj is not a valid edje object,
 *         otherwise #EINA_TRUE
 */
EAPI Eina_Bool        edje_object_preload         (Evas_Object *obj, Eina_Bool cancel);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Scale Edje Scale
 * @ingroup Edje_Group
 *
 * @brief This group discusses functions that deal with scaling objects.
 *
 * @remarks Edje allows one to build scalable interfaces. Scaling factors,
 *          which are set to neutral (@c 1.0) values by default (no scaling,
 *          actual sizes), are of two types: @b global and @b individual.
 *
 * @remarks Scaling affects the values of minimum/maximum @b part sizes, which
 *          are @b multiplied by it. Font sizes are scaled, too.
 *
 * @{
 */

/**
 * @brief Sets Edje's global scaling factor.
 *
 * @since_tizen 2.3
 *
 * @remarks Edje's global scaling factor affects all its objects whose
 *          individual scaling factors are not altered from the default
 *          value (which is zero). If they had it set differently, by
 *          edje_object_scale_set(), that factor @b overrides the global
 *          one.
 *
 * @remarks Only parts which, at EDC level, have their @c "scale"
 *          property set to @c 1, are affected by this function. Check the
 *          complete @ref edcref "syntax reference" for EDC files.
 *
 * @param[in] scale The global scaling factor (the default value is @c 1.0)
 *
 * @see edje_scale_get()
 */
EAPI void         edje_scale_set                  (double scale);

/**
 * @brief Gets Edje's global scaling factor.
 *
 * @details This function returns Edje's global scaling factor.
 *
 * @since_tizen 2.3
 *
 * @return The global scaling factor
 *
 * @see edje_scale_set()
 *
 */
EAPI double       edje_scale_get                  (void);

/**
 * @brief Sets the scaling factor for a given edje object.
 *
 * @details This function sets an @b individual scaling factor on the @a obj
 *          edje object. This property (or Edje's global scaling factor, when
 *          applicable), affects this object's part sizes. If @a scale is
 *          not zero, then the individual scaling @b overrides any global
 *          scaling set, for the object @a obj's parts. Put it back to zero to
 *          get the effects of global scaling again.
 *
 * @since_tizen 2.3
 *
 * @remarks Only parts which, at EDC level, have their @c "scale"
 *          property set to @c 1, are affected by this function. Check the
 *          complete @ref edcref "syntax reference" for EDC files.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] scale The scaling factor (the default value is @c 0.0,
 *              meaning individual scaling is @b not set)
 * @return #EINA_TRUE if success, otherwise #EINA_FALSE

 * @see edje_object_scale_get()
 * @see edje_scale_get()
 */
EAPI Eina_Bool    edje_object_scale_set           (Evas_Object *obj, double scale);

/**
 * @brief Gets a given Edje object's scaling factor.
 *
 * @details This function returns the @c individual scaling factor set on the
 *          @a obj edje object.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @return    A scale factor
 *
 * @see edje_object_scale_set()
 *
 */
EAPI double       edje_object_scale_get           (const Evas_Object *obj);

/**
 * @brief Gets a given Edje object's base_scale factor.
 *
 * @details This function returns the base_scale factor set on the
 *          Edje object.
 *          The base_scale can be set in the collection of edc.
 *          If it isn't set, the default value is 1.0.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @return    A base scale factor
 */
EAPI double edje_object_base_scale_get(const Evas_Object *obj);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Color_Class Edje Class: Color
 * @ingroup Edje_Group
 *
 * @brief This group discusses functions that deal with Color Classes.
 *
 * @remarks Sometimes we want to change the color of two or more parts equally and
 *          that's when we use color classes.
 *
 * @remarks If one or more parts are assigned with a color class, when we set color
 *          values to this class it causes all these parts to have their colors
 *          multiplied by the values. Setting values to a color class at a process level
 *          affects all parts within that color class, while at an object level it
 *          only affects the parts inside a specified object.
 *
 * @{
 */

/**
 * @brief Sets the Edje color class.
 *
 * @details This function sets the color values for a process level color
 *          class. This causes all the edje parts in the current process that
 *          have the specified color class to have their colors multiplied by
 *          these values.  (Object level color classes set by
 *          edje_object_color_class_set() override the values set by this
 *          function).
 *
 * @since_tizen 2.3
 *
 * @remarks The first color is the object, the second is the text outline, and
 *          the third is the text shadow. (Note that the second and third only apply
 *          to text parts).
 *
 * @remarks Setting color emits a signal "color_class,set" with source being
 *          the given color class in all objects.
 *
 * @remarks Unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *          half-transparent white is 255 255 255 128.
 *
 * @param[in] color_class The color class name
 * @param[in] r The object Red value
 * @param[in] g The object Green value
 * @param[in] b The object Blue value
 * @param[in] a The object Alpha value
 * @param[in] r2 The outline Red value
 * @param[in] g2 The outline Green value
 * @param[in] b2 The outline Blue value
 * @param[in] a2 The outline Alpha value
 * @param[in] r3 The shadow Red value
 * @param[in] g3 The shadow Green value
 * @param[in] b3 The shadow Blue value
 * @param[in] a3 The shadow Alpha value
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_color_class_set()
 */
EAPI Eina_Bool    edje_color_class_set            (const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Gets the Edje color class.
 *
 * @details This function gets the color values for a process level color
 *          class. This value is globally set and not per-object, that is,
 *          the value that would be used by objects if they do not override using
 *          edje_object_color_class_set().
 *
 * @since_tizen 2.3
 *
 * @remarks The first color is the object, the second is the text outline, and
 *          the third is the text shadow. (Note that the second and third only apply
 *          to text parts).
 *
 * @remarks Unlike Evas, edje colors are @b not pre-multiplied. That is,
 *          half-transparent white is 255 255 255 128.
 *
 * @param[in] color_class The color class name
 * @param[out] r The object Red value
 * @param[out] g The object Green value
 * @param[out] b The object Blue value
 * @param[out] a The object Alpha value
 * @param[out] r2 The outline Red value
 * @param[out] g2 The outline Green value
 * @param[out] b2 The outline Blue value
 * @param[out] a2 The outline Alpha value
 * @param[out] r3 The shadow Red value
 * @param[out] g3 The shadow Green value
 * @param[out] b3 The shadow Blue value
 * @param[out] a3 The shadow Alpha value
 *
 * @return #EINA_TRUE if found,
 *         otherwise #EINA_FALSE if not found and all values are zeroed
 *
 * @see edje_color_class_set()
 */
EAPI Eina_Bool    edje_color_class_get            (const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Deletes the Edje color class.
 *
 * @details This function deletes any values at the process level for the
 *          specified color class.
 *
 * @since_tizen 2.3
 *
 * @remarks Deleting the color class reverts it to the
 *          values defined in the theme file.
 *          Deleting the color class emits the signal "color_class,del"
 *          to all the edje objects in the running program.
 *
 * @param[in] color_class The color class name
 *
 */
EAPI void         edje_color_class_del            (const char *color_class);

/**
 * @brief Lists the color classes.
 *
 * @details This function lists all the color classes known by the current
 *          process.
 *
 * @since_tizen 2.3
 *
 * @return A list of color class names (strings) \n
 *         These strings and
 *         the list must be freed by the caller using free().
 *
 */
EAPI Eina_List   *edje_color_class_list           (void);

/**
 * @brief Sets the object color class.
 *
 * @details This function sets the color values for an object level color
 *          class. This causes all edje parts in the specified object that
 *          have the specified color class to have their colors multiplied by
 *          these values.
 *
 * @since_tizen 2.3
 *
 * @remarks The first color is the object, the second is the text outline, and
 *          the third is the text shadow. (Note that the second and third only apply
 *          to text parts).
 *
 * @remarks Setting color emits a signal "color_class,set" with source being
 *          the given color.
 *
 * @remarks Unlike Evas, edje colors are @b not pre-multiplied. That is,
 *          half-transparent white is 255 255 255 128.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] color_class The color class name
 * @param[in] r The object Red value
 * @param[in] g The object Green value
 * @param[in] b The object Blue value
 * @param[in] a The object Alpha value
 * @param[in] r2 The outline Red value
 * @param[in] g2 The outline Green value
 * @param[in] b2 The outline Blue value
 * @param[in] a2 The outline Alpha value
 * @param[in] r3 The shadow Red value
 * @param[in] g3 The shadow Green value
 * @param[in] b3 The shadow Blue value
 * @param[in] a3 The shadow Alpha value
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 */
EAPI Eina_Bool    edje_object_color_class_set         (Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Gets the object color class.
 *
 * @details This function gets the color values for an object level color
 *          class. If no explicit object color is set, then global values are
 *          used.
 *
 * @since_tizen 2.3
 *
 * @remarks The first color is the object, the second is the text outline, and
 *          the third is the text shadow. (Note that the second and third only apply
 *          to text parts).
 *
 * @remarks Unlike Evas, edje colors are @b not pre-multiplied. That is,
 *          half-transparent white is 255 255 255 128.
 *
 * @param[in] o A valid Evas_Object handle
 * @param[in] color_class The color class name
 * @param[out] r The object Red value
 * @param[out] g The object Green value
 * @param[out] b The object Blue value
 * @param[out] a The object Alpha value
 * @param[out] r2 The outline Red value
 * @param[out] g2 The outline Green value
 * @param[out] b2 The outline Blue value
 * @param[out] a2 The outline Alpha value
 * @param[out] r3 The shadow Red value
 * @param[out] g3 The shadow Green value
 * @param[out] b3 The shadow Blue value
 * @param[out] a3 The shadow Alpha value
 *
 * @return #EINA_TRUE if found, otherwise #EINA_FALSE if not found and all
 *         values are zeroed
 *
 */
EAPI Eina_Bool    edje_object_color_class_get         (const Evas_Object *o, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Deletes the object color class.
 *
 * @details This function deletes any values at the object level for the
 *          specified object and color class.
 *
 * @since_tizen 2.3
 *
 * @remarks Deleting the color class reverts it to the values
 *          defined by edje_color_class_set() or the color class
 *          defined in the theme file.
 *          Deleting the color class emits the signal "color_class,del"
 *          for the given edje object.
 *
 * @param[in] obj The edje object reference
 * @param[in] color_class The color class to be deleted
 *
 */
 EAPI void         edje_object_color_class_del         (Evas_Object *obj, const char *color_class);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Text_Class Edje Class: Text
 * @ingroup Edje_Group
 *
 * @brief This group discusses functions that deal with Text Classes.
 *
 * @remarks Sometimes we want to change the text of two or more parts equally and
 *          that's when we use text classes.
 *
 * @remarks If one or more parts are assigned with a text class, when we set font
 *          attributes to this class it updates all these parts with the new font
 *          attributes. Setting values to a text class at a process level affects
 *          all parts within that text class, while at the object level it only affects
 *          the parts inside a specified object.
 *
 * @{
 */

/**
 * @brief Sets the Edje text class.
 *
 * @details This function updates all edje members at the process level which
 *          belong to this text class with the new font attributes.
 *          If the @a size is 0 then the font size is kept as the previous size.
 *          If the @a size is less then 0 then the font size is calculated as a
 *          percentage. For example, if the @a size is -50, then the font size is
 *          scaled to half of the original size and if the @a size is -10 then the font
 *          size is scaled by 0.1x.
 *
 * @since_tizen 2.3
 *
 * @param[in] text_class The text class name
 * @param[in] font The font name
 * @param[in] size The font size
 *
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE on error
 *
 * @see edje_text_class_get()
 *
 */
EAPI Eina_Bool    edje_text_class_set             (const char *text_class, const char *font, Evas_Font_Size size);

/**
 * @brief Gets the Edje text class.
 *
 * @details This function gets the text values for a process level text class.
 *
 * @since_tizen 2.3
 *
 * @param[in] text_class The text class name
 * @param[out] font The font name \n
 *             This string is a stringshare and must be freed by the caller using free().
 * @param[out] size The font size
 *
 * @return #EINA_TRUE if found, otherwise #EINA_FALSE if not found
 *
 * @see edje_text_class_set()
 *
 */
EAPI Eina_Bool    edje_text_class_get             (const char *text_class, char **font, Evas_Font_Size *size);

/**
 * @brief Deletes the text class.
 *
 * @details This function deletes any values at the process level for the
 *          specified text class.
 *
 * @since_tizen 2.3
 *
 * @param[in] text_class The text class name string
 */
EAPI void         edje_text_class_del             (const char *text_class);

/**
 * @brief Lists the text classes.
 *
 * @details This function lists all the text classes known by the current
 *          process.
 *
 * @since_tizen 2.3
 *
 * @return A list of text class names (strings) \n
 *         These strings are
 *         stringshares and the list must be freed by the caller using free().
 *
 */
EAPI Eina_List   *edje_text_class_list            (void);

/**
 * @brief Sets the Edje text class.
 *
 * @details This function sets the text class for Edje.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] text_class The text class name
 * @param[in] font The font name
 * @param[in] size The font Size
 *
 * @return #EINA_TRUE on success,
 *         otherwise #EINA_FALSE on error
 *
 */
EAPI Eina_Bool    edje_object_text_class_set          (Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size);

/**
 * @brief Gets the Edje text class.
 *
 * @details This function gets the text values for a process level text class.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] text_class The text class name
 * @param[out] font The font name \n
 *             This string is a stringshare and must be freed by the caller using free().
 * @param[out] size The font size
 *
 * @return #EINA_TRUE if found,
 *         otherwise #EINA_FALSE if not found
 *
 * @see edje_text_class_set()
 *
 */
EAPI Eina_Bool    edje_object_text_class_get(const Evas_Object *obj, const char *text_class, char **font, Evas_Font_Size *size);

/**
 * @}
 */

/**
 * @defgroup Edje_File Edje File
 * @ingroup Edje_Group
 *
 * @brief This group discusses functions to deal with EDJ files.
 *
 * @remarks Layouts in Edje are usually called themes and they are
 *          created using the EDC language. The EDC language is declarative
 *          and must be compiled before being used. The output of this
 *          compilation is an EDJ file, this file can be loaded by Edje,
 *          and the result is an edje object.
 *
 * @remarks This group of functions interacts with these EDJ files,
 *          either by loading them or retrieving information from the EDC
 *          file about objects.
 *
 * @{
 */

/**
 * @brief Edje Load Error Type
 */
typedef enum _Edje_Load_Error
{
   EDJE_LOAD_ERROR_NONE = 0, /**< No error occurred, the loading is successful */
   EDJE_LOAD_ERROR_GENERIC = 1, /**< A generic error occurred during loading */
   EDJE_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< The file being pointed to does not exist */
   EDJE_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission to read the given file has been denied */
   EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Resource allocation failed during loading */
   EDJE_LOAD_ERROR_CORRUPT_FILE = 5, /**< The file being pointed to is corrupt */
   EDJE_LOAD_ERROR_UNKNOWN_FORMAT = 6, /**< The file being pointed to has an unknown format */
   EDJE_LOAD_ERROR_INCOMPATIBLE_FILE = 7, /**< The file being pointed to is incompatible, i.e., it doesn't match the library's current version format */
   EDJE_LOAD_ERROR_UNKNOWN_COLLECTION = 8, /**< The group/collection set to load from is @b not found in the file */
   EDJE_LOAD_ERROR_RECURSIVE_REFERENCE = 9 /**< The group/collection set to load from had <b>recursive references</b> on its components */
} Edje_Load_Error; /**< Edje file is loading error codes that one can get - see edje_load_error_str() too */

/**
 * @brief Gets data from the file level data block of an edje file.
 *
 * @since_tizen 2.3
 *
 * @remarks If an edje file is built from the following edc:
 *
 * data {
 *   item: "key1" "value1";
 *   item: "key2" "value2";
 * }
 * collections { ... }
 *
 * Then, edje_file_data_get("key1") returns "value1".
 *
 * @param[in] file The path to the .edj file
 * @param[in] key The data key
 * @return The string value of the data, must be freed by the user when no
 *         longer needed
 *
 */
EAPI char        *edje_file_data_get              (const char *file, const char *key);

/**
 * @brief Gets the list of groups in an edje file.
 *
 * @since_tizen 2.3
 *
 * @remarks The list must be freed using edje_file_collection_list_free()
 *          when you are done with it.
 *
 * @param[in] file The path to the edje file
 *
 * @return An Eina_List of group names (char *)
 */
EAPI Eina_List        *edje_file_collection_list  (const char *file);

/**
 * @brief Frees the file collection list.
 *
 * @details This frees the list returned by edje_file_collection_list().
 *
 * @param[in] lst An Eina_List of groups
 */
EAPI void             edje_file_collection_list_free (Eina_List *lst);

/**
 * @brief Checks whether a group matching glob exists in an edje file.
 *
 * @since_tizen 2.3
 *
 * @param[in] file The file path
 * @param[in] glob The glob to match with
 *
 * @return @c 1 if a match is found, otherwise @c 0
 */
EAPI Eina_Bool        edje_file_group_exists      (const char *file, const char *glob);

/**
 * @brief Sets the @b EDJ file (and group within it) from which to load an edje
 *        object's contents.
 *
 * @since_tizen 2.3
 *
 * @remarks Edje expects EDJ files, which are theming objects' descriptions and
 *          resources packed together in an EET file, to read edje object
 *          definitions from. They are usually created with the @c .edj
 *          extension. EDJ files, in turn, are assembled from @b textual object
 *          description files, where one describes edje objects declaratively,
 *          the EDC files (see @ref edcref "the syntax" for those files).
 *
 * @remarks Those description files were designed so that many edje object
 *          definitions, also called @b groups (or collections), could be
 *          packed together <b>in the same EDJ file</b>, so that a whole
 *          application's theme could be packed into a single file only. This is the
 *          reason for the @a group argument.
 *
 * @remarks Use this function after you instantiate a new edje object, so that
 *          you can "give it life", by telling it from where it can get its contents.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] file The path to the EDJ file to load @a from
 * @param[in] group The name of the group, in @a file, which implements an
 *              edje object
 * @return #EINA_TRUE on success otherwise #EINA_FALSE on errors (check
 *         edje_object_load_error_get() after this call to get errors causes)
 *
 * @see edje_object_add()
 * @see edje_object_file_get()
 */
EAPI Eina_Bool        edje_object_file_set        (Evas_Object *obj, const char *file, const char *group);

/**
 * @brief Gets the file and group name that a given edje object is bound to.
 *
 * @since_tizen 2.3
 *
 * @remarks This gets the EDJ file's path, with the respective group set for
 *          the given edje object. If @a obj is either not an edje file, or has
 *          not had its file/group set previously, by edje_object_file_set(),
 *          then both @a file and @a group are set to @c NULL, indicating
 *          an error.
 *
 * @remarks Use @c NULL pointers on the file/group components you're not
 *          interested in, they are ignored by the function.
 *
 * @param[in] obj A handle to an edje object
 * @param[out] file A pointer to a variable to store the <b>file path</b>
 *
 * @param[out] group A pointer to a variable to store the <b>group name</b>
 *
 * @see edje_object_file_set()
 */
EAPI void             edje_object_file_get        (const Evas_Object *obj, const char **file, const char **group);

/**
 * @brief Gets an <b>EDC data field value</b> from a given Edje
 *        object group.
 *
 * @details This function fetches an EDC data field value, which is declared
 *          on the object's building EDC file, <b>under its group</b>. EDC data
 *          blocks are most commonly used to pass arbitrary parameters from an
 *          application's theme to its code.
 *
 * @since_tizen 2.3
 *
 * They look like the following:
 *
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     data {
 *	 item: "key1" "value1";
 *	 item: "key2" "value2";
 *     }
 *   }
 * }
 * @endcode
 *
 * @remarks EDC data fields always hold @b strings as values, hence the return
 *          type of this function. Check the complete @ref edcref "syntax reference"
 *          for EDC files.
 *
 * @remarks Do not confuse this call with edje_file_data_get(), which
 *          queries for a @b global EDC data field on an EDC declaration file.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] key The data field key string
 * @return The data value string, must not be freed
 *
 * @see edje_object_file_set()
 */
EAPI const char  *edje_object_data_get            (const Evas_Object *obj, const char *key);

/**
 * @brief Gets the (last) file loading error for a given edje object.
 *
 * @details This function is meant to be used after an edje EDJ <b>file
 *          load</b> that takes place with the edje_object_file_set()
 *          function. If that function does not return #EINA_TRUE, one should
 *          check for the reason of failure.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 *
 * @return The Edje loading error, one from:
 * - #EDJE_LOAD_ERROR_NONE
 * - #EDJE_LOAD_ERROR_GENERIC
 * - #EDJE_LOAD_ERROR_DOES_NOT_EXIST
 * - #EDJE_LOAD_ERROR_PERMISSION_DENIED
 * - #EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED
 * - #EDJE_LOAD_ERROR_CORRUPT_FILE
 * - #EDJE_LOAD_ERROR_UNKNOWN_FORMAT
 * - #EDJE_LOAD_ERROR_INCOMPATIBLE_FILE
 * - #EDJE_LOAD_ERROR_UNKNOWN_COLLECTION
 * - #EDJE_LOAD_ERROR_RECURSIVE_REFERENCE
 *
 * @see edje_load_error_str()
 */
EAPI Edje_Load_Error  edje_object_load_error_get  (const Evas_Object *obj);

/**
 * @brief Converts the given edje file load error code into a string
 *        describing it in English.
 *
 * @since_tizen 2.3
 *
 * @remarks edje_object_file_set() is a function that sets an error value later,
 *          which can be fetched with edje_object_load_error_get().
 *          The function in question is meant
 *          to be used in conjunction with the latter, for pretty-printing any
 *          possible error cause.
 *
 * @param[in] error The error code, a value in ::Edje_Load_Error
 * @return A valid string \n
 *         If the given @a error is not
 *         supported, <code>"Unknown error"</code> is returned.
 */
EAPI const char	      *edje_load_error_str	  (Edje_Load_Error error);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Communication_Interface_Signal Edje Communication Interface: Signal
 * @ingroup Edje_Object_Group

 *
 * @brief This group discusses functions that deal with signals.
 *
 * @remarks Edje has two communication interfaces between @b code and @b theme,
 *          signals and messages.
 *
 * @remarks This group has functions that deal with signals. One can either
 *          emit a signal from @b code to @b theme or create handles for
 *          the ones emitted from @b theme. Signals are identified by strings.
 *
 * @{
 */

/**
 * @brief Edje signal callback functions's prototype definition.
 *
 * @c data will have the auxiliary data pointer set at the time the callback
 * registration. @c obj will be a pointer the Edje object where the signal
 * comes from. @c emission will identify the exact signal's emission string and
 * @c source the exact signal's source one.
 */
typedef void         (*Edje_Signal_Cb)          (void *data, Evas_Object *obj, const char *emission, const char *source);

/**
 * @brief Adds a callback for an arriving edje signal, emitted by
 *        a given Ejde object.
 *
 * @since_tizen 2.3
 *
 * @remarks Edje signals are one of the communication interfaces between
 *          @b code and a given edje object's @b theme. With signals, one can
 *          communicate two string values at a time, which are:
 *          - "emission" value: The name of the signal, in general.
 *          - "source" value: A name for the signal's context, in general.
 *
 *          Though there are common uses for the two strings, one is free
 *          to use them however they like.
 *
 * @remarks This function adds a callback function to a signal emitted by @a obj, to
 *          be issued every time an EDC program like the following:
 * @code
 * program {
 *   name: "emit_example";
 *   action: SIGNAL_EMIT "a_signal" "a_source";
 * }
 * @endcode
 *          is run, if @a emission and @a source are given those same values,
 *          here.
 *
 * @remarks Signal callback registration is powerful because @b blobs
 *          may be used to match <b>multiple signals at once</b>. All the @c
 *          *?[\ set of @c fnmatch() operators can be used, both for @a
 *          emission and @a source.
 *
 * @remarks Edje has @b internal signals that it emits, automatically, on
 *          various actions taking place on group parts. For example, the mouse
 *          cursor being moved, pressed, released, etc., over a given part's
 *          area, all generate individual signals.
 *
 *          By using something like
 * @code
 * edje_object_signal_callback_add(obj, "mouse,down,*", "button.*",
 *                                 signal_cb, NULL);
 * @endcode
 *          @c "button.*" being the pattern for the names of parts implementing
 *          buttons on an interface that you would be registering for notifications on
 *          events of mouse buttons being pressed down on either of those parts
 *          (all those events have the @c "mouse,down," common prefix on their
 *          names, with a suffix giving the button number). The actual emission
 *          and source strings of an event are passed as the @a emission
 *          and @a source parameters of the callback function (e.g. @c
 *          "mouse,down,2" and @c "button.close"), for each of those events.
 *
 * @remarks See @ref edcref "the syntax" for EDC files.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] emission The signal's "emission" string
 * @param[in] source The signal's "source" string
 * @param[in] func The callback function to be executed when the signal is
 *             emitted
 * @param[in] data A pointer to the data to pass to @a func
 *
 * @see edje_object_signal_emit() on how to emit edje signals from
 *      the code to an object
 * @see edje_object_signal_callback_del_full()
 */
EAPI void         edje_object_signal_callback_add (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Deletes a signal-triggered callback from an object.
 *
 * @details This function removes a callback that had been previously attached to the
 *          emission of a signal, from the object @a obj. The parameters @a
 *          emission, @a source, and @a func must exactly match those that are passed to
 *          a previous call to edje_object_signal_callback_add(). The data
 *          pointer that is passed to this call is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] emission The emission string
 * @param[in] source The source string
 * @param[in] func The callback function
 * @return The data pointer
 *
 * @see edje_object_signal_callback_add()
 * @see edje_object_signal_callback_del_full()
 *
 */
EAPI void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);

/**
 * @brief Unregisters/deletes a callback set for an arriving edje
 *        signal, emitted by a given Ejde object.
 *
 * @details This function removes a callback that had been previously attached to the
 *          emission of a signal, from the object @a obj. The parameters
 *          @a emission, @a source, @a func, and @a data must exactly match those
 *          that are passed to a previous call to edje_object_signal_callback_add(). The
 *          data pointer that is passed to this call is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @param[in] emission The signal's "emission" string
 * @param[in] source The signal's "source" string
 * @param[in] func The callback function passed on the callback
 *             registration
 * @param[in] data A pointer to be passed as data to @a func
 * @return @a data on success, otherwise @c NULL on errors (or if @a data
 *         has this value)
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del().
 *
 */
EAPI void        *edje_object_signal_callback_del_full(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Sends/emits an edje signal to a given edje object.
 *
 * @details This function sends a signal to the object @a obj. An edje program,
 *          at @a obj's EDC specification level, can respond to a signal by
 *          declaring matching @c 'signal' and @c 'source' fields on its
 *          block (see @ref edcref "the syntax" for EDC files).
 *
 * @since_tizen 2.3
 *
 * As an example,
 * @code
 * edje_object_signal_emit(obj, "a_signal", "");
 * @endcode
 * would trigger a program which has an EDC declaration block like
 * @code
 * program {
 *  name: "a_program";
 *  signal: "a_signal";
 *  source: "";
 *  action: ...
 * }
 * @endcode
 *
 * @param[in] obj A handle to an edje object
 * @param[in] emission The signal's "emission" string
 * @param[in] source The signal's "source" string
 *
 * @see edje_object_signal_callback_add() for more on edje signals.
 */
EAPI void         edje_object_signal_emit         (Evas_Object *obj, const char *emission, const char *source);

/**
 * @brief Gets the extra data passed to callbacks.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @remarks Some callbacks pass extra information. This function gives access to that
 *          extra information. It's somehow like event_info when it comes to smart callbacks.
 *
 * @return The extra data for that callback.
 *
 * @see edje_object_signal_callback_add() for more on edje signals.
 *
 */
EAPI void *       edje_object_signal_callback_extra_data_get(void);

/**
 * @}
 */

/**
 * @defgroup Edje_Animation Edje Animation
 * @ingroup Edje_Group
 *
 * @brief This group discusses functions that deal with animations.
 *
 * @remarks Edje has the ability to animate objects. One can start, stop, play,
 *          pause, freeze, and thaw edje animations using the functions of this section.
 *
 * @{
 */

typedef enum _Edje_Tween_Mode
{
   EDJE_TWEEN_MODE_NONE              = 0,
   EDJE_TWEEN_MODE_LINEAR            = 1,
   EDJE_TWEEN_MODE_SINUSOIDAL        = 2,
   EDJE_TWEEN_MODE_ACCELERATE        = 3,
   EDJE_TWEEN_MODE_DECELERATE        = 4,
   EDJE_TWEEN_MODE_ACCELERATE_FACTOR = 5,
   EDJE_TWEEN_MODE_DECELERATE_FACTOR = 6,
   EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR = 7,
   EDJE_TWEEN_MODE_DIVISOR_INTERP    = 8,
   EDJE_TWEEN_MODE_BOUNCE            = 9,
   EDJE_TWEEN_MODE_SPRING            = 10,
   EDJE_TWEEN_MODE_LAST              = 11,
   EDJE_TWEEN_MODE_MASK              = 0xff,
   EDJE_TWEEN_MODE_OPT_FROM_CURRENT  = (1 << 31)
} Edje_Tween_Mode;

#define PLUGIN

/**
 * @brief Edje Action Type
 */
typedef enum _Edje_Action_Type
{
   EDJE_ACTION_TYPE_NONE                = 0,
   EDJE_ACTION_TYPE_STATE_SET           = 1,
   EDJE_ACTION_TYPE_ACTION_STOP         = 2,
   EDJE_ACTION_TYPE_SIGNAL_EMIT         = 3,
   EDJE_ACTION_TYPE_DRAG_VAL_SET        = 4,
   EDJE_ACTION_TYPE_DRAG_VAL_STEP       = 5,
   EDJE_ACTION_TYPE_DRAG_VAL_PAGE       = 6,
   EDJE_ACTION_TYPE_SCRIPT              = 7,
   EDJE_ACTION_TYPE_FOCUS_SET           = 8,
   EDJE_ACTION_TYPE_RESERVED00          = 9,
   EDJE_ACTION_TYPE_FOCUS_OBJECT        = 10,
   EDJE_ACTION_TYPE_PARAM_COPY          = 11,
   EDJE_ACTION_TYPE_PARAM_SET           = 12,
   EDJE_ACTION_TYPE_SOUND_SAMPLE        = 13, /**< @since 1.1 */
   EDJE_ACTION_TYPE_SOUND_TONE          = 14, /**< @since 1.1 */
#ifdef PLUGIN
   EDJE_ACTION_TYPE_RUN_PLUGIN          = 15,
   EDJE_ACTION_TYPE_LAST                = 16
#else
   EDJE_ACTION_TYPE_LAST                = 15
#endif
} Edje_Action_Type;

/**
 * @brief Sets the edje trasitions' frame time.
 *
 * @details This function sets the edje built-in animations' frame time (thus,
 *          affecting their resolution) by calling
 *          ecore_animator_frametime_set(). This frame time can be retrieved
 *          with edje_frametime_get().
 *
 * @since_tizen 2.3
 *
 * @param[in] t The frame time, in seconds \n
 *          Default value is 1/30.
 *
 * @see edje_frametime_get()
 */
EAPI void         edje_frametime_set              (double t);

/**
 * @brief Gets the edje trasitions' frame time.
 *
 * @details This function returns the edje frame time set by
 *          edje_frametime_set() or the default value of 1/30.
 *
 * @since_tizen 2.3
 *
 * @return The frame time, in seconds
 *
 * @see edje_frametime_set()
 */
EAPI double       edje_frametime_get              (void);

/**
 * @brief Freezes the edje objects.
 *
 * @details This function freezes all edje animations in the current process.
 *
 * @since_tizen 2.3
 *
 * @see edje_object_freeze() to freeze a specific object
 *
 * @see edje_thaw()
 */
EAPI void         edje_freeze                     (void);

/**
 * @brief Thaws the edje objects.
 *
 * @details This function thaws all the edje animations in the current process.
 *
 * @since_tizen 2.3
 *
 * @see edje_object_thaw() to thaw a specific object
 *
 * @see edje_freeze()
 */
EAPI void         edje_thaw                       (void);

/**
 * @brief Sets the edje object to the playing or paused states.
 *
 * @details This function sets the edje object @a obj to the playing or paused
 *          states, depending on the parameter @a play. This has no effect if
 *          the object is already in that state.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @param[in] play The object state (#EINA_TRUE for the playing state,
 *                           #EINA_FALSE for the paused state)
 *
 * @see edje_object_play_get()
 */
EAPI void         edje_object_play_set            (Evas_Object *obj, Eina_Bool play);

/**
 * @brief Gets the edje object's state.
 *
 * @details This function gets whether an edje object is playing or not. This state
 *          is set by edje_object_play_set().
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @return #EINA_FALSE if the object is not connected, its @c delete_me flag
 *         is set, or it is in the paused state; otherwise #EINA_TRUE if the object is in playing
 *         state.
 *
 * @see edje_object_play_set()
 */
EAPI Eina_Bool    edje_object_play_get            (const Evas_Object *obj);

/**
 * @brief Sets the object's animation state.
 *
 * @details This function starts or stops an edje object's animation. The
 *          information about whether it's stopped can be retrieved by
 *          edje_object_animation_get().
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @param[in] on The animation state \n
 *           If #EINA_TRUE the object starts,
 *           otherwise #EINA_FALSE to stop it
 *
 * @see edje_object_animation_get()
 */
EAPI void         edje_object_animation_set       (Evas_Object *obj, Eina_Bool on);

/**
 * @brief Gets the edje object's animation state.
 *
 * @details This function gets whether the animation is stopped or not. The
 *          animation state is set by edje_object_animation_set().
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @return #EINA_FALSE on error or if the object is not animated,
 *         otherwise #EINA_TRUE if it is animated
 *
 * @see edje_object_animation_set()
 */
EAPI Eina_Bool    edje_object_animation_get       (const Evas_Object *obj);

/**
 * @brief Freezes the edje object.
 *
 * @details This function puts all changes on hold. Successive freezes are
 *          nested, requiring an equal number of thaws.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @return The frozen state, otherwise @c 0 on error
 *
 * @see edje_object_thaw()
 */
EAPI int          edje_object_freeze                  (Evas_Object *obj);

/**
 * @brief Thaws the edje object.
 *
 * @details This function thaws the given edje object.
 *
 * @since_tizen 2.3
 *
 * @remarks If sucessives freezes are done, an equal number of
 *          thaws are required.
 *
 * @param[in] obj A handle to an edje object
 * @return The frozen state, otherwise @c 0 if the object is not frozen or on error
 *
 * @see edje_object_freeze()
 */
EAPI int          edje_object_thaw                    (Evas_Object *obj);

/**
 * @brief Gets the state of the edje part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[out] val_ret The part state name
 *
 * @return The part state:\n
 *         "default" for the default state\n
 *         "" for other states
 */
EAPI const char      *edje_object_part_state_get      (const Evas_Object *obj, const char *part, double *val_ret);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Geometry_Group Edje Geometry
 * @ingroup Edje_Object_Group
 *
 * @brief This group discusses functions that deal with an object's geometry.
 *
 * @remarks By geometry we mean size and position. So in this group there are
 *          functions to manipulate an object's geometry or retrieve information
 *          about it.
 *
 * @remarks Keep in mind that by changing an object's geometry, it may affect
 *          its appearance on the screen of the parts inside. Most of the time
 *          that is what you want.
 *
 * @{
 */

/**
 * @brief Gets the minimum size specified, as an EDC property, for a
 *        given edje object.
 *
 * @details This function retrieves the @a obj object's minimum size values,
 *          <b>as declared in its EDC group definition</b>. Minimum size
 *          groups have the following syntax:
 *
 * @since_tizen 2.3
 *
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     min: 100 100;
 *   }
 * }
 * @endcode
 *
 *          where one declares a minimum size of 100 pixels for both width and
 *          height. Those are (hint) values that should be respected when the
 *          given object/group is to be controlled by a given container object
 *          (e.g. an edje object being "swallowed" into a given @c SWALLOW
 *          type part, as in edje_object_part_swallow()). Check the complete
 *          @ref edcref "syntax reference" for EDC files.
 *
 * @remarks If the @c min EDC property is not declared for @a obj, this
 *          call returns the value 0, for each axis.
 *
 * @remarks On failure, this function makes all non-@c NULL size
 *          pointers' pointed variables as zero.
 *
 *
 * @param[in] obj A handle to an edje object
 * @param[out] minw A pointer to a variable to store the minimum width
 * @param[out] minh A pointer to a variable to store the minimum height
 *
 *
 * @see edje_object_size_max_get()
 */
EAPI void         edje_object_size_min_get            (const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

/**
 * @brief Sets whether Edje is going to update size hints on itself.
 *
 * @since_tizen 2.3
 *
 * @remarks By default Edje doesn't set size hints on itself. With this function
 *          call, it does so if @a update is @c true. Be careful, it costs a lot to
 *          trigger this feature as it recalculates the object every time it makes
 *          sense to be sure that its minimal size hint is always accurate.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] update A boolean value that indicates whether to update the size hints
 *
 */
EAPI void edje_object_update_hints_set(Evas_Object *obj, Eina_Bool update);

/**
 * @brief Gets whether Edje is going to update size hints on itself.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 * @return @c true if it does, otherwise @c false if it doesn't
 */
EAPI Eina_Bool edje_object_update_hints_get(Evas_Object *obj);

/**
 * @brief Gets the maximum size specified, as an EDC property, for a
 *        given edje object.
 *
 * @details This function retrieves the @a obj object's maximum size values,
 *          <b>as declared in its EDC group definition</b>. Maximum size
 *          groups have the following syntax:
 *
 * @since_tizen 2.3
 *
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     max: 100 100;
 *   }
 * }
 * @endcode
 *
 *          where one declares a maximum size of 100 pixels for both width and
 *          height. Those are (hint) values that should be respected when the
 *          given object/group is to be controlled by a given container object
 *          (e.g. an edje object being "swallowed" into a given @c SWALLOW
 *          type part, as in edje_object_part_swallow()). Check the complete
 *          @ref edcref "syntax reference" for EDC files.
 *
 * @remarks If the @c max EDC property is not declared for @a obj, this
 *          call returns the maximum size that a given edje object may have, for
 *          each axis.
 *
 * @remarks On failure, this function makes all non-@c NULL size
 *          pointers' pointed variables as zero.
 *
 * @param[in] obj A handle to an edje object
 * @param[out] maxw A pointer to a variable to store the maximum width
 * @param[out] maxh A pointer to a variable to store the maximum height
 *
 * @see edje_object_size_min_get()
 */
EAPI void         edje_object_size_max_get            (const Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh);

/**
 * @brief Forces a Size/Geometry calculation.
 *
 * @details This forces the object @a obj to recalculate the layout regardless of
 *          freeze/thaw.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 *
 */
EAPI void         edje_object_calc_force              (Evas_Object *obj);

/**
 * @brief Calculates the minimum required size for a given edje object.
 *
 * @since_tizen 2.3
 *
 * @remarks This call works exactly like edje_object_size_min_restricted_calc(),
 *          with the last two arguments set to 0. Please refer to its
 *          documentation.
 *
 * @param[in] obj A handle to an edje object
 * @param[out] minw A pointer to a variable to store the minimum
 *			   required width
 * @param[out] minh A pointer to a variable to store the minimum
 *			   required height
 */
EAPI void         edje_object_size_min_calc           (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

/**
 * @brief Calculates the geometry of the region, relative to a given edje
 *        object's area, <b>occupied by all parts in the object</b>.
 *
 * @details This function gets the geometry of the rectangle that is equal to the area
 *          required to group all parts in @a obj's group/collection. The @a x
 *          and @a y coordinates are relative to the top left corner of the
 *          whole @a obj object's area. Parts placed out of the group's
 *          boundaries are also taken into account, so that @a x and @a y
 *          <b>may be negative</b>.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the geometry components you're not
 *          interested in, they are ignored by the function.
 *
 * @remarks On failure, this function makes all non-@c NULL geometry
 *          pointers' pointed variables as zero.
 *
 * @param[in] obj A handle to an edje object
 * @param[out] x A pointer to a variable to store the parts region's
 *			x coordinate
 * @param[out] y A pointer to a variable to store the parts region's
 *			y coordinate
 * @param[out] w A pointer to a variable to store the parts region's
 *			width
 * @param[out] h A pointer to a variable to store the parts region's
 *			height
 * @return #EINA_TRUE if success, otherwise #EINA_FALSE
 */
EAPI Eina_Bool    edje_object_parts_extends_calc      (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Calculates the minimum required size for a given edje object.
 *
 * @since_tizen 2.3
 *
 * @remarks This call triggers an internal recalculation of all the parts of
 *          the @a obj object, in order to return its minimum required
 *          dimensions of width and height. The user might choose to @b impose
 *          those minimum sizes, making the resulting calculation to get to values
 *          equal or bigger than @a restrictedw and @a restrictedh, for width and
 *          height, respectively.
 *
 * @remarks At the end of this call, @a obj @b isn't automatically
 *          resized to new dimensions, but just returns the calculated
 *          sizes. The caller is the one who changes its geometry.
 *
 * @remarks Be advised that invisible parts in @a obj @b are taken
 *          into account in this calculation.
 *
 * @param[in] obj A handle to an edje object
 * @param[out] minw A pointer to a variable to store the minimum required width
 *
 * @param[out] minh A pointer to a variable to store the minimum required width
 *
 * @param[in] restrictedw Minimum value for an object's calculated (minimum) width
 *
 * @param[in] restrictedh Minimum value for an object's calculated (minimum) height
 *
 */
EAPI void         edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Part Edje Part
 * @ingroup Edje_Object_Group
 *
 * @brief This group discusses functions that deal with layout components.
 *
 * @remarks Parts are layout components, but as a layout, they are objects too.
 *
 *          There are several types of parts, these types can be divided into two
 *          main categories, the first being containers. Containers are parts
 *          that are in effect a group of elements. The second group is that of
 *          the elements, these part types may not contain others.
 *
 * @remarks This section has some functions specific for some types and others that
 *          could be applied to any type.
 *
 * @{
 */

/**
 * @brief Edje Part Type
 */
typedef enum _Edje_Part_Type
{
   EDJE_PART_TYPE_NONE      = 0,
   EDJE_PART_TYPE_RECTANGLE = 1,
   EDJE_PART_TYPE_TEXT      = 2,
   EDJE_PART_TYPE_IMAGE     = 3,
   EDJE_PART_TYPE_SWALLOW   = 4,
   EDJE_PART_TYPE_TEXTBLOCK = 5,
   EDJE_PART_TYPE_GRADIENT  = 6,
   EDJE_PART_TYPE_GROUP     = 7,
   EDJE_PART_TYPE_BOX       = 8,
   EDJE_PART_TYPE_TABLE     = 9,
   EDJE_PART_TYPE_EXTERNAL  = 10,
   EDJE_PART_TYPE_PROXY     = 11,
   EDJE_PART_TYPE_SPACER    = 12, /**< @since 1.7 */
   EDJE_PART_TYPE_LAST      = 13
} Edje_Part_Type;

/**
 * @brief Checks whether an edje part exists in a given edje object's group
 *        definition.
 *
 * @details This function returns if a given part exists in the edje group
 *          bound to object @a obj (with edje_object_file_set()).
 *
 * @since_tizen 2.3
 *
 * @remarks This call is useful, for example, when one could expect a
 *          given GUI element, depending on the @b theme applied to @a obj.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] part The part's name to check for existence in @a obj's
 *             group
 * @return #EINA_TRUE, if the edje part exists in @a obj's group,
 *         otherwise #EINA_FALSE (and on errors)
 */
EAPI Eina_Bool    edje_object_part_exists             (const Evas_Object *obj, const char *part);

/**
 * @brief Get a handle to the Evas object implementing a given edje
 *        part, in an edje object.
 *
 * @details This function gets a pointer of the Evas object corresponding to a
 *          given part in the @a obj object's group.
 *
 * @since_tizen 2.3
 *
 * @remarks You should @b never modify the state of the returned object (with
 *          @c evas_object_move() or @c evas_object_hide() for example),
 *          because it's meant to be managed by Edje, solely. You are safe to
 *          query information about its current state (with @c
 *          evas_object_visible_get() or @c evas_object_color_get() for
 *          example), though.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] part The edje part's name
 * @return A pointer to the Evas object implementing the given part,
 *         otherwise @c NULL on failure (e.g. the given part doesn't exist)
 */
EAPI const Evas_Object *edje_object_part_object_get   (const Evas_Object *obj, const char *part);

/**
 * @brief Gets the geometry of a given edje part, in a given edje
 *        object's group definition, <b>relative to the object's area</b>.
 *
 * @details This function gets the geometry of an edje part within its
 *          group. The @a x and @a y coordinates are relative to the top left
 *          corner of the whole @a obj object's area.
 *
 * @since_tizen 2.3
 *
 * @remarks Use @c NULL pointers on the geometry components you're not
 *          interested in, they are ignored by the function.
 *
 * @remarks On failure, this function makes all non-@c NULL geometry
 *          pointers' pointed variables as zero.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] part The edje part's name
 * @param[out] x A pointer to a variable to store the part's x
 *          coordinate
 * @param[out] y A pointer to a variable to store the part's y
 *          coordinate
 * @param[out] w A pointer to a variable to store the part's width
 * @param[out] h A pointer to a variable to store the part's height
 * @return #EINA_TRUE if success, otherwise #EINA_FALSE
 *
 */
EAPI Eina_Bool    edje_object_part_geometry_get       (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Gets a list of all accessibility part names.
 *
 * @since 1.7.0
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @return A list of all accessibility part names on @a obj
 */
EAPI Eina_List    *edje_object_access_part_list_get   (const Evas_Object *obj);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Text Edje Text Part
 * @ingroup Edje_Object_Part
 *
 * @brief This group discusses functions that deal with parts of type text.
 *
 * @remarks Text is an element type for parts. It's basic functionality is to
 *          display a string on the layout, but a lot more can be done
 *          with texts, like string selection, setting the cursor, and including
 *          an input panel, where one can set a virtual keyboard to handle
 *          keyboard entry easily.
 *
 * @{
 */

typedef enum _Edje_Text_Effect
{
#define EDJE_TEXT_EFFECT_MASK_BASIC 0xf
#define EDJE_TEXT_EFFECT_BASIC_SET(x, s) \
   do { x = ((x) & ~EDJE_TEXT_EFFECT_MASK_BASIC) | (s); } while (0)
   EDJE_TEXT_EFFECT_NONE                = 0,
   EDJE_TEXT_EFFECT_PLAIN               = 1,
   EDJE_TEXT_EFFECT_OUTLINE             = 2,
   EDJE_TEXT_EFFECT_SOFT_OUTLINE        = 3,
   EDJE_TEXT_EFFECT_SHADOW              = 4,
   EDJE_TEXT_EFFECT_SOFT_SHADOW         = 5,
   EDJE_TEXT_EFFECT_OUTLINE_SHADOW      = 6,
   EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW = 7,
   EDJE_TEXT_EFFECT_FAR_SHADOW          = 8,
   EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW     = 9,
   EDJE_TEXT_EFFECT_GLOW                = 10,
// ******************************************************************
// ******************* TIZEN ONLY - START****************************
// ******************************************************************
// 2013.11.06 : Font effect for tizen.
   EDJE_TEXT_EFFECT_TIZEN_GLOW_SHADOW   = 11,
   EDJE_TEXT_EFFECT_TIZEN_SOFT_GLOW_SHADOW = 12,
   EDJE_TEXT_EFFECT_TIZEN_SHADOW        = 13,
// ******************************************************************
// ******************* TIZEN ONLY - END  ****************************
// ******************************************************************
   EDJE_TEXT_EFFECT_LAST                = 14,

#define EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION (0x7 << 4)
#define EDJE_TEXT_EFFECT_SHADOW_DIRECTION_SET(x, s) \
   do { x = ((x) & ~EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION) | (s); } while (0)
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_RIGHT = (0x0 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM       = (0x1 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_LEFT  = (0x2 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_LEFT         = (0x3 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_LEFT     = (0x4 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP          = (0x5 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_RIGHT    = (0x6 << 4),
   EDJE_TEXT_EFFECT_SHADOW_DIRECTION_RIGHT        = (0x7 << 4)
} Edje_Text_Effect;

/**
 * @brief Edje text change callback prototype
 */
typedef void         (*Edje_Text_Change_Cb)     (void *data, Evas_Object *obj, const char *part);

/**
 * @brief Sets the object text callback.
 *
 * @details This function sets the callback to be called when the text changes.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] func The callback function to handle the text change
 * @param[in] data The data associated with the callback function
 */
EAPI void         edje_object_text_change_cb_set      (Evas_Object *obj, Edje_Text_Change_Cb func, void *data);

/**
 * @brief Sets the text for an object part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas Object handle
 * @param[in] part The part name
 * @param[in] text The text string
 *
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE
 */
EAPI Eina_Bool    edje_object_part_text_set           (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Sets the text for an object part, but converts HTML escapes to UTF8.
 *
 * @details This converts the given string @a text to UTF8 assuming it contains HTML
 *          style escapes like "&amp;" and "&copy;", IF the part is of type TEXT,
 *          as opposed to TEXTBLOCK.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas Object handle
 * @param[in] part The part name
 * @param[in] text The text string
 *
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE
 */
EAPI Eina_Bool    edje_object_part_text_escaped_set   (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Gets the text of the object part.
 *
 * @details This function returns the text associated with the object part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @return The text string
 *
 * @see edje_object_part_text_set()
 */
EAPI const char  *edje_object_part_text_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the style of the textblock part
 *
 * @details This function sets the style associated with the textblock part
 *
 * @since_tizen 2.3
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] style The style to set (textblock conventions)
 */
EAPI void edje_object_part_text_style_user_push(Evas_Object *obj, const char *part, const char *style);

/**
 * @brief Gets the text of the object part.
 *
 * @details This function returns the style associated with the textblock part.
 *
 * @since 1.2.0
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @return The text string
 */
EAPI const char *edje_object_part_text_style_user_peek(const Evas_Object *obj, const char *part);

/**
 * @brief Deletes the top style form of the the user style stack.
 *
 * @since 1.2.0
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 */
EAPI void edje_object_part_text_style_user_pop(Evas_Object *obj, const char *part);

/**
 * @brief Sets the raw (non escaped) text for an object part.
 *
 * @details This funciton does escape for you if it is a TEXTBLOCK part, that is,
 *          if text contains tags, these tags are not interpreted/parsed by the TEXTBLOCK.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas Object handle
 * @param[in] part The part name
 * @param[in] text_to_escape The text string
 * @return #EINA_TRUE if success, otherwise #EINA_FALSE
 *
 * @see edje_object_part_text_unescaped_get()
 */
EAPI Eina_Bool    edje_object_part_text_unescaped_set (Evas_Object *obj, const char *part, const char *text_to_escape);

/**
 * @brief Gets the text of the object part, without escaping.
 *
 * @details This function is the counterpart of
 *          edje_object_part_text_unescaped_set(). Note that the
 *          result is newly allocated memory and should be released with free()
 *          when done.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return The @b allocated text string without escaping, otherwise @c NULL on errors
 *
 * @see edje_object_part_text_unescaped_set()
 */
EAPI char        *edje_object_part_text_unescaped_get (const Evas_Object *obj, const char *part);

/**
 * @brief Inserts text for an object part.
 *
 * @details This function inserts the text for an object part just before the
 *          cursor position.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas Object handle
 * @param[in] part The part name
 * @param[in] text The text string
 *
 */
EAPI void             edje_object_part_text_insert                  (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Inserts text for an object part.
 *
 * @details This function inserts the text for an object part at the end. It does not
 *          move the cursor.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas Object handle
 * @param[in] part The part name
 * @param[in] text The text string
 */
EAPI void             edje_object_part_text_append(Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Gets a list of char anchor names.
 *
 * @details This function returns a list of char anchor names.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @return The list of anchors (const char *), do not modify them
 *
 */
EAPI const Eina_List *edje_object_part_text_anchor_list_get         (const Evas_Object *obj, const char *part);

/**
 * @brief Gets a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * @details This function returns a list of Evas_Textblock_Rectangle anchor
 *          rectangles.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] anchor The anchor name
 *
 * @return The list of anchor rects (const Evas_Textblock_Rectangle *),
 *         do not modify them \n
 *         Geometry is relative to the entry part.
 *
 */
EAPI const Eina_List *edje_object_part_text_anchor_geometry_get     (const Evas_Object *obj, const char *part, const char *anchor);

/**
 * @brief Gets a list of char item names.
 *
 * @details This function returns a list of char item names.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @return The list of items (const char *), do not modify them
 *
 */
EAPI const Eina_List *edje_object_part_text_item_list_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Gets the item geometry.
 *
 * @details This function returns a list of Evas_Textblock_Rectangle item
 *          rectangles.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] item The item name
 * @param[out] cx The item's x coordinate (relative to the entry part)
 * @param[out] cy The item's y coordinate (relative to the entry part)
 * @param[out] cw The item width
 * @param[out] ch The item height
 *
 * @return @c 1 if the item exists, otherwise @c 0 if it does not
 *
 */
EAPI Eina_Bool        edje_object_part_text_item_geometry_get       (const Evas_Object *obj, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);

/**
 * @brief Inserts text as if the user has inserted it.
 *
 * @since 1.2.0
 *
 * @since_tizen 2.3
 *
 * @remarks This means it actually registers as a change and emits signals, triggers
 *          callbacks as appropriate.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] text The text string
 */
EAPI void             edje_object_part_text_user_insert        (const Evas_Object *obj, const char *part, const char *text);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Edje_Text_Selection Edje Text Selection
 * @ingroup Edje_Part_Text
 *
 * @brief This group discusses functions that deal with selection of text parts.
 *
 * @remarks Selection is a known functionality for texts in the whole computational
 *          world. It is a block of text marked for further manipulation.
 *
 * @remarks Edje is responsible for handling this functionality through the
 *          following functions.
 *
 * @{
 */

/**
 * @brief Gets the selection text of the object part.
 *
 * @details This function returns selection text of the object part.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return The text string
 *
 * @see edje_object_part_text_select_all()
 * @see edje_object_part_text_select_none()
 */
EAPI const char      *edje_object_part_text_selection_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the selection to be none.
 *
 * @details This function sets the selection text to be none.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 */
EAPI void             edje_object_part_text_select_none             (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the selection to be everything.
 *
 * @details This function selects all the text of the object of the part.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 */
EAPI void             edje_object_part_text_select_all              (const Evas_Object *obj, const char *part);

/**
 * @internal
 * @remarks Tizen only feature
 *
 * @brief Sets the selection to be word pointed by the current cursor.
 *
 * @details This function selects the word pointed by the cursor of the object of the part.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 */
EAPI void             edje_object_part_text_select_word              (const Evas_Object *obj, const char *part);

/**
 * @brief Enables selection if the entry is an EXPLICIT selection mode
 *        type.
 *
 * @remarks The default is to @b not allow selection. This function only affects user
 *          selection, functions such as edje_object_part_text_select_all() and
 *          edje_object_part_text_select_none() are not affected.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] allow If #EINA_TRUE it enables selection, otherwise #EINA_FALSE
 *
 */
EAPI void             edje_object_part_text_select_allow_set        (const Evas_Object *obj, const char *part, Eina_Bool allow);

/**
 * @brief Aborts any selection action on a part.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 */
EAPI void             edje_object_part_text_select_abort            (const Evas_Object *obj, const char *part);

/**
 * @brief Starts selecting at the current cursor position.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 */
EAPI void             edje_object_part_text_select_begin            (const Evas_Object *obj, const char *part);

/**
 * @brief Extends the current selection to the current cursor position.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 */
EAPI void             edje_object_part_text_select_extend           (const Evas_Object *obj, const char *part);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Edje_Text_Cursor Edje Text Cursor
 * @ingroup Edje_Part_Text
 *
 * @brief This group discusses functions that deal with a cursor in text parts.
 *
 * @remarks Cursor is a known functionality for texts in the whole computational
 *          world. It marks a position in the text from where one may want
 *          to make an insertion, deletion, or selection.
 *
 * @remarks Edje is responsible for handling this functionality through the
 *          following functions.
 *
 * @{
 */

typedef enum _Edje_Cursor
{
   EDJE_CURSOR_MAIN,
   EDJE_CURSOR_SELECTION_BEGIN,
   EDJE_CURSOR_SELECTION_END,
   EDJE_CURSOR_PREEDIT_START,
   EDJE_CURSOR_PREEDIT_END,
   EDJE_CURSOR_USER,
   EDJE_CURSOR_USER_EXTRA,
   // more later
} Edje_Cursor;

/**
 * @brief Advances the cursor to the next cursor position.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The edje cursor to advance
 *
 * @see evas_textblock_cursor_char_next
 */
EAPI Eina_Bool        edje_object_part_text_cursor_next                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the previous char
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @see evas_textblock_cursor_char_prev
 */
EAPI Eina_Bool        edje_object_part_text_cursor_prev                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the char above the current cursor position.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_up                   (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the char below the current cursor position.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_down                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the beginning of the text part
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @see evas_textblock_cursor_paragraph_first
 */
EAPI void             edje_object_part_text_cursor_begin_set            (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the end of the text part.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @see evas_textblock_cursor_paragraph_last
 */
EAPI void             edje_object_part_text_cursor_end_set              (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Copies the cursor to another cursor.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] src The cursor to copy from
 * @param[in] dst The cursor to copy to
 */
EAPI void             edje_object_part_text_cursor_copy                 (Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst);

/**
 * @brief Moves the cursor to the beginning of the line.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @see evas_textblock_cursor_line_char_first
 */
EAPI void             edje_object_part_text_cursor_line_begin_set       (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the end of the line.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The edje cursor to work on
 *
 * @see evas_textblock_cursor_line_char_last
 */
EAPI void             edje_object_part_text_cursor_line_end_set         (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Sets the given cursor to an X,Y position.
 *
 * @remarks This is frequently used with the user cursor.
 *
 * @param[in] obj An edje object
 * @param[in] part The part containing the object
 * @param[in] cur The cursor to adjust
 * @param[in] x The X Coordinate
 * @param[in] y The Y Coordinate
 * @return @c true on success, @c false on error
 */
EAPI Eina_Bool        edje_object_part_text_cursor_coord_set            (Evas_Object *obj, const char *part, Edje_Cursor cur, Evas_Coord x, Evas_Coord y);

/**
 * @brief Gets whether the cursor points to a format.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The cursor to adjust
 * @return #EINA_TRUE if it's true, otherwise #EINA_FALSE
 *
 * @see evas_textblock_cursor_is_format
 */
EAPI Eina_Bool        edje_object_part_text_cursor_is_format_get        (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @internal
 * @remarks Tizen only feature
 *
 * @brief Checks whether (x,y) is on top of the selected-text.
 * @details This function returns @c true if (x,y) is on top of the selected-text.
 *
 * @param[in] obj An edje object
 * @param[in] part The part containing the object.
 * @param[in] x The X Coordinate
 * @param[in] y The Y Coordinate
 * @return @c true if (x,y) is on top of the selected-text, otherwise @c false
 */
EAPI Eina_Bool
edje_object_part_text_xy_in_selection_get(Evas_Object *obj, const char *part, Evas_Coord x, Evas_Coord y);

/**
 * @brief Gets @c true if the cursor points to a visible format.
 *        For example \\t, \\n, item and so on.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The cursor to adjust
 *
 * @see  evas_textblock_cursor_format_is_visible_get
 */
EAPI Eina_Bool        edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Gets the content (char) at the cursor position.
 * 
 * @remarks You must free the return value (if not @c NULL) after you are done with it.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The cursor to use
 * @return The character string pointed to, (may be a multi-byte utf8 sequence) which is terminated by a @c null byte.
 *
 * @see evas_textblock_cursor_content_get
 */
EAPI char            *edje_object_part_text_cursor_content_get          (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Sets the cursor position to the given value.
 *
 * @since 1.1.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The cursor to move
 * @param[in] pos The position of the cursor
 */
EAPI void             edje_object_part_text_cursor_pos_set              (Evas_Object *obj, const char *part, Edje_Cursor cur, int pos);

/**
 * @brief Gets the current position of the cursor.
 *
 * @since 1.1.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] cur The cursor to get the position
 * @return The cursor position
 */
EAPI int              edje_object_part_text_cursor_pos_get              (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Gets the cursor geometry of the part relative to the edje
 *        object.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[out] x The cursor's X position
 * @param[out] y The cursor's Y position
 * @param[out] w The cursor's width
 * @param[out] h The cursor's height
 *
 */
EAPI void             edje_object_part_text_cursor_geometry_get     (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Edje_Text_Entry Edje Text Entry
 * @ingroup Edje_Part_Text
 *
 * @brief This group discusses functions that deal with text entries.
 *
 * @remarks In Edje it's possible to use a text part as an entry so that the user is
 *          able to make inputs of text. To do so, the text part must be set
 *          with an input panel that works as a virtual keyboard.
 *
 * @remarks Some of the effects can be applied to the entered text and also plenty
 *          actions can be performed after any input.
 *
 * @remarks Use the functions of this section to handle the user input of text.
 *
 * @{
 */

struct _Edje_Entry_Change_Info
{
   union {
        struct {
             const char *content;
             size_t pos;
             size_t plain_length; /**< Number of cursor positions represented
                                     in the content */
        } insert;
        struct {
             const char *content;
             size_t start, end;
        } del;
   } change;
   Eina_Bool insert : 1; /**< @c true if the "change" union's "insert" is valid */
   Eina_Bool merge : 1; /**< @c true if it can be merged with the previous one. Used, for example, with insertion when something is already selected */
};

/**
 * @since 1.1.0
 */
typedef struct _Edje_Entry_Change_Info        Edje_Entry_Change_Info;

typedef enum _Edje_Text_Filter_Type
{
   EDJE_TEXT_FILTER_TEXT = 0,
   EDJE_TEXT_FILTER_FORMAT = 1,
   EDJE_TEXT_FILTER_MARKUP = 2
} Edje_Text_Filter_Type;

typedef enum _Edje_Text_Autocapital_Type
{
   EDJE_TEXT_AUTOCAPITAL_TYPE_NONE,
   EDJE_TEXT_AUTOCAPITAL_TYPE_WORD,
   EDJE_TEXT_AUTOCAPITAL_TYPE_SENTENCE,
   EDJE_TEXT_AUTOCAPITAL_TYPE_ALLCHARACTER
} Edje_Text_Autocapital_Type;

/**
 * @brief Edje Input Panel Language
 */
typedef enum _Edje_Input_Panel_Lang
{
   EDJE_INPUT_PANEL_LANG_AUTOMATIC,    /**< Automatic @since 1.2 */
   EDJE_INPUT_PANEL_LANG_ALPHABET      /**< Alphabet @since 1.2 */
} Edje_Input_Panel_Lang;

/**
 * @brief Edje Input Panel Return Key Type
 */
typedef enum _Edje_Input_Panel_Return_Key_Type
{
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT, /**< Default @since 1.2 */
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_DONE,    /**< Done @since 1.2 */
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_GO,      /**< Go @since 1.2 */
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_JOIN,    /**< Join @since 1.2 */
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN,   /**< Login @since 1.2 */
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_NEXT,    /**< Next @since 1.2 */
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH,  /**< Search or magnifier icon @since 1.2 */
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_SEND,    /**< Send @since 1.2 */
   EDJE_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN   /**< Sign-in @since 1.8 */
} Edje_Input_Panel_Return_Key_Type;

/**
 * @brief Edje Input Panel Layout
 */
typedef enum _Edje_Input_Panel_Layout
{
   EDJE_INPUT_PANEL_LAYOUT_NORMAL,          /**< Default layout */
   EDJE_INPUT_PANEL_LAYOUT_NUMBER,          /**< Number layout */
   EDJE_INPUT_PANEL_LAYOUT_EMAIL,           /**< Email layout */
   EDJE_INPUT_PANEL_LAYOUT_URL,             /**< URL layout */
   EDJE_INPUT_PANEL_LAYOUT_PHONENUMBER,     /**< Phone number layout */
   EDJE_INPUT_PANEL_LAYOUT_IP,              /**< IP layout */
   EDJE_INPUT_PANEL_LAYOUT_MONTH,           /**< Month layout */
   EDJE_INPUT_PANEL_LAYOUT_NUMBERONLY,      /**< Number only layout */
   EDJE_INPUT_PANEL_LAYOUT_INVALID,         /**< Never use this */
   EDJE_INPUT_PANEL_LAYOUT_HEX,             /**< Hexadecimal layout @since 1.2 */
   EDJE_INPUT_PANEL_LAYOUT_TERMINAL,        /**< Command-line terminal layout including esc, alt, ctrl key, snd so on (no auto-correct, no auto-capitalization) @since 1.2 */
   EDJE_INPUT_PANEL_LAYOUT_PASSWORD,        /**< Like normal, but no auto-correct, no auto-capitalization. @since 1.2 */
   EDJE_INPUT_PANEL_LAYOUT_DATETIME,        /**< Date and time layout @since 1.8 */
   EDJE_INPUT_PANEL_LAYOUT_EMOTICON         /**< Emoticon layout @since 1.10 */
} Edje_Input_Panel_Layout;


/**
 * @brief Edje Input Hints
 */
typedef enum
{
   EDJE_INPUT_HINT_NONE                = 0,        /**< No active hints @since 1.12 */
   EDJE_INPUT_HINT_AUTO_COMPLETE       = 1 << 0,   /**< suggest word auto completion @since 1.12 */
   EDJE_INPUT_HINT_SENSITIVE_DATA      = 1 << 1,   /**< typed text should not be stored. @since 1.12 */
} Edje_Input_Hints;

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

typedef void         (*Edje_Text_Filter_Cb)     (void *data, Evas_Object *obj, const char *part, Edje_Text_Filter_Type type, char **text);
typedef void         (*Edje_Markup_Filter_Cb)   (void *data, Evas_Object *obj, const char *part, char **text);
typedef Evas_Object *(*Edje_Item_Provider_Cb)   (void *data, Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Shows the last character in the password mode.
 *
 * @details This function enables the last input to be visible when in the password mode for a few seconds
 *          or until the next input is entered.
 *
 * @remarks The time out value is obtained by the edje_password_show_last_timeout_set function.
 *
 * @param[in] password_show_last If #EINA_TRUE it enables the last character shown in the password mode
 *
 * @see edje_password_show_last_timeout_set()
 */
EAPI void edje_password_show_last_set(Eina_Bool password_show_last);

/**
 * @brief Sets the timeout value for the last shown input in the password mode.
 *
 * @details This function sets the time out value for which the last input entered in the password
 *          mode is visible.
 *
 * @remarks This value can be used only when last shown input is set in the password mode.
 *
 * @param[in] password_show_last_timeout The timeout value
 *
 * @see edje_password_show_last_set()
 *
 */
EAPI void edje_password_show_last_timeout_set(double password_show_last_timeout);

/**
 * @brief Sets the RTL orientation for this object.
 *
 * @since 1.1.0
 *
 * @param[in] obj A handle to an edje object
 * @param[in] rtl A new flag value (#EINA_TRUE/#EINA_FALSE)
 */
EAPI void         edje_object_mirrored_set        (Evas_Object *obj, Eina_Bool rtl);

/**
 * @brief Gets the RTL orientation for this object.
 *
 * @since 1.1.0
 *
 * @remarks You can set the RTL orientation explicitly with edje_object_mirrored_set.
 *
 * @param[in] obj A handle to an edje object
 * @return #EINA_TRUE if the flag is set, otherwise #EINA_FALSE if it is not
 */
EAPI Eina_Bool    edje_object_mirrored_get        (const Evas_Object *obj);

/**
 * @brief Sets the function that provides item objects for named items in an edje entry text.
 *
 * @remarks Item objects may be deleted any time by Edje, and are deleted when the
 *          edje object is deleted (or the file is set to a new file).
 *
 * @param[in] obj A valid Evas Object handle
 * @param[in] func The function to call (or @c NULL to disable) to get item objects
 * @param[in] data The data pointer to pass to the @a func callback
 */
EAPI void         edje_object_item_provider_set       (Evas_Object *obj, Edje_Item_Provider_Cb func, void *data);

/**
 * @brief Resets the input method context if needed.
 *
 * @since 1.2.0
 *
 * @remarks This can be necessary in case modifying the buffer would confuse an on-going input method behavior.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 */
EAPI void             edje_object_part_text_imf_context_reset           (const Evas_Object *obj, const char *part);

/**
 * @brief Gets the input method context in the entry.
 *
 * @since 1.2.0
 *
 * @remarks If ecore_imf is not available when Edje is compiled, this function returns @c NULL,
 *          otherwise the returned pointer is an Ecore_IMF *.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @return The input method context (Ecore_IMF_Context *) in the entry 
 */
EAPI void              *edje_object_part_text_imf_context_get           (const Evas_Object *obj, const char *part);

// ******************************************************************
// ******************* TIZEN ONLY - START****************************
// ******************************************************************
/**
 * @internal
 * @brief Gets the current position of the selection.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param x A pointer to a variable to select the part's x
 *          coordinate
 * @param y A pointer to a variable to select the part's y
 *          coordinate
 * @param w A pointer to a variable to select the part's width
 * @param h A pointer to a variable to select the part's height
 * @return #EINA_TRUE if the part is selected, otherwise #EINA_FALSE
 */
EAPI Eina_Bool        edje_object_part_text_selection_geometry_get     (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
// ******************************************************************
// ******************* TIZEN ONLY - END  ****************************
// ******************************************************************

/**
 * @internal
 * @remarks Tizen only feature
 *
 * @brief Gets the number of lines occupied by the selection.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The number of lines occupied by the selection
 */
EAPI unsigned long   edje_object_part_text_selection_lines_number_get(const Evas_Object *obj, const char *part);

/**
 * @internal
 * @remarks Tizen only feature
 *
 * @brief Gets the current position of the given selection line.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param line given selection line
 * @param x A pointer to a variable to insert the line's x
 *          coordinate
 * @param y A pointer to a variable to insert the line's y
 *          coordinate
 * @param w A pointer to a variable to inset the line's width
 * @param h A pointer to a variable to insert the line's height
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool       edje_object_part_text_selection_line_properties_get(const Evas_Object *obj, const char *part, unsigned long line, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);


/**
 * @brief Sets the layout of the input panel.
 *
 * @since 1.1
 *
 * @remarks The layout of the input panel or virtual keyboard can make it easier or
 *          harder to enter content. This allows you to hint what kind of input you
 *          are expecting to enter and thus have the input panel automatically
 *          come up with the right mode.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] layout The layout type
 */
EAPI void             edje_object_part_text_input_panel_layout_set      (Evas_Object *obj, const char *part, Edje_Input_Panel_Layout layout);

/**
 * @brief Gets the layout of the input panel.
 *
 * @since 1.1
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @return The layout type of the input panel
 *
 * @see edje_object_part_text_input_panel_layout_set
 */
EAPI Edje_Input_Panel_Layout edje_object_part_text_input_panel_layout_get (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the layout variation of the input panel.
 *
 * @since 1.8
 *
 * @remarks The layout variation of the input panel or virtual keyboard can make it easier or
 *          harder to enter content. This allows you to hint what kind of input you
 *          are expecting to enter and thus have the input panel automatically
 *          come up with the right mode.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] variation The layout variation type
 */
EAPI void             edje_object_part_text_input_panel_layout_variation_set(Evas_Object *obj, const char *part, int variation);

/**
 * @brief Gets the layout variation of the input panel.
 *
 * @since 1.8
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @return The layout variation type of the input panel
 *
 * @see edje_object_part_text_input_panel_layout_variation_set
 */
EAPI int              edje_object_part_text_input_panel_layout_variation_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the autocapitalization type on the immodule.
 *
 * @since 1.1.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] autocapital_type The type of autocapitalization
 */
EAPI void         edje_object_part_text_autocapital_type_set            (Evas_Object *obj, const char *part, Edje_Text_Autocapital_Type autocapital_type);

/**
 * @brief Gets the autocapitalization type.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return The autocapitalization type
 */
EAPI Edje_Text_Autocapital_Type edje_object_part_text_autocapital_type_get (const Evas_Object *obj, const char *part);

/**
 * @brief Sets whether prediction is allowed.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] prediction If @c true, the prediction feature is allowed
 */
EAPI void             edje_object_part_text_prediction_allow_set        (Evas_Object *obj, const char *part, Eina_Bool prediction);

/**
 * @brief Gets whether prediction is allowed.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return #EINA_TRUE if the prediction feature is allowed
 */
EAPI Eina_Bool        edje_object_part_text_prediction_allow_get        (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the attribute to show the input panel automatically.
 *
 * @since 1.1.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] enabled If @c true, the input panel appears when an entry is clicked or has focus
 */
EAPI void             edje_object_part_text_input_panel_enabled_set     (Evas_Object *obj, const char *part, Eina_Bool enabled);

/**
 * @brief Gets the attribute to show the input panel automatically.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return #EINA_TRUE if it supports this feature, otherwise #EINA_FALSE
 *
 * @see edje_object_part_text_input_panel_enabled_set
 */
EAPI Eina_Bool        edje_object_part_text_input_panel_enabled_get (const Evas_Object *obj, const char *part);

/**
 * @brief Shows the input panel (virtual keyboard) based on the input panel property such as layout, autocapital types, and so on.
 *
 * @since 1.2.0
 *
 * @remarks Note that the input panel is shown or hidden automatically according to the focus state.
 *          This API can be used for manually controlling by using edje_object_part_text_input_panel_enabled_set.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 */
EAPI void             edje_object_part_text_input_panel_show(const Evas_Object *obj, const char *part);

/**
 * @brief Hides the input panel (virtual keyboard).
 *
 * @since 1.2.0
 *
 * @remarks Note that the input panel is shown or hidden automatically according to the focus state.
 *          This API can be used for manually controlling by using edje_object_part_text_input_panel_enabled_set.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @see edje_object_part_text_input_panel_show
 */
EAPI void             edje_object_part_text_input_panel_hide(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the language mode of the input panel.
 *
 * @since 1.2.0
 *
 * @remarks This API can be used if you want to show the Alphabet keyboard.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] lang The language to be set for the input panel
 */
EAPI void             edje_object_part_text_input_panel_language_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Lang lang);

/**
 * @brief Gets the language mode of the input panel.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return The input panel language type
 *
 * @see @ref edje_object_part_text_input_panel_language_set
 */
EAPI Edje_Input_Panel_Lang edje_object_part_text_input_panel_language_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the input panel-specific data to deliver to the input panel.
 *
 * @since 1.2.0
 *
 * @remarks This API is used by applications to deliver specific data to the input panel.
 *          The data format MUST be negotiated by both the application and the input panel.
 *          The size and format of the data are defined by the input panel.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] data The specific data to set to the input panel
 * @param[in] len The length of the data, in bytes, to send to the input panel
 */
EAPI void             edje_object_part_text_input_panel_imdata_set(Evas_Object *obj, const char *part, const void *data, int len);

/**
 * @brief Gets the specific data of the current active input panel.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] data The specific data to get from the input panel
 * @param[in] len The length of data
 */
EAPI void             edje_object_part_text_input_panel_imdata_get(const Evas_Object *obj, const char *part, void *data, int *len);

/**
 * @brief Sets the "return" key type. This type is used to set a string or icon on the "return" key of the input panel.
 *
 * @since 1.2.0
 *
 * @remarks An input panel displays the string or icon associated with this type.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] return_key_type The type of "return" key on the input panel
 */
EAPI void             edje_object_part_text_input_panel_return_key_type_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Return_Key_Type return_key_type);

/**
 * @brief Gets the "return" key type.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return The type of "return" key on the input panel
 *
 * @see edje_object_part_text_input_panel_return_key_type_set()
 */
EAPI Edje_Input_Panel_Return_Key_Type edje_object_part_text_input_panel_return_key_type_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the return key on the input panel to be disabled.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] disabled The state
 */
EAPI void             edje_object_part_text_input_panel_return_key_disabled_set(Evas_Object *obj, const char *part, Eina_Bool disabled);

/**
 * @brief Gets whether the return key on the input panel should be disabled.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return #EINA_TRUE if it should be disabled, otherwise #EINA_FALSE
 */
EAPI Eina_Bool        edje_object_part_text_input_panel_return_key_disabled_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the attribute to show the input panel only on a user's explicit Mouse Up event.
 *
 * @since 1.8.0
 *
 * @remarks It doesn't request to show the input panel even though it has focus.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] ondemand If @c true the input panel is shown on a Mouse up event(The Focus event is ignored.)
 */
EAPI void             edje_object_part_text_input_panel_show_on_demand_set(Evas_Object *obj, const char *part, Eina_Bool ondemand);

/**
 * @brief Gets the attribute to show the input panel only on a user's explicit Mouse Up event.
 *
 * @since 1.8.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return #EINA_TRUE if the input panel is shown on a Mouse up event
 */
EAPI Eina_Bool        edje_object_part_text_input_panel_show_on_demand_get(const Evas_Object *obj, const char *part);

/**
 * @brief Sets the input hint which allows input methods to fine-tune their behavior.
 *
 * @since 1.12
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param input_hints input hints
 */
EAPI void             edje_object_part_text_input_hint_set(Evas_Object *obj, const char *part, Edje_Input_Hints input_hints);

/**
 * @brief Gets the value of input hint.
 *
 * @since 1.12
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The value of input hint
 */
EAPI Edje_Input_Hints edje_object_part_text_input_hint_get(const Evas_Object *obj, const char *part);

// ******************************************************************
// ******************* TIZEN ONLY - START****************************
// ******************************************************************
typedef enum _Edje_Selection_Handler
{
   EDJE_SELECTION_HANDLER_START,
   EDJE_SELECTION_HANDLER_END,
} Edje_Selection_Handler;

/**
 * @internal
 * @brief Sets the viewport region of the text.
 *
 * @remarks Viewport region is used for showing or hiding text selection handlers.
 *
 * @param obj A valid object handle
 * @param part The part name
 * @param x A pointer to a variable to set the viewport's x coordinate
 * @param y A pointer to a variable to set the viewport's y coordinate
 * @param w A pointer to a variable to set the viewport's width
 * @param h A pointer to a variable to set the viewport's height
 */
EAPI void             edje_object_part_text_viewport_region_set     (const Evas_Object *obj, const char *part, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @internal
 * @brief Sets the layout region of the text.
 *
 * @remarks Layout region is used for showing or hiding text selection handlers.
 *
 * @param obj A valid object handle
 * @param part The part name
 * @param x A pointer to a variable to set the layout's x coordinate
 * @param y A pointer to a variable to set the layout's y coordinate
 * @param w A pointer to a variable to set the layout's width
 * @param h A pointer to a variable to set the layout's height
 */
EAPI void             edje_object_part_text_layout_region_set     (const Evas_Object *obj, const char *part, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @internal
 * @brief Gets the geometry of the selection handler.
 *
 * @param obj A valid object handle
 * @param part The edje part
 * @param type The type of handler (start, end)
 * @param x A pointer to a variable to get the handler's x coordinate
 * @param y A pointer to a variable to get the handler's y coordinate
 * @param w A pointer to a variable to get the handler's width
 * @param h A pointer to a variable to get the handler's height
 */
EAPI Eina_Bool        edje_object_part_text_selection_handler_geometry_get  (const Evas_Object *obj, const char *part, Edje_Selection_Handler type, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @internal
 * @brief Gets whether the cursor handler is disabled.
 *
 * @param obj A valid object handle
 * @param part The edje part
 * @return #EINA_TRUE if the cursor handler is disabled, otherwise #EINA_FALSE
 */
EAPI Eina_Bool        edje_object_part_text_cursor_handler_disabled_get  (const Evas_Object *obj, const char *part);

/**
 * @internal
 * @brief Sets whether the cursor handler is disabled.
 *
 * @param obj A valid object handle
 * @param part The edje part
 * @param disabled #EINA_TRUE to disable the cursor handler, otherwise #EINA_FALSE
 */
EAPI void        edje_object_part_text_cursor_handler_disabled_set  (Evas_Object *obj, const char *part, Eina_Bool disabled);

/**
 * @internal
 * @brief Gets the geometry of the cursor handler.
 *
 * @param obj A valid object handle
 * @param part The edje part
 * @param x A pointer to a variable to get the handler's x coordinate
 * @param y A pointer to a variable to get the handler's y coordinate
 * @param w A pointer to a variable to get the handler's width
 * @param h A pointer to a variable to get the handler's height
 * @return #EINA_TRUE if the cursor handler is valid (existing and visible), otherwise #EINA_FALSE
 */
EAPI Eina_Bool        edje_object_part_text_cursor_handler_geometry_get  (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
// ******************************************************************
// ******************* TIZEN ONLY - END *****************************
// ******************************************************************

/**
 * @brief Adds a filter function for a newly inserted text.
 *
 * @remarks Whenever text is inserted (not the same as set) into the given @a part,
 *          the list of filter functions are called to decide if and how the new
 *          text should be accepted.
 * @remarks There are three types of filters, EDJE_TEXT_FILTER_TEXT,
 *          EDJE_TEXT_FILTER_FORMAT, and EDJE_TEXT_FILTER_MARKUP.
 * @remarks	The text parameter in the @a func filter can be modified by the user and
 *          it's up to him to free the one passed if he has to change the pointer. On
 *          doing so, the newly set text should be malloc'ed, as once all the filters
 *          are called Edje frees it.
 *          If the text is to be rejected, freeing it and setting the pointer to @c NULL
 *          makes Edje break out of the filter cycle and reject the inserted
 *          text.
 *
 * @remarks This function is deprecated because of difficulty in use.
 *          The type(format, text, or markup) of text should always
 *          be checked in the filter function for correct filtering.
 *          Use edje_object_text_markup_filter_callback_add() instead. There
 *          is no need to check the type of text in the filter function
 *          because the text is always markup.
 * @remarks If you use this function with
 *          edje_object_text_markup_filter_callback_add(), all
 *          Edje_Text_Filter_Cb functions and Edje_Markup_Filter_Cb functions
 *          are executed, and then the filtered text is inserted.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] func The callback function that acts as a filter
 * @param[in] data The user provided data to pass to the filter function
 *
 *
 * @see edje_object_text_insert_filter_callback_del
 * @see edje_object_text_insert_filter_callback_del_full
 * @see edje_object_text_markup_filter_callback_add
 */
EAPI void             edje_object_text_insert_filter_callback_add       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * @brief Deletes a function from the filter list.
 *
 * @details This deletes the given @a func filter from the list in @a part and returns
 *          the user data pointer given when added.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] func The function callback to remove
 *
 * @return The user data pointer if successful, otherwise @c NULL
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del_full
 */
EAPI void            *edje_object_text_insert_filter_callback_del       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func);

/**
 * @brief Deletes a function and its matching user data from the filter list.
 *
 * @details This deletes the given @a func filter and @a data user data from the list
 *          in @a part.
 *          It returns the user data pointer given when added.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] func The function callback to remove
 * @param[in] data The data passed to the callback function
 *
 * @return The same data pointer if successful, otherwise @c NULL
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del
 */
EAPI void            *edje_object_text_insert_filter_callback_del_full  (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * @brief Adds a markup filter function for a newly inserted text.
 *
 * @since 1.2.0
 *
 * @remarks Whenever text is inserted (not the same as set) into the given @a part,
 *          the list of markup filter functions are called to decide if and how 
 *          the new text should be accepted.
 * @remarks The text parameter in the @a func filter is always markup. It can be 
 *          modified by the user and it's up to him to free the one passed if he has to
 *          change the pointer. On doing so, the newly set text should be malloc'ed,
 *          as once all the filters are called Edje it frees it.
 *          If the text is to be rejected, freeing it and setting the pointer to @c NULL
 *          makes Edje break out of the filter cycle and reject the inserted
 *          text.
 * @remarks This function is different from edje_object_text_insert_filter_callback_add(),
 *          in that the text parameter in the @a func filter is always markup.
 *
 * @remarks If you use this function with
 *          edje_object_text_insert_filter_callback_add(), all
 *          Edje_Text_Filter_Cb functions and Edje_Markup_Filter_Cb functions
 *          are executed, and then the filtered text is inserted.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] func The callback function that acts as a markup filter
 * @param[in] data The user provided data to pass to the filter function
 *
 * @see edje_object_text_markup_filter_callback_del
 * @see edje_object_text_markup_filter_callback_del_full
 * @see edje_object_text_insert_filter_callback_add
 */
EAPI void edje_object_text_markup_filter_callback_add(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data);

/**
 * @brief Deletes a function from the markup filter list.
 *
 * @details This deletes the given @a func filter from the list in @a part and returns
 *          the user data pointer given when added.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] func The function callback to remove
 *
 * @return The user data pointer if successful, otherwise @c NULL
 *
 * @see edje_object_text_markup_filter_callback_add
 * @see edje_object_text_markup_filter_callback_del_full
 */
EAPI void *edje_object_text_markup_filter_callback_del(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func);

/**
 * @brief Deletes a function and its matching user data from the markup filter list.
 *
 * @details This deletes the given @a func filter and @a data user data from the list
 *          in @a part and returns the user data pointer given when added.
 *
 * @since 1.2.0
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] func The function callback to remove
 * @param[in] data The data passed to the callback function
 *
 * @return The same data pointer if successful, otherwise @c NULL
 *
 * @see edje_object_text_markup_filter_callback_add
 * @see edje_object_text_markup_filter_callback_del
 */
EAPI void *edje_object_text_markup_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Swallow Edje Swallow Part
 * @ingroup Edje_Object_Part
 *
 * @brief This group discusses functions that deal with parts of type swallow and swallowed objects.
 *
 * @remarks An important feature of Edje is to be able to create Evas_Objects
 *          in the code and place them in a layout. And that is what swallowing
 *          is all about.
 *
 * @remarks Swallow parts are place holders defined in the EDC file for
 *          objects that one may want to include in the layout later, or for
 *          objects that are not natives of Edje. In this last case, Edje only
 *          only treat the Evas_Object properties of the swallowed objects.
 *
 * @{
 */

typedef enum _Edje_Aspect_Control
{
   EDJE_ASPECT_CONTROL_NONE = 0,
   EDJE_ASPECT_CONTROL_NEITHER = 1,
   EDJE_ASPECT_CONTROL_HORIZONTAL = 2,
   EDJE_ASPECT_CONTROL_VERTICAL = 3,
   EDJE_ASPECT_CONTROL_BOTH = 4
} Edje_Aspect_Control;

/**
 * @brief Sets the object's minimum size.
 *
 * @details This sets the minimum size restriction for the object.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] minw The minimum width
 * @param[in] minh The minimum height
 */
EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);

/**
 * @brief Sets the object's maximum size.
 *
 * @details This sets the maximum size restriction for the object.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] maxw The maximum width
 * @param[in] maxh The maximum height
 */
EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);

/**
 * @brief Sets the object's aspect size.
 *
 * @details This sets the desired aspect ratio to keep for an object that is
 *          swallowed by Edje. The width and height defines a preferred size
 *          ASPECT and the object may be scaled to be larger or smaller, but
 *          retaining the relative scale of both the aspect width and height.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] aspect The aspect control axes
 * @param[in] aw The aspect radio width
 * @param[in] ah The aspect ratio height
 */
EAPI void         edje_extern_object_aspect_set   (Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);

/**
 * @brief "Swallows" an object into one of the edje object @c SWALLOW
 *        parts.
 *
 * @since_tizen 2.3
 *
 * @remarks Swallowing an object into an edje object is, for a given part of
 *          type @c SWALLOW in the EDC group, which gave life to @a obj, to set
 *          an external object to be controlled by @a obj, being displayed
 *          exactly over that part's region inside the whole edje object's
 *          viewport.
 *
 *          From this point on, @a obj has total control over @a
 *          obj_swallow's geometry and visibility. For instance, if @a obj is
 *          visible, as in @c evas_object_show(), the swallowed object is
 *          visible too, if the given @c SWALLOW part it's in is also
 *          visible. Other actions on @a obj also reflect on the swallowed
 *          object (e.g. resizing, moving, raising/lowering, and so on).
 *
 *          Finally, all internal changes to @a part, specifically,
 *          reflect on the displaying of @a obj_swallow, for example state
 *          changes leading to different visibility states, geometries,
 *          positions, and so on.
 *
 * @remarks	If an object has already been swallowed into this part, then it
 *          is first unswallowed (as in edje_object_part_unswallow())
 *          before the new object is swallowed.
 *
 *          @a obj @b won't delete the swallowed object once it is
 *          deleted, @a obj_swallow gets to an unparented state again.
 *
 *          For more details on EDC @c SWALLOW parts, see @ref edcref "syntax
 *          reference".
 *
 * @param[in] obj A valid edje object handle
 * @param[in] part The swallow part's name
 * @param[in] obj_swallow The object to occupy that part
 * @return #EINA_TRUE if success, otherwise #EINA_FALSE
 */
EAPI Eina_Bool        edje_object_part_swallow        (Evas_Object *obj, const char *part, Evas_Object *obj_swallow);

/**
 * @brief Unswallows an object.
 *
 * @details It causes the Edje to regurgitate a previously swallowed object.
 *
 * @since_tizen 2.3
 *
 * @remarks @a obj_swallow is @b not deleted or hidden.
 *          @a obj_swallow may appear on the evas depending on the state in which
 *          it got unswallowed. Make sure you delete it or hide it if you do not want it.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] obj_swallow The swallowed object
 */
EAPI void             edje_object_part_unswallow      (Evas_Object *obj, Evas_Object *obj_swallow);

/**
 * @brief Gets the object currently swallowed by a part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @return The swallowed object, otherwise @c NULL if there are none
 */
EAPI Evas_Object     *edje_object_part_swallow_get    (const Evas_Object *obj, const char *part);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Drag Edje Drag
 * @ingroup Edje_Object_Part
 *
 * @brief This group discusses functions that deal with draggable parts.
 *
 * @remarks To create a movable part it must be declared as draggable
 *          in the EDC file. To do so, one must define a "draggable" block inside
 *          the "part" block.
 *
 * @remarks These functions are used to set dragging properties for a
 *          part or get dragging information about it.
 *
 * @{
 */


/**
 * @brief Edje drag direction
 */
typedef enum _Edje_Drag_Dir
{
   EDJE_DRAG_DIR_NONE = 0, /**< Direction None. */
   EDJE_DRAG_DIR_X = 1,    /**< Direction X. */
   EDJE_DRAG_DIR_Y = 2,    /**< Direction Y. */
   EDJE_DRAG_DIR_XY = 3    /**< Direction XY. */
} Edje_Drag_Dir;

/**
 * @brief Gets the draggable directions.
 *
 * @since_tizen 2.3
 *
 * @remarks The draggable directions are defined in the EDC file, inside the @c draggable
 *          section, by the attributes @c x and @c y. See the @ref edcref for more
 *          information.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 *
 * @return #EDJE_DRAG_DIR_NONE: Not draggable\n
 *         #EDJE_DRAG_DIR_X: Draggable in the X direction\n
 *         #EDJE_DRAG_DIR_Y: Draggable in the Y direction\n
 *         #EDJE_DRAG_DIR_XY: Draggable in the X & Y directions
 */
EAPI Edje_Drag_Dir    edje_object_part_drag_dir_get   (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the draggable object location.
 *
 * @details It places the draggable object at the given location.
 *
 * @since_tizen 2.3
 *
 * @remarks The values for @a dx and @a dy are real numbers that range from @c 0 to @c 1,
 *          representing its relative position on the draggable area on that axis.
 *
 *          This value means, for the vertical axis, @c 0.0 is at the top if the
 *          first parameter of @c y in the draggable part theme is @c 1, and it is at the bottom if it
 *          is @c -1.
 *
 *          For the horizontal axis, @c 0.0 means left if the first parameter of @c x in the
 *          draggable part theme is @c 1, and it means right if it is @c -1.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] dx The x value
 * @param[in] dy The y value
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_value_get()
 */
EAPI Eina_Bool        edje_object_part_drag_value_set (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the draggable object location.
 *
 * @details This gets the drag location values.
 *
 * @since_tizen 2.3
 *
 * @remarks The values for @a dx and @a dy are real numbers that range from @c 0 to @c 1,
 *          representing its relative position on the draggable area on that axis.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[out] dx The X value pointer
 * @param[out] dy The Y value pointer
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_value_set()
 */
EAPI Eina_Bool        edje_object_part_drag_value_get (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Sets the draggable object size.
 *
 * @details This sets the size of the draggable object.
 *
 * @since_tizen 2.3
 *
 * @remarks The values for @a dw and @a dh are real numbers that range from @c 0 to @c 1,
 *          representing the relative size of the draggable area on that axis.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] dw The drag width
 * @param[in] dh The drag height
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_size_get()
 */
EAPI Eina_Bool        edje_object_part_drag_size_set  (Evas_Object *obj, const char *part, double dw, double dh);

/**
 * @brief Gets the draggable object size.
 *
 * @details This gets the draggable object size
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[out] dw The drag width pointer
 * @param[out] dh The drag height pointer
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_size_set()
 */
EAPI Eina_Bool        edje_object_part_drag_size_get  (const Evas_Object *obj, const char *part, double *dw, double *dh);

/**
 * @brief Sets the drag step increment.
 *
 * @details This sets the x,y step increments for a draggable object.
 *
 * @since_tizen 2.3
 *
 * @remarks The values for @a dx and @a dy are real numbers that range from @c 0 to @c 1,
 *          representing the relative size of the draggable area on that axis by which the
 *          part is moved.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] dx The x step amount
 * @param[in] dy The y step amount
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_step_get()
 */
EAPI Eina_Bool        edje_object_part_drag_step_set  (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the drag step increment values.
 *
 * @details This gets the x and y step increments for the draggable object.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part
 * @param[out] dx The x step increment pointer
 * @param[out] dy The y step increment pointer
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_step_set()
 */
EAPI Eina_Bool        edje_object_part_drag_step_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Sets the page step increments.
 *
 * @details This sets the x,y page step increment values.
 *
 * @since_tizen 2.3
 *
 * @remarks The values for @a dx and @a dy are real numbers that range from @c 0 to @c 1,
 *          representing the relative size of the draggable area on that axis by which the
 *          part is moved.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] dx The x page step increment
 * @param[in] dy The y page step increment
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_page_get()
 */
EAPI Eina_Bool        edje_object_part_drag_page_set  (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the page step increments.
 *
 * @details This sets the x,y page step increments for the draggable object.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[out] dx The dx page increment pointer
 * @param[out] dy The dy page increment pointer
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_page_set()
 */
EAPI Eina_Bool        edje_object_part_drag_page_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Steps the draggable x,y steps.
 *
 * @details This steps x,y, where the step increment is the amount set by
 *          edje_object_part_drag_step_set.
 *
 * @since_tizen 2.3
 *
 * @remarks The values for @a dx and @a dy are real numbers that range from @c 0 to @c 1.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] dx The x step
 * @param[in] dy The y step
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_page()
 */
EAPI Eina_Bool        edje_object_part_drag_step      (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Pages the draggable x,y steps.
 *
 * @details This pages x,y, where the increment is defined by
 *          edje_object_part_drag_page_set.
 *
 * @since_tizen 2.3
 *
 * @remarks The values for @a dx and @a dy are real numbers that range from @c 0 to @c 1.
 *
 * @remarks Paging contains bugs.
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] dx The x step
 * @param[in] dy The y step
 * @return #EINA_TRUE if succeed, otherwise #EINA_FALSE
 *
 * @see edje_object_part_drag_step()
 */
EAPI Eina_Bool        edje_object_part_drag_page      (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Box Edje Box Part
 * @ingroup Edje_Object_Part
 *
 * @brief This group discusses functions that deal with parts of type box.
 *
 * @remarks A box is a container type for parts, that means it can contain
 *          other parts.
 *
 * @{
 */

/**
 * @brief Registers a custom layout to be used in edje boxes.
 *
 * @details This function registers custom layouts that can be referred from
 *          themes by the registered name. The Evas_Object_Box_Layout
 *          functions receive two pointers for internal use, one being private
 *          data, and the other being the function to free that data when it's not
 *          longer needed. From Edje, this private data is retrieved by
 *          calling layout_data_get, and layout_data_free is the free
 *          function passed to @c func. layout_data_get is called with @a data
 *          as its parameter, and this one is freed by free_data whenever
 *          the layout is unregistered from Edje.
 *
 * @since_tizen 2.3
 *
 * @param[in] name The name of the layout
 * @param[in] func The function defining the layout
 * @param[in] layout_data_get This function gets the custom data pointer
 *                        for @a func
 * @param[in] layout_data_free Passed to @a func to free its private data
 *                         when needed
 * @param[in] free_data Frees data
 * @param[in] data A private pointer passed to layout_data_get
 */
EAPI void         edje_box_layout_register        (const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data);

/**
 * @brief Appends an object to the box.
 *
 * @details This appends a child to the box indicated by the part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] child The object to append
 *
 * @return #EINA_TRUE if it is successfully added, \n
 *         otherwise #EINA_FALSE if an error occurs
 *
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_before()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_append             (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Prepends an object to the box.
 *
 * @details This prepends a child to the box indicated by the part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] child The object to prepend
 *
 * @return #EINA_TRUE if it is successfully added, \n
 *         otherwise #EINA_FALSE if an error occurs
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_insert_before()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_prepend            (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Adds an object to the box.
 *
 * @details This inserts a child in the box given by the part, in the position marked by
 *          the reference.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] child The object to insert
 * @param[in] reference The object to be used as a reference
 *
 * @return #EINA_TRUE if it is successfully added, \n
 *         otherwise #EINA_FALSE if an error occurs
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_insert_before      (Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * @brief Inserts an object to the box.
 *
 * @details This adds a child to the box indicated by the part, in the position given by
 *          @a pos.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] child The object to insert
 * @param[in] pos The position at which to insert the child
 *
 * @return #EINA_TRUE if it is successfully added, \n
 *         otherwise #EINA_FALSE if an error occurs
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_before()
 */
EAPI Eina_Bool    edje_object_part_box_insert_at          (Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);

/**
 * @brief Removes an object from the box.
 *
 * @details This removes a child from the box indicated by the part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] child The object to remove
 *
 * @return A pointer to the removed object, otherwise @c NULL
 *
 * @see edje_object_part_box_remove_at()
 * @see edje_object_part_box_remove_all()
 */
EAPI Evas_Object *edje_object_part_box_remove             (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Removes an object from the box.
 *
 * @details This removes a child from the box indicated by the part, in the position given by
 *          @a pos.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] pos The position index of the object (starts counting from 0)
 *
 * @return A pointer to the object that is removed, otherwise @c NULL
 *
 * @see edje_object_part_box_remove()
 * @see edje_object_part_box_remove_all()
 */
EAPI Evas_Object *edje_object_part_box_remove_at          (Evas_Object *obj, const char *part, unsigned int pos);

/**
 * @brief Removes all elements from the box.
 *
 * @details This removes all the external objects from the box indicated by the part.
 *          The elements created from the theme are not removed.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] clear Deletes objects on removal
 *
 * @return @c 1 if it is successfully cleared \n
 *         otherwise, @c 0 if an error occurs
 *
 * @see edje_object_part_box_remove()
 * @see edje_object_part_box_remove_at()
 */
EAPI Eina_Bool    edje_object_part_box_remove_all         (Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Table Edje Table Part
 * @ingroup Edje_Object_Part
 *
 * @brief This group discusses functions that deal with parts of type table.
 *
 * @remarks Table is a container type for parts, that means it can contain
 *          other parts.
 *
 * @{
 */

typedef enum _Edje_Object_Table_Homogeneous_Mode
{
   EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE = 0,
   EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE = 1,
   EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM = 2
} Edje_Object_Table_Homogeneous_Mode;

/**
 * @brief Gets a child from a table.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] col The column of the child to get
 * @param[in] row The row of the child to get
 * @return The child Evas_Object
 */
EAPI Evas_Object *edje_object_part_table_child_get        (const Evas_Object *obj, const char *part, unsigned int col, unsigned int row);

/**
 * @brief Packs an object into the table.
 *
 * @details This packs an object into the table indicated by the part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] child_obj The object to pack in
 * @param[in] col The column to place it in
 * @param[in] row The row to place it in
 * @param[in] colspan The columns the child takes
 * @param[in] rowspan The rows the child takes
 *
 * @return #EINA_TRUE if the object is added, otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool    edje_object_part_table_pack             (Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);

/**
 * @brief Removes an object from the table.
 *
 * @details This removes an object from the table indicated by the part.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] child_obj The object to pack in
 *
 * @return #EINA_TRUE if the object is removed, otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool    edje_object_part_table_unpack           (Evas_Object *obj, const char *part, Evas_Object *child_obj);

/**
 * @brief Gets the number of columns and rows that the table has.
 *
 * @details This gets the size of the table in the form of number of columns and rows.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[out] cols A pointer to store the number of columns (can be @c NULL)
 * @param[out] rows A pointer to store the number of rows (can be @c NULL)
 *
 * @return #EINA_TRUE to get some data, otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool    edje_object_part_table_col_row_size_get (const Evas_Object *obj, const char *part, int *cols, int *rows);

/**
 * @brief Removes all the objects from the table.
 *
 * @details This removes all the objects from the table indicated by the part, except the
 *          internal ones set from the theme.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A valid Evas_Object handle
 * @param[in] part The part name
 * @param[in] clear If set, it deletes subobjs on removal
 *
 * @return #EINA_TRUE to clear the table, otherwise #EINA_FALSE on failure
 */
EAPI Eina_Bool    edje_object_part_table_clear            (Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Communication_Interface_Message Edje Communication Interface: Message
 * @ingroup Edje_Object_Group
 *
 * @brief This group discusses functions that deal with messages.
 *
 * @remarks Edje has two communication interfaces between @b code and @b theme,
 *          signals and messages.
 *
 * @remarks Edje messages are one of the communication interfaces between
 *          @b code and a given edje object's @b theme. With messages, one can
 *          communicate values like strings, float numbers, and integer
 *          numbers. Moreover, messages can be identified by integer
 *          numbers. See #Edje_Message_Type for the full list of message types.
 *
 * @remarks Messages must be handled by scripts.
 *
 * @{
 */

/**
 * @brief Enumeration of identifiers of edje message types, which can be sent back and forth
 *        through code and a given edje object's theme file/group.
 *
 * @see edje_object_message_send()
 * @see edje_object_message_handler_set()
 */
typedef enum _Edje_Message_Type
{
   EDJE_MESSAGE_NONE = 0,

   EDJE_MESSAGE_SIGNAL = 1, /* DONT USE THIS */

   EDJE_MESSAGE_STRING = 2, /**< A message with a string as a value. Use #Edje_Message_String structs as the message body, for this type */
   EDJE_MESSAGE_INT = 3, /**< A message with an integer number as a value. Use #Edje_Message_Int structs as the message body, for this type */
   EDJE_MESSAGE_FLOAT = 4, /**< A message with a floating pointer number as a value. Use #Edje_Message_Float structs as the message body, for this type */

   EDJE_MESSAGE_STRING_SET = 5, /**< A message with a list of strings as values. Use #Edje_Message_String_Set structs as the message body, for this type */
   EDJE_MESSAGE_INT_SET = 6, /**< A message with a list of integer numbers as values. Use #Edje_Message_Int_Set structs as the message body, for this type */
   EDJE_MESSAGE_FLOAT_SET = 7, /**< A message with a list of floating point numbers as values. Use #Edje_Message_Float_Set structs as the message body, for this type */

   EDJE_MESSAGE_STRING_INT = 8, /**< A message with a struct containing a string and an integer number as values. Use #Edje_Message_String_Int structs as the message body, for this type */
   EDJE_MESSAGE_STRING_FLOAT = 9, /**< A message with a struct containing a string and a floating point number as values. Use #Edje_Message_String_Float structs as the message body, for this type */

   EDJE_MESSAGE_STRING_INT_SET = 10, /**< A message with a struct containing a string and a list of integer numbers as values. Use #Edje_Message_String_Int_Set structs as the message body, for this type */
   EDJE_MESSAGE_STRING_FLOAT_SET = 11 /**< A message with a struct containing a string and a list of floating point numbers as values. Use #Edje_Message_String_Float_Set structs as the message body, for this type */
} Edje_Message_Type;

/**
 * @brief typedef of struct _Edje_Message_String
 */
typedef struct _Edje_Message_String           Edje_Message_String;

/**
 * @brief typedef of struct _Edje_Message_Int
 */
typedef struct _Edje_Message_Int              Edje_Message_Int;

/**
 * @brief typedef of struct _Edje_Message_Float
 */
typedef struct _Edje_Message_Float            Edje_Message_Float;

/**
 * @brief typedef of struct _Edje_Message_String_Set
 */
typedef struct _Edje_Message_String_Set       Edje_Message_String_Set;

/**
 * @brief typedef of struct _Edje_Message_Int_Set
 */
typedef struct _Edje_Message_Int_Set          Edje_Message_Int_Set;

/**
 * @brief typedef of struct _Edje_Message_Float_Set
 */
typedef struct _Edje_Message_Float_Set        Edje_Message_Float_Set;

/**
 * @brief typedef of struct _Edje_Message_String_Int_Set
 */
typedef struct _Edje_Message_String_Int       Edje_Message_String_Int;

/**
 * @brief typedef of struct _Edje_Message_String_Int_Set
 */
typedef struct _Edje_Message_String_Float     Edje_Message_String_Float;

/**
 * @brief typedef of struct _Edje_Message_String_Int_Set
 */
typedef struct _Edje_Message_String_Int_Set   Edje_Message_String_Int_Set;

/**
 * @brief typedef of struct _Edje_Message_String_Float_Set
 */
typedef struct _Edje_Message_String_Float_Set Edje_Message_String_Float_Set;

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_STRING messages.
 *        The string in it is automatically freed by Edje if passed by Edje
 */
struct _Edje_Message_String
{
   char *str; /**< The message's string pointer */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_INT messages.
 */
struct _Edje_Message_Int
{
   int val; /**< The message's value */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_FLOAT messages
 */
struct _Edje_Message_Float
{
   double val; /**< The message's value */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_STRING_SET messages.
 *        The array in it is automatically freed by Edje if passed by Edje
 */
struct _Edje_Message_String_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   char *str[1]; /**< The message's @b array of string pointers */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_INT_SET messages.
 *        The array in it is automatically freed by Edje if passed by Edje
 */
struct _Edje_Message_Int_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   int val[1]; /**< The message's @b array of integers */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_FLOAT_SET messages.
 *        The array in it is automatically freed by Edje if passed by Edje
 */
struct _Edje_Message_Float_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   double val[1]; /**< The message's @b array of floats */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_STRING_INT messages.
 *        The string in it is automatically freed by Edje if passed by Edje
 */
struct _Edje_Message_String_Int
{
   char *str; /**< The message's string value */
   int val; /**< The message's integer value */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_STRING_FLOAT messages.
 *        The string in it is automatically freed by Edje if passed by Edje
 */
struct _Edje_Message_String_Float
{
   char *str; /**< The message's string value */
   double val; /**< The message's float value */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_STRING_INT_SET messages.
 *        The array and string in it are automatically freed by Edje if passed 
 *        by Edje
 */
struct _Edje_Message_String_Int_Set
{
   char *str; /**< The message's string value */
   int count; /**< The size of the message's array (may be greater than 1) */
   int val[1]; /**< The message's @b array of integers */
};

/**
 * @brief Structure passed as a value for #EDJE_MESSAGE_STRING_FLOAT_SET
 *        messages. The array and string in it are automatically freed by
 *        Edje if passed by Edje
 */
struct _Edje_Message_String_Float_Set
{
   char *str; /**< The message's string value */
   int count; /**< The size of the message's array (may be greater than 1) */
   double val[1]; /**< The message's @b array of floats */
};

typedef void         (*Edje_Message_Handler_Cb) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg); /**< Edje message handler callback functions' prototype definition. @c data has an auxiliary data pointer set at the time of the callback registration. @c obj is a pointer to the Edje object from where the message comes. @c type identifies the type of the given message and @c msg is a pointer to the message's contents, de facto, which depend on @c type */

/**
 * @brief Sends an (edje) message to a given edje object.
 *
 * @details This function sends an edje message to @a obj and to all of its
 *          child objects, if it has any (swallowed objects are a kind of
 *          child object). @a type and @a msg @b must be matched accordingly,
 *          as documented in #Edje_Message_Type.
 *
 * @since_tizen 2.3
 *
 * @remarks The @a id argument is a form of code and theme defining a common
 *          interface on message communication. One should define the same IDs
 *          on both the code and the EDC declaration (see @ref edcref "the syntax" for
 *          EDC files), to individualize messages (binding them to a given
 *          context).
 *
 * @remarks The function to handle messages arriving @b from @a obj is set with
 *          edje_object_message_handler_set().
 *
 * @param[in] obj A handle to an edje object
 * @param[in] type The type of message to send to @a obj
 * @param[in] id An identification number for the message to be sent
 * @param[in] msg The message's body, a struct depending on @a type
 *
 */
EAPI void         edje_object_message_send                (Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

/**
 * @brief Sets an edje message handler function for a given edje object.
 *
 * @since_tizen 2.3
 *
 * @remarks For scriptable programs on an edje object's defining EDC file, which
 *          send messages with the @c send_message() primitive, one can attach
 *          <b>handler functions</b>, to be called in the code which creates
 *          that object (see @ref edcref "the syntax" for EDC files).
 *
 * @remarks This function associates a message handler function and the
 *          attached data pointer to the object @a obj.
 *
 * @param[in] obj A handle to an edje object
 * @param[in] func The function to handle messages @b coming from @a obj
 * @param[in] data The auxiliary data to be passed to @a func
 *
 * @see edje_object_message_send()
 */
EAPI void         edje_object_message_handler_set         (Evas_Object *obj, Edje_Message_Handler_Cb func, void *data);

/**
 * @brief Processes an object's message queue.
 *
 * @details This function goes through the object message queue and processes the
 *          pending messages for @b this specific edje object. Normally they
 *          are processed only at idle time.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj A handle to an edje object
 *
 *
 */
EAPI void         edje_object_message_signal_process      (Evas_Object *obj);


/**
 * @brief Processes all queued up edje messages.
 *
 * @details This function triggers the processing of messages addressed to any
 *          (alive) edje objects.
 *
 * @since_tizen 2.3
 *
 */
EAPI void         edje_message_signal_process             (void);

/**
 * @}
 */

/**
 * @defgroup Edje_Perspective Edje Perspective
 * @ingroup Edje_Group
 *
 * @brief This group discusses functions that deal with 3D projection of a 2D object.
 *
 * @remarks Perspective is a graphical tool that makes objects represented in the 2D format
 *          look like they have a 3D appearance.
 *
 *          Edje allows us to use perspective on any edje object. This group of
 *          functions deals with the use of perspective, by creating and configuring
 *          a perspective object that must be set to an edje object or a canvas,
 *          affecting all the objects inside that has no particular perspective
 *          set already.
 *
 * @{
 */

/**
 * @brief perspective info for maps inside edje objects
 */
typedef struct _Edje_Perspective Edje_Perspective;

/**
 * @brief Creates a new perspective in the given canvas.
 *
 * @details This function creates a perspective object that can be set on an edje
 *          object, or can be set globally to all edje objects on this canvas.
 *
 * @param[in] e The given canvas (Evas)
 * @return An @ref Edje_Perspective object for this canvas, otherwise @c NULL on errors
 *
 *
 * @see edje_perspective_set()
 * @see edje_perspective_free()
 */
EAPI Edje_Perspective       *edje_perspective_new            (Evas *e);

/**
 * @brief Deletes the given perspective object.
 *
 * @details This function deletes the perspective object. If the perspective
 *          effect is being applied to any edje object or part, this effect won't be
 *          applied anymore.
 *
 * @since_tizen 2.3
 *
 * @param[in] ps A valid perspective object, otherwise @c NULL
 *
 *
 * @see edje_perspective_new()
 */
EAPI void                    edje_perspective_free           (Edje_Perspective *ps);

/**
 * @brief Setsup the transform for this perspective object.
 *
 * @details This sets the parameters of the perspective transformation. X, Y, and Z
 *          values are used. The px and py points specify the "infinite distance" point
 *          in the 3D conversion (where all lines converge like when artists draw
 *          3D by hand). The @a z0 value specifies the z value at which there is a 1:1
 *          mapping between spatial coordinates and screen coordinates. Any points
 *          on this z value do not have their X and Y values modified in the transform.
 *          Those further away (Z value higher) shrink into the distance, and
 *          those less than this value expand and become bigger. The @a foc value
 *          determines the "focal length" of the camera. This is in reality the distance
 *          between the camera lens plane itself (at or closer than this, rendering
 *          results are undefined) and the "z0" z value. This allows for some "depth"
 *          control and @a foc must be greater than 0.
 *
 * @since_tizen 2.3
 *
 * @param[in] ps The perspective object
 * @param[in] px The X coordinate of the perspective distance
 * @param[in] py The Y coordinate of the perspective distance
 * @param[in] z0 The "0" z plane value
 * @param[in] foc The focal distance
 */

EAPI void                    edje_perspective_set            (Edje_Perspective *ps, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc);

/**
 * @brief Sets this perspective object to be global for its canvas.
 *
 * @since_tizen 2.3
 *
 * @remarks The canvas for which this perspective object is being set as global is the one
 *          given as an argument upon object creation (the @a evas parameter of the
 *          function @c edje_perspective_new(evas) ).
 *
 *          There can only be one global perspective object set per canvas, and if
 *          a perspective object is set to global when there is already another
 *          global perspective set, the old one is set as non-global.
 *
 *          A global perspective just affects a part, if its edje object doesn't have a
 *          perspective object set to it and if the part doesn't point to another
 *          part to be used as a perspective.
 *
 * @param[in] ps The given perspective object
 * @param[in] global If #EINA_TRUE the perspective should be global,
 *               otherwise #EINA_FALSE
 *
 * @see edje_object_perspective_set()
 * @see edje_perspective_global_get()
 * @see edje_perspective_new()
 */
EAPI void                    edje_perspective_global_set     (Edje_Perspective *ps, Eina_Bool global);

/**
 * @brief Gets whether the given perspective object is global.
 *
 * @since_tizen 2.3
 *
 * @param[in] ps The given perspective object
 * @return #EINA_TRUE if this perspective object is global,
 *         otherwise #EINA_FALSE
 *
 * @see edje_perspective_global_set()
 */
EAPI Eina_Bool               edje_perspective_global_get     (const Edje_Perspective *ps);

/**
 * @brief Gets the global perspective object set for this canvas.
 *
 * @details This function returns the perspective object that is set as global
 *          with edje_perspective_global_set().
 *
 * @since_tizen 2.3
 *
 * @param[in] e The given canvas (Evas)
 * @return The perspective object set as global for this canvas, otherwise @c NULL
 *         if there is no global perspective set and on errors
 *
 * @see edje_perspective_global_set()
 * @see edje_perspective_global_get()
 */
EAPI const Edje_Perspective *edje_evas_global_perspective_get(const Evas *e);

/**
 * @brief Sets the given perspective object on this edje object.
 *
 * @details This makes the given perspective object the default perspective for this edje
 *          object.
 *
 *          There can only be one perspective object per edje object, and if a
 *          previous one is set, it is removed and the new perspective object
 *          is used.
 *
 *          An Edje perspective only affects a part if it doesn't point to another
 *          part to be used as a perspective.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj The edje object on the perspective that is set
 * @param[in] ps The perspective object that is used
 *
 * @see edje_object_perspective_new()
 * @see edje_object_perspective_get()
 * @see edje_perspective_set()
 */
EAPI void                    edje_object_perspective_set     (Evas_Object *obj, Edje_Perspective *ps);

/**
 * @brief Gets the current perspective used on this edje object.
 *
 * @since_tizen 2.3
 *
 * @param[in] obj The given edje object
 * @return The perspective object being used on this edje object, otherwise @c NULL
 *         if there is no object set and on errors
 *
 * @see edje_object_perspective_set()
 */
EAPI const Edje_Perspective *edje_object_perspective_get     (const Evas_Object *obj);

/**
 * @internal
 * @brief These APIs are for communicating with edje_entry for not moving
 *        cursor position meanwhile.
 *
 * @remarks Tizen only feature (2013.01.29)
 *
 * @remarks Applications NEVER USE THESE APIs for private purporse.
 *
 * @see edje_object_part_text_thaw()
 */
EAPI void                    edje_object_part_text_freeze    (Evas_Object *obj, const char *part);

/**
 * @internal
 * @brief These APIs are for communicating with edje_entry for not moving
 *        cursor position meanwhile.
 *
 * @remarks Tizen only feature (2013.01.29)
 *
 * @remarks Applications NEVER USE THESE APIs for private purporse.
 *
 * @see edje_object_part_text_freeze()
 */
EAPI void                    edje_object_part_text_thaw      (Evas_Object *obj, const char *part);

/**
 * @internal
 * @brief For cursor movement when mouse up.
 *
 * @remarks Tizen only feature (2013.08.30)
 *
 * @remarks Applications NEVER USE THESE APIs for private purporse.
 *
 * @see edje_object_part_text_freeze()
 */
EAPI void                    edje_object_part_text_select_disabled_set(const Evas_Object *obj, const char *part, Eina_Bool disabled);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
