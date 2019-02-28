-- EFL LuaJIT bindings: Eolian
-- For use with Elua

local ffi = require("ffi")
local bit = require("bit")

ffi.cdef [[
    void eina_stringshare_del(const char *str);
    void free(void *ptr);
]]

ffi.cdef [[
    typedef unsigned char Eina_Bool;
    typedef struct _Eina_Iterator Eina_Iterator;

    typedef struct _Eolian_State Eolian_State;
    typedef struct _Eolian_Object Eolian_Object;
    typedef struct _Eolian_Class Eolian_Class;
    typedef struct _Eolian_Function Eolian_Function;
    typedef struct _Eolian_Type Eolian_Type;
    typedef struct _Eolian_Typedecl Eolian_Typedecl;
    typedef struct _Eolian_Function_Parameter Eolian_Function_Parameter;
    typedef struct _Eolian_Implement Eolian_Implement;
    typedef struct _Eolian_Constructor Eolian_Constructor;
    typedef struct _Eolian_Event Eolian_Event;
    typedef struct _Eolian_Expression Eolian_Expression;
    typedef struct _Eolian_Variable Eolian_Variable;
    typedef struct _Eolian_Struct_Type_Field Eolian_Struct_Type_Field;
    typedef struct _Eolian_Enum_Type_Field Eolian_Enum_Type_Field;
    typedef struct _Eolian_Documentation Eolian_Documentation;
    typedef struct _Eolian_Value Eolian_Value;
    typedef struct _Eolian_Unit Eolian_Unit;

    typedef enum {
        EOLIAN_OBJECT_UNKNOWN = 0,
        EOLIAN_OBJECT_CLASS,
        EOLIAN_OBJECT_TYPEDECL,
        EOLIAN_OBJECT_STRUCT_FIELD,
        EOLIAN_OBJECT_ENUM_FIELD,
        EOLIAN_OBJECT_TYPE,
        EOLIAN_OBJECT_VARIABLE,
        EOLIAN_OBJECT_EXPRESSION,
        EOLIAN_OBJECT_FUNCTION,
        EOLIAN_OBJECT_FUNCTION_PARAMETER,
        EOLIAN_OBJECT_EVENT,
        EOLIAN_OBJECT_PART,
        EOLIAN_OBJECT_IMPLEMENT,
        EOLIAN_OBJECT_CONSTRUCTOR,
        EOLIAN_OBJECT_DOCUMENTATION
    } Eolian_Object_Type;

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

    typedef enum {
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

        EOLIAN_TYPE_BUILTIN_SLICE,
        EOLIAN_TYPE_BUILTIN_RW_SLICE,

        EOLIAN_TYPE_BUILTIN_VOID,

        EOLIAN_TYPE_BUILTIN_ACCESSOR,
        EOLIAN_TYPE_BUILTIN_ARRAY,
        EOLIAN_TYPE_BUILTIN_FUTURE,
        EOLIAN_TYPE_BUILTIN_ITERATOR,
        EOLIAN_TYPE_BUILTIN_HASH,
        EOLIAN_TYPE_BUILTIN_LIST,
        EOLIAN_TYPE_BUILTIN_INARRAY,
        EOLIAN_TYPE_BUILTIN_INLIST,

        EOLIAN_TYPE_BUILTIN_ANY_VALUE,
        EOLIAN_TYPE_BUILTIN_ANY_VALUE_PTR,

        EOLIAN_TYPE_BUILTIN_MSTRING,
        EOLIAN_TYPE_BUILTIN_STRING,
        EOLIAN_TYPE_BUILTIN_STRINGSHARE,
        EOLIAN_TYPE_BUILTIN_STRBUF,

        EOLIAN_TYPE_BUILTIN_VOID_PTR,
        EOLIAN_TYPE_BUILTIN_FREE_CB
    } Eolian_Type_Builtin_Type;

    typedef enum {
        EOLIAN_C_TYPE_DEFAULT = 0,
        EOLIAN_C_TYPE_PARAM,
        EOLIAN_C_TYPE_RETURN
    } Eolian_C_Type_Type;

    typedef enum {
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

    typedef enum {
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

    typedef enum {
        EOLIAN_VAR_UNKNOWN = 0,
        EOLIAN_VAR_CONSTANT,
        EOLIAN_VAR_GLOBAL
    } Eolian_Variable_Type;

    typedef union {
        char               c;
        Eina_Bool          b;
        const char        *s;
        signed int         i;
        unsigned int       u;
        signed long        l;
        unsigned long      ul;
        signed long long   ll;
        unsigned long long ull;
        float              f;
        double             d;
    } Eolian_Value_Union;

    typedef struct _Eolian_Value_t {
        Eolian_Expression_Type type;
        Eolian_Value_Union value;
    } Eolian_Value_t;

    typedef enum {
        EOLIAN_BINOP_INVALID = 0,

        EOLIAN_BINOP_ADD, /* + int, float */
        EOLIAN_BINOP_SUB, /* - int, float */
        EOLIAN_BINOP_MUL, /* * int, float */
        EOLIAN_BINOP_DIV, /* / int, float */
        EOLIAN_BINOP_MOD, /* % int */

        EOLIAN_BINOP_EQ, /* == all types */
        EOLIAN_BINOP_NQ, /* != all types */
        EOLIAN_BINOP_GT, /* > int, float */
        EOLIAN_BINOP_LT, /* < int, float */
        EOLIAN_BINOP_GE, /* >= int, float */
        EOLIAN_BINOP_LE, /* <= int, float */

        EOLIAN_BINOP_AND, /* && all types */
        EOLIAN_BINOP_OR, /* || all types */

        EOLIAN_BINOP_BAND, /* & int */
        EOLIAN_BINOP_BOR, /* | int */
        EOLIAN_BINOP_BXOR, /* ^ int */
        EOLIAN_BINOP_LSH, /* << int */
        EOLIAN_BINOP_RSH /* >> int */
    } Eolian_Binary_Operator;

    typedef enum {
        EOLIAN_UNOP_INVALID = 0,

        EOLIAN_UNOP_UNM, /* - sint */
        EOLIAN_UNOP_UNP, /* + sint */

        EOLIAN_UNOP_NOT, /* ! int, float, bool */
        EOLIAN_UNOP_BNOT, /* ~ int */
    } Eolian_Unary_Operator;

    typedef enum {
        EOLIAN_DOC_TOKEN_UNKNOWN = 0,
        EOLIAN_DOC_TOKEN_TEXT,
        EOLIAN_DOC_TOKEN_REF,
        EOLIAN_DOC_TOKEN_MARK_NOTE,
        EOLIAN_DOC_TOKEN_MARK_WARNING,
        EOLIAN_DOC_TOKEN_MARK_REMARK,
        EOLIAN_DOC_TOKEN_MARK_TODO,
        EOLIAN_DOC_TOKEN_MARKUP_MONOSPACE
    } Eolian_Doc_Token_Type;

    typedef struct _Eolian_Doc_Token {
        Eolian_Doc_Token_Type type;
        const char *text, *text_end;
    } Eolian_Doc_Token;

    int eolian_init(void);
    int eolian_shutdown(void);
    Eolian_State *eolian_state_new(void);
    void eolian_state_free(Eolian_State *state);
    Eolian_Object_Type eolian_object_type_get(const Eolian_Object *obj);
    const char *eolian_object_file_get(const Eolian_Object *obj);
    int eolian_object_line_get(const Eolian_Object *obj);
    int eolian_object_column_get(const Eolian_Object *obj);
    const char *eolian_object_name_get(const Eolian_Object *obj);
    const char *eolian_object_short_name_get(const Eolian_Object *obj);
    Eina_Iterator *eolian_object_namespaces_get(const Eolian_Object *obj);
    Eina_Bool eolian_state_directory_add(Eolian_State *state, const char *dir);
    Eina_Bool eolian_state_system_directory_add(Eolian_State *state);
    Eina_Iterator *eolian_state_eo_file_paths_get(const Eolian_State *state);
    Eina_Iterator *eolian_state_eot_file_paths_get(const Eolian_State *state);
    Eina_Iterator *eolian_state_eo_files_get(const Eolian_State *state);
    Eina_Iterator *eolian_state_eot_files_get(const Eolian_State *state);
    const Eolian_Unit *eolian_state_file_parse(Eolian_State *state, const char *filepath);
    Eina_Bool eolian_state_all_eo_files_parse(Eolian_State *state);
    Eina_Bool eolian_state_all_eot_files_parse(Eolian_State *state);
    Eina_Bool eolian_state_check(const Eolian_State *state);
    const Eolian_Unit *eolian_state_unit_by_file_get(const Eolian_State *state, const char *file_name);
    Eina_Iterator *eolian_state_units_get(const Eolian_State *state);

    Eina_Iterator *eolian_unit_children_get(const Eolian_Unit *unit);
    const char *eolian_unit_file_get(const Eolian_Unit *unit);
    const Eolian_Object *eolian_unit_object_by_name_get(const Eolian_Unit *unit, const char *name);
    Eina_Iterator *eolian_unit_objects_get(const Eolian_Unit *unit);
    const Eolian_Class *eolian_unit_class_by_name_get(const Eolian_Unit *unit, const char *class_name);
    Eina_Iterator *eolian_unit_classes_get(const Eolian_Unit *unit);
    const Eolian_Variable *eolian_unit_global_by_name_get(const Eolian_Unit *unit, const char *name);
    const Eolian_Variable *eolian_unit_constant_by_name_get(const Eolian_Unit *unit, const char *name);
    Eina_Iterator *eolian_unit_constants_get(const Eolian_Unit *unit);
    Eina_Iterator *eolian_unit_globals_get(const Eolian_Unit *unit);
    const Eolian_Typedecl *eolian_unit_alias_by_name_get(const Eolian_Unit *unit, const char *name);
    const Eolian_Typedecl *eolian_unit_struct_by_name_get(const Eolian_Unit *unit, const char *name);
    const Eolian_Typedecl *eolian_unit_enum_by_name_get(const Eolian_Unit *unit, const char *name);
    Eina_Iterator *eolian_unit_aliases_get(const Eolian_Unit *unit);
    Eina_Iterator *eolian_unit_structs_get(const Eolian_Unit *unit);
    Eina_Iterator *eolian_unit_enums_get(const Eolian_Unit *unit);
    Eina_Iterator *eolian_state_objects_by_file_get(const Eolian_State *state, const char *file_name);
    const Eolian_Class *eolian_state_class_by_file_get(const Eolian_State *state, const char *file_name);
    Eina_Iterator *eolian_state_globals_by_file_get(const Eolian_State *state, const char *file_name);
    Eina_Iterator *eolian_state_constants_by_file_get(const Eolian_State *state, const char *file_name);
    Eina_Iterator *eolian_state_aliases_by_file_get(const Eolian_State *state, const char *file_name);
    Eina_Iterator *eolian_state_structs_by_file_get(const Eolian_State *state, const char *file_name);
    Eina_Iterator *eolian_state_enums_by_file_get(const Eolian_State *state, const char *file_name);

    Eolian_Class_Type eolian_class_type_get(const Eolian_Class *klass);
    const Eolian_Documentation *eolian_class_documentation_get(const Eolian_Class *klass);
    const char *eolian_class_legacy_prefix_get(const Eolian_Class *klass);
    const char *eolian_class_eo_prefix_get(const Eolian_Class *klass);
    const char *eolian_class_data_type_get(const Eolian_Class *klass);
    const Eolian_Class *eolian_class_parent_get(const Eolian_Class *klass);
    Eina_Iterator *eolian_class_extensions_get(const Eolian_Class *klass);
    Eina_Iterator *eolian_class_functions_get(const Eolian_Class *klass, Eolian_Function_Type func_type);
    Eolian_Function_Type eolian_function_type_get(const Eolian_Function *function_id);
    Eolian_Object_Scope eolian_function_scope_get(const Eolian_Function *function_id, Eolian_Function_Type ftype);
    const char *eolian_function_full_c_name_get(const Eolian_Function *function_id, Eolian_Function_Type ftype, Eina_Bool use_legacy);
    const Eolian_Function *eolian_class_function_by_name_get(const Eolian_Class *klass, const char *func_name, Eolian_Function_Type f_type);
    const char *eolian_function_legacy_get(const Eolian_Function *function_id, Eolian_Function_Type f_type);
    const Eolian_Implement *eolian_function_implement_get(const Eolian_Function *function_id);
    Eina_Bool eolian_function_is_legacy_only(const Eolian_Function *function_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_is_class(const Eolian_Function *function_id);
    Eina_Bool eolian_function_is_beta(const Eolian_Function *function_id);
    Eina_Bool eolian_function_is_constructor(const Eolian_Function *function_id, const Eolian_Class *klass);
    Eina_Bool eolian_function_is_function_pointer(const Eolian_Function *function_id);
    Eina_Iterator *eolian_property_keys_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Iterator *eolian_property_values_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Iterator *eolian_function_parameters_get(const Eolian_Function *function_id);
    Eolian_Parameter_Dir eolian_parameter_direction_get(const Eolian_Function_Parameter *param);
    const Eolian_Type *eolian_parameter_type_get(const Eolian_Function_Parameter *param);
    const Eolian_Expression *eolian_parameter_default_value_get(const Eolian_Function_Parameter *param);
    const Eolian_Documentation *eolian_parameter_documentation_get(const Eolian_Function_Parameter *param);
    Eina_Bool eolian_parameter_is_nonull(const Eolian_Function_Parameter *param_desc);
    Eina_Bool eolian_parameter_is_nullable(const Eolian_Function_Parameter *param_desc);
    Eina_Bool eolian_parameter_is_optional(const Eolian_Function_Parameter *param_desc);
    const Eolian_Type *eolian_function_return_type_get(const Eolian_Function *function_id, Eolian_Function_Type ftype);
    const Eolian_Expression *eolian_function_return_default_value_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    const Eolian_Documentation *eolian_function_return_documentation_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_return_is_warn_unused(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_object_is_const(const Eolian_Function *function_id);
    const Eolian_Class *eolian_implement_class_get(const Eolian_Implement *impl);
    const Eolian_Class *eolian_implement_implementing_class_get(const Eolian_Implement *impl);
    const Eolian_Function *eolian_implement_function_get(const Eolian_Implement *impl, Eolian_Function_Type *func_type);
    const Eolian_Documentation *eolian_implement_documentation_get(const Eolian_Implement *impl, Eolian_Function_Type f_type);
    Eina_Bool eolian_implement_is_auto(const Eolian_Implement *impl, Eolian_Function_Type ftype);
    Eina_Bool eolian_implement_is_empty(const Eolian_Implement *impl, Eolian_Function_Type ftype);
    Eina_Bool eolian_implement_is_pure_virtual(const Eolian_Implement *impl, Eolian_Function_Type ftype);
    Eina_Bool eolian_implement_is_prop_get(const Eolian_Implement *impl);
    Eina_Bool eolian_implement_is_prop_set(const Eolian_Implement *impl);
    Eina_Iterator *eolian_class_implements_get(const Eolian_Class *klass);
    const Eolian_Class *eolian_constructor_class_get(const Eolian_Constructor *ctor);
    const Eolian_Function *eolian_constructor_function_get(const Eolian_Constructor *ctor);
    Eina_Bool eolian_constructor_is_optional(const Eolian_Constructor *ctor);
    Eina_Iterator *eolian_class_constructors_get(const Eolian_Class *klass);
    Eina_Iterator *eolian_class_events_get(const Eolian_Class *klass);
    const Eolian_Type *eolian_event_type_get(const Eolian_Event *event);
    const Eolian_Class *eolian_event_class_get(const Eolian_Event *event);
    const Eolian_Documentation *eolian_event_documentation_get(const Eolian_Event *event);
    Eolian_Object_Scope eolian_event_scope_get(const Eolian_Event *event);
    Eina_Bool eolian_event_is_beta(const Eolian_Event *event);
    Eina_Bool eolian_event_is_hot(const Eolian_Event *event);
    Eina_Bool eolian_event_is_restart(const Eolian_Event *event);
    const char *eolian_event_c_name_get(const Eolian_Event *event);
    Eina_Bool eolian_class_ctor_enable_get(const Eolian_Class *klass);
    Eina_Bool eolian_class_dtor_enable_get(const Eolian_Class *klass);
    const char *eolian_class_c_get_function_name_get(const Eolian_Class *klass);
    Eolian_Type_Type eolian_type_type_get(const Eolian_Type *tp);
    Eolian_Type_Builtin_Type eolian_type_builtin_type_get(const Eolian_Type *tp);
    Eolian_Typedecl_Type eolian_typedecl_type_get(const Eolian_Typedecl *tp);
    Eina_Iterator *eolian_typedecl_struct_fields_get(const Eolian_Typedecl *tp);
    const Eolian_Struct_Type_Field *eolian_typedecl_struct_field_get(const Eolian_Typedecl *tp, const char *field);
    const Eolian_Documentation *eolian_typedecl_struct_field_documentation_get(const Eolian_Struct_Type_Field *fl);
    const Eolian_Type *eolian_typedecl_struct_field_type_get(const Eolian_Struct_Type_Field *fl);
    Eina_Iterator *eolian_typedecl_enum_fields_get(const Eolian_Typedecl *tp);
    const Eolian_Enum_Type_Field *eolian_typedecl_enum_field_get(const Eolian_Typedecl *tp, const char *field);
    const char *eolian_typedecl_enum_field_c_name_get(const Eolian_Enum_Type_Field *fl);
    const Eolian_Documentation *eolian_typedecl_enum_field_documentation_get(const Eolian_Enum_Type_Field *fl);
    const Eolian_Expression *eolian_typedecl_enum_field_value_get(const Eolian_Enum_Type_Field *fl, Eina_Bool force);

    const char *eolian_typedecl_enum_legacy_prefix_get(const Eolian_Typedecl *tp);
    const Eolian_Documentation *eolian_typedecl_documentation_get(const Eolian_Typedecl *tp);

    const Eolian_Type *eolian_type_base_type_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_type_next_type_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_typedecl_base_type_get(const Eolian_Typedecl *tp);
    const Eolian_Typedecl *eolian_type_typedecl_get(const Eolian_Type *tp);

    const Eolian_Type *eolian_type_aliased_base_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_typedecl_aliased_base_get(const Eolian_Typedecl *tp);

    const Eolian_Class *eolian_type_class_get(const Eolian_Type *tp);
    Eina_Bool eolian_type_is_owned(const Eolian_Type *tp);
    Eina_Bool eolian_type_is_const(const Eolian_Type *tp);
    Eina_Bool eolian_type_is_ptr(const Eolian_Type *tp);

    Eina_Bool eolian_typedecl_is_extern(const Eolian_Typedecl *tp);

    const char *eolian_type_c_type_get(const Eolian_Type *tp, Eolian_C_Type_Type ctype);
    const char *eolian_typedecl_c_type_get(const Eolian_Typedecl *tp);

    const char *eolian_type_free_func_get(const Eolian_Type *tp);
    const char *eolian_typedecl_free_func_get(const Eolian_Typedecl *tp);

    const Eolian_Function *eolian_typedecl_function_pointer_get(const Eolian_Typedecl *tp);

    Eolian_Value_t eolian_expression_eval(const Eolian_Expression *expr, Eolian_Expression_Mask m);
    Eolian_Value_t eolian_expression_eval_type(const Eolian_Expression *expr, const Eolian_Type *type);
    const char *eolian_expression_value_to_literal(const Eolian_Value *v);
    const char *eolian_expression_serialize(const Eolian_Expression *expr);
    Eolian_Expression_Type eolian_expression_type_get(const Eolian_Expression *expr);
    Eolian_Binary_Operator eolian_expression_binary_operator_get(const Eolian_Expression *expr);
    const Eolian_Expression *eolian_expression_binary_lhs_get(const Eolian_Expression *expr);
    const Eolian_Expression *eolian_expression_binary_rhs_get(const Eolian_Expression *expr);
    Eolian_Unary_Operator eolian_expression_unary_operator_get(const Eolian_Expression *expr);
    const Eolian_Expression *eolian_expression_unary_expression_get(const Eolian_Expression *expr);
    Eolian_Value_t eolian_expression_value_get(const Eolian_Expression *expr);
    Eolian_Variable_Type eolian_variable_type_get(const Eolian_Variable *var);
    const Eolian_Documentation *eolian_variable_documentation_get(const Eolian_Variable *var);
    const Eolian_Type *eolian_variable_base_type_get(const Eolian_Variable *var);
    const Eolian_Expression *eolian_variable_value_get(const Eolian_Variable *var);
    Eina_Bool eolian_variable_is_extern(const Eolian_Variable *var);
    const char *eolian_documentation_summary_get(const Eolian_Documentation *doc);
    const char *eolian_documentation_description_get(const Eolian_Documentation *doc);
    const char *eolian_documentation_since_get(const Eolian_Documentation *doc);

    const char *eolian_documentation_tokenize(const char *doc, Eolian_Doc_Token *ret);
    void eolian_doc_token_init(Eolian_Doc_Token *tok);
    Eolian_Doc_Token_Type eolian_doc_token_type_get(const Eolian_Doc_Token *tok);
    char *eolian_doc_token_text_get(const Eolian_Doc_Token *tok);
    Eolian_Object_Type eolian_doc_token_ref_resolve(const Eolian_Doc_Token *tok, const Eolian_State *state, const Eolian_Object **data, const Eolian_Object **data2);
]]

local cutil = require("cutil")
local util  = require("util")

local iterator = require("eina.iterator")

local Ptr_Iterator = iterator.Ptr_Iterator

local M = {}

local eolian
local eina

local init = function()
    eolian = util.lib_load("eolian")
    eina = util.lib_load("eina")
    eolian.eolian_init()
end

local shutdown = function()
    eolian.eolian_shutdown()
    util.lib_unload("eolian")
    util.lib_unload("eina")
end

local ffi_stringshare = function(s)
    local r = ffi.string(s)
    eina.eina_stringshare_del(s)
    return r
end

cutil.init_module(init, shutdown)

M.object_type = {
    UNKNOWN            = 0,
    CLASS              = 1,
    TYPEDECL           = 2,
    STRUCT_FIELD       = 3,
    ENUM_FIELD         = 4,
    TYPE               = 5,
    VARIABLE           = 6,
    EXPRESSION         = 7,
    FUNCTION           = 8,
    FUNCTION_PARAMETER = 9,
    EVENT              = 10,
    PART               = 11,
    IMPLEMENT          = 12,
    CONSTRUCTOR        = 13,
    DOCUMENTATION      = 14
}

M.object_scope = {
    UNKNOWN   = 0,
    PUBLIC    = 1,
    PRIVATE   = 2,
    PROTECTED = 3
}

local cast_obj = function(self)
    return ffi.cast("const Eolian_Object *", self)
end

local cast_unit = function(self)
    return ffi.cast("const Eolian_Unit *", self)
end

local gen_wrap = function(t)
    return t, function(tt)
        return setmetatable(tt, { __index = t })
    end
end

local object_idx, wrap_object = gen_wrap {
    object_get = function(self)
        return cast_obj(self)
    end,

    type_get = function(self)
        return tonumber(eolian.eolian_object_type_get(cast_obj(self)))
    end,

    line_get = function(self)
        return tonumber(eolian.eolian_object_line_get(cast_obj(self)))
    end,

    column_get = function(self)
        return tonumber(eolian.eolian_object_column_get(cast_obj(self)))
    end,

    file_get = function(self)
        local v = eolian.eolian_object_file_get(cast_obj(self))
        if v == nil then
            return nil
        end
        return ffi.string(v)
    end,

    name_get = function(self)
        local v = eolian.eolian_object_name_get(cast_obj(self))
        if v == nil then
            return nil
        end
        return ffi.string(v)
    end,

    short_name_get = function(self)
        local v = eolian.eolian_object_short_name_get(cast_obj(self))
        if v == nil then
            return nil
        end
        return ffi.string(v)
    end,

    namespaces_get = function(self)
        return iterator.String_Iterator(
            eolian.eolian_object_namespaces_get(cast_obj(self)))
    end
}

ffi.metatype("Eolian_Object", { __index = object_idx })

local unit_idx, wrap_unit = gen_wrap {
    children_get = function(self)
        return Ptr_Iterator("const Eolian_Unit*",
            eolian.eolian_unit_children_get(cast_unit(self)))
    end,

    file_get = function(self)
        local v = eolian.eolian_unit_file_get(cast_unit(self))
        if v == nil then return nil end
        return ffi.string(v)
    end,

    object_by_name_get = function(self, name)
        local v = eolian.eolian_unit_object_by_name_get(cast_unit(self), name)
        if v == nil then return nil end
        return v
    end,

    objects_get = function(self, fname)
        return Ptr_Iterator("const Eolian_Object *",
            eolian.eolian_unit_objects_get(cast_unit(self), fname))
    end,

    class_by_name_get = function(self, cname)
        local v = eolian.eolian_unit_class_by_name_get(cast_unit(self), cname)
        if v == nil then return nil end
        return v
    end,

    classes_get = function(self)
        return Ptr_Iterator("const Eolian_Class*",
            eolian.eolian_unit_classes_get(cast_unit(self)))
    end,

    global_by_name_get = function(self, name)
        local v = eolian.eolian_unit_global_by_name_get(cast_unit(self), name)
        if v == nil then return nil end
        return v
    end,

    constant_by_name_get = function(self, name)
        local v = eolian.eolian_unit_constant_by_name_get(cast_unit(self), name)
        if v == nil then return nil end
        return v
    end,

    constants_get = function(self)
        return Ptr_Iterator("const Eolian_Variable *",
            eolian.eolian_unit_constants_get(cast_unit(self)))
    end,

    globals_get = function(self)
        return Ptr_Iterator("const Eolian_Variable *",
            eolian.eolian_unit_globals_get(cast_unit(self)))
    end,

    alias_by_name_get = function(self, name)
        local v = eolian.eolian_unit_alias_by_name_get(cast_unit(self), name)
        if v == nil then return nil end
        return v
    end,

    struct_by_name_get = function(self, name)
        local v = eolian.eolian_unit_struct_by_name_get(cast_unit(self), name)
        if v == nil then return nil end
        return v
    end,

    enum_by_name_get = function(self, name)
        local v = eolian.eolian_unit_enum_by_name_get(cast_unit(self), name)
        if v == nil then return nil end
        return v
    end,

    aliases_get = function(self)
        return Ptr_Iterator("const Eolian_Typedecl *",
            eolian.eolian_unit_aliases_get(cast_unit(self)))
    end,

    structs_get = function(self)
        return Ptr_Iterator("const Eolian_Typedecl *",
            eolian.eolian_unit_structs_get(cast_unit(self)))
    end,

    enums_get = function(self)
        return Ptr_Iterator("const Eolian_Typedecl *",
            eolian.eolian_unit_enums_get(cast_unit(self)))
    end
}

ffi.metatype("Eolian_Unit", { __index = unit_idx })

ffi.metatype("Eolian_State", {
    __index = wrap_unit {
        directory_add = function(self, dir)
            return eolian.eolian_state_directory_add(self, dir) ~= 0
        end,

        system_directory_add = function(self)
            return eolian.eolian_state_system_directory_add(self) ~= 0
        end,

        file_parse = function(self, fpath)
            local v = eolian.eolian_state_file_parse(self, fpath)
            if v == nil then
                return nil
            end
            return v
        end,

        all_eo_files_parse = function(self)
            return eolian.eolian_state_all_eo_files_parse(self) ~= 0
        end,

        all_eot_files_parse = function(self)
            return eolian.eolian_state_all_eot_files_parse(self) ~= 0
        end,

        check = function(self)
            return eolian.eolian_state_check(self) ~= 0
        end,

        eo_file_paths_get = function(self)
            return iterator.String_Iterator(eolian.eolian_state_eo_file_paths_get(self))
        end,

        eot_file_paths_get = function(self)
            return iterator.String_Iterator(eolian.eolian_state_eot_file_paths_get(self))
        end,

        eo_files_get = function(self)
            return iterator.String_Iterator(eolian.eolian_state_eo_files_get(self))
        end,

        eot_files_get = function(self)
            return iterator.String_Iterator(eolian.eolian_state_eot_files_get(self))
        end,

        unit_get = function(self)
            return ffi.cast("Eolian_Unit *", self)
        end,

        unit_by_file_get = function(self, fname)
            local v = eolian.eolian_state_unit_by_file_get(state, fname)
            if v == nil then
                return nil
            end
            return v
        end,

        units_get = function(self)
            return Ptr_Iterator("const Eolian_Unit*",
                eolian.eolian_state_units_get(self))
        end,

        objects_by_file_get = function(self, fname)
            return Ptr_Iterator("const Eolian_Object *",
                eolian.eolian_state_objects_by_file_get(self, fname))
        end,

        class_by_file_get = function(self, fname)
            local v = eolian.eolian_state_class_by_file_get(self, fname)
            if v == nil then return nil end
            return v
        end,

        globals_by_file_get = function(unit, fname)
            return Ptr_Iterator("const Eolian_Variable*",
                eolian.eolian_state_globals_by_file_get(self, fname))
        end,

        constants_by_file_get = function(unit, fname)
            return Ptr_Iterator("const Eolian_Variable*",
                eolian.eolian_state_constants_by_file_get(self, fname))
        end,

        aliases_by_file_get = function(self, fname)
            return Ptr_Iterator("const Eolian_Typedecl *",
                eolian.eolian_state_aliases_by_file_get(self, fname))
        end,

        structs_by_file_get = function(self, fname)
            return Ptr_Iterator("const Eolian_Typedecl *",
                eolian.eolian_state_structs_by_file_get(self, fname))
        end,

        enums_by_file_get = function(self, fname)
            return Ptr_Iterator("const Eolian_Typedecl *",
                eolian.eolian_state_enums_by_file_get(self, fname))
        end
    },
    __gc = function(self)
        eolian.eolian_state_free(self)
    end
})

M.new = function()
    return eolian.eolian_state_new()
end

M.type_type = {
    UNKNOWN          = 0,
    VOID             = 1,
    REGULAR          = 2,
    CLASS            = 3,
    UNDEFINED        = 4
}

M.type_builtin_type = {
   INVALID       = 0,

   BYTE          = 1,
   UBYTE         = 2,
   CHAR          = 3,
   SHORT         = 4,
   USHORT        = 5,
   INT           = 6,
   UINT          = 7,
   LONG          = 8,
   ULONG         = 9,
   LLONG         = 10,
   ULLONG        = 11,

   INT8          = 12,
   UINT8         = 13,
   INT16         = 14,
   UINT16        = 15,
   INT32         = 16,
   UINT32        = 17,
   INT64         = 18,
   UINT64        = 19,
   INT128        = 20,
   UINT128       = 21,

   SIZE          = 22,
   SSIZE         = 23,
   INTPTR        = 24,
   UINTPTR       = 25,
   PTRDIFF       = 26,

   TIME          = 27,

   FLOAT         = 28,
   DOUBLE        = 29,

   BOOL          = 30,

   SLICE         = 31,
   RW_SLICE      = 32,

   VOID          = 33,

   ACCESSOR      = 34,
   ARRAY         = 35,
   FUTURE        = 36,
   ITERATOR      = 37,
   HASH          = 38,
   LIST          = 39,
   INARRAY       = 40,
   INLIST        = 41,

   ANY_VALUE     = 42,
   ANY_VALUE_PTR = 43,

   MSTRING       = 44,
   STRING        = 45,
   STRINGSHARE   = 46,
   STRBUF        = 47,

   VOID_PTR      = 48,
   FREE_CB       = 49
}

M.typedecl_type = {
    UNKNOWN          = 0,
    STRUCT           = 1,
    STRUCT_OPAQUE    = 2,
    ENUM             = 3,
    ALIAS            = 4,
    FUNCTION_POINTER = 5
}

M.c_type_type = {
    DEFAULT = 0,
    PARAM   = 1,
    RETURN  = 2
}

ffi.metatype("Eolian_Struct_Type_Field", {
    __index = wrap_object {
        documentation_get = function(self)
            local v = eolian.eolian_typedecl_struct_field_documentation_get(self)
            if v == nil then return nil end
            return v
        end,

        type_get = function(self)
            local v = eolian.eolian_typedecl_struct_field_type_get(self)
            if v == nil then return nil end
            return v
        end
    }
})

ffi.metatype("Eolian_Enum_Type_Field", {
    __index = wrap_object {
        c_name_get = function(self)
            local v = eolian.eolian_typedecl_enum_field_c_name_get(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        documentation_get = function(self)
            local v = eolian.eolian_typedecl_enum_field_documentation_get(self)
            if v == nil then return nil end
            return v
        end,

        value_get = function(self, force)
            local v = eolian.eolian_typedecl_enum_field_value_get(self, force and 1 or 0)
            if v == nil then return nil end
            return v
        end
    }
})

M.Typedecl = ffi.metatype("Eolian_Typedecl", {
    __index = wrap_object {
        type_get = function(self)
            return tonumber(eolian.eolian_typedecl_type_get(self))
        end,

        struct_fields_get = function(self)
            return Ptr_Iterator("const Eolian_Struct_Type_Field*",
                eolian.eolian_typedecl_struct_fields_get(self))
        end,

        struct_field_get = function(self, name)
            local v = eolian.eolian_typedecl_struct_field_get(self, name)
            if v == nil then return nil end
            return v
        end,

        enum_fields_get = function(self)
            return Ptr_Iterator("const Eolian_Enum_Type_Field*",
                eolian.eolian_typedecl_enum_fields_get(self))
        end,

        enum_field_get = function(self, field)
            local v = eolian.eolian_typedecl_enum_field_get(self, field)
            if v == nil then return nil end
            return v
        end,

        enum_legacy_prefix_get = function(self)
            local v = eolian.eolian_typedecl_enum_legacy_prefix_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        documentation_get = function(self, name)
            local v = eolian.eolian_typedecl_documentation_get(self)
            if v == nil then return nil end
            return v
        end,

        base_type_get = function(self)
            local v = eolian.eolian_typedecl_base_type_get(self)
            if v == nil then return nil end
            return v
        end,

        aliased_base_get = function(self)
            local v = eolian.eolian_typedecl_aliased_byse_get(self)
            if v == nil then return nil end
            return v
        end,

        is_extern = function(self)
            return eolian.eolian_typedecl_is_extern(self) ~= 0
        end,

        c_type_get = function(self)
            local v = eolian.eolian_typedecl_c_type_get(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        free_func_get = function(self)
            local v = eolian.eolian_typedecl_free_func_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        function_pointer_get = function(self)
            local v = eolian.eolian_typedecl_function_pointer_get(self)
            if v == nil then return nil end
            return v
        end
    }
})

M.Type = ffi.metatype("Eolian_Type", {
    __index = wrap_object {
        type_get = function(self)
            return tonumber(eolian.eolian_type_type_get(self))
        end,

        builtin_type_get = function(self)
            return tonumber(eolian.eolian_type_builtin_type_get(self))
        end,

        base_type_get = function(self)
            local v = eolian.eolian_type_base_type_get(self)
            if v == nil then return nil end
            return v
        end,

        next_type_get = function(self)
            local v = eolian.eolian_type_next_type_get(self)
            if v == nil then return nil end
            return v
        end,

        typedecl_get = function(self)
            local v = eolian.eolian_type_typedecl_get(self)
            if v == nil then return nil end
            return v
        end,

        aliased_base_get = function(self)
            local v = eolian.eolian_type_aliased_byse_get(self)
            if v == nil then return nil end
            return v
        end,

        class_get = function(self)
            local v = eolian.eolian_type_class_get(self)
            if v == nil then return nil end
            return v
        end,

        is_owned = function(self)
            return eolian.eolian_type_is_owned(self) ~= 0
        end,

        is_const = function(self)
            return eolian.eolian_type_is_const(self) ~= 0
        end,

        is_ptr = function(self)
            return eolian.eolian_type_is_ptr(self) ~= 0
        end,

        c_type_get = function(self, ctype)
            local v = eolian.eolian_type_c_type_get(self, ctype)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        free_func_get = function(self)
            local v = eolian.eolian_type_free_func_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end
    }
})

M.function_type = {
    UNRESOLVED       = 0,
    PROPERTY         = 1,
    PROP_SET         = 2,
    PROP_GET         = 3,
    METHOD           = 4,
    FUNCTION_POINTER = 5
}

M.Function = ffi.metatype("Eolian_Function", {
    __index = wrap_object {
        type_get = function(self)
            return tonumber(eolian.eolian_function_type_get(self))
        end,

        scope_get = function(self, ftype)
            return tonumber(eolian.eolian_function_scope_get(self, ftype))
        end,

        full_c_name_get = function(self, ftype, use_legacy)
            local v = eolian.eolian_function_full_c_name_get(self, ftype, use_legacy or false)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        legacy_get = function(self, ftype)
            local v = eolian.eolian_function_legacy_get(self, ftype)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        implement_get = function(self)
            local v = eolian.eolian_function_implement_get(self)
            if v == nil then return nil end
            return v
        end,

        is_legacy_only = function(self, ftype)
            return eolian.eolian_function_is_legacy_only(self, ftype) ~= 0
        end,

        is_class = function(self)
            return eolian.eolian_function_is_class(self) ~= 0
        end,

        is_beta = function(self)
            return eolian.eolian_function_is_beta(self) ~= 0
        end,

        is_constructor = function(self, klass)
            return eolian.eolian_function_is_constructor(self, klass) ~= 0
        end,

        is_function_pointer = function(self)
            return eolian.eolian_function_is_function_pointer(self) ~= 0
        end,

        property_keys_get = function(self, ftype)
            return Ptr_Iterator("const Eolian_Function_Parameter*",
                eolian.eolian_property_keys_get(self, ftype))
        end,

        property_values_get = function(self, ftype)
            return Ptr_Iterator("const Eolian_Function_Parameter*",
                eolian.eolian_property_values_get(self, ftype))
        end,

        parameters_get = function(self)
            return Ptr_Iterator("const Eolian_Function_Parameter*",
                eolian.eolian_function_parameters_get(self))
        end,

        return_type_get = function(self, ftype)
            local v = eolian.eolian_function_return_type_get(self, ftype)
            if v == nil then return nil end
            return v
        end,

        return_default_value_get = function(self, ftype)
            local v = eolian.eolian_function_return_default_value_get(self, ftype)
            if v == nil then return nil end
            return v
        end,

        return_documentation_get = function(self, ftype)
            local v = eolian.eolian_function_return_documentation_get(self, ftype)
            if v == nil then return nil end
            return v
        end,

        return_is_warn_unused = function(self, ftype)
            return eolian.eolian_function_return_is_warn_unused(self,
                ftype) ~= 0
        end,

        is_const = function(self)
            return eolian.eolian_function_object_is_const(self) ~= 0
        end
    }
})

M.parameter_dir = {
    UNKNOWN = 0,
    IN      = 1,
    OUT     = 2,
    INOUT   = 3
}

ffi.metatype("Eolian_Function_Parameter", {
    __index = wrap_object {
        direction_get = function(self)
            return tonumber(eolian.eolian_parameter_direction_get(self))
        end,

        type_get = function(self)
            local v = eolian.eolian_parameter_type_get(self)
            if v == nil then return nil end
            return v
        end,

        default_value_get = function(self)
            local v = eolian.eolian_parameter_default_value_get(self)
            if v == nil then return nil end
            return v
        end,

        documentation_get = function(self)
            local v = eolian.eolian_parameter_documentation_get(self)
            if v == nil then return nil end
            return v
        end,

        is_nonull = function(self)
            return eolian.eolian_parameter_is_nonull(self) ~= 0
        end,

        is_nullable = function(self)
            return eolian.eolian_parameter_is_nullable(self) ~= 0
        end,

        is_optional = function(self)
            return eolian.eolian_parameter_is_optional(self) ~= 0
        end
    }
})

ffi.metatype("Eolian_Implement", {
    __index = wrap_object {
        class_get = function(self)
            local v = eolian.eolian_implement_class_get(self)
            if v == nil then return nil end
            return v
        end,

        implementing_class_get = function(self)
            local v = eolian.eolian_implement_implementing_class_get(self)
            if v == nil then return nil end
            return v
        end,

        function_get = function(self)
            local tp = ffi.new("Eolian_Function_Type[1]")
            local v = eolian.eolian_implement_function_get(self, tp)
            if v == nil then return nil end
            return v, tp[0]
        end,

        documentation_get = function(self, ftype)
            local v = eolian.eolian_implement_documentation_get(self, ftype)
            if v == nil then return nil end
            return v
        end,

        is_auto = function(self, ftype)
            return eolian.eolian_implement_is_auto(self, ftype) ~= 0
        end,

        is_empty = function(self, ftype)
            return eolian.eolian_implement_is_empty(self, ftype) ~= 0
        end,

        is_pure_virtual = function(self, ftype)
            return eolian.eolian_implement_is_pure_virtual(self, ftype) ~= 0
        end,

        is_prop_get = function(self)
            return eolian.eolian_implement_is_prop_get(self) ~= 0
        end,

        is_prop_set = function(self)
            return eolian.eolian_implement_is_prop_set(self) ~= 0
        end
    }
})

ffi.metatype("Eolian_Constructor", {
    __index = wrap_object {
        class_get = function(self)
            local v = eolian.eolian_constructor_class_get(self)
            if v == nil then return nil end
            return v
        end,

        function_get = function(self)
            local v = eolian.eolian_constructor_function_get(self)
            if v == nil then return nil end
            return v
        end,

        is_optional = function(self)
            return eolian.eolian_constructor_is_optional(self) ~= 0
        end
    }
})

ffi.metatype("Eolian_Event", {
    __index = wrap_object {
        type_get = function(self)
            local v = eolian.eolian_event_type_get(self)
            if v == nil then return nil end
            return v
        end,

        class_get = function(self)
            local v = eolian.eolian_event_class_get(self)
            if v == nil then return nil end
            return v
        end,

        documentation_get = function(self)
            local v = eolian.eolian_event_documentation_get(self)
            if v == nil then return nil end
            return v
        end,

        scope_get = function(self)
            return tonumber(eolian.eolian_event_scope_get(self))
        end,

        c_name_get = function(self)
            local v = eolian.eolian_event_c_name_get(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        is_beta = function(self)
            return eolian.eolian_event_is_beta(self) ~= 0
        end,

        is_hot = function(self)
            return eolian.eolian_event_is_hot(self) ~= 0
        end,

        is_restart = function(self)
            return eolian.eolian_event_is_restart(self) ~= 0
        end
    }
})

M.class_type = {
    UNKNOWN   = 0,
    REGULAR   = 1,
    ABSTRACT  = 2,
    MIXIN     = 3,
    INTERFACE = 4
}

M.Class = ffi.metatype("Eolian_Class", {
    __index = wrap_object {
        type_get = function(self)
            return tonumber(eolian.eolian_class_type_get(self))
        end,

        documentation_get = function(self)
            local v = eolian.eolian_class_documentation_get(self)
            if v == nil then return nil end
            return v
        end,

        legacy_prefix_get = function(self)
            local v = eolian.eolian_class_legacy_prefix_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        eo_prefix_get = function(self)
            local v = eolian.eolian_class_eo_prefix_get(self)
            if v == nil then
                local buf = self:namespaces_get()
                buf[#buf + 1] = self:short_name_get()
                return table.concat(buf, "_"):lower()
            end
            return ffi.string(v)
        end,

        data_type_get = function(self)
            local v = eolian.eolian_class_data_type_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        parent_get = function(self)
            local v = eolian.eolian_class_parent_get(self)
            if v == nil then return nil end
            return v
        end,

        extensions_get = function(self)
            return Ptr_Iterator("const Eolian_Class*",
                eolian.eolian_class_extensions_get(self))
        end,

        functions_get = function(self, func_type)
            return Ptr_Iterator("const Eolian_Function*",
                eolian.eolian_class_functions_get(self, func_type))
        end,

        function_by_name_get = function(self, fname, ftype)
            local v = eolian.eolian_class_function_by_name_get(self, fname,
                ftype)
            if v == nil then return nil end
            return v
        end,

        implements_get = function(self)
            return Ptr_Iterator("const Eolian_Implement*",
                eolian.eolian_class_implements_get(self))
        end,

        constructors_get = function(self)
            return Ptr_Iterator("const Eolian_Constructor*",
                eolian.eolian_class_constructors_get(self))
        end,

        events_get = function(self)
            return Ptr_Iterator("const Eolian_Event*",
                eolian.eolian_class_events_get(self))
        end,

        ctor_enable_get = function(self)
            return eolian.eolian_class_ctor_enable_get(self) ~= 0
        end,

        dtor_enable_get = function(self)
            return eolian.eolian_class_dtor_enable_get(self) ~= 0
        end,

        c_get_function_name_get = function(self)
            local v = eolian.eolian_class_c_get_function_name_get(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end
    }
})

M.expression_type = {
    UNKNOWN = 0,
    INT     = 1,
    UINT    = 2,
    LONG    = 3,
    ULONG   = 4,
    LLONG   = 5,
    ULLONG  = 6,
    FLOAT   = 7,
    DOUBLE  = 8,
    STRING  = 9,
    CHAR    = 10,
    NULL    = 11,
    BOOL    = 12,
    NAME    = 13,
    UNARY   = 15,
    BINARY  = 16
}

local etype = M.expression_type

M.expression_mask = {
    SINT   = bit.lshift(1, 0),
    UINT   = bit.lshift(1, 1),
    FLOAT  = bit.lshift(1, 2),
    BOOL   = bit.lshift(1, 3),
    STRING = bit.lshift(1, 4),
    CHAR   = bit.lshift(1, 5),
    NULL   = bit.lshift(1, 6)
}

local emask = M.expression_mask

emask.INT    = bit.bor(emask.SINT  , emask.UINT )
emask.SIGNED = bit.bor(emask.SINT  , emask.FLOAT)
emask.NUMBER = bit.bor(emask.INT   , emask.FLOAT)
emask.ALL    = bit.bor(emask.NUMBER, emask.BOOL,
                       emask.STRING, emask.CHAR, emask.NULL)

M.variable_type = {
    UNKNOWN  = 0,
    CONSTANT = 1,
    GLOBAL   = 2
}

local value_con = {
    [etype.INT   ] = function(v) return tonumber(v.value.i   ) end,
    [etype.UINT  ] = function(v) return tonumber(v.value.u   ) end,
    [etype.LONG  ] = function(v) return v.value.l              end,
    [etype.ULONG ] = function(v) return v.value.ul             end,
    [etype.LLONG ] = function(v) return v.value.ll             end,
    [etype.ULLONG] = function(v) return v.value.ull            end,
    [etype.FLOAT ] = function(v) return tonumber(v.value.f   ) end,
    [etype.DOUBLE] = function(v) return tonumber(v.value.d   ) end,
    [etype.STRING] = function(v) return ffi.string(v.value.s ) end,
    [etype.CHAR  ] = function(v) return string.char(v.value.c) end,
    [etype.NULL  ] = function(v) return nil                    end,
    [etype.BOOL  ] = function(v) return v.value.b ~= 0         end
}

M.Value = ffi.metatype("Eolian_Value", {
    __index = {
        get_type = function(self)
            return tonumber(ffi.cast("Eolian_Value_t*", self).type)
        end,

        get_value = function(self)
            local   tp = self:get_type()
            local  fun = value_con[tonumber(tp)]
            if not fun then return nil end
            return fun()
        end,

        to_literal = function(self)
            local v = eolian.eolian_expression_value_to_literal(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end
    }
})

M.binary_operator = {
    INVALID = 0,

    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4,
    MOD = 5,

    EQ = 6,
    NQ = 7,
    GT = 8,
    LT = 9,
    GE = 10,
    LE = 11,

    AND = 12,
    OR  = 13,

    BAND = 14,
    BOR  = 15,
    BXOR = 16,
    LSH  = 17,
    RSH  = 18
}

M.unary_operator = {
    INVALID = 0,

    UNM = 1,
    UNP = 2,

    NOT  = 3,
    BNOT = 4
}

M.Expression = ffi.metatype("Eolian_Expression", {
    __index = wrap_object {
        eval = function(self, mask)
            mask = mask or emask.ALL
            local v = eolian.eolian_expression_eval(self, mask)
            if v == nil then return nil end
            return ffi.cast("Eolian_Value*", v)
        end,

        eval_type = function(self, tp)
            local v = eolian.eolian_expression_eval_type(self, tp)
            if v == nil then return nil end
            return ffi.cast("Eolian_Value*", v)
        end,

        serialize = function(self)
            local v = eolian.eolian_expression_serialize(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        type_get = function(self)
            return tonumber(eolian.eolian_expression_type_get(self))
        end,

        binary_operator_get = function(self)
            return tonumber(eolian.eolian_expression_binary_operator_get(self))
        end,

        binary_lhs_get = function(self)
            local v = eolian.eolian_expression_binary_lhs_get(self)
            if v == nil then return nil end
            return v
        end,

        binary_rhs_get = function(self)
            local v = eolian.eolian_expression_binary_rhs_get(self)
            if v == nil then return nil end
            return v
        end,

        unary_operator_get = function(self)
            return tonumber(eolian.eolian_expression_unary_operator_get(self))
        end,

        unary_expression_get = function(self)
            local v = eolian.eolian_expression_unary_expression_get(self)
            if v == nil then return nil end
            return v
        end,

        value_get = function(self)
            local v = eolian.eolian_expression_value_get(self)
            if v == nil then return nil end
            return ffi.cast("Eolian_Value*", v)
        end
    }
})

M.Variable = ffi.metatype("Eolian_Variable", {
    __index = wrap_object {
        type_get = function(self)
            return tonumber(eolian.eolian_variable_type_get(self))
        end,

        documentation_get = function(self)
            local v = eolian.eolian_variable_documentation_get(self)
            if v == nil then return nil end
            return v
        end,

        base_type_get = function(self)
            local v = eolian.eolian_variable_base_type_get(self)
            if v == nil then return nil end
            return v
        end,

        value_get = function(self)
            local v = eolian.eolian_variable_value_get(self)
            if v == nil then return nil end
            return v
        end,

        is_extern = function(self)
            return eolian.eolian_variable_is_extern(self) ~= 0
        end
    }
})

M.Documentation = ffi.metatype("Eolian_Documentation", {
    __index = wrap_object {
        summary_get = function(self)
            local v = eolian.eolian_documentation_summary_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        description_get = function(self)
            local v = eolian.eolian_documentation_description_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        since_get = function(self)
            local v = eolian.eolian_documentation_since_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end
    }
})

M.doc_token_type = {
    UNKNOWN          = 0,
    TEXT             = 1,
    REF              = 2,
    MARK_NOTE        = 3,
    MARK_WARNING     = 4,
    MARK_REMARK      = 5,
    MARK_TODO        = 6,
    MARKUP_MONOSPACE = 7
}

M.documentation_string_split = function(str)
    if not str then
        return {}
    end
    local sep = str:find("\n\n", 1, true)
    local ret = {}
    while true do
        local pstr = (sep and str:sub(1, sep - 1) or str):match("^%s*(.-)%s*$")
        if #pstr > 0 then
            ret[#ret + 1] = pstr
        end
        if not sep then
            break
        end
        str = str:sub(sep + 2)
        sep = str:find("\n\n", 1, true)
    end
    return ret
end

M.documentation_tokenize = function(doc, ret)
    local ret = eolian.eolian_documentation_tokenize(doc, ret)
    if ret == nil then
        return nil
    end
    return ffi.string(ret)
end

M.doc_token_init = function()
    local ret = ffi.new("Eolian_Doc_Token")
    eolian.eolian_doc_token_init(ret)
    return ret
end

M.Eolian_Doc_Token = ffi.metatype("Eolian_Doc_Token", {
    __index = {
        type_get = function(self)
            return tonumber(eolian.eolian_doc_token_type_get(self))
        end,

        text_get = function(self)
            local str = eolian.eolian_doc_token_text_get(self)
            if str == nil then
                return nil
            end
            local ret = ffi.string(str)
            ffi.C.free(str)
            return ret
        end,

        ref_resolve = function(self, state)
            local stor = ffi.new("const Eolian_Object *[2]")
            local tp = tonumber(eolian.eolian_doc_token_ref_resolve(self, state, stor, stor + 1))
            local reft = M.object_type
            if tp == reft.CLASS then
                return tp, ffi.cast("const Eolian_Class *", stor[0])
            elseif tp == reft.FUNCTION then
                return tp, ffi.cast("const Eolian_Class *", stor[0]),
                           ffi.cast("const Eolian_Function *", stor[1])
            elseif tp == reft.EVENT then
                return tp, ffi.cast("const Eolian_Class *", stor[0]),
                           ffi.cast("const Eolian_Event *", stor[1])
            elseif tp == reft.TYPEDECL then
                return tp, ffi.cast("const Eolian_Typedecl *", stor[0])
            elseif tp == reft.STRUCT_FIELD then
                return tp, ffi.cast("const Eolian_Typedecl *", stor[0]),
                           ffi.cast("const Eolian_Struct_Type_Field *", stor[1])
            elseif tp == reft.ENUM_FIELD then
                return tp, ffi.cast("const Eolian_Typedecl *", stor[0]),
                           ffi.cast("const Eolian_Enum_Type_Field *", stor[1])
            elseif tp == reft.VARIABLE then
                return tp, ffi.cast("const Eolian_Variable *", stor[0])
            else
                return reft.UNKNOWN
            end
        end
    }
})

return M
