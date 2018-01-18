#ifndef _ECORE_GETOPT_H
#define _ECORE_GETOPT_H

#include <stdio.h>
#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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

/**
 * @ingroup Ecore
 * @defgroup Ecore_Getopt_Group Ecore Getopt
 *
 * This group contains powerful getopt replacement.
 *
 * This replacement handles both short (-X) or long options (--ABC)
 * options, with various actions supported, like storing one value and
 * already converting to required type, counting number of
 * occurrences, setting true or false values, show help, license,
 * copyright and even support user-defined callbacks.
 *
 * It is provided a set of C Pre Processor macros so definition is
 * straightforward.
 *
 * Values will be stored elsewhere indicated by an array of pointers
 * to values, it is given in separate to parser description so you can
 * use multiple values with the same parser.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef Ecore_Getopt_Action
 * @brief Enumeration for defining the actions to do when parsing command line
 * parameters.
 */
typedef enum {
   ECORE_GETOPT_ACTION_STORE, /**< Store a value */
   ECORE_GETOPT_ACTION_STORE_CONST, /**< Store a const */
   ECORE_GETOPT_ACTION_STORE_TRUE, /**< Store TRUE */
   ECORE_GETOPT_ACTION_STORE_FALSE, /**< Store FALSE */
   ECORE_GETOPT_ACTION_CHOICE, /**< Store a choice between several values */
   ECORE_GETOPT_ACTION_APPEND, /**< Allocate and store a new value of type Ecore_Getopt_Type */
   ECORE_GETOPT_ACTION_COUNT, /**< Store a count number */
   ECORE_GETOPT_ACTION_CALLBACK, /**< Call a callback */
   ECORE_GETOPT_ACTION_HELP, /**< Show help text */
   ECORE_GETOPT_ACTION_VERSION, /**< Show version */
   ECORE_GETOPT_ACTION_COPYRIGHT, /**< Show copyright */
   ECORE_GETOPT_ACTION_LICENSE, /**< Show license */
   ECORE_GETOPT_ACTION_BREAK, /**< Stop parsing options */
   ECORE_GETOPT_ACTION_CATEGORY
} Ecore_Getopt_Action;

/**
 * @typedef Ecore_Getopt_Type
 * @brief Enumeration for defining the type of the values to store when using
 * append action.
 */
typedef enum {
   ECORE_GETOPT_TYPE_STR, /**< Value of type string */
   ECORE_GETOPT_TYPE_BOOL, /**< Value of type boolean */
   ECORE_GETOPT_TYPE_SHORT, /**< Value of type short */
   ECORE_GETOPT_TYPE_INT, /**< Value of type int */
   ECORE_GETOPT_TYPE_LONG, /**< Value of type long */
   ECORE_GETOPT_TYPE_USHORT, /**< Value of type unsigned short */
   ECORE_GETOPT_TYPE_UINT, /**< Value of type unsigned int */
   ECORE_GETOPT_TYPE_ULONG, /**< Value of type unsigned long */
   ECORE_GETOPT_TYPE_DOUBLE /**< Value of type double */
} Ecore_Getopt_Type;

/**
 * @typedef Ecore_Getopt_Desc_Arg_Requirement
 * @brief Enumeration for defining if the command line options require an
 * argument.
 */
typedef enum {
   ECORE_GETOPT_DESC_ARG_REQUIREMENT_NO = 0, /**< Argument is not required */
   ECORE_GETOPT_DESC_ARG_REQUIREMENT_YES = 1, /**< Argument is required */
   ECORE_GETOPT_DESC_ARG_REQUIREMENT_OPTIONAL = 3 /**< Argument is optional */
} Ecore_Getopt_Desc_Arg_Requirement;

typedef struct _Ecore_Getopt_Desc_Store    Ecore_Getopt_Desc_Store;
typedef struct _Ecore_Getopt_Desc_Callback Ecore_Getopt_Desc_Callback;

