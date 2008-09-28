#ifndef _EDJE_EDIT_H
#define _EDJE_EDIT_H

#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EDJE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EDJE_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif


enum {
   EDJE_EDIT_IMAGE_COMP_RAW,
   EDJE_EDIT_IMAGE_COMP_USER,
   EDJE_EDIT_IMAGE_COMP_COMP,
   EDJE_EDIT_IMAGE_COMP_LOSSY
};

/**
 * @file
 * @brief Functions to deal with edje internal object. Don't use in standard
 * situations. The use of any of the edje_edit_* functions can break your
 * theme ability, remember that the program must be separated from the interface!
 *
 * The API can be used to query or set every part of an edje object in real time.
 * You can manage every aspect of parts, part states, programs, script and whatever
 * is contained in the edje file. For a reference of what all parameter means
 * look at the complete @ref edcref.
 *
 * All the functions that deals with part states include the state value inside
 * the returned strings (ex: "defaut 0.00"). For this reason there aren't
 * functions to set/get a state value, you have to pass the value inside the
 * name string (always in the form "state x.xx").
 *
 * Don't forget to free all the strings and the lists returned by any edje_edit_*()
 * functions using edje_edit_string_free() and edje_edit_string_list_free() when
 * you don't need anymore.
 *
 * Example: print all the part in a loaded edje_object
 * @code
 *  Evas_List *parts, *l;
 *
 *  parts = edje_edit_parts_list_get(edje_object);
 *  while(l = parts; l; l = l->next)
 *  {
 *     printf("Part: %s\n", (char*)l->data);
 *  }
 *  edje_edit_string_list_free(parts);
 * @endcode
 *
 * Example: Change the color of a rect inside an edje file
 * @code
 * Evas_Object *edje;
 *
 * edje = edje_object_add(evas);
 * edje_object_file_set(edje,"edj/file/name", "group to load");
 * edje_edit_state_color_set(edje, "MyRectName", "default 0.00", 255, 255, 0, 255);
 * edje_edit_save(edje);
 * @endcode
 *
*/


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/**************************   GENERAL API   ***********************************/
/******************************************************************************/
/** @name General API
 *  General functions that don't fit in other cateories.
 */ //@{

/** Free a generic Evas_List of (char *) allocated by an edje_edit_*_get() function.*/
EAPI void
edje_edit_string_list_free(
   Evas_List *lst          ///< The list to free. Will also free all the strings.
);

/** Free a generic string (char *) allocated by an edje_edit_*_get() function.*/
EAPI void
edje_edit_string_free(
   const char *str         ///< The string to free.
);

/**Save the modified edje object back to his file.
 * Use this function when you are done with your editing, all the change made
 * to the current loaded group will be saved back to the original file.
 *
 * NOTE: for now this as 2 limitations
 *    -the saved edje file cannot be decompiled anymore
 *    -you will lost your #define in the edc source
 */
EAPI int                   ///@return 1 on success, 0 on failure
edje_edit_save(
   Evas_Object *obj        ///< The edje object to save
);

/**Print on standard output many information about the internal status
 * of the edje object.
 * This is probably only usefull to debug.
 */
EAPI void
edje_edit_print_internal_status(
   Evas_Object *obj        ///< The edje object to inspect
);

//@}
/******************************************************************************/
/**************************   GROUPS API   ************************************/
/******************************************************************************/
/** @name Groups API
 *  Functions to deal with groups property (see @ref edcref).
 */ //@{

/**Create a new empty group in the given edje.
 * If a group with the same name exist none is created.
 */
EAPI unsigned char         ///@return 1 on success, 0 on failure
edje_edit_group_add(
   Evas_Object *obj,       ///< The edje object
   const char  *name       ///< The name for the new empty group
);

/**Delete the current group from the given edje.
 * You can only delete the currently loaded group.
 * All the parts and the programs inside the group will be deleted as well,
 * but not image or font embedded in the edje.
 */
EAPI unsigned char         ///@return 1 on success, 0 on failure
edje_edit_group_del(
   Evas_Object *obj        ///< The edje object
);

/**Check if a group with the given name exist in the edje.
 */
EAPI unsigned char         ///< 1 if the group exist, 0 otherwise.
edje_edit_group_exist(
   Evas_Object *obj,       ///< The edje object
   const char *group       ///< The name of the group
);

/**Set a new name for the current open group.
 * You can only rename a group that is currently loaded
 * Note that the relative getter function don't exist as it don't make sense ;)
 */
EAPI unsigned char         ///@return 1 on success, 0 on failure
edje_edit_group_name_set(
   Evas_Object *obj,       ///< The edje object
   const char  *new_name   ///< The new name for the group
);
/**Get the group min width*/
EAPI int                  ///@return The minw value or -1 on errors
edje_edit_group_min_w_get(
   Evas_Object *obj       ///< The edje object
);
/**Set the group min width*/
EAPI void
edje_edit_group_min_w_set(
   Evas_Object *obj,       ///< The edje object
   int w                   ///< The new group minimum width in pixel
);
/**Get the group min height*/
EAPI int                  ///@return The minh value or -1 on errors
edje_edit_group_min_h_get(
   Evas_Object *obj       ///< The edje object
);
/**Set the group min height*/
EAPI void
edje_edit_group_min_h_set(
   Evas_Object *obj,       ///< The edje object
   int h                   ///< The new group minimum height in pixel
);
/**Get the group max width*/
EAPI int                  ///@return The maxw value or -1 on errors
edje_edit_group_max_w_get(
   Evas_Object *obj       ///< The edje object
);
/**Set the group max width*/
EAPI void
edje_edit_group_max_w_set(
   Evas_Object *obj,       ///< The edje object
   int w                   ///< The new group maximum width in pixel
);
/**Get the group max height*/
EAPI int                  ///@return The maxh value or -1 on errors
edje_edit_group_max_h_get(
   Evas_Object *obj       ///< The edje object
);
/**Set the group max height*/
EAPI void
edje_edit_group_max_h_set(
   Evas_Object *obj,       ///< The edje object
   int h                   ///< The new group maximum height in pixel
);

