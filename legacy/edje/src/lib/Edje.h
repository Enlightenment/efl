/**
@brief Edje Graphical Design Library

These routines are used for Edje.

@mainpage Edje Library Documentation
@version 1.7
@date 2003-2012

Please see the @ref authors page for contact details.

@section intro What is Edje?

Edje is a complex graphical design & layout library.

It doesn't intend to do containing and regular layout like a widget
set, but it is the base for such components. Based on the requirements
of Enlightenment 0.17, Edje should serve all the purposes of creating
visual elements (borders of windows, buttons, scrollbars, etc.) and
allow the designer the ability to animate, layout and control the look
and feel of any program using Edje as its basic GUI constructor. This
library allows for multiple collections of Layouts in one file,
sharing the same image and font database and thus allowing a whole
theme to be conveniently packaged into 1 file and shipped around.

Edje separates the layout and behavior logic. Edje files ship with an
image and font database, used by all the parts in all the collections
to source graphical data. It has a directory of logical part names
pointing to the part collection entry ID in the file (thus allowing
for multiple logical names to point to the same part collection,
allowing for the sharing of data between display elements). Each part
collection consists of a list of visual parts, as well as a list of
programs. A program is a conditionally run program that if a
particular event occurs (a button is pressed, a mouse enters or leaves
a part) will trigger an action that may affect other parts. In this
way a part collection can be "programmed" via its file as to hilight
buttons when the mouse passes over them or show hidden parts when a
button is clicked somewhere etc. The actions performed in changing
from one state to another are also allowed to transition over a period
of time, allowing animation. Programs and animations can be run in
"parallel".

This separation and simplistic event driven style of programming can produce
almost any look and feel one could want for basic visual elements. Anything
more complex is likely the domain of an application or widget set that may
use Edje as a convenient way of being able to configure parts of the display.

For details of Edje's history, see the \ref history section.


@section requirements What does Edje require?

Edje requires fairly little on your system. to use the Edje runtime library
you need:

  - Evas (library)
  - Ecore (library)
  - Eet (library)
  - Embryo (library)
  - Eina (library)
  - Lua 5.1 (library)

Evas needs to be build with the JPEG, PNG and EET image loaders enabled at a
minimum. You will also need the buffer engine (which requires the
software_generic engine) as well.

Ecore (library) needs the ECORE and ECORE_EVAS modules built at a minimum.
It's suggested to build all the Ecore modules. You will beed the Buffer
engine support built into Ecore_Evas for edje_cc to function.


@section compiling How to compile and test Edje

Now you need to compile and install Edje.

@verbatim
  ./configure
  make
  sudo make install
@endverbatim

You now have it installed and ready to go, but you need input
data. There are lots of examples in SVN, the best one is
Enlightenment's own theme file.

You may use different tools to edit and view the generated ".edj"
files, for instance:

  - edje_player (provided by Edje)
  - edje_codegen (provided by Edje) @since 1.8.0
  - editje (http://trac.enlightenment.org/e/wiki/Editje)
  - edje_viewer (http://trac.enlightenment.org/e/wiki/Edje_Viewer)


@section details So how does this all work?

Edje internally holds a geometry state machine and state graph of what is
visible, not, where, at what size, with what colors etc. This is described
to Edje from an Edje .edj file containing this information. These files can
be produced by using edje_cc to take a text file (a .edc file) and "compile"
an output .edj file that contains this information, images and any other
data needed.

The application using Edje will then create an object in its Evas
canvas and set the bundle file to use, specifying the @b group name to
use. Edje will load such information and create all the required
children objects with the specified properties as defined in each @b
part of the given group. See the following example:
@include edje_example.c

The above example requires the following annotated source Edje file:
@include edje_example.edc


One should save these files as edje_example.c and edje_example.edc then:
@verbatim
gcc -o edje_example edje_example.c `pkg-config --cflags --libs eina evas ecore ecore-evas edje`
edje_cc edje_example.edc

./edje_example "some text"
@endverbatim

Although simple, this example illustrates that animations and state
changes can be done from the Edje file itself without any requirement
in the C application.

Before digging into changing or creating your own Edje source (edc)
files, read the @ref edcref.


@section history Edje History

It's a sequel to "Ebits" which has serviced the needs of Enlightenment
development for early version 0.17. The original design parameters under
which Ebits came about were a lot more restricted than the resulting
use of them, thus Edje was born.

Edje is a more complex layout engine compared to Ebits. It doesn't
pretend to do containing and regular layout like a widget set. It
still inherits the more simplistic layout ideas behind Ebits, but it
now does them a lot more cleanly, allowing for easy expansion, and the
ability to cover much more ground than Ebits ever could. For the
purposes of Enlightenment 0.17, Edje was conceived to serve all the
purposes of creating visual elements (borders of windows, buttons,
scrollbars, etc.) and allow the designer the ability to animate,
layout and control the look and feel of any program using Edje as its
basic GUI constructor.

Unlike Ebits, Edje separates the layout and behavior logic.


@section Edje_Examples Examples on Edje's usage

What follows is a list with various commented examples, covering a great
part of Edje's API:

@note The example files are located at /Where/Enlightenment/is/installed/share/edje/examples

- @ref Example_Edje_Basics
- @ref tutorial_edje_basic2
- @ref tutorial_edje_swallow
- @ref tutorial_edje_swallow2
- @ref tutorial_edje_table
- @ref tutorial_edje_box
- @ref tutorial_edje_box2
- @ref tutorial_edje_color_class
- @ref tutorial_edje_animations
- @ref tutorial_edje_animations_2
- @ref Example_Edje_Signals_Messages
- @ref tutorial_edje_signals_2
- @ref tutorial_edje_text
- @ref tutorial_edje_drag
- @ref tutorial_edje_perspective
*/

/**
@page authors Authors
@author Carsten Haitzler <raster@@rasterman.com>
@author Tilman Sauerbeck (tilman at code-monkey de)
@author ZigsMcKenzie <zigsmckenzie@@gmail.com>
@author Cedric BAIL <cedric.bail@@free.fr>
@author Brian Mattern <rephorm@@rephorm.com>
@author Mathieu Taillefumier <mathieu.taillefumier@@free.fr>
@author Tristan <blunderer@@gmail.com>
@author Gustavo Lima Chaves <glima@@profusion.mobi>
@author Bruno Dilly <bdilly@@profusion.mobi>
@author Fabiano Fidêncio <fidencio@@profusion.mobi>
@author Jihoon Kim <jihoon48.kim@@samsung.com>
@author Tiago Falcão <tiago@@profusion.mobi>
@author Davide Andreoli <dave@@gurumeditation.it>
@author Sebastian Dransfeld <sd@@tango.flipp.net>
@author Tom Hacohen <tom@@stosb.com>
@author Aharon Hillel <a.hillel@@samsung.com>
@author Shilpa Singh <shilpa.singh@samsung.com> <shilpasingh.o@gmail.com>
@author Mike Blumenkrantz <michael.blumenkrantz@gmail.com
@author Jaehwan Kim <jae.hwan.kim@samsung.com>
@author billiob (Boris Faure) <billiob@gmail.com>
@author Govindaraju SM <govi.sm@samsung.com> <govism@gmail.com>
@author Prince Kumar Dubey <prince.dubey@samsung.com> <prince.dubey@gmail.com>
@author David Seikel <onefang at gmail.com>
@author Guilherme Íscaro <iscaro@profusion.mobi>

Please contact <enlightenment-devel@lists.sourceforge.net> to get in
contact with the developers and maintainers.
*/


/**

@example embryo_custom_state.edc
This example show how to create a custom state from embryo. Clicking on the
3 labels will rotate the object in the given direction.

@example embryo_pong.edc
Super-simple Pong implementation in pure embryo.

@example embryo_run_program.edc
This example show how to run an edje program from embryo code.

@example embryo_set_state.edc
This example show how to change the state of a part from embryo code.

@example embryo_set_text.edc
This example show how to set the text in TEXT part from embryo code.

@example embryo_timer.edc
This example show the usage of timers in embryo.

@example external_elm_anchorblock.edc
This example use an elementary anchorblock and a button to animate the text.

@example external_elm_button.edc
This example create some elementary buttons and do some actions on user click.

@example external_elm_check.edc
This example show EXTERNAL checkbox in action.

@example external_elm_panes.edc
This example show EXTERNAL elementary panes in action.

@example external_emotion_elm.edc
Super-concise video player example using Edje/Emotion/Elementary.

@example lua_script.edc
This example show the usage of lua scripting to create and animate some
objects in the canvas.

@example toggle_using_filter.edc
This example show how to toggle the state of a part using the 'filter'
param in edje programs

*/

/**
 * @file Edje.h
 * @brief Edje Graphical Design Library
 *
 * These routines are used for Edje.
 */

/**
 * @defgroup Edje_General_Group Edje General
 *
 * @brief This group discusses functions that have general purposes or affect Edje as a whole.
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
 * @brief Initialize the Edje library.
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
 * @brief Shutdown the Edje library.
 *
 * @return The number of times the library has been initialised
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
 * @brief Set the edje append fontset.
 *
 * @param fonts The fontset to append.
 *
 * This function sets the edje append fontset.
 *
 */
EAPI void		edje_fontset_append_set		(const char *fonts);

/**
 * Get data from the file level data block of an edje file
 * @param file The path to the .edj file
 * @param key The data key
 * @return The string value of the data. Must be freed by the user when no
 * longer needed.
 *
 * If an edje file is built from the following edc:
 *
 * data {
 *   item: "key1" "value1";
 *   item: "key2" "value2";
 * }
 * collections { ... }
 *
 * Then, edje_file_data_get("key1") will return "value1"
 */
EAPI char        *edje_file_data_get              (const char *file, const char *key);

/**
 * @brief Load a new module in Edje.
 * @param module The name of the module that will be added to Edje.
 * @return EINA_TRUE if if the module was successfully loaded. Otherwise, EINA_FALSE.
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
 * @brief Get the edje append fontset.
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
 * @brief Set the file cache size.
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
 * @brief Return the file cache size.
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
 * @brief Clean the file cache.
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
 * @brief Set the collection cache size.
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
 * @brief Return the collection cache size.
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
 * @brief Clean the collection cache.
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

/**
 * The possible types the parameters of an EXTERNAL part can be.
 */
