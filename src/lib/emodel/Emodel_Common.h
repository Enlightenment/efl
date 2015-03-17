#ifndef _EMODEL_COMMON_H
#define _EMODEL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum _Emodel_Load_Status
 * XXX/TODO/FIXME: Remove this enum (and possibly other data) from here
 * as soon as eolian translates these data types in .eo's.
 */
enum _Emodel_Load_Status
  {
     EMODEL_LOAD_STATUS_ERROR = 0,
     EMODEL_LOAD_STATUS_LOADING_PROPERTIES = (1 << 0),
     EMODEL_LOAD_STATUS_LOADING_CHILDREN =   (1 << 1),
     EMODEL_LOAD_STATUS_LOADING = (1 << 0) | (1 << 1),

     EMODEL_LOAD_STATUS_LOADED_PROPERTIES = (1 << 2),
     EMODEL_LOAD_STATUS_LOADED_CHILDREN =   (1 << 3),
     EMODEL_LOAD_STATUS_LOADED = (1 << 2) | (1 << 3),

     EMODEL_LOAD_STATUS_UNLOADING = (1 << 4),
     EMODEL_LOAD_STATUS_UNLOADED = (1 << 5)
  };
/**
 * @typedef Emodel_Load_Status
 */
typedef enum _Emodel_Load_Status Emodel_Load_Status;

/**
 * @struct _Emodel_Load
 * Structure to hold Emodel_Load_Status enum
 * (and possible other data) to avoid ABI break.
 */
struct _Emodel_Load
  {
     Emodel_Load_Status status;
     /* add more data below here if necessary */
  };

/**
 * @typedef Emodel_Load
 */
typedef struct _Emodel_Load Emodel_Load;

/**
 * @struct _Emodel_Property_Event
 */
struct _Emodel_Property_Event
{
   const Eina_Array *changed_properties; /**< array of property name */
   const Eina_Array *invalidated_properties; /**< array of property name */
};

/**
 * @typedef Emodel_Property_Event
 */
typedef struct _Emodel_Property_Event Emodel_Property_Event;

/**
 * @struct _Emodel_Children_Event
 * Every time a child id added the event
 * EMODEL_EVENT_CHILD_ADDED is dispatched
 * passing along this structure.
 */
struct _Emodel_Children_Event
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
 * @struct Emodel_Children_Event
 */
typedef struct _Emodel_Children_Event Emodel_Children_Event;


#ifdef __cplusplus
}
#endif
#endif