//@}
/******************************************************************************/
/**************************   DATA API   **************************************/
/******************************************************************************/
/** @name Data API
 *  Functions to deal with data embedded in the edje (see @ref edcref).
 */ //@{

/** Retrieves a list with the item names inside the data block **/
EAPI Evas_List *          ///@return An Evas_List* of string (char *)containing all the data names.
edje_edit_data_list_get(
   Evas_Object *obj       ///< The edje object
);

/**Create a new data object in the given edje
 * If another data with the same name exists nothing is created and FALSE is returned.
 */
EAPI unsigned char         ///@return TRUE on success
edje_edit_data_add(
   Evas_Object *obj,       ///< The edje object
   const char *itemname,   ///< The name for the new data
   const char *value       ///< The value for the new data
);

/**Delete the given data object from edje */
EAPI unsigned char         ///@return TRUE on success
edje_edit_data_del(
   Evas_Object *obj,       ///< The edje object
   const char *itemname    ///< The name of the data to remove
);

/** Get the data associated with the given itemname **/
EAPI const char *         ///@return The data value
edje_edit_data_value_get(
   Evas_Object * obj,     ///< The edje object
   char *itemname         ///< The name of the data item
);

/** Set the data associated with the given itemname **/
EAPI unsigned char        ///@return TRUE on success
edje_edit_data_value_set(
   Evas_Object * obj,     ///< The edje object
   const char *itemname,  ///< The name of the data item
   const char *value      ///< The new value to set
);

/** Change the name of the given data object */
EAPI unsigned char        ///@return TRUE on success
edje_edit_data_name_set(
   Evas_Object *obj,     ///< The edje object
   const char *itemname, ///< The name of the data item
   const char *newname   ///< The new name to set
);

//@}
/******************************************************************************/
/***********************   COLOR CLASSES API   ********************************/
/******************************************************************************/
/** @name Color Classes API
 *  Functions to deal with Color Classes (see @ref edcref).
 */ //@{

/** Get the list of all the Color Classes in the given edje object.
 *  Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Evas_List *           ///@return An Evas_List* of string (char *)containing all the classes names.
edje_edit_color_classes_list_get(
   Evas_Object * obj       ///< The edje object
);

/** Create a new color class object in the given edje
 *  If another class with the same name exists nothing is created and FALSE is returned.
 */
EAPI unsigned char        ///@return TRUE on success
edje_edit_color_class_add(
   Evas_Object *obj,      ///< The edje object
   const char *name       ///< The name of the new color class
);

/** Delete the given class object from edje */
EAPI unsigned char        ///@return TRUE on success
edje_edit_color_class_del(
   Evas_Object *obj,      ///< The edje object
   const char *name       ///< The name of the color class to delete
);

/** Get all the colors that compose the class.
 *  You can pass NULL to colors you are not intrested in
 */
EAPI unsigned char         ///@return TRUE on success
edje_edit_color_class_colors_get(
   Evas_Object *obj,       ///< The edje object
   const char *class_name, ///< The name of the color class
   int *r,  ///< Where to store the red component of the standard color
   int *g,  ///< Where to store the green component of the standard color
   int *b,  ///< Where to store the blue component of the standard color
   int *a,  ///< Where to store the alpha component of the standard color
   int *r2, ///< Where to store the red component of the second color
   int *g2, ///< Where to store the green component of the second color
   int *b2, ///< Where to store the green component of the second color
   int *a2, ///< Where to store the green component of the second color
   int *r3, ///< Where to store the red component of the third color
   int *g3, ///< Where to store the green component of the third color
   int *b3, ///< Where to store the blue component of the third color
   int *a3  ///< Where to store the alpha component of the third color
);

/** Set the colors for the given color class.
 *  If you set a color to -1 it will not be touched
 */
EAPI unsigned char         ///@return TRUE on success
edje_edit_color_class_colors_set(
   Evas_Object *obj,       ///< The edje object
   const char *class_name, ///< The name of the color class
   int r,    ///< The red component of the standard color
   int g,    ///< The green component of the standard color
   int b,    ///< The blue component of the standard color
   int a,    ///< The alpha component of the standard color
   int r2,   ///< The red component of the second color
   int g2,   ///< The green component of the second color
   int b2,   ///< The blue component of the second color
   int a2,   ///< The alpha component of the second color
   int r3,   ///< The red component of the third color
   int g3,   ///< The green component of the third color
   int b3,   ///< The blue component of the third color
   int a3    ///< The alpha component of the third color
);

/** Change the name of a color class */
EAPI unsigned char        ///@return TRUE on success
edje_edit_color_class_name_set(
   Evas_Object *obj,      ///< The edje object
   const char *name,      ///< The name of the color class
   const char *newname    ///< The new name to assign
);


//@}
/******************************************************************************/
/**************************   PARTS API   *************************************/
/******************************************************************************/
/** @name Parts API
 *  Functions to deal with part objects (see @ref edcref).
 */ //@{

/**Get the list of all the parts in the given edje object.
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Evas_List *           ///@return An Evas_List* of string (char *)containing all the part names.
edje_edit_parts_list_get(
   Evas_Object *obj        ///< The edje object
);

/**Create a new part in the given edje
 * If another part with the same name just exists nothing is created and FALSE is returned.
 * Note that this function also create a default description for the part.
 */
EAPI unsigned char         ///@return TRUE on success, FALSE if the part can't be created
edje_edit_part_add(
   Evas_Object *obj,       ///< The edje object
   const char *name,       ///< The name for the new part
   unsigned char type      ///< The type of the part to create (One of: EDJE_PART_TYPE_NONE, EDJE_PART_TYPE_RECTANGLE, EDJE_PART_TYPE_TEXT,EDJE_PART_TYPE_IMAGE, EDJE_PART_TYPE_SWALLOW, EDJE_PART_TYPE_TEXTBLOCK,EDJE_PART_TYPE_GRADIENT or EDJE_PART_TYPE_GROUP)
);