typedef enum _Edje_External_Param_Type
{
   EDJE_EXTERNAL_PARAM_TYPE_INT, /**< Parameter value is an integer. */
   EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, /**< Parameter value is a double. */
   EDJE_EXTERNAL_PARAM_TYPE_STRING, /**< Parameter value is a string. */
   EDJE_EXTERNAL_PARAM_TYPE_BOOL, /**< Parameter value is boolean. */
   EDJE_EXTERNAL_PARAM_TYPE_CHOICE, /**< Parameter value is one of a set of
                                      predefined string choices. */
   EDJE_EXTERNAL_PARAM_TYPE_MAX /**< Sentinel. Don't use. */
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
 * Struct that holds parameters for parts of type EXTERNAL.
 */
struct _Edje_External_Param
{
   const char               *name; /**< The name of the parameter. */
   Edje_External_Param_Type  type; /**< The type of the parameter. This defines
                                     which of the next three variables holds
                                     the value for it. */
   // XXX these could be in a union, but eet doesn't support them (or does it?)
   int                       i; /**< Used by both integer and boolean */
   double                    d; /**< Used by double */
   const char               *s; /**< Used by both string and choice */
};
/**
 * Struct that holds parameters for parts of type EXTERNAL.
 */
typedef struct _Edje_External_Param Edje_External_Param;

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

#define EDJE_OBJ_CLASS edje_object_class_get()

const Eo_Class *edje_object_class_get(void) EINA_CONST;

extern EAPI Eo_Op EDJE_OBJ_BASE_ID;

enum
{
   EDJE_OBJ_SUB_ID_SCALE_SET,
   EDJE_OBJ_SUB_ID_SCALE_GET,
   EDJE_OBJ_SUB_ID_MIRRORED_GET,
   EDJE_OBJ_SUB_ID_MIRRORED_SET,
   EDJE_OBJ_SUB_ID_DATA_GET,
   EDJE_OBJ_SUB_ID_FREEZE,
   EDJE_OBJ_SUB_ID_THAW,
   EDJE_OBJ_SUB_ID_COLOR_CLASS_SET,
   EDJE_OBJ_SUB_ID_COLOR_CLASS_GET,
   EDJE_OBJ_SUB_ID_TEXT_CLASS_SET,
   EDJE_OBJ_SUB_ID_PART_EXISTS,
   EDJE_OBJ_SUB_ID_PART_OBJECT_GET,
   EDJE_OBJ_SUB_ID_PART_GEOMETRY_GET,
   EDJE_OBJ_SUB_ID_ITEM_PROVIDER_SET,
   EDJE_OBJ_SUB_ID_TEXT_CHANGE_CB_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PUSH,
   EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_POP,
   EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PEEK,
   EDJE_OBJ_SUB_ID_PART_TEXT_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_ESCAPED_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECTION_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_NONE,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALL,
   EDJE_OBJ_SUB_ID_PART_TEXT_INSERT,
   EDJE_OBJ_SUB_ID_PART_TEXT_APPEND,
   EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_LIST_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_GEOMETRY_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_LIST_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_GEOMETRY_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_GEOMETRY_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_USER_INSERT,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALLOW_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ABORT,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_BEGIN,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_EXTEND,
   EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_NEXT,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_PREV,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_UP,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_DOWN,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_BEGIN_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_END_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COPY,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_BEGIN_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_END_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COORD_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_FORMAT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_VISIBLE_FORMAT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_CONTENT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_RESET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_HIDE,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_GET,
   EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_ADD,
   EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL,
   EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL_FULL,
   EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_ADD,
   EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL,
   EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL_FULL,
   EDJE_OBJ_SUB_ID_PART_SWALLOW,
   EDJE_OBJ_SUB_ID_PART_UNSWALLOW,
   EDJE_OBJ_SUB_ID_PART_SWALLOW_GET,
   EDJE_OBJ_SUB_ID_SIZE_MIN_GET,
   EDJE_OBJ_SUB_ID_SIZE_MAX_GET,
   EDJE_OBJ_SUB_ID_CALC_FORCE,
   EDJE_OBJ_SUB_ID_SIZE_MIN_CALC,
   EDJE_OBJ_SUB_ID_PARTS_EXTENDS_CALC,
   EDJE_OBJ_SUB_ID_SIZE_MIN_RESTRICTED_CALC,
   EDJE_OBJ_SUB_ID_PART_STATE_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_DIR_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_SET,
   EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_SET,
   EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_STEP_SET,
   EDJE_OBJ_SUB_ID_PART_DRAG_STEP_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_SET,
   EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_STEP,
   EDJE_OBJ_SUB_ID_PART_DRAG_PAGE,
   EDJE_OBJ_SUB_ID_PART_BOX_APPEND,
   EDJE_OBJ_SUB_ID_PART_BOX_PREPEND,
   EDJE_OBJ_SUB_ID_PART_BOX_INSERT_BEFORE,
   EDJE_OBJ_SUB_ID_PART_BOX_INSERT_AT,
   EDJE_OBJ_SUB_ID_PART_BOX_REMOVE,
   EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_AT,
   EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_ALL,
   EDJE_OBJ_SUB_ID_ACCESS_PART_LIST_GET,
   EDJE_OBJ_SUB_ID_PART_TABLE_CHILD_GET,
   EDJE_OBJ_SUB_ID_PART_TABLE_PACK,
   EDJE_OBJ_SUB_ID_PART_TABLE_UNPACK,
   EDJE_OBJ_SUB_ID_PART_TABLE_COL_ROW_SIZE_GET,
   EDJE_OBJ_SUB_ID_PART_TABLE_CLEAR,
   EDJE_OBJ_SUB_ID_PERSPECTIVE_SET,
   EDJE_OBJ_SUB_ID_PERSPECTIVE_GET,
   EDJE_OBJ_SUB_ID_PRELOAD,
   EDJE_OBJ_SUB_ID_UPDATE_HINTS_SET,
   EDJE_OBJ_SUB_ID_UPDATE_HINTS_GET,

   EDJE_OBJ_SUB_ID_PART_EXTERNAL_OBJECT_GET,
   EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_SET,
   EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_GET,
   EDJE_OBJ_SUB_ID_PART_EXTERNAL_CONTENT_GET,
   EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_TYPE_GET,

   EDJE_OBJ_SUB_ID_FILE_SET,
   EDJE_OBJ_SUB_ID_FILE_GET,
   EDJE_OBJ_SUB_ID_LOAD_ERROR_GET,

   EDJE_OBJ_SUB_ID_MESSAGE_SEND,
   EDJE_OBJ_SUB_ID_MESSAGE_HANDLER_SET,
   EDJE_OBJ_SUB_ID_MESSAGE_SIGNAL_PROCESS,

   EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_ADD,
   EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_DEL,
   EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_DEL_FULL,
   EDJE_OBJ_SUB_ID_SIGNAL_EMIT,
   EDJE_OBJ_SUB_ID_PLAY_SET,
   EDJE_OBJ_SUB_ID_PLAY_GET,
   EDJE_OBJ_SUB_ID_ANIMATION_SET,
   EDJE_OBJ_SUB_ID_ANIMATION_GET,

   EDJE_OBJ_SUB_ID_LAST
};

#define EDJE_OBJ_ID(sub_id) (EDJE_OBJ_BASE_ID + sub_id)


/**
 * @def edje_obj_scale_set
 * @since 1.8
 *
 * @brief Set the scaling factor for a given Edje object.
 *
 * @param[in] scale
 * @param[out] ret
 *
 * @see edje_object_scale_set
 */
#define edje_obj_scale_set(scale, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SCALE_SET), EO_TYPECHECK(double, scale), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_scale_get
 * @since 1.8
 *
 * @brief Get a given Edje object's scaling factor.
 *
 * @param[out] ret
 *
 * @see edje_object_scale_get
 */
#define edje_obj_scale_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SCALE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def edje_obj_mirrored_get
 * @since 1.8
 *
 * @brief Get the RTL orientation for this object.
 *
 * @param[out] ret
 *
 * @see edje_object_mirrored_get
 */
#define edje_obj_mirrored_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MIRRORED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_mirrored_set
 * @since 1.8
 *
 * @brief Set the RTL orientation for this object.
 *
 * @param[in] rtl
 *
 * @see edje_object_mirrored_set
 */
#define edje_obj_mirrored_set(rtl) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MIRRORED_SET), EO_TYPECHECK(Eina_Bool, rtl)

/**
 * @def edje_obj_data_get
 * @since 1.8
 *
 * @brief Retrieve an <b>EDC data field's value</b> from a given Edje.
 *
 * @param[in] key
 * @param[out] ret
 *
 * @see edje_object_data_get
 */
#define edje_obj_data_get(key, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_DATA_GET), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_freeze
 * @since 1.8
 *
 * @brief Freezes the Edje object.
 *
 * @param[out] ret
 *
 * @see edje_object_freeze
 */
#define edje_obj_freeze(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FREEZE), EO_TYPECHECK(int *, ret)

/**
 * @def edje_obj_thaw
 * @since 1.8
 *
 * @brief Thaws the Edje object.
 *
 * @param[out] ret
 *
 * @see edje_object_thaw
 */
#define edje_obj_thaw(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_THAW), EO_TYPECHECK(int *, ret)

/**
 * @def edje_obj_color_class_set
 * @since 1.8
 *
 * @brief Sets the object color class.
 *
 * @param[in] color_class
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 * @param[in] r2
 * @param[in] g2
 * @param[in] b2
 * @param[in] a2
 * @param[in] r3
 * @param[in] g3
 * @param[in] b3
 * @param[in] a3
 * @param[out] ret
 *
 * @see edje_object_color_class_set
 */
#define edje_obj_color_class_set(color_class, r, g, b, a, r2, g2, b2, a2, r3, g3, b3, a3, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_COLOR_CLASS_SET), EO_TYPECHECK(const char *, color_class), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a), EO_TYPECHECK(int, r2), EO_TYPECHECK(int, g2), EO_TYPECHECK(int, b2), EO_TYPECHECK(int, a2), EO_TYPECHECK(int, r3), EO_TYPECHECK(int, g3), EO_TYPECHECK(int, b3), EO_TYPECHECK(int, a3), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_color_class_get
 * @since 1.8
 *
 * @brief Gets the object color class.
 *
 * @param[in] color_class
 * @param[out] r
 * @param[out] g
 * @param[out] b
 * @param[out] a
 * @param[out] r2
 * @param[out] g2
 * @param[out] b2
 * @param[out] a2
 * @param[out] r3
 * @param[out] g3
 * @param[out] b3
 * @param[out] a3
 * @param[out] ret
 *
 * @see edje_object_color_class_get
 */
#define edje_obj_color_class_get(color_class, r, g, b, a, r2, g2, b2, a2, r3, g3, b3, a3, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_COLOR_CLASS_GET), EO_TYPECHECK(const char *, color_class), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a), EO_TYPECHECK(int *, r2), EO_TYPECHECK(int *, g2), EO_TYPECHECK(int *, b2), EO_TYPECHECK(int *, a2), EO_TYPECHECK(int *, r3), EO_TYPECHECK(int *, g3), EO_TYPECHECK(int *, b3), EO_TYPECHECK(int *, a3), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_text_class_set
 * @since 1.8
 *
 * @brief Sets Edje text class.
 *
 * @param[in] text_class
 * @param[in] font
 * @param[in] size
 * @param[out] ret
 *
 * @see edje_object_text_class_set
 */
#define edje_obj_text_class_set(text_class, font, size, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_CLASS_SET), EO_TYPECHECK(const char *, text_class), EO_TYPECHECK(const char *, font), EO_TYPECHECK(Evas_Font_Size, size), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_exists
 * @since 1.8
 *
 * @brief Check if an Edje part exists in a given Edje object's group
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_exists
 */
#define edje_obj_part_exists(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXISTS), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_object_get
 * @since 1.8
 *
 * @brief Get a handle to the Evas object implementing a given Edje
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_object_get
 */
#define edje_obj_part_object_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_OBJECT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const Evas_Object **, ret)

/**
 * @def edje_obj_part_geometry_get
 * @since 1.8
 *
 * @brief Retrieve the geometry of a given Edje part, in a given Edje
 *
 * @param[in] part
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @param[out] ret
 *
 * @see edje_object_part_geometry_get
 */
#define edje_obj_part_geometry_get(part, x, y, w, h, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_GEOMETRY_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_item_provider_set
 * @since 1.8
 *
 * @brief Set the function that provides item objects for named items in an edje entry text
 *
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_item_provider_set
 */
#define edje_obj_item_provider_set(func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ITEM_PROVIDER_SET), EO_TYPECHECK(Edje_Item_Provider_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_text_change_cb_set
 * @since 1.8
 *
 * @brief Set the object text callback.
 *
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_text_change_cb_set
 */
#define edje_obj_text_change_cb_set(func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_CHANGE_CB_SET), EO_TYPECHECK(Edje_Text_Change_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_part_text_style_user_push
 * @since 1.8
 *
 * @brief Set the style of the part.
 *
 * @param[in] part
 * @param[in] style
 *
 * @see edje_object_part_text_style_user_push
 */
#define edje_obj_part_text_style_user_push(part, style) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PUSH), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, style)

/**
 * @def edje_obj_part_text_style_user_pop
 * @since 1.8
 *
 * @brief Delete the top style form the user style stack.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_style_user_pop
 */
#define edje_obj_part_text_style_user_pop(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_POP), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_style_user_peek
 * @since 1.8
 *
 * @brief Return the text of the object part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_style_user_peek
 */
#define edje_obj_part_text_style_user_peek(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PEEK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_part_text_set
 * @since 1.8
 *
 * @brief Sets the text for an object part
 *
 * @param[in] part
 * @param[in] text
 * @param[out] ret
 *
 * @see edje_object_part_text_set
 */
#define edje_obj_part_text_set(part, text, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_get
 * @since 1.8
 *
 * @brief Return the text of the object part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_get
 */
#define edje_obj_part_text_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_part_text_escaped_set
 * @since 1.8
 *
 * @brief Sets the text for an object part, but converts HTML escapes to UTF8
 *
 * @param[in] part
 * @param[in] text
 * @param[out] ret
 *
 * @see edje_object_part_text_escaped_set
 */
#define edje_obj_part_text_escaped_set(part, text, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ESCAPED_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_unescaped_set
 * @since 1.8
 *
 * @brief Sets the raw (non escaped) text for an object part.
 *
 * @param[in] part
 * @param[in] text_to_escape
 * @param[out] ret
 *
 * @see edje_object_part_text_unescaped_set
 */
#define edje_obj_part_text_unescaped_set(part, text_to_escape, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text_to_escape), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_unescaped_get
 * @since 1.8
 *
 * @brief Returns the text of the object part, without escaping.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_unescaped_get
 */
#define edje_obj_part_text_unescaped_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(char **, ret)

/**
 * @def edje_obj_part_text_selection_get
 * @since 1.8
 *
 * @brief Return the selection text of the object part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_selection_get
 */
#define edje_obj_part_text_selection_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECTION_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_part_text_select_none
 * @since 1.8
 *
 * @brief Set the selection to be none.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_none
 */
#define edje_obj_part_text_select_none(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_NONE), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_select_all
 * @since 1.8
 *
 * @brief Set the selection to be everything.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_all
 */
#define edje_obj_part_text_select_all(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALL), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_insert
 * @since 1.8
 *
 * @brief Insert text for an object part.
 *
 * @param[in] part
 * @param[in] text
 *
 * @see edje_object_part_text_insert
 */
#define edje_obj_part_text_insert(part, text) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INSERT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text)

/**
 * @def edje_obj_part_text_append
 * @since 1.8
 *
 * @brief Insert text for an object part.
 *
 * @param[in] part
 * @param[in] text
 *
 * @see edje_object_part_text_append
 */
#define edje_obj_part_text_append(part, text) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_APPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text)

/**
 * @def edje_obj_part_text_anchor_list_get
 * @since 1.8
 *
 * @brief Return a list of char anchor names.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_anchor_list_get
 */
#define edje_obj_part_text_anchor_list_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_LIST_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def edje_obj_part_text_anchor_geometry_get
 * @since 1.8
 *
 * @brief Return a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * @param[in] part
 * @param[in] anchor
 * @param[out] ret
 *
 * @see edje_object_part_text_anchor_geometry_get
 */
#define edje_obj_part_text_anchor_geometry_get(part, anchor, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_GEOMETRY_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, anchor), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def edje_obj_part_text_item_list_get
 * @since 1.8
 *
 * @brief Return a list of char item names.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_item_list_get
 */
#define edje_obj_part_text_item_list_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_LIST_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def edje_obj_part_text_item_geometry_get
 * @since 1.8
 *
 * @brief Return item geometry.
 *
 * @param[in] part
 * @param[in] item
 * @param[out] cx
 * @param[out] cy
 * @param[out] cw
 * @param[out] ch
 * @param[out] ret
 *
 * @see edje_object_part_text_item_geometry_get
 */
#define edje_obj_part_text_item_geometry_get(part, item, cx, cy, cw, ch, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_GEOMETRY_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, item), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy), EO_TYPECHECK(Evas_Coord *, cw), EO_TYPECHECK(Evas_Coord *, ch), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_geometry_get
 * @since 1.8
 *
 * @brief Returns the cursor geometry of the part relative to the edje
 *
 * @param[in] part
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see edje_object_part_text_cursor_geometry_get
 */
#define edje_obj_part_text_cursor_geometry_get(part, x, y, w, h) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_GEOMETRY_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def edje_obj_part_text_user_insert
 * @since 1.8
 *
 * @brief This function inserts text as if the user has inserted it.
 *
 * @param[in] part
 * @param[in] text
 *
 * @see edje_object_part_text_user_insert
 */
#define edje_obj_part_text_user_insert(part, text) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_USER_INSERT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text)

/**
 * @def edje_obj_part_text_select_allow_set
 * @since 1.8
 *
 * @brief Enables selection if the entry is an EXPLICIT selection mode
 *
 * @param[in] part
 * @param[in] allow
 *
 * @see edje_object_part_text_select_allow_set
 */
#define edje_obj_part_text_select_allow_set(part, allow) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALLOW_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, allow)

/**
 * @def edje_obj_part_text_select_abort
 * @since 1.8
 *
 * @brief Aborts any selection action on a part.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_abort
 */
#define edje_obj_part_text_select_abort(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ABORT), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_select_begin
 * @since 1.8
 *
 * @brief Starts selecting at current cursor position
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_begin
 */
#define edje_obj_part_text_select_begin(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_BEGIN), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_select_extend
 * @since 1.8
 *
 * @brief Extends the current selection to the current cursor position
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_extend
 */
#define edje_obj_part_text_select_extend(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_EXTEND), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_imf_context_get
 * @since 1.8
 *
 * @brief Get the input method context in entry.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_imf_context_get
 */
#define edje_obj_part_text_imf_context_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_part_text_cursor_next
 * @since 1.8
 *
 * @brief Advances the cursor to the next cursor position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_next
 */
#define edje_obj_part_text_cursor_next(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_NEXT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_prev
 * @since 1.8
 *
 * @brief Moves the cursor to the previous char
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_prev
 */
#define edje_obj_part_text_cursor_prev(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_PREV), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_up
 * @since 1.8
 *
 * @brief Move the cursor to the char above the current cursor position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_up
 */
#define edje_obj_part_text_cursor_up(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_UP), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_down
 * @since 1.8
 *
 * @brief Moves the cursor to the char below the current cursor position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_down
 */
#define edje_obj_part_text_cursor_down(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_DOWN), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_begin_set
 * @since 1.8
 *
 * @brief Moves the cursor to the beginning of the text part
 *
 * @param[in] part
 * @param[in] cur
 *
 * @see edje_object_part_text_cursor_begin_set
 */
#define edje_obj_part_text_cursor_begin_set(part, cur) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_BEGIN_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur)

/**
 * @def edje_obj_part_text_cursor_end_set
 * @since 1.8
 *
 * @brief Moves the cursor to the end of the text part.
 *
 * @param[in] part
 * @param[in] cur
 *
 * @see edje_object_part_text_cursor_end_set
 */
#define edje_obj_part_text_cursor_end_set(part, cur) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_END_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur)

/**
 * @def edje_obj_part_text_cursor_copy
 * @since 1.8
 *
 * @brief Copy the cursor to another cursor.
 *
 * @param[in] part
 * @param[in] src
 * @param[in] dst
 *
 * @see edje_object_part_text_cursor_copy
 */
#define edje_obj_part_text_cursor_copy(part, src, dst) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COPY), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, src), EO_TYPECHECK(Edje_Cursor, dst)

/**
 * @def edje_obj_part_text_cursor_line_begin_set
 * @since 1.8
 *
 * @brief Move the cursor to the beginning of the line.
 *
 * @param[in] part
 * @param[in] cur
 *
 * @see edje_object_part_text_cursor_line_begin_set
 */
#define edje_obj_part_text_cursor_line_begin_set(part, cur) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_BEGIN_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur)

/**
 * @def edje_obj_part_text_cursor_line_end_set
 * @since 1.8
 *
 * @brief Move the cursor to the end of the line.
 *
 * @param[in] part
 * @param[in] cur
 *
 * @see edje_object_part_text_cursor_line_end_set
 */
#define edje_obj_part_text_cursor_line_end_set(part, cur) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_END_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur)

/**
 * @def edje_obj_part_text_cursor_coord_set
 * @since 1.8
 *
 * Position the given cursor to a X,Y position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[in] x
 * @param[in] y
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_coord_set
 */
#define edje_obj_part_text_cursor_coord_set(part, cur, x, y, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COORD_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_is_format_get
 * @since 1.8
 *
 * @brief Returns whether the cursor points to a format.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_is_format_get
 */
#define edje_obj_part_text_cursor_is_format_get(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_FORMAT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_is_visible_format_get
 * @since 1.8
 *
 * @brief Return true if the cursor points to a visible format
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_is_visible_format_get
 */
#define edje_obj_part_text_cursor_is_visible_format_get(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_VISIBLE_FORMAT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_content_get
 * @since 1.8
 *
 * @brief Returns the content (char) at the cursor position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_content_get
 */
#define edje_obj_part_text_cursor_content_get(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_CONTENT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(char **, ret)

/**
 * @def edje_obj_part_text_cursor_pos_set
 * @since 1.8
 *
 * @brief Sets the cursor position to the given value
 *
 * @param[in] part
 * @param[in] cur
 * @param[in] pos
 *
 * @see edje_object_part_text_cursor_pos_set
 */
#define edje_obj_part_text_cursor_pos_set(part, cur, pos) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(int, pos)

/**
 * @def edje_obj_part_text_cursor_pos_get
 * @since 1.8
 *
 * @brief Retrieves the current position of the cursor
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_pos_get
 */
#define edje_obj_part_text_cursor_pos_get(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(int *, ret)

/**
 * @def edje_obj_part_text_imf_context_reset
 * @since 1.8
 *
 * @brief Reset the input method context if needed.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_imf_context_reset
 */
#define edje_obj_part_text_imf_context_reset(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_RESET), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_input_panel_layout_set
 * @since 1.8
 *
 * @brief Set the layout of the input panel.
 *
 * @param[in] part
 * @param[in] layout
 *
 * @see edje_object_part_text_input_panel_layout_set
 */
#define edje_obj_part_text_input_panel_layout_set(part, layout) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Layout, layout)

/**
 * @def edje_obj_part_text_input_panel_layout_get
 * @since 1.8
 *
 * @brief Get the layout of the input panel.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_layout_get
 */
#define edje_obj_part_text_input_panel_layout_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Layout *, ret)

/**
 * @def edje_obj_part_text_autocapital_type_set
 * @since 1.8
 *
 * @brief Set the autocapitalization type on the immodule.
 *
 * @param[in] part
 * @param[in] autocapital_type
 *
 * @see edje_object_part_text_autocapital_type_set
 */
#define edje_obj_part_text_autocapital_type_set(part, autocapital_type) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Autocapital_Type, autocapital_type)

/**
 * @def edje_obj_part_text_autocapital_type_get
 * @since 1.8
 *
 * @brief Retrieves the autocapitalization type
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_autocapital_type_get
 */
#define edje_obj_part_text_autocapital_type_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Autocapital_Type *, ret)

/**
 * @def edje_obj_part_text_prediction_allow_set
 * @since 1.8
 *
 * @brief Set whether the prediction is allowed or not.
 *
 * @param[in] part
 * @param[in] prediction
 *
 * @see edje_object_part_text_prediction_allow_set
 */
#define edje_obj_part_text_prediction_allow_set(part, prediction) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, prediction)

/**
 * @def edje_obj_part_text_prediction_allow_get
 * @since 1.8
 *
 * @brief Get whether the prediction is allowed or not.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_prediction_allow_get
 */
#define edje_obj_part_text_prediction_allow_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_input_panel_enabled_set
 * @since 1.8
 *
 * @brief Sets the attribute to show the input panel automatically.
 *
 * @param[in] part
 * @param[in] enabled
 *
 * @see edje_object_part_text_input_panel_enabled_set
 */
#define edje_obj_part_text_input_panel_enabled_set(part, enabled) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def edje_obj_part_text_input_panel_enabled_get
 * @since 1.8
 *
 * @brief Retrieve the attribute to show the input panel automatically.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_enabled_get
 */
#define edje_obj_part_text_input_panel_enabled_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_input_panel_show
 * @since 1.8
 *
 * @brief Show the input panel (virtual keyboard) based on the input panel property such as layout, autocapital types, and so on.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_input_panel_show
 */
#define edje_obj_part_text_input_panel_show(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_input_panel_hide
 * @since 1.8
 *
 * @brief Hide the input panel (virtual keyboard).
 *
 * @param[in] part
 *
 * @see edje_object_part_text_input_panel_hide
 */
#define edje_obj_part_text_input_panel_hide(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_HIDE), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_input_panel_language_set
 * @since 1.8
 *
 * Set the language mode of the input panel.
 *
 * @param[in] part
 * @param[in] lang
 *
 * @see edje_object_part_text_input_panel_language_set
 */
#define edje_obj_part_text_input_panel_language_set(part, lang) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Lang, lang)

/**
 * @def edje_obj_part_text_input_panel_language_get
 * @since 1.8
 *
 * Get the language mode of the input panel.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_language_get
 */
#define edje_obj_part_text_input_panel_language_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Lang *, ret)

/**
 * @def edje_obj_part_text_input_panel_imdata_set
 * @since 1.8
 *
 * Set the input panel-specific data to deliver to the input panel.
 *
 * @param[in] part
 * @param[in] data
 * @param[in] len
 *
 * @see edje_object_part_text_input_panel_imdata_set
 */
#define edje_obj_part_text_input_panel_imdata_set(part, data, len) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const void *, data), EO_TYPECHECK(int, len)

/**
 * @def edje_obj_part_text_input_panel_imdata_get
 * @since 1.8
 *
 * Get the specific data of the current active input panel.
 *
 * @param[in] part
 * @param[in] data
 * @param[in] len
 *
 * @see edje_object_part_text_input_panel_imdata_get
 */
#define edje_obj_part_text_input_panel_imdata_get(part, data, len) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(void *, data), EO_TYPECHECK(int *, len)

/**
 * @def edje_obj_part_text_input_panel_return_key_type_set
 * @since 1.8
 *
 * Set the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * @param[in] part
 * @param[out] return_key_type
 *
 * @see edje_object_part_text_input_panel_return_key_type_set
 */
#define edje_obj_part_text_input_panel_return_key_type_set(part, return_key_type) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Return_Key_Type, return_key_type)

/**
 * @def edje_obj_part_text_input_panel_return_key_type_get
 * @since 1.8
 *
 * Get the "return" key type.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_return_key_type_get
 */
#define edje_obj_part_text_input_panel_return_key_type_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Return_Key_Type *, ret)

/**
 * @def edje_obj_part_text_input_panel_return_key_disabled_set
 * @since 1.8
 *
 * Set the return key on the input panel to be disabled.
 *
 * @param[in] part
 * @param[in] disabled
 *
 * @see edje_object_part_text_input_panel_return_key_disabled_set
 */
#define edje_obj_part_text_input_panel_return_key_disabled_set(part, disabled) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def edje_obj_part_text_input_panel_return_key_disabled_get
 * @since 1.8
 *
 * Get whether the return key on the input panel should be disabled or not.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_return_key_disabled_get
 */
#define edje_obj_part_text_input_panel_return_key_disabled_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_text_insert_filter_callback_add
 * @since 1.8
 *
 * Add a filter function for newly inserted text.
 *
 * @param[in] part
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_text_insert_filter_callback_add
 */
#define edje_obj_text_insert_filter_callback_add(part, func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_ADD), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_text_insert_filter_callback_del
 * @since 1.8
 *
 * Delete a function from the filter list.
 *
 * @param[in] part
 * @param[in] func
 * @param[out] ret
 *
 * @see edje_object_text_insert_filter_callback_del
 */
#define edje_obj_text_insert_filter_callback_del(part, func, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Filter_Cb, func), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_text_insert_filter_callback_del_full
 * @since 1.8
 *
 * Delete a function and matching user data from the filter list.
 *
 * @param[in] part
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see edje_object_text_insert_filter_callback_del_full
 */
#define edje_obj_text_insert_filter_callback_del_full(part, func, data, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL_FULL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Filter_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_text_markup_filter_callback_add
 * @since 1.8
 *
 * Add a markup filter function for newly inserted text.
 *
 * @param[in] part
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_text_markup_filter_callback_add
 */
#define edje_obj_text_markup_filter_callback_add(part, func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_ADD), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Markup_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_text_markup_filter_callback_del
 * @since 1.8
 *
 * Delete a function from the markup filter list.
 *
 * @param[in] part
 * @param[in] func
 * @param[out] ret
 *
 * @see edje_object_text_markup_filter_callback_del
 */
#define edje_obj_text_markup_filter_callback_del(part, func, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Markup_Filter_Cb, func), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_text_markup_filter_callback_del_full
 * @since 1.8
 *
 * Delete a function and matching user data from the markup filter list.
 *
 * @param[in] part
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see edje_object_text_markup_filter_callback_del_full
 */
#define edje_obj_text_markup_filter_callback_del_full(part, func, data, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL_FULL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Markup_Filter_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_part_swallow
 * @since 1.8
 *
 * @brief "Swallows" an object into one of the Edje object SWALLOW
 *
 * @param[in] part
 * @param[in] obj_swallow
 * @param[out] ret
 *
 * @see edje_object_part_swallow
 */
#define edje_obj_part_swallow(part, obj_swallow, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_SWALLOW), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, obj_swallow), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_unswallow
 * @since 1.8
 *
 * @brief Unswallow an object.
 *
 * @param[in] obj_swallow
 *
 * @see edje_object_part_unswallow
 */
#define edje_obj_part_unswallow(obj_swallow) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_UNSWALLOW), EO_TYPECHECK(Evas_Object *, obj_swallow)

/**
 * @def edje_obj_part_swallow_get
 * @since 1.8
 *
 * @brief Get the object currently swallowed by a part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_swallow_get
 */
#define edje_obj_part_swallow_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_SWALLOW_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_size_min_get
 * @since 1.8
 *
 * @brief Get the minimum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param[out] minw
 * @param[out] minh
 *
 * @see edje_object_size_min_get
 */
#define edje_obj_size_min_get(minw, minh) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_GET), EO_TYPECHECK(Evas_Coord *, minw), EO_TYPECHECK(Evas_Coord *, minh)

/**
 * @def edje_obj_size_max_get
 * @since 1.8
 *
 * @brief Get the maximum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param[out] maxw
 * @param[out] maxh
 *
 * @see edje_object_size_max_get
 */
#define edje_obj_size_max_get(maxw, maxh) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MAX_GET), EO_TYPECHECK(Evas_Coord *, maxw), EO_TYPECHECK(Evas_Coord *, maxh)

/**
 * @def edje_obj_calc_force
 * @since 1.8
 *
 * @brief Force a Size/Geometry calculation.
 *
 *
 * @see edje_object_calc_force
 */
#define edje_obj_calc_force() EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_CALC_FORCE)

/**
 * @def edje_obj_size_min_calc
 * @since 1.8
 *
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param[out] minw
 * @param[out] minh
 *
 * @see edje_object_size_min_calc
 */
#define edje_obj_size_min_calc(minw, minh) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_CALC), EO_TYPECHECK(Evas_Coord *, minw), EO_TYPECHECK(Evas_Coord *, minh)

/**
 * @def edje_obj_parts_extends_calc
 * @since 1.8
 *
 * Calculate the geometry of the region, relative to a given Edje
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @param[out] ret
 *
 * @see edje_object_parts_extends_calc
 */
#define edje_obj_parts_extends_calc(x, y, w, h, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PARTS_EXTENDS_CALC), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_size_min_restricted_calc
 * @since 1.8
 *
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param[out] minw
 * @param[out] minh
 * @param[in] restrictedw
 * @param[in] restrictedh
 *
 * @see edje_object_size_min_restricted_calc
 */
#define edje_obj_size_min_restricted_calc(minw, minh, restrictedw, restrictedh) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_RESTRICTED_CALC), EO_TYPECHECK(Evas_Coord *, minw), EO_TYPECHECK(Evas_Coord *, minh), EO_TYPECHECK(Evas_Coord, restrictedw), EO_TYPECHECK(Evas_Coord, restrictedh)

/**
 * @def edje_obj_part_state_get
 * @since 1.8
 *
 * @brief Returns the state of the Edje part.
 *
 * @param[in] part
 * @param[out] val_ret
 * @param[out] ret
 *
 * @see edje_object_part_state_get
 */
#define edje_obj_part_state_get(part, val_ret, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_STATE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, val_ret), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_part_drag_dir_get
 * @since 1.8
 *
 * @brief Determine dragable directions.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_drag_dir_get
 */
#define edje_obj_part_drag_dir_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_DIR_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Drag_Dir *, ret)

/**
 * @def edje_obj_part_drag_value_set
 * @since 1.8
 *
 * @brief Set the dragable object location.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_value_set
 */
#define edje_obj_part_drag_value_set(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_value_get
 * @since 1.8
 *
 * @brief Get the dragable object location.
 *
 * @param[in] part
 * @param[out] dx
 * @param[out] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_value_get
 */
#define edje_obj_part_drag_value_get(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, dx), EO_TYPECHECK(double *, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_size_set
 * @since 1.8
 *
 * @brief Set the dragable object size.
 *
 * @param[in] part
 * @param[in] dw
 * @param[in] dh
 * @param[out] ret
 *
 * @see edje_object_part_drag_size_set
 */
#define edje_obj_part_drag_size_set(part, dw, dh, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dw), EO_TYPECHECK(double, dh), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_size_get
 * @since 1.8
 *
 * @brief Get the dragable object size.
 *
 * @param[in] part
 * @param[out] dw
 * @param[out] dh
 * @param[out] ret
 *
 * @see edje_object_part_drag_size_get
 */
#define edje_obj_part_drag_size_get(part, dw, dh, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, dw), EO_TYPECHECK(double *, dh), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_step_set
 * @since 1.8
 *
 * @brief Sets the drag step increment.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_step_set
 */
#define edje_obj_part_drag_step_set(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_step_get
 * @since 1.8
 *
 * @brief Gets the drag step increment values.
 *
 * @param[in] part
 * @param[out] dx
 * @param[out] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_step_get
 */
#define edje_obj_part_drag_step_get(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, dx), EO_TYPECHECK(double *, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_page_set
 * @since 1.8
 *
 * @brief Sets the page step increments.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_page_set
 */
#define edje_obj_part_drag_page_set(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_page_get
 * @since 1.8
 *
 * @brief Gets the page step increments.
 *
 * @param[in] part
 * @param[out] dx
 * @param[out] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_page_get
 */
#define edje_obj_part_drag_page_get(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, dx), EO_TYPECHECK(double *, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_step
 * @since 1.8
 *
 * @brief Steps the dragable x,y steps.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_step
 */
#define edje_obj_part_drag_step(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_page
 * @since 1.8
 *
 * @brief Pages x,y steps.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_page
 */
#define edje_obj_part_drag_page(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_append
 * @since 1.8
 *
 * @brief Appends an object to the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see edje_object_part_box_append
 */
#define edje_obj_part_box_append(part, child, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_APPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_prepend
 * @since 1.8
 *
 * @brief Prepends an object to the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see edje_object_part_box_prepend
 */
#define edje_obj_part_box_prepend(part, child, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_PREPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_insert_before
 * @since 1.8
 *
 * @brief Adds an object to the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] reference
 * @param[out] ret
 *
 * @see edje_object_part_box_insert_before
 */
