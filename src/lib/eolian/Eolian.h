#ifndef EOLIAN_H
#define EOLIAN_H

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

#ifdef __cplusplus
extern "C" {
#endif

#include <Eina.h>

/**
 * @page eolian_main Eolian (BETA)
 *
 * @date 2014 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref eolian_main_intro
 * @li @ref eolian_main_compiling
 * @li @ref eolian_main_next_steps
 *
 * @section eolian_main_intro Introduction
 *
 * The Eolian EO file parser and code generator.

 * @section eolian_main_compiling How to compile
 *
 * Eolian is a library your application links to. The procedure for this is
 * very simple. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script outputs. For
 * example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags eolian`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs eolian`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section eolian_main_next_steps Next Steps
 *
 * After you understood what Eolian is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Eolian
 *
 * @addtogroup Eolian
 * @{
 */

#ifdef EFL_BETA_API_SUPPORT

/* State information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian Eolian;

/* Class type used to extract information on classes
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Class Eolian_Class;

/* Function Id used to extract information on class functions
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Function Eolian_Function;

/* Part information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Part Eolian_Part;

/* Parameter/return type.
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Type Eolian_Type;

/* Type declaration.
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Typedecl Eolian_Typedecl;

/* Class function parameter information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Function_Parameter Eolian_Function_Parameter;

/* Class implement information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Implement Eolian_Implement;

/* Class constructor information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Constructor Eolian_Constructor;

/* Event information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Event Eolian_Event;

/* Expression information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Expression Eolian_Expression;

/* Variable information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Variable Eolian_Variable;

/* Struct field information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Struct_Type_Field Eolian_Struct_Type_Field;

/* Enum field information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Enum_Type_Field Eolian_Enum_Type_Field;

/* Declaration information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Declaration Eolian_Declaration;

/* Documentation information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Documentation Eolian_Documentation;

/* Unit information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Unit Eolian_Unit;

typedef enum
{
   EOLIAN_UNRESOLVED = 0,
   EOLIAN_PROPERTY,
   EOLIAN_PROP_SET,
   EOLIAN_PROP_GET,
   EOLIAN_METHOD,
   EOLIAN_FUNCTION_POINTER
} Eolian_Function_Type;

typedef enum
{
   EOLIAN_UNKNOWN_PARAM = 0,
   EOLIAN_IN_PARAM,
   EOLIAN_OUT_PARAM,
   EOLIAN_INOUT_PARAM
} Eolian_Parameter_Dir;

typedef enum
{
   EOLIAN_CLASS_UNKNOWN_TYPE = 0,
   EOLIAN_CLASS_REGULAR,
   EOLIAN_CLASS_ABSTRACT,
   EOLIAN_CLASS_MIXIN,
   EOLIAN_CLASS_INTERFACE
} Eolian_Class_Type;

typedef enum
{
   EOLIAN_SCOPE_UNKNOWN = 0,
   EOLIAN_SCOPE_PUBLIC,
   EOLIAN_SCOPE_PRIVATE,
   EOLIAN_SCOPE_PROTECTED
} Eolian_Object_Scope;

typedef enum
{
   EOLIAN_TYPEDECL_UNKNOWN = 0,
   EOLIAN_TYPEDECL_STRUCT,
   EOLIAN_TYPEDECL_STRUCT_OPAQUE,
   EOLIAN_TYPEDECL_ENUM,
   EOLIAN_TYPEDECL_ALIAS,
   EOLIAN_TYPEDECL_FUNCTION_POINTER
} Eolian_Typedecl_Type;

typedef enum
{
   EOLIAN_TYPE_UNKNOWN_TYPE = 0,
   EOLIAN_TYPE_VOID,
   EOLIAN_TYPE_REGULAR,
   EOLIAN_TYPE_CLASS,
   EOLIAN_TYPE_UNDEFINED
} Eolian_Type_Type;

typedef enum
{
   EOLIAN_TYPE_BUILTIN_INVALID = 0,

   EOLIAN_TYPE_BUILTIN_BYTE,
   EOLIAN_TYPE_BUILTIN_UBYTE,
   EOLIAN_TYPE_BUILTIN_CHAR,
   EOLIAN_TYPE_BUILTIN_SHORT,
   EOLIAN_TYPE_BUILTIN_USHORT,
   EOLIAN_TYPE_BUILTIN_INT,
   EOLIAN_TYPE_BUILTIN_UINT,
   EOLIAN_TYPE_BUILTIN_LONG,
   EOLIAN_TYPE_BUILTIN_ULONG,
   EOLIAN_TYPE_BUILTIN_LLONG,
   EOLIAN_TYPE_BUILTIN_ULLONG,

   EOLIAN_TYPE_BUILTIN_INT8,
   EOLIAN_TYPE_BUILTIN_UINT8,
   EOLIAN_TYPE_BUILTIN_INT16,
   EOLIAN_TYPE_BUILTIN_UINT16,
   EOLIAN_TYPE_BUILTIN_INT32,
   EOLIAN_TYPE_BUILTIN_UINT32,
   EOLIAN_TYPE_BUILTIN_INT64,
   EOLIAN_TYPE_BUILTIN_UINT64,
   EOLIAN_TYPE_BUILTIN_INT128,
   EOLIAN_TYPE_BUILTIN_UINT128,

   EOLIAN_TYPE_BUILTIN_SIZE,
   EOLIAN_TYPE_BUILTIN_SSIZE,
   EOLIAN_TYPE_BUILTIN_INTPTR,
   EOLIAN_TYPE_BUILTIN_UINTPTR,
   EOLIAN_TYPE_BUILTIN_PTRDIFF,

   EOLIAN_TYPE_BUILTIN_TIME,

   EOLIAN_TYPE_BUILTIN_FLOAT,
   EOLIAN_TYPE_BUILTIN_DOUBLE,

   EOLIAN_TYPE_BUILTIN_BOOL,

   EOLIAN_TYPE_BUILTIN_VOID,

   EOLIAN_TYPE_BUILTIN_ACCESSOR,
   EOLIAN_TYPE_BUILTIN_ARRAY,
   EOLIAN_TYPE_BUILTIN_ITERATOR,
   EOLIAN_TYPE_BUILTIN_HASH,
   EOLIAN_TYPE_BUILTIN_LIST,
   EOLIAN_TYPE_BUILTIN_INARRAY,
   EOLIAN_TYPE_BUILTIN_INLIST,

   EOLIAN_TYPE_BUILTIN_FUTURE,

   EOLIAN_TYPE_BUILTIN_ANY_VALUE,
   EOLIAN_TYPE_BUILTIN_ANY_VALUE_PTR,

   EOLIAN_TYPE_BUILTIN_MSTRING,
   EOLIAN_TYPE_BUILTIN_STRING,
   EOLIAN_TYPE_BUILTIN_STRINGSHARE,

   EOLIAN_TYPE_BUILTIN_VOID_PTR,
   EOLIAN_TYPE_BUILTIN_FREE_CB
} Eolian_Type_Builtin_Type;

typedef enum
{
   EOLIAN_C_TYPE_DEFAULT = 0,
   EOLIAN_C_TYPE_PARAM,
   EOLIAN_C_TYPE_RETURN
} Eolian_C_Type_Type;

typedef enum
{
   EOLIAN_EXPR_UNKNOWN = 0,
   EOLIAN_EXPR_INT,
   EOLIAN_EXPR_UINT,
   EOLIAN_EXPR_LONG,
   EOLIAN_EXPR_ULONG,
   EOLIAN_EXPR_LLONG,
   EOLIAN_EXPR_ULLONG,
   EOLIAN_EXPR_FLOAT,
   EOLIAN_EXPR_DOUBLE,
   EOLIAN_EXPR_STRING,
   EOLIAN_EXPR_CHAR,
   EOLIAN_EXPR_NULL,
   EOLIAN_EXPR_BOOL,
   EOLIAN_EXPR_NAME,
   EOLIAN_EXPR_UNARY,
   EOLIAN_EXPR_BINARY
} Eolian_Expression_Type;

typedef enum
{
   EOLIAN_MASK_SINT   = 1 << 0,
   EOLIAN_MASK_UINT   = 1 << 1,
   EOLIAN_MASK_INT    = EOLIAN_MASK_SINT | EOLIAN_MASK_UINT,
   EOLIAN_MASK_FLOAT  = 1 << 2,
   EOLIAN_MASK_BOOL   = 1 << 3,
   EOLIAN_MASK_STRING = 1 << 4,
   EOLIAN_MASK_CHAR   = 1 << 5,
   EOLIAN_MASK_NULL   = 1 << 6,
   EOLIAN_MASK_SIGNED = EOLIAN_MASK_SINT   | EOLIAN_MASK_FLOAT,
   EOLIAN_MASK_NUMBER = EOLIAN_MASK_INT    | EOLIAN_MASK_FLOAT,
   EOLIAN_MASK_ALL    = EOLIAN_MASK_NUMBER | EOLIAN_MASK_BOOL
                      | EOLIAN_MASK_STRING | EOLIAN_MASK_CHAR
                      | EOLIAN_MASK_NULL
} Eolian_Expression_Mask;

typedef enum
{
   EOLIAN_VAR_UNKNOWN = 0,
   EOLIAN_VAR_CONSTANT,
   EOLIAN_VAR_GLOBAL
} Eolian_Variable_Type;

typedef union
{
   char               c;
   Eina_Bool          b;
   const    char     *s;
   signed   int       i;
   unsigned int       u;
   signed   long      l;
   unsigned long      ul;
   signed   long long ll;
   unsigned long long ull;
   float              f;
   double             d;
} Eolian_Value_Union;

typedef struct _Eolian_Value
{
   Eolian_Expression_Type type;
   Eolian_Value_Union value;
} Eolian_Value;

typedef enum
{
   EOLIAN_BINOP_INVALID = 0,

   EOLIAN_BINOP_ADD, /* + int, float */
   EOLIAN_BINOP_SUB, /* - int, float */
   EOLIAN_BINOP_MUL, /* * int, float */
   EOLIAN_BINOP_DIV, /* / int, float */
   EOLIAN_BINOP_MOD, /* % int */

   EOLIAN_BINOP_EQ, /* == all types */
   EOLIAN_BINOP_NQ, /* != all types */
   EOLIAN_BINOP_GT, /* >  int, float */
   EOLIAN_BINOP_LT, /* <  int, float */
   EOLIAN_BINOP_GE, /* >= int, float */
   EOLIAN_BINOP_LE, /* <= int, float */

   EOLIAN_BINOP_AND, /* && all types */
   EOLIAN_BINOP_OR,  /* || all types */

   EOLIAN_BINOP_BAND, /* &  int */
   EOLIAN_BINOP_BOR,  /* |  int */
   EOLIAN_BINOP_BXOR, /* ^  int */
   EOLIAN_BINOP_LSH,  /* << int */
   EOLIAN_BINOP_RSH   /* >> int */
} Eolian_Binary_Operator;

