#ifndef EOLIAN_H
#define EOLIAN_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EOLIAN_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EOLIAN_BUILD */
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

#ifdef __cplusplus
extern "C" {
#endif

#include <Eina.h>
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef EFL_BETA_API_SUPPORT

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

/* Parameter/return type.
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Type Eolian_Type;

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

typedef enum
{
   EOLIAN_UNRESOLVED,
   EOLIAN_PROPERTY,
   EOLIAN_PROP_SET,
   EOLIAN_PROP_GET,
   EOLIAN_METHOD
} Eolian_Function_Type;

typedef enum
{
   EOLIAN_IN_PARAM,
   EOLIAN_OUT_PARAM,
   EOLIAN_INOUT_PARAM
} Eolian_Parameter_Dir;

typedef enum
{
   EOLIAN_CLASS_UNKNOWN_TYPE,
   EOLIAN_CLASS_REGULAR,
   EOLIAN_CLASS_ABSTRACT,
   EOLIAN_CLASS_MIXIN,
   EOLIAN_CLASS_INTERFACE
} Eolian_Class_Type;

typedef enum
{
   EOLIAN_SCOPE_PUBLIC,
   EOLIAN_SCOPE_PRIVATE,
   EOLIAN_SCOPE_PROTECTED
} Eolian_Object_Scope;

typedef enum
{
   EOLIAN_TYPE_UNKNOWN_TYPE,
   EOLIAN_TYPE_VOID,
   EOLIAN_TYPE_REGULAR,
   EOLIAN_TYPE_REGULAR_STRUCT,
   EOLIAN_TYPE_REGULAR_ENUM,
   EOLIAN_TYPE_POINTER,
   EOLIAN_TYPE_FUNCTION,
   EOLIAN_TYPE_STRUCT,
   EOLIAN_TYPE_STRUCT_OPAQUE,
   EOLIAN_TYPE_ENUM,
   EOLIAN_TYPE_ALIAS,
   EOLIAN_TYPE_CLASS
} Eolian_Type_Type;

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
   EOLIAN_EXPR_ENUM,
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
   EOLIAN_BINOP_INVALID = -1,

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
   EOLIAN_UNOP_INVALID = -1,

   EOLIAN_UNOP_UNM, /* - sint */
   EOLIAN_UNOP_UNP, /* + sint */

   EOLIAN_UNOP_NOT,  /* ! int, float, bool */
   EOLIAN_UNOP_BNOT, /* ~ int */
} Eolian_Unary_Operator;

