#ifndef ELM_CODE_COMMON_H_
# define ELM_CODE_COMMON_H_

#include <Eo.h>
#include <Eina.h>

EAPI extern const Eo_Event_Description ELM_CODE_EVENT_LINE_SET_DONE;
EAPI extern const Eo_Event_Description ELM_CODE_EVENT_FILE_LOAD_DONE;

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
   ELM_CODE_TOKEN_TYPE_COMMENT,


   ELM_CODE_TOKEN_TYPE_ADDED,
   ELM_CODE_TOKEN_TYPE_REMOVED,
   ELM_CODE_TOKEN_TYPE_CHANGED,

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