typedef enum
{
   EOLIAN_UNOP_INVALID = 0,

   EOLIAN_UNOP_UNM, /* - sint */
   EOLIAN_UNOP_UNP, /* + sint */

   EOLIAN_UNOP_NOT,  /* ! int, float, bool */
   EOLIAN_UNOP_BNOT, /* ~ int */
} Eolian_Unary_Operator;

typedef enum
{
   EOLIAN_DECL_UNKNOWN = 0,
   EOLIAN_DECL_CLASS,
   EOLIAN_DECL_ALIAS,
   EOLIAN_DECL_STRUCT,
   EOLIAN_DECL_ENUM,
   EOLIAN_DECL_VAR
} Eolian_Declaration_Type;

typedef enum
{
   EOLIAN_DOC_TOKEN_UNKNOWN = 0,
   EOLIAN_DOC_TOKEN_TEXT,
   EOLIAN_DOC_TOKEN_REF,
   EOLIAN_DOC_TOKEN_MARK_NOTE,
   EOLIAN_DOC_TOKEN_MARK_WARNING,
   EOLIAN_DOC_TOKEN_MARK_REMARK,
   EOLIAN_DOC_TOKEN_MARK_TODO,
   EOLIAN_DOC_TOKEN_MARKUP_MONOSPACE
} Eolian_Doc_Token_Type;

typedef enum
{
   EOLIAN_DOC_REF_INVALID = 0,
   EOLIAN_DOC_REF_CLASS,
   EOLIAN_DOC_REF_FUNC,
   EOLIAN_DOC_REF_EVENT,
   EOLIAN_DOC_REF_ALIAS,
   EOLIAN_DOC_REF_STRUCT,
   EOLIAN_DOC_REF_STRUCT_FIELD,
   EOLIAN_DOC_REF_ENUM,
   EOLIAN_DOC_REF_ENUM_FIELD,
   EOLIAN_DOC_REF_VAR
} Eolian_Doc_Ref_Type;

typedef struct _Eolian_Doc_Token
{
   Eolian_Doc_Token_Type type;
   const char *text, *text_end;
} Eolian_Doc_Token;

/*
 * @brief Init Eolian.
 *
 * @ingroup Eolian
 */
EAPI int eolian_init(void);

/*
 * @brief Shutdown Eolian.
 *
 * @ingroup Eolian
 */
EAPI int eolian_shutdown(void);

/*
 * @brief Create a new Eolian state.
 *
 * This creates a new Eolian state that consists of a "master unit" with
 * the same address (therefore, you can cast it to Eolian_Unit) plus extra
 * state information.
 *
 * You need to free this with eolian_free once you're done.
 *
 * @return A new state (or NULL on failure).
 *
 * @ingroup Eolian
 */
EAPI Eolian *eolian_new(void);

/*
 * @brief Free an Eolian state.
 *
 * You can use this to free an Eolian state.
 *
 * If the input is NULL, this function has no effect.
 *
 * @param[in] state the state to free
 *
 */
EAPI void eolian_free(Eolian *state);