/*
 * @brief Parse a given .eo file and fill the database.
 *
 * During parsing, the class described into the .eo file is created with
 * all the information related to this class.
 *
 * @param[in] filename Name of the file to parse.
 * @see eolian_eot_file_parse
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_eo_file_parse(const char *filename);

/*
 * @brief Parse a given .eot file and fill the database.
 *
 * @param[in] filename Name of the file to parse.
 * @see eolian_eo_file_parse
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_eot_file_parse(const char *filepath);

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
 * @brief Scan the given directory (recursively) and search for .eo and
 * .eot files.
 *
 * The found files are just open to extract the class name.
 *
 * @param[in] dir the directory to scan
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @see eolian_system_directory_scan
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_directory_scan(const char *dir);

/*
 * @brief Scan the system directory (recursively) and search for .eo and
 * .eot files.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @see eolian_directory_scan
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_system_directory_scan();

/*
 * @brief Force parsing of all the .eo files located in the directories
 * given in eolian_directory_scan..
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @see eolian_directory_scan
 * @see eolian_all_eot_files_parse
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_all_eo_files_parse();

/*
 * @brief Force parsing of all the .eot files located in the directories
 * given in eolian_directory_scan..
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @see eolian_directory_scan
 * @see eolian_all_eo_files_parse
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_all_eot_files_parse();

/*
 * @brief Validates the database, printing errors and warnings.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * Useful to catch type errors etc. early on.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_database_validate(void);

/*
 * @brief Show information about a given class.
 *
 * If klass is NULL, this function will print information of
 * all the classes stored into the database.
 *
 * @param[in] klass the class to show
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise (currently always
 * succeeds).
 *
 * @see eolian_show_typedef
 * @see eolian_show_struct
 * @see eolian_show_enum
 * @see eolian_show_global
 * @see eolian_show_constant
 * @see eolian_show_all
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_show_class(const Eolian_Class *klass);

/*
 * @brief Show information about a given typedef.
 *
 * If @c alias is NULL, this function will print information of
 * all the typedefs.
 *
 * @param[in] alias the typedef to show.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise (when typedef is not
 * found).
 *
 * @see eolian_show_class
 * @see eolian_show_struct
 * @see eolian_show_enum
 * @see eolian_show_global
 * @see eolian_show_constant
 * @see eolian_show_all
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_show_typedef(const char *alias);

/*
 * @brief Show information about a given struct.
 *
 * If @c name is NULL, this function will print information of
 * all the named global structs.
 *
 * @param[in] name the struct to show.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise (when struct is not
 * found).
 *
 * @see eolian_show_class
 * @see eolian_show_typedef
 * @see eolian_show_enum
 * @see eolian_show_global
 * @see eolian_show_constant
 * @see eolian_show_all
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_show_struct(const char *name);

/*
 * @brief Show information about a given enum.
 *
 * If @c name is NULL, this function will print information of
 * all the enums.
 *
 * @param[in] name the enum to show.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise (when enum is not
 * found).
 *
 * @see eolian_show_class
 * @see eolian_show_typedef
 * @see eolian_show_struct
 * @see eolian_show_global
 * @see eolian_show_constant
 * @see eolian_show_all
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_show_enum(const char *name);

/*
 * @brief Show information about a given global.
 *
 * If @c name is NULL, this function will print information of
 * all the globals.
 *
 * @param[in] name the global to show.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise (when global is not
 * found).
 *
 * @see eolian_show_class
 * @see eolian_show_typedef
 * @see eolian_show_struct
 * @see eolian_show_enum
 * @see eolian_show_constant
 * @see eolian_show_all
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_show_global(const char *name);

/*
 * @brief Show information about a given constant.
 *
 * If @c name is NULL, this function will print information of
 * all the constants.
 *
 * @param[in] name the constant to show.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise (when constant is not
 * found).
 *
 * @see eolian_show_class
 * @see eolian_show_typedef
 * @see eolian_show_struct
 * @see eolian_show_enum
 * @see eolian_show_global
 * @see eolian_show_all
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_show_constant(const char *name);

/*
 * @brief Show information about everything.
 *
 * This will print a complete dump of all information stored in the Eolian
 * database.
 *
 * @see eolian_show_class
 * @see eolian_show_typedef
 * @see eolian_show_struct
 * @see eolian_show_enum
 *
 * @ingroup Eolian
 */
EAPI void eolian_show_all();

/*
 * @brief Gets a class by its name
 *
 * @param[in] class_name name of the class to get.
 * @return the class
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_class_get_by_name(const char *class_name);

/*
 * @brief Gets a class by its filename (name.eo)
 *
 * @param[in] file_name the filename
 * @return the class stored in the file
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Class *eolian_class_get_by_file(const char *file_name);

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
 * @return the iterator
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_all_classes_get(void);

/*
 * @brief Returns the description of a class.
 *
 * @param[in] klass the class
 * @return the description of a class
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_description_get(const Eolian_Class *klass);

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
 * @brief Returns the data type of a class
 *
 * @param[in] klass the class
 * @return the data type
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_class_data_type_get(const Eolian_Class *klass);

/*
 * @brief Returns an iterator to the names of inherit classes of a class
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
 * @return the function scope
 *
 * @ingroup Eolian
 */
EAPI Eolian_Object_Scope eolian_function_scope_get(const Eolian_Function *function_id);

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
 * @brief Returns the full C name of a function (with prefix). It's here
 * because the C API names are deduplicated (prefix of function and suffix
 * of prefix merge if applicable) and this helps generators not write the
 * same code over and over.
 *
 * @param[in] function_id Id of the function
 * @return the function name
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_function_full_c_name_get(const Eolian_Function *function_id, const char *prefix);

