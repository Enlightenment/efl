#ifndef _ECORE_EO_H
#define _ECORE_EO_H

/* This include has been added to support Eo in Ecore */
#include <Eo.h>

/**
 * @addtogroup Ecore_Timer_Group
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
 * @addtogroup Ecore_Main_Loop_Group
 *
 * @{
 */

#include "efl_core_env.eo.h"
#include "efl_core_proc_env.eo.h"
#include "efl_core_command_line.eo.h"

#include "efl_loop_message.eo.h"
#include "efl_loop_message_handler.eo.h"

#include "efl_task.eo.h"
#include "efl_thread.eo.h"
#include "efl_threadio.eo.h"
#include "efl_exe.eo.h"

#include "efl_loop.eo.h"
#include "efl_app.eo.h"
#include "efl_appthread.eo.h"

/* To be deprecated at some point */

/** Please use efl_provider_register instead. */
EAPI Eina_Bool efl_loop_register(Efl_Loop *obj, const Efl_Class *klass, const Efl_Object *provider);
/** Please use efl_provider_unregister instead. */
EAPI Eina_Bool efl_loop_unregister(Efl_Loop *obj, const Efl_Class *klass, const Efl_Object *provider);

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
 * @param[in] obj An object which is either a loop or a loop consumer
 * @return The current loop's future scheduler.
 */
EAPI Eina_Future_Scheduler *efl_loop_future_scheduler_get(const Eo *obj);

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

/**
 * @}
 */

/**
 * @ingroup Ecore
 * @addtogroup Ecore_Fd_Io_Group
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
 * @ingroup Ecore
 * @addtogroup Ecore_Model_Group
 *
 * @{
 */

#include "efl_loop_model.eo.h"
#include "efl_generic_model.eo.h"
#include "efl_container_model.eo.h"
#include "efl_composite_model.eo.h"
#include "efl_boolean_model.eo.h"
#include "efl_filter_model.eo.h"

/**
 * @}
 */

#endif