/**Delete the given part from the edje
 * All the reference to this part will be zeroed.
 * A group must have at least one part, so it's not possible to
 * remove the last remaining part.
 */
EAPI unsigned char         ///@return 1 on success, 0 if the part can't be removed
edje_edit_part_del(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part to remove
);

/**Check if a part with the given name exist in the edje object.
 */
EAPI unsigned char         ///< 1 if the part exist, 0 otherwise.
edje_edit_part_exist(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Move the given part below the previous one.*/
EAPI unsigned char         ///@return 1 on success, 0 on failure
edje_edit_part_restack_below(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part to restack
);

/**Move the given part above the next one.*/
EAPI unsigned char         ///@return 1 on success, 0 on failure
edje_edit_part_restack_above(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part to restack
);

/**Set a new name for part.
 * Note that the relative getter function don't exist as it don't make sense ;)
 */
EAPI unsigned char         ///@return 1 on success, 0 on failure
edje_edit_part_name_set(
   Evas_Object *obj,       ///< The edje object
   const char  *part,      ///< The name of the part to rename
   const char  *new_name   ///< The new name for the part
);

/**Get the type of a part */
EAPI unsigned char         ///@return One of: EDJE_PART_TYPE_NONE, EDJE_PART_TYPE_RECTANGLE, EDJE_PART_TYPE_TEXT,EDJE_PART_TYPE_IMAGE, EDJE_PART_TYPE_SWALLOW, EDJE_PART_TYPE_TEXTBLOCK,EDJE_PART_TYPE_GRADIENT or EDJE_PART_TYPE_GROUP
edje_edit_part_type_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Get the clip_to part.
 * NULL is returned on errors and if the part don't have a clip.
 */
EAPI const char *          ///@return The name of the part to clip part to. Use edje_edit_string_free() when you don't need it anymore.
edje_edit_part_clip_to_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Set a part to clip part to.
 * If clip_to is NULL then the clipping value will be cancelled (unset clipping)
 */
EAPI unsigned char         ///@return 1 on success, 0 otherwise.
edje_edit_part_clip_to_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *clip_to     ///< The name of the part to clip part to.
);

/**Get the source of part.
 * Source is used in part of type EDJE_PART_TYPE_GROUP to specify the group to
 * 'autoswallow' inside the given part.
 */
EAPI const char *          ///@return The name of the group to source the given part. Use edje_edit_string_free() when you don't need it anymore.
edje_edit_part_source_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Set the source of part.
 * Source is used in part of type EDJE_PART_TYPE_GROUP to specify the group to
 * 'autoswallow' inside the given part.
 * NOTE: This is not applied now. You must reload the edje to see the change.
 */
EAPI unsigned char         ///@return 1 on success, 0 otherwise.
edje_edit_part_source_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *source      ///< The name of the group to autoswallow in the given part
);

/**Get the effect for a given part. */
EAPI unsigned char         ///@return One of: EDJE_TEXT_EFFECT_NONE, _PLAIN, _OUTLINE, _SOFT_OUTLINE, _SHADOW, _SOFT_SHADOW, _OUTLINE_SHADOW, _OUTLINE_SOFT_SHADOW, _FAR_SHADOW, _FAR_SOFT_SHADOW, _GLOW.
edje_edit_part_effect_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Set the effect for a given part. */
EAPI void
edje_edit_part_effect_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   unsigned char effect    ///< The effect to set. See edje_edit_part_effect_get() for possible value
);

/**Get the current selected state in part. */
EAPI const char *          ///@return The name of the selected state including the float value. Use edje_edit_string_free() when you don't need it anymore.
edje_edit_part_selected_state_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Set the current state in part.*/
EAPI unsigned char         ///@return 1 on success, 0 otherwise.
edje_edit_part_selected_state_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the state to set (including the float value)
);

/**Get mouse_events for part.*/
EAPI unsigned char         ///@return 1 if part accept mouse events, 0 if not
edje_edit_part_mouse_events_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Set mouse_events for part.*/
EAPI void
edje_edit_part_mouse_events_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   unsigned char mouse_events ///< If set to 1 part will accept mouse events, 0 to ignore all mouse events from part.
);

/**Get repeat_events for part.*/
EAPI unsigned char         ///@return 1 if part will pass all events to the other parts, 0 if not
edje_edit_part_repeat_events_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Set repeat_events for part. */
EAPI void
edje_edit_part_repeat_events_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   unsigned char repeat_events /**< If set to 1 part will repeat
                                 * all the received mouse events to other parts.
                                 * If set to 0 the events received will not propagate to other parts.*/
);

/**Get ignore_flags for part.*/
EAPI Evas_Event_Flags        ///@return event flags ignored
edje_edit_part_ignore_flags_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Set repeat_events for part. */
EAPI void
edje_edit_part_ignore_flags_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   Evas_Event_Flags ignore_flags ///< event flags to be ignored
);


//@}
/******************************************************************************/
/**************************   STATES API   ************************************/
/******************************************************************************/
/** @name States API
 *  Functions to deal with part states (see @ref edcref).
 */ //@{

/**Get the list of all the states in the given part.*/
EAPI Evas_List *           /**@return An Evas_List* of string (char *)containing all the states names found
                            * in part, including the float value (ex: "default 0.00").
                            * Use edje_edit_string_list_free() when you don't need it anymore. */
edje_edit_part_states_list_get(
   Evas_Object *obj,       ///< The edje object
   const char *part        ///< The name of the part
);

/**Set a new name for the given state in the given part.
 * Note that state and new_name must include the floating value inside the string (ex. "default 0.00")
 */
EAPI int
edje_edit_state_name_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part that contain state
   const char *state,      ///< The current name of the state
   const char *new_name    ///< The new name to assign (including the value)
);

/**Create a new state to the give part
 */
EAPI void
edje_edit_state_add(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *name        ///< The name for the new state (not including the state value)
);

/**Delete the given part state from the edje
 */
EAPI void
edje_edit_state_del(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part that contain state
   const char *state       ///< The current name of the state (including the state value)
);