/*
 * @brief Get a function in a class by its name and type
 *
 * @param[in] klass the class
 * @param[in] func_name name of the function
 * @param[in] f_type type of the function
 * @return the function id if found, NULL otherwise.
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
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_function_legacy_get(const Eolian_Function *function_id, Eolian_Function_Type f_type);

/*
 * @brief Returns a description for a function.
 *
 * @param[in] function_id Id of the function
 * @param[in] f_type The function type, for property get/set distinction.
 * @return the description or NULL.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_function_description_get(const Eolian_Function *function_id, Eolian_Function_Type f_type);

/*
 * @brief Indicates if a function is virtual pure.
 *
 * @param[in] function_id Id of the function
 * @param[in] f_type The function type, for property get/set distinction.
 * @return EINA_TRUE if virtual pure, EINA_FALSE othrewise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_virtual_pure(const Eolian_Function *function_id, Eolian_Function_Type f_type);

/*
 * @brief Indicates if a function is auto.
 *
 * @param[in] function_id Id of the function
 * @param[in] f_type The function type, for property get/set distinction.
 * @return EINA_TRUE if auto, EINA_FALSE othrewise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_auto(const Eolian_Function *function_id, Eolian_Function_Type f_type);

/*
 * @brief Indicates if a function is empty.
 *
 * @param[in] function_id Id of the function
 * @param[in] f_type The function type, for property get/set distinction.
 * @return EINA_TRUE if empty, EINA_FALSE othrewise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_empty(const Eolian_Function *function_id, Eolian_Function_Type f_type);

/*
 * @brief Indicates if a function is legacy only.
 *
 * @param[in] function_id Id of the function
 * @param[in] f_type The function type, for property get/set distinction.
 * @return EINA_TRUE if legacy only, EINA_FALSE otherwise.
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
 * @brief Returns a parameter of a function pointed by its id.
 *
 * @param[in] function_id Id of the function
 * @param[in] param_name Name of the parameter
 * @return a handle to this parameter.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Function_Parameter *eolian_function_parameter_get_by_name(const Eolian_Function *function_id, const char *param_name);

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
 * @return the iterator
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_property_keys_get(const Eolian_Function *foo_id);

/*
 * @brief Returns an iterator to the values params of a given function.
 *
 * @param[in] function_id Id of the function
 * @return the iterator
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_property_values_get(const Eolian_Function *foo_id);

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
 * @brief Get description of a parameter
 *
 * @param[in] param_desc parameter handle
 * @return the description of the parameter or NULL
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_parameter_description_get(const Eolian_Function_Parameter *param);

/*
 * @brief Indicates if a parameter has a const attribute.
 *
 * This function is relevant for properties, to know if a parameter is a const
 * parameter in the get operation.
 *
 * @param[in] param_desc parameter handle
 * @param[in] is_get indicates if the information needed is for get or set.
 * @return EINA_TRUE if const in get, EINA_FALSE otherwise
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_parameter_const_attribute_get(const Eolian_Function_Parameter *param_desc, Eina_Bool is_get);

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
 * @brief Get the return type of a function.
 *
 * @param[in] function_id id of the function
 * @param[in] ftype type of the function
 * @return the return type of the function
 *
 * The type of the function is needed because a given function can represent a
 * property, that can be set and get functions.
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
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *
eolian_function_return_default_value_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);

/*
 * @brief Get the return comment of a function.
 *
 * @param[in] function_id id of the function
 * @param[in] ftype type of the function
 * @return the return comment of the function
 *
 * The type of the function is needed because a given function can represent a
 * property, that can be set and get functions.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_function_return_comment_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);

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
 * @brief Get whether an implement is tagged with @auto.
 *
 * @param[in] impl the handle of the implement
 * @return EINA_TRUE when it is, EINA_FALSE when it's not.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_implement_is_auto(const Eolian_Implement *impl);

/*
 * @brief Get whether an implement is tagged with @empty.
 *
 * @param[in] impl the handle of the implement
 * @return EINA_TRUE when it is, EINA_FALSE when it's not.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_implement_is_empty(const Eolian_Implement *impl);

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
 * file (i.e. overriding and virtual/auto/empty functions) and all other
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
 * @return the iterator
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
 * @brief Get the description of an event.
 *
 * @param[in] event the event handle
 * @return the description or NULL
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_event_description_get(const Eolian_Event *event);

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
 * @brief Get an alias type by name. Supports namespaces.
 *
 * @param[in] name the name of the alias
 * @return the alias type or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_alias_get_by_name(const char *name);

/*
 * @brief Get a struct by name. Supports namespaces.
 *
 * @param[in] name the name of the struct
 * @return the struct or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_struct_get_by_name(const char *name);

/*
 * @brief Get an enum by name. Supports namespaces.
 *
 * @param[in] name the name of the struct
 * @return the struct or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_enum_get_by_name(const char *name);

/*
 * @brief Get an iterator to all aliases contained in a file.
 *
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_aliases_get_by_file(const char *fname);

/*
 * @brief Get an iterator to all named structs contained in a file.
 *
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_structs_get_by_file(const char *fname);

/*
 * @brief Get an iterator to all enums contained in a file.
 *
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_enums_get_by_file(const char *fname);

/*
 * @brief Get the type of a type (regular, function, pointer)
 *
 * @param[in] tp the type.
 * @return an Eolian_Type_Type.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type_Type eolian_type_type_get(const Eolian_Type *tp);

/*
 * @brief Get an iterator to all arguments of a function type.
 *
 * @param[in] tp the type.
 * @return the iterator when @c tp is an EOLIAN_TYPE_FUNCTION, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_arguments_get(const Eolian_Type *tp);

/*
 * @brief Get an iterator to all subtypes of a type.
 *
 * @param[in] tp the type.
 * @return the iterator when @c tp is a regular/regular struct/class/pointer
 * type.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_subtypes_get(const Eolian_Type *tp);

/*
 * @brief Get an iterator to all fields of a struct type.
 *
 * @param[in] tp the type.
 * @return the iterator when @c tp is EOLIAN_TYPE_STRUCT, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_struct_fields_get(const Eolian_Type *tp);

/*
 * @brief Get a field of a struct type.
 *
 * @param[in] tp the type.
 * @param[in] field the field name.
 * @return the field when @c tp is EOLIAN_TYPE_STRUCT, @c field is not NULL
 * and the field exists, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Struct_Type_Field *eolian_type_struct_field_get(const Eolian_Type *tp, const char *field);

/*
 * @brief Get the name of a field of a struct type.
 *
 * @param[in] fl the field.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_struct_field_name_get(const Eolian_Struct_Type_Field *fl);

/*
 * @brief Get the description of a field of a struct type.
 *
 * @param[in] fl the field.
 * @return the description.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_struct_field_description_get(const Eolian_Struct_Type_Field *fl);

/*
 * @brief Get the type of a field of a struct type.
 *
 * @param[in] fl the field.
 * @return the type.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_struct_field_type_get(const Eolian_Struct_Type_Field *fl);

/*
 * @brief Get an iterator to all fields of an enum type.
 *
 * @param[in] tp the type.
 * @return the iterator when @c tp is EOLIAN_TYPE_ENUM, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_enum_fields_get(const Eolian_Type *tp);

/*
 * @brief Get a field of an enum type.
 *
 * @param[in] tp the type.
 * @param[in] field the field name.
 * @return the field when @c tp is EOLIAN_TYPE_ENUM, @c field is not NULL,
 * field exists and has a value set, NULL otherwise.
 *
 * Keep in mind that this can return NULL for an existing field, particularly
 * when the field has no value set (i.e. increments by 1 over previous value).
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Enum_Type_Field *eolian_type_enum_field_get(const Eolian_Type *tp, const char *field);

/*
 * @brief Get the name of a field of an enum type.
 *
 * @param[in] fl the field.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_enum_field_name_get(const Eolian_Enum_Type_Field *fl);

/*
 * @brief Get the description of a field of an enum type.
 *
 * @param[in] fl the field.
 * @return the description.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_enum_field_description_get(const Eolian_Enum_Type_Field *fl);

/*
 * @brief Get the value of a field of an enum type.
 *
 * @param[in] fl the field.
 * @return the description.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Expression *eolian_type_enum_field_value_get(const Eolian_Enum_Type_Field *fl);

/*
 * @brief Get the legacy prefix of enum field names. When not specified,
 * enum name is used.
 *
 * @param[in] tp the type.
 * @return the legacy prefix or NULL.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_enum_legacy_prefix_get(const Eolian_Type *tp);

/*
 * @brief Get the description of a struct/alias type.
 *
 * @param[in] tp the type.
 * @return the description when @c tp is EOLIAN_TYPE_STRUCT or
 * EOLIAN_TYPE_STRUCT_OPAQUE, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_description_get(const Eolian_Type *tp);

/*
 * @brief Get the filename of a struct/alias type.
 *
 * @param[in] tp the type.
 * @return the filename.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_file_get(const Eolian_Type *tp);

/*
 * @brief Get the return type of a function type.
 *
 * @param[in] tp the type.
 * @return the return type when @c tp is an EOLIAN_TYPE_FUNCTION, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_return_type_get(const Eolian_Type *tp);

/*
 * @brief Get the base type of a pointer or alias type.
 *
 * @param[in] tp the type.
 * @return the base type when @c tp is a pointer or alias, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Type *eolian_type_base_type_get(const Eolian_Type *tp);

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
 * @brief Get whether the given type is @own.
 *
 * @param[in] tp the type.
 * @return EINA_TRUE when @c tp is a non-function type and not NULL,
 * EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_type_is_own(const Eolian_Type *tp);

/*
 * @brief Get whether the given type is const.
 *
 * @param[in] tp the type.
 * @return EINA_TRUE when @c tp is a non-function type and not NULL,
 * EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_type_is_const(const Eolian_Type *tp);

/*
 * @brief Check if a struct or alias type is extern.
 *
 * @param[in] tp the type.
 * @return EINA_TRUE if it's extern, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_type_is_extern(const Eolian_Type *tp);

/*
 * @brief Get the full C type name of the given type with a name.
 *
 * @param[in] tp the type.
 * @param[in] name the name.
 * @return The C type name assuming @c tp is not NULL.
 *
 * Providing the name is useful for function types, as in C a function
 * pointer type alone is not valid syntax. For non-function types, the
 * name is simply appended to the type (with a space). C type names do
 * not include subtypes as C doesn't support them. Name is ignored for
 * alias types. Alias types are turned into C typedefs.
 *
 * Keep in mind that if @c name is NULL, the name won't be included.
 *
 * @see eolian_type_c_type_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_c_type_named_get(const Eolian_Type *tp, const char *name);

/*
 * @brief Get the full C type name of the given type without a name.
 *
 * @param[in] tp the type.
 * @return The C type name assuming @c tp is not NULL.
 *
 * This behaves exactly like eolian_type_c_type_named_get when name is NULL.
 * Keep in mind that this is not useful for function types as a function
 * pointer type in C cannot be used without a name.
 *
 * @see eolian_type_c_type_named_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_c_type_get(const Eolian_Type *tp);

/*
 * @brief Get the name of the given type. You have to manually delete
 * the stringshare. For EOLIAN_TYPE_REGULAR and EOLIAN_TYPE_REGULAR_STRUCT,
 * this is for example "int". For EOLIAN_TYPE_STRUCT, EOLIAN_TYPE_STRUCT_OPAQUE
 * and EOLIAN_TYPE_ALIAS, this is the name of the alias or of the struct. For
 * EOLIAN_TYPE_CLASS, this can be "Button". Keep in mind that the name doesn't
 * include namespaces for structs and aliases.
 *
 * @param[in] tp the type.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_name_get(const Eolian_Type *tp);

/*
 * @brief Get the full (namespaced) name of a function. Only works on named
 * types (not pointers, not functions, not void).
 *
 * @param[in] tp the type.
 * @return the name.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_full_name_get(const Eolian_Type *tp);

/*
 * @brief Get an iterator to the list of namespaces of the given type. Only
 * works on named types (not pointers, not functions, not void).
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
 * For pointer types, this returns name of the func used to free the pointer.
 * For struct and alias types, this returns name of the func used to free a
 * pointer to that type. For other types, this returns NULL.
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
 * @param[in] etp the eolian type of the value.
 * @return a stringshare containing the literal (quoted and escaped as needed)
 * or NULL.
 *
 * For e.g. strings this only uses a subset of regular C escape sequences
 * so that interoperability is wider than just C (no octal escapes). For
 * languages that differ too much, you can write an equivalent function
 * yourself.
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
 * @param[in] name the name of the variable
 * @return the variable handle or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Variable *eolian_variable_global_get_by_name(const char *name);

/*
 * @brief Get a constant variable by name. Supports namespaces.
 *
 * @param[in] name the name of the variable
 * @return the variable handle or NULL
 *
 * @ingroup Eolian
 */
EAPI const Eolian_Variable *eolian_variable_constant_get_by_name(const char *name);

/*
 * @brief Get an iterator to all global variables contained in a file.
 *
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_variable_globals_get_by_file(const char *fname);

/*
 * @brief Get an iterator to all constant variables contained in a file.
 *
 * @param[in] fname the file name without full path
 * @return the iterator or NULL
 *
 * Thanks to internal caching, this is an O(1) operation.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_variable_constants_get_by_file(const char *fname);

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
 * @brief Get the description of a variable.
 *
 * @param[in] var the variable.
 * @return the description or NULL.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_variable_description_get(const Eolian_Variable *var);

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

#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
