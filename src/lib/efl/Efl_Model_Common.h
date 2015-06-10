#ifndef _EFL_MODEL_COMMON_H
#define _EFL_MODEL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct _Efl_Model_Children_Event
 * Every time a child id added the event
 * EFL_MODEL_EVENT_CHILD_ADDED is dispatched
 * passing along this structure.
 */
struct _Efl_Model_Children_Event
{
   Eo *child; /**< child, for child_add */
   /**
    * index is a hint and is intended
    * to provide a way for applications
    * to control/know children relative
    * positions through listings.
    *
    * NOTE: If listing is performed asynchronously
    * exact order may not be guaranteed.
    */
   unsigned int index;
};

/**
 * @struct Efl_Model_Children_Event
 */
typedef struct _Efl_Model_Children_Event Efl_Model_Children_Event;


#ifdef __cplusplus
}
#endif
#endif