/**Check if a part state with the given name exist.
 */
EAPI unsigned char         ///< 1 if the part state exist, 0 otherwise.
edje_edit_state_exist(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the state to check (including the state value)
);

/**Get the rel1 relative x value of state*/
EAPI double                ///@return The 'rel1 relative X' value of the part state
edje_edit_state_rel1_relative_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the part state (ex. "default 0.00")
);
/**Get the rel1 relative y value of state*/
EAPI double                ///@return The 'rel1 relative Y' value of the part state
edje_edit_state_rel1_relative_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the rel2 relative x value of state*/
EAPI double                ///@return The 'rel2 relative X' value of the part state
edje_edit_state_rel2_relative_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the rel2 relative y value of state*/
EAPI double                ///@return The 'rel2 relative Y' value of the part state
edje_edit_state_rel2_relative_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the rel1 relative x value of state*/
EAPI void
edje_edit_state_rel1_relative_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new 'rel1 relative X' value to set
);
/**Set the rel1 relative y value of state*/
EAPI void
edje_edit_state_rel1_relative_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double y                ///< The new 'rel1 relative Y' value to set
);
/**Set the rel2 relative x value of state*/
EAPI void
edje_edit_state_rel2_relative_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new 'rel2 relative X' value to set
);
/**Set the rel2 relative y value of state*/
EAPI void
edje_edit_state_rel2_relative_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double y                ///< The new 'rel2 relative Y' value to set
);

/**Get the rel1 offset x value of state*/
EAPI int                   /// @return The 'rel1 offset X' value of the part state
edje_edit_state_rel1_offset_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the rel1 offset y value of state*/
EAPI int                   /// @return The 'rel1 offset Y' value of the part state
edje_edit_state_rel1_offset_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the rel2 offset x value of state*/
EAPI int                   /// @return The 'rel2 offset X' value of the part state
edje_edit_state_rel2_offset_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the rel2 offset y value of state*/
EAPI int                   /// @return The 'rel2 offset Y' value of the part state
edje_edit_state_rel2_offset_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the rel1 offset x value of state*/
EAPI void
edje_edit_state_rel1_offset_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new 'rel1 offset X' value to set
);
/**Get the rel1 offset y value of state*/
EAPI void
edje_edit_state_rel1_offset_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double y                ///< The new 'rel1 offset Y' value to set
);
/**Get the rel2 offset x value of state*/
EAPI void
edje_edit_state_rel2_offset_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new 'rel2 offset X' value to set
);
/**Get the rel2 offset y value of state*/
EAPI void
edje_edit_state_rel2_offset_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double y                ///< The new 'rel2 offset Y' value to set
);

/**Get the part name rel1x is relative to. The function return NULL if the part is relative to the whole interface.*/
EAPI const char *          ///@return The name of the part to apply the relativity
edje_edit_state_rel1_to_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the part name rel1y is relative to. The function return NULL if the part is relative to the whole interface.*/
EAPI const char *          ///@return The name of the part to apply the relativity
edje_edit_state_rel1_to_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the part name rel2x is relative to. The function return NULL if the part is relative to the whole interface.*/
EAPI const char *         ///@return The name of the part to apply the relativity
edje_edit_state_rel2_to_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the part name rel2y is relative to. The function return NULL if the part is relative to the whole interface.*/
EAPI const char *         ///@return The name of the part to apply the relativity
edje_edit_state_rel2_to_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the part rel1x is relative to. Set rel_to to NULL make the part relative to the whole interface.*/
EAPI void
edje_edit_state_rel1_to_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *rel_to      ///< The name of the part that is used as container/parent
);
/**Set the part rel1y is relative to. Set rel_to to NULL make the part relative to the whole interface.*/
EAPI void
edje_edit_state_rel1_to_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *rel_to      ///< The name of the part that is used as container/parent
);
/**Set the part rel2x is relative to. Set rel_to to NULL make the part relative to the whole interface.*/
EAPI void
edje_edit_state_rel2_to_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *rel_to      ///< The name of the part that is used as container/parent
);
/**Set the part rel2y is relative to. Set rel_to to NULL make the part relative to the whole interface.*/
EAPI void
edje_edit_state_rel2_to_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *rel_to      ///< The name of the part that is used as container/parent
);

/**Get the color of a part state. Pass NULL to any of [r,g,b,a] to get only the others.*/
EAPI void
edje_edit_state_color_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int *r,                 ///< A pointer to store the red value
   int *g,                 ///< A pointer to store the green value
   int *b,                 ///< A pointer to store the blue value
   int *a                  ///< A pointer to store the alpha value
);
/**Get the color2 of a part state. Pass NULL to any of [r,g,b,a] to get only the others.*/
EAPI void
edje_edit_state_color2_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int *r,                 ///< A pointer to store the red value
   int *g,                 ///< A pointer to store the green value
   int *b,                 ///< A pointer to store the blue value
   int *a                  ///< A pointer to store the alpha value
);
/**Get the color3 of a part state. Pass NULL to any of [r,g,b,a] to get only the others.*/
EAPI void
edje_edit_state_color3_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int *r,                 ///< A pointer to store the red value
   int *g,                 ///< A pointer to store the green value
   int *b,                 ///< A pointer to store the blue value
   int *a                  ///< A pointer to store the alpha value
);

/**Set the color of a part state. Pass -1 to any of [r,g,b,a] to leave the value untouched.*/
EAPI void
edje_edit_state_color_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int r,                  ///< The red value of the color
   int g,                  ///< The green value of the color
   int b,                  ///< The blue value of the color
   int a                   ///< The alpha value of the color
);
/**Set the color2 of a part state. Pass -1 to any of [r,g,b,a] to leave the value untouched.*/
EAPI void
edje_edit_state_color2_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int r,                  ///< The red value of the color
   int g,                  ///< The green value of the color
   int b,                  ///< The blue value of the color
   int a                   ///< The alpha value of the color
);
/**Set the color3 of a part state. Pass -1 to any of [r,g,b,a] to leave the value untouched.*/
EAPI void
edje_edit_state_color3_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int r,                  ///< The red value of the color
   int g,                  ///< The green value of the color
   int b,                  ///< The blue value of the color
   int a                   ///< The alpha value of the color
);

