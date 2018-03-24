#ifndef ELM_CODE_COMMON_H_
# define ELM_CODE_COMMON_H_

typedef struct _Elm_Code Elm_Code;
typedef struct _Elm_Code_File Elm_Code_File;

/** Event marking that a single line has loaded or changed */
EAPI extern const Efl_Event_Description ELM_CODE_EVENT_LINE_LOAD_DONE;
/** Event that marks a file load has been completed */
EAPI extern const Efl_Event_Description ELM_CODE_EVENT_FILE_LOAD_DONE;

typedef enum {
   ELM_CODE_STATUS_TYPE_DEFAULT = 0,
   ELM_CODE_STATUS_TYPE_CURRENT,
   ELM_CODE_STATUS_TYPE_IGNORED,
   ELM_CODE_STATUS_TYPE_NOTE,
   ELM_CODE_STATUS_TYPE_WARNING,
   ELM_CODE_STATUS_TYPE_ERROR,
   ELM_CODE_STATUS_TYPE_FATAL,

   ELM_CODE_STATUS_TYPE_ADDED,
   ELM_CODE_STATUS_TYPE_REMOVED,
   ELM_CODE_STATUS_TYPE_CHANGED,

   ELM_CODE_STATUS_TYPE_PASSED,
   ELM_CODE_STATUS_TYPE_FAILED,

   ELM_CODE_STATUS_TYPE_TODO,

   ELM_CODE_STATUS_TYPE_COUNT
} Elm_Code_Status_Type;


typedef enum {
   ELM_CODE_TOKEN_TYPE_DEFAULT = ELM_CODE_STATUS_TYPE_COUNT,
   ELM_CODE_TOKEN_TYPE_COMMENT,
   ELM_CODE_TOKEN_TYPE_STRING,
   ELM_CODE_TOKEN_TYPE_NUMBER,
   ELM_CODE_TOKEN_TYPE_BRACE,
   ELM_CODE_TOKEN_TYPE_TYPE,
   ELM_CODE_TOKEN_TYPE_CLASS,
   ELM_CODE_TOKEN_TYPE_FUNCTION,
   ELM_CODE_TOKEN_TYPE_PARAM,
   ELM_CODE_TOKEN_TYPE_KEYWORD,
   ELM_CODE_TOKEN_TYPE_PREPROCESSOR,

   ELM_CODE_TOKEN_TYPE_ADDED,
   ELM_CODE_TOKEN_TYPE_REMOVED,
   ELM_CODE_TOKEN_TYPE_CHANGED,

   ELM_CODE_TOKEN_TYPE_MATCH,

   ELM_CODE_TOKEN_TYPE_COUNT
} Elm_Code_Token_Type;

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @file
 * @brief Common data structures and constants.
 */

struct _Elm_Code_Config
{
   Eina_Bool trim_whitespace;
   Eina_Bool indent_style_efl;
};

struct _Elm_Code
{
   Elm_Code_File *file;
   Eina_List *widgets;
   Eina_List *parsers;

   struct _Elm_Code_Config config;
};

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_COMMON_H_ */