/*
 * @brief Parse the given .eo or .eot file and fill the database.
 *
 * The input can be either a full path to the file or only a filename.
 * If it's a filename, it must be scanned for first.
 *
 * @param[in] state The Eolian state.
 * @param[in] filepath Path to the file to parse.
 * @return The unit corresponding to the parsed file or NULL.
 *
 * @see eolian_directory_scan
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Unit *eolian_file_parse(Eolian *state, const char *filepath);

/*
 * @brief Get an iterator to all .eo file names with paths.
 *
 * @param[in] state The Eolian state.
 *
 * @see eolian_all_eo_files_get
 * @see eolian_all_eot_file_paths_get
 * @see eolian_all_eot_files_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_all_eo_file_paths_get(const Eolian *state);

/*
 * @brief Get an iterator to all .eot file names with paths.
 *
 * @param[in] state The Eolian state.
 *
 * @see eolian_all_eo_files_get
 * @see eolian_all_eo_file_paths_get
 * @see eolian_all_eot_files_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_all_eot_file_paths_get(const Eolian *state);

/*
 * @brief Get an iterator to all .eo file names (without paths).
 *
 * @param[in] state The Eolian state.
 *
 * @see eolian_all_eo_file_paths_get
 * @see eolian_all_eot_file_paths_get
 * @see eolian_all_eot_files_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_all_eo_files_get(const Eolian *state);

/*
 * @brief Get an iterator to all .eot file names (without paths).
 *
 * @param[in] state The Eolian state.
 *
 * @see eolian_all_eo_file_paths_get
 * @see eolian_all_eot_file_paths_get
 * @see eolian_all_eo_files_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_all_eot_files_get(const Eolian *state);

/*
 * @brief Scan the given directory (recursively) and search for .eo and
 * .eot files.
 *
 * The found files are just open to extract the class name.
 *
 * @param[in] state The Eolian state.
 * @param[in] dir the directory to scan
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @see eolian_system_directory_scan
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_directory_scan(Eolian *state, const char *dir);

/*
 * @brief Scan the system directory (recursively) and search for .eo and
 * .eot files.
 *
 * @param[in] state The Eolian state.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @see eolian_directory_scan
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_system_directory_scan(Eolian *state);

/*
 * @brief Force parsing of all the .eo files located in the directories
 * given in eolian_directory_scan..
 *
 * @param[in] state The Eolian state.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @see eolian_directory_scan
 * @see eolian_all_eot_files_parse
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_all_eo_files_parse(Eolian *state);

/*
 * @brief Force parsing of all the .eot files located in the directories
 * given in eolian_directory_scan..
 *
 * @param[in] state The Eolian state.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @see eolian_directory_scan
 * @see eolian_all_eo_files_parse
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_all_eot_files_parse(Eolian *state);

/*
 * @brief Gets a class by its name
 *
 * @param[in] unit the unit to look in
 * @param[in] class_name name of the class to get.
 * @return the class
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_class_get_by_name(const Eolian_Unit *unit, const char *class_name);

/*
 * @brief Gets a class by its filename (name.eo)
 *
 * @param[in] unit the unit to look in
 * @param[in] file_name the filename
 * @return the class stored in the file
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_class_get_by_file(const Eolian_Unit *unit, const char *file_name);

/*
 * @brief Returns the name of the file containing the given class.
 *
 * @param[in] klass the class.
 * @return the name of the file on success or NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_file_get(const Eolian_Class *klass);

/*
 * @brief Returns the full name of the given class.
 *
 * @param[in] class the class.
 * @return the full name of the class on success or NULL otherwise.
 *
 * The full name and the name of a class will be different if namespaces
 * are used.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_full_name_get(const Eolian_Class *klass);

/*
 * @brief Returns the name of the given class.
 *
 * @param[in] class the class.
 * @return the name of the class on success or NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_name_get(const Eolian_Class *klass);

/*
 * @brief Returns an iterator to the namespaces of the given class.
 *
 * @param[in] class the class.
 * @return the iterator on success or NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_class_namespaces_get(const Eolian_Class *klass);

/*
 * @brief Returns the class type of the given class
 *
 * @param[in] klass the class
 * @return the class type
 *
 * @ingroup Eolian
 */
EAPI Eolian_Class_Type eolian_class_type_get(const Eolian_Class *klass);

/*
 * @brief Returns an iterator to all the classes stored into the database.
 *
 * @param[in] unit the unit to look in
 * @return the iterator
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_all_classes_get(const Eolian_Unit *unit);

/*
 * @brief Returns the documentation of a class.
 *
 * @param[in] klass the class
 * @return the documentation of a class
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_class_documentation_get(const Eolian_Class *klass);

/*
 * @brief Returns the legacy prefix of a class
 *
 * @param[in] klass the class
 * @return the legacy prefix
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_legacy_prefix_get(const Eolian_Class *klass);

/*
 * @brief Returns the eo prefix of a class
 *
 * @param[in] klass the class
 * @return the eo prefix
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare* eolian_class_eo_prefix_get(const Eolian_Class *klass);

/*
 * @brief Returns the event prefix of a class
 *
 * @param[in] klass the class
 * @return the event prefix
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare* eolian_class_event_prefix_get(const Eolian_Class *klass);

/*
 * @brief Returns the data type of a class
 *
 * @param[in] klass the class
 * @return the data type
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_data_type_get(const Eolian_Class *klass);

/*
 * @brief Returns an iterator to the inherited classes.
 *
 * @param[in] klass the class
 * @return the iterator
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_class_inherits_get(const Eolian_Class *klass);

/*
 * @brief Returns an iterator to functions of a class.
 *
 * @param[in] klass the class
 * @param[in] func_type type of the functions to insert into the list.
 * @return the iterator
 *
 * Acceptable inputs are EOLIAN_PROPERTY or EOLIAN_METHOD.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_class_functions_get(const Eolian_Class *klass, Eolian_Function_Type func_type);

/*
 * @brief Returns the type of a function
 *
 * @param[in] function_id Id of the function
 * @return the function type
 *
 * @ingroup Eolian
 */
EAPI Eolian_Function_Type eolian_function_type_get(const Eolian_Function *function_id);

/*
 * @brief Returns the scope of a function
 *
 * @param[in] function_id Id of the function
 * @param[in] ftype The type of function to get the scope for
 * @return the function scope
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Object_Scope eolian_function_scope_get(const Eolian_Function *function_id, Eolian_Function_Type ftype);

/*
 * @brief Returns the name of a function
 *
 * @param[in] function_id Id of the function
 * @return the function name
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_function_name_get(const Eolian_Function *function_id);

/*
 * @brief Returns the full C name of a function.
 *
 * @param[in] function_id Id of the function
 * @param[in] ftype The type of function to get the name for
 * @param[in] use_legacy If true, legacy prefix or name will be used when available
 * @return the function name
 *
 * It's here because the C API names are deduplicated (prefix of function and
 * suffix of prefix merge if applicable) and this helps generators not write
 * the same code over and over.
 *
 * If legacy name is supplied for the given type and use_legacy is set, it
 * will be used. Also, if the given type is PROP_GET or PROPERTY, a "_get"
 * suffix will be applied when not using legacy name, and "_set" for PROP_SET.
 *
 * Also, you're responsible for deleting the stringshare.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_function_full_c_name_get(const Eolian_Function *function_id, Eolian_Function_Type ftype, Eina_Bool use_legacy);

/*
 * @brief Get a function in a class by its name and type
 *
 * @param[in] klass the class
 * @param[in] func_name name of the function
 * @param[in] f_type type of the function
 * @return the function id if found, NULL otherwise.
 *
 * Providing EOLIAN_UNRESOLVED finds any func, EOLIAN_PROPERTY any property,
 * EOLIAN_METHOD any method, EOLIAN_PROP_GET properties with either only a getter
 * or full property, EOLIAN_PROP_SET either only a setter or full property.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Function *eolian_class_function_get_by_name(const Eolian_Class *klass, const char *func_name, Eolian_Function_Type f_type);

/*
 * @brief Returns a legacy name for a function.
 *
 * @param[in] function_id Id of the function
 * @param[in] f_type The function type, for property get/set distinction.
 * @return the legacy name or NULL.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_function_legacy_get(const Eolian_Function *function_id, Eolian_Function_Type f_type);

/*
 * @brief Returns the implement for a function.
 *
 * @param[in] function_id Id of the function
 * @return the implement or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Implement *eolian_function_implement_get(const Eolian_Function *function_id);

/*
 * @brief Indicates if a function is legacy only.
 *
 * @param[in] function_id Id of the function
 * @param[in] f_type The function type, for property get/set distinction.
 * @return EINA_TRUE if legacy only, EINA_FALSE otherwise.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_legacy_only(const Eolian_Function *function_id, Eolian_Function_Type ftype);

/*
 * @brief Get whether a function is a class method/property.
 *
 * @param[in] function_id Id of the function
 * @return EINA_TRUE and EINA_FALSE respectively
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_class(const Eolian_Function *function_id);

/*
 * @brief Get whether a function is beta.
 *
 * @param[in] function_id Id of the function
 * @return EINA_TRUE and EINA_FALSE respectively
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_beta(const Eolian_Function *function_id);

/*
 * @brief Indicates if a function is a constructing function of a given class.
 *
 * @param[in] klass the class
 * @param[in] function_id Id of the function
 * @return EINA_TRUE and EINA_FALSE respectively
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_constructor(const Eolian_Function *function_id, const Eolian_Class *klass);

/*
 * @brief Get whether a function is a function pointer.
 *
 * @param[in] function_id Id of the function
 * @return EINA_TRUE and EINA_FALSE respectively
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_function_pointer(const Eolian_Function *function_id);

/*
 * @brief Returns an iterator to the parameter handles for a method/ctor/dtor.
 *
 * @param[in] function_id Id of the function
 * @return the iterator
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_function_parameters_get(const Eolian_Function *function_id);

/*
 * @brief Returns an iterator to the keys params of a given function.
 *
 * @param[in] function_id Id of the function
 * @param[in] ftype The function type, for property get/set distinction.
 * @return the iterator
 *
 * Acceptable input types are PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_property_keys_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);

/*
 * @brief Returns an iterator to the values params of a given function.
 *
 * @param[in] function_id Id of the function
 * @param[in] ftype The function type, for property get/set distinction.
 * @return the iterator
 *
 * Acceptable input types are PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_property_values_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);

/*
 * @brief Get direction of a parameter
 *
 * @param[in] param_desc parameter handle
 * @return the direction of the parameter
 *
 * @ingroup Eolian
 */