/**Get the align_x value of a part state.*/
EAPI double                ///@return The horizontal align value for the given state
edje_edit_state_align_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the align_y value of a part state.*/
EAPI double                ///@return The vertical align value for the given state
edje_edit_state_align_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Set the align_x value of a part state.*/
EAPI void
edje_edit_state_align_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double align            ///< The new horizontal align to set
);
/**Set the align_y value of a part state.*/
EAPI void
edje_edit_state_align_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double align            ///< The new vertical align to set
);
/**Get the min_w value of a part state.*/
EAPI int                   ///@return The minimum width of a part state
edje_edit_state_min_w_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Set the min_w value of a part state.*/
EAPI void
edje_edit_state_min_w_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int min_w               ///< The new minimum width to set for the part state
);
/**Get the min_h value of a part state.*/
EAPI int                   ///@return The minimum height of a part state
edje_edit_state_min_h_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Set the min_h value of a part state.*/
EAPI void
edje_edit_state_min_h_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int min_h               ///< The new minimum height to set for the part state
);

/**Get the max_w value of a part state.*/
EAPI int                   ///@return The maximum width of a part state
edje_edit_state_max_w_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Set the max_w value of a part state.*/
EAPI void
edje_edit_state_max_w_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int max_w               ///< The new maximum width to set for the part state
);
/**Get the max_h value of a part state.*/
EAPI int                   ///@return The maximum height of a part state
edje_edit_state_max_h_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Set the max_h value of a part state.*/
EAPI void
edje_edit_state_max_h_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int max_h               ///< The new maximum height to set for the part state
);

/**Get the minimum aspect value of a part state.*/
EAPI double                ///@return The aspect minimum value of a part state
edje_edit_state_aspect_min_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Get the maximum aspect value of a part state.*/
EAPI double                ///@return The aspect maximum value of a part state
edje_edit_state_aspect_max_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Set the minimum aspect value of a part state.*/
EAPI void
edje_edit_state_aspect_min_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double aspect           ///< The new minimum aspect value to set
);
/**Set the maximum aspect value of a part state.*/
EAPI void
edje_edit_state_aspect_max_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double aspect           ///< The new maximum aspect value to set
);
/**Get the aspect preference value of a part state.*/
EAPI unsigned char         ///@return The aspect preference (0=none, 1=vertical, 2=horizontal, 3=both)
edje_edit_state_aspect_pref_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);
/**Set the aspect preference value of a part state.*/
EAPI void
edje_edit_state_aspect_pref_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   unsigned char pref      ///< The new aspect preference to set (0=none, 1=vertical, 2=horizontal, 3=both)
);

/**Get the fill origin relative x value of a part state.*/
EAPI double                ///@return The fill offset x relative to area
edje_edit_state_fill_origin_relative_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the fill origin relative y value of a part state.*/
EAPI double                ///@return The fill origin y relative to area
edje_edit_state_fill_origin_relative_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the fill origin offset x value of a part state.*/
EAPI int                   ///@return The fill origin offset x relative to area
edje_edit_state_fill_origin_offset_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the fill origin offset y value of a part state.*/
EAPI int                   ///@return The fill origin offset y relative to area
edje_edit_state_fill_origin_offset_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the fill origin relative x value of a part state.*/
EAPI void
edje_edit_state_fill_origin_relative_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new value to set
);

/**Set the fill origin relative y value of a part state.*/
EAPI void
edje_edit_state_fill_origin_relative_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new value to set
);

/**Set the fill origin offset x value of a part state.*/
EAPI void
edje_edit_state_fill_origin_offset_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new value to set
);

/**Set the fill origin offset x value of a part state.*/
EAPI void
edje_edit_state_fill_origin_offset_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double y                ///< The new value to set
);

/**Get the fill size relative x value of a part state.*/
EAPI double                ///@return The fill size offset x relative to area
edje_edit_state_fill_size_relative_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the fill size relative y value of a part state.*/
EAPI double                ///@return The fill size y relative to area
edje_edit_state_fill_size_relative_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the fill size offset x value of a part state.*/
EAPI int                    ///@return The fill size offset x relative to area
edje_edit_state_fill_size_offset_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the fill size offset y value of a part state.*/
EAPI int                    ///@return The fill size offset y relative to area
edje_edit_state_fill_size_offset_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the fill size relative x value of a part state.*/
EAPI void
edje_edit_state_fill_size_relative_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new value to set
);

/**Set the fill size relative y value of a part state.*/
EAPI void
edje_edit_state_fill_size_relative_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new value to set
);

/**Set the fill size offset x value of a part state.*/
EAPI void
edje_edit_state_fill_size_offset_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double x                ///< The new value to set
);

/**Set the fill size offset x value of a part state.*/
EAPI void
edje_edit_state_fill_size_offset_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double y                ///< The new value to set
);

/**Get the visibility of a part state.*/
EAPI unsigned char         ///@return TRUE if the state is visible
edje_edit_state_visible_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the visibility of a part state.*/
EAPI void
edje_edit_state_visible_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   unsigned char visible   ///< TRUE to set the state visible
);

/**Get the color class of the given part state. Remember to free the string with edje_edit_string_free()*/
EAPI const char*           ///@return The current color_class of the part state
edje_edit_state_color_class_get(
   Evas_Object *obj,      ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the color class for the given part state.*/
EAPI void
edje_edit_state_color_class_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *color_class ///< The new color_class to assign
);


//@}
/******************************************************************************/
/**************************   TEXT API   ************************************/
/******************************************************************************/
/** @name Text API
 *  Functions to deal with text objects (see @ref edcref).
 */ //@{

/**Get the text of a part state. Remember to free the returned string with edje_edit_string_free(). */
EAPI const char *          ///@return A newly allocated string containing the text for the given state
edje_edit_state_text_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the text of a part state.*/
EAPI void
edje_edit_state_text_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *text        ///< The new text to assign
);