#ifndef _ECORE_GETOPT_PREDEF
typedef struct _Ecore_Getopt Ecore_Getopt;
#define _ECORE_GETOPT_PREDEF 1
#endif
#ifndef _ECORE_GETOPT_DESC_PREDEF
typedef struct _Ecore_Getopt_Desc Ecore_Getopt_Desc;
#define _ECORE_GETOPT_DESC_PREDEF 1
#endif
#ifndef _ECORE_GETOPT_VALUE_PREDEF
typedef union _Ecore_Getopt_Value Ecore_Getopt_Value;
#define _ECORE_GETOPT_VALUE_PREDEF 1
#endif

/**
 * @union _Ecore_Getopt_Value
 * @brief Union listing the types of parameters that can take Getopt values.
 */
union _Ecore_Getopt_Value
{
   char          **strp; /**< String pointer */
   unsigned char  *boolp; /**< Boolean pointer */
   short          *shortp; /**< Short pointer */
   int            *intp; /**< Int pointer */
   long           *longp; /**< Long pointer */
   unsigned short *ushortp; /**< Unsigned short pointer */
   unsigned int   *uintp; /**< Unsigned int pointer */
   unsigned long  *ulongp; /**< Unsigned long pointer */
   double         *doublep; /**< Double pointer */
   Eina_List     **listp; /**< List pointer */
   void          **ptrp; /**< Void pointer */
};

/**
 * @struct _Ecore_Getopt_Desc_Store
 * @brief Structure used when action is ECORE_GETOPT_ACTION_STORE. It contains
 * information about the value to store.
 */
struct _Ecore_Getopt_Desc_Store
{
   Ecore_Getopt_Type                 type; /**< type of data being handled */
   Ecore_Getopt_Desc_Arg_Requirement arg_req; /**< option argument requirement */
   union
   {
      const char    *strv; /**< String value */
      Eina_Bool      boolv; /**< Boolean value */
      short          shortv; /**< Short value */
      int            intv; /**< Int value */
      long           longv; /**< Long value */
      unsigned short ushortv; /**< Unsigned short value */
      unsigned int   uintv; /**< Unsigned int value */
      unsigned long  ulongv; /**< Unsigned long value */
      double         doublev; /**< Double value */
   } def; /**< value of data being handled */
};

/**
 * @struct _Ecore_Getopt_Desc_Callback
 * @brief Structure used when action is ECORE_GETOPT_ACTION_CALLBACK. It
 * contains information about the callback to call.
 */
struct _Ecore_Getopt_Desc_Callback
{
   Eina_Bool                         (*func)(const Ecore_Getopt *parser,
                                             const Ecore_Getopt_Desc *desc,
                                             const char *str,
                                             void *data,
                                             Ecore_Getopt_Value *storage); /**< function to call as a callback */
   const void                       *data; /**< data to pass to the callback */
   Ecore_Getopt_Desc_Arg_Requirement arg_req; /**< option argument requirement */
   const char                       *def;
};

/**
 * @struct _Ecore_Getopt_Desc
 * @brief Structure that describe an option of the command line.
 */
struct _Ecore_Getopt_Desc
{
   char                shortname; /**< used with a single dash */
   const char         *longname; /**< used with double dashes */
   const char         *help; /**< used by --help/ecore_getopt_help() */
   const char         *metavar; /**< used by ecore_getopt_help() with nargs > 0 */

   Ecore_Getopt_Action action;   /**< define how to handle it */
   union
   {
      const Ecore_Getopt_Desc_Store    store;
      const void                      *store_const;
      const char *const               *choices; /* NULL terminated. */
      const Ecore_Getopt_Type          append_type;
      const Ecore_Getopt_Desc_Callback callback;
      const void                      *dummy;
   } action_param; /**< Action parameter */
};

/**
 * @struct _Ecore_Getopt
 * @brief Structure that contains information on all command line options.
 */
struct _Ecore_Getopt
{
   const char             *prog; /**< to be used when ecore_app_args_get() fails */
   const char             *usage; /**< usage example, %%prog is replaced by @ref prog */
   const char             *version; /**< if exists, --version will work */
   const char             *copyright; /**< if exists, --copyright will work */
   const char             *license; /**< if exists, --license will work */
   const char             *description; /**< long description, possible multiline */
   Eina_Bool               strict : 1; /**< fail on errors */
   const Ecore_Getopt_Desc descs[];   /**< A table that contains the description of all the other options (NULL terminated).*/

};

/**
 * @brief Definition for macro that helps to fill the Ecore_Getopt_Desc table.
 */