EAPI Eolian_Parameter_Dir eolian_parameter_direction_get(const Eolian_Function_Parameter *param);

/*
 * @brief Get type of a parameter
 *
 * @param[in] param_desc parameter handle
 * @return the type of the parameter
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_parameter_type_get(const Eolian_Function_Parameter *param);

/*
 * @brief Get the default value of a parameter
 *
 * @param[in] param_desc parameter handle
 * @return the value or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *eolian_parameter_default_value_get(const Eolian_Function_Parameter *param);

/*
 * @brief Get name of a parameter
 *
 * @param[in] param_desc parameter handle
 * @return the name of the parameter
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_parameter_name_get(const Eolian_Function_Parameter *param);

/*
 * @brief Get documentation of a parameter
 *
 * @param[in] param_desc parameter handle
 * @return the documentation of the parameter or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_parameter_documentation_get(const Eolian_Function_Parameter *param);

/*
 * @brief Indicates if a parameter cannot be NULL.
 *
 * @param[in] param_desc parameter handle
 * @return EINA_TRUE if cannot be NULL, EINA_FALSE otherwise
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_parameter_is_nonull(const Eolian_Function_Parameter *param_desc);

/*
 * @brief Indicates if a parameter is nullable.
 *
 * @param[in] param_desc parameter handle
 * @return EINA_TRUE if nullable, EINA_FALSE otherwise
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_parameter_is_nullable(const Eolian_Function_Parameter *param_desc);

/*
 * @brief Indicates if a parameter is optional.
 *
 * @param[in] param_desc parameter handle
 * @return EINA_TRUE if optional, EINA_FALSE otherwise
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_parameter_is_optional(const Eolian_Function_Parameter *param_desc);

/*
 * @brief Get the return type of a function.
 *
 * @param[in] function_id id of the function
 * @param[in] ftype type of the function
 * @return the return type of the function
 *
 * The type of the function is needed because a given function can represent a
 * property, that can be set and get functions.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_function_return_type_get(const Eolian_Function *function_id, Eolian_Function_Type ftype);

/*
 * @brief Get the return default value of a function.
 *
 * @param[in] function_id id of the function
 * @param[in] ftype type of the function
 * @return the return default value of the function
 *
 * The return default value is needed to return an appropriate
 * value if an error occurs (eo_do failure...).
 * The default value is not mandatory, so NULL can be returned.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *
eolian_function_return_default_value_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);

/*
 * @brief Get the return docs of a function.
 *
 * @param[in] function_id id of the function
 * @param[in] ftype type of the function
 * @return the return docs of the function
 *
 * The type of the function is needed because a given function can represent a
 * property, that can be set and get functions.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_function_return_documentation_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);

/*
 * @brief Indicates if a function return is warn-unused.
 *
 * @param[in] function_id id of the function
 * @param[in] ftype type of the function
 * @return EINA_TRUE is warn-unused, EINA_FALSE otherwise.
 *
 * The type of the function is needed because a given function can represent a
 * property, that can be set and get functions.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_return_is_warn_unused(const Eolian_Function *foo_id, Eolian_Function_Type ftype);

/*
 * @brief Indicates if a function object is const.
 *
 * @param[in] function_id id of the function
 * @return EINA_TRUE if the object is const, EINA_FALSE otherwise
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_object_is_const(const Eolian_Function *function_id);

/*
 * @brief Return the Eolian class associated to the function.
 *
 * @param[in] function_id id of the function
 * @return the class, NULL otherwise
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_function_class_get(const Eolian_Function *function_id);

/*
 * @brief Get full string of an overriding function (implement).
 *
 * @param[in] impl the handle of the implement
 * @return the full string.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_implement_full_name_get(const Eolian_Implement *impl);

/*
 * @brief Get the class of an overriding function (implement).
 *
 * @param[in] impl the handle of the implement
 * @return the class handle or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_implement_class_get(const Eolian_Implement *impl);

/*
 * @brief Get the function of an implement.
 *
 * @param[in] impl the handle of the implement
 * @param[out] func_type the function type.
 * @return the function handle or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Function *eolian_implement_function_get(const Eolian_Implement *impl, Eolian_Function_Type *func_type);

/*
 * @brief Returns a documentation for an implement.
 *
 * @param[in] impl the handle of the implement
 * @param[in] f_type The function type, for property get/set distinction.
 * @return the documentation or NULL.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_implement_documentation_get(const Eolian_Implement *impl, Eolian_Function_Type f_type);

/*
 * @brief Get whether an implement is tagged with @auto.
 *
 * @param[in] impl the handle of the implement
 * @param[in] f_type The function type, for property get/set distinction.
 * @return EINA_TRUE when it is, EINA_FALSE when it's not.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_implement_is_auto(const Eolian_Implement *impl, Eolian_Function_Type f_type);

/*
 * @brief Get whether an implement is tagged with @empty.
 *
 * @param[in] impl the handle of the implement
 * @param[in] f_type The function type, for property get/set distinction.
 * @return EINA_TRUE when it is, EINA_FALSE when it's not.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_implement_is_empty(const Eolian_Implement *impl, Eolian_Function_Type f_type);

/*
 * @brief Get whether an implement is pure virtual.
 *
 * @param[in] impl the handle of the implement
 * @param[in] f_type The function type, for property get/set distinction.
 * @return EINA_TRUE when it is, EINA_FALSE when it's not.
 *
 * Acceptable input types are METHOD, PROP_GET and PROP_SET.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_implement_is_pure_virtual(const Eolian_Implement *impl, Eolian_Function_Type f_type);

/*
 * @brief Get whether an implement references a property getter.
 *
 * @param[in] impl the handle of the implement
 * @return EINA_TRUE when it does, EINA_FALSE when it's not.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_implement_is_prop_get(const Eolian_Implement *impl);

/*
 * @brief Get whether an implement references a property setter.
 *
 * @param[in] impl the handle of the implement
 * @return EINA_TRUE when it does, EINA_FALSE when it's not.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_implement_is_prop_set(const Eolian_Implement *impl);

/*
 * @brief Get an iterator to implements of a class.
 *
 * @param[in] klass the class.
 * @return the iterator
 *
 * Implements include fields specified in the "implements" section of your Eo
 * file (i.e. overriding and pure virtual/auto/empty functions) and all other
 * methods/properties of your class (local only) that are not specified
 * within that section.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_class_implements_get(const Eolian_Class *klass);

/*
 * @brief Get full string of a constructing function.
 *
 * @param[in] ctor the handle of the constructor
 * @return the full string.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_constructor_full_name_get(const Eolian_Constructor *ctor);

/*
 * @brief Get the class of a constructing function.
 *
 * @param[in] ctor the handle of the constructor
 * @return the class handle or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_constructor_class_get(const Eolian_Constructor *ctor);

/*
 * @brief Get the function of a constructing function.
 *
 * @param[in] ctor the handle of the constructor
 * @return the function handle or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Function *eolian_constructor_function_get(const Eolian_Constructor *ctor);

/*
 * @brief Checks if a constructor is tagged optional.
 *
 * @param[in] ctor the handle of the constructor
 * @return EINA_TRUE if optional, EINA_FALSE if not (or if input is NULL).
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_constructor_is_optional(const Eolian_Constructor *ctor);

/*
 * @brief Get an iterator to the constructing functions defined in a class.
 *
 * @param[in] klass the class.
 * @return the iterator
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_class_constructors_get(const Eolian_Class *klass);

/*
 * @brief Get an iterator to the events defined in a class.
 *
 * @param[in] klass the class.
 * @return an iterator over const Eolian_Event* objects
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_class_events_get(const Eolian_Class *klass);

/*
 * @brief Get the name of an event.
 *
 * @param[in] event the event handle
 * @return the name or NULL
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_event_name_get(const Eolian_Event *event);

/*
 * @brief Get the type of an event.
 *
 * @param[in] event the event handle
 * @return the type or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_event_type_get(const Eolian_Event *event);

/*
 * @brief Get the documentation of an event.
 *
 * @param[in] event the event handle
 * @return the documentation or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_event_documentation_get(const Eolian_Event *event);

/*
 * @brief Returns the scope of an event
 *
 * @param[in] event the event handle
 * @return the event scope
 *
 * @ingroup Eolian
 */
