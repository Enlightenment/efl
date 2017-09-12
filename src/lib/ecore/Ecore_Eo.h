/* This include has been added to support Eo in Ecore */
#include <Eo.h>

#ifdef __cplusplus
extern "C" {
#endif

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

EAPI Eina_Future_Scheduler *efl_loop_future_scheduler_get(Eo *obj);

#include "efl_loop_fd.eo.h"

#include "efl_promise.eo.h"

#include "efl_interpolator.eo.h"
#include "efl_interpolator_linear.eo.h"
#include "efl_interpolator_accelerate.eo.h"
#include "efl_interpolator_decelerate.eo.h"
#include "efl_interpolator_sinusoidal.eo.h"
#include "efl_interpolator_divisor.eo.h"
#include "efl_interpolator_bounce.eo.h"
#include "efl_interpolator_spring.eo.h"
#include "efl_interpolator_cubic_bezier.eo.h"

/* We ue the factory pattern here, so you shouldn't call eo_add directly. */
EAPI Eo *ecore_main_loop_get(void);

typedef struct _Efl_Future_Composite_Progress Efl_Future_All_Progress;

struct _Efl_Future_Composite_Progress
{
   Efl_Future *inprogress;
   void *progress;

   unsigned int index;
};

EAPI Efl_Future *efl_future_all_internal(Efl_Future *f1, ...);
EAPI Efl_Future *efl_future_iterator_all(Eina_Iterator *it);

#define efl_future_all(...) efl_future_all_internal(__VA_ARGS__, NULL)

typedef struct _Efl_Future_Race_Success Efl_Future_Race_Success;
typedef struct _Efl_Future_Composite_Progress Efl_Future_Race_Progress;

struct _Efl_Future_Race_Success
{
   Efl_Future *winner;
   void *value;

   unsigned int index;
};

EAPI Efl_Future *efl_future_race_internal(Efl_Future *f1, ...);
EAPI Efl_Future *efl_future_iterator_race(Eina_Iterator *it);

#define efl_future_race(...) efl_future_race_internal(__VA_ARGS__, NULL)

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
#include "efl_model_composite_boolean.eo.h"
#include "efl_model_composite_boolean_children.eo.h"
#include "efl_model_composite_selection.eo.h"
#include "efl_model_composite_selection_children.eo.h"

/**
 * @}
 */


#ifdef __cplusplus
}
#endif
