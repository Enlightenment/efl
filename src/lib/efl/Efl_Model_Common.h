#ifndef _EFL_MODEL_COMMON_H
#define _EFL_MODEL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum _Efl_Model_Load_Status
 * XXX/TODO/FIXME: Remove this enum (and possibly other data) from here
 * as soon as eolian translates these data types in .eo's.
 */
enum _Efl_Model_Load_Status
  {
     EFL_MODEL_LOAD_STATUS_ERROR = 0,
     EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES = (1 << 0),
     EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN =   (1 << 1),
     EFL_MODEL_LOAD_STATUS_LOADING = (1 << 0) | (1 << 1),

     EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES = (1 << 2),
     EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN =   (1 << 3),
     EFL_MODEL_LOAD_STATUS_LOADED = (1 << 2) | (1 << 3),

     EFL_MODEL_LOAD_STATUS_UNLOADING = (1 << 4),
     EFL_MODEL_LOAD_STATUS_UNLOADED = (1 << 5)
  };
/**
 * @typedef Efl_Model_Load_Status
 */
typedef enum _Efl_Model_Load_Status Efl_Model_Load_Status;

/**
 * @struct _Efl_Model_Load
 * Structure to hold Efl_Model_Load_Status enum
 * (and possible other data) to avoid ABI break.
 */
struct _Efl_Model_Load
  {
     Efl_Model_Load_Status status;
     /* add more data below here if necessary */
  };

/**
 * @typedef Efl_Model_Load
 */
typedef struct _Efl_Model_Load Efl_Model_Load;

/**
 * @struct _Efl_Model_Property_Event
 */
struct _Efl_Model_Property_Event
{
   const Eina_Array *changed_properties; /**< array of property name */
   const Eina_Array *invalidated_properties; /**< array of property name */
};

/**
 * @typedef Efl_Model_Property_Event
 */
typedef struct _Efl_Model_Property_Event Efl_Model_Property_Event;

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
