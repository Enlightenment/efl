/* This include has been added to support Eo in Ecore */
#include <Eo.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup Ecore_Poller_Group
 *
 * @{
 */

#include "ecore_poller.eo.h"

/**
 * @}
 */

/**
 * @ingroup Ecore_Animator_Group
 *
 * @{
 */

#include "ecore_animator.eo.h"

/**
 * @}
 */

/**
 * @ingroup Ecore_Timer_Group
 *
 * @{
 */

#include "efl_timer.eo.h"

/**
 * @}
 */

/**
 * @ingroup Ecore_Exe_Group
 *
 * @{
 */

#include "ecore_exe.eo.h"

/**
 * @}
 */


/**
 * @ingroup Ecore_MainLoop_Group
 *
 * @{
 */

#include "efl_loop.eo.h"

#include "efl_loop_user.eo.h"

/* We ue the factory pattern here, so you shouldn't call eo_add directly. */
EAPI Eo *ecore_main_loop_get(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