EAPI Eolian_Object_Scope eolian_event_scope_get(const Eolian_Event *event);

/*
 * @brief Get whether an event is beta.
 *
 * @param[in] event the event handle
 * @return EINA_TRUE and EINA_FALSE respectively
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_event_is_beta(const Eolian_Event *event);

/*
 * @brief Get whether an event is hot (unfreezable).
 *
 * @param[in] event the event handle
 * @return EINA_TRUE and EINA_FALSE respectively
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_event_is_hot(const Eolian_Event *event);

/*
 * @brief Get whether an event is a restartable event.
 *
 * @param[in] event the event handle
 * @return EINA_TRUE and EINA_FALSE respectively
 *
 * In case of nested call, restartable event will start processing from where
 * they where in the parent callback call skipping all the previously executed
 * callback. Especially useful for nested main loop use case.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_event_is_restart(const Eolian_Event *event);

/*
 * @brief Get an iterator to the parts defined in a class.
 *
 * @param[in] klass the class.
 * @return an iterator over const Eolian_Part* objects
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_class_parts_get(const Eolian_Class *klass);

/*
 * @brief Get the name of a part.
 *
 * @param[in] part the part handle
 * @return the name or NULL
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_part_name_get(const Eolian_Part *part);

/*
 * @brief Get the type of a part.
 *
 * @param[in] part the part handle
 * @return the type or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_part_class_get(const Eolian_Part *part);

/*
 * @brief Get the documentation of an part.
 *
 * @param[in] part the part handle
 * @return the documentation or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_part_documentation_get(const Eolian_Part *part);

/*
 * @brief Returns the C name of an event
 *
 * @param[in] event the event handle
 * @return the event C name
 *
 * You're responsible for deleting the stringshare.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_event_c_name_get(const Eolian_Event *event);

/*
 * @brief Get an event in a class by its name
 *
 * @param[in] klass the class
 * @param[in] event_name name of the event
 * @return the Eolian_Event if found, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Event *eolian_class_event_get_by_name(const Eolian_Class *klass, const char *event_name);

/*
 * @brief Indicates if the class constructor has to invoke
 * a non-generated class constructor function.
 *
 * @param[in] klass the class.
 * @return EINA_TRUE if the invocation is needed, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_class_ctor_enable_get(const Eolian_Class *klass);

/*
 * @brief Indicates if the class destructor has to invoke
 * a non-generated class destructor function.
 *
 * @param[in] klass the class.
 * @return EINA_TRUE if the invocation is needed, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_class_dtor_enable_get(const Eolian_Class *klass);

/*
 * @brief Returns the name of the C function used to get the Efl_Class pointer.
 *
 * @param[in] klass the class.
 * @return a stringshare containing the func name or NULL on error.
 *
 * You have to delete the stringshare manually.
 *
 * @see eolian_class_c_name_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_c_get_function_name_get(const Eolian_Class *klass);

/*
 * @brief Get the C name of the class.
 *
 * @param[in] klass the class
 * @return the C name
 *
 * The C name is the name of the macro the class is accessed through, in format
 * CLASS_NAME_SUFFIX where SUFFIX is CLASS, MIXIN or INTERFACE. You're responsible
 * for the stringshare afterwards.
 *
 * @see eolian_class_c_get_function_name_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_c_name_get(const Eolian_Class *klass);

/*
 * @brief Get the C data type of the class.
 *
 * @param[in] klass the class
 * @return the C data type
 *
 * This will sanitize the data type of the class for C usage; if it's "null",
 * this returns "void"; if it's actually explicitly set, it returns the sanitized
 * version of the string, otherwise it returns Class_Name_Data. Keep in mind that
 * this does not add an asterisk (it doesn't return a pointer type name). You're
 * responsible for the stringshare afterwards.
 *
 * @see eolian_class_c_get_function_name_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_c_data_type_get(const Eolian_Class *klass);

/*
 * @brief Get an alias type declaration by name. Supports namespaces.
 *
 * @param[in] unit the unit to look in
 * @param[in] name the name of the alias
 * @return the alias type or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Typedecl *eolian_typedecl_alias_get_by_name(const Eolian_Unit *unit, const char *name);

/*
 * @brief Get a struct declaration by name. Supports namespaces.
 *
 * @param[in] unit the unit to look in
 * @param[in] name the name of the struct
 * @return the struct or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Typedecl *eolian_typedecl_struct_get_by_name(const Eolian_Unit *unit, const char *name);

/*
 * @brief Get an enum declaration by name. Supports namespaces.
 *
 * @param[in] unit the unit to look in
 * @param[in] name the name of the struct
 * @return the struct or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Typedecl *eolian_typedecl_enum_get_by_name(const Eolian_Unit *unit, const char *name);

/*
 * @brief Get an iterator to all aliases contained in a file.
 *
 * @param[in] unit the unit to look in
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_aliases_get_by_file(const Eolian_Unit *unit, const char *fname);

/*
 * @brief Get an iterator to all named structs contained in a file.
 *
 * @param[in] unit the unit to look in
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_structs_get_by_file(const Eolian_Unit *unit, const char *fname);

/*
 * @brief Get an iterator to all enums contained in a file.
 *
 * @param[in] unit the unit to look in
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_enums_get_by_file(const Eolian_Unit *unit, const char *fname);

/*
 * @brief Get an iterator to all aliases in the Eolian database.
 *
 * @param[in] unit the unit to look in
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_all_aliases_get(const Eolian_Unit *unit);

/*
 * @brief Get an iterator to all structs in the Eolian database.
 *
 * @param[in] unit the unit to look in
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_all_structs_get(const Eolian_Unit *unit);

/*
 * @brief Get an iterator to all enums in the Eolian database.
 *
 * @param[in] unit the unit to look in
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_all_enums_get(const Eolian_Unit *unit);

/*
 * @brief Get the type of a type declaration.
 *
 * @param[in] tp the type declaration.
 * @return an Eolian_Typedecl_Type.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Typedecl_Type eolian_typedecl_type_get(const Eolian_Typedecl *tp);

/*
 * @brief Get an iterator to all fields of a struct type.
 *
 * @param[in] tp the type declaration.
 * @return the iterator when @c tp is EOLIAN_TYPEDECL_STRUCT, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_struct_fields_get(const Eolian_Typedecl *tp);

/*
 * @brief Get a field of a struct type.
 *
 * @param[in] tp the type declaration.
 * @param[in] field the field name.
 * @return the field when @c tp is EOLIAN_TYPEDECL_STRUCT, @c field is not NULL
 * and the field exists, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Struct_Type_Field *eolian_typedecl_struct_field_get(const Eolian_Typedecl *tp, const char *field);

/*
 * @brief Get the name of a field of a struct type.
 *
 * @param[in] fl the field.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_struct_field_name_get(const Eolian_Struct_Type_Field *fl);

/*
 * @brief Get the documentation of a field of a struct type.
 *
 * @param[in] fl the field.
 * @return the documentation.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_typedecl_struct_field_documentation_get(const Eolian_Struct_Type_Field *fl);

/*
 * @brief Get the type of a field of a struct type.
 *
 * @param[in] fl the field.
 * @return the type.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_typedecl_struct_field_type_get(const Eolian_Struct_Type_Field *fl);

/*
 * @brief Get an iterator to all fields of an enum type.
 *
 * @param[in] tp the type declaration.
 * @return the iterator when @c tp is EOLIAN_TYPEDECL_ENUM, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_enum_fields_get(const Eolian_Typedecl *tp);

/*
 * @brief Get a field of an enum type.
 *
 * @param[in] tp the type declaration.
 * @param[in] field the field name.
 * @return the field when @c tp is EOLIAN_TYPEDECL_ENUM, @c field is not NULL,
 * field exists and has a value set, NULL otherwise.
 *
 * Keep in mind that this can return NULL for an existing field, particularly
 * when the field has no value set (i.e. increments by 1 over previous value).
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Enum_Type_Field *eolian_typedecl_enum_field_get(const Eolian_Typedecl *tp, const char *field);

/*
 * @brief Get the name of a field of an enum type.
 *
 * @param[in] fl the field.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_enum_field_name_get(const Eolian_Enum_Type_Field *fl);

/*
 * @brief Get the C name of a field of an enum type.
 *
 * The user of the API is responsible for the resulting stringshare.
 *
 * @param[in] fl the field.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_enum_field_c_name_get(const Eolian_Enum_Type_Field *fl);

/*
 * @brief Get the documentation of a field of an enum type.
 *
 * @param[in] fl the field.
 * @return the documentation.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_typedecl_enum_field_documentation_get(const Eolian_Enum_Type_Field *fl);

/*
 * @brief Get the value of a field of an enum type.
 *
 * When the @c force parameter is EINA_FALSE, this will only return values for
 * fields which are explicitly specified in the eo file, otherwise it will
 * return a valid expression for any field.
 *
 * @param[in] fl the field.
 * @param[in] force force the value retrieval.
 * @return the expression.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *eolian_typedecl_enum_field_value_get(const Eolian_Enum_Type_Field *fl, Eina_Bool force);

/*
 * @brief Get the legacy prefix of enum field names. When not specified,
 * enum name is used.
 *
 * @param[in] tp the type declaration.
 * @return the legacy prefix or NULL.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_enum_legacy_prefix_get(const Eolian_Typedecl *tp);

/*
 * @brief Get the documentation of a struct/alias type.
 *
 * @param[in] tp the type declaration.
 * @return the documentation when @c tp is EOLIAN_TYPE_STRUCT or
 * EOLIAN_TYPE_STRUCT_OPAQUE, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_typedecl_documentation_get(const Eolian_Typedecl *tp);

/*
 * @brief Get the filename of a type declaration.
 *
 * @param[in] tp the type declaration.
 * @return the filename.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_file_get(const Eolian_Typedecl *tp);

/*
 * @brief Get the base type of an alias declaration.
 *
 * @param[in] tp the type declaration.
 * @return the base type when @c tp is an alias, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_typedecl_base_type_get(const Eolian_Typedecl *tp);

/*
 * @brief Get the lowest base type of an alias stack.
 *
 * If the given typedecl is an alias, it returns the result of
 * eolian_type_aliased_base_get on its base type. Otherwise this returns NULL.
 *
 * @param[in] tp the type declaration.
 * @return the lowest alias base or the given type.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_typedecl_aliased_base_get(const Eolian_Typedecl *tp);

/*
 * @brief Check if a struct or alias type declaration is extern.
 *
 * @param[in] tp the type declaration.
 * @return EINA_TRUE if it's extern, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_typedecl_is_extern(const Eolian_Typedecl *tp);

/*
 * @brief Get the full C type name of the given type.
 *
 * @param[in] tp the type declaration.
 * @return The C type name assuming @c tp is not NULL.
 *
 * You're responsible for deleting the stringshare.
 *
 * @see eolian_type_c_type_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_c_type_get(const Eolian_Typedecl *tp);

/*
 * @brief Get the name of the given type declaration. Keep in mind that the
 * name doesn't include namespaces.
 *
 * @param[in] tp the type declaration.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_name_get(const Eolian_Typedecl *tp);

/*
 * @brief Get the full (namespaced) name of a type declaration.
 *
 * @param[in] tp the type declaration.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_full_name_get(const Eolian_Typedecl *tp);

/*
 * @brief Get an iterator to the list of namespaces of the given type decl.
 *
 * @param[in] tp the type declaration.
 * @return the iterator.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_typedecl_namespaces_get(const Eolian_Typedecl *tp);

/*
 * @brief Get the name of the function used to free this type declaration.
 *
 * @param[in] tp the type declaration.
 * @return the free func name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_typedecl_free_func_get(const Eolian_Typedecl *tp);

/*
 * @breif Get the function object for this function pointer type.
 *
 * @param[in] tp the type.
 * @return the function or NULL;
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Function *eolian_typedecl_function_pointer_get(const Eolian_Typedecl *tp);

/*
 * @brief Get the type of a type.
 *
 * @param[in] tp the type.
 * @return an Eolian_Type_Type.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type_Type eolian_type_type_get(const Eolian_Type *tp);

/*
 * @brief Get the builtin type of a type.
 *
 * @param[in] tp the type.
 * @return an Eolian_Type_Builtin_Type.
 *
 * If the input type is NULL or it's not a builtin, EOLIAN_TYPE_BUILTIN_INVALID
 * is returned.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type_Builtin_Type eolian_type_builtin_type_get(const Eolian_Type *tp);

/*
 * @brief Get the filename of a type.
 *
 * @param[in] tp the type.
 * @return the filename.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_file_get(const Eolian_Type *tp);

/*
 * @brief Get the base type of a type.
 *
 * Only applies to "complex" ordinary types, i.e. this is the first inner
 * type in <>.
 *
 * @param[in] tp the type.
 * @return the base type or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_base_type_get(const Eolian_Type *tp);

/*
 * @brief Get the next inner type of a complex type.
 *
 * The inner types of a complex type form a chain. Therefore, you first retrieve
 * the first one via eolian_type_base_type_get and then get the next one via
 * this API function called on the first inner type if necessary.
 *
 * @param[in] tp the type.
 * @return the next type or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_next_type_get(const Eolian_Type *tp);

/*
 * @brief Get the declaration a regular type points to.
 *
 * This tries to look up alias, struct and enum in that order.
 *
 * @param[in] tp the type.
 * @return the pointed to type decalration or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Typedecl *eolian_type_typedecl_get(const Eolian_Type *tp);

/*
 * @brief Get the lowest base type of an alias stack.
 *
 * If this is a regular type, it first tries to retrieve its base declaration
 * using eolian_type_typedecl_get and if the retrieved base is an alias, returns
 * a call of eolian_typedecl_aliased_base_get function on it. Otherwise it
 * returns the given type. This is useful in order to retrieve what an aliased
 * type actually is while still having convenience. Keep in mind that this stops
 * if the found type is actually a pointer (has a ptr() on it).
 *
 * @param[in] tp the type.
 * @return the lowest alias base or the given type.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_aliased_base_get(const Eolian_Type *tp);

/*
 * @brief Get the class associated with an EOLIAN_TYPE_CLASS type.
 *
 * @param[in] tp the type.
 * @return the class or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_type_class_get(const Eolian_Type *tp);

/*
 * @brief Get whether the given type is owned.
 *
 * This is true when a parameter, return or whatever is marked as @owned.
 *
 * @param[in] tp the type.
 * @return EINA_TRUE when the type is marked owned, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_type_is_owned(const Eolian_Type *tp);

/*
 * @brief Get whether the given type is const.
 *
 * @param[in] tp the type.
 * @return EINA_TRUE when the type is const, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_type_is_const(const Eolian_Type *tp);

/*
 * @brief Get whether the given type is a reference.
 *
 * @param[in] tp the type.
 * @return EINA_TRUE when the type is marked ref, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_type_is_ptr(const Eolian_Type *tp);

/*
 * @brief Get the full C type name of the given type.
 *
 * @param[in] tp the type.
 * @param[in] ctype the context within which the C type string will be used.
 * @return The C type name assuming @c tp is not NULL.
 *
 * You're responsible for the stringshare.
 *
 * @see eolian_typedecl_c_type_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_c_type_get(const Eolian_Type *tp, Eolian_C_Type_Type ctype);

/*
 * @brief Get the name of the given type. For regular types, this is for
 * example "int". For EOLIAN_TYPE_CLASS, this can be "Button". Keep in
 * mind that the name doesn't include namespaces.
 *
 * @param[in] tp the type.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_name_get(const Eolian_Type *tp);

/*
 * @brief Get the full (namespaced) name of a type.
 *
 * @param[in] tp the type.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_full_name_get(const Eolian_Type *tp);

/*
 * @brief Get an iterator to the list of namespaces of the given type.
 *
 * @param[in] tp the type.
 * @return the iterator.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_namespaces_get(const Eolian_Type *tp);

/*
 * @brief Get the name of the function used to free this type.
 *
 * @param[in] tp the type.
 * @return the free func name.
 *
 * For types that translate to C pointers, this is the function used to
 * free them. For other types, this is the function to free a pointer to
 * those types.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_free_func_get(const Eolian_Type *tp);

/*
 * @brief Evaluate an Eolian expression.
 *
 * @param[in] expr the expression.
 * @param[in] mask the mask of allowed values (can combine with bitwise OR).
 * @return the value, its type is set to EOLIAN_EXPR_UNKNOWN on error.
 *
 * Represents value types from Eolian_Expression_Type. Booleans
 * are represented as unsigned char, strings as a stringshare.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Value eolian_expression_eval(const Eolian_Expression *expr, Eolian_Expression_Mask m);

/*
 * @brief Evaluate an Eolian expression given a type instead of a mask.
 *
 * @param[in] expr the expression.
 * @param[in] type the type the expression is assigned to.
 * @return the value, its type is set to EOLIAN_EXPR_UNKNOWN on error.
 *
 * The mask is automatically decided from the given type, allowing only values
 * that can be assigned to that type.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Value eolian_expression_eval_type(const Eolian_Expression *expr, const Eolian_Type *type);

/*
 * @brief Convert the result of expression evaluation to a literal as in how
 * it would appear in C (e.g. strings are quoted and escaped).
 *
 * @param[in] v the value.
 * @return a stringshare containing the literal (quoted and escaped as needed)
 * or NULL.
 *
 * For e.g. strings this only uses a subset of regular C escape sequences
 * so that interoperability is wider than just C (no octal escapes). For
 * languages that differ too much, you can write an equivalent function
 * yourself.
 * Also, you're responsible for deleting the stringshare.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_expression_value_to_literal(const Eolian_Value *v);

/*
 * @brief Serialize an expression.
 *
 * @param[in] expr the expression.
 * @return the serialized expression or NULL.
 *
 * This serializes the expression into the original form as written in the .eo
 * file (but with parens on binary operators explicitly specifying precedence).
 * Keep in mind that it cannot be used alone pasted into C code as it doesn't
 * resolve namespaces and enum field names.
 * Also, you're responsible for deleting the stringshare.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_expression_serialize(const Eolian_Expression *expr);

/*
 * @brief Get the type of an expression.
 *
 * @param[in] expr the expression.
 * @return the expression type.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Expression_Type eolian_expression_type_get(const Eolian_Expression *expr);

/*
 * @brief Get the binary operator of an expression.
 *
 * @param[in] expr the expression.
 * @return the binary operator, EOLIAN_BINOP_INVALID on failure.
 *
 * This only works on binary expressions, otherwise it returns
 * EOLIAN_BINOP_INVALID.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Binary_Operator eolian_expression_binary_operator_get(const Eolian_Expression *expr);

/*
 * @brief Get the lhs (left hand side) of a binary expression.
 *
 * @param[in] expr the expression.
 * @return the expression or NULL.
 *
 * This only works on binary expressions, otherwise it returns NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *eolian_expression_binary_lhs_get(const Eolian_Expression *expr);

/*
 * @brief Get the rhs (right hand side) of a binary expression.
 *
 * @param[in] expr the expression.
 * @return the expression or NULL.
 *
 * This only works on binary expressions, otherwise it returns NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *eolian_expression_binary_rhs_get(const Eolian_Expression *expr);

/*
 * @brief Get the unary operator of an expression.
 *
 * @param[in] expr the expression.
 * @return the unary operator, EOLIAN_UNOP_INVALID on failure.
 *
 * This only works on unary expressions, otherwise it returns
 * EOLIAN_UNOP_INVALID.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Unary_Operator eolian_expression_unary_operator_get(const Eolian_Expression *expr);

/*
 * @brief Get the expression of an unary expression.
 *
 * @param[in] expr the expression.
 * @return the expression or NULL.
 *
 * This only works on unary expressions, otherwise it returns NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *eolian_expression_unary_expression_get(const Eolian_Expression *expr);

/*
 * @brief Get the value of an expression.
 *
 * @param[in] expr the expression.
 * @return the value.
 *
 * Keep in mind that this doesn't evaluate anything. That's why it only works
 * on expressions that actually hold values (not unknown, not binary, not
 * unary). For some types of expressions (enum, name), this stores the actual
 * name (in the value.s field). Resources for this are held by the database.
 * Don't attempt to free the string or anything like that.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Value eolian_expression_value_get(const Eolian_Expression *expr);

/*
 * @brief Get a global variable by name. Supports namespaces.
 *
 * @param[in] unit the unit to look in
 * @param[in] name the name of the variable
 * @return the variable handle or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Variable *eolian_variable_global_get_by_name(const Eolian_Unit *unit, const char *name);

/*
 * @brief Get a constant variable by name. Supports namespaces.
 *
 * @param[in] unit the unit to look in
 * @param[in] name the name of the variable
 * @return the variable handle or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Variable *eolian_variable_constant_get_by_name(const Eolian_Unit *unit, const char *name);

/*
 * @brief Get an iterator to all global variables contained in a file.
 *
 * @param[in] unit the unit to look in
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_variable_globals_get_by_file(const Eolian_Unit *unit, const char *fname);

/*
 * @brief Get an iterator to all constant variables contained in a file.
 *
 * @param[in] unit the unit to look in
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_variable_constants_get_by_file(const Eolian_Unit *unit, const char *fname);

/*
 * @brief Get an iterator to all constant variables in the Eolian database.
 *
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_variable_all_constants_get(const Eolian_Unit *unit);

/*
 * @brief Get an iterator to all global variables in the Eolian database.
 *
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_variable_all_globals_get(const Eolian_Unit *unit);

/*
 * @brief Get the type of a variable (global, constant)
 *
 * @param[in] var the variable.
 * @return an Eolian_Type_Type.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Variable_Type eolian_variable_type_get(const Eolian_Variable *var);

/*
 * @brief Get the documentation of a variable.
 *
 * @param[in] var the variable.
 * @return the documentation or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Documentation *eolian_variable_documentation_get(const Eolian_Variable *var);

/*
 * @brief Get the filename of a variable.
 *
 * @param[in] var the variable.
 * @return the filename or NULL.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_variable_file_get(const Eolian_Variable *var);

/*
 * @brief Get the base type of a variable.
 *
 * @param[in] var the variable.
 * @return the base type or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_variable_base_type_get(const Eolian_Variable *var);

/*
 * @brief Get the value of a variable.
 *
 * @param[in] var the variable.
 * @return the value or NULL.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *eolian_variable_value_get(const Eolian_Variable *var);

/*
 * @brief Get the name of the given variable (without namespaces).
 *
 * @param[in] var the variable.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_variable_name_get(const Eolian_Variable *var);

/*
 * @brief Get the name of the given variable (with namespaces).
 *
 * @param[in] var the variable.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_variable_full_name_get(const Eolian_Variable *var);

/*
 * @brief Get an iterator to the list of namespaces of the given variable.
 *
 * @param[in] var the variable.
 * @return the iterator.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_variable_namespaces_get(const Eolian_Variable *var);

/*
 * @brief Check if a variable is extern.
 *
 * @param[in] var the variable.
 * @return EINA_TRUE if it's extern, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_variable_is_extern(const Eolian_Variable *var);

/*
 * @brief Get a declaration by name.
 *
 * @param[in] unit the unit to look in
 * @param[in] name the declaration name.
 * @return the declaration.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Declaration *eolian_declaration_get_by_name(const Eolian_Unit *unit, const char *name);

/*
 * @brief Get a list of declarations in a file.
 *
 * This function gets an iterator to a list of declarations in a particular
 * file. Declarations are either a class, a type alias, a struct, an enum
 * or a variable. This way you can get them all in the original order they
 * were declared in, which is useful during generation.
 *
 * @param[in] state the state to look in
 * @param[in] fname the filename.
 * @return the iterator or NULL.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_declarations_get_by_file(const Eolian *state, const char *fname);

/*
 * @brief Get an iterator to all declarations in the Eolian database.
 *
 * @param[in] unit the unit to look in
 * @return the iterator or NULL.
 *
 * Thanks to internal caching this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_all_declarations_get(const Eolian_Unit *unit);

/*
 * @brief Get the type of a declaration
 *
 * @param[in] decl the declaration
 * @return the declaration type
 *
 * @ingroup Eolian
 */
