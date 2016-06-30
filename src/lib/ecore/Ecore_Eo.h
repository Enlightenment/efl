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
 * @ingroup Ecore_Timer_Group
 *
 * @{
 */

#include "efl_loop_timer.eo.h"

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

#include "efl_loop_fd.eo.h"

#include "efl_promise.eo.h"

/* We ue the factory pattern here, so you shouldn't call eo_add directly. */
EAPI Eo *ecore_main_loop_get(void);

/**
 * @}
 */

/**
 * @ingroup Ecore_Fd_Io_Group
 *
 * @{
 */

#include "efl_io_closer_fd.eo.h"
#include "efl_io_positioner_fd.eo.h"
#include "efl_io_reader_fd.eo.h"
#include "efl_io_sizer_fd.eo.h"
#include "efl_io_writer_fd.eo.h"
#include "efl_io_stdin.eo.h"
#include "efl_io_stdout.eo.h"
#include "efl_io_stderr.eo.h"
#include "efl_io_file.eo.h"
#include "efl_io_copier.eo.h"

/**
 * @}
 */


#ifdef __cplusplus
}
#endif