#define edje_obj_part_box_insert_before(part, child, reference, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_INSERT_BEFORE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_insert_at
 * @since 1.8
 *
 * @brief Inserts an object to the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] pos
 * @param[out] ret
 *
 * @see edje_object_part_box_insert_at
 */
#define edje_obj_part_box_insert_at(part, child, pos, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_INSERT_AT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_remove
 * @since 1.8
 *
 * @brief Removes an object from the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see edje_object_part_box_remove
 */
#define edje_obj_part_box_remove(part, child, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_box_remove_at
 * @since 1.8
 *
 * @brief Removes an object from the box.
 *
 * @param[in] part
 * @param[in] pos
 * @param[out] ret
 *
 * @see edje_object_part_box_remove_at
 */
#define edje_obj_part_box_remove_at(part, pos, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_AT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_box_remove_all
 * @since 1.8
 *
 * @brief Removes all elements from the box.
 *
 * @param[in] part
 * @param[in] clear
 * @param[out] ret
 *
 * @see edje_object_part_box_remove_all
 */
#define edje_obj_part_box_remove_all(part, clear, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_ALL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_access_part_list_get
 * @since 1.8
 *
 * @brief Retrieve a list all accessibility part names
 *
 * @param[out] ret
 *
 * @see edje_object_access_part_list_get
 */
#define edje_obj_access_part_list_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ACCESS_PART_LIST_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def edje_obj_part_table_child_get
 * @since 1.8
 *
 * @brief Retrieve a child from a table
 *
 * @param[in] part
 * @param[in] col
 * @param[in] row
 * @param[out] ret
 *
 * @see edje_object_part_table_child_get
 */
#define edje_obj_part_table_child_get(part, col, row, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_CHILD_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(unsigned int, col), EO_TYPECHECK(unsigned int, row), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_table_pack
 * @since 1.8
 *
 * @brief Packs an object into the table.
 *
 * @param[in] part
 * @param[in] child_obj
 * @param[in] col
 * @param[in] row
 * @param[in] colspan
 * @param[in] rowspan
 * @param[out] ret
 *
 * @see edje_object_part_table_pack
 */
#define edje_obj_part_table_pack(part, child_obj, col, row, colspan, rowspan, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_PACK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child_obj), EO_TYPECHECK(unsigned short, col), EO_TYPECHECK(unsigned short, row), EO_TYPECHECK(unsigned short, colspan), EO_TYPECHECK(unsigned short, rowspan), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_table_unpack
 * @since 1.8
 *
 * @brief Removes an object from the table.
 *
 * @param[in] part
 * @param[in] child_obj
 * @param[out] ret
 *
 * @see edje_object_part_table_unpack
 */
#define edje_obj_part_table_unpack(part, child_obj, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_UNPACK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child_obj), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_table_col_row_size_get
 * @since 1.8
 *
 * @brief Gets the number of columns and rows the table has.
 *
 * @param[in] part
 * @param[out] cols
 * @param[out] rows
 * @param[out] ret
 *
 * @see edje_object_part_table_col_row_size_get
 */
#define edje_obj_part_table_col_row_size_get(part, cols, rows, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_COL_ROW_SIZE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(int *, cols), EO_TYPECHECK(int *, rows), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_table_clear
 * @since 1.8
 *
 * @brief Removes all object from the table.
 *
 * @param[in] part
 * @param[in] clear
 * @param[out] ret
 *
 * @see edje_object_part_table_clear
 */
#define edje_obj_part_table_clear(part, clear, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_CLEAR), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_perspective_set
 * @since 1.8
 *
 * Set the given perspective object on this Edje object.
 *
 * @param[in] ps
 *
 * @see edje_object_perspective_set
 */
#define edje_obj_perspective_set(ps) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PERSPECTIVE_SET), EO_TYPECHECK(Edje_Perspective *, ps)

/**
 * @def edje_obj_perspective_get
 * @since 1.8
 *
 * Get the current perspective used on this Edje object.
 *
 * @param[out] ret
 *
 * @see edje_object_perspective_get
 */
#define edje_obj_perspective_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PERSPECTIVE_GET), EO_TYPECHECK(const Edje_Perspective **, ret)

/**
 * @def edje_obj_preload
 * @since 1.8
 *
 * @brief Preload the images on the Edje Object in the background.
 *
 * @param[in] cancel
 * @param[out] ret
 *
 * @see edje_object_preload
 */
#define edje_obj_preload(cancel, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PRELOAD), EO_TYPECHECK(Eina_Bool, cancel), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_update_hints_set
 * @since 1.8
 *
 * @brief Edje will automatically update the size hints on itself.
 *
 * @param[in] update
 *
 * @see edje_object_update_hints_set
 */
#define edje_obj_update_hints_set(update) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_UPDATE_HINTS_SET), EO_TYPECHECK(Eina_Bool, update)

/**
 * @def edje_obj_update_hints_get
 * @since 1.8
 *
 * @brief Wether or not Edje will update size hints on itself.
 *
 * @param[out] ret
 *
 * @see edje_object_update_hints_get
 */
#define edje_obj_update_hints_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_UPDATE_HINTS_GET), EO_TYPECHECK(Eina_Bool *, ret)


/**
 * @def edje_obj_part_external_object_get
 * @since 1.8
 *
 * @brief Get the object created by this external part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_external_object_get
 */
#define edje_obj_part_external_object_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_OBJECT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_external_param_set
 * @since 1.8
 *
 * @brief Set the parameter for the external part.
 *
 * @param[in] part
 * @param[in] param
 * @param[out] ret
 *
 * @see edje_object_part_external_param_set
 */
#define edje_obj_part_external_param_set(part, param, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const Edje_External_Param *, param), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_external_param_get
 * @since 1.8
 *
 * @brief Get the parameter for the external part.
 *
 * @param[in] part
 * @param[out] param
 * @param[out] ret
 *
 * @see edje_object_part_external_param_get
 */
#define edje_obj_part_external_param_get(part, param, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_External_Param *, param), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_external_content_get
 * @since 1.8
 *
 * @brief Get an object contained in an part of type EXTERNAL
 *
 * @param[in] part
 * @param[out] content
 * @param[out] ret
 *
 * @see edje_object_part_external_content_get
 */
