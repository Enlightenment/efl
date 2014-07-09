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
typedef struct _Eolian_Function* Eolian_Function;

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

/* Event information
 *
 * @ingroup Eolian
 */
typedef struct _Eolian_Event Eolian_Event;

#define EOLIAN_LEGACY "legacy"
#define EOLIAN_LEGACY_GET "legacy_get"
#define EOLIAN_LEGACY_SET "legacy_set"
#define EOLIAN_COMMENT "comment"
#define EOLIAN_COMMENT_SET "comment_set"
#define EOLIAN_COMMENT_GET "comment_get"

typedef enum
{
   EOLIAN_UNRESOLVED,
   EOLIAN_PROPERTY,
   EOLIAN_PROP_SET,
   EOLIAN_PROP_GET,
   EOLIAN_METHOD,
   EOLIAN_CTOR
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
   EOLIAN_SCOPE_PROTECTED
} Eolian_Function_Scope;

typedef enum
{
   EOLIAN_TYPE_UNKNOWN_TYPE,
   EOLIAN_TYPE_VOID,
   EOLIAN_TYPE_REGULAR,
   EOLIAN_TYPE_REGULAR_STRUCT,
   EOLIAN_TYPE_POINTER,
   EOLIAN_TYPE_FUNCTION,
   EOLIAN_TYPE_STRUCT
} Eolian_Type_Type;

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
 * @brief Show information about a given class.
 *
 * If klass is NULL, this function will print information of
 * all the classes stored into the database.
 *
 * @param[in] klass the class to show
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_show(const Eolian_Class *klass);

/*
 * @brief Finds a class by its name
 *
 * @param[in] class_name name of the class to find.
 * @return the class
 *
 * @ingroup Eolian
 */
EAPI Eolian_Class *
eolian_class_find_by_name(const char *class_name);

/*
 * @brief Finds a class by its location (.eo file)
 *
 * @param[in] file_name filename where the class is stored.
 * @return the class stored in the file
 *
 * @ingroup Eolian
 */
EAPI Eolian_Class *
eolian_class_find_by_file(const char *file_name);

/*
 * @brief Returns the name of the file containing the given class.
 *
 * @param[in] klass the class.
 * @return the name of the file on success or NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const char *
eolian_class_file_get(const Eolian_Class *klass);

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
EAPI const char *
eolian_class_full_name_get(const Eolian_Class *klass);

/*
 * @brief Returns the name of the given class.
 *
 * @param[in] class the class.
 * @return the name of the class on success or NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const char *
eolian_class_name_get(const Eolian_Class *klass);

/*
 * @brief Returns the namespaces list of the given class.
 *
 * @param[in] class the class.
 * @return the namespaces list of the class on success or NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *
eolian_class_namespaces_list_get(const Eolian_Class *klass);

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
 * @brief Returns a list of all the classes stored into the database.
 *
 * @return the list
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *eolian_all_classes_list_get(void);

/*
 * @brief Returns the description of a class.
 *
 * @param[in] klass the class
 * @return the description of a class
 *
 * @ingroup Eolian
 */
EAPI const char *eolian_class_description_get(const Eolian_Class *klass);

/*
 * @brief Returns the legacy prefix of a class
 *
 * @param[in] klass the class
 * @return the legacy prefix
 *
 * @ingroup Eolian
 */
EAPI const char *eolian_class_legacy_prefix_get(const Eolian_Class *klass);

/*
 * @brief Returns the eo prefix of a class
 *
 * @param[in] klass the class
 * @return the eo prefix
 *
 * @ingroup Eolian
 */
EAPI const char* eolian_class_eo_prefix_get(const Eolian_Class *klass);

/*
 * @brief Returns the data type of a class
 *
 * @param[in] klass the class
 * @return the data type
 *
 * @ingroup Eolian
 */
EAPI const char*
eolian_class_data_type_get(const Eolian_Class *klass);

/*
 * @brief Returns the names list of the inherit classes of a class
 *
 * @param[in] klass the class
 * @return the list
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *eolian_class_inherits_list_get(const Eolian_Class *klass);

/*
 * @brief Returns a list of functions of a class.
 *
 * @param[in] klass the class
 * @param[in] func_type type of the functions to insert into the list.
 * @return the list of Eolian_Function
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *eolian_class_functions_list_get(const Eolian_Class *klass, Eolian_Function_Type func_type);

/*
 * @brief Returns the type of a function
 *
 * @param[in] function_id Id of the function
 * @return the function type
 *
 * @ingroup Eolian
 */
EAPI Eolian_Function_Type eolian_function_type_get(Eolian_Function function_id);

/*
 * @brief Returns the scope of a function
 *
 * @param[in] function_id Id of the function
 * @return the function scope
 *
 * @ingroup Eolian
 */