EAPI Eolian_Declaration_Type eolian_declaration_type_get(const Eolian_Declaration *decl);

/*
 * @brief Get the name of a declaration
 *
 * This matches the full namespaced name of the data it's holding.
 *
 * @param[in] decl the declaration
 * @return the declaration name
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_declaration_name_get(const Eolian_Declaration *decl);

/*
 * @brief Get the class of a class declaration.
 *
 * @param[in] decl the declaration
 * @return the class
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_declaration_class_get(const Eolian_Declaration *decl);

/*
 * @brief Get the type of a type (alias, struct, enum) declaration.
 *
 * @param[in] decl the declaration
 * @return the type
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Typedecl *eolian_declaration_data_type_get(const Eolian_Declaration *decl);

/*
 * @brief Get the variable of a variable (constant, global) declaration.
 *
 * @param[in] decl the declaration
 * @return the class
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Variable *eolian_declaration_variable_get(const Eolian_Declaration *decl);

/*
 * @brief Get the summary of the documentation.
 *
 * This should never return NULL unless the input is invalid.
 *
 * @param[in] doc the documentation
 * @return the summary or NULL
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_documentation_summary_get(const Eolian_Documentation *doc);

/*
 * @brief Get the description of the documentation.
 *
 * This can return NULL if the description wasn't specified or
 * if the input is wrong.
 *
 * @param[in] doc the documentation
 * @return the description or NULL
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_documentation_description_get(const Eolian_Documentation *doc);

/*
 * @brief Get the "since" tag of the documentation.
 *
 * This can return NULL if the tag wasn't specified or
 * if the input is wrong.
 *
 * @param[in] doc the documentation
 * @return the description or NULL
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_documentation_since_get(const Eolian_Documentation *doc);

/*
 * @brief Split a documentation string into individual paragraphs.
 *
 * The items of the resulting list are strings that are fred with free().
 *
 * @param[in] doc the documentation string
 * @return a list of allocated strings containing paragraphs
 *
 * @ingroup Eolian
 */