#define edje_obj_part_external_content_get(part, content, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_CONTENT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, content), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_external_param_type_get
 * @since 1.8
 *
 * Facility to query the type of the given parameter of the given part.
 *
 * @param[in] part
 * @param[out] param
 * @param[out] ret
 *
 * @see edje_object_part_external_param_type_get
 */
#define edje_obj_part_external_param_type_get(part, param, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_TYPE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, param), EO_TYPECHECK(Edje_External_Param_Type *, ret)

/**
 * @def edje_obj_file_set
 * @since 1.8
 *
 * @param[in] file in
 * @param[in] group in
 * @param[in] nested in
 * @param[out] ret out
 *
 * @see edje_object_file_set
 */
#define edje_obj_file_set(file, group, nested, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FILE_SET), EO_TYPECHECK(const char*, file), EO_TYPECHECK(const char *, group), EO_TYPECHECK(Eina_Array *, nested), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_file_get
 * @since 1.8
 *
 * @brief Get the file and group name that a given Edje object is bound to
 *
 * @param[out] file
 * @param[out] group
 *
 * @see edje_object_file_get
 */
#define edje_obj_file_get(file, group) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FILE_GET), EO_TYPECHECK(const char **, file), EO_TYPECHECK(const char **, group)

/**
 * @def edje_obj_load_error_get
 * @since 1.8
 *
 * @brief Gets the (last) file loading error for a given Edje object
 *
 * @param[out] ret
 *
 * @see edje_object_load_error_get
 */
#define edje_obj_load_error_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_LOAD_ERROR_GET), EO_TYPECHECK(Edje_Load_Error *, ret)


/**
 * @def edje_obj_message_send
 * @since 1.8
 *
 * @brief Send an (Edje) message to a given Edje object
 *
 * @param[in] type
 * @param[in] id
 * @param[in] msg
 *
 * @see edje_object_message_send
 */
#define edje_obj_message_send(type, id, msg) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_SEND), EO_TYPECHECK(Edje_Message_Type, type), EO_TYPECHECK(int, id), EO_TYPECHECK(void *, msg)

/**
 * @def edje_obj_message_handler_set
 * @since 1.8
 *
 * @brief Set an Edje message handler function for a given Edje object.
 *
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_message_handler_set
 */
#define edje_obj_message_handler_set(func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_HANDLER_SET), EO_TYPECHECK(Edje_Message_Handler_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_message_signal_process
 * @since 1.8
 *
 * @brief Process an object's message queue.
 *
 *
 * @see edje_object_message_signal_process
 */
#define edje_obj_message_signal_process() EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_SIGNAL_PROCESS)


/**
 * @def edje_obj_signal_callback_add
 * @since 1.8
 *
 * @brief Add a callback for an arriving Edje signal, emitted by
 * a given Ejde object.
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_signal_callback_add
 */
#define edje_obj_signal_callback_add(emission, source, func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_ADD), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_signal_callback_del
 * @since 1.8
 *
 * @brief Remove a signal-triggered callback from an object.
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[out] ret
 *
 * @see edje_object_signal_callback_del
 */
#define edje_obj_signal_callback_del(emission, source, func, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_DEL), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_signal_callback_del_full
 * @since 1.8
 *
 * @brief Unregister/delete a callback set for an arriving Edje
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see edje_object_signal_callback_del_full
 */
#define edje_obj_signal_callback_del_full(emission, source, func, data, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_DEL_FULL), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_signal_emit
 * @since 1.8
 *
 * @brief Send/emit an Edje signal to a given Edje object
 *
 * @param[in] emission
 * @param[in] source
 *
 * @see edje_object_signal_emit
 */
#define edje_obj_signal_emit(emission, source) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_EMIT), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source)

/**
 * @def edje_obj_play_set
 * @since 1.8
 *
 * @brief Set the Edje object to playing or paused states.
 *
 * @param[in] play
 *
 * @see edje_object_play_set
 */
#define edje_obj_play_set(play) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PLAY_SET), EO_TYPECHECK(Eina_Bool, play)

/**
 * @def edje_obj_play_get
 * @since 1.8
 *
 * @brief Get the Edje object's state.
 *
 * @param[out] ret
 *
 * @see edje_object_play_get
 */
#define edje_obj_play_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PLAY_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_animation_set
 * @since 1.8
 *
 * @brief Set the object's animation state.
 *
 * @param[in] on
 *
 * @see edje_object_animation_set
 */
#define edje_obj_animation_set(on) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ANIMATION_SET), EO_TYPECHECK(Eina_Bool, on)

/**
 * @def edje_obj_animation_get
 * @since 1.8
 *
 * @brief Get the Edje object's animation state.
 *
 * @param[out] ret
 *
 * @see edje_object_animation_get
 */
#define edje_obj_animation_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ANIMATION_GET), EO_TYPECHECK(Eina_Bool *, ret)

#define EDJE_EDIT_CLASS edje_edit_class_get()

const Eo_Class *edje_edit_class_get(void) EINA_CONST;

/**
 * @defgroup Edje_External_Part_Group Edje Use of External Parts
 *
 * @brief Functions to manipulate parts of type EXTERNAL.
 *
 * Edje supports parts of type EXTERNAL, which will call plugins defined by the user
 * to create and manipulate the object that's allocated in that part.
 *
 * Parts of type external may carry extra properties that have meanings defined
 * by the external plugin. For instance, it may be a string that defines a button
 * label and setting this property will change that label on the fly.
 *
 * @ingroup Edje_External_Group
 *
 * @{
 */

/**
 * @brief Get the object created by this external part.
 *
 * Parts of type external creates the part object using information
 * provided by external plugins. It's somehow like "swallow"
 * (edje_object_part_swallow()), but it's all set automatically.
 *
 * This function returns the part created by such external plugins and
 * being currently managed by this Edje.
 *
 * @note Almost all swallow rules apply: you should not move, resize,
 *       hide, show, set the color or clipper of such part. It's a bit
 *       more restrictive as one must @b never delete this object!
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The externally created object, or NULL if there is none or
 *         part is not an external.
 */
EAPI Evas_Object              *edje_object_part_external_object_get     (const Evas_Object *obj, const char *part);

/**
 * @brief Set the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have
 * meanings defined by the external plugin. For instance, it may be a
 * string that defines a button label and setting this property will
 * change that label on the fly.
 *
 * @note external parts have parameters set when they change
 *       states. Those parameters will never be changed by this
 *       function. The interpretation of how state_set parameters and
 *       param_set will interact is up to the external plugin.
 *
 * @note this function will not check if parameter value is valid
 *       using #Edje_External_Param_Info minimum, maximum, valid
 *       choices and others. However these should be checked by the
 *       underlying implementation provided by the external
 *       plugin. This is done for performance reasons.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param param the parameter details, including its name, type and
 *        actual value. This pointer should be valid, and the
 *        parameter must exist in
 *        #Edje_External_Type::parameters_info, with the exact type,
 *        otherwise the operation will fail and @c EINA_FALSE will be
 *        returned.
 *
 * @return @c EINA_TRUE if everything went fine, @c EINA_FALSE on errors.
 */
EAPI Eina_Bool                 edje_object_part_external_param_set      (Evas_Object *obj, const char *part, const Edje_External_Param *param);

/**
 * @brief Get the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have
 * meanings defined by the external plugin. For instance, it may be a
 * string that defines a button label. This property can be modified by
 * state parameters, by explicit calls to
 * edje_object_part_external_param_set() or getting the actual object
 * with edje_object_part_external_object_get() and calling native
 * functions.
 *
 * This function asks the external plugin what is the current value,
 * independent on how it was set.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name

 * @param param the parameter details. It is used as both input and
 *        output variable. This pointer should be valid, and the
 *        parameter must exist in
 *        #Edje_External_Type::parameters_info, with the exact type,
 *        otherwise the operation will fail and @c EINA_FALSE will be
 *        returned.
 *
 * @return @c EINA_TRUE if everything went fine and @p param members
 *         are filled with information, @c EINA_FALSE on errors and @p
 *         param member values are not set or valid.
 */
EAPI Eina_Bool                 edje_object_part_external_param_get      (const Evas_Object *obj, const char *part, Edje_External_Param *param);

/**
 * @brief Get an object contained in an part of type EXTERNAL
 *
 * The @p content string must not be NULL. Its actual value depends on the
 * code providing the EXTERNAL.
 *
 * @param obj The Edje object
 * @param part The name of the part holding the EXTERNAL
 * @param content A string identifying which content from the EXTERNAL to get
 */
EAPI Evas_Object              *edje_object_part_external_content_get    (const Evas_Object *obj, const char *part, const char *content);

/**
 * Facility to query the type of the given parameter of the given part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param param the parameter name to use.
 *
 * @return @c EDJE_EXTERNAL_PARAM_TYPE_MAX on errors, or another value
 *         from #Edje_External_Param_Type on success.
 */
EAPI Edje_External_Param_Type  edje_object_part_external_param_type_get (const Evas_Object *obj, const char *part, const char *param);

/**
 * @}
 */

/**
 * @defgroup Edje_External_Plugin_Development_Group Edje Development of External Plugins
 *
 * @brief Functions to register, unregister EXTERNAL types and develop the plugins.
 *
 * This group dicusses functions useful for the development of new plugins.
 * These functions deal with the newly EXTERNAL types by registering, unregistering and manipulating them.
 *
 * @ingroup Edje_External_Group
 *
 * @{
 */

/**
 * Register a type to be used by EXTERNAL parts.
 *
 * Parts of type EXTERNAL will call user defined functions
 * to create and manipulate the object that's allocated in that part. This is
 * done by expecifying in the @c source property of the part the name of the
 * external to use, which must be one registered with this function.
 *
 * @param type_name name to register and be known by edje's "source:"
 *        parameter of "type: EXTERNAL" parts.
 * @param type_info meta-information describing how to interact with it.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type already registered).
 *
 * @see edje_external_type_array_register()
 */
EAPI Eina_Bool    edje_external_type_register             (const char *type_name, const Edje_External_Type *type_info);

/**
 * Unregister a previously registered EXTERNAL type.
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
 * Register a batch of types and their information.
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
 * Unregister a batch of given external type previously registered.
 *
 * @param array @c NULL terminated array, should be the same as the
 *        one used to register with edje_external_type_array_register()
 *
 * @see edje_external_type_unregister()
 */
EAPI void         edje_external_type_array_unregister     (const Edje_External_Type_Info *array);

/**
 * Return the current ABI version for Edje_External_Type structure.
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
 * Returns an interator of all the registered EXTERNAL types.
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
 * Conevenience function to find a specific parameter in a list of them.
 *
 * @param params The list of parameters for the external
 * @param key The parameter to look for
 *
 * @return The matching #Edje_External_Param or NULL if it's not found.
 */
EAPI Edje_External_Param            *edje_external_param_find       (const Eina_List *params, const char *key);

/**
 * Get the value of the given parameter of integer type.
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
 * @return EINA_TRUE if the parameter was found and is of integer type,
 * EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_int_get    (const Eina_List *params, const char *key, int *ret);

/**
 * Get the value of the given parameter of double type.
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
 * @return EINA_TRUE if the parameter was found and is of double type,
 * EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_double_get (const Eina_List *params, const char *key, double *ret);

/**
 * Get the value of the given parameter of string type.
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
 * @return EINA_TRUE if the parameter was found and is of string type,
 * EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_string_get (const Eina_List *params, const char *key, const char **ret);

/**
 * Get the value of the given parameter of boolean type.
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
 * @return EINA_TRUE if the parameter was found and is of boolean type,
 * EINA_FALSE otherwise.
 */
EAPI Eina_Bool                       edje_external_param_bool_get   (const Eina_List *params, const char *key, Eina_Bool *ret);

/**
 * Get the value of the given parameter of choice type.
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
 * Get the array of parameters information about a type given its name.
 *
 * @note the type names and other strings are static, that means they are
 *       @b NOT translated. One must use
 *       Edje_External_Type::translate() to translate those.
 *
 * @return the NULL terminated array, or @c NULL if type is unknown or
 *         it does not have any parameter information.
 *
 * @see edje_external_type_get()
 */
EAPI const Edje_External_Param_Info *edje_external_param_info_get   (const char *type_name);

/**
 * Get the #Edje_External_Type that defines an EXTERNAL type registered with
 * the name @p type_name.
 */
EAPI const Edje_External_Type       *edje_external_type_get         (const char *type_name);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Group Edje Object
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
 * @brief Instantiate a new Edje object
 *
 * @param evas A valid Evas handle, the canvas to place the new object
 * in
 * @return A handle to the new object created or @c NULL, on errors.
 *
 * This function creates a new Edje smart object, returning its @c
 * Evas_Object handle. An Edje object is useless without a (source)
 * file set to it, so you'd most probably call edje_object_file_set()
 * afterwards, like in:
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
 *      	errmsg);
 *
 *      evas_object_del(edje);
 *      return NULL;
 *   }
 *
 * @endcode
 *
 * @note You can get a callback every time edje re-calculates the object
 * (either due to animation or some kind of signal or input). This is called
 * in-line just after the recalculation has occurred. It is a good idea not
 * to go and delete or alter the object inside this callbacks, simply make
 * a note that the recalculation has taken place and then do something about
 * it outside the callback. to register a callback use code like:
 *
 * @code
 *    evas_object_smart_callback_add(edje_obj, "recalc", my_cb, my_cb_data);
 * @endcode
 *
 * @see evas_object_smart_callback_add()
 *
 * @note Before creating the first Edje object in your code, remember
 * to initialize the library, with edje_init(), or unexpected behavior
 * might occur.
 */
EAPI Evas_Object *edje_object_add                 (Evas *evas);

/**
 * @brief Preload the images on the Edje Object in the background.
 *
 * @param obj A handle to an Edje object
 * @param cancel @c EINA_FALSE will add it the preloading work queue,
 *               @c EINA_TRUE will remove it (if it was issued before).
 * @return @c EINA_FASLE if obj was not a valid Edje object
 *         otherwise @c EINA_TRUE
 *
 * This function requests the preload of all data images (on the given
 * object) in the background. The work is queued before being processed
 * (because there might be other pending requests of this type).
 * It emits a signal "preload,done" when finished.
 *
 * @note Use @c EINA_TRUE on scenarios where you don't need
 *       the image data preloaded anymore.
 */