EAPI Eolian_Function_Scope eolian_function_scope_get(Eolian_Function function_id);

/*
 * @brief Returns the name of a function
 *
 * @param[in] function_id Id of the function
 * @return the function name
 *
 * @ingroup Eolian
 */
EAPI const char *eolian_function_name_get(Eolian_Function function_id);

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
EAPI const char *eolian_function_full_c_name_get(Eolian_Function function_id, const char *prefix);

/*
 * @brief Find a function in a class by its name and type
 *
 * @param[in] klass the class
 * @param[in] func_name name of the function
 * @param[in] f_type type of the function
 * @return the function id if found, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Function eolian_class_function_find_by_name(const Eolian_Class *klass, const char *func_name, Eolian_Function_Type f_type);

/*
 * @brief Returns a specific data for a function.
 *
 * @param[in] function_id Id of the function
 * @param[in] key key to access the data
 * @return the data.
 *
 * @ingroup Eolian
 */
EAPI const char *eolian_function_data_get(Eolian_Function function_id, const char *key);

/*
 * @brief Indicates if a function is virtual pure.
 *
 * @param[in] function_id Id of the function
 * @return EINA_TRUE if virtual pure, EINA_FALSE othrewise..
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_is_virtual_pure(Eolian_Function function_id, Eolian_Function_Type f_type);

/*
 * @brief Returns a specific description for a function.
 *
 * @param[in] function_id Id of the function
 * @param[in] key key to access the description
 * @return the description.
 *
 * @ingroup Eolian
 */
#define eolian_function_description_get(function_id, key) eolian_function_data_get((function_id), (key))

/*
 * @brief Returns a parameter of a function pointed by its id.
 *
 * @param[in] function_id Id of the function
 * @param[in] param_name Name of the parameter
 * @return a handle to this parameter.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Function_Parameter *eolian_function_parameter_get(const Eolian_Function function_id, const char *param_name);

/*
 * @brief Returns a list of keys params of a given function.
 *
 * @param[in] function_id Id of the function
 * @return list of Eolian_Function_Parameter*
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *eolian_property_keys_list_get(Eolian_Function foo_id);

/*
 * @brief Returns a list of values params of a given function.
 *
 * @param[in] function_id Id of the function
 * @return list of Eolian_Function_Parameter*
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *eolian_property_values_list_get(Eolian_Function foo_id);

/*
 * @brief Returns a list of parameter handles for a method/ctor/dtor.
 *
 * @param[in] function_id Id of the function
 * @return list of Eolian_Function_Parameter*
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *eolian_parameters_list_get(Eolian_Function function_id);

/*
 * @brief Get information about a function parameter
 *
 * @param[in] param_desc parameter handle
 * @param[out] param_dir in/out/inout
 * @param[out] type type of the parameter
 * @param[out] name name of the parameter
 * @param[out] description description of the parameter
 *
 * @ingroup Eolian
 */
EAPI void eolian_parameter_information_get(const Eolian_Function_Parameter *param_desc, Eolian_Parameter_Dir *param_dir, Eolian_Type **type, const char **name, const char **description);

/*
 * @brief Get type of a parameter
 *
 * @param[in] param_desc parameter handle
 * @return the type of the parameter
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type *eolian_parameter_type_get(const Eolian_Function_Parameter *param);

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
EAPI Eolian_Type *eolian_function_return_type_get(Eolian_Function function_id, Eolian_Function_Type ftype);

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
EAPI const char *
eolian_function_return_dflt_value_get(Eolian_Function foo_id, Eolian_Function_Type ftype);

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
EAPI const char *
eolian_function_return_comment_get(Eolian_Function foo_id, Eolian_Function_Type ftype);

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
EAPI Eina_Bool eolian_function_return_is_warn_unused(Eolian_Function foo_id, Eolian_Function_Type ftype);

/*
 * @brief Indicates if a function object is const.
 *
 * @param[in] function_id id of the function
 * @return EINA_TRUE if the object is const, EINA_FALSE otherwise
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_function_object_is_const(Eolian_Function function_id);

/*
 * @brief Get full string of an overriding function (implement).
 *
 * @param[in] impl handle of the implement
 * @return the full string.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare * eolian_implement_full_name_get(const Eolian_Implement *impl);

/*
 * @brief Get information about an overriding function (implement).
 *
 * @param[in] impl handle of the implement
 * @param[out] class overridden class
 * @param[out] func overridden function
 * @param[out] type type of the overridden function
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_implement_information_get(const Eolian_Implement *impl,
      Eolian_Class **klass, Eolian_Function *function, Eolian_Function_Type *type);

/*
 * @brief Get the list of overriding functions defined in a class.
 *
 * @param[in] klass the class.
 * @return a list of Eolian_Implement* handles
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *eolian_class_implements_list_get(const Eolian_Class *klass);

/*
 * @brief Get the list of events defined in a class.
 *
 * @param[in] klass the class.
 * @return a list of Eolian_Event* handles
 *
 * @ingroup Eolian
 */