#define ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar, type, arg_requirement, default_value) \
  {shortname, longname, help, metavar, ECORE_GETOPT_ACTION_STORE,                                         \
   {.store = {type, arg_requirement, default_value}}}

/**
 * @brief Definition for macro that fills an option in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param type The option value type.
 */
#define ECORE_GETOPT_STORE(shortname, longname, help, type)      \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, NULL, type, \
                          ECORE_GETOPT_DESC_ARG_REQUIREMENT_YES, {})
/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type string.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_STR(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_STR)

/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type boolean.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_BOOL(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_BOOL)

/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type short.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_SHORT(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_SHORT)

/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type int.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_INT(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_INT)

/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type long.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_LONG(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_LONG)

/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type ushort.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_USHORT(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_USHORT)

/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type uint.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_UINT(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_UINT)

/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type ulong.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_ULONG(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_ULONG)

/**
 * @brief Definition for macro that fill Ecore_Getopt_Desc table with an option of type double.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_DOUBLE(shortname, longname, help) \
  ECORE_GETOPT_STORE(shortname, longname, help, ECORE_GETOPT_TYPE_DOUBLE)

/**
 * Definition for macro that helps to fill the Ecore_Getopt_Desc table with a metavar after
 * the description of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, type) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar, type,          \
                          ECORE_GETOPT_DESC_ARG_REQUIREMENT_YES, {})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type string and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_STR(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_STR)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type boolean and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_BOOL(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_BOOL)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type short and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_SHORT(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_SHORT)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type int and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_INT(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_INT)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type long and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_LONG(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_LONG)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type unsigned short and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_USHORT(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_USHORT)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type unsigned int and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_UINT(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_UINT)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type unsigned long and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_ULONG(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_ULONG)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type double and metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 */
#define ECORE_GETOPT_STORE_METAVAR_DOUBLE(shortname, longname, help, metavar) \
  ECORE_GETOPT_STORE_METAVAR(shortname, longname, help, metavar, ECORE_GETOPT_TYPE_DOUBLE)

/**
 * Definition for macro that helps to fill the Ecore_Getopt_Desc table with a default value.
 */