/**Get the text size of a part state. The function will return -1 on errors.*/
EAPI int                   ///@return The font size in pixel or -1 on errors.
edje_edit_state_text_size_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the text size of a part state.*/
EAPI void
edje_edit_state_text_size_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int size                ///< The new font size to set (in pixel)
);

/**Get the text horizontal align of a part state. The value range is from 0.0(right) to 1.0(left)*/
EAPI double                ///@return The text align X value
edje_edit_state_text_align_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the text vertical align of a part state. The value range is from 0.0(top) to 1.0(bottom)*/
EAPI double                ///@return The text align Y value
edje_edit_state_text_align_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the text horizontal align of a part state. The value range is from 0.0(right) to 1.0(left)*/
EAPI void
edje_edit_state_text_align_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double align            ///< The new text align X value
);

/**Set the text vertical align of a part state. The value range is from 0.0(top) to 1.0(bottom)*/
EAPI void
edje_edit_state_text_align_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double align            ///< The new text align Y value
);

/**Get the list of all the fonts in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 */
EAPI Evas_List *          ///@return An Evas_List* of string (char *)containing all the fonts names found in the edje file.
edje_edit_fonts_list_get(
   Evas_Object *obj       ///< The edje object
);

/**Add a new ttf font to the edje file.
 * The newly created font will be available to all the groups in the edje, not only the current one.
 * If font can't be load FALSE is returned.
 */
EAPI unsigned char         ///@return TRUE on success or FALSE on failure
edje_edit_font_add(
   Evas_Object *obj,       ///< The edje object
   const char* path        ///< The file path to load the ttf font from
);

/**Get font name for a given part state. Remember to free the returned string using edje_edit_string_free().*/
EAPI const char *          ///@return The name of the font used in the given part state
edje_edit_state_font_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set font name for a given part state. */
EAPI void
edje_edit_state_font_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *font        ///< The name of the font to use in the given part state
);

//@}
/******************************************************************************/
/**************************   IMAGES API   ************************************/
/******************************************************************************/
/** @name Images API
 *  Functions to deal with image objects (see @ref edcref).
 */ //@{

/**Get the list of all the images in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 */
EAPI Evas_List *          ///@return An Evas_List* of string (char *)containing all the images names found in the edje file.
edje_edit_images_list_get(
   Evas_Object *obj       ///< The edje object
);

/**Add an new image to the image collection
 *
 * This function add the given image inside the edje. Don't add a new image part
 * but only put the image inside the edje file. It actually write directly to
 * the file so you don't have to save (and you can't undo!).
 * After you have to create a new image_part that use this image. Note that all
 * the parts in the edje share the same image collection, thus you can/must use
 * the same image for different part.
 *
 * The format of the image files that can be loaded depend on the evas engine on your system
 */
EAPI unsigned char         ///@return TRUE on success or FALSE on failure
edje_edit_image_add(
   Evas_Object *obj,       ///< The edje object
   const char* path        ///< The name of the image file to include in the edje
);

/**Get normal image name for a given part state. Remember to free the returned string using edje_edit_string_free().*/
EAPI const char *          ///@return The name of the image used by state
edje_edit_state_image_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set normal image for a given part state.*/
EAPI void
edje_edit_state_image_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *image       ///< The name of the image for the given state (must be an image contained in the edje file)
);

/**Get image id for a given image name. Could be usefull to directly load the image from the eet file.*/
EAPI int                   ///< The ID of the givan image name
edje_edit_image_id_get(
   Evas_Object *obj,       ///< The edje object
   const char *image_name
);

/**Get compression type for the given image.*/
EAPI int                  ///@return One of EDJE_EDIT_IMAGE_COMP_RAW, EDJE_EDIT_IMAGE_COMP_USER, EDJE_EDIT_IMAGE_COMP_COMP or EDJE_EDIT_IMAGE_COMP_LOSSY
edje_edit_image_compression_type_get(
   Evas_Object *obj,      ///< The edje object
   const char *image      ///< The name of the image
);

/**Get compression rate for the given image.*/
EAPI int                  ///@return the compression rate if the image is EDJE_EDIT_IMAGE_COMP_LOSSY. Or < 0 on errors
edje_edit_image_compression_rate_get(
   Evas_Object *obj,      ///< The edje object
   const char *image      ///< The name of the image
);

/**Get the image border of a part state. Pass NULL to any of [r,g,b,a] to get only the others.*/
EAPI void
edje_edit_state_image_border_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int *l,                 ///< A pointer to store the left value
   int *r,                 ///< A pointer to store the right value
   int *t,                 ///< A pointer to store the top value
   int *b                  ///< A pointer to store the bottom value
);

/**Set the image border of a part state. Pass -1 to any of [l,r,t,b] to leave the value untouched.*/
EAPI void
edje_edit_state_image_border_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int l,                  ///< The new left border (or -1)
   int r,                  ///< The new right border (or -1)
   int t,                  ///< The new top border (or -1)
   int b                   ///< The new bottom border (or -1)
);

/**Get the list of all the tweens images in the given part state.
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Evas_List *           ///@return A string list containing all the image name that form a tween animation in the given part state
edje_edit_state_tweens_list_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Add a new tween frame to the given part state
 * The tween param must be the name of an existing image.
 */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_tween_add(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *tween       ///< The name of the image to add.
);

/**Remove the first tween with the given name.
 * If none is removed the function return 0.
 * The image is not removed from the edje.
 */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_tween_del(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *tween       ///< The name of the image to remove from the tween list.
);

//@}
/******************************************************************************/
/*************************   SPECTRUM API   ***********************************/
/******************************************************************************/
/** @name Spectrum API
 *  Functions to manage spectrum (see @ref edcref).
 */ //@{

/**Get the list of all the spectrum in the given edje object.
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Evas_List *           ///@return An Evas_List* of string(char *) containing all the spectra names.
edje_edit_spectrum_list_get(
   Evas_Object *obj        ///< The edje object
);

/**Add a new spectra in the given edje object.*/
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_spectra_add(
   Evas_Object *obj,       ///< The edje object
   const char* name        ///< The name of the new spectra
);

