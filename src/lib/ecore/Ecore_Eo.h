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

typedef struct _Efl_Future_Composite_Progress Efl_Future_All_Progress;

struct _Efl_Future_Composite_Progress
{
   Efl_Future *inprogress;
   void *progress;

   unsigned int index;
};

EAPI Efl_Future *efl_future_all_internal(Efl_Future *f1, ...);

#define efl_future_all(...) efl_future_all_internal(__VA_ARGS__, NULL)

typedef struct _Efl_Future_Race_Success Efl_Future_Race_Success;
typedef struct _Efl_Future_Composite_Progress Efl_Future_Race_Progress;

struct _Efl_Future_Race_Success
{
   Efl_Promise *next;
   void *value;

   Efl_Future *winner;
   unsigned int index;
};

EAPI Efl_Future *efl_future_race_internal(Efl_Future *f1, ...);

#define efl_future_race(...) efl_future_race_internal(__VA_ARGS__, NULL)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