#define ECORE_GETOPT_STORE_DEF(shortname, longname, help, type, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, NULL, type,               \
                          ECORE_GETOPT_DESC_ARG_REQUIREMENT_OPTIONAL,          \
                          default_value)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type string and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_STR(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                          \
                         ECORE_GETOPT_TYPE_STR,                              \
                         {.strv = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type boolean and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_BOOL(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                           \
                         ECORE_GETOPT_TYPE_BOOL,                              \
                         {.boolv = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type short and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_SHORT(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                            \
                         ECORE_GETOPT_TYPE_SHORT,                              \
                         {.shortv = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type int and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_INT(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                          \
                         ECORE_GETOPT_TYPE_INT,                              \
                         {.intv = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type long and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_LONG(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                           \
                         ECORE_GETOPT_TYPE_LONG,                              \
                         {.longv = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type unsigned short and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_USHORT(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                             \
                         ECORE_GETOPT_TYPE_USHORT,                              \
                         {.ushortv = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type unsigned int and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_UINT(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                           \
                         ECORE_GETOPT_TYPE_UINT,                              \
                         {.uintv = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type unsigned long and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_ULONG(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                            \
                         ECORE_GETOPT_TYPE_ULONG,                              \
                         {.ulongv = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an option of type double and default value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_DEF_DOUBLE(shortname, longname, help, default_value) \
  ECORE_GETOPT_STORE_DEF(shortname, longname, help,                             \
                         ECORE_GETOPT_TYPE_DOUBLE,                              \
                         {.doublev = default_value})

/**
 * @brief Definition for filling full string type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_STR(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                           \
                          ECORE_GETOPT_TYPE_STR,                                                        \
                          arg_requirement,                                                              \
                          {.strv = default_value})

/**
 * @brief Definition for filling full boolean type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_BOOL(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                            \
                          ECORE_GETOPT_TYPE_BOOL,                                                        \
                          arg_requirement,                                                               \
                          {.boolv = default_value})

/**
 * @brief Definition for filling full short type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_SHORT(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                             \
                          ECORE_GETOPT_TYPE_SHORT,                                                        \
                          arg_requirement,                                                                \
                          {.shortv = default_value})

/**
 * @brief Definition for filling full int type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_INT(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                           \
                          ECORE_GETOPT_TYPE_INT,                                                        \
                          arg_requirement,                                                              \
                          {.intv = default_value})

/**
 * @brief Definition for filling full long type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_LONG(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                            \
                          ECORE_GETOPT_TYPE_LONG,                                                        \
                          arg_requirement,                                                               \
                          {.longv = default_value})

/**
 * @brief Definition for filling full unsigned short type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_USHORT(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                              \
                          ECORE_GETOPT_TYPE_USHORT,                                                        \
                          arg_requirement,                                                                 \
                          {.ushortv = default_value})

/**
 * @brief Definition for filling full unsigned int type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_UINT(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                            \
                          ECORE_GETOPT_TYPE_UINT,                                                        \
                          arg_requirement,                                                               \
                          {.uintv = default_value})

/**
 * @brief Definition for filling full unsigned long type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_ULONG(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                             \
                          ECORE_GETOPT_TYPE_ULONG,                                                        \
                          arg_requirement,                                                                \
                          {.ulongv = default_value})

/**
 * @brief Definition for filling full double type option description in Ecore_Getopt_Desc table.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param arg_requirement The option argument requirements.
 * @param default_value The default value for the parameter of the option.
 */
#define ECORE_GETOPT_STORE_FULL_DOUBLE(shortname, longname, help, metavar, arg_requirement, default_value) \
  ECORE_GETOPT_STORE_FULL(shortname, longname, help, metavar,                                              \
                          ECORE_GETOPT_TYPE_DOUBLE,                                                        \
                          arg_requirement,                                                                 \
                          {.doublev = default_value})

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a constant value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param value The constant value to store.
 */
#define ECORE_GETOPT_STORE_CONST(shortname, longname, help, value)   \
  {shortname, longname, help, NULL, ECORE_GETOPT_ACTION_STORE_CONST, \
   {.store_const = value}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a true boolean value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_TRUE(shortname, longname, help)          \
  {shortname, longname, help, NULL, ECORE_GETOPT_ACTION_STORE_TRUE, \
   {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a false boolean value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_STORE_FALSE(shortname, longname, help)          \
  {shortname, longname, help, NULL, ECORE_GETOPT_ACTION_STORE_FALSE, \
   {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a true boolean value.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param choices_array An string array of different choices.
 */
#define ECORE_GETOPT_CHOICE(shortname, longname, help, choices_array) \
  {shortname, longname, help, NULL, ECORE_GETOPT_ACTION_CHOICE,       \
   {.choices = choices_array}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a choice.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param choices_array An string array of different choices.
 */
#define ECORE_GETOPT_CHOICE_METAVAR(shortname, longname, help, metavar, choices_array) \
  {shortname, longname, help, metavar, ECORE_GETOPT_ACTION_CHOICE,                     \
   {.choices = choices_array}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an append action.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param sub_type The type of the new value to store.
 */
#define ECORE_GETOPT_APPEND(shortname, longname, help, sub_type) \
  {shortname, longname, help, NULL, ECORE_GETOPT_ACTION_APPEND,  \
   {.append_type = sub_type}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an append action and a metavar.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param type The type of the new value to store.
 */
#define ECORE_GETOPT_APPEND_METAVAR(shortname, longname, help, metavar, type) \
  {shortname, longname, help, metavar, ECORE_GETOPT_ACTION_APPEND,            \
   {.append_type = type}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an count action.
 *
 * This will store the number of time the option has been passed to the command
 * line.
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_COUNT(shortname, longname, help)          \
  {shortname, longname, help, NULL, ECORE_GETOPT_ACTION_COUNT, \
   {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an callback action and argument requirements.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param callback_func The callback function to call.
 * @param callback_data The data to pass to the callback.
 * @param argument_requirement The required arguments to this option.
 * @param default_value The default values for these arguments.
 */
#define ECORE_GETOPT_CALLBACK_FULL(shortname, longname, help, metavar, callback_func, callback_data, argument_requirement, default_value) \
  {shortname, longname, help, metavar, ECORE_GETOPT_ACTION_CALLBACK,                                                                      \
   {.callback = {callback_func, callback_data,                                                                                            \
                 argument_requirement, default_value}}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an callback action and no arguments.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param callback_func The callback function to call.
 * @param callback_data The data to pass to the callback.
 */
#define ECORE_GETOPT_CALLBACK_NOARGS(shortname, longname, help, callback_func, callback_data) \
  ECORE_GETOPT_CALLBACK_FULL(shortname, longname, help, NULL,                                 \
                             callback_func, callback_data,                                    \
                             ECORE_GETOPT_DESC_ARG_REQUIREMENT_NO,                            \
                             NULL)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with an callback action.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 * @param metavar The metavar message concerning the parameter of the option.
 * @param callback_func The callback function to call.
 * @param callback_data The data to pass to the callback.
 */
#define ECORE_GETOPT_CALLBACK_ARGS(shortname, longname, help, metavar, callback_func, callback_data) \
  ECORE_GETOPT_CALLBACK_FULL(shortname, longname, help, metavar,                                     \
                             callback_func, callback_data,                                           \
                             ECORE_GETOPT_DESC_ARG_REQUIREMENT_YES,                                  \
                             NULL)

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a help action.
 *
 * @param shortname The help option short name.
 * @param longname The help option long name.
 */
#define ECORE_GETOPT_HELP(shortname, longname)            \
  {shortname, longname, "show this message.", "CATEGORY", \
   ECORE_GETOPT_ACTION_HELP,                              \
   {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a version action.
 *
 * @param shortname The version option short name.
 * @param longname The version option long name.
 */
#define ECORE_GETOPT_VERSION(shortname, longname)      \
  {shortname, longname, "show program version.", NULL, \
   ECORE_GETOPT_ACTION_VERSION,                        \
   {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a copyright action.
 *
 * @param shortname The copyright option short name.
 * @param longname The copyright option long name.
 */
#define ECORE_GETOPT_COPYRIGHT(shortname, longname) \
  {shortname, longname, "show copyright.", NULL,    \
   ECORE_GETOPT_ACTION_COPYRIGHT,                   \
   {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a license action.
 *
 * @param shortname The license option short name.
 * @param longname The license option long name.
 */
#define ECORE_GETOPT_LICENSE(shortname, longname) \
  {shortname, longname, "show license.", NULL,    \
   ECORE_GETOPT_ACTION_LICENSE,                   \
   {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a break action.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 */
#define ECORE_GETOPT_BREAK(shortname, longname) \
  {shortname, longname, "stop parsing options.", NULL,    \
   ECORE_GETOPT_ACTION_BREAK,                   \
   {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a break action with help message.
 *
 * @param shortname The option short name.
 * @param longname The option long name.
 * @param help The help message concerning this option.
 */
#define ECORE_GETOPT_BREAK_STR(shortname, longname, help) \
  {shortname, longname, help, NULL,    \
   ECORE_GETOPT_ACTION_BREAK,                   \
   {.dummy = NULL}}

#define ECORE_GETOPT_CATEGORY(name, help) \
  {0, name, help, NULL, ECORE_GETOPT_ACTION_CATEGORY, {.dummy = NULL}}

/**
 * @brief Definition for filling Ecore_Getopt_Desc table with a sentinel to indicate the end of descriptions.
 *
 */
#define ECORE_GETOPT_SENTINEL {0, NULL, NULL, NULL, 0, {.dummy = NULL}}

/**
 * @brief Definition for options that store a single value in a variable of type string.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_STR(val)      {.strp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type boolean.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_BOOL(val)     {.boolp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type short.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_SHORT(val)    {.shortp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type int.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_INT(val)      {.intp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type long.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_LONG(val)     {.longp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type unsigned short.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_USHORT(val)   {.ushortp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type unsigned int.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_UINT(val)     {.uintp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type unsigned long.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_ULONG(val)    {.ulongp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type double.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_DOUBLE(val)   {.doublep = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type pointer.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_PTR(val)      {.ptrp = &(val)}

/**
 * @brief Definition for options that store a single value in a variable of type pointer casted.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_PTR_CAST(val) {.ptrp = (void **)&(val)}

/**
 * @brief Definition for options that store a single value in a variable of type list.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_LIST(val)     {.listp = &(val)}

/**
 * @brief Definition for options that store a NULL value.
 *
 * @param val The value to store.
 */
#define ECORE_GETOPT_VALUE_NONE {.ptrp = NULL}

/**
 * Shows nicely formatted help message for the given parser.
 *
 * @param fp The file the message will be printed on.
 * @param info The structure containing information about command line options.
 *
 * @see ecore_getopt_help_category()
 */
EAPI void       ecore_getopt_help(FILE *fp, const Ecore_Getopt *info);

/**
 * Shows help for a single category (along with program usage and description).
 *
 * @param fp The file the message will be printed on.
 * @param info The structure containing information about command line options.
 * @param category The category to print.
 *
 * @return @c EINA_TRUE when the category exists, @c EINA_FALSE otherwise.
 *
 * @see ecore_getopt_help()
 */
EAPI Eina_Bool  ecore_getopt_help_category(FILE *fp, const Ecore_Getopt *info, const char *category);

/**
 * Checks parser for duplicate entries, print them out.
 *
 * @return @c EINA_TRUE if there are duplicates, @c EINA_FALSE otherwise.
 * @param parser The parser to be checked.
 */
EAPI Eina_Bool  ecore_getopt_parser_has_duplicates(const Ecore_Getopt *parser);

/**
 * Parses command line parameters.
 *
 * Walks the command line parameters and parse them based on @a parser
 * description, doing actions based on @c parser->descs->action, like
 * showing help text, license, copyright, storing values in values and
 * so on.
 *
 * It is expected that values is of the same size than @c parser->descs,
 * options that do not need a value it will be left untouched.
 *
 * All values are expected to be initialized before use. Options with
 * action @c ECORE_GETOPT_ACTION_STORE and non required arguments
 * (others than @c ECORE_GETOPT_DESC_ARG_REQUIREMENT_YES), are expected
 * to provide a value in @c def to be used.
 *
 * The following actions will store @c 1 on value as a boolean
 * (@c value->boolp) if it's not @c NULL to indicate these actions were
 * executed:
 *   - @c ECORE_GETOPT_ACTION_HELP
 *   - @c ECORE_GETOPT_ACTION_VERSION
 *   - @c ECORE_GETOPT_ACTION_COPYRIGHT
 *   - @c ECORE_GETOPT_ACTION_LICENSE
 *
 * Just @c ECORE_GETOPT_ACTION_APPEND will allocate memory and thus
 * need to be freed. For consistency between all of appended subtypes,
 * @c eina_list->data will contain an allocated memory with the value,
 * that is, for @c ECORE_GETOPT_TYPE_STR it will contain a copy of the
 * argument, @c ECORE_GETOPT_TYPE_INT a pointer to an allocated
 * integer and so on.
 *
 * If parser is in strict mode (see @c Ecore_Getopt->strict), then any
 * error will abort parsing and @c -1 is returned. Otherwise it will try
 * to continue as far as possible.
 *
 * This function may reorder @a argv elements.
 *
 * Translation of help strings (description), metavar, usage, license
 * and copyright may be translated, standard/global gettext() call
 * will be applied on them if ecore was compiled with such support.
 *
 * This function will @b not parse positional arguments! If these are
 * declared (metavar is defined with both shortname and longname being
 * empty), then you must call ecore_getopt_parse_positional() with the
 * last argument (@c start) being the result of this function. This is
 * done so you can have "quit options", those that once called you
 * want to exit without doing further parsing, as is the case with
 * help, license, copyright, version and eventually others you may
 * define.
 *
 * @param parser Description of how to work.
 * @param values Where to store values, it is assumed that this is a vector
 *        of the same size as @c parser->descs. Values should be previously
 *        initialized.
 * @param argc How many elements in @a argv. If not provided it will be
 *        retrieved with ecore_app_args_get().
 * @param argv Command line parameters.
 *
 * @return Index of first non-option parameter or @c -1 on error.
 *
 * @see ecore_getopt_parse_positional()
 */
EAPI int        ecore_getopt_parse(const Ecore_Getopt *parser, Ecore_Getopt_Value *values, int argc, char **argv);

/**
 * Parses command line positional parameters.
 *
 * Walks the command line positional parameters (those that do not
 * start with "-" or "--") and parse them based on @a parser
 * description, doing actions based on @c parser->descs->action, like
 * storing values of some type.
 *
 * It is expected that @a values is of the same size than @c
 * parser->descs, same as with ecore_getopt_parse().
 *
 * All values are expected to be initialized before use.
 *
 * Unlike the ecore_getopt_parse(), only the following options are
 * supported:
 *  - @c ECORE_GETOPT_ACTION_STORE
 *  - @c ECORE_GETOPT_ACTION_CHOICE
 *  - @c ECORE_GETOPT_ACTION_APPEND
 *  - @c ECORE_GETOPT_ACTION_CALLBACK
 *
 * There is a special case for @c ECORE_GETOPT_ACTION_APPEND as it
 * will consume all remaining elements. It is also special in the
 * sense that it will allocate memory and thus need to be freed. For
 * consistency between all of appended subtypes, @c eina_list->data
 * will contain an allocated memory with the value, that is, for @c
 * ECORE_GETOPT_TYPE_STR it will contain a copy of the argument, @c
 * ECORE_GETOPT_TYPE_INT a pointer to an allocated integer and so on.
 *
 * If parser is in strict mode (see @c Ecore_Getopt->strict), then any
 * error will abort parsing and @c -1 is returned. Otherwise it will try
 * to continue as far as possible.
 *
 * Translation of help strings (description) and metavar may be done,
 * standard/global gettext() call will be applied on them if ecore was
 * compiled with such support.
 *
 * @param parser Description of how to work.
 * @param values Where to store values, it is assumed that this is a vector
 *        of the same size as @c parser->descs. Values should be previously
 *        initialized.
 * @param argc How many elements in @a argv. If not provided it will be
 *        retrieved with ecore_app_args_get().
 * @param argv Command line parameters.
 * @param start The initial position argument to look at, usually the
 *        return of ecore_getopt_parse(). If less than 1, will try to
 *        find it automatically.
 *
 * @return Index of first non-option parameter or @c -1 on error. If the
 *         last positional argument is of action @c
 *         ECORE_GETOPT_ACTION_APPEND then it will be the same as @a argc.
 */
EAPI int        ecore_getopt_parse_positional(const Ecore_Getopt *parser, Ecore_Getopt_Value *values, int argc, char **argv, int start);


/**
 * Utilities to free list and nodes allocated by @a ECORE_GETOPT_ACTION_APPEND.
 *
 * @param list Pointer to list to be freed.
 * @return always @c NULL, so you can easily make your list head @c NULL.
 */
EAPI Eina_List *ecore_getopt_list_free(Eina_List *list);

/**
 * Helper ecore_getopt callback to parse geometry (x:y:w:h).
 *
 * @param parser This parameter isn't in use.
 * @param desc This parameter isn't in use.
 * @param str Geometry value
 * @param data This parameter isn't in use.
 * @param storage Must be a pointer to @c Eina_Rectangle and will be used to
 * store the four values passed in the given string.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on incorrect geometry value.
 *
 * This is a helper function to be used with ECORE_GETOPT_CALLBACK_*().
 * 
 * @c callback_data value is ignored, you can safely use @c NULL.
 */
EAPI Eina_Bool  ecore_getopt_callback_geometry_parse(const Ecore_Getopt *parser, const Ecore_Getopt_Desc *desc, const char *str, void *data, Ecore_Getopt_Value *storage);

/**
 * Helper ecore_getopt callback to parse geometry size (WxH).
 *
 * @param parser This parameter isn't in use.
 * @param desc This parameter isn't in use.
 * @param str size value
 * @param data This parameter isn't in use.
 * @param storage Must be a pointer to @c Eina_Rectangle and will be used to
 * store the two values passed in the given string and @c 0 in the x and y
 * fields.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on incorrect size value.
 *
 * @c callback_data value is ignored, you can safely use @c NULL.
 */
EAPI Eina_Bool  ecore_getopt_callback_size_parse(const Ecore_Getopt *parser, const Ecore_Getopt_Desc *desc, const char *str, void *data, Ecore_Getopt_Value *storage);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* _ECORE_GETOPT_H */