/**Delete the given spectra from the edje object.*/
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_spectra_del(
   Evas_Object *obj,       ///< The edje object
   const char* spectra     ///< The name of the spectra to delete
);

/**Change the name of the given spectra.*/
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_spectra_name_set(
   Evas_Object *obj,       ///< The edje object
   const char* spectra,    ///< The current name of the spectra
   const char* name        ///< The new name to assign
);

/**Get the number of stops in the given spectra.*/
EAPI int                   ///@return The number of stops, or 0 on errors
edje_edit_spectra_stop_num_get(
   Evas_Object *obj,       ///< The edje object
   const char* spectra     ///< The name of the spectra
);

/**Set the number of stops in the given spectra.*/
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_spectra_stop_num_set(
   Evas_Object *obj,       ///< The edje object
   const char* spectra,    ///< The name of the spectra
   int num                 ///< The number of stops you want
);

/**Get the colors of the given stop.*/
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_spectra_stop_color_get(
   Evas_Object *obj,       ///< The edje object
   const char* spectra,    ///< The name of the spectra
   int stop_number,        ///< The number of the stop
   int *r,                 ///< Where to store the red color value
   int *g,                 ///< Where to store the green color value
   int *b,                 ///< Where to store the blue color value
   int *a,                 ///< Where to store the alpha color value
   int *d                  ///< Where to store the delta stop value
);

/**Set the colors of the given stop.*/
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_spectra_stop_color_set(
   Evas_Object *obj,       ///< The edje object
   const char* spectra,    ///< The name of the spectra
   int stop_number,        ///< The number of the stop
   int r,                  ///< The red color value to set
   int g,                  ///< The green color value to set
   int b,                  ///< The blue color value to set
   int a,                  ///< The alpha color value to set
   int d                   ///< The delta stop value to set
);


//@}
/******************************************************************************/
/*************************   GRADIENT API   ***********************************/
/******************************************************************************/
/** @name Gradient API
 *  Functions to deal with gradient objects (see @ref edcref).
 */ //@{

/**Get the type of gradient. Remember to free the string with edje_edit_string_free().*/
EAPI const char *          ///@return The type of gradient used in state
edje_edit_state_gradient_type_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the type of gradient.
 * Gradient type can be on of the following: linear, linear.diag, linear.codiag, radial, rectangular, angular, sinusoidal
 */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_type_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char *type        ///< The type of gradient to use
);

/**Get if the current gradient use the fill properties or the gradient_rel as params.*/
EAPI unsigned char         ///@return 1 if use fill, 0 if use gradient_rel
edje_edit_state_gradient_use_fill_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the spectra used by part state. Remember to free the string with edje_edit_string_free()*/
EAPI const char *          ///@return The spectra name used in state
edje_edit_state_gradient_spectra_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the spectra used by part state.*/
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_spectra_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   const char* spectra     ///< The spectra name to assign
);

/**Get the gradien rel1 relative x value */
EAPI double                ///@return The gradien rel1 relative x value
edje_edit_state_gradient_rel1_relative_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the gradien rel1 relative y value */
EAPI double                ///@return The gradien rel1 relative y value
edje_edit_state_gradient_rel1_relative_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the gradien rel2 relative x value */
EAPI double                ///@return The gradien rel2 relative x value
edje_edit_state_gradient_rel2_relative_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the gradien rel2 relative y value */
EAPI double                ///@return The gradien rel2 relative y value
edje_edit_state_gradient_rel2_relative_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the gradien rel1 relative x value */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_rel1_relative_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double val
);

/**Set the gradien rel1 relative y value */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_rel1_relative_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double val
);

/**Set the gradien rel2 relative x value */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_rel2_relative_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double val
);

/**Set the gradien rel2 relative y value */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_rel2_relative_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   double val
);

/**Get the gradien rel1 offset x value */
EAPI int                   ///@return The gradient rel1 offset x value
edje_edit_state_gradient_rel1_offset_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the gradien rel1 offset y value */
EAPI int                   ///@return The gradient rel1 offset y value
edje_edit_state_gradient_rel1_offset_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the gradien rel2 offset x value */
EAPI int                   ///@return The gradient rel2 offset x value
edje_edit_state_gradient_rel2_offset_x_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Get the gradien rel2 offset y value */
EAPI int                   ///@return The gradient rel2 offset y value
edje_edit_state_gradient_rel2_offset_y_get(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state       ///< The name of the 'part state' (ex. "default 0.00")
);

/**Set the gradien rel1 offset x value */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_rel1_offset_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int val
);

/**Set the gradien rel1 offset y value */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_rel1_offset_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int val
);

/**Set the gradien rel2 offset x value */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_rel2_offset_x_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int val
);

/**Set the gradien rel2 offset y value */
EAPI unsigned char         ///@return 1 on success, 0 otherwise
edje_edit_state_gradient_rel2_offset_y_set(
   Evas_Object *obj,       ///< The edje object
   const char *part,       ///< The name of the part
   const char *state,      ///< The name of the 'part state' (ex. "default 0.00")
   int val
);

//@}
/******************************************************************************/
/*************************   PROGRAMS API   ***********************************/
/******************************************************************************/
/** @name Programs API
 *  Functions to deal with programs (see @ref edcref).
 */ //@{

/**Get the list of all the programs in the given edje object.
 * @param obj The edje object
 * @return An Evas_List* of string (char *)containing all the program names.
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Evas_List *          ///@return A string list containing all the program names
edje_edit_programs_list_get(
   Evas_Object *obj       ///< The edje object
);

/**Add a new program to the edje file
 * If a program with the same name just exist the function will fail.
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_add(
   Evas_Object *obj,       ///< The edje object
   const char *name        ///< The name of the new program
);

/**Remove the given program from the edje file.
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_del(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The name of the program to remove
);

/**Check if a program with the given name exist in the edje object.
 */
EAPI unsigned char         ///< 1 if the program exist, 0 otherwise.
edje_edit_program_exist(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The name of the program
);

