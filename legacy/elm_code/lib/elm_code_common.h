#ifndef ELM_CODE_COMMON_H_
# define ELM_CODE_COMMON_H_

#include <Eina.h>

// TODO figure out how this can be fixed
#define ELM_CODE_EVENT_LINE_SET_DONE "line,set,done"
//EAPI const Eo_Event_Description ELM_CODE_EVENT_LINE_SET_DONE = 
//    EO_EVENT_DESCRIPTION("line,set,done", "");

typedef enum {
   ELM_CODE_STATUS_TYPE_DEFAULT = 0,
   ELM_CODE_STATUS_TYPE_ERROR,

   ELM_CODE_STATUS_TYPE_ADDED,
   ELM_CODE_STATUS_TYPE_REMOVED,
   ELM_CODE_STATUS_TYPE_CHANGED,

   ELM_CODE_STATUS_TYPE_COUNT
} Elm_Code_Status_Type;


typedef enum {
   ELM_CODE_TOKEN_TYPE_DEFAULT = ELM_CODE_STATUS_TYPE_COUNT,

   ELM_CODE_TOKEN_TYPE_COUNT
} Elm_Code_Token_Type;

#include "elm_code_file.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @file
 * @brief Common data structures and constants.
 */

typedef struct _Elm_Code
{
   Elm_Code_File *file;
   Eina_List *widgets;
} Elm_Code;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_COMMON_H_ */
