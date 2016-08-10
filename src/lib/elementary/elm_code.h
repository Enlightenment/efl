#ifndef ELM_CODE_H_
# define ELM_CODE_H_

#ifdef EFL_BETA_API_SUPPORT

#include "elm_code_common.h"
#include "elm_code_line.h"
#include "elm_code_text.h"
#include "elm_code_file.h"
#include "elm_code_parse.h"
#include "elm_code_widget.eo.h"
#include "elm_code_widget_legacy.h"
#include "elm_code_widget_selection.h"
#include "elm_code_diff_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for loading Elm Code widgets.
 */

/**
 * @brief Init / shutdown functions.
 * @defgroup Init  Init / Shutdown
 *
 * @{
 *
 * Functions of obligatory usage, handling proper initialization
 * and shutdown routines.
 *
 * Before the usage of any other function, Elm Code should be properly
 * initialized with @ref elm_code_init() and the last call to Elm Code's
 * functions should be @ref elm_code_shutdown(), so everything will
 * be correctly freed.
 *
 * Elm Code logs everything with Eina Log, using the "elm_code" log domain.
 *
 */

/**
 * Initialize Elm Code.
 *
 * Initializes Elm Code, its dependencies and modules. Should be the first
 * function of Elm Code to be called.
 *
 * @return The init counter value.
 *
 * @see elm_code_shutdown().
 *
 * @ingroup Init
 */
EAPI int elm_code_init(void);

/**
 * Shutdown Elm Code
 *
 * Shutdown Elm Code. If init count reaches 0, all the internal structures will
 * be freed. Any Elm Code library call after this point will leads to an error.
 *
 * @return Elm Code's init counter value.
 *
 * @see elm_code_init()
 *
 * @ingroup Init
 */
EAPI int elm_code_shutdown(void);

/**
 * Create a new Elm Code instance
 *
 * This method creates a new Elm Code instance using an in-memory file for backing changes.
 * A regular file can be set after creation if required.
 * Once an Elm Code has been created you can create widgets that render the content.
 *
 * @return an allocated Elm_Code that references the given file
 * @see elm_code_file_open()
 */
EAPI Elm_Code *elm_code_create(void);

/**
 * Free an Elm Code instance
 *
 * Releases the resources retained by the code instance and any files it references.
 */
EAPI void elm_code_free(Elm_Code *code);

/**
 * @}
 *
 * @brief Callbacks and message passing.
 * @defgroup Callbacks  Managing the information flow between Elm_Code objects and Evas_Object widgets
 *
 * @{
 *
 * Managing the callbacks and other behaviours that cross the backend - frontend divide.
 */


EAPI void elm_code_callback_fire(Elm_Code *code, const Efl_Event_Description *signal, void *data);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* EFL_BETA_API_SUPPORT */

#endif /* ELM_CODE_H_ */