/**Run the given program. */
EAPI unsigned char         ///< 1 on success, 0 otherwise.
edje_edit_program_run(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The name of the program to execute
);

/**Set a new name for the given program */
EAPI unsigned char        ///@return 1 on success or 0 on errors
edje_edit_program_name_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *new_name    ///< The new name to assign
);

/**Get source of a given program. Remember to free the returned string using edje_edit_string_free().*/
EAPI const char *          ///@return The source value for prog
edje_edit_program_source_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set source of the given program. */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_source_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *source      ///< The new source value
);

/**Get signal of a given program. Remember to free the returned string using edje_edit_string_free().*/
EAPI const char *          ///@return The signal value for prog
edje_edit_program_signal_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set signal of the given program. */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_signal_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *signal      ///< The new signal value
);

/**Get in.from of a given program.*/
EAPI double                ///@return The delay
edje_edit_program_in_from_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set in.from of a given program.*/
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_in_from_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double seconds          ///< Number of seconds to delay the program execution
);

/**Get in.range of a given program.*/
EAPI double                ///@return The delay random
edje_edit_program_in_range_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set in.range of a given program.*/
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_in_range_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double seconds          ///< Max random number of seconds to delay
);

/**Get the action of a given program.
 * Action can be one of EDJE_ACTION_TYPE_NONE, _STATE_SET, ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, _DRAG_VAL_STEP, _DRAG_VAL_PAGE, _SCRIPT
 */
EAPI int                   ///@return The action type, or -1 on errors
edje_edit_program_action_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the action of a given program.
 * Action can be one of EDJE_ACTION_TYPE_NONE, _STATE_SET, ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, _DRAG_VAL_STEP, _DRAG_VAL_PAGE, _SCRIPT
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_action_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   int action              ///< The new action type
);

/**Get the list of the targets for the given program
 * Return a list of target name
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Evas_List*            ///@return An Evas_List of char*, or NULL on error
edje_edit_program_targets_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Add a new target name to the list of 'targets' in the given program.
 * If program action is EDJE_ACTION_TYPE_ACTION_STOP then 'target' must be an existing program name.
 * If action is EDJE_ACTION_TYPE_STATE_SET then 'target' must be an existing part name.
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_target_add(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *target      ///< The name of another program or another part
);

/**Clear the 'targets' list of the given program */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_targets_clear(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Get the list of action that will be run after the give program
 * Return a list of program name.
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Evas_List*            ///@return An Evas_List of char*, or NULL on error
edje_edit_program_afters_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Add a new program name to the list of 'afters' in the given program.
 * All the programs listed in 'afters' will be executed after program execution.
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_after_add(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *after       ///< The name of another program to add to the afters list
);

/**Clear the 'afters' list of the given program */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_afters_clear(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Get the state for the given program
 * In a STATE_SET action this is the name of state to set.
 * In a SIGNAL_EMIT action is the name of the signal to emit.
 */
EAPI const char*           ///@return The name of state for prog
edje_edit_program_state_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the state for the given program
 * In a STATE_SET action this is the name of state to set.
 * In a SIGNAL_EMIT action is the name of the signal to emit.
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_state_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *state       ///< The state to set (not including the state value)
);

/**Get the value of state for the given program.
 * In a STATE_SET action this is the value of state to set.
 * Not used on SIGNAL_EMIT action.
 */
EAPI double                ///@return The value of state for prog
edje_edit_program_value_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the value of state for the given program.
 * In a STATE_SET action this is the value of state to set.
 * Not used on SIGNAL_EMIT action.
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_value_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double value            ///< The value to set
);

/**Get the state2 for the given program
 * In a STATE_SET action is not used
 * In a SIGNAL_EMIT action is the source of the emitted signal.
 */
EAPI const char*           ///@return The source to emit for prog
edje_edit_program_state2_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the state2 for the given program
 * In a STATE_SET action is not used
 * In a SIGNAL_EMIT action is the source of the emitted signal.
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_state2_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   const char *state2      ///< The state2 value to set
);

/**Get the value of state2 for the given program.
 * I don't know what this is used for. :P
 */
EAPI double                ///@return The value of state2 for prog
edje_edit_program_value2_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the value2 of state for the given program.
 * This is used in DRAG_ACTION
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_value2_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double value            ///< The value to set
);

/**Get the type of transition to use when apply animations.
 * Can be one of: EDJE_TWEEN_MODE_NONE, EDJE_TWEEN_MODE_LINEAR, EDJE_TWEEN_MODE_SINUSOIDAL, EDJE_TWEEN_MODE_ACCELERATE or EDJE_TWEEN_MODE_DECELERATE.
 */
EAPI int                   ///@return The type of transition used by program
edje_edit_program_transition_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the type of transition to use when apply animations.
 * Can be one of: EDJE_TWEEN_MODE_NONE, EDJE_TWEEN_MODE_LINEAR, EDJE_TWEEN_MODE_SINUSOIDAL, EDJE_TWEEN_MODE_ACCELERATE or EDJE_TWEEN_MODE_DECELERATE.
 */
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_transition_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   int transition          ///< The transition type to set
);

/**Get the duration of the transition in seconds.*/
EAPI double                ///@return The duration of the transition
edje_edit_program_transition_time_get(
   Evas_Object *obj,       ///< The edje object
   const char *prog        ///< The program name
);

/**Set the duration of the transition in seconds.*/
EAPI unsigned char         ///@return 1 on success or 0 on errors
edje_edit_program_transition_time_set(
   Evas_Object *obj,       ///< The edje object
   const char *prog,       ///< The program name
   double seconds          ///< The duration of the transition (in seconds)
);

//@}
/******************************************************************************/
/**************************   SCRIPTS API   ***********************************/
/******************************************************************************/
/** @name Scripts API
 *  Functions to deal with embryo scripts (see @ref edcref).
 */ //@{
EAPI const char* edje_edit_script_get(Evas_Object *obj);



#ifdef __cplusplus
}
#endif

#endif