EAPI Eina_List *eolian_documentation_string_split(const char *doc);

/*
 * @brief Tokenize a documentation paragraph.
 *
 * This gradually splits the string into pieces (text, references, paragraph
 * separators etc.) so that it can be more easily turned into a representation
 * you want. On failure, token is initialized with EOLIAN_DOC_TOKEN_UNKNOWN.
 *
 * The function never allocates any memory and doesn't hold any state, instead
 * the returned continuation has to be passed as first param on next iteration
 * and you have to make sure the input data stays valid until you're completely
 * done.
 *
 * The input string is assumed to be a single paragraph with all unnecessary
 * whitespace already trimmed.
 *
 * If the given token is NULL, it will still tokenize, but without saving anything.
 *
 * @param[in] doc the documentation string
 * @param[out] ret the token
 * @return a continuation of the input string
 *
 * @ingroup Eolian
 */
EAPI const char *eolian_documentation_tokenize(const char *doc, Eolian_Doc_Token *ret);

/*
 * @brief Initialize a documentation token into an empty state.
 *
 * @param[in] tok the token
 * @return the token type
 */
EAPI void eolian_doc_token_init(Eolian_Doc_Token *tok);

/*
 * @brief Get the type of a documentation token.
 *
 * @param[in] tok the token
 * @return the token type
 */