EAPI const Eina_List *eolian_class_events_list_get(const Eolian_Class *klass);

/*
 * @brief Get information about an event.
 *
 * @param[in] event handle of the event
 * @param[out] event_name name of the event
 * @param[out] event_desc description of the event
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_class_event_information_get(const Eolian_Event *event, const char **event_name, const char **event_type, const char **event_desc);

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
 * @brief Find the type for a certain alias
 *
 * @param[in] alias alias of the type definition
 * @return real type of the type definition
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type *eolian_type_find_by_alias(const char *alias);

/*
 * @brief Find a struct by name
 *
 * @param[in] name the name of the struct
 * @return the struct or NULL
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type *eolian_type_struct_find_by_name(const char *name);

/*
 * @brief Get the type of a type (regular, function, pointer)
 *
 * @param[in] tp the type.
 * @return EOLIAN_TYPE_UNKNOWN_TYPE when @c tp is NULL, otherwise
 * EOLIAN_TYPE_REGULAR, EOLIAN_TYPE_POINTER or EOLIAN_TYPE_FUNCTION.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type_Type eolian_type_type_get(Eolian_Type *tp);

/*
 * @brief Get an iterator to all arguments of a function type.
 *
 * @param[in] tp the type.
 * @return the iterator when @c tp is an EOLIAN_TYPE_FUNCTION, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_arguments_list_get(Eolian_Type *tp);

/*
 * @brief Get an iterator to all subtypes of a type.
 *
 * @param[in] tp the type.
 * @return the iterator when @c tp is an EOLIAN_TYPE_REGULAR or
 * EOLIAN_TYPE_POINTER and has subtypes, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_subtypes_list_get(Eolian_Type *tp);

/*
 * @brief Get an iterator to all field names of a struct type.
 *
 * @param[in] tp the type.
 * @return the iterator when @c tp is EOLIAN_TYPE_STRUCT, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Iterator *eolian_type_struct_field_names_list_get(Eolian_Type *tp);

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
EAPI Eolian_Type *eolian_type_struct_field_get(Eolian_Type *tp, const char *field);

/*
 * @brief Get the description of a field of a struct type.
 *
 * @param[in] tp the type.
 * @param[in] field the field name.
 * @return the description when @c tp is EOLIAN_TYPE_STRUCT, @c field is not NULL
 * and the field exists, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const char *eolian_type_struct_field_description_get(Eolian_Type *tp, const char *field);

/*
 * @brief Get the description of a struct type.
 *
 * @param[in] tp the type.
 * @return the description when @c tp is EOLIAN_TYPE_STRUCT, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI const char *eolian_type_struct_description_get(Eolian_Type *tp);

/*
 * @brief Get the return type of a function type.
 *
 * @param[in] tp the type.
 * @return the return type when @c tp is an EOLIAN_TYPE_FUNCTION, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type *eolian_type_return_type_get(Eolian_Type *tp);

/*
 * @brief Get the base type of a function type.
 *
 * @param[in] tp the type.
 * @return the base type when @c tp is an EOLIAN_TYPE_POINTER, NULL otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eolian_Type *eolian_type_base_type_get(Eolian_Type *tp);

/*
 * @brief Get whether the given type is @own.
 *
 * @param[in] tp the type.
 * @return EINA_TRUE when @c tp is a non-function type and not NULL,
 * EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_type_is_own(Eolian_Type *tp);

/*
 * @brief Get whether the given type is const.
 *
 * @param[in] tp the type.
 * @return EINA_TRUE when @c tp is a non-function type and not NULL,
 * EINA_FALSE otherwise.
 *
 * @ingroup Eolian
 */
EAPI Eina_Bool eolian_type_is_const(Eolian_Type *tp);

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
 * not include subtypes as C doesn't support them.
 *
 * Keep in mind that if @c name is NULL, the name won't be included.
 *
 * @see eolian_type_c_type_get
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_c_type_named_get(Eolian_Type *tp, const char *name);

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
EAPI Eina_Stringshare *eolian_type_c_type_get(Eolian_Type *tp);

/*
 * @brief Get the type name of the given type. You have to manually delete
 * the stringshare.
 *
 * @param[in] tp the type.
 * @return the name assuming @c tp is an EOLIAN_TYPE_REGULAR, NULL otherwise.
 * The name may include a "struct" keyword.
 *
 * @ingroup Eolian
 */
EAPI Eina_Stringshare *eolian_type_name_get(Eolian_Type *tp);

#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#endif
