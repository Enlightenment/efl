#ifndef _ECORE_EO_H
#define _ECORE_EO_H

/* This include has been added to support Eo in Ecore */
#include <Eo.h>

/**
 * @ingroup Ecore_Timer_Group
 *
 * @{
 */

#include "efl_loop_timer.eo.h"

/**
 * @}
 */


#include "efl_loop_message_future.eo.h"
#include "efl_loop_message_future_handler.eo.h"

/**
 * @ingroup Ecore_MainLoop_Group
 *
 * @{
 */

#include "efl_loop_message.eo.h"
#include "efl_loop_message_handler.eo.h"

#include "efl_task.eo.h"
#include "efl_thread.eo.h"
#include "efl_threadio.eo.h"
#include "efl_exe.eo.h"

#include "efl_loop.eo.h"
#include "efl_app.eo.h"
#include "efl_appthread.eo.h"

/**
 * @brief Quits the main loop once all the events currently on the queue have
 * been processed.
 *
 * @param[in] exit_code Returned value by begin()
 *
 * @note This function can only be called from the main loop.
 *
 * @ingroup Efl_Loop
 */
EAPI void efl_exit(int exit_code);

EAPI int efl_loop_exit_code_process(Eina_Value *value);

#include "efl_loop_consumer.eo.h"

/**
 * @brief Get the future scheduler for the current loop.
 *
 * @param[in] An object which is either a loop or a loop consumer
 * @return The current loop's future scheduler.
 */
EAPI Eina_Future_Scheduler *efl_loop_future_scheduler_get(const Eo *obj);

/**
 * @brief Create a promise attached to the current loop
 *
 * @param[in] An object which will provide a loop, either by being a loop or a loop consumer
 * @param cancel_cb A callback used to inform that the promise was canceled. Use
 * this callback to @c free @p data. @p cancel_cb must not be @c NULL !
 * @param data Data to @p cancel_cb.
 * @return A promise or @c NULL on error.
 *
 * @see eina_promise_new()
 */
EAPI Eina_Promise *efl_loop_promise_new(const Eo *obj, Eina_Promise_Cancel_Cb cancel_cb, const void *data);

#include "efl_loop_fd.eo.h"
#include "efl_loop_handler.eo.h"

#include "efl_linear_interpolator.eo.h"
#include "efl_accelerate_interpolator.eo.h"
#include "efl_decelerate_interpolator.eo.h"
#include "efl_sinusoidal_interpolator.eo.h"
#include "efl_divisor_interpolator.eo.h"
#include "efl_bounce_interpolator.eo.h"
#include "efl_spring_interpolator.eo.h"
#include "efl_cubic_bezier_interpolator.eo.h"

/* We ue the factory pattern here, so you shouldn't call eo_add directly. */
EAPI Eo *efl_main_loop_get(void);
EAPI Eo *efl_app_get(void);

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
#include "efl_io_buffered_stream.eo.h"

/**
 * @}
 */

/**
 * @ingroup Ecore_Model_Group
 *
 * @{
 */

#include "efl_model_item.eo.h"
#include "efl_model_container.eo.h"
#include "efl_model_container_item.eo.h"
#include "efl_model_composite.eo.h"
#include "efl_model_composite_boolean.eo.h"
#include "efl_model_composite_boolean_children.eo.h"
#include "efl_model_composite_selection.eo.h"
#include "efl_model_composite_selection_children.eo.h"

/**
 * @}
 */

#endif