EAPI Eolian_Doc_Token_Type eolian_doc_token_type_get(const Eolian_Doc_Token *tok);

/*
 * @brief Get the text of a documentation token.
 *
 * Works on every token type, but for unknown tokens it returns NULL.
 * You need to free the text once you're done using normal free().
 * This makes sure all escapes in the original doc comments are properly
 * removed so you can use the string as-is.
 *
 * @param[in] tok the token
 * @return the token text
 */
EAPI char *eolian_doc_token_text_get(const Eolian_Doc_Token *tok);

/*
 * @brief Get the thing that a reference token references.
 *
 * Returns EOLIAN_DOC_REF_INVALID on failure (when not ref token or
 * invalid ref, but invalid refs don't happen when database is valid).
 *
 * When the reference is a class, alias, struct, enum or var, the first data arg
 * is filled. When it's a func, the first data is class and second data is
 * the respective Eolian_Implement, when it's an event the first data is class
 * and the second data is the event, when it's a struct field or enum field
 * the first data is is the struct/enum and the second data is the field.
 *
 * @param[in] unit the unit to look in
 * @param[in] tok the token
 * @param[out] data the primary data
 * @param[out] data2 the secondary data
 * @return the kind of reference this is
 */
EAPI Eolian_Doc_Ref_Type eolian_doc_token_ref_get(const Eolian_Unit *unit, const Eolian_Doc_Token *tok, const void **data, const void **data2);

#endif

/**
 * @}
 */

#ifdef __cplusplus
} // extern "C" {
#endif

#undef EAPI
#define EAPI

#endif