EAPI Eina_Bool        edje_object_preload         (Evas_Object *obj, Eina_Bool cancel);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Scale Edje Scale
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
 * @ingroup Edje_Object_Group
 *
 * @{
 */

/**
 * @brief Set Edje's global scaling factor.
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
 * @brief Retrieve Edje's global scaling factor.
 *
 * @return The global scaling factor
 *
 * This function returns Edje's global scaling factor.
 *
 * @see edje_scale_set() for more details
 *
 */
EAPI double       edje_scale_get                  (void);

/**
 * @brief Set the scaling factor for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param scale The scaling factor (the default value is @c 0.0,
 * meaning individual scaling @b not set)
 *
 * This function sets an @b individual scaling factor on the @a obj
 * Edje object. This property (or Edje's global scaling factor, when
 * applicable), will affect this object's part sizes. If @p scale is
 * not zero, than the individual scaling will @b override any global
 * scaling set, for the object @p obj's parts. Put it back to zero to
 * get the effects of the global scaling again.
 *
 * @warning Only parts which, at EDC level, had the @c "scale"
 * property set to @c 1, will be affected by this function. Check the
 * complete @ref edcref "syntax reference" for EDC files.
 *
 * @see edje_object_scale_get()
 * @see edje_scale_get() for more details
 */
EAPI Eina_Bool    edje_object_scale_set           (Evas_Object *obj, double scale);

/**
 * @brief Get a given Edje object's scaling factor.
 *
 * @param obj A handle to an Edje object
 *
 * This function returns the @c individual scaling factor set on the
 * @a obj Edje object.
 *
 * @see edje_object_scale_set() for more details
 *
 */
EAPI double       edje_object_scale_get           (const Evas_Object *obj);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Color_Class Edje Class: Color
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
 * @ingroup Edje_Object_Group
 *
 * @{
 */

/**
 * @brief Set Edje color class.
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
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_color_class_set            (const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Get Edje color class.
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
 * @return EINA_TRUE if found or EINA_FALSE if not found and all
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
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_color_class_get            (const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Delete edje color class.
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
 * @brief Sets the object color class.
 *
 * @param obj A valid Evas_Object handle
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
 * This function sets the color values for an object level color
 * class. This will cause all edje parts in the specified object that
 * have the specified color class to have their colors multiplied by
 * these values.
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * Setting color emits a signal "color_class,set" with source being
 * the given color.
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_object_color_class_set         (Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Gets the object color class.
 *
 * @param o A valid Evas_Object handle
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
 * @return EINA_TRUE if found or EINA_FALSE if not found and all
 *         values are zeroed.
 *
 * This function gets the color values for an object level color
 * class. If no explicit object color is set, then global values will
 * be used.
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool    edje_object_color_class_get         (const Evas_Object *o, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Delete the object color class.
 *
 * @param obj The edje object's reference.
 * @param color_class The color class to be deleted.
 *
 * This function deletes any values at the object level for the
 * specified object and color class.
 * @note Deleting the color class will revert it to the values
 *       defined by edje_color_class_set() or the color class
 *       defined in the theme file.
 *
 * Deleting the color class will emit the signal "color_class,del"
 * for the given Edje object.
 */
 EAPI void         edje_object_color_class_del         (Evas_Object *obj, const char *color_class);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Text_Class Edje Class: Text
 *
 * @brief Functions that deal with Text Classes
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
 * @ingroup Edje_Object_Group
 *
 * @{
 */

/**
 * @brief Set the Edje text class.
 *
 * @param text_class The text class name
 * @param font The font name
 * @param size The font size
 *
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on error
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
 * @brief Delete the text class.
 *
 * @param text_class The text class name string
 *
 * This function deletes any values at the process level for the
 * specified text class.
 *
 */
EAPI void         edje_text_class_del             (const char *text_class);

/**
 * @brief List text classes.
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
 * @brief Sets Edje text class.
 *
 * @param obj A valid Evas_Object handle
 * @param text_class The text class name
 * @param font Font name
 * @param size Font Size
 *
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on error
 *
 * This function sets the text class for the Edje.
 *
 */
EAPI Eina_Bool    edje_object_text_class_set          (Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_File Edje Object File
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
 * @ingroup Edje_Object_Group
 *
 * @{
 */

typedef enum _Edje_Load_Error
{
   EDJE_LOAD_ERROR_NONE = 0, /**< No error happened, the loading was successful */
   EDJE_LOAD_ERROR_GENERIC = 1, /**< A generic error happened during the loading */
   EDJE_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< The file pointed to did not exist */
   EDJE_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission to read the given file was denied */
   EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Resource allocation failed during the loading */
   EDJE_LOAD_ERROR_CORRUPT_FILE = 5, /**< The file pointed to was corrupt */
   EDJE_LOAD_ERROR_UNKNOWN_FORMAT = 6, /**< The file pointed to had an unknown format */
   EDJE_LOAD_ERROR_INCOMPATIBLE_FILE = 7, /**< The file pointed to is incompatible, i.e., it doesn't match the library's current version's format */
   EDJE_LOAD_ERROR_UNKNOWN_COLLECTION = 8, /**< The group/collection set to load from was @b not found in the file */
   EDJE_LOAD_ERROR_RECURSIVE_REFERENCE = 9 /**< The group/collection set to load from had <b>recursive references</b> on its components */
} Edje_Load_Error; /**< Edje file loading error codes one can get - see edje_load_error_str() too. */

/**
 * Get a list of groups in an edje file
 * @param file The path to the edje file
 *
 * @return The Eina_List of group names (char *)
 *
 * Note: the list must be freed using edje_file_collection_list_free()
 * when you are done with it.
 */
EAPI Eina_List        *edje_file_collection_list  (const char *file);

/**
 * Free file collection list
 * @param lst The Eina_List of groups
 *
 * Frees the list returned by edje_file_collection_list().
 */
EAPI void             edje_file_collection_list_free (Eina_List *lst);

/**
 * Determine whether a group matching glob exists in an edje file.
 * @param file The file path
 * @param glob A glob to match on
 *
 * @return 1 if a match is found, 0 otherwise
 */
EAPI Eina_Bool        edje_file_group_exists      (const char *file, const char *glob);

/**
 * @brief Retrive an <b>EDC data field's value</b> from a given Edje
 * object's group.
 *
 * @param obj A handle to an Edje object
 * @param key The data field's key string
 * @return The data's value string. Must not be freed.
 *
 * This function fetches an EDC data field's value, which is declared
 * on the objects building EDC file, <b>under its group</b>. EDC data
 * blocks are most commonly used to pass arbitrary parameters from an
 * application's theme to its code.
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
 * EDC data fields always hold @b strings as values, hence the return
 * type of this function. Check the complete @ref edcref "syntax reference"
 * for EDC files.
 *
 * @warning Do not confuse this call with edje_file_data_get(), which
 * queries for a @b global EDC data field on an EDC declaration file.
 *
 * @see edje_object_file_set()
 */
EAPI const char  *edje_object_data_get            (const Evas_Object *obj, const char *key);

/**
 * @brief Sets the @b EDJ file (and group within it) to load an Edje
 * object's contents from
 *
 * @param obj A handle to an Edje object
 * @param file The path to the EDJ file to load @p from
 * @param group The name of the group, in @p file, which implements an
 * Edje object
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on errors (check
 * edje_object_load_error_get() after this call to get errors causes)
 *
 * Edje expects EDJ files, which are theming objects' descriptions and
 * resources packed together in an EET file, to read Edje object
 * definitions from. They usually are created with the @c .edj
 * extension. EDJ files, in turn, are assembled from @b textual object
 * description files, where one describes Edje objects declaratively
 * -- the EDC files (see @ref edcref "the syntax" for those files).
 *
 * Those description files were designed so that many Edje object
 * definitions -- also called @b groups (or collections) -- could be
 * packed together <b>in the same EDJ file</b>, so that a whole
 * application's theme could be packed in one file only. This is the
 * reason for the @p group argument.
 *
 * Use this function after you instantiate a new Edje object, so that
 * you can "give him life", telling where to get its contents from.
 *
 * @see edje_object_add()
 * @see edje_object_file_get()
 */
EAPI Eina_Bool        edje_object_file_set        (Evas_Object *obj, const char *file, const char *group);

/**
 * @brief Get the file and group name that a given Edje object is bound to
 *
 * @param obj A handle to an Edje object
 * @param file A pointer to a variable whero to store the <b>file's
 * path</b>
 * @param group A pointer to a variable where to store the <b>group
 * name</b> in
 *
 * This gets the EDJ file's path, with the respective group set for
 * the given Edje object. If @a obj is either not an Edje file, or has
 * not had its file/group set previously, by edje_object_file_set(),
 * then both @p file and @p group will be set to @c NULL, indicating
 * an error.
 *
 * @see edje_object_file_set()
 *
 * @note Use @c NULL pointers on the file/group components you're not
 * interested in: they'll be ignored by the function.
 */
EAPI void             edje_object_file_get        (const Evas_Object *obj, const char **file, const char **group);

/**
 * @brief Gets the (last) file loading error for a given Edje object
 *
 * @param obj A handlet to an Edje object
 *
 * @return The Edje loading error, one of:
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
 * This function is meant to be used after an Edje EDJ <b>file
 * loading</b>, what takes place with the edje_object_file_set()
 * function. If that function does not return @c EINA_TRUE, one should
 * check for the reason of failure with this one.
 *
 * @see edje_load_error_str()
 */
EAPI Edje_Load_Error  edje_object_load_error_get  (const Evas_Object *obj);

/**
 * Converts the given Edje file load error code into a string
 * describing it in English.
 *
 * @param error the error code, a value in ::Edje_Load_Error.
 * @return Always returns a valid string. If the given @p error is not
 *         supported, <code>"Unknown error"</code> is returned.
 *
 * edje_object_file_set() is a function which sets an error value,
 * afterwards, which can be fetched with
 * edje_object_load_error_get(). The function in question is meant
 * to be used in conjunction with the latter, for pretty-printing any
 * possible error cause.
 */
EAPI const char	      *edje_load_error_str	  (Edje_Load_Error error);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Communication_Interface_Signal Edje Communication Interface: Signal
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
 * @ingroup Edje_Object_Group
 *
 * @{
 */

typedef void         (*Edje_Signal_Cb)          (void *data, Evas_Object *obj, const char *emission, const char *source); /**< Edje signal callback functions's prototype definition. @c data will have the auxiliary data pointer set at the time the callback registration. @c obj will be a pointer the Edje object where the signal comes from. @c emission will identify the exact signal's emission string and @c source the exact signal's source one. */

/**
 * @brief Add a callback for an arriving Edje signal, emitted by
 * a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @param data A pointer to data to pass in to @p func.
 *
 * Edje signals are one of the communication interfaces between
 * @b code and a given Edje object's @b theme. With signals, one can
 * communicate two string values at a time, which are:
 * - "emission" value: the name of the signal, in general
 * - "source" value: a name for the signal's context, in general
 *
 * Though there are those common uses for the two strings, one is free
 * to use them however they like.
 *
 * This function adds a callback function to a signal emitted by @a obj, to
 * be issued every time an EDC program like the following
 * @code
 * program {
 *   name: "emit_example";
 *   action: SIGNAL_EMIT "a_signal" "a_source";
 * }
 * @endcode
 * is run, if @p emission and @p source are given those same values,
 * here.
 *
 * Signal callback registration is powerful, in the way that @b blobs
 * may be used to match <b>multiple signals at once</b>. All the @c
 * "*?[\" set of @c fnmatch() operators can be used, both for @p
 * emission and @p source.
 *
 * Edje has @b internal signals it will emit, automatically, on
 * various actions taking place on group parts. For example, the mouse
 * cursor being moved, pressed, released, etc., over a given part's
 * area, all generate individual signals.
 *
 * By using something like
 * @code
 * edje_object_signal_callback_add(obj, "mouse,down,*", "button.*",
 *                                 signal_cb, NULL);
 * @endcode
 * being @c "button.*" the pattern for the names of parts implementing
 * buttons on an interface, you'd be registering for notifications on
 * events of mouse buttons being pressed down on either of those parts
 * (those events all have the @c "mouse,down," common prefix on their
 * names, with a suffix giving the button number). The actual emission
 * and source strings of an event will be passed in as the @a emission
 * and @a source parameters of the callback function (e.g. @c
 * "mouse,down,2" and @c "button.close"), for each of those events.
 *
 * @note See @ref edcref "the syntax" for EDC files
 * @see edje_object_signal_emit() on how to emits Edje signals from
 * code to a an object
 * @see edje_object_signal_callback_del_full()
 */
EAPI void         edje_object_signal_callback_add (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Remove a signal-triggered callback from an object.
 *
 * @param obj A valid Evas_Object handle.
 * @param emission The emission string.
 * @param source The source string.
 * @param func The callback function.
 * @return The data pointer
 *
 * This function removes a callback, previously attached to the
 * emittion of a signal, from the object @a obj. The parameters @a
 * emission, @a source and @a func must match exactly those passed to
 * a previous call to edje_object_signal_callback_add(). The data
 * pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del_full().
 *
 */
EAPI void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);

/**
 * @brief Unregister/delete a callback set for an arriving Edje
 * signal, emitted by a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 * @param func The callback function passed on the callback's
 * registration
 * @param data The pointer given to be passed as data to @p func
 * @return @p data, on success or @c NULL, on errors (or if @p data
 * had this value)
 *
 * This function removes a callback, previously attached to the
 * emittion of a signal, from the object @a obj. The parameters
 * @a emission, @a source, @a func and @a data must match exactly those
 * passed to a previous call to edje_object_signal_callback_add(). The
 * data pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del().
 *
 */
EAPI void        *edje_object_signal_callback_del_full(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * @brief Send/emit an Edje signal to a given Edje object
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 *
 * This function sends a signal to the object @a obj. An Edje program,
 * at @p obj's EDC specification level, can respond to a signal by
 * having declared matching @c 'signal' and @c 'source' fields on its
 * block (see @ref edcref "the syntax" for EDC files).
 *
 * As an example,
 * @code
 * edje_object_signal_emit(obj, "a_signal", "");
 * @endcode
 * would trigger a program which had an EDC declaration block like
 * @code
 * program {
 *  name: "a_program";
 *  signal: "a_signal";
 *  source: "";
 *  action: ...
 * }
 * @endcode
 *
 * @see edje_object_signal_callback_add() for more on Edje signals.
 */
EAPI void         edje_object_signal_emit         (Evas_Object *obj, const char *emission, const char *source);

/**
 * @brief Get extra data passed to callbacks.
 *
 * @return the extra data for that callback.
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
 * @defgroup Edje_Object_Animation Edje Object Animation
 *
 * @brief Functions that deal with animations.
 *
 * Edje has the ability to animate objects. One can start, stop, play,
 * pause, freeze and thaw edje animations using the functions of this section.
 *
 * @ingroup Edje_Object_Group
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

typedef enum _Edje_Action_Type
{
   EDJE_ACTION_TYPE_NONE                     = 0,
   EDJE_ACTION_TYPE_STATE_SET                = 1,
   EDJE_ACTION_TYPE_ACTION_STOP              = 2,
   EDJE_ACTION_TYPE_SIGNAL_EMIT              = 3,
   EDJE_ACTION_TYPE_DRAG_VAL_SET             = 4,
   EDJE_ACTION_TYPE_DRAG_VAL_STEP            = 5,
   EDJE_ACTION_TYPE_DRAG_VAL_PAGE            = 6,
   EDJE_ACTION_TYPE_SCRIPT                   = 7,
   EDJE_ACTION_TYPE_FOCUS_SET                = 8,
   EDJE_ACTION_TYPE_RESERVED00               = 9,
   EDJE_ACTION_TYPE_FOCUS_OBJECT             = 10,
   EDJE_ACTION_TYPE_PARAM_COPY               = 11,
   EDJE_ACTION_TYPE_PARAM_SET                = 12,
   EDJE_ACTION_TYPE_SOUND_SAMPLE             = 13, /**< @since 1.1 */
   EDJE_ACTION_TYPE_SOUND_TONE               = 14, /**< @since 1.1 */
   EDJE_ACTION_TYPE_PHYSICS_IMPULSE          = 15, /**< @since 1.8 */
   EDJE_ACTION_TYPE_PHYSICS_TORQUE_IMPULSE   = 16, /**< @since 1.8 */
   EDJE_ACTION_TYPE_PHYSICS_FORCE            = 17, /**< @since 1.8 */
   EDJE_ACTION_TYPE_PHYSICS_TORQUE           = 18, /**< @since 1.8 */
   EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR     = 19, /**< @since 1.8 */
   EDJE_ACTION_TYPE_PHYSICS_VEL_SET          = 20, /**< @since 1.8 */
   EDJE_ACTION_TYPE_PHYSICS_ANG_VEL_SET      = 21, /**< @since 1.8 */
   EDJE_ACTION_TYPE_PHYSICS_STOP             = 22, /**< @since 1.8 */
   EDJE_ACTION_TYPE_LAST                     = 23
} Edje_Action_Type;

/**
 * @brief Set edje trasitions' frame time.
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
 * @brief Get edje trasitions' frame time.
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
 * @brief Freeze Edje objects.
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
 * @brief Thaw Edje objects.
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
 * @brief Set the Edje object to playing or paused states.
 *
 * @param obj A handle to an Edje object.
 * @param play Object state (@c EINA_TRUE to playing,
 *                           @c EINA_FALSE to paused).
 *
 * This function sets the Edje object @a obj to playing or paused
 * states, depending on the parameter @a play. This has no effect if
 * the object was already at that state.
 *
 * @see edje_object_play_get().
 *
 */
EAPI void         edje_object_play_set            (Evas_Object *obj, Eina_Bool play);

/**
 * @brief Get the Edje object's state.
 *
 * @param obj A handle to an Edje object.
 * @return @c EINA_FALSE if the object is not connected, its @c delete_me flag
 * is set, or it is at paused state; @c EINA_TRUE if the object is at playing
 * state.
 *
 * This function tells if an Edje object is playing or not. This state
 * is set by edje_object_play_set().
 *
 * @see edje_object_play_set().
 *
 */
EAPI Eina_Bool    edje_object_play_get            (const Evas_Object *obj);

/**
 * @brief Set the object's animation state.
 *
 * @param obj A handle to an Edje object.
 * @param on The animation state. @c EINA_TRUE to starts or
 *           @c EINA_FALSE to stops.
 *
 * This function starts or stops an Edje object's animation. The
 * information if it's stopped can be retrieved by
 * edje_object_animation_get().
 *
 * @see edje_object_animation_get()
 *
 */
EAPI void         edje_object_animation_set       (Evas_Object *obj, Eina_Bool on);

/**
 * @brief Get the Edje object's animation state.
 *
 * @param obj A handle to an Edje object.
 * @return @c EINA_FALSE on error or if object is not animated;
 *         @c EINA_TRUE if animated.
 *
 * This function returns if the animation is stopped or not. The
 * animation state is set by edje_object_animation_set().
 *
 * @see edje_object_animation_set().
 *
 */
EAPI Eina_Bool    edje_object_animation_get       (const Evas_Object *obj);

/**
 * @brief Freezes the Edje object.
 *
 * @param obj A handle to an Edje object.
 * @return The frozen state or 0 on Error
 *
 * This function puts all changes on hold. Successive freezes will
 * nest, requiring an equal number of thaws.
 *
 * @see edje_object_thaw()
 */
EAPI int          edje_object_freeze                  (Evas_Object *obj);

/**
 * @brief Thaws the Edje object.
 *
 * @param obj A handle to an Edje object.
 * @return The frozen state or 0 if the object is not frozen or on error.
 *
 * This function thaws the given Edje object.
 *
 * @note: If sucessives freezes were done, an equal number of
 *        thaws will be required.
 *
 * @see edje_object_freeze()
 */
EAPI int          edje_object_thaw                    (Evas_Object *obj);

/**
 * @brief Returns the state of the Edje part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param val_ret
 *
 * @return The part state:\n
 * "default" for the default state\n
 * "" for other states
 */
EAPI const char      *edje_object_part_state_get      (const Evas_Object *obj, const char *part, double *val_ret);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Geometry_Group Edje Object Geometry
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
 * @ingroup Edje_Object_Group
 *
 * @{
 */

/**
 * @brief Get the minimum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum width
 * @param minh Pointer to a variable where to store the minimum height
 *
 * This function retrieves the @p obj object's minimum size values,
 * <b>as declared in its EDC group definition</b>. Minimum size of
 * groups have the following syntax
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     min: 100 100;
 *   }
 * }
 * @endcode
 *
 * where one declares a minimum size of 100 pixels both for width and
 * height. Those are (hint) values which should be respected when the
 * given object/group is to be controlled by a given container object
 * (e.g. an Edje object being "swallowed" into a given @c SWALLOW
 * typed part, as in edje_object_part_swallow()). Check the complete
 * @ref edcref "syntax reference" for EDC files.
 *
 * @note If the @c min EDC property was not declared for @p obj, this
 * call will return the value 0, for each axis.
 *
 * @note On failure, this function will make all non-@c NULL size
 * pointers' pointed variables be set to zero.
 *
 * @see edje_object_size_max_get()
 */
EAPI void         edje_object_size_min_get            (const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

/**
 * @brief Edje will automatically update the size hints on itself.
 *
 * @param obj A handle to an Edje object.
 * @param update Wether or not update the size hints.
 *
 * By default edje doesn't set size hints on itself. With this function
 * call, it will do so if update is true. Be carefully, it cost a lot to
 * trigger this feature as it will recalc the object every time it make
 * sense to be sure that's its minimal size hint is always accurate.
 */
EAPI void edje_object_update_hints_set(Evas_Object *obj, Eina_Bool update);

/**
 * @brief Wether or not Edje will update size hints on itself.
 *
 * @param obj A handle to an Edje object.
 * @return @c true if does, @c false if it doesn't.
 */
EAPI Eina_Bool edje_object_update_hints_get(Evas_Object *obj);

/**
 * @brief Get the maximum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param obj A handle to an Edje object
 * @param maxw Pointer to a variable where to store the maximum width
 * @param maxh Pointer to a variable where to store the maximum height
 *
 * This function retrieves the @p obj object's maximum size values,
 * <b>as declared in its EDC group definition</b>. Maximum size of
 * groups have the following syntax
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     max: 100 100;
 *   }
 * }
 * @endcode
 *
 * where one declares a maximum size of 100 pixels both for width and
 * height. Those are (hint) values which should be respected when the
 * given object/group is to be controlled by a given container object
 * (e.g. an Edje object being "swallowed" into a given @c SWALLOW
 * typed part, as in edje_object_part_swallow()). Check the complete
 * @ref edcref "syntax reference" for EDC files.
 *
 * @note If the @c max EDC property was not declared for @p obj, this
 * call will return the maximum size a given Edje object may have, for
 * each axis.
 *
 * @note On failure, this function will make all non-@c NULL size
 * pointers' pointed variables be set to zero.
 *
 * @see edje_object_size_min_get()
 */
EAPI void         edje_object_size_max_get            (const Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh);

/**
 * @brief Force a Size/Geometry calculation.
 *
 * @param obj A valid Evas_Object handle
 *
 * Forces the object @p obj to recalculation layout regardless of
 * freeze/thaw.
 */
EAPI void         edje_object_calc_force              (Evas_Object *obj);

/**
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum
 * required width
 * @param minh Pointer to a variable where to store the minimum
 * required height
 *
 * This call works exactly as edje_object_size_min_restricted_calc(),
 * with the last two arguments set to 0. Please refer to its
 * documentation, then.
 */
EAPI void         edje_object_size_min_calc           (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

/**
 * Calculate the geometry of the region, relative to a given Edje
 * object's area, <b>occupied by all parts in the object</b>
 *
 * @param obj A handle to an Edje object
 * @param x A pointer to a variable where to store the parts region's
 * x coordinate
 * @param y A pointer to a variable where to store the parts region's
 * y coordinate
 * @param w A pointer to a variable where to store the parts region's
 * width
 * @param h A pointer to a variable where to store the parts region's
 * height
 *
 * This function gets the geometry of the rectangle equal to the area
 * required to group all parts in @p obj's group/collection. The @p x
 * and @p y coordinates are relative to the top left corner of the
 * whole @p obj object's area. Parts placed out of the group's
 * boundaries will also be taken in account, so that @p x and @p y
 * <b>may be negative</b>.
 *
 * @note Use @c NULL pointers on the geometry components you're not
 * interested in: they'll be ignored by the function.
 *
 * @note On failure, this function will make all non-@c NULL geometry
 * pointers' pointed variables be set to zero.
 */
EAPI Eina_Bool    edje_object_parts_extends_calc      (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param minw Pointer to a variable where to store the minimum
 * required width
 * @param minh Pointer to a variable where to store the minimum
 * required height
 * @param restrictedw Do not allow object's calculated (minimum) width
 * to be less than this value
 * @param restrictedh Do not allow object's calculated (minimum)
 * height to be less than this value
 *
 * This call will trigger an internal recalculation of all parts of
 * the @p obj object, in order to return its minimum required
 * dimensions for width and height. The user might choose to @b impose
 * those minimum sizes, making the resulting calculation to get to values
 * equal or bigger than @p restrictedw and @p restrictedh, for width and
 * height, respectively.
 *
 * @note At the end of this call, @p obj @b won't be automatically
 * resized to new dimensions, but just return the calculated
 * sizes. The caller is the one up to change its geometry or not.
 *
 * @warning Be advised that invisible parts in @p obj @b will be taken
 * into account in this calculation.
 */
EAPI void         edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Part Edje Part
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
 * @ingroup Edje_Object_Group
 *
 * @{
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
 * @brief Check if an Edje part exists in a given Edje object's group
 * definition.
 *
 * @param obj A handle to an Edje object
 * @param part The part's name to check for existence in @p obj's
 * group
 * @return @c EINA_TRUE, if the Edje part exists in @p obj's group or
 * @c EINA_FALSE, otherwise (and on errors)
 *
 * This function returns if a given part exists in the Edje group
 * bound to object @p obj (with edje_object_file_set()).
 *
 * This call is useful, for example, when one could expect or not a
 * given GUI element, depending on the @b theme applied to @p obj.
 */
EAPI Eina_Bool    edje_object_part_exists             (const Evas_Object *obj, const char *part);

/**
 * @brief Get a handle to the Evas object implementing a given Edje
 * part, in an Edje object.
 *
 * @param obj A handle to an Edje object
 * @param part The Edje part's name
 * @return A pointer to the Evas object implementing the given part,
 * or @c NULL on failure (e.g. the given part doesn't exist)
 *
 * This function gets a pointer of the Evas object corresponding to a
 * given part in the @p obj object's group.
 *
 * You should @b never modify the state of the returned object (with
 * @c evas_object_move() or @c evas_object_hide() for example),
 * because it's meant to be managed by Edje, solely. You are safe to
 * query information about its current state (with @c
 * evas_object_visible_get() or @c evas_object_color_get() for
 * example), though.
 */
EAPI const Evas_Object *edje_object_part_object_get   (const Evas_Object *obj, const char *part);

/**
 * @brief Retrieve the geometry of a given Edje part, in a given Edje
 * object's group definition, <b>relative to the object's area</b>
 *
 * @param obj A handle to an Edje object
 * @param part The Edje part's name
 * @param x A pointer to a variable where to store the part's x
 * coordinate
 * @param y A pointer to a variable where to store the part's y
 * coordinate
 * @param w A pointer to a variable where to store the part's width
 * @param h A pointer to a variable where to store the part's height
 *
 * This function gets the geometry of an Edje part within its
 * group. The @p x and @p y coordinates are relative to the top left
 * corner of the whole @p obj object's area.
 *
 * @note Use @c NULL pointers on the geometry components you're not
 * interested in: they'll be ignored by the function.
 *
 * @note On failure, this function will make all non-@c NULL geometry
 * pointers' pointed variables be set to zero.
 */
EAPI Eina_Bool    edje_object_part_geometry_get       (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Retrieve a list all accessibility part names
 *
 * @param obj A valid Evas_Object handle
 * @return A list all accessibility part names on @p obj
 * @since 1.7.0
 */
EAPI Eina_List    *edje_object_access_part_list_get   (const Evas_Object *obj);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Text Edje Text Part
 *
 * @brief Functions that deal with parts of type text
 *
 * Text is an element type for parts. It's basic functionality is to
 * display a string on the layout, but a lot more things can be done
 * with texts, like string selection, setting the cursor and include
 * a input panel, where one can set a virtual keyboard to handle
 * keyboard entry easily.
 *
 * @ingroup Edje_Object_Part
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

   EDJE_TEXT_EFFECT_LAST                = 11,

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

typedef void         (*Edje_Text_Change_Cb)     (void *data, Evas_Object *obj, const char *part);

/**
 * @brief Set the object text callback.
 *
 * @param obj A valid Evas_Object handle
 * @param func The callback function to handle the text change
 * @param data The data associated to the callback function.
 *
 * This function sets the callback to be called when the text changes.
 */
EAPI void         edje_object_text_change_cb_set      (Evas_Object *obj, Edje_Text_Change_Cb func, void *data);

/**
 * @brief Sets the text for an object part
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 */
EAPI Eina_Bool    edje_object_part_text_set           (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Sets the text for an object part, but converts HTML escapes to UTF8
 *
 * This converts the given string @p text to UTF8 assuming it contains HTML
 * style escapes like "&amp;" and "&copy;" etc. IF the part is of type TEXT,
 * as opposed to TEXTBLOCK.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @since 1.2
 */
EAPI Eina_Bool    edje_object_part_text_escaped_set   (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Return the text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The text string
 *
 * This function returns the text associated to the object part.
 *
 * @see edje_object_part_text_set().
 */
EAPI const char  *edje_object_part_text_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Set the style of the
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param style The style to set (textblock conventions).
 *
 * This function sets the style associated with the textblock part.
 *
 * @since 1.2.0
 */
EAPI void edje_object_part_text_style_user_push(Evas_Object *obj, const char *part, const char *style);

/**
 * @brief Return the text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The text string
 *
 * This function returns the style associated with the textblock part.
 *
 * @since 1.2.0
 */
EAPI const char *edje_object_part_text_style_user_peek(const Evas_Object *obj, const char *part);

/**
 * @brief Delete the top style form the user style stack.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @since 1.2.0
 */
EAPI void edje_object_part_text_style_user_pop(Evas_Object *obj, const char *part);

/**
 * @brief Sets the raw (non escaped) text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text_to_escape The text string
 *
 * This funciton will not do escape for you if it is a TEXTBLOCK part, that is,
 * if text contain tags, these tags will not be interpreted/parsed by TEXTBLOCK.
 *
 * @see edje_object_part_text_unescaped_get().
 */
EAPI Eina_Bool    edje_object_part_text_unescaped_set (Evas_Object *obj, const char *part, const char *text_to_escape);

/**
 * @brief Returns the text of the object part, without escaping.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The @b allocated text string without escaping, or NULL on
 * problems.
 *
 * This function is the counterpart of
 * edje_object_part_text_unescaped_set(). Please notice that the
 * result is newly allocated memory and should be released with free()
 * when done.
 *
 * @see edje_object_part_text_unescaped_set().
 */
EAPI char        *edje_object_part_text_unescaped_get (const Evas_Object *obj, const char *part);

/**
 * @brief Insert text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * This function inserts the text for an object part just before the
 * cursor position.
 *
 */
EAPI void             edje_object_part_text_insert                  (Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Insert text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * This function inserts the text for an object part at the end; It does not
 * move the cursor.
 *
 * @since 1.1
 */
EAPI void             edje_object_part_text_append(Evas_Object *obj, const char *part, const char *text);

/**
 * @brief Return a list of char anchor names.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The list of anchors (const char *), do not modify!
 *
 * This function returns a list of char anchor names.
 *
 */
EAPI const Eina_List *edje_object_part_text_anchor_list_get         (const Evas_Object *obj, const char *part);

/**
 * @brief Return a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param anchor The anchor name
 *
 * @return The list of anchor rects (const Evas_Textblock_Rectangle
 * *), do not modify! Geometry is relative to entry part.
 *
 * This function return a list of Evas_Textblock_Rectangle anchor
 * rectangles.
 *
 */
EAPI const Eina_List *edje_object_part_text_anchor_geometry_get     (const Evas_Object *obj, const char *part, const char *anchor);

/**
 * @brief Return a list of char item names.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The list of items (const char *), do not modify!
 *
 * This function returns a list of char item names.
 *
 */
EAPI const Eina_List *edje_object_part_text_item_list_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Return item geometry.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param item The item name
 * @param cx Item x return (relative to entry part)
 * @param cy Item y return (relative to entry part)
 * @param cw Item width return
 * @param ch Item height return
 *
 * @return 1 if item exists, 0 if not
 *
 * This function return a list of Evas_Textblock_Rectangle item
 * rectangles.
 *
 */
EAPI Eina_Bool        edje_object_part_text_item_geometry_get       (const Evas_Object *obj, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);

/**
 * @brief This function inserts text as if the user has inserted it.
 *
 * This means it actually registers as a change and emits signals, triggers
 * callbacks as appropriate.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param text The text string
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_user_insert        (const Evas_Object *obj, const char *part, const char *text);

/**
 * @}
 */

/**
 * @defgroup Edje_Text_Selection Edje Text Selection
 *
 * @brief Functions that deal with selection in text parts
 *
 * Selection is a known functionality for texts in the whole computational
 * world. It is a block of text marked for further manipulation.
 *
 * Edje is responsible for handling this functionality through the
 * following functions.
 *
 * @ingroup Edje_Part_Text
 *
 * @{
 */

/**
 * @brief Return the selection text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The text string
 *
 * This function returns selection text of the object part.
 *
 * @see edje_object_part_text_select_all()
 * @see edje_object_part_text_select_none()
 */
EAPI const char      *edje_object_part_text_selection_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Set the selection to be none.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * This function sets the selection text to be none.
 */
EAPI void             edje_object_part_text_select_none             (const Evas_Object *obj, const char *part);

/**
 * @brief Set the selection to be everything.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * This function selects all text of the object of the part.
 */
EAPI void             edje_object_part_text_select_all              (const Evas_Object *obj, const char *part);

/**
 * @brief Enables selection if the entry is an EXPLICIT selection mode
 * type.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param allow EINA_TRUE to enable, EINA_FALSE otherwise
 *
 * The default is to @b not allow selection. This function only affects user
 * selection, functions such as edje_object_part_text_select_all() and
 * edje_object_part_text_select_none() are not affected.
 */
EAPI void             edje_object_part_text_select_allow_set        (const Evas_Object *obj, const char *part, Eina_Bool allow);

/**
 * @brief Aborts any selection action on a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_abort            (const Evas_Object *obj, const char *part);

/**
 * @brief Starts selecting at current cursor position
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_begin            (const Evas_Object *obj, const char *part);

/**
 * @brief Extends the current selection to the current cursor position
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void             edje_object_part_text_select_extend           (const Evas_Object *obj, const char *part);

/**
 * @}
 */

/**
 * @defgroup Edje_Text_Cursor Edje Text Cursor
 *
 * @brief Functions that deal with cursor in text parts
 *
 * Cursor is a known functionality for texts in the whole computational
 * world. It marks a position in the text from where one may want
 * to make a insertion, deletion or selection.
 *
 * Edje is responsible for handling this functionality through the
 * following functions.
 *
 * @ingroup Edje_Part_Text
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
 * @see evas_textblock_cursor_char_next
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The edje cursor to advance
 */
EAPI Eina_Bool        edje_object_part_text_cursor_next                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the previous char
 * @see evas_textblock_cursor_char_prev
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_prev                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Move the cursor to the char above the current cursor position.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_up                   (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the char below the current cursor position.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI Eina_Bool        edje_object_part_text_cursor_down                 (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the beginning of the text part
 * @see evas_textblock_cursor_paragraph_first
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_begin_set            (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Moves the cursor to the end of the text part.
 * @see evas_textblock_cursor_paragraph_last
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_end_set              (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Copy the cursor to another cursor.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param src the cursor to copy from
 * @param dst the cursor to copy to
 */
EAPI void             edje_object_part_text_cursor_copy                 (Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst);

/**
 * @brief Move the cursor to the beginning of the line.
 * @see evas_textblock_cursor_line_char_first
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_line_begin_set       (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Move the cursor to the end of the line.
 * @see evas_textblock_cursor_line_char_last
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur the edje cursor to work on
 */
EAPI void             edje_object_part_text_cursor_line_end_set         (Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * Position the given cursor to a X,Y position.
 *
 * This is frequently used with the user cursor.
 *
 * @param obj An Edje object.
 * @param part The part containing the object.
 * @param cur The cursor to adjust.
 * @param x X Coordinate.
 * @param y Y Coordinate.
 * @return True on success, false on error.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_coord_set            (Evas_Object *obj, const char *part, Edje_Cursor cur, Evas_Coord x, Evas_Coord y);

/**
 * @brief Returns whether the cursor points to a format.
 * @see evas_textblock_cursor_is_format
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to adjust.
 * @return EINA_TRUE if it's true, EINA_FALSE otherwise.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_is_format_get        (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Return true if the cursor points to a visible format
 * For example \\t, \\n, item and etc.
 * @see  evas_textblock_cursor_format_is_visible_get
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to adjust.
 */
EAPI Eina_Bool        edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Returns the content (char) at the cursor position.
 * @see evas_textblock_cursor_content_get
 *
 * You must free the return (if not NULL) after you are done with it.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to use
 * @return The character string pointed to (may be a multi-byte utf8 sequence) terminated by a nul byte.
 */
EAPI char            *edje_object_part_text_cursor_content_get          (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Sets the cursor position to the given value
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to move
 * @param pos the position of the cursor
 * @since 1.1.0
 */
EAPI void             edje_object_part_text_cursor_pos_set              (Evas_Object *obj, const char *part, Edje_Cursor cur, int pos);

/**
 * @brief Retrieves the current position of the cursor
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to get the position
 * @return The cursor position
 * @since 1.1.0
 */
EAPI int              edje_object_part_text_cursor_pos_get              (const Evas_Object *obj, const char *part, Edje_Cursor cur);

/**
 * @brief Returns the cursor geometry of the part relative to the edje
 * object.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param x Cursor X position
 * @param y Cursor Y position
 * @param w Cursor width
 * @param h Cursor height
 *
 */
EAPI void             edje_object_part_text_cursor_geometry_get     (const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @}
 */

/**
 * @defgroup Edje_Text_Entry Edje Text Entry
 *
 * @brief Functions that deal with text entries
 *
 * In Edje it's possible to use a text part as a entry so the user is
 * able to make inputs of text. To do so, the text part must be set
 * with a input panel taht will work as a virtual keyboard.
 *
 * Some of effects can be applied to the entered text and also plenty
 * actions can be performed after any input.
 *
 * Use the functions of this section to handle the user input of text.
 *
 * @ingroup Edje_Part_Text
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

typedef enum _Edje_Input_Panel_Lang
{
   EDJE_INPUT_PANEL_LANG_AUTOMATIC,    /**< Automatic @since 1.2 */
   EDJE_INPUT_PANEL_LANG_ALPHABET      /**< Alphabet @since 1.2 */
} Edje_Input_Panel_Lang;

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

typedef enum _Edje_Input_Panel_Layout
{
   EDJE_INPUT_PANEL_LAYOUT_NORMAL,          /**< Default layout */
   EDJE_INPUT_PANEL_LAYOUT_NUMBER,          /**< Number layout */
   EDJE_INPUT_PANEL_LAYOUT_EMAIL,           /**< Email layout */
   EDJE_INPUT_PANEL_LAYOUT_URL,             /**< URL layout */
   EDJE_INPUT_PANEL_LAYOUT_PHONENUMBER,     /**< Phone Number layout */
   EDJE_INPUT_PANEL_LAYOUT_IP,              /**< IP layout */
   EDJE_INPUT_PANEL_LAYOUT_MONTH,           /**< Month layout */
   EDJE_INPUT_PANEL_LAYOUT_NUMBERONLY,      /**< Number Only layout */
   EDJE_INPUT_PANEL_LAYOUT_INVALID,         /**< Never use this */
   EDJE_INPUT_PANEL_LAYOUT_HEX,             /**< Hexadecimal layout @since 1.2 */
   EDJE_INPUT_PANEL_LAYOUT_TERMINAL,        /**< Command-line terminal layout including esc, alt, ctrl key, so on (no auto-correct, no auto-capitalization) @since 1.2 */
   EDJE_INPUT_PANEL_LAYOUT_PASSWORD         /**< Like normal, but no auto-correct, no auto-capitalization etc. @since 1.2 */
} Edje_Input_Panel_Layout;

typedef void         (*Edje_Text_Filter_Cb)     (void *data, Evas_Object *obj, const char *part, Edje_Text_Filter_Type type, char **text);
typedef void         (*Edje_Markup_Filter_Cb)   (void *data, Evas_Object *obj, const char *part, char **text);
typedef Evas_Object *(*Edje_Item_Provider_Cb)   (void *data, Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Show last character in password mode.
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
 * @brief Set's the timeout value in last show password mode.
 *
 * @param password_show_last_timeout The timeout value.
 *
 * This functions sets the time out value for which the last input entered in password
 * mode will be visible.
 *
 * This value can be used only when last show mode is set in password mode.
 *
 * @see edje_password_show_last_set().
 *
 */
EAPI void edje_password_show_last_timeout_set(double password_show_last_timeout);

/**
 * @brief Set the RTL orientation for this object.
 *
 * @param obj A handle to an Edje object.
 * @param rtl new value of flag EINA_TRUE/EINA_FALSE
 * @since 1.1.0
 */
EAPI void         edje_object_mirrored_set        (Evas_Object *obj, Eina_Bool rtl);

/**
 * @brief Get the RTL orientation for this object.
 *
 * You can RTL orientation explicitly with edje_object_mirrored_set.
 *
 * @param obj A handle to an Edje object.
 * @return @c EINA_TRUE if the flag is set or @c EINA_FALSE if not.
 * @since 1.1.0
 */
EAPI Eina_Bool    edje_object_mirrored_get        (const Evas_Object *obj);

/**
 * @brief Set the function that provides item objects for named items in an edje entry text
 *
 * @param obj A valid Evas Object handle
 * @param func The function to call (or NULL to disable) to get item objects
 * @param data The data pointer to pass to the @p func callback
 *
 * Item objects may be deleted any time by Edje, and will be deleted when the
 * Edje object is deleted (or file is set to a new file).
 */
EAPI void         edje_object_item_provider_set       (Evas_Object *obj, Edje_Item_Provider_Cb func, void *data);

/**
 * @brief Reset the input method context if needed.
 *
 * This can be necessary in the case where modifying the buffer would confuse on-going input method behavior
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_imf_context_reset           (const Evas_Object *obj, const char *part);

/**
 * @brief Get the input method context in entry.
 *
 * If ecore_imf was not available when edje was compiled, this function returns NULL
 * otherwise, the returned pointer is an Ecore_IMF *
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The input method context (Ecore_IMF_Context *) in entry
 * @since 1.2.0
 */
EAPI void              *edje_object_part_text_imf_context_get           (const Evas_Object *obj, const char *part);

/**
 * @brief Set the layout of the input panel.
 *
 * The layout of the input panel or virtual keyboard can make it easier or
 * harder to enter content. This allows you to hint what kind of input you
 * are expecting to enter and thus have the input panel automatically
 * come up with the right mode.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param layout layout type
 * @since 1.1
 */
EAPI void             edje_object_part_text_input_panel_layout_set      (Evas_Object *obj, const char *part, Edje_Input_Panel_Layout layout);

/**
 * @brief Get the layout of the input panel.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return Layout type of the input panel
 *
 * @see edje_object_part_text_input_panel_layout_set
 * @since 1.1
 */
EAPI Edje_Input_Panel_Layout edje_object_part_text_input_panel_layout_get (const Evas_Object *obj, const char *part);

/**
 * @brief Set the autocapitalization type on the immodule.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param autocapital_type The type of autocapitalization
 * @since 1.1.0
 */
EAPI void         edje_object_part_text_autocapital_type_set            (Evas_Object *obj, const char *part, Edje_Text_Autocapital_Type autocapital_type);

/**
 * @brief Retrieves the autocapitalization type
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The autocapitalization type
 * @since 1.1.0
 */
EAPI Edje_Text_Autocapital_Type edje_object_part_text_autocapital_type_get (const Evas_Object *obj, const char *part);

/**
 * @brief Set whether the prediction is allowed or not.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param prediction If true, the prediction feature is allowed.
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_prediction_allow_set        (Evas_Object *obj, const char *part, Eina_Bool prediction);

/**
 * @brief Get whether the prediction is allowed or not.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return EINA_TRUE if prediction feature is allowed.
 * @since 1.2.0
 */
EAPI Eina_Bool        edje_object_part_text_prediction_allow_get        (const Evas_Object *obj, const char *part);

/**
 * @brief Sets the attribute to show the input panel automatically.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param enabled If true, the input panel is appeared when entry is clicked or has a focus
 * @since 1.1.0
 */
EAPI void             edje_object_part_text_input_panel_enabled_set     (Evas_Object *obj, const char *part, Eina_Bool enabled);

/**
 * @brief Retrieve the attribute to show the input panel automatically.
 * @see edje_object_part_text_input_panel_enabled_set
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return EINA_TRUE if it supports or EINA_FALSE otherwise
 * @since 1.1.0
 */
EAPI Eina_Bool        edje_object_part_text_input_panel_enabled_get (const Evas_Object *obj, const char *part);

/**
 * @brief Show the input panel (virtual keyboard) based on the input panel property such as layout, autocapital types, and so on.
 *
 * Note that input panel is shown or hidden automatically according to the focus state.
 * This API can be used in the case of manually controlling by using edje_object_part_text_input_panel_enabled_set.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_show(const Evas_Object *obj, const char *part);

/**
 * @brief Hide the input panel (virtual keyboard).
 * @see edje_object_part_text_input_panel_show
 *
 * Note that input panel is shown or hidden automatically according to the focus state.
 * This API can be used in the case of manually controlling by using edje_object_part_text_input_panel_enabled_set.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_hide(const Evas_Object *obj, const char *part);

/**
 * Set the language mode of the input panel.
 *
 * This API can be used if you want to show the Alphabet keyboard.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param lang the language to be set to the input panel.
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_language_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Lang lang);

/**
 * Get the language mode of the input panel.
 *
 * See @ref edje_object_part_text_input_panel_language_set for more details.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return input panel language type
 * @since 1.2.0
 */
EAPI Edje_Input_Panel_Lang edje_object_part_text_input_panel_language_get(const Evas_Object *obj, const char *part);

/**
 * Set the input panel-specific data to deliver to the input panel.
 *
 * This API is used by applications to deliver specific data to the input panel.
 * The data format MUST be negotiated by both application and the input panel.
 * The size and format of data are defined by the input panel.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param data The specific data to be set to the input panel.
 * @param len the length of data, in bytes, to send to the input panel
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_imdata_set(Evas_Object *obj, const char *part, const void *data, int len);

/**
 * Get the specific data of the current active input panel.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param data The specific data to be got from the input panel
 * @param len The length of data
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_imdata_get(const Evas_Object *obj, const char *part, void *data, int *len);

/**
 * Set the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * An input panel displays the string or icon associated with this type
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param return_key_type The type of "return" key on the input panel
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_return_key_type_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Return_Key_Type return_key_type);

/**
 * Get the "return" key type.
 *
 * @see edje_object_part_text_input_panel_return_key_type_set() for more details
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The type of "return" key on the input panel
 * @since 1.2.0
 */
EAPI Edje_Input_Panel_Return_Key_Type edje_object_part_text_input_panel_return_key_type_get(const Evas_Object *obj, const char *part);

/**
 * Set the return key on the input panel to be disabled.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param disabled The state
 * @since 1.2.0
 */
EAPI void             edje_object_part_text_input_panel_return_key_disabled_set(Evas_Object *obj, const char *part, Eina_Bool disabled);

/**
 * Get whether the return key on the input panel should be disabled or not.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return EINA_TRUE if it should be disabled
 * @since 1.2.0
 */
EAPI Eina_Bool        edje_object_part_text_input_panel_return_key_disabled_get(const Evas_Object *obj, const char *part);

/**
 * Add a filter function for newly inserted text.
 *
 * Whenever text is inserted (not the same as set) into the given @p part,
 * the list of filter functions will be called to decide if and how the new
 * text will be accepted.
 * There are three types of filters, EDJE_TEXT_FILTER_TEXT,
 * EDJE_TEXT_FILTER_FORMAT and EDJE_TEXT_FILTER_MARKUP.
 * The text parameter in the @p func filter can be modified by the user and
 * it's up to him to free the one passed if he's to change the pointer. If
 * doing so, the newly set text should be malloc'ed, as once all the filters
 * are called Edje will free it.
 * If the text is to be rejected, freeing it and setting the pointer to NULL
 * will make Edje break out of the filter cycle and reject the inserted
 * text.
 *
 * @warning This function will be deprecated because of difficulty in use.
 *          The type(format, text, or markup) of text should be always
 *          checked in the filter function for correct filtering.
 *          Please use edje_object_text_markup_filter_callback_add() instead. There
 *          is no need to check the type of text in the filter function
 *          because the text is always markup.
 * @warning If you use this function with
 *          edje_object_text_markup_filter_callback_add() together, all
 *          Edje_Text_Filter_Cb functions and Edje_Markup_Filter_Cb functions
 *          will be executed, and then filtered text will be inserted.
 *
 * @see edje_object_text_insert_filter_callback_del
 * @see edje_object_text_insert_filter_callback_del_full
 * @see edje_object_text_markup_filter_callback_add
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The callback function that will act as filter
 * @param data User provided data to pass to the filter function
 */
EAPI void             edje_object_text_insert_filter_callback_add       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * Delete a function from the filter list.
 *
 * Delete the given @p func filter from the list in @p part. Returns
 * the user data pointer given when added.
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del_full
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 *
 * @return The user data pointer if successful, or NULL otherwise
 */
EAPI void            *edje_object_text_insert_filter_callback_del       (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func);

/**
 * Delete a function and matching user data from the filter list.
 *
 * Delete the given @p func filter and @p data user data from the list
 * in @p part.
 * Returns the user data pointer given when added.
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 * @param data The data passed to the callback function
 *
 * @return The same data pointer if successful, or NULL otherwise
 */
EAPI void            *edje_object_text_insert_filter_callback_del_full  (Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data);

/**
 * Add a markup filter function for newly inserted text.
 *
 * Whenever text is inserted (not the same as set) into the given @p part,
 * the list of markup filter functions will be called to decide if and how
 * the new text will be accepted.
 * The text parameter in the @p func filter is always markup. It can be
 * modified by the user and it's up to him to free the one passed if he's to
 * change the pointer. If doing so, the newly set text should be malloc'ed,
 * as once all the filters are called Edje will free it.
 * If the text is to be rejected, freeing it and setting the pointer to NULL
 * will make Edje break out of the filter cycle and reject the inserted
 * text.
 * This function is different from edje_object_text_insert_filter_callback_add()
 * in that the text parameter in the @p fucn filter is always markup.
 *
 * @warning If you use this function with
 *          edje_object_text_insert_filter_callback_add() togehter, all
 *          Edje_Text_Filter_Cb functions and Edje_Markup_Filter_Cb functions
 *          will be executed, and then filtered text will be inserted.
 *
 * @see edje_object_text_markup_filter_callback_del
 * @see edje_object_text_markup_filter_callback_del_full
 * @see edje_object_text_insert_filter_callback_add
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The callback function that will act as markup filter
 * @param data User provided data to pass to the filter function
 * @since 1.2.0
 */
EAPI void edje_object_text_markup_filter_callback_add(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data);

/**
 * Delete a function from the markup filter list.
 *
 * Delete the given @p func filter from the list in @p part. Returns
 * the user data pointer given when added.
 *
 * @see edje_object_text_markup_filter_callback_add
 * @see edje_object_text_markup_filter_callback_del_full
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 *
 * @return The user data pointer if successful, or NULL otherwise
 * @since 1.2.0
 */
EAPI void *edje_object_text_markup_filter_callback_del(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func);

/**
 * Delete a function and matching user data from the markup filter list.
 *
 * Delete the given @p func filter and @p data user data from the list
 * in @p part.
 * Returns the user data pointer given when added.
 *
 * @see edje_object_text_markup_filter_callback_add
 * @see edje_object_text_markup_filter_callback_del
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 * @param data The data passed to the callback function
 *
 * @return The same data pointer if successful, or NULL otherwise
 * @since 1.2.0
 */
EAPI void *edje_object_text_markup_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Swallow Edje Swallow Part
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
 * @ingroup Edje_Object_Part
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
 * @brief Set the object minimum size.
 *
 * @param obj A valid Evas_Object handle
 * @param minw The minimum width
 * @param minh The minimum height
 *
 * This sets the minimum size restriction for the object.
 */
EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);

/**
 * @brief Set the object maximum size.
 *
 * @param obj A valid Evas_Object handle
 * @param maxw The maximum width
 * @param maxh The maximum height
 *
 * This sets the maximum size restriction for the object.
 */
EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);

/**
 * @brief Set the object aspect size.
 *
 * @param obj A valid Evas_Object handle
 * @param aspect The aspect control axes
 * @param aw The aspect radio width
 * @param ah The aspect ratio height
 *
 * This sets the desired aspect ratio to keep an object that will be
 * swallowed by Edje. The width and height define a preferred size
 * ASPECT and the object may be scaled to be larger or smaller, but
 * retaining the relative scale of both aspect width and height.
 */
EAPI void         edje_extern_object_aspect_set   (Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);

/**
 * @brief "Swallows" an object into one of the Edje object @c SWALLOW
 * parts.
 *
 * @param obj A valid Edje object handle
 * @param part The swallow part's name
 * @param obj_swallow The object to occupy that part
 *
 * Swallowing an object into an Edje object is, for a given part of
 * type @c SWALLOW in the EDC group which gave life to @a obj, to set
 * an external object to be controlled by @a obj, being displayed
 * exactly over that part's region inside the whole Edje object's
 * viewport.
 *
 * From this point on, @a obj will have total control over @a
 * obj_swallow's geometry and visibility. For instance, if @a obj is
 * visible, as in @c evas_object_show(), the swallowed object will be
 * visible too -- if the given @c SWALLOW part it's in is also
 * visible. Other actions on @a obj will also reflect on the swallowed
 * object as well (e.g. resizing, moving, raising/lowering, etc.).
 *
 * Finally, all internal changes to @a part, specifically, will
 * reflect on the displaying of @a obj_swallow, for example state
 * changes leading to different visibility states, geometries,
 * positions, etc.
 *
 * If an object has already been swallowed into this part, then it
 * will first be unswallowed (as in edje_object_part_unswallow())
 * before the new object is swallowed.
 *
 * @note @a obj @b won't delete the swallowed object once it is
 * deleted -- @a obj_swallow will get to an unparented state again.
 *
 * For more details on EDC @c SWALLOW parts, see @ref edcref "syntax
 * reference".
 */
EAPI Eina_Bool        edje_object_part_swallow        (Evas_Object *obj, const char *part, Evas_Object *obj_swallow);

/**
 * @brief Unswallow an object.
 *
 * @param obj A valid Evas_Object handle
 * @param obj_swallow The swallowed object
 *
 * Causes the edje to regurgitate a previously swallowed object. :)
 *
 * @note @p obj_swallow will @b not be deleted or hidden.
 * @note @p obj_swallow may appear shown on the evas depending on its state when
 * it got unswallowed. Make sure you delete it or hide it if you do not want it to.
 */
EAPI void             edje_object_part_unswallow      (Evas_Object *obj, Evas_Object *obj_swallow);

/**
 * @brief Get the object currently swallowed by a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The swallowed object, or NULL if there is none.
 */
EAPI Evas_Object     *edje_object_part_swallow_get    (const Evas_Object *obj, const char *part);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Drag Edje Drag
 *
 * @brief Functions that deal with dragable parts.
 *
 * To create a movable part it must be declared as dragable
 * in EDC file. To do so, one must define a "dragable" block inside
 * the "part" block.
 *
 * These functions are used to set dragging properties to a
 * part or get dragging information about it.
 *
 * @see @ref tutorial_edje_drag
 *
 * @ingroup Edje_Object_Part
 *
 * @{
 */

typedef enum _Edje_Drag_Dir
{
   EDJE_DRAG_DIR_NONE = 0,
   EDJE_DRAG_DIR_X = 1,
   EDJE_DRAG_DIR_Y = 2,
   EDJE_DRAG_DIR_XY = 3
} Edje_Drag_Dir;

/**
 * @brief Determine dragable directions.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * The dragable directions are defined in the EDC file, inside the @c dragable
 * section, by the attributes @c x and @c y. See the @ref edcref for more
 * information.
 *
 * @return #EDJE_DRAG_DIR_NONE: Not dragable\n
 * #EDJE_DRAG_DIR_X: Dragable in X direction\n
 * #EDJE_DRAG_DIR_Y: Dragable in Y direction\n
 * #EDJE_DRAG_DIR_XY: Dragable in X & Y directions
 */
EAPI Edje_Drag_Dir    edje_object_part_drag_dir_get   (const Evas_Object *obj, const char *part);

/**
 * @brief Set the dragable object location.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x value
 * @param dy The y value
 *
 * Places the dragable object at the given location.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1,
 * representing the relative position to the dragable area on that axis.
 *
 * This value means, for the vertical axis, that 0.0 will be at the top if the
 * first parameter of @c y in the dragable part theme is 1, and at bottom if it
 * is -1.
 *
 * For the horizontal axis, 0.0 means left if the first parameter of @c x in the
 * dragable part theme is 1, and right if it is -1.
 *
 * @see edje_object_part_drag_value_get()
 */
EAPI Eina_Bool        edje_object_part_drag_value_set (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Get the dragable object location.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The X value pointer
 * @param dy The Y value pointer
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1,
 * representing the relative position to the dragable area on that axis.
 *
 * @see edje_object_part_drag_value_set()
 *
 * Gets the drag location values.
 */
EAPI Eina_Bool        edje_object_part_drag_value_get (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Set the dragable object size.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width
 * @param dh The drag height
 *
 * Values for @p dw and @p dh are real numbers that range from 0 to 1,
 * representing the relative size of the dragable area on that axis.
 *
 * Sets the size of the dragable object.
 *
 * @see edje_object_part_drag_size_get()
 */
EAPI Eina_Bool        edje_object_part_drag_size_set  (Evas_Object *obj, const char *part, double dw, double dh);

/**
 * @brief Get the dragable object size.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width pointer
 * @param dh The drag height pointer
 *
 * Gets the dragable object size.
 *
 * @see edje_object_part_drag_size_set()
 */
EAPI Eina_Bool        edje_object_part_drag_size_get  (const Evas_Object *obj, const char *part, double *dw, double *dh);

/**
 * @brief Sets the drag step increment.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step amount
 * @param dy The y step amount
 *
 * Sets the x,y step increments for a dragable object.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1,
 * representing the relative size of the dragable area on that axis by which the
 * part will be moved.
 *
 * @see edje_object_part_drag_step_get()
 */
EAPI Eina_Bool        edje_object_part_drag_step_set  (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the drag step increment values.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part
 * @param dx The x step increment pointer
 * @param dy The y step increment pointer
 *
 * Gets the x and y step increments for the dragable object.
 *
 *
 * @see edje_object_part_drag_step_set()
 */
EAPI Eina_Bool        edje_object_part_drag_step_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Sets the page step increments.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x page step increment
 * @param dy The y page step increment
 *
 * Sets the x,y page step increment values.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1,
 * representing the relative size of the dragable area on that axis by which the
 * part will be moved.
 *
 * @see edje_object_part_drag_page_get()
 */
EAPI Eina_Bool        edje_object_part_drag_page_set  (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Gets the page step increments.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The dx page increment pointer
 * @param dy The dy page increment pointer
 *
 * Gets the x,y page step increments for the dragable object.
 *
 * @see edje_object_part_drag_page_set()
 */
EAPI Eina_Bool        edje_object_part_drag_page_get  (const Evas_Object *obj, const char *part, double *dx, double *dy);

/**
 * @brief Steps the dragable x,y steps.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Steps x,y where the step increment is the amount set by
 * edje_object_part_drag_step_set.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1.
 *
 * @see edje_object_part_drag_page()
 */
EAPI Eina_Bool        edje_object_part_drag_step      (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @brief Pages x,y steps.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Pages x,y where the increment is defined by
 * edje_object_part_drag_page_set.
 *
 * Values for @p dx and @p dy are real numbers that range from 0 to 1.
 *
 * @warning Paging is bugged!
 *
 * @see edje_object_part_drag_step()
 */
EAPI Eina_Bool        edje_object_part_drag_page      (Evas_Object *obj, const char *part, double dx, double dy);

/**
 * @}
 */

/**
 * @defgroup Edje_Part_Box Edje Box Part
 *
 * @brief Functions that deal with parts of type box.
 *
 * Box is a container type for parts, that means it can contain
 * other parts.
 *
 * @ingroup Edje_Object_Part
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
 * @brief Appends an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to append
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Appends child to the box indicated by part.
 *
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_before()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_append             (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Prepends an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to prepend
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Prepends child to the box indicated by part.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_insert_before()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_prepend            (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Adds an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to insert
 * @param reference The object to be used as reference
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Inserts child in the box given by part, in the position marked by
 * reference.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_at()
 */
EAPI Eina_Bool    edje_object_part_box_insert_before      (Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * @brief Inserts an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to insert
 * @param pos The position where to insert child
 *
 * @return @c EINA_TRUE: Successfully added.\n
 * @c EINA_FALSE: An error occurred.
 *
 * Adds child to the box indicated by part, in the position given by
 * pos.
 *
 * @see edje_object_part_box_append()
 * @see edje_object_part_box_prepend()
 * @see edje_object_part_box_insert_before()
 */
EAPI Eina_Bool    edje_object_part_box_insert_at          (Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);

/**
 * @brief Removes an object from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to remove
 *
 * @return Pointer to the object removed, or @c NULL.
 *
 * Removes child from the box indicated by part.
 *
 * @see edje_object_part_box_remove_at()
 * @see edje_object_part_box_remove_all()
 */
EAPI Evas_Object *edje_object_part_box_remove             (Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Removes an object from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param pos The position index of the object (starts counting from 0)
 *
 * @return Pointer to the object removed, or @c NULL.
 *
 * Removes from the box indicated by part, the object in the position
 * pos.
 *
 * @see edje_object_part_box_remove()
 * @see edje_object_part_box_remove_all()
 */
EAPI Evas_Object *edje_object_part_box_remove_at          (Evas_Object *obj, const char *part, unsigned int pos);

/**
 * @brief Removes all elements from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param clear Delete objects on removal
 *
 * @return 1: Successfully cleared.\n
 * 0: An error occurred.
 *
 * Removes all the external objects from the box indicated by part.
 * Elements created from the theme will not be removed.
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
 *
 * @brief Functions that deal with parts of type table.
 *
 * Table is a container type for parts, that means it can contain
 * other parts.
 *
 * @ingroup Edje_Object_Part
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
 * @brief Retrieve a child from a table
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param col The column of the child to get
 * @param row The row of the child to get
 * @return The child Evas_Object
 */
EAPI Evas_Object *edje_object_part_table_child_get        (const Evas_Object *obj, const char *part, unsigned int col, unsigned int row);

/**
 * @brief Packs an object into the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child_obj The object to pack in
 * @param col The column to place it in
 * @param row The row to place it in
 * @param colspan Columns the child will take
 * @param rowspan Rows the child will take
 *
 * @return @c EINA_TRUE object was added, @c EINA_FALSE on failure
 *
 * Packs an object into the table indicated by part.
 */
EAPI Eina_Bool    edje_object_part_table_pack             (Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);

/**
 * @brief Removes an object from the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child_obj The object to pack in
 *
 * @return @c EINA_TRUE object removed, @c EINA_FALSE on failure
 *
 * Removes an object from the table indicated by part.
 */
EAPI Eina_Bool    edje_object_part_table_unpack           (Evas_Object *obj, const char *part, Evas_Object *child_obj);

/**
 * @brief Gets the number of columns and rows the table has.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cols Pointer where to store number of columns (can be NULL)
 * @param rows Pointer where to store number of rows (can be NULL)
 *
 * @return @c EINA_TRUE get some data, @c EINA_FALSE on failure
 *
 * Retrieves the size of the table in number of columns and rows.
 */
EAPI Eina_Bool    edje_object_part_table_col_row_size_get (const Evas_Object *obj, const char *part, int *cols, int *rows);

/**
 * @brief Removes all object from the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param clear If set, will delete subobjs on remove
 *
 * @return @c EINA_TRUE clear the table, @c EINA_FALSE on failure
 *
 * Removes all object from the table indicated by part, except the
 * internal ones set from the theme.
 */
EAPI Eina_Bool    edje_object_part_table_clear            (Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @}
 */

/**
 * @defgroup Edje_Object_Communication_Interface_Message Edje Communication Interface: Message
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
 * @ingroup Edje_Object_Group
 *
 * @{
 */

/**
 * Identifiers of Edje message types, which can be sent back and forth
 * code and a given Edje object's theme file/group.
 *
 * @see edje_object_message_send()
 * @see edje_object_message_handler_set()
 */
typedef enum _Edje_Message_Type
{
   EDJE_MESSAGE_NONE = 0,

   EDJE_MESSAGE_SIGNAL = 1, /* DONT USE THIS */

   EDJE_MESSAGE_STRING = 2, /**< A message with a string as value. Use #Edje_Message_String structs as message body, for this type. */
   EDJE_MESSAGE_INT = 3, /**< A message with an integer number as value. Use #Edje_Message_Int structs as message body, for this type. */
   EDJE_MESSAGE_FLOAT = 4, /**< A message with a floating pointer number as value. Use #Edje_Message_Float structs as message body, for this type. */

   EDJE_MESSAGE_STRING_SET = 5, /**< A message with a list of strings as value. Use #Edje_Message_String_Set structs as message body, for this type. */
   EDJE_MESSAGE_INT_SET = 6, /**< A message with a list of integer numbers as value. Use #Edje_Message_Int_Set structs as message body, for this type. */
   EDJE_MESSAGE_FLOAT_SET = 7, /**< A message with a list of floating point numbers as value. Use #Edje_Message_Float_Set structs as message body, for this type. */

   EDJE_MESSAGE_STRING_INT = 8, /**< A message with a struct containing a string and an integer number as value. Use #Edje_Message_String_Int structs as message body, for this type. */
   EDJE_MESSAGE_STRING_FLOAT = 9, /**< A message with a struct containing a string and a floating point number as value. Use #Edje_Message_String_Float structs as message body, for this type. */

   EDJE_MESSAGE_STRING_INT_SET = 10, /**< A message with a struct containing a string and list of integer numbers as value. Use #Edje_Message_String_Int_Set structs as message body, for this type. */
   EDJE_MESSAGE_STRING_FLOAT_SET = 11 /**< A message with a struct containing a string and list of floating point numbers as value. Use #Edje_Message_String_Float_Set structs as message body, for this type. */
} Edje_Message_Type;

typedef struct _Edje_Message_String           Edje_Message_String;
typedef struct _Edje_Message_Int              Edje_Message_Int;
typedef struct _Edje_Message_Float            Edje_Message_Float;
typedef struct _Edje_Message_String_Set       Edje_Message_String_Set;
typedef struct _Edje_Message_Int_Set          Edje_Message_Int_Set;
typedef struct _Edje_Message_Float_Set        Edje_Message_Float_Set;
typedef struct _Edje_Message_String_Int       Edje_Message_String_Int;
typedef struct _Edje_Message_String_Float     Edje_Message_String_Float;
typedef struct _Edje_Message_String_Int_Set   Edje_Message_String_Int_Set;
typedef struct _Edje_Message_String_Float_Set Edje_Message_String_Float_Set;

struct _Edje_Message_String
{
   char *str; /**< The message's string pointer */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING messages. The string in it is automatically freed be Edje if passed to you by Edje */

struct _Edje_Message_Int
{
   int val; /**< The message's value */
}; /**< Structure passed as value on #EDJE_MESSAGE_INT messages */

struct _Edje_Message_Float
{
   double val; /**< The message's value */
}; /**< Structure passed as value on #EDJE_MESSAGE_FLOAT messages */

struct _Edje_Message_String_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   char *str[1]; /**< The message's @b array of string pointers */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_SET messages. The array in it is automatically freed be Edje if passed to you by Edje */

struct _Edje_Message_Int_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   int val[1]; /**< The message's @b array of integers */
}; /**< Structure passed as value on #EDJE_MESSAGE_INT_SET messages. The array in it is automatically freed be Edje if passed to you by Edje */

struct _Edje_Message_Float_Set
{
   int count; /**< The size of the message's array (may be greater than 1) */
   double val[1]; /**< The message's @b array of floats */
}; /**< Structure passed as value on #EDJE_MESSAGE_FLOAT_SET messages. The array in it is automatically freed be Edje if passed to you by Edje */

struct _Edje_Message_String_Int
{
   char *str; /**< The message's string value */
   int val; /**< The message's integer value */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_INT messages. The string in it is automatically freed be Edje if passed to you by Edje */

struct _Edje_Message_String_Float
{
   char *str; /**< The message's string value */
   double val; /**< The message's float value */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_FLOAT messages. The string in it is automatically freed be Edje if passed to you by Edje */

struct _Edje_Message_String_Int_Set
{
   char *str; /**< The message's string value */
   int count; /**< The size of the message's array (may be greater than 1) */
   int val[1]; /**< The message's @b array of integers */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_INT_SET messages. The array and string in it are automatically freed be Edje if passed to you by Edje */

struct _Edje_Message_String_Float_Set
{
   char *str; /**< The message's string value */
   int count; /**< The size of the message's array (may be greater than 1) */
   double val[1]; /**< The message's @b array of floats */
}; /**< Structure passed as value on #EDJE_MESSAGE_STRING_FLOAT_SET messages. The array and string in it are automatically freed be Edje if passed to you by Edje */

typedef void         (*Edje_Message_Handler_Cb) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg); /**< Edje message handler callback functions's prototype definition. @c data will have the auxiliary data pointer set at the time the callback registration. @c obj will be a pointer the Edje object where the message comes from. @c type will identify the type of the given message and @c msg will be a pointer the message's contents, de facto, which depend on @c type. */

/**
 * @brief Send an (Edje) message to a given Edje object
 *
 * @param obj A handle to an Edje object
 * @param type The type of message to send to @p obj
 * @param id A identification number for the message to be sent
 * @param msg The message's body, a struct depending on @p type
 *
 * This function sends an Edje message to @p obj and to all of its
 * child objects, if it has any (swallowed objects are one kind of
 * child object). @p type and @p msg @b must be matched accordingly,
 * as documented in #Edje_Message_Type.
 *
 * The @p id argument as a form of code and theme defining a common
 * interface on message communication. One should define the same IDs
 * on both code and EDC declaration (see @ref edcref "the syntax" for
 * EDC files), to individualize messages (binding them to a given
 * context).
 *
 * The function to handle messages arriving @b from @b obj is set with
 * edje_object_message_handler_set().
 */
EAPI void         edje_object_message_send                (Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

/**
 * @brief Set an Edje message handler function for a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param func The function to handle messages @b coming from @p obj
 * @param data Auxiliary data to be passed to @p func
 *
 * For scriptable programs on an Edje object's defining EDC file which
 * send messages with the @c send_message() primitive, one can attach
 * <b>handler functions</b>, to be called in the code which creates
 * that object (see @ref edcref "the syntax" for EDC files).
 *
 * This function associates a message handler function and the
 * attached data pointer to the object @p obj.
 *
 * @see edje_object_message_send()
 */
EAPI void         edje_object_message_handler_set         (Evas_Object *obj, Edje_Message_Handler_Cb func, void *data);

/**
 * @brief Process an object's message queue.
 *
 * @param obj A handle to an Edje object.
 *
 * This function goes through the object message queue processing the
 * pending messages for @b this specific Edje object. Normally they'd
 * be processed only at idle time.
 *
 */
EAPI void         edje_object_message_signal_process      (Evas_Object *obj);


/**
 * @brief Process all queued up edje messages.
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
 * @ingroup Edje_Object_Group
 *
 * @{
 */

/* perspective info for maps inside edje objects */
typedef struct _Edje_Perspective Edje_Perspective;

/**
 * Creates a new perspective in the given canvas.
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
 * Delete the given perspective object.
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
 * Setup the transform for this perspective object.
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
 * Make this perspective object be global for its canvas.
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
 * Get whether the given perspective object is global or not.
 *
 * @param ps The given perspective object.
 * @return @c EINA_TRUE if this perspective object is global, @c EINA_FALSE
 * otherwise.
 *
 * @see edje_perspective_global_set()
 */
EAPI Eina_Bool               edje_perspective_global_get     (const Edje_Perspective *ps);
/**
 * Get the global perspective object set for this canvas.
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
 * Set the given perspective object on this Edje object.
 *
 * @param obj The Edje object on the perspective will be set.
 * @param ps The perspective object that will be used.
 *
 * Make the given perspective object be the default perspective for this Edje
 * object.
 *
 * There can be only one perspective object per Edje object, and if a
 * previous one was set, it will be removed and the new perspective object
 * will be used.
 *
 * An Edje perspective will only affect a part if it doesn't point to another
 * part to be used as perspective.
 *
 * @see edje_object_perspective_new()
 * @see edje_object_perspective_get()
 * @see edje_perspective_set()
 */
EAPI void                    edje_object_perspective_set     (Evas_Object *obj, Edje_Perspective *ps);
/**
 * Get the current perspective used on this Edje object.
 *
 * @param obj the given Edje object.
 * @return The perspective object being used on this Edje object. Or @c NULL
 * if there was none, and on errors.
 *
 * @see edje_object_perspective_set()
 */
EAPI const Edje_Perspective *edje_object_perspective_get     (const Evas_Object *obj);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
