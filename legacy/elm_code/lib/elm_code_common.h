#ifndef ELM_CODE_COMMON_H_
# define ELM_CODE_COMMON_H_

#include <Eina.h>

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
