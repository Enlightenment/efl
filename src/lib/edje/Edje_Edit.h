#ifndef _EDJE_EDIT_H
#define _EDJE_EDIT_H

#ifndef EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT
#error "Do not use the Edje_Edit API unless you know what you are doing. It's meant only for writing editors and nothing else."
#endif

#include <Edje.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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

/**
 * Compression type for the image.
 *
 * @ref edcref
 */
typedef enum _Edje_Edit_Image_Comp
{
   EDJE_EDIT_IMAGE_COMP_RAW,
   EDJE_EDIT_IMAGE_COMP_USER,
   EDJE_EDIT_IMAGE_COMP_COMP,
   EDJE_EDIT_IMAGE_COMP_LOSSY,
   EDJE_EDIT_IMAGE_COMP_LOSSY_ETC1,
   EDJE_EDIT_IMAGE_COMP_LOSSY_ETC2
} Edje_Edit_Image_Comp;

/**
 * Mode for a textblock part.
 *
 * @ref edcref
 */
typedef enum _Edje_Edit_Select_Mode
{
   EDJE_EDIT_SELECT_MODE_DEFAULT,
   EDJE_EDIT_SELECT_MODE_EXPLICIT
} Edje_Edit_Select_Mode;

/**
 * Sound type compression.
 *
 * @ref edcref
 */
typedef enum _Edje_Edit_Sound_Comp
{
   EDJE_EDIT_SOUND_COMP_NONE,
   EDJE_EDIT_SOUND_COMP_RAW,
   EDJE_EDIT_SOUND_COMP_COMP,
   EDJE_EDIT_SOUND_COMP_LOSSY,
   EDJE_EDIT_SOUND_COMP_AS_IS
} Edje_Edit_Sound_Comp;

/**
 * Mode for a textblock part.
 *
 * @ref edcref
 */
typedef enum _Edje_Edit_Entry_Mode
{
   EDJE_EDIT_ENTRY_MODE_NONE,
   EDJE_EDIT_ENTRY_MODE_PLAIN,
   EDJE_EDIT_ENTRY_MODE_EDITABLE,
   EDJE_EDIT_ENTRY_MODE_PASSWORD
} Edje_Edit_Entry_Mode;

/**
 * @typedef Edje_Edit_Script_Error
 *
 * This is structure used for the list of errors that resulted from the last
 * attempt to rebuild the Embryo script for the edited group.
 *
 * @see edje_edit_script_error_list_get()
 */
struct _Edje_Edit_Script_Error
{
   const char *program_name; /**< name of the script, if null then it is group shared script */
   int line; /**< Line of the error inside in scriptcode */
   const char *error_str; /**< Error Message */
};
typedef struct _Edje_Edit_Script_Error Edje_Edit_Script_Error;

/**
 * @typedef Edje_Part_Image_Use
 *
 * This is structure used for the list of group-part-state triplets where certain
 * image is being used and pointed.
 *
 * @see edje_edit_image_usage_list_get()
 * @see edje_edit_vector_usage_list_get()
 * @see edje_edit_image_usage_list_free()
 */
struct _Edje_Part_Image_Use
{
   const char *group; /**< name of group (or set) that use image */
   const char *part; /**< name of part that use image */
   struct {
      const char     *name; /**< name of the state */
      double         value; /**< value of the state (-1 if it is set) */
   } state; /**< structure that contain state's information */
};
typedef struct _Edje_Part_Image_Use Edje_Part_Image_Use;

/**
 * @typedef Edje_Edit_Limit
 *
 * This is structure used for list with the item names inside the limits block.
 *
 * @see edje_edit_group_limits_vertical_list_get()
 * @see edje_edit_group_limits_horizontal_list_get()
 * @see edje_edit_group_limits_vertical_del()
 * @see edje_edit_group_limits_horizontal_del()
 * @see edje_edit_group_limits_vertical_add()
 * @see edje_edit_group_limits_horizontal_add()
 * @see edje_edit_limits_list_free()
 */
struct _Edje_Edit_Limit
{
   Eina_Stringshare  *name; /**< name of the limit */
   int               value; /**< value of the limit */
};
typedef struct _Edje_Edit_Limit Edje_Edit_Limit;

/**
 * @file
 * @brief Functions to deal with edje internal object. Don't use in standard
 * situations. The use of any of the edje_edit_* functions can break your
 * theme ability, remember that the program must be separated from the interface!
 *
 * This was intended ONLY for use in an actual edje editor program. Unless
 * you are writing one of these, do NOT use this API here.
 *
 * The API can be used to query or set every part of an edje object in real time.
 * You can manage every aspect of parts, part states, programs, script and whatever
 * is contained in the edje file. For a reference of what all parameter means
 * look at the complete @ref edcref.
 *
 * Don't forget to free all the strings and the lists returned by any edje_edit_*()
 * functions using edje_edit_string_free() and edje_edit_string_list_free() when
 * you don't need anymore.
 *
 * Example: print all the part in a loaded edje_object
 * @code
 *  Eina_List *parts, *l;
 *  char *part;
 *
 *  parts = edje_edit_parts_list_get(edje_object);
 *  EINA_LIST_FOREACH(parts, l, part)
 *  {
 *     printf("Part: %s\n", part);
 *  }
 *  edje_edit_string_list_free(parts);
 * @endcode
 *
 * Example: Change the color of a rect inside an edje file
 * @code
 * Evas_Object *edje;
 *
 * edje = edje_edit_object_add(evas);
 * edje_object_file_set(edje, "edj/file/name", "group to load");
 * edje_edit_state_color_set(edje, "MyRectName", "default", 0.00, 255, 255, 0, 255);
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
/**
 * @name General API
 * General functions that don't fit in other categories.
 */ //@{

/**
 * @brief Adds an editable Edje object to the canvas.
 * An Edje_Edit object is, for the most part, a standard Edje object. Only
 * difference is you can use the Edje_Edit API on them.
 *
 * @param e Evas canvas where to add the object.
 *
 * @return An Evas_Object of type Edje_Edit, or NULL if an error occurred.
 */
EAPI Evas_Object * edje_edit_object_add(Evas *e);

/**
 * @brief Frees a generic Eina_List of (char *) allocated by an edje_edit_*_get() function.
 * @param lst List of strings to free.
 */
EAPI void edje_edit_string_list_free(Eina_List *lst);

/**
 * @brief Frees a generic string (char *) allocated by an edje_edit_*_get() function.
 * @param str String to free.
 */
EAPI void edje_edit_string_free(const char *str);

/**
 * @brief Gets the name of the program that compiled the edje file.
 * Can be 'edje_cc' or 'edje_edit'
 *
 * @param obj Object being edited.
 *
 * @return Compiler stored in the Edje file
 */
EAPI const char * edje_edit_compiler_get(Evas_Object *obj);

/**
 * @brief Saves the modified edje object back to his file.
 * Use this function when you are done with your editing, all the change made
 * to the current loaded group will be saved back to the original file.
 *
 * @note Source for the whole file will be auto generated and will overwrite
 * any previously stored source.
 *
 * @param obj Object to save back to the file it was loaded from.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @todo Add a way to check what the error actually was, the way Edje Load does.
 */
EAPI Eina_Bool edje_edit_save(Evas_Object *obj);

/**
 * @brief Saves every group back into the file.
 * @param obj Object to save.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see edje_edit_save()
 */
EAPI Eina_Bool edje_edit_save_all(Evas_Object *obj);

/**
 * @brief Saves every group into new file.
 * Use this function when you need clean eet dictionary in .edj file from
 * unnecessary text entries (e.g. names of deleted groups etc.).
 *
 * @param obj Object to save.
 * @param new_file_name Where to save object. File should not exist, otherwise
 * EINA_FALSE will be returned.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see edje_edit_save()
 */
EAPI Eina_Bool edje_edit_clean_save_as(Evas_Object *obj, const char* new_file_name);

/**
 * @brief Saves the group(s) back to the file, without generation source code.
 * This function saves changes in group(s) back into the edj file. Process of
 * saving takes a bit time in compare with @see edje_edit_save() and @see edje_edit_save_all(),
 * because this function DOES NOT generate source code for groups.
 *
 * @note With using this function all source code will be erased. And DOES NOT
 * generated new code. In attempt to decompile edj file, which was saved with
 * using this functions will unpacked only resources(like fonts, images, sounds).
 * If needed saving source code into file, please use  @see edje_edit_save() or
 * @see edje_edit_save_all().

 * @param obj Object to save back to the file it was loaded from.
 * @param current_group EINA_TRUE if needed save only group which loaded with obj,
 * or EINA_FALSE for save all groups, which exists in edj file.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_without_source_save(Evas_Object *obj, Eina_Bool current_group);

/**
 * @brief Prints on standard output many information about the internal status
 * of the edje object.
 *
 * This is probably only useful to debug.
 *
 * @param obj Object being edited.
 */
EAPI void edje_edit_print_internal_status(Evas_Object *obj);


//@}
/******************************************************************************/
/**************************   GROUPS API   ************************************/
/******************************************************************************/
/**
 * @name Groups API
 * Functions to deal with groups property (see @ref edcref).
 */ //@{

/**
 * @brief Adds an edje (empty) group to an edje object's group set.
 *
 * @param obj The pointer to edje object.
 * @param name The name of the group.
 *
 * @return @c EINA_TRUE If it could allocate memory to the part group added
 * or zero if not.
 *
 * This function adds, at run time, one more group, which will reside
 * in memory, to the group set found in the .edj file which @a obj was
 * loaded with. This group can be manipulated by other API functions,
 * like @c edje_edit_part_add(), for example. If desired, the new
 * group can be actually committed the respective .edj by use of @c
 * edje_edit_save().
 *
 */
EAPI Eina_Bool edje_edit_group_add(Evas_Object *obj, const char *name);

/**
 * @brief Copies whole group and all it's data into separate group.
 *
 * @param obj The pointer to edje object.
 * @param group_name The name of the group.
 * @param copy_name The name of the new group that is a copy.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * This function copy, at run time, a whole group, which will reside
 * in memory, to the group set found in the .edj file which @a obj was
 * loaded with. This group can be manipulated by other API functions,
 * like @c edje_edit_part_add(), for example.
 *
 * You need to save everything to make sure the file won't have broken
 * references the next time is loaded.
 * @see edje_edit_save_all(), edje_edit_without_source_save().
 *
 * @attention This group will copy the whole group and this operation can't be undone as all references to the group will be added to the file.
 * (for example all scripts will be written to the file directly)
 *
 */
EAPI Eina_Bool edje_edit_group_copy(Evas_Object *obj, const char *group_name, const char *copy_name);

/**
 * @brief Deletes the specified group from the edje file.
 *
 * @param obj The pointer to the edje object.
 * @param group_name Group to delete.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * This function deletes the given group from the file @a obj is set to. This
 * operation can't be undone as all references to the group are removed from
 * the file.
 * This function may fail if the group to be deleted is currently in use.
 * You need to save everything to make sure the file won't have broken
 * references the next time is loaded.
 * @see edje_edit_save_all(), edje_edit_without_source_save().
 *
 * @attention be careful, if you deleting group, it will delete all it's aliases also,
 * if you deleting alias, then it will delete alias only.
 *
 */
EAPI Eina_Bool edje_edit_group_del(Evas_Object *obj, const char *group_name);

/**
 * @brief Checks if a group with the given name exist in the edje.
 * @param obj Object being edited.
 * @param group Group name to check for.
 *
 * @return @c EINA_TRUE if group exists, @c EINA_FALSE if not.
 */
EAPI Eina_Bool edje_edit_group_exist(Evas_Object *obj, const char *group);

/**
 * @brief Sets a new name for the current open group.
 *
 * You can only rename a group that is currently loaded
 * Note that the relative getter function don't exist as it doesn't make sense ;)
 * @param obj Object being edited.
 * @param new_name New name for the group.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_name_set(Evas_Object *obj, const char *new_name);

/**
 * @brief Gets the group minimum width.
 *
 * @param obj Object being edited.
 *
 * @return The minimum width set for the group. -1 if an error occurred.
 */
EAPI int edje_edit_group_min_w_get(Evas_Object *obj);

/**
 * @brief Sets the group minimum width.
 *
 * @param obj Object being edited.
 * @param w New minimum width for the group.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_min_w_set(Evas_Object *obj, int w);

/**
 * @brief Gets the group minimum height.
 *
 * @param obj Object being edited.
 *
 * @return The minimum height set for the group. @c -1 if an error occurred.
 */
EAPI int edje_edit_group_min_h_get(Evas_Object *obj);

/**
 * @brief Sets the group minimum height.
 *
 * @param obj Object being edited.
 * @param h New minimum height for the group.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_min_h_set(Evas_Object *obj, int h);

/**
 * @brief Gets the group maximum width.
 *
 * @param obj Object being edited.
 *
 * @return The maximum width set for the group. @c -1 if an error occurred.
 */
EAPI int edje_edit_group_max_w_get(Evas_Object *obj);

/**
 * @brief Sets the group maximum width.
 *
 * @param obj Object being edited.
 * @param w New maximum width for the group.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_max_w_set(Evas_Object *obj, int w);

/**
 * @brief Gets the group maximum height.
 *
 * @param obj Object being edited.
 *
 * @return The maximum height set for the group. @c -1 if an error occurred.
 */
EAPI int edje_edit_group_max_h_get(Evas_Object *obj);

/**
 * @brief Sets the group maximum height.
 *
 * @param obj Object being edited.
 * @param h New maximum height for the group.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_max_h_set(Evas_Object *obj, int h);

/**
 * @brief Gets the group broadcast_signal.
 *
 * @param obj Object being edited.
 *
 * @return @c EINA_FALSE if group not accept broadcast signal, @c EINA_TRUE otherwise (Default to true since 1.1.).
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_group_broadcast_signal_get(Evas_Object *obj);

/**
 * @brief Sets the group broadcast signal.
 *
 * @param obj Object being edited.
 * @param bs @c EINA_TRUE if group will accept broadcast signal, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_group_broadcast_signal_set(Evas_Object *obj, Eina_Bool bs);

//@}


/**
 * @brief Retrieves a list with the item names inside the vertical limits block at the group level.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being a name of vertical limit in the limits block for the group.
 */
EAPI Eina_List * edje_edit_group_limits_vertical_list_get(Evas_Object *obj);

/**
 * @brief Deletes given pair name-value from the vertical limits block at the group level.
 *
 * @param obj Object being edited.
 * @param name Limit name.
 * @param value Limit value.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_limits_vertical_del(Evas_Object *obj, const char *name, int value);

/**
 * @brief Adds given pair name-value to the vertical limits block at the group level.
 *
 * @param obj Object being edited.
 * @param name Limit name.
 * @param value Limit value.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_limits_vertical_add(Evas_Object *obj, const char *name, int value);

/**
 * @brief Retrieves a list with the item names inside the horizontal limits block at the group level.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being a name of horizontal limit in the limits block for the group.
 */
EAPI Eina_List * edje_edit_group_limits_horizontal_list_get(Evas_Object *obj);

/**
 * @brief Deletes given pair name-value from the horizontal limits block at the group level.
 *
 * @param obj Object being edited.
 * @param name Limit name.
 * @param value Limit value.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_limits_horizontal_del(Evas_Object *obj, const char *name, int value);

/**
 * @brief Adds given pair name-value to the horizontal limits block at the group level.
 *
 * @param obj Object being edited.
 * @param name Limit name.
 * @param value Limit value.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_limits_horizontal_add(Evas_Object *obj, const char *name, int value);

/**
 @brief Frees an Eina_List of (Edje_Edit_List *) allocated by an edje_edit_limits_vertical_list_get() or edje_edit_limits_horizontal_list_get() functions.
 *
 * @param lst List to free.
 */
EAPI void edje_edit_limits_list_free(Eina_List *lst);

/******************************************************************************/
/**************************   ALIAS API   **************************************/
/******************************************************************************/
/**
 * @name Alias API
 * Functions to deal with aliases that just another names of the group in the edje (see @ref edcref).
 */ //@{

/**
 * @brief Retrieves a list of aliases for this group.
 * If given group name is an alias name then this function will return NULL.
 *
 * @attention After you done using returned list, please use edje_edit_string_list_free to free this list.
 *
 * @param obj Object being edited.
 * @param group_name Group name or alias.
 *
 * @return List of strings, each being a name of alias of given group or alias name.
 */
EAPI Eina_List * edje_edit_group_aliases_get(Evas_Object *obj, const char *group_name);

/**
 * @brief Checks if this group is an alias name.
 *
 * @param obj Object being edited.
 * @param alias_name Group name that is alias.
 *
 * @return @c EINA_TRUE if alias, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_alias_is(Evas_Object *obj, const char *alias_name);

/**
 * @brief Returns the main group name that is aliased by given alias name.
 *
 * @attention After you done using this string, please use edje_edit_string_free to free this string.
 *
 * @param obj Object being edited.
 * @param alias_name Group name that is alias.
 *
 * @return name of the main group that is being aliased.
 */
EAPI const char * edje_edit_group_aliased_get(Evas_Object *obj, const char *alias_name);

/**
 * @brief Adds new alias to the given group.
 *
 * @attention when aliasing a group, be sure that the given group_name is no an alias.
 *
 * @param obj Object being edited.
 * @param group_name Group name that is being aliased.
 * @param alias_name Group name that is alias.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_alias_add(Evas_Object *obj, const char *group_name, const char *alias_name);

//@}
/******************************************************************************/
/**************************   DATA API   **************************************/
/******************************************************************************/
/**
 * @name Data API
 * Functions to deal with data embedded in the edje (see @ref edcref).
 */ //@{

/**
 * @brief Retrieves a list with the item names inside the data block.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being a name entry in the global data block for the file.
 */
EAPI Eina_List * edje_edit_data_list_get(Evas_Object *obj);

/**
 * @brief Creates a new *global* data object in the given edje file.
 *
 * If another data entry with the same name exists, nothing is created and
 * EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param itemname Name for the new data entry.
 * @param value Value for the new data entry.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_data_add(Evas_Object *obj, const char *itemname, const char *value);

/**
 * @brief Deletes the given data object from edje.
 *
 * @param obj Object being edited.
 * @param itemname Data entry to remove from the global data block.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_data_del(Evas_Object *obj, const char *itemname);

/**
 * @brief Gets the data associated with the given itemname.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry to fetch the value for.
 *
 * @return Value of the given entry, or NULL if not found.
 */
EAPI const char * edje_edit_data_value_get(Evas_Object *obj, const char *itemname);

/**
 * @brief Sets the data associated with the given itemname.
 *
 * @param obj Object being edited.
 * @param itemname Name of data entry to change the value.
 * @param value New value for the entry.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_data_value_set(Evas_Object *obj, const char *itemname, const char *value);

/**
 * @brief Changes the name of the given data object.
 *
 * @param obj Object being edited.
 * @param itemname Data entry to rename.
 * @param newname New name for the data entry.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_data_name_set(Evas_Object *obj, const char *itemname, const char *newname);

/**
 * @brief Retrieves a list with the item names inside the data block at the group level.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being a name entry in the data block for the group.
 */
EAPI Eina_List * edje_edit_group_data_list_get(Evas_Object *obj);

/**
 * @brief Creates a new data object in the given edje file *belonging to the current group*.
 *
 * If another data entry with the same name exists,
 * nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param itemname Name for the new data entry.
 * @param value Value for the new data entry.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_data_add(Evas_Object *obj, const char *itemname, const char *value);

/**
 * @brief Deletes the given data object from the group.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry to remove.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_data_del(Evas_Object *obj, const char *itemname);

/**
 * @brief Gets the data associated with the given itemname.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry.
 *
 * @return Value of the data entry or NULL if not found.
 */
EAPI const char * edje_edit_group_data_value_get(Evas_Object *obj, const char *itemname);

/**
 * @brief Sets the data associated with the given itemname.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry to set the value.
 * @param value Value to set for the data entry.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_data_value_set(Evas_Object *obj, const char *itemname, const char *value);

/**
 * @brief Changes the name of the given data object.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry to rename.
 * @param newname New name for the data entry.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_data_name_set(Evas_Object *obj, const char *itemname, const char *newname);


//@}
/*****************************************************************************/
/***********************   SIZE CLASSES API   ********************************/
/*****************************************************************************/
/**
 * @name Size Classes API
 * Functions to deal with Size Classes (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the Size Classes in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being one size class.
 *         The return value should be freed with edje_edit_string_list_free().
 *
 * @see edje_edit_string_list_free()
 *
 * @since 1.18
 */
EAPI Eina_List *
edje_edit_size_classes_list_get(Evas_Object *obj);

/**
 * @brief Creates a new size class object in the given edje.
 *
 * If class is already exist then nothing is created and EINA_FALSE returned.
 *
 * @param obj Object being edited.
 * @param name Name for the new size class.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_size_class_add(Evas_Object *obj, const char *name);

/**
 * @brief Deletes size class object from edje.
 *
 * @param obj Object being edited.
 * @param name Size class to delete.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_size_class_del(Evas_Object *obj, const char *name);

/**
 * @brief Changes name of a size class.
 *
 * @param obj Object being edited.
 * @param name Size class to rename.
 * @param newname New name for the size class.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_size_class_name_set(Evas_Object *obj, const char *name, const char *newname);

/**
 * @brief Returns width min size of specified size class.
 *
 * @param obj Object being edited.
 * @param class_name Size class to fetch values.
 *
 * @return @c Evas_Coord.
 *
 * @since 1.18
 */
EAPI Evas_Coord
edje_edit_size_class_min_w_get(Evas_Object *obj, const char *class_name);

/**
 * @brief Sets width min size of specified size class.
 *
 * @param obj Object being edited.
 * @param class_name Size class to set values.
 * @param size Size which is greater or equal than zero (0).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_size_class_min_w_set(Evas_Object *obj, const char *class_name, Evas_Coord size);

/**
 * @brief Returns width max size of specified size class.
 *
 * @param obj Object being edited.
 * @param class_name Size class to fetch values.
 *
 * @return @c Evas_Coord.
 *
 * @since 1.18
 */
EAPI Evas_Coord
edje_edit_size_class_max_w_get(Evas_Object *obj, const char *class_name);

/**
 * @brief Sets width max size of specified size class.
 *
 * @param obj Object being edited.
 * @param class_name Size class to set values.
 * @param size Size which is greater or equal than zero (0).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_size_class_max_w_set(Evas_Object *obj, const char *class_name, Evas_Coord size);

/**
 * @brief Returns height min size of specified size class.
 *
 * @param obj Object being edited.
 * @param class_name Size class to fetch values.
 *
 * @return @c Evas_Coord.
 *
 * @since 1.18
 */
EAPI Evas_Coord
edje_edit_size_class_min_h_get(Evas_Object *obj, const char *class_name);

/**
 * @brief Sets height min size of specified size class.
 *
 * @param obj Object being edited.
 * @param class_name Size class to set values.
 * @param size Size which is greater or equal than zero (0).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_size_class_min_h_set(Evas_Object *obj, const char *class_name, Evas_Coord size);

/**
 * @brief Returns height max size of specified size class.
 *
 * @param obj Object being edited.
 * @param class_name Size class to fetch values.
 *
 * @return @c Evas_Coord (-1 is default value).
 *
 * @since 1.18
 */
EAPI Evas_Coord
edje_edit_size_class_max_h_get(Evas_Object *obj, const char *class_name);

/**
 * @brief Sets height max size of specified size class.
 *
 * @param obj Object being edited.
 * @param class_name Size class to set values.
 * @param size Size which is greater or equal minus one (-1, which is default value).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_size_class_max_h_set(Evas_Object *obj, const char *class_name, Evas_Coord size);

//@}
/*****************************************************************************/
/***********************   TEXT CLASSES API   ********************************/
/*****************************************************************************/
/**
 * @name Text Classes API
 * Functions to deal with Text Classes (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the Text Classes in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being one text class.
 *         The return value should be freed with edje_edit_string_list_free().
 *
 * @see edje_edit_string_list_free()
 *
 * @since 1.18
 */
EAPI Eina_List *
edje_edit_text_classes_list_get(Evas_Object *obj);

/**
 * @brief Creates a new text class object in the given edje.
 *
 * If class is already exist then nothing is created and EINA_FALSE returned.
 *
 * @param obj Object being edited.
 * @param name Name for the new text class.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_text_class_add(Evas_Object *obj, const char *name);

/**
 * @brief Deletes text class object from edje.
 *
 * @param obj Object being edited.
 * @param name Text class to delete.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_text_class_del(Evas_Object *obj, const char *name);

/**
 * @brief Changes name of a text class.
 *
 * @param obj Object being edited.
 * @param name Text class to rename.
 * @param newname New name for the text class.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_text_class_name_set(Evas_Object *obj, const char *name, const char *newname);

/**
 * @brief Returns font name of specified text class.
 *
 * @param obj Object being edited.
 * @param class_name Text class to fetch values.
 *
 * @return font name in case of success, NULL otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Stringshare *
edje_edit_text_class_font_get(Evas_Object *obj, const char *class_name);

/**
 * @brief Sets font for the given text class.
 *
 * NULL is possible value.
 *
 * @param obj Object being edited.
 * @param class_name Text class to set values.
 * @param font Name of font.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_text_class_font_set(Evas_Object *obj, const char *class_name, const char *font);

/**
 * @brief Returns font size of specified text class.
 *
 * @param obj Object being edited.
 * @param class_name Text class to fetch values.
 *
 * @return @c Evas_Font_Size.
 *
 * @since 1.18
 */
EAPI Evas_Font_Size
edje_edit_text_class_size_get(Evas_Object *obj, const char *class_name);

/**
 * @brief Sets font size for specified text class
 *
 * @param obj Object being edited.
 * @param class_name Text class to set values.
 * @param size Font size which is greater or equal than zero (0).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_text_class_size_set(Evas_Object *obj, const char *class_name, Evas_Font_Size size);


//@}
/******************************************************************************/
/***********************   COLOR CLASSES API   ********************************/
/******************************************************************************/
/**
 * @name Color Classes API
 * Functions to deal with Color Classes (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the Color Classes in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being one color class.
 */
EAPI Eina_List * edje_edit_color_classes_list_get(Evas_Object *obj);

/**
 * @brief Creates a new color class object in the given edje.
 *
 * If another class with the same name exists nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param name Name for the new color class.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_add(Evas_Object *obj, const char *name);

/**
 * @brief Deletes the given class object from edje.
 *
 * @param obj Object being edited.
 * @param name Color class to delete.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_del(Evas_Object *obj, const char *name);

/**
 * @brief Gets all the colors that compose the class.
 *
 * You can pass NULL to colors you are not intrested in.
 *
 * @param obj Object being edited.
 * @param class_name Color class to fetch values.
 * @param r Red component of main color.
 * @param g Green component of main color.
 * @param b Blue component of main color.
 * @param a Alpha component of main color.
 * @param r2 Red component of secondary color.
 * @param g2 Green component of secondary color.
 * @param b2 Blue component of secondary color.
 * @param a2 Alpha component of secondary color.
 * @param r3 Red component of tertiary color.
 * @param g3 Green component of tertiary color.
 * @param b3 Blue component of tertiary color.
 * @param a3 Alpha component of tertiary color.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_colors_get(Evas_Object *obj, const char *class_name, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/**
 * @brief Sets the colors for the given color class.
 *
 * If you set a color to -1 it will not be touched.
 *
 * @param obj Object being edited.
 * @param class_name Color class to fetch values.
 * @param r Red component of main color.
 * @param g Green component of main color.
 * @param b Blue component of main color.
 * @param a Alpha component of main color.
 * @param r2 Red component of secondary color.
 * @param g2 Green component of secondary color.
 * @param b2 Blue component of secondary color.
 * @param a2 Alpha component of secondary color.
 * @param r3 Red component of tertiary color.
 * @param g3 Green component of tertiary color.
 * @param b3 Blue component of tertiary color.
 * @param a3 Alpha component of tertiary color.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_colors_set(Evas_Object *obj, const char *class_name, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/**
 * @brief Changes the name of a color class.
 *
 * @param obj Object being edited.
 * @param name Color class to rename.
 * @param newname New name for the color class.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_name_set(Evas_Object *obj, const char *name, const char *newname);

/**
 * @brief Gets the description of a color class.
 *
 * @param obj Object being edited.
 * @param name Color class to get the description of.
 *
 * @return The description of the color class or @c NULL if not found
 * @since 1.14
 */
EAPI Eina_Stringshare *edje_edit_color_class_description_get(Evas_Object *obj, const char *class_name);

/**
 * @brief Changes the description of a color class.
 *
 * @param obj Object being edited.
 * @param name Color class to edit.
 * @param desc New description for the color class
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EAPI Eina_Bool edje_edit_color_class_description_set(Evas_Object *obj, const char *class_name, const char *desc);
//@}


/******************************************************************************/
/**************************   TEXT STYLES *************************************/
/******************************************************************************/
/**
 * @name Text styles API
 * Functions to deal with text styles (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the text styles in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being the name for a text style.
 */
EAPI Eina_List * edje_edit_styles_list_get(Evas_Object *obj);

/**
 * @brief Creates a new text style object in the given edje.
 *
 * If another style with the same name exists nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param style Name for the new style.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_add(Evas_Object *obj, const char *style);

/**
 * @brief Deletes the given text style and all the child tags.
 *
 * @param obj Object being edited.
 * @param style Style to delete.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_del(Evas_Object *obj, const char *style);

/**
 * @brief Gets the list of all the tags name in the given text style.
 *
 * @param obj Object being edited.
 * @param style Style to get the tags for.
 *
 * @return List of strings, each being one tag in the given style.
 */
EAPI Eina_List * edje_edit_style_tags_list_get(Evas_Object *obj, const char *style);

/**
 * @brief Gets the value of the given tag.
 *
 * @param obj Object being edited.
 * @param style Style containing the tag being.
 * @param tag Tag to get the value for.
 *
 * @return Value of the given tag.
 */
EAPI const char * edje_edit_style_tag_value_get(Evas_Object *obj, const char *style, const char *tag);

/**
 * @brief Sets the value of the given tag.
 *
 * @param obj Object being edited.
 * @param style Style containing the tag to change.
 * @param tag Name of the tag to set the value for.
 * @param new_value Value for the tag.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_tag_value_set(Evas_Object *obj, const char *style, const char *tag, const char *new_value);

/**
 * @brief Sets the name of the given tag.
 *
 * @param obj Object being edited.
 * @param style Style containing the tag to rename.
 * @param tag Tag to rename.
 * @param new_name New name for the tag.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_tag_name_set(Evas_Object *obj, const char *style, const char *tag, const char *new_name);

/**
 @ @brief Adds a new tag to the given text style.
 *
 * If another tag with the same name exists nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param style Style where to add the new tag.
 * @param tag_name Name for the new tag.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_tag_add(Evas_Object *obj, const char *style, const char *tag_name);

/**
 * @brief Deletes the given tag.
 *
 * @param obj Object being edited.
 * @param style Style from where to remove the tag.
 * @param tag Tag to delete.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_tag_del(Evas_Object *obj, const char *style, const char *tag);


//@}
/******************************************************************************/
/************************   EXTERNALS API   ***********************************/
/******************************************************************************/
/**
 * @name Externals API
 * Functions to deal with list of external modules (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the externals requested in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being an entry in the block of automatically loaded external modules.
 */
EAPI Eina_List * edje_edit_externals_list_get(Evas_Object *obj);

/**
 * @brief Adds an external module to be requested on edje load.
 *
 * @param obj Object being edited.
 * @param external Name of the external module to add to the list of autoload.
 *
 * @return @c EINA_TRUE on success (or it was already there), @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_external_add(Evas_Object *obj, const char *external);

/**
 * @brief Deletes the given external from the list.
 *
 * @param obj Object being edited.
 * @param external Name of the external module to remove from the autoload list.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_external_del(Evas_Object *obj, const char *external);


//@}
/******************************************************************************/
/**************************   PARTS API   *************************************/
/******************************************************************************/
/**
 * @name Parts API
 * Functions to deal with part objects (see @ref edcref).
 */ //@{

/**
 * @brief Gets the select mode for a textblock part.

 * @param obj Object being edited.
 * @param part Name of the part.
 *
 * @return One of possible enum Edje_Edit_Select_Mode.
 * @since 1.11
 */
EAPI Edje_Edit_Select_Mode edje_edit_part_select_mode_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the select mode for a textblock part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 * @param mode One of possible enum Edje_Edit_Select_Mode:
 * EDJE_EDIT_SELECT_MODE_DEFAULT, EDJE_EDIT_SELECT_MODE_EXPLICIT.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_select_mode_set(Evas_Object *obj, const char *part, Edje_Edit_Select_Mode mode);

/**
 * @brief Gets the edit mode for a textblock part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 *
 * @return One of possible enum Edje_Entry_Mode.
 * @since 1.11
 */
EAPI Edje_Edit_Entry_Mode edje_edit_part_entry_mode_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the edit mode for a textblock part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 * @param mode One of possible enum Edje_Entry_Mode:
 * EDJE_EDIT_ENTRY_MODE_NONE, EDJE_EDIT_ENTRY_MODE_PLAIN, EDJE_EDIT_ENTRY_MODE_EDITABLE, EDJE_EDIT_ENTRY_MODE_PASSWORD.

 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_entry_mode_set(Evas_Object *obj, const char *part, Edje_Edit_Entry_Mode mode);

/**
 * @brief Gets the list of all the parts in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being the name for a part in the open group.
 *         The return value should be freed with edje_edit_string_list_free().
 *
 * @see edje_edit_string_list_free()
 */
EAPI Eina_List * edje_edit_parts_list_get(Evas_Object *obj);

/**
 * @brief Creates a new part in the given edje.
 *
 * If another part with the same name just exists nothing is created and EINA_FALSE is returned.
 * Note that this function also create a default description for the part.
 *
 * @param obj Object being edited.
 * @param name Name for the new part.
 * @param type Type of the new part. See @ref edcref for more info on this.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_add(Evas_Object *obj, const char *name, Edje_Part_Type type);

/**
 * @brief Creates a new part of type EXTERNAL in the given edje.
 *
 * If another part with the same name just exists nothing is created and EINA_FALSE is returned.
 * Note that this function also create a default description for the part.
 *
 * @param obj Object being edited.
 * @param name Name for the new part.
 * @param source The registered external type to use for this part.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_external_add(Evas_Object *obj, const char *name, const char *source);

/**
 * @brief Deletes the given part from the edje.
 *
 * All the reference to this part will be zeroed.
 *
 * @param obj Object being edited.
 * @param part Name of part to delete.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_del(Evas_Object *obj, const char *part);

/**
 * @brief Copies the given part in edje.
 *
 * If another part with the same name just exists nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 * @param new_copy Name of the new copied part.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_copy(Evas_Object *obj, const char *part, const char *new_copy);

/**
 * @brief Checks if a part with the given name exist in the edje object.
 *
 * @param obj Object being edited.
 * @param part Name of part to check for its existence.
 *
 * @return @c EINA_TRUE if the part exists, @c EINA_FALSE if not.
 */
EAPI Eina_Bool edje_edit_part_exist(Evas_Object *obj, const char *part);

/**
 * @brief Gets the name of part stacked above the one passed.
 *
 * @param obj Object being edited.
 * @param part Name of part of which to check the one above.
 *
 * @return Name of the part above. NULL if an error occurred or if @p part is
 * the topmost part in the group.
 */
EAPI const char * edje_edit_part_above_get(Evas_Object *obj, const char *part);

/**
 * @brief Gets the name of part stacked below the one passed.
 *
 * @param obj Object being edited.
 * @param part Name of part of which to check the one below.
 *
 * @return Name of the part below. NULL if an error occurred or if @p part is
 * the bottommost part in the group.
 */
EAPI const char * edje_edit_part_below_get(Evas_Object *obj, const char *part);

/**
 * @brief Moves the given part below the previous one.
 *
 * @param obj Object being edited.
 * @param part Name of part to move one step below.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_restack_below(Evas_Object *obj, const char *part);

/**
 * @brief Moves the given part below the part named below.
 *
 * @param obj Object being edited.
 * @param part Name of part which will be moved.
 * @param below Name of part for which will be moved 'part'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_restack_part_below(Evas_Object *obj, const char* part, const char *below);

/**
 * @brief Moves the given part above the next one.
 *
 * @param obj Object being edited.
 * @param part Name of part to move one step above.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_restack_above(Evas_Object *obj, const char *part);

/**
 * @brief Moves the given part above the part named above.
 *
 * @param obj Object being edited.
 * @param part Name of part which will be moved.
 * @param above Name of part for which will be moved 'part'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_restack_part_above(Evas_Object *obj, const char* part, const char *above);

/**
 * @brief Sets a new name for part.
 *
 * Note that the relative getter function don't exist as it don't make sense ;)
 *
 * @param obj Object being edited.
 * @param part Name of part to rename.
 * @param new_name New name for the given part.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_name_set(Evas_Object *obj, const char *part, const char *new_name);

/**
 * @brief Gets api's name of a part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 *
 * @return Name of the API if successful, NULL otherwise.
 */
EAPI const char * edje_edit_part_api_name_get(Evas_Object *obj, const char *part);

/**
 * @brief Gets api's description of a part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 *
 * @return Description of the api if successful, NULL otherwise.
 */
EAPI const char * edje_edit_part_api_description_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets api's name of a part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 * @param name New name for the api property.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_api_name_set(Evas_Object *obj, const char *part, const char *name);

/**
 * @brief Sets api's description of a part.
 *
 * @param obj Object being edited.
 * @param part Name of part.
 * @param description New description for the api property.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_api_description_set(Evas_Object *obj, const char *part, const char *description);

/**
 * @brief Gets the type of a part.
 *
 * @param obj Object being edited.
 * @param part Name of part to get the type of.
 *
 * @return Type of the part. See @ref edcref for details.
 */
EAPI Edje_Part_Type edje_edit_part_type_get(Evas_Object *obj, const char *part);

/**
 * @brief Gets the clip_to part.
 *
 * @param obj Object being edited.
 * @param part Name of the part whose clipper to get.
 *
 * @return Name of the part @p part is clipped to. NULL is returned on errors and if the part don't have a clip.
 */
EAPI const char * edje_edit_part_clip_to_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets a part to clip part to.
 *
 * @param obj Object being edited.
 * @param part Part to set the clipper to.
 * @param clip_to Part to use as clipper, if NULL then the clipping value will be cancelled (unset clipping).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_clip_to_set(Evas_Object *obj, const char *part, const char *clip_to);

/**
 * @brief Gets the source of part.
 *
 * The meaning of this parameter varies depending on the type of the part.
 * For GROUP parts, it's the name of another group in the Edje file which will
 * be autoloaded and swallowed on this part.
 * For TEXTBLOCK parts, it's the name of a group to be used for selection
 * display under the text.
 * For EXTERNAL parts, it's the name of the registered external widget to load
 * and swallow on this part.
 *
 * @param obj Object being edited.
 * @param part Part to get the source from.
 *
 * @return Content of the source parameter or NULL if nothing set or an error occurred.
 */
EAPI const char * edje_edit_part_source_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the source of part.
 *
 * If setting source of the part will lead to recursive reference
 * (when A source to B, and B is going to be source to A because of this function),
 * then it will return EINA_FALSE.
 *
 * @param obj Object being edited.
 * @param part Part to set the source of.
 * @param source Value for the source parameter.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see edje_edit_part_source_get()
 *
 * @note You can't change the source for EXTERNAL parts, it's akin to changing
 * the type of the part.
 *
 * NOTE: This is not applied now. You must reload the edje to see the change.
 */
EAPI Eina_Bool edje_edit_part_source_set(Evas_Object *obj, const char *part, const char *source);

/**
 * @brief Gets the source2 of part.
 *
 * Only available to TEXTBLOCK parts. It is used for the group to be loaded and
 * used for selection display OVER the selected text. source is used for under
 * of the selected text, if source is specified.
 *
 * @param obj Object being edited.
 * @param part Part to get the source from.
 *
 * @return Content of the source2 parameter or NULL if nothing set or an error occurred.
 * @since 1.11
 */
EAPI const char * edje_edit_part_source2_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the source2 of part.
 *
 * @param obj Object being edited.
 * @param part Part to set the source of.
 * @param source Value for the source parameter.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see edje_edit_part_source2_get()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_source2_set(Evas_Object *obj, const char *part, const char *source);

/**
 * @brief Gets the source3 of part.
 *
 * Only available to TEXTBLOCK parts. It is used for the group to be loaded and
 * used for cursor display UNDER the cursor position. source4 is used for over
 * the cursor text, if source4 is specified.
 *
 * @param obj Object being edited.
 * @param part Part to get the source from.
 *
 * @return Content of the source3 parameter or NULL if nothing set or an error occurred.
 * @since 1.11
 */
EAPI const char * edje_edit_part_source3_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the source3 of part.
 *
 * @param obj Object being edited.
 * @param part Part to set the source of.
 * @param source Value for the source parameter.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see edje_edit_part_source3_get()
 * @since 1.11
 *
 * NOTE: This is not applied now. You must reload the edje to see the change.
 */
EAPI Eina_Bool edje_edit_part_source3_set(Evas_Object *obj, const char *part, const char *source);

/**
 * @brief Gets the source4 of part.
 *
 * Only available to TEXTBLOCK parts. It is used for the group to be loaded and
 * used for cursor display OVER the cursor position. source3 is used for under
 * the cursor text, if source4 is specified.
 *
 * @param obj Object being edited.
 * @param part Part to get the source from.
 *
 * @return Content of the source4 parameter or NULL if nothing set or an error occurred.
 * @since 1.11
 */
EAPI const char * edje_edit_part_source4_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the source4 of part.
 *
 * @param obj Object being edited.
 * @param part Part to set the source of.
 * @param source Value for the source parameter.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see edje_edit_part_source4_get()
 * @since 1.11
 *
 * NOTE: This is not applied now. You must reload the edje to see the change.
 */
EAPI Eina_Bool edje_edit_part_source4_set(Evas_Object *obj, const char *part, const char *source);

/**
 * @brief Gets the source5 of part.
 *
 * Only available to TEXTBLOCK parts. It is used for the group to be loaded and
 * used for anchors display UNDER the anchor position. source6 is used for over
 * the anchors text, if source6 is specified.
 *
 * @param obj Object being edited.
 * @param part Part to get the source from.
 *
 * @return Content of the source5 parameter or NULL if nothing set or an error occurred.
 * @since 1.11
 */
EAPI const char * edje_edit_part_source5_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the source5 of part.
 *
 * @param obj Object being edited.
 * @param part Part to set the source of.
 * @param source Value for the source parameter.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see edje_edit_part_source5_get()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_source5_set(Evas_Object *obj, const char *part, const char *source);

/**
 * @brief Gets the source6 of part.
 *
 * Only available to TEXTBLOCK parts. It is used for the group to be loaded and
 * used for anchor display OVER the anchor position. source5 is used for under
 * the anchor text, if source6 is specified.
 *
 * @param obj Object being edited.
 * @param part Part to get the source from.
 *
 * @return Content of the source6 parameter or NULL if nothing set or an error occurred.
 * @since 1.11
 */
EAPI const char * edje_edit_part_source6_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the source6 of part.
 *
 * @param obj Object being edited.
 * @param part Part to set the source of.
 * @param source Value for the source parameter.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see edje_edit_part_source6_get()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_source6_set(Evas_Object *obj, const char *part, const char *source);

/**
 * @brief Gets the effect for a given part.
 *
 * Gets the effect used for parts of type TEXT. See @ref edcref for more details.
 *
 * @param obj Object being edited.
 * @param part Part to get the effect of.
 *
 * @return The effect set for the part.
 */
EAPI Edje_Text_Effect edje_edit_part_effect_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the effect for a given part.
 * Effects and shadow directions can be combined.
 *
 * For effect and shadow direction list please look at Edje Part Text ref page.
 *
 * @param obj Object being edited.
 * @param part Part to set the effect to. Only makes sense on type TEXT.
 * @param effect Effect to set for the part.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @see Edje_Part_Text
 */
EAPI Eina_Bool edje_edit_part_effect_set(Evas_Object *obj, const char *part, Edje_Text_Effect effect);

/**
 * @brief Gets the current selected state in part.
 *
 * @param obj Object being edited.
 * @param part Part to get the selected state of.
 * @param value Pointer to a double where the value of the state will be stored.
 *
 * @return The name of the currently selected state for the part.
 */
EAPI const char * edje_edit_part_selected_state_get(Evas_Object *obj, const char *part, double *value);

/**
 * @brief Sets the current state in part.
 *
 * @param obj Object being edited.
 * @param part Part to set the state of.
 * @param state Name of the state to set.
 * @param value Value of the state.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_selected_state_set(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets mouse_events for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if the mouse events is accepted.
 *
 * @return @c EINA_TRUE if part will accept mouse events, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_mouse_events_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets mouse_events for part.
 *
 * @param obj Object being edited.
 * @param part The part to set if the mouse events is accepted.
 * @param mouse_events @c EINA_TRUE if part will accept mouse events, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_mouse_events_set(Evas_Object *obj, const char *part, Eina_Bool mouse_events);

/**
 * @brief Gets anti-aliasing for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if the anti-aliasing is accepted.
 *
 * @return @c EINA_TRUE if part will draw anti-aliased, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_anti_alias_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets anti-aliasing for part.
 *
 * @param obj Object being edited.
 * @param part The part to set if the anti-aliasing is accepted.
 * @param mouse_events @c EINA_TRUE if part should to draw anti-aliased, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_anti_alias_set(Evas_Object *obj, const char *part, Eina_Bool anti_alias);


/**
 * @brief Gets repeat_events for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if it will pass all events to the other parts.
 *
 * @return @c EINA_TRUE if the events received will propagate to other parts, @c EINA_FALSE otherwise
 */
EAPI Eina_Bool edje_edit_part_repeat_events_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets repeat_events for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if will repeat all the received mouse events to other parts.
 * @param repeat_events @c EINA_TRUE if the events received will propagate to other parts, @c EINA_FALSE otherwise
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_repeat_events_set(Evas_Object *obj, const char *part, Eina_Bool repeat_events);

/**
 * @brief Gets use_alternate_font_metrics variable for part.
 *
 * @param obj Object being edited.
 * @param part Part to get use_alternate_font_metrics for text or textblock part is enabled.
 *
 * @return @c EINA_TRUE if use_alternate_font_metrics, @c EINA_FALSE otherwise
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_use_alternate_font_metrics_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets use_alternate_font_metrics variable for part.
 *
 * @param obj Object being edited.
 * @param part Part to set use_alternate_font_metrics for text or textblock part is enabled.
 * @param use EINA_TRUE if use_alternate_font_metrics, @c EINA_FALSE otherwise
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_use_alternate_font_metrics_set(Evas_Object *obj, const char *part, Eina_Bool use);

/**
 * @brief Gets multiline for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if editing multiple lines for text or textblock part is enabled.
 *
 * @return @c EINA_TRUE if multiple lines for editing is enabled, @c EINA_FALSE otherwise
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_multiline_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets multiline for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if editing multiple lines for text or textblock part is enabled.
 * @param multiline @c EINA_TRUE if multiple lines for editing is enabled, @c EINA_FALSE otherwise
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_multiline_set(Evas_Object *obj, const char *part, Eina_Bool multiline);

/**
 * @brief Gets precise_is_inside for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if it will enable point collision detection for the part.
 *
 * @return @c EINA_TRUE if point collision detection for the part is enabled, @c EINA_FALSE otherwise
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_precise_is_inside_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets precise_is_inside for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if it will enable point collision detection for the part.
 * @param precise_is_inside EINA_TRUE if point collision detection for the part is enabled, @c EINA_FALSE otherwise
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_precise_is_inside_set(Evas_Object *obj, const char *part, Eina_Bool precise_is_inside);

/**
 * @brief Gets accessibility for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if it uses accessibility feature.
 *
 * @return @c EINA_TRUE if part uses accessibility feature, @c EINA_FALSE otherwise
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_access_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets accessibility for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if it uses accessibility feature.
 * @param access EINA_TRUE if part uses accessibility feature, @c EINA_FALSE otherwise
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_access_set(Evas_Object *obj, const char *part, Eina_Bool access);

/**
 * @brief Gets ignore_flags for part.
 *
 * @param obj Object being edited.
 * @param part Part to get which event_flags are being ignored.
 *
 * @return The Event flags set to the part.
 */
EAPI Evas_Event_Flags edje_edit_part_ignore_flags_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets ignore_flags for part.
 *
 * @param obj Object being edited.
 * @param part Part to set which event flags will be ignored.
 * @param ignore_flags The Event flags to be ignored by the part.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_ignore_flags_set(Evas_Object *obj, const char *part, Evas_Event_Flags ignore_flags);

/**
 * @brief Gets mask_flags for part.
 *
 * @param obj Object being edited.
 * @param part Part to get which event_flags are being masked.
 *
 * @return The Event flags set to the part.
 */
EAPI Evas_Event_Flags edje_edit_part_mask_flags_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets mask_flags for part.
 *
 * @param obj Object being edited.
 * @param part Part to set which event flags will be masked.
 * @param mask_flags The Event flags to be masked by the part.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_mask_flags_set(Evas_Object *obj, const char *part, Evas_Event_Flags mask_flags);

/**
 * @brief Gets pointer_mode of a part.
 *
 * @param obj Object being edited.
 * @param part Part name to get it's pointer_mode.
 *
 * @return Ponter Mode of the part.
 * @since 1.11
 */
EAPI Evas_Object_Pointer_Mode edje_edit_part_pointer_mode_get(Evas_Object *obj, const char *part);

/**
 * @brief Gets pointer_mode of a part.
 *
 * Note that Ponter Mode can be:
 * - EVAS_OBJECT_POINTER_MODE_AUTOGRAB - default, X11-like
 * - EVAS_OBJECT_POINTER_MODE_NOGRAB - pointer always bound to the object right below it
 * - EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN - useful on object with "repeat events" enabled, @since 1.2
 *
 * @param obj Object being edited.
 * @param part Part name to get it's pointer_mode.
 * @param pointer_mode Pointer Mode.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_pointer_mode_set(Evas_Object *obj, const char *part, Evas_Object_Pointer_Mode pointer_mode);

/**
 * @brief Gets cursor_mode of a part.
 *
 * Note that Cursor Mode can be:
 * 0 - UNDER cursor mode means the cursor will draw below the character pointed
 *     at. That's the default.
 * 1 - BEFORE cursor mode means the cursor is drawn as a vertical line before
 *     the current character, just like many other GUI toolkits handle it.
 *
 * @param obj Object being edited.
 * @param part Part name to get it's cursor_mode.
 *
 * @return Ponter Mode of the part.
 * @since 1.11
 */
EAPI unsigned char edje_edit_part_cursor_mode_get(Evas_Object *obj, const char *part);

/**
 * @brief Gets pointer_mode of a part.
 *
 * Note that Cursor Mode can be:
 * 0 - UNDER cursor mode means the cursor will draw below the character pointed
 *     at. That's the default.
 * 1 - BEFORE cursor mode means the cursor is drawn as a vertical line before
 *     the current character, just like many other GUI toolkits handle it.
 *
 * @param obj Object being edited.
 * @param part Part name to get it's pointer_mode.
 * @param cursor_mode Pointer Mode.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_cursor_mode_set(Evas_Object *obj, const char *part, unsigned char cursor_mode);

/**
 * @brief Sets scale property for the part.
 *
 * This property tells Edje that the given part should be scaled by the
 * Edje scale factor.
 *
 * @param obj Object being edited.
 * @param part Part to set scale for.
 * @param scale Scale value to set.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_scale_set(Evas_Object *obj, const char *part, Eina_Bool scale);

/**
 * @brief Gets scale for the part.
 *
 * @param obj Object being edited.
 * @param part Part to get the scale value of.
 *
 * @return Whether scale is on (EINA_TRUE) or not.
 */
EAPI Eina_Bool edje_edit_part_scale_get(Evas_Object *obj, const char *part);

/**
 * @brief Gets horizontal dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if can be dragged horizontally.
 *
 * @return @c 1 (or -1) if the part can be dragged horizontally, @c 0 otherwise.
 */
EAPI int edje_edit_part_drag_x_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets horizontal dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if should be dragged horizontally.
 * @param drag @c 1 (or -1) if the part should be dragged horizontally, @c 0 otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_x_set(Evas_Object *obj, const char *part, int drag);

/**
 * @brief Gets vertical dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if can be dragged vertically.
 *
 * @return @c 1 (or - 1) if the part can be dragged vertically, @c 0 otherwise.
 */
EAPI int edje_edit_part_drag_y_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets vertical dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if should be dragged vertically.
 * @param drag @c 1 (or -1) of the part should be dragged vertically, @c 0 otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_y_set(Evas_Object *obj, const char *part, int drag);

/**
 * @brief Gets horizontal dragable step for part.
 *
 * @param obj Object being edited.
 * @param part Part to get the drag horizontal step value.
 *
 * @return The step value.
 */
EAPI int edje_edit_part_drag_step_x_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets horizontal dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to set the drag horizontal step value.
 * @param step The step the will be dragged.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_step_x_set(Evas_Object *obj, const char *part, int step);

/**
 * @brief Gets vertical dragable step for part.
 *
 * @param obj Object being edited.
 * @param part Part to get the drag vertical step value.
 *
 * @return The step value.
 */
EAPI int edje_edit_part_drag_step_y_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets vertical dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to set the drag vertical step value.
 * @param step The step the will be dragged.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_step_y_set(Evas_Object *obj, const char *part, int step);

/**
 * @brief Gets horizontal dragable count for part.
 *
 * @param obj Object being edited.
 * @param part Part to get the drag horizontal count value.
 *
 * @return Horizontal dragable count value
 */
EAPI int edje_edit_part_drag_count_x_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets horizontal dragable count for part.
 *
 * @param obj Object being edited.
 * @param part Part to set the drag horizontal count value.
 * @param count The count value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_count_x_set(Evas_Object *obj, const char *part, int count);

/**
 * @brief Gets vertical dragable count for part.
 *
 * @param obj Object being edited.
 * @param part Part to get the drag vertical count value.
 *
 * @return Vertical dragable count value
 */
EAPI int edje_edit_part_drag_count_y_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets vertical dragable count for part.
 *
 * @param obj Object being edited.
 * @param part Part to set the drag vertical count value.
 * @param count The count value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_count_y_set(Evas_Object *obj, const char *part, int count);

/**
 * @brief Gets the name of the part that is used as 'confine' for the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to get the name that is used as 'confine' for the given draggies.
 *
 * @return The name of the confine part, or NULL (if unset).
 */
EAPI const char * edje_edit_part_drag_confine_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the name of the part that is used as 'confine' for the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to set the name that is used as 'confine' for the given draggies.
 * @param confine The name of the confine part, or NULL to unset confine.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_confine_set(Evas_Object *obj, const char *part, const char *confine);

/**
 * @brief Gets the name of the part that is used as the receiver of the drag event.
 *
 * @param obj Object being edited.
 * @param part Part to get the name that is used as the receiver of the drag event.
 *
 * @return The name of the part that will receive events, or NULL (if unset).
 */
EAPI const char * edje_edit_part_drag_event_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the name of the part that will receive events from the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to set the name that will receive events from the given draggies.
 * @param event The name of the part that will receive events, or NULL to unset.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_event_set(Evas_Object *obj, const char *part, const char *event);

/**
 * @brief Gets the name of the part that is used as 'threshold' for the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to get the name that is used as 'threshold' for the given draggies.
 *
 * @return The name of the threshold part, or NULL (if unset).
 */
EAPI const char * edje_edit_part_drag_threshold_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets the name of the part that is used as 'threshold' for the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to set the name that is used as 'threshold' for the given draggies.
 * @param threshold The name of the threshold part, or NULL to unset confine.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_threshold_set(Evas_Object *obj, const char *part, const char *threshold);

//@}
/******************************************************************************/
/*******************************      BOX API      ****************************/
/******************************************************************************/
/**
 * @name Box Description API
 * Functions to deal with box (see @ref edcref).
 */ //@{

/**
 * @brief Gets primary layout of the box.
 *
 * @note The returned string should be freed with @c eina_stringshare_del().
 * @param obj Object being edited.
 * @param part Part that have BOX type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * @return Primary layout of a BOX part in given state.
 * @since 1.14
 */
EAPI Eina_Stringshare * edje_edit_state_box_layout_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets primary layout of the box.
 *
 * When trying to set primary layout to NULL, function will use
 * alternative layout instead.
 *
 * @param obj Object being edited.
 * @param part Part that have BOX type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param layout New primary layout to set name.
 *
 * Possible layouts:
 *     @li horizontal (default)
 *     @li vertical
 *     @li horizontal_homogeneous
 *     @li vertical_homogeneous
 *     @li horizontal_max (homogeneous to the max sized child)
 *     @li vertical_max
 *     @li horizontal_flow
 *     @li vertical_flow
 *     @li stack
 *     @li some_other_custom_layout_set_by_the_application
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EAPI Eina_Bool edje_edit_state_box_layout_set(Evas_Object *obj, const char *part, const char *state, double value, char *layout);

/**
 * @brief Gets fallback layout of the box.
 *
 * @note The returned string should be freed with @c eina_stringshare_del().
 * @param obj Object being edited.
 * @param part Part that have BOX type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * @return Fallback layout of a BOX part in given state.
 * @since 1.14
 */
EAPI Eina_Stringshare * edje_edit_state_box_alt_layout_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets fallback layout of the box.
 *
 * When trying to set fallback layout to NULL, function will use
 * default layout ("horizontal") instead.
 *
 * @param obj Object being edited.
 * @param part Part that have BOX type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param layout New fallback layout to set name.
 *
 * Possible layouts:
 *     @li horizontal (default)
 *     @li vertical
 *     @li horizontal_homogeneous
 *     @li vertical_homogeneous
 *     @li horizontal_max (homogeneous to the max sized child)
 *     @li vertical_max
 *     @li horizontal_flow
 *     @li vertical_flow
 *     @li stack
 *     @li some_other_custom_layout_set_by_the_application
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EAPI Eina_Bool edje_edit_state_box_alt_layout_set(Evas_Object *obj, const char *part, const char *state, double value, char *layout);

//@}
/******************************************************************************/
/**************************   TABLE API   *************************************/
/******************************************************************************/
/**
 * @name Table Description API
 * Functions to deal with table (see @ref edcref).
 */ //@{

/**
 * @brief Sets homogeneous mode for TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that have TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param homogeneous Homogeneous mode for table.
 *
 * Possible modes:
 *     @li EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE,
 *     @li EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE,
 *     @li EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EAPI Eina_Bool
edje_edit_state_table_homogeneous_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char homogeneous);
/**
 * @brief Gets homogeneous mode for TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that have TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * Possible modes:
 *     @li EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE,
 *     @li EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE,
 *     @li EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM
 *
 * @return Table homogeneous mode on success, or @c 0 on any error.
 * @since 1.14
 */
EAPI unsigned char edje_edit_state_table_homogeneous_get(Evas_Object *obj, const char *part, const char *state, double value);

//@}
/******************************************************************************/
/***********************   TABLE & BOX API   **********************************/
/******************************************************************************/
/**
 * @name "Container" Description API
 * Functions to deal both with box and table (see @ref edcref).
 * Those functions called edje_edit_state_container_<something> because they
 * are working both for TABLE and BOX at same time.
 */ //@{

/**
 * @deprecated Use edje_edit_state_container_min_h_get() and
 * edje_edit_state_container_min_v_get() instead.
 *
 * @brief Gets whether vertical or horizontal minimum size's of the box are equal
 * to the minimum vertical or horizontal size of items
 * (function for BOX or TABLE part.
 * If EINA_TRUE - is equal, if EINA_FALSE - is not)
 *
 * @param obj Object being edited.
 * @param part Part that have BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param h Variable to store horizontal min value.
 * @param v Variable to store vertical min value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_state_container_min_get(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool *h, Eina_Bool *v);

/**
 * @brief Gets whether horizontal min size of the container is equal to the min
 * horizontal size of items (BOX and TABLE part functions).
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * @return @c EINA_TRUE If the part forces container's minimal horizontal size,
 *         @c EINA_FALSE otherwise.
 * @since 1.16
 */
EAPI Eina_Bool
edje_edit_state_container_min_h_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets whether vertical min size of the container is equal to the min vertical
 * size of items (BOX and TABLE part functions).
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * @return @c EINA_TRUE If the part forces container's minimal horizontal size,
 *         @c EINA_FALSE otherwise.
 * @since 1.16
 */
EAPI Eina_Bool
edje_edit_state_container_min_v_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @deprecated Use edje_edit_state_container_min_h_set() and
 * edje_edit_state_container_min_v_set() instead.
 *
 * @brief Sets whether vertical or horizontal minimum size's of the box are equal
 * to the minimum vertical or horizontal size of items
 * (function for BOX or TABLE part.
 * If EINA_TRUE - is equal, if EINA_FALSE - is not)
 *
 * @param obj Object being edited.
 * @param part Part that have BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param h horizontal min value.
 * @param v vertical min value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_state_container_min_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool h, Eina_Bool v);

/**
 * @brief Sets whether horizontal min size of the container should be equal to the min
 * horizontal size of items (BOX and TABLE part functions).
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param h New horizontal min value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.16
 */
EAPI Eina_Bool
edje_edit_state_container_min_h_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool h);

/**
 * @brief Sets whether vertical min size of the container should be equal to the min
 * vertical size of items (BOX and TABLE part functions).
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param v New vertical min value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EAPI Eina_Bool
edje_edit_state_container_min_v_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/**
 * @deprecated Use edje_edit_state_container_padding_x_get() and
 * edje_edit_state_container_padding_y_get() instead.
 *
 * @brief Gets x and y paddings for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that have BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param x Variable to store x padding.
 * @param y Variable to store y padding.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EINA_DEPRECATED EAPI Eina_Bool
edje_edit_state_container_padding_get(Evas_Object *obj, const char *part, const char *state, double value, int *x, int *y);

/**
 * @brief Gets x padding for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * @return The padding by x axis.
 * @since 1.16
 */
EAPI int
edje_edit_state_container_padding_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets y padding for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * @return The padding by y axis.
 * @since 1.16
 */
EAPI int
edje_edit_state_container_padding_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @deprecated Use edje_edit_state_container_padding_x_set() and
 * edje_edit_state_container_padding_y_set() instead.
 *
 * @brief Sets x and y paddings for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that have BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param x Value for setting x padding.
 * @param y Value for setting y padding.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EINA_DEPRECATED EAPI Eina_Bool
edje_edit_state_container_padding_set(Evas_Object *obj, const char *part, const char *state, double value, int x, int y);

/**
 * @brief Sets x padding for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param x New x padding value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EAPI Eina_Bool
edje_edit_state_container_padding_x_set(Evas_Object *obj, const char *part, const char *state, double value, int x);

/**
 * @brief Sets y padding for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param y New y padding value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EAPI Eina_Bool
edje_edit_state_container_padding_y_set(Evas_Object *obj, const char *part, const char *state, double value, int y);

/**
 * @deprecated Use edje_edit_state_container_align_x_set() and
 * edje_edit_state_container_align_y_set() instead.
 *
 * @brief Sets x and y align for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that have BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param x Variable to store x value.
 * @param y Variable to store y value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EINA_DEPRECATED EAPI Eina_Bool
edje_edit_state_container_align_set(Evas_Object *obj, const char *part, const char *state, double value, double x, double y);

/**
 * @brief Sets x align for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param x New x align value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EAPI Eina_Bool
edje_edit_state_container_align_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/**
 * @brief Sets y align for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param y New y align value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EAPI Eina_Bool
edje_edit_state_container_align_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/**
 * @deprecated Use edje_edit_state_container_align_x_set() and
 * edje_edit_state_container_align_y_set() instead.
 *
 * @brief Gets x and y align for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that have BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 * @param x Value for setting x align.
 * @param y Value for setting y align.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.14
 */
EINA_DEPRECATED EAPI Eina_Bool
edje_edit_state_container_align_get(Evas_Object *obj, const char *part, const char *state, double value, double *x, double *y);

/**
 * @brief Gets x align for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * @return The align by x axis.
 * @since 1.16
 */
EAPI double
edje_edit_state_container_align_x_get(Evas_Object *obj, const char *part, const char *state, double value);
/**
 * @brief Gets y align for BOX or TABLE part.
 *
 * @param obj Object being edited.
 * @param part Part that has BOX/TABLE type.
 * @param state Name of the state.
 * @param value Value of the state.
 *
 * @return The align by y axis.
 * @since 1.16
 */
EAPI double
edje_edit_state_container_align_y_get(Evas_Object *obj, const char *part, const char *state, double value);

//@}
/******************************************************************************/
/**************************   BOX & TABLE ITEMS API   *************************/
/******************************************************************************/
/**
 * @name Items API
 * Functions to deal with table and box part's items (see @ref edcref).
 */ //@{

/**
 * @brief Appends new item to box or table part.
 *
 * @param obj Object being edited.
 * @param part Part to add a new item. This part should have BOX or TABLE type.
 * @param item_name Name of new item that is not exist in BOX or TABLE yet.
 * @param source_group Source (means group name) of the new item
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_part_item_append(Evas_Object *obj, const char *part, const char *item_name, const char *source_group);

/**
 * @brief Inserts new item to box or table part before specified existing item.
 *
 * @param obj Object being edited.
 * @param part Part to add a new item. This part should have BOX or TABLE type.
 * @param item_name Name of new item that is not exist in BOX or TABLE yet.
 * @param item_before Name of repated item that is exist in BOX or TABLE.
 * @param source_group Source (means group name) of the new item.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EINA_DEPRECATED
EAPI Eina_Bool
edje_edit_part_item_insert_before(Evas_Object *obj, const char *part, const char *item_name, const char *item_before, const char *source_group);

/**
 * @brief Inserts new item to box or table part before specified existing item.
 *
 * @param obj Object being edited.
 * @param part Part to add a new item. This part should have BOX or TABLE type.
 * @param item_name Name of new item that is not exist in BOX or TABLE yet.
 * @param index Index of repated item that is exist in BOX or TABLE.
 * @param source_group Source (means group name) of the new item.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_insert_before_index(Evas_Object *obj, const char *part, const char *item_name, unsigned int index, const char *source_group);

/**
 * @brief Inserts new item to box or table part after specified existing item.
 *
 * @param obj Object being edited.
 * @param part Part to add a new item. This part should have BOX or TABLE type.
 * @param item_name Name of new item that is not exist in BOX or TABLE yet.
 * @param item_after Name of repated item that is exist in BOX or TABLE.
 * @param source_group Source (means group name) of the new item.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EINA_DEPRECATED
EAPI Eina_Bool
edje_edit_part_item_insert_after(Evas_Object *obj, const char *part, const char *item_name, const char *item_after, const char *source_group);

/**
 * @brief Inserts new item to box or table part after specified existing item.
 *
 * @param obj Object being edited.
 * @param part Part to add a new item. This part should have BOX or TABLE type.
 * @param item_name Name of new item that is not exist in BOX or TABLE yet.
 * @param index Index of repated item that is exist in BOX or TABLE.
 * @param source_group Source (means group name) of the new item.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_insert_after_index(Evas_Object *obj, const char *part, const char *item_name, unsigned int index, const char *source_group);

/**
 * @brief Inserts new item to box or table part directly into specified position.
 *
 * @param obj Object being edited.
 * @param part Part to add a new item. This part should have BOX or TABLE type.
 * @param item_name Name of new item that is not exist in BOX or TABLE yet.
 * @param source_group Source (means group name) of the new item.
 * @param place Specified place to insert item into. Place cannot be less than 0 or
 *              greater than current number of items in BOX or TABLE.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_insert_at(Evas_Object *obj, const char *part, const char *item_name, const char *source_group, unsigned int place);

/**
 * @brief Restacks existing item above.
 *
 * @param obj Object being edited.
 * @param part Part which contain items. This part should have BOX or TABLE type.
 * @param item_name Name of item that will be moved above.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EINA_DEPRECATED
EAPI Eina_Bool
edje_edit_part_item_move_above(Evas_Object *obj, const char *part, const char *item_name);

/**
 * @brief Restacks existing item above.
 *
 * @param obj Object being edited.
 * @param part Part which contain items. This part should have BOX or TABLE type.
 * @param index Index of item that will be moved above.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_move_above_index(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Restacks existing item below.
 *
 * @param obj Object being edited.
 * @param part Part which contain items. This part should have BOX or TABLE type.
 * @param item_name Name of item that will be moved below.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EINA_DEPRECATED
EAPI Eina_Bool
edje_edit_part_item_move_below(Evas_Object *obj, const char *part, const char *item_name);

/**
 * @brief Restacks existing item below.
 *
 * @param obj Object being edited.
 * @param part Part which contain items. This part should have BOX or TABLE type.
 * @param index Index of item that will be moved below.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_move_below_index(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Gets the list of all part items in the given edje.
 *
 * @param obj Object being edited.
 * @param part Name of part which is TABLE or BOX part and contain items.
 *
 * @return A List containing all part items names found in the edje file.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_List * edje_edit_part_items_list_get(Evas_Object *obj, const char *part);

/**
 * @brief Gets the count of part items in the given edje.
 *
 * @param obj Object being edited.
 * @param part Name of part which is TABLE or BOX part and contain items.
 *
 * @return A count part items in case of success, and -1 otherwise.
 * @since 1.18
 */
EAPI int
edje_edit_part_items_count_get(Evas_Object *obj, const char *part);

/**
 * @brief Deletes item from box or table part.
 *
 * @param obj Object being edited.
 * @param part Part to delete exist item. This part should have BOX or TABLE type.
 * @param name Name of exist item to delete it from BOX or TABLE.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_del(Evas_Object *obj, const char *part, const char* name);

/**
 * @brief Deletes item from box or table part by index.
 *
 * @param obj Object being edited.
 * @param part Part to delete exist item. This part should have BOX or TABLE type.
 * @param index Index of exist item to delete it from BOX or TABLE.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_del(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets name for item from table or box items.
 *
 * @param obj Object being edited.
 * @param part Part to change item's source. This part should have BOX or TABLE type.
 * @param index Index of item
 * @param name New item name.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_name_set(Evas_Object *obj, const char *part, unsigned int index, const char *name);

/**
 * @brief Gets name for item from table or box items.
 *
 * @param obj Object being edited.
 * @param part Part to return item's source. This part should have BOX or TABLE type.
 * @param index Index of item.
 *
 * @return name of the given item.
 * @since 1.18
 */
EAPI const char *
edje_edit_part_item_index_name_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets source for item from table or box items.
 *
 * @param obj Object being edited.
 * @param part Part to change item's source. This part should have BOX or TABLE type.
 * @param item_name Name of item.
 * @param source_group New gorup name.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_source_set(Evas_Object *obj, const char *part, const char *item_name, const char *source_group);

/**
 * @brief Sets source for item from table or box items.
 *
 * @param obj Object being edited.
 * @param part Part to change item's source. This part should have BOX or TABLE type.
 * @param index Index of item
 * @param source_group New gorup name.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_source_set(Evas_Object *obj, const char *part, unsigned int index, const char *source_group);

/**
 * @brief Gets source for item from table or box items.
 *
 * @param obj Object being edited.
 * @param part Part to return item's source. This part should have BOX or TABLE type.
 * @param item_name Name of item.
 *
 * @return source of the given item.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI const char * edje_edit_part_item_source_get(Evas_Object *obj, const char *part, const char *item_name);

/**
 * @brief Gets source for item from table or box items.
 *
 * @param obj Object being edited.
 * @param part Part to return item's source. This part should have BOX or TABLE type.
 * @param index Index of item.
 *
 * @return source of the given item.
 * @since 1.18
 */
EAPI const char *
edje_edit_part_item_index_source_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Gets the minimum width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get minimum width.
 *
 * @return The minimum width value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_min_w_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the minimum width value of a part's item.
 * The minimum width should be greater than 0.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set minimum width.
 * @param min_w Minimum width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_min_w_set(Evas_Object *obj, const char *part, const char *item, int min_w);

/**
 * @brief Gets the minimum height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get minimum height.
 *
 * @return The minimum height value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_min_h_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the minimum height value of a part's item.
 * The minimum height should be greater than 0.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set minimum height.
 * @param min_h Minimum height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_min_h_set(Evas_Object *obj, const char *part, const char *item, int min_h);

/**
 * @brief Gets the maximum width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get maximum width.
 *
 * @return The maximum width value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_max_w_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the maximum width value of a part's item.
 * The maximum width should be greater than -1.
 * The value -1 means that state doesn't have any boundaries on width direction.
 * (it can be any size that is bigger than it's min)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set maximum width.
 * @param max_w Maximum width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_max_w_set(Evas_Object *obj, const char *part, const char *item, int max_w);

/**
 * @brief Gets the maximum height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get maximum height.
 *
 * @return The maximum height value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_max_h_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the maximum height value of a part's item.
 * The maximum height should be greater than -1.
 * The value -1 means that state doesn't have any boundaries on height direction.
 * (it can be any size that is bigger than it's min)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set maximum height.
 * @param max_h Maximum height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_max_h_set(Evas_Object *obj, const char *part, const char *item, int max_h);

/**
 * @brief Gets the aspect width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get aspect width.
 *
 * @return The aspect width value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_aspect_w_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the aspect width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set aspect width.
 * @param aspect_w Aspect width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_aspect_w_set(Evas_Object *obj, const char *part, const char *item, int aspect_w);

/**
 * @brief Gets the aspect height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get aspect height.
 *
 * @return The maximum height value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_aspect_h_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the aspect height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set aspect height.
 * @param aspect_h Aspect height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_aspect_h_set(Evas_Object *obj, const char *part, const char *item, int aspect_h);

/**
 * @brief Gets the prefer width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get prefer width.
 *
 * @return The prefer width value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_prefer_w_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Gets aspect mode for an item of TABLE or BOX.
 *
 * This may return next values:
 * - EDJE_ASPECT_CONTROL_NONE
 * - EDJE_ASPECT_CONTROL_NEITHER
 * - EDJE_ASPECT_CONTROL_HORIZONTAL
 * - EDJE_ASPECT_CONTROL_VERTICAL
 * - EDJE_ASPECT_CONTROL_BOTH
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item to set aspect mode.
 *
 * @return One of possible enum Edje_Aspect_Control.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Edje_Aspect_Control
edje_edit_part_item_aspect_mode_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets aspect mode for an item of TABLE or BOX.
 *
 * Mode may be next:
 * - EDJE_ASPECT_CONTROL_NONE
 * - EDJE_ASPECT_CONTROL_NEITHER
 * - EDJE_ASPECT_CONTROL_HORIZONTAL
 * - EDJE_ASPECT_CONTROL_VERTICAL
 * - EDJE_ASPECT_CONTROL_BOTH
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item to set aspect mode.
 * @param mode One of possible enum from Edje_Aspect_Control:

 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_aspect_mode_set(Evas_Object *obj, const char *part, const char *item, Edje_Aspect_Control mode);

/**
 * @brief Sets the prefer width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set prefer width.
 * @param prefer_w Prefer width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_prefer_w_set(Evas_Object *obj, const char *part, const char *item, int prefer_w);

/**
 * @brief Gets the prefer height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get prefer height.
 *
 * @return The maximum height value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_prefer_h_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the prefer height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set prefer height.
 * @param prefer_h Prefer height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_prefer_h_set(Evas_Object *obj, const char *part, const char *item, int prefer_h);

/**
 * @brief Gets the spread width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get spread width.
 *
 * @return The spread width value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_spread_w_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the spread width value of a part's item.
 *
 * @attention be careful, if you set up huge number (like 10 or 100). width and height of
 * spread is being multiplied and you will get huge number of objects that may "eat"
 * all of your processor performance at once... Or if you want, you may
 * get some coffee and wait until it will recalculate all of those objects :)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set spread width.
 * @param spread_w Maximum width value.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_spread_w_set(Evas_Object *obj, const char *part, const char *item, int spread_w);

/**
 * @brief Gets the spread height value of a part's item.
 *
 * @attention be careful, if you set up huge number (like 10 or 100). width and height of
 * spread is being multiplied and you will get huge number of objects that may "eat"
 * all of your processor performance at once... Or if you want, you may
 * get some coffee and wait until it will recalculate all of those objects :)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to get spread height.
 *
 * @return The spread height value.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI int edje_edit_part_item_spread_h_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the spread height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param item The name of the item to set spread height.
 * @param spread_h spread height value.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_spread_h_set(Evas_Object *obj, const char *part, const char *item, int spread_h);

/**
 * @brief Gets the minimum width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get minimum width.
 *
 * @return The minimum width value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_min_w_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the minimum width value of a part's item.
 * The minimum width should be greater than 0.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set minimum width.
 * @param min_w Minimum width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_min_w_set(Evas_Object *obj, const char *part, unsigned int index, int min_w);

/**
 * @brief Gets the minimum height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get minimum height.
 *
 * @return The minimum height value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_min_h_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the minimum height value of a part's item.
 * The minimum height should be greater than 0.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set minimum height.
 * @param min_h Minimum height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_min_h_set(Evas_Object *obj, const char *part, unsigned int index, int min_h);

/**
 * @brief Gets the maximum width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get maximum width.
 *
 * @return The maximum width value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_max_w_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the maximum width value of a part's item.
 * The maximum width should be greater than -1.
 * The value -1 means that state doesn't have any boundaries on width direction.
 * (it can be any size that is bigger than it's min)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set maximum width.
 * @param max_w Maximum width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_max_w_set(Evas_Object *obj, const char *part, unsigned int index, int max_w);

/**
 * @brief Gets the maximum height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get maximum height.
 *
 * @return The maximum height value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_max_h_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the maximum height value of a part's item.
 * The maximum height should be greater than -1.
 * The value -1 means that state doesn't have any boundaries on height direction.
 * (it can be any size that is bigger than it's min)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set maximum height.
 * @param max_h Maximum height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_max_h_set(Evas_Object *obj, const char *part, unsigned int index, int max_h);

/**
 * @brief Gets the aspect width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get aspect width.
 *
 * @return The aspect width value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_aspect_w_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the aspect width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set aspect width.
 * @param aspect_w Aspect width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_aspect_w_set(Evas_Object *obj, const char *part, unsigned int index, int aspect_w);

/**
 * @brief Gets the aspect height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get aspect height.
 *
 * @return The maximum height value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_aspect_h_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the aspect height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set aspect height.
 * @param aspect_h Aspect height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_aspect_h_set(Evas_Object *obj, const char *part, unsigned int index, int aspect_h);

/**
 * @brief Gets the prefer width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get prefer width.
 *
 * @return The prefer width value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_prefer_w_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Gets aspect mode for an item of TABLE or BOX.
 *
 * This may return next values:
 * - EDJE_ASPECT_CONTROL_NONE
 * - EDJE_ASPECT_CONTROL_NEITHER
 * - EDJE_ASPECT_CONTROL_HORIZONTAL
 * - EDJE_ASPECT_CONTROL_VERTICAL
 * - EDJE_ASPECT_CONTROL_BOTH
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to set aspect mode.
 *
 * @return One of possible enum Edje_Aspect_Control.
 * @since 1.18
 */
EAPI Edje_Aspect_Control
edje_edit_part_item_index_aspect_mode_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets aspect mode for an item of TABLE or BOX.
 *
 * Mode may be next:
 * - EDJE_ASPECT_CONTROL_NONE
 * - EDJE_ASPECT_CONTROL_NEITHER
 * - EDJE_ASPECT_CONTROL_HORIZONTAL
 * - EDJE_ASPECT_CONTROL_VERTICAL
 * - EDJE_ASPECT_CONTROL_BOTH
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to set aspect mode.
 * @param mode One of possible enum from Edje_Aspect_Control:

 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_aspect_mode_set(Evas_Object *obj, const char *part, unsigned int index, Edje_Aspect_Control mode);

/**
 * @brief Sets the prefer width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set prefer width.
 * @param prefer_w Prefer width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_prefer_w_set(Evas_Object *obj, const char *part, unsigned int index, int prefer_w);

/**
 * @brief Gets the prefer height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get prefer height.
 *
 * @return The maximum height value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_prefer_h_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the prefer height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set prefer height.
 * @param prefer_h Prefer height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_prefer_h_set(Evas_Object *obj, const char *part, unsigned int index, int prefer_h);

/**
 * @brief Gets the spread width value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get spread width.
 *
 * @return The spread width value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_spread_w_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the spread width value of a part's item.
 *
 * @attention be careful, if you set up huge number (like 10 or 100). width and height of
 * spread is being multiplied and you will get huge number of objects that may "eat"
 * all of your processor performance at once... Or if you want, you may
 * get some coffee and wait until it will recalculate all of those objects :)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set spread width.
 * @param spread_w Maximum width value.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_spread_w_set(Evas_Object *obj, const char *part, unsigned int index, int spread_w);

/**
 * @brief Gets the spread height value of a part's item.
 *
 * @attention be careful, if you set up huge number (like 10 or 100). width and height of
 * spread is being multiplied and you will get huge number of objects that may "eat"
 * all of your processor performance at once... Or if you want, you may
 * get some coffee and wait until it will recalculate all of those objects :)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to get spread height.
 *
 * @return The spread height value.
 * @since 1.18
 */
EAPI int
edje_edit_part_item_index_spread_h_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the spread height value of a part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param index Index of the item to set spread height.
 * @param spread_h spread height value.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_spread_h_set(Evas_Object *obj, const char *part, unsigned int index, int spread_h);

/**
 * @brief Gets paddings of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item_name The name of the item.
 * @param l A pointer to store the left padding value.
 * @param r A pointer to store the right padding value.
 * @param t A pointer to store the top padding value.
 * @param b A pointer to store the bottom padding value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_padding_get(Evas_Object *obj, const char *part, const char *item_name, int *l, int *r, int *t, int *b);

/**
 * @brief Sets paddings of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item_name The name of the item.
 * @param l Value of the left padding.
 * @param r Value of the right padding.
 * @param t Value of the top padding.
 * @param b Value of the bottom padding.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_padding_set(Evas_Object *obj, const char *part, const char *item_name, int l, int r, int t, int b);

/**
 * @brief Gets paddings of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item.
 * @param l A pointer to store the left padding value.
 * @param r A pointer to store the right padding value.
 * @param t A pointer to store the top padding value.
 * @param b A pointer to store the bottom padding value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_padding_get(Evas_Object *obj, const char *part, unsigned int index, int *l, int *r, int *t, int *b);

/**
 * @brief Sets paddings of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item.
 * @param l Value of the left padding.
 * @param r Value of the right padding.
 * @param t Value of the top padding.
 * @param b Value of the bottom padding.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_padding_set(Evas_Object *obj, const char *part, unsigned int index, int l, int r, int t, int b);

/**
 * @brief Gets the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item to get horizontal align value.
 *
 * @return The horizontal align value for the given align (value is between -1.0 and 1.0)
 * @since 1.11
 */
EINA_DEPRECATED
EAPI double edje_edit_part_item_align_x_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Gets the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to get horizontal align value.
 *
 * @return The horizontal align value for the given align (value is between -1.0 and 1.0)
 * @since 1.18
 */
EAPI double
edje_edit_part_item_index_align_x_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain itemf
 * @param item The name of the item to set horizontal align value.
 * @param align_x New value of the horizontal align.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_align_x_set(Evas_Object *obj, const char *part, const char *item, double align_x);

/**
 * @brief Sets the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain itemf
 * @param index Index of the item to set horizontal align value.
 * @param align_x New value of the horizontal align.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_align_x_set(Evas_Object *obj, const char *part, unsigned int index, double align_x);

/**
 * @brief Gets the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item to get vertical align value.
 *
 * @return The vertical align value for the given align (value is between -1.0 and 1.0)
 * @since 1.11
 */
EINA_DEPRECATED
EAPI double edje_edit_part_item_align_y_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Gets the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to get vertical align value.
 *
 * @return The vertical align value for the given align (value is between -1.0 and 1.0)
 * @since 1.18
 */
EAPI double
edje_edit_part_item_index_align_y_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item to set vertical align value.
 * @param align_y New value of the vertical align.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_align_y_set(Evas_Object *obj, const char *part, const char *item, double align_y);

/**
 * @brief Sets the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to set vertical align value.
 * @param align_y New value of the vertical align.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_align_y_set(Evas_Object *obj, const char *part, unsigned int index, double align_y);

/**
 * @brief Gets the horizontal weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item to get horizontal weight value.
 *
 * @return The horizontal weight value for the given item (value is between -1.0 and 1.0)
 * @since 1.11
 */
EINA_DEPRECATED
EAPI double edje_edit_part_item_weight_x_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Gets the horizontal weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to get horizontal weight value.
 *
 * @return The horizontal weight value for the given item (value is between -1.0 and 1.0)
 * @since 1.18
 */
EAPI double
edje_edit_part_item_index_weight_x_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the horizontal we value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain itemf
 * @param item The name of the item to set horizontal weight value.
 * @param weight_x New value of the horizontal weight.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_weight_x_set(Evas_Object *obj, const char *part, const char *item, double weight_x);

/**
 * @brief Sets the horizontal we value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain itemf
 * @param index Index of the item to set horizontal weight value.
 * @param weight_x New value of the horizontal weight.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_weight_x_set(Evas_Object *obj, const char *part, unsigned int index, double weight_x);

/**
 * @brief Gets the vertical weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item to get vertical weight value.
 *
 * @return The vertical weight value for the given item (value is between -1.0 and 1.0)
 * @since 1.11
 */
EINA_DEPRECATED
EAPI double edje_edit_part_item_weight_y_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Gets the vertical weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to get vertical weight value.
 *
 * @return The vertical weight value for the given item (value is between -1.0 and 1.0)
 * @since 1.18
 */
EAPI double
edje_edit_part_item_index_weight_y_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the vertical weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item to set vertical weight value.
 * @param weight_y New value of the vertical weight.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_weight_y_set(Evas_Object *obj, const char *part, const char *item, double weight_y);

/**
 * @brief Sets the vertical weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to set vertical weight value.
 * @param weight_y New value of the vertical weight.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_weight_y_set(Evas_Object *obj, const char *part, unsigned int index, double weight_y);

/**
 * @deprecated Use edje_edit_part_item_position_col_get() and
 * edje_edit_part_item_position_row_get() instead.
 *
 * @brief Gets column/row position of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item_name The name of the item.
 * @param col Column item position.
 * @param row Row item position.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_position_get(Evas_Object *obj, const char *part, const char *item_name, unsigned short *col, unsigned short *row);

/**
 * @brief Gets the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to get horizontal align value.
 *
 * @return The horizontal align value for the given align (value is between -1.0 and 1.0)
 * @since 1.18
 */
EAPI double
edje_edit_part_item_item_align_x_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item
 * @param index Index of the item to set horizontal align value.
 * @param align_x New value of the horizontal align.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_item_align_x_set(Evas_Object *obj, const char *part, unsigned int index, double align_x);

/**
 * @brief Gets the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to get vertical align value.
 *
 * @return The vertical align value for the given align (value is between -1.0 and 1.0)
 * @since 1.18
 */
EAPI double
edje_edit_part_item_item_align_y_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to set vertical align value.
 * @param align_y New value of the vertical align.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_item_align_y_set(Evas_Object *obj, const char *part, unsigned int index, double align_y);

/**
 * @brief Gets the horizontal weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to get horizontal weight value.
 *
 * @return The horizontal weight value for the given item (value is between -1.0 and 1.0)
 * @since 1.18
 */
EAPI double
edje_edit_part_item_item_weight_x_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the horizontal we value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain itemf
 * @param index Index of the item to set horizontal weight value.
 * @param weight_x New value of the horizontal weight.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_item_weight_x_set(Evas_Object *obj, const char *part, unsigned int index, double weight_x);

/**
 * @brief Gets the vertical weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to get vertical weight value.
 *
 * @return The vertical weight value for the given item (value is between -1.0 and 1.0)
 * @since 1.18
 */
EAPI double
edje_edit_part_item_item_weight_y_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Sets the vertical weight value of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item to set vertical weight value.
 * @param weight_y New value of the vertical weight.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_item_weight_y_set(Evas_Object *obj, const char *part, unsigned int index, double weight_y);

/**
 * @brief Gets column position of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item_name The name of the item.
 *
 * @return The item column value.
 * @since 1.16
 */
EINA_DEPRECATED
EAPI unsigned short
edje_edit_part_item_position_col_get(Evas_Object *obj, const char *part, const char *item_name);

/**
 * @brief Gets row position of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item_name The name of the item.
 *
 * @return The item row value.
 * @since 1.16
 */
EINA_DEPRECATED
EAPI unsigned short
edje_edit_part_item_position_row_get(Evas_Object *obj, const char *part, const char *item_name);

/**
 * @brief Gets column position of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item.
 *
 * @return The item column value.
 * @since 1.18
 */
EAPI unsigned short
edje_edit_part_item_index_position_col_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Gets row position of the part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item.
 *
 * @return The item row value.
 * @since 1.18
 */
EAPI unsigned short
edje_edit_part_item_index_position_row_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @deprecated Use edje_edit_part_item_position_col_set() and
 * edje_edit_part_item_position_row_set() instead.
 *
 * @brief Sets column/row position of a new part's item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item_name The name of the item.
 * @param col Column item position.
 * @param row Row item position.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_position_set(Evas_Object *obj, const char *part, const char *item_name, unsigned short col, unsigned short row);

/**
 * @brief Sets column position of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item_name The name of the item.
 * @param col Column item position.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EINA_DEPRECATED
EAPI Eina_Bool
edje_edit_part_item_position_col_set(Evas_Object *obj, const char *part, const char *item_name, unsigned short col);

/**
 * @brief Sets row position of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item_name The name of the item.
 * @param row Row item position.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EINA_DEPRECATED
EAPI Eina_Bool
edje_edit_part_item_position_row_set(Evas_Object *obj, const char *part, const char *item_name, unsigned short row);

/**
 * @brief Sets column position of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item.
 * @param col Column item position.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_position_col_set(Evas_Object *obj, const char *part, unsigned int index, unsigned short col);

/**
 * @brief Sets row position of a part item.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item.
 * @param row Row item position.
 *
 * @return @c EINA_TRUE If successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_position_row_set(Evas_Object *obj, const char *part, unsigned int index, unsigned short row);


/**
 * @brief Retrieves the how many columns and rows will span for use by item.
 *
 * @param obj object being edited.
 * @param part part that contain item.
 * @param item the name of the item of part.
 * @param col Pointer to an unsigned char in which to store the columns count.
 * @param row Pointer to an unsigned char in which to store the rows count.
 *
 * @deprecated Use edje_edit_part_item_span_row_get() and
 * edje_edit_part_item_span_col_get() instead.
 *
 * @since 1.11
 */
EINA_DEPRECATED
EAPI void edje_edit_part_item_span_get(Evas_Object *obj, const char *part, const char *item, unsigned char *col, unsigned char *row);

/**
 * @brief Gets the number of span columns.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item of part.
 *
 * @return The count of span columns.
 * @since 1.16
 */
EINA_DEPRECATED
EAPI unsigned short
edje_edit_part_item_span_col_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Gets the number of span rows.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item of part.
 *
 * @return The count of span rows.
 * @since 1.16
 */
EINA_DEPRECATED
EAPI unsigned short
edje_edit_part_item_span_row_get(Evas_Object *obj, const char *part, const char *item);

/**
 * @brief Sets the count of columns and rows, which this item will spans for use.
 *
 * @param obj object being edited.
 * @param part part that contain item.
 * @param item the name of the item to set new count of columns spans.
 * @param col new count of the columns spans.
 * @param row new count of the rows spans.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EINA_DEPRECATED
EAPI Eina_Bool edje_edit_part_item_span_set(Evas_Object *obj, const char *part, const char *item, unsigned char col, unsigned char row);

/**
 * @brief Sets the count of columns which this item will spans for use.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item.
 * @param col new count of the columns spans.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EINA_DEPRECATED
EAPI Eina_Bool
edje_edit_part_item_span_col_set(Evas_Object *obj, const char *part, const char *item, unsigned short col);

/**
 * @brief Sets the count of rows which this item will spans for use.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param item The name of the item.
 * @param row new count of the rows spans.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.16
 */
EINA_DEPRECATED
EAPI Eina_Bool
edje_edit_part_item_span_row_set(Evas_Object *obj, const char *part, const char *item, unsigned short row);

/**
 * @brief Sets the count of columns which this item will spans for use.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item.
 * @param col new count of the columns spans.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_span_col_set(Evas_Object *obj, const char *part, unsigned int index, unsigned short col);

/**
 * @brief Sets the count of rows which this item will spans for use.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item.
 * @param row new count of the rows spans.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_part_item_index_span_row_set(Evas_Object *obj, const char *part, unsigned int index, unsigned short row);

/**
 * @brief Gets the number of span columns.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item of part.
 *
 * @return The count of span columns.
 * @since 1.18
 */
EAPI unsigned short
edje_edit_part_item_index_span_col_get(Evas_Object *obj, const char *part, unsigned int index);

/**
 * @brief Gets the number of span rows.
 *
 * @param obj Object being edited.
 * @param part Part that contain item.
 * @param index Index of the item of part.
 *
 * @return The count of span rows.
 * @since 1.18
 */
EAPI unsigned short
edje_edit_part_item_index_span_row_get(Evas_Object *obj, const char *part, unsigned int index);

//@}
/******************************************************************************/
/**************************   STATES API   ************************************/
/******************************************************************************/
/**
 * @name States API
 * Functions to deal with part states (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the states in the given part.
 *
 * @param obj Object being edited.
 * @param part Part to get the states names list.
 *
 * @return An Eina_List* of string (char *)containing all the states names found
 * in part, including the float value (ex: "default 0.00").
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 */
EAPI Eina_List * edje_edit_part_states_list_get(Evas_Object *obj, const char *part);

/**
 * @brief Sets a new name for the given state in the given part.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state Name of the state to rename.
 * @param value Value of the state to rename.
 * @param new_name The new name for the state.
 * @param new_value The new value for the state.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_name_set(Evas_Object *obj, const char *part, const char *state, double value, const char *new_name, double new_value);

/**
 * @brief Creates a new state to the give part.
 *
 * @param obj Object being edited.
 * @param part Part to set the name of the new state.
 * @param name Name for the new state (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE if successfully, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_add(Evas_Object *obj, const char *part, const char *name, double value);

/**
 * @brief Deletes the given part state from the edje.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The current name of the state (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE if successfully, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_del(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Checks if a part state with the given name exist.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to check (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE if the part state exist, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_exist(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Copies the state @p from into @p to. If @p to doesn't exist it will be created.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param from State to copy from (not including state value).
 * @param val_from The value of the state to copy from.
 * @param to State to copy into (not including state value).
 * @param val_to The value of the state to copy into.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_copy(Evas_Object *obj, const char *part, const char *from, double val_from, const char *to, double val_to);

/**
 * @brief Gets the 'rel1 relative X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel1 relative X' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel1 relative X' value of the part state.
 */
EAPI double edje_edit_state_rel1_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the 'rel1 relative Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel1 relative Y' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel1 relative Y' value of the part state.
 */
EAPI double edje_edit_state_rel1_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the 'rel2 relative X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel2 relative X' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel2 relative X' value of the part state.
 */
EAPI double edje_edit_state_rel2_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the 'rel2 relative Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel2 relative Y' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel2 relative Y' value of the part state.
 */
EAPI double edje_edit_state_rel2_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the 'rel1 relative X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel1 relative X' (not including the state value).
 * @param value The state value.
 * @param x The new 'rel1 relative X' value to set'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/**
 * @brief Sets the 'rel1 relative Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel1 relative Y' (not including the state value).
 * @param value The state value.
 * @param y The new 'rel1 relative Y' value to set'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/**
 * @brief Sets the 'rel2 relative X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel2 relative X' (not including the state value).
 * @param value The state value.
 * @param x The new 'rel2 relative X' value to set'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/**
 * @brief Sets the 'rel2 relative Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel2 relative Y' (not including the state value).
 * @param value The state value.
 * @param y The new 'rel2 relative Y' value to set'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/**
 * @brief Gets the 'rel1 offset X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel1 offset X' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel1 offset X' value of the part state.
 */
EAPI int edje_edit_state_rel1_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the 'rel1 offset Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel1 offset Y' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel1 offset Y' value of the part state.
 */
EAPI int edje_edit_state_rel1_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the 'rel2 offset X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel2 offset X' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel2 offset X' value of the part state.
 */
EAPI int edje_edit_state_rel2_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the 'rel2 offset Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel2 offset Y' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel2 offset Y' value of the part state.
 */
EAPI int edje_edit_state_rel2_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the 'rel1 offset X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel1 offset X' (not including the state value).
 * @param value The state value.
 * @param x The new 'rel1 offset X' value to set'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, int x);

/**
 * @brief Sets the 'rel1 offset Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel1 offset Y' (not including the state value).
 * @param value The state value.
 * @param y The new 'rel1 offset Y' value to set'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, int y);

/**
 * @brief Sets the 'rel2 offset X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel2 offset X' (not including the state value).
 * @param value The state value.
 * @param x The new 'rel2 offset X' value to set'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, int x);

/**
 * @brief Sets the 'rel2 offset Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel2 offset Y' (not including the state value).
 * @param value The state value.
 * @param y The new 'rel2 offset Y' value to set'.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, int y);

/**
 * @brief Gets the part name rel1x is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The state that contain which the part name rel1x is relative to (not including the state value).
 * @param value The state value.
 *
 * @return The part name rel1x is relative to, or NULL if the part is relative to the whole interface.
 */
EAPI const char * edje_edit_state_rel1_to_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the part name rel1y is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The state that contain which the part name rel1y is relative to (not including the state value).
 * @param value The state value.
 *
 * @return The part name rel1y is relative to, or NULL if the part is relative to the whole interface.
 */
EAPI const char * edje_edit_state_rel1_to_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the part name rel2x is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The state that contain which the part name rel2x is relative to (not including the state value).
 * @param value The state value.
 *
 * @return The part name rel2x is relative to, or NULL if the part is relative to the whole interface.
 */
EAPI const char * edje_edit_state_rel2_to_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the part name rel2y is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The state that contain which the part name rel2y is relative to (not including the state value).
 * @param value The state value.
 *
 * @return The part name rel2y is relative to, or NULL if the part is relative to the whole interface.
 */
EAPI const char * edje_edit_state_rel2_to_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the part rel1x is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set rel1x is relative to (not including the state value).
 * @param value The state value.
 * @param rel_to The name of the part that is used as container/parent (NULL make the part relative to the whole interface).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_to_x_set(Evas_Object *obj, const char *part, const char *state, double value, const char *rel_to);

/**
 * @brief Sets the part rel1y is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set rel1y is relative to (not including the state value).
 * @param value The state value.
 * @param rel_to The name of the part that is used as container/parent (NULL make the part relative to the whole interface).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_to_y_set(Evas_Object *obj, const char *part, const char *state, double value, const char *rel_to);

/**
 * @brief Sets the part rel2x is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set rel2x is relative to (not including the state value).
 * @param value The state value.
 * @param rel_to The name of the part that is used as container/parent (NULL make the part relative to the whole interface).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_to_x_set(Evas_Object *obj, const char *part, const char *state, double value, const char *rel_to);

/**
 * @brief Sets the part rel2y is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set rel2y is relative to (not including the state value).
 * @param value The state value.
 * @param rel_to The name of the part that is used as container/parent (NULL make the part relative to the whole interface).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_to_y_set(Evas_Object *obj, const char *part, const char *state, double value, const char *rel_to);

/**
 * @brief Gets the color of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get color (not including the state value).
 * @param value The state value.
 * @param r A pointer to store the red value.
 * @param g A pointer to store the green value.
 * @param b A pointer to store the blue value.
 * @param a A pointer to store the alpha value.
 */
EAPI void edje_edit_state_color_get(Evas_Object *obj, const char *part, const char *state, double value, int *r, int *g, int *b, int *a);

/**
 * @brief Gets the color2 of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get color (not including the state value).
 * @param value The state value.
 * @param r A pointer to store the red value.
 * @param g A pointer to store the green value.
 * @param b A pointer to store the blue value.
 * @param a A pointer to store the alpha value.
 */
EAPI void edje_edit_state_color2_get(Evas_Object *obj, const char *part, const char *state, double value, int *r, int *g, int *b, int *a);

/**
 * @brief Gets the color3 of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get color (not including the state value).
 * @param value The state value.
 * @param r A pointer to store the red value.
 * @param g A pointer to store the green value.
 * @param b A pointer to store the blue value.
 * @param a A pointer to store the alpha value.
 */
EAPI void edje_edit_state_color3_get(Evas_Object *obj, const char *part, const char *state, double value, int *r, int *g, int *b, int *a);

/**
 * @brief Sets the color of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set color (not including the state value).
 * @param value The state value.
 * @param r The red value of the color.
 * @param g The green value of the color.
 * @param b The blue value of the color.
 * @param a The alpha value of the color.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_color_set(Evas_Object *obj, const char *part, const char *state, double value, int r, int g, int b, int a);

/**
 * @brief Sets the color2 of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set color (not including the state value).
 * @param value The state value.
 * @param r The red value of the color.
 * @param g The green value of the color.
 * @param b The blue value of the color.
 * @param a The alpha value of the color.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_color2_set(Evas_Object *obj, const char *part, const char *state, double value, int r, int g, int b, int a);

/**
 * @brief Sets the color3 of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set color (not including the state value).
 * @param value The state value.
 * @param r The red value of the color.
 * @param g The green value of the color.
 * @param b The blue value of the color.
 * @param a The alpha value of the color.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_color3_set(Evas_Object *obj, const char *part, const char *state, double value, int r, int g, int b, int a);

/**
 * @brief Gets the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get horizontal align (not including the state value).
 * @param value The state value.
 *
 * @return The horizontal align value for the given state
 */
EAPI double edje_edit_state_align_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get horizontal align (not including the state value).
 * @param value The state value.
 *
 * @return The vertical align value for the given state
 */
EAPI double edje_edit_state_align_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get horizontal align (not including the state value).
 * @param value The state value.
 * @param align The new vertical align value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_align_x_set(Evas_Object *obj, const char *part, const char *state, double value,  double align);

/**
 * @brief Sets the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get vertical align (not including the state value).
 * @param value The state value.
 * @param align The new vertical align value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_align_y_set(Evas_Object *obj, const char *part, const char *state, double value,  double align);

/**
 * @brief Sets the size class of the given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set size class (not including the state value).
 * @param value The state value.
 * @param text_class The size class to assign.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_state_text_class_set(Evas_Object *obj, const char *part, const char *state, double value, const char *size_class);

/**
 * @brief Gets the minimum width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get minimum width (not including the state value).
 * @param value The state value.
 *
 * @return The minimum width value.
 */
EAPI int edje_edit_state_min_w_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the minimum width value of a part state.
 * The minimum width should be greater than 0.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set minimum width (not including the state value).
 * @param value The state value.
 * @param min_w Minimum width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_min_w_set(Evas_Object *obj, const char *part, const char *state, double value, int min_w);

/**
 * @brief Gets the minimum height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get minimum height (not including the state value).
 * @param value The state value.
 *
 * @return The minimum height value.
 */
EAPI int edje_edit_state_min_h_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the minimum height value of a part state.
 * The minimum height should be greater than 0.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set minimum height (not including the state value).
 * @param value The state value.
 * @param min_h Minimum height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_min_h_set(Evas_Object *obj, const char *part, const char *state, double value, int min_h);

/**
 * @brief Gets the maximum width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get maximum width (not including the state value).
 * @param value The state value.
 *
 * @return The maximum width value.
 */
EAPI int edje_edit_state_max_w_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the maximum width value of a part state.
 * The maximum width should be greater than -1.
 * The value -1 means that state doesn't have any boundaries on width direction.
 * (it can be any size that is bigger than it's min)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set maximum width (not including the state value).
 * @param value The state value.
 * @param max_w Maximum width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_max_w_set(Evas_Object *obj, const char *part, const char *state, double value, int max_w);

/**
 * @brief Gets the maximum height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get maximum height (not including the state value).
 * @param value The state value.
 *
 * @return The maximum height value.
 */
EAPI int edje_edit_state_max_h_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the maximum height value of a part state.
 * The maximum height should be greater than -1.
 * The value -1 means that state doesn't have any boundaries on height direction.
 * (it can be any size that is bigger than it's min)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set maximum height (not including the state value).
 * @param value The state value.
 * @param max_h Maximum height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_max_h_set(Evas_Object *obj, const char *part, const char *state, double value, int max_h);

/**
 * @brief Gets the multiplier (minmul) width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get multiplier width (not including the state value).
 * @param value The state value.
 *
 * @return The maximum width value.
 * @since 1.11
 */
EAPI double edje_edit_state_minmul_w_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the multiplier (minmul) width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set multiplier width (not including the state value).
 * @param value The state value.
 * @param minmul_w Multiplier width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_state_minmul_w_set(Evas_Object *obj, const char *part, const char *state, double value, double minmul_w);

/**
 * @brief Gets the multiplier (minmul) height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get multiplier height (not including the state value).
 * @param value The state value.
 *
 * @return The maximum height value.
 * @since 1.11
 */
EAPI double edje_edit_state_minmul_h_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the multiplier (minmul) height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set multiplier height (not including the state value).
 * @param value The state value.
 * @param minmul_h Multiplier height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_state_minmul_h_set(Evas_Object *obj, const char *part, const char *state, double value, double minmul_h);

/**
 * @brief Gets the fixed width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fixed width value (not including the state value).
 * @param value The state value.
 *
 * @return The fixed width value.
 */
EAPI Eina_Bool edje_edit_state_fixed_w_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the fixed width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fixed width value (not including the state value).
 * @param value The state value.
 * @param fixed Fixed width value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fixed_w_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool fixed);

/**
 * @brief Gets the fixed height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fixed height value (not including the state value).
 * @param value The state value.
 *
 * @return The fixed height value.
 */
EAPI Eina_Bool edje_edit_state_fixed_h_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the fixed height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set maximum height (not including the state value).
 * @param value The state value.
 * @param fixed Fixed height value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fixed_h_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool fixed);

/**
 * @brief Gets the minimum aspect value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get minimum aspect (not including the state value).
 * @param value The state value.
 *
 * @return The minimum aspect
 */
EAPI double edje_edit_state_aspect_min_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the maximum aspect value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get maximum aspect (not including the state value).
 * @param value The state value.
 *
 * @return The maximum aspect
 */
EAPI double edje_edit_state_aspect_max_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the minimum aspect value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set minimum aspect (not including the state value).
 * @param value The state value.
 * @param aspect Minimum aspect value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_aspect_min_set(Evas_Object *obj, const char *part, const char *state, double value, double aspect);

/**
 * @brief Sets the maximum aspect value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set maximum aspect (not including the state value).
 * @param value The state value.
 * @param aspect Maximum aspect value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_aspect_max_set(Evas_Object *obj, const char *part, const char *state, double value, double aspect);

/**
 * @brief Gets the aspect preference of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get aspect preference (not including the state value).
 * @param value The state value.
 *
 * @return The aspect preference (0 = None, 1 = Vertical, 2 = Horizontal, 3 = Both)
 */
EAPI unsigned char edje_edit_state_aspect_pref_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the aspect preference of a part state.
 *
 * The available values of aspect preference are:
 * <ul style="list-style-type:none">
 *     <li>0 - None</li>
 *     <li>1 - Vertical</li>
 *     <li>2 - Horizontal</li>
 *     <li>3 - Both</li>
 *     <li>4 - Source</li>
 * </ul>
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set aspect preference (not
 *              including the state value).
 * @param value The state value.
 * @param pref The aspect preference to be set
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_aspect_pref_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char pref);

/**
 * @brief Gets the smooth property for given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the fill horizontal origin relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The smooth value.
 */
EAPI Eina_Bool edje_edit_state_fill_smooth_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the smooth property for given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal origin relative to area (not including the state value).
 * @param value The state value.
 * @param smooth The smooth value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_smooth_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool smooth);

/**
 * @brief Gets the fill type property for given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 *
 * @return The value that represents fill type: 0 for SCALE or 1 for TILE. In case of error (for example, if part type does not match) returns 2.
 * @see edje_edit_state_fill_type_set()
 * @since 1.11
 */
EAPI unsigned char edje_edit_state_fill_type_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the fill type property for given part state.
 *
 * Sets the image fill type. The available types are:
 * <dl>
 * <dt>SCALE</dt>
 * <dd>image will be scaled accordingly to the 'relative' and 'offset' params values from 'origin' and 'size' blocks.</dd>
 * <dt>TILE</dt>
 * <dd>image will be tiled accordingly to the 'relative' and 'offset' params values from 'origin' and 'size' blocks.</dd>
 * </dl>
 * <b>Important</b>: the part parameter 'min' must be set, it's size of tiled image.
 * If parameter 'max' is set tiled area will be resized accordingly to the 'max' values of part.
 * The default value of fill type is SCALE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 * @param fill_type The value that represents fill type: 0 for SCALE or 1 for TILE.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @see edje_edit_state_fill_type_get()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_state_fill_type_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char fill_type);

/**
 * @brief Gets the fill horizontal origin relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the fill horizontal origin relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill horizontal origin relative to area.
 */
EAPI double edje_edit_state_fill_origin_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the fill vertical origin relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill vertical origin relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill vertical origin relative to area.
 */
EAPI double edje_edit_state_fill_origin_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the fill horizontal origin offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill horizontal origin offset relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill horizontal origin offset relative to area.
 */
EAPI int edje_edit_state_fill_origin_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the fill vertical origin offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill vertical origin offset relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill vertical origin offset value.
 */
EAPI int edje_edit_state_fill_origin_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the fill horizontal origin relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal origin relative to area (not including the state value).
 * @param value The state value.
 * @param x The fill horizontal origin value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_origin_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/**
 * @brief Sets the fill horizontal origin relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill vertical origin relative to area (not including the state value).
 * @param value The state value.
 * @param y The fill vertical origin value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_origin_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/**
 * @brief Sets the fill horizontal origin offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal origin offset relative to area (not including the state value).
 * @param value The state value.
 * @param x The fill horizontal origin offset value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_origin_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/**
 * @brief Sets the fill vertical origin offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill vertical origin offset relative to area (not including the state value).
 * @param value The state value.
 * @param y The fill vertical origin offset value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_origin_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/**
 * @brief Gets the fill horizontal size relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill horizontal size relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill horizontal size relative to area.
 */
EAPI double edje_edit_state_fill_size_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the fill vertical size relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill vertical size relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill vertical size relative to area.
 */
EAPI double edje_edit_state_fill_size_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the fill horizontal size offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill horizontal size
 * offset relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill horizontal size offset relative to area.
 */
EAPI int edje_edit_state_fill_size_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the fill vertical size offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill vertical size offset
 * relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill vertical size offset relative to area.
 */
EAPI int edje_edit_state_fill_size_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the fill horizontal size relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal size
 * relative value (not including the state value).
 * @param value The state value.
 * @param x The horizontal size relative value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_size_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/**
 * @brief Sets the fill vertical size relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill vertical size
 * relative value (not including the state value).
 * @param value The state value.
 * @param x The vertical size relative value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_size_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/**
 * @brief Sets the fill horizontal size offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal size
 * offset relative value (not including the state value).
 * @param value The state value.
 * @param x The horizontal size offset value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_size_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/**
 * @brief Sets the fill vertical size offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill vertical size offset
 * relative value (not including the state value).
 * @param value The state value.
 * @param y The vertical size offset value.
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_size_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/**
 * @brief Gets the visibility of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get visibility (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE if the state is visible, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_visible_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the visibility of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set visibility (not including the state value).
 * @param value The state value.
 * @param visible To set state visible (EINA_TRUE if the state is visible, @c EINA_FALSE otherwise)
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_visible_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool visible);

/**
 * @brief Gets the color class of the given part state.
 *
 * Remember to free the string with edje_edit_string_free()
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get color class (not including the state value).
 * @param value The state value.
 *
 * @return The current color class.
 */
EAPI const char *edje_edit_state_color_class_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the color class of the given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set color class (not including the state value).
 * @param value The state value.
 * @param color_class The color class to assign.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_color_class_set(Evas_Object *obj, const char *part, const char *state, double value, const char *color_class);

/**
 * @brief Gets the list of parameters for an external part.
 *
 * DO NOT FREE THE LIST!
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get list of Edje_External_Param (not including the state value).
 * @param value The state value.
 *
 * @return The list of Edje_External_Param.
 */
EAPI const Eina_List * edje_edit_state_external_params_list_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the external parameter type and value.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter to look for.
 * @param type The type of the parameter will be stored here.
 * @param val Pointer to value will be stored here - DO NOT FREE IT!
 *
 * @return @c EINA_TRUE if the parameter was found, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Edje_External_Param_Type *type, void **val);

/**
 * @brief Gets external parameter of type INT.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type INT (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val The value of the parameter.
 *
 * @return @c EINA_TRUE if successful. @c EINA_FALSE if not found or is of different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_int_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, int *val);

/**
 * @brief Gets external parameter of type BOOL.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type BOOL (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val The value of the parameter.
 *
 * @return @c EINA_TRUE if successful. @c EINA_FALSE if not found or is of different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_bool_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Eina_Bool *val);

/**
 * @brief Gets external parameter of type DOUBLE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type DOUBLE (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val The value of the parameter.
 *
 * @return @c EINA_TRUE if successful. @c EINA_FALSE if not found or is of different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_double_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, double *val);

/**
 * @brief Gets external parameter of type STRING.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of
 *              type STRING (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val The value of the parameter.
 *
 * @return @c EINA_TRUE if successful. @c EINA_FALSE if not found or is of
 * different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_string_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char **val);

/**
 * @brief Gets external parameter of type CHOICE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of
 *        type CHOICE (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val The value of the parameter.
 *
 * @return @c EINA_TRUE if successful. @c EINA_FALSE if not found or is of
 * different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_choice_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char **val);

/**
 * @brief Sets the external parameter type and value, adding it if it didn't
 * exist before.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter (not
 *              including the state value).
 * @param value The state value.
 * @param param The name of the parameter set.
 * @param type The type of the parameter.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */

/**
 * Arguments should have proper sized values matching their types:
 *   - EDJE_EXTERNAL_PARAM_TYPE_INT: int
 *   - EDJE_EXTERNAL_PARAM_TYPE_BOOL: int
 *   - EDJE_EXTERNAL_PARAM_TYPE_DOUBLE: double
 *   - EDJE_EXTERNAL_PARAM_TYPE_STRING: char*
 *   - EDJE_EXTERNAL_PARAM_TYPE_CHOICE: char*
 *
 * @note: The validation of the parameter will occur only if the part
 * is in the same state as the one being modified.
 */
EAPI Eina_Bool edje_edit_state_external_param_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Edje_External_Param_Type type, ...);

/**
 * @brief Sets external parameter of type INT.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of
 *              type INT (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val Value will be stored here.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_int_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, int val);

/**
 * @brief Sets external parameter of type BOOL.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type BOOL (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val Value will be stored here.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_bool_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Eina_Bool val);

/**
 * @brief Sets external parameter of type DOUBLE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type DOUBLE (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val Value will be stored here.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_double_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, double val);

/**
 * @brief Sets external parameter of type STRING.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type STRING (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val Value will be stored here.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_string_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char *val);

/**
 * @brief Sets external parameter of type CHOICE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type CHOICE (not including the state value).
 * @param value The state value.
 * @param param The name of the parameter.
 * @param val Value will be stored here.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_choice_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char *val);

/**
 * @brief Sets the states step parameter values.
 *
 * Step parameter restricts resizing of each dimension to values divisibles by
 * its value. This causes the part to jump from value to value while resizing.
 * The default value is "0 0" disabling stepping.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal size
 * relative value (not including the state value).
 * @param value The state value.
 * @param step_x The horizontal step value.
 * @param step_y The vertical step value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @see edje_edit_state_step_get()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_state_step_set(Evas_Object *obj, const char *part, const char *state, double value, int step_x, int step_y);

/**
 * @brief Gets the states step values.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal size
 * relative value (not including the state value).
 * @param value The state value.
 * @param step_x The pointer to the variable where horizontal step value should be written.
 * @param step_y The pointer to the variable where vertical step value should be written.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @see edje_edit_state_step_set()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_state_step_get(Evas_Object *obj, const char *part, const char *state, double value, int *step_x, int *step_y);

/**
 * @brief Sets the states limit parameter value.
 *
 * Set limit causes the emission of signals when the the size of part changes
 * from zero or to a zero size in corresponding to the limit value.
 * For example, the signals emitted on width changing are <i>'limit,width,over'</i>
 * and <i>'limit,width,zero'</i>
 * The available values are:
 * <ul>
 * <li>NONE - 0 (the default value)</li>
 * <li>WIDTH - 1</li>
 * <li>HEIGHT - 2</li>
 * <li>BOTH - 3</li>
 * </ul>
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 * @param limit The value that represents the states limit value in case of success.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @see edje_edit_state_limit_get()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_state_limit_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char limit);

/**
 * @brief Gets the states limit value.
 *
 * Returns value that represents the states limit value:
 * <ul>
 * <li>NONE - 0 (the default value)</li>
 * <li>WIDTH - 1</li>
 * <li>HEIGHT - 2</li>
 * <li>BOTH - 3</li>
 * </ul>
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 *
 * @return The value that represents the states limit value in case of success, othervise returns 4.
 * @see edje_edit_state_limit_set()
 * @since 1.11
 */
EAPI unsigned char edje_edit_state_limit_get(Evas_Object *obj, const char *part, const char *state, double value);

//@}
/******************************************************************************/
/**************************   MAP API   ************************************/
/******************************************************************************/
/**
 * @name Map API
 * Functions to deal with objects with rotation properties (see @ref edcref).
 */ //@{

/**
 * @brief Gets the flag which enables mapping for the part.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return  @c EINA_TRUE in case if mapping allowed or @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_on_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Enables mapping for the part. Default is 0.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param on The flag which allow mapping for the part.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_on_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool on);

/**
 * @brief Gets the part's name that is used as the 'perspective point'.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state to get perspective (not including the state value).
 * @param value The state value.
 *
 * @return The name of the source part that is used as 'perspective point'.
 * @since 1.11
 */
EAPI const char * edje_edit_state_map_perspective_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the part's name that is used as the 'perspective point'.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state to get perspective (not including the state value).
 * @param value The state value.
 * @param source_part The source part's name.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_state_map_perspective_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source_part);

/**
 * @brief Gets the part's name that is used as the 'light' for calculating the brightness.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return The name of the source part that is used as 'light'.
 * @since 1.11
 **/
EAPI const char * edje_edit_state_map_light_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the part that is used as the 'light'.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param source_part The source part's name.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_light_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source_part);

/**
 * @brief Gets backface_cull value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return backface_cull value of given part state.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_backface_cull_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets backface_cull value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param backface_cull New backface_cull value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_backface_cull_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool backface_cull);

/**
 * @brief Gets perspective_on value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return perspective_on value of given part state.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_perspective_on_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets perspective_on value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param perspective_on New perspective_on value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_perspective_on_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool perspective_on);

/**
 * @brief Gets map.alpha value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return map.alpha value of given part state.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_alpha_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets map.alpha value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param alpha New map.alpha value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_alpha_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool alpha);

/**
 * @brief Gets map.smooth value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return map.smooth value of given part state.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_smooth_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets map.smooth value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param smooth New map.smooth value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_smooth_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool smooth);

/**
 * @brief Gets map.rotation of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param x x-rotation.
 * @param y x-rotation.
 * @param z z-rotation.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_rotation_get(Evas_Object *obj, const char *part, const char *state, double value, double *x, double *y, double *z);

/**
 * @brief Sets map.rotation of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param x x-rotation.
 * @param y x-rotation.
 * @param z z-rotation.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_rotation_set(Evas_Object *obj, const char *part, const char *state, double value, double x, double y, double z);

/**
 * @brief Gets map.perspective.focal value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return map.perspective.focal value of given part state.
 * @since 1.11
 **/
EAPI int edje_edit_state_map_perspective_focal_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets map.perspective.focal value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param focal New map.perspective.focal value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_perspective_focal_set(Evas_Object *obj, const char *part, const char *state, double value, int focal);

/**
 * @brief Gets map.perspective.zplane value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return map.perspective.zplane value of given part state.
 * @since 1.11
 **/
EAPI int edje_edit_state_map_perspective_zplane_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets map.perspective.zplane value of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param zplane New map.perspective.zplane value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_perspective_zplane_set(Evas_Object *obj, const char *part, const char *state, double value, int zplane);

/**
 * @brief Gets the part's name that is used as the center rotation.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 *
 * @return The name of the source part that is used as center rotation.
 * @since 1.11
 **/
EAPI const char * edje_edit_state_map_rotation_center_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets map.zoom (x and y) values of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param x value of x
 * @param y value of y
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.19
 **/
EAPI Eina_Bool
edje_edit_state_map_zoom_set(Evas_Object *obj, const char *part, const char *state, double value, double x, double y);

/**
 * @brief Gets map.zoom (x and y) values of given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param x variable to store value of x
 * @param y variable to store value of y
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.19
 **/
EAPI Eina_Bool
edje_edit_state_map_zoom_get(Evas_Object *obj, const char *part, const char *state, double value, double *x, double *y);

/**
 * @brief Sets the part that is used as the center of rotation when rotating the part with this description. If no center is given, the parts original center itself is used for the rotation center.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param source_part The source part's name.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_rotation_center_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source_part);

/**
 * @brief Sets the color for vertex/point of the current part.
 * For more detailed information please @see evas_map_point_color_set().
 *
 * In edje there is (currently) only 4 main point:
 *  - Top-Left (0), Top-Right (1), Bottom-Right (2), Bottom-Left (3).
 *
 *  Default value is 255 255 255 255 for every point.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param idx The index of point.
 * @param r The red value to set.
 * @param g The green color value to set.
 * @param b The blue color value to set.
 * @param a The alpha color value to set.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_point_color_set(Evas_Object *obj, const char *part, const char *state, double value, int idx, int r, int g, int b, int a);

/**
 * @brief Gets the color of given vertex/point of the current part.
 * For more detailed information please @see evas_map_point_color_set().
 *
 * In edje there is (currently) only 4 main point:
 *  - Top-Left (0), Top-Right (1), Bottom-Right (2), Bottom-Left (3).
 *
 *  Default value is 255 255 255 255 for every point.
 *
 * @param obj Object being edited.
 * @param part The name of the part.
 * @param state The name of the state (not including the state value).
 * @param value The state value.
 * @param idx The index of point.
 * @param r The red value to get.
 * @param g The green color value to get.
 * @param b The blue color value to get.
 * @param a The alpha color value to get.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 **/
EAPI Eina_Bool edje_edit_state_map_point_color_get(Evas_Object *obj, const char *part, const char *state, double value, int idx, int *r, int *g, int *b, int *a);

/**
 * @brief Sets the source part for given part state.
 *
 * Set another part content as the content of this part.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 * @param source_name The name of part to be set as source. If NULL is passed, the source will be unset.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @see edje_edit_state_proxy_source_get()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_state_proxy_source_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source_name);

/**
 * @brief Gets the source name for given state of part.
 *
 * @note The returned string should be freed with @c eina_stringshare_del().
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 *
 * @return The name of the source part in case of success. Otherwise returns NULL.
 * @see edje_edit_state_proxy_source_set()
 * @since 1.11
 */
EAPI Eina_Stringshare * edje_edit_state_proxy_source_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the source clip for given PROXY part state.
 *
 * The source clipper is ignored or used when rendering the proxy part.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 * @param clip Value to set if ignore or use source cliper.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_state_proxy_source_clip_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool clip);

/**
 * @brief Gets the source clip for given PROXY part state.
 *
 * The source clipper is ignored or used when rendering the proxy part.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 *
 * @return @c EINA_TRUE in case if source clipper is used, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_state_proxy_source_clip_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the source visibility for given PROXY part state.
 *
 * Defines if both the proxy and its source object will be visible or not.
 * In case of false flag, the source object will not be visible at all while
 * proxy will still show source object.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 * @param visibility Value to set if source object is visible or not.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_state_proxy_source_visible_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool visibility);

/**
 * @brief Gets the source visibility for given PROXY part state.
 *
 * Defines if both the proxy and its source object will be visible or not.
 * In case of false flag, the source object will not be visible at all while
 * proxy will still show source object.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state.
 * @param value The state value.
 *
 * @return @c EINA_TRUE in case when source object visibility is set to true, @c EINA_FALSE otherwise.
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_state_proxy_source_visible_get(Evas_Object *obj, const char *part, const char *state, double value);

//@}
/******************************************************************************/
/**************************   TEXT API   ************************************/
/******************************************************************************/
/**
 * @name Text API
 * Functions to deal with text objects (see @ref edcref).
 */ //@{

/**
 * @brief Gets the text of a part state.
 *
 * Remember to free the returned string with edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get text (not including the state value).
 * @param value The state value.
 *
 * @return A newly allocated string containing the text for the given state.
 */
EAPI const char * edje_edit_state_text_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the text of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set text (not including the state value).
 * @param value The state value.
 * @param text The new text to assign.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_set(Evas_Object *obj, const char *part, const char *state, double value,const char *text);

/**
 * @brief Gets font name for a given part state.
 *
 * Remember to free the returned string using edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param part The name of the part to get the font of.
 * @param state The state of the part to get the font of.
 * @param value Value of the state.
 *
 * @return Font used by the part or NULL if error or nothing is set.
 */
EAPI const char * edje_edit_state_font_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets font name for a given part state.
 *
 * Font name can be any alias of an internal font in the Edje file and,
 * if it doesn't match any, Edje will look for a font with the given name
 * in the system fonts.
 *
 * @param obj Object being edited.
 * @param part Part to set the font of.
 * @param state State in which the font is set.
 * @param value Value of the state.
 * @param font The font name to use.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_font_set(Evas_Object *obj, const char *part, const char *state, double value, const char *font);

/**
 * @brief Gets the text size of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get text size (not including the state value).
 * @param value The state value.
 *
 * @return The text size or @c -1 on errors.
 */
EAPI int edje_edit_state_text_size_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the text size of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set text size (not including the state value).
 * @param value The state value.
 * @param size The new font size to set (in pixel)
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_size_set(Evas_Object *obj, const char *part, const char *state, double value, int size);

/**
 * @brief Gets the text horizontal align of a part state.
 *
 * The value range is from 0.0(right) to 1.0(left)
 * If the value is between -1.0 and 0.0 then it uses align automatically.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the text horizontal align (not including the state value).
 * @param value The state value.
 *
 * @return The text horizontal align value
 */
EAPI double edje_edit_state_text_align_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets the text vertical align of a part state.
 *
 * The value range is from 0.0(top) to 1.0(bottom)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the text vertical align (not including the state value).
 * @param value The state value.
 *
 * @return The text horizontal align value
 */
EAPI double edje_edit_state_text_align_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the text horizontal align of a part state.
 *
 * The value range is from 0.0(right) to 1.0(left)
 * If the value is between -1.0 and 0.0 then it uses align automatically.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the text horizontal align (not including the state value).
 * @param value The state value.
 * @param align The new text horizontal align value
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_align_x_set(Evas_Object *obj, const char *part, const char *state, double value, double align);

/**
 * @brief Sets the text vertical align of a part state.
 *
 * The value range is from 0.0(top) to 1.0(bottom)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the text vertical align (not including the state value).
 * @param value The state value.
 * @param align The new text vertical align value
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_align_y_set(Evas_Object *obj, const char *part, const char *state, double value, double align);

/**
 * @brief Gets the text elipsis of a part state.
 *
 * The value range is from 0.0(right) to 1.0(left), and -1.0 (if disabled)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the text elipsis value (not including the state value).
 * @param value The state value.
 *
 * @return The text elipsis value
 */
EAPI double edje_edit_state_text_elipsis_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the text vertical align of a part state.
 *
 * The value range is from 0.0(right) to 1.0(left)
 * If the value is in range from -1.0 to 0.0 then ellipsis is disabled.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the text elipsis value (not including the state value).
 * @param value The state value.
 * @param balance The position where to cut the string
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_elipsis_set(Evas_Object *obj, const char *part, const char *state, double value, double balance);

/**
 * @brief Gets if the text part fit it's container horizontally.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the if the text part fit it's container horizontally (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE If the part fit it's container horizontally, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_fit_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets if the text part should fit it's container horizontally.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the if the text part fit it's container horizontally (not including the state value).
 * @param value The state value.
 * @param fit @c EINA_TRUE to make the text fit it's container horizontally, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_fit_x_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool fit);

/**
 * @brief Gets if the text part fit it's container vertically.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the if the text part fit it's container vertically (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE If the part fit it's container vertically, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_fit_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets if the text part should fit it's container vertically.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the if the text part fit it's container vertically (not including the state value).
 * @param value The state value.
 * @param fit @c EINA_TRUE to make the text fit it's container vertically, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_fit_y_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool fit);

/**
 * @brief Gets if the text part forces the minimum horizontal size of the container to be equal to the minimum horizontal size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the the minimum horizontal size of the container to be equal (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE If the part forces container's minimum horizontal size, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_min_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets if the text part forces the maximum horizontal size of the container to be equal to the maximum horizontal size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the the minimum horizontal size of the container to be equal (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE If the part forces container's maximum horizontal size, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_max_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets if the text part forces the minimum vertical size of the container to be equal to the minimum vertical size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the the minimum vertical size of the container to be equal (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE If the part forces container's minimum vertical size, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_min_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Gets if the text part forces the maximum vertical size of the container to be equal to the maximum vertical size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the the maximum vertical size of the container to be equal (not including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE If the part forces container's maximum vertical size, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_max_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets if the text part forces the minimum horizontal size of the container to be equal to the minimum horizontal size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the minimum horizontal size of the container to be equal (not including the state value).
 * @param value The state value.
 * @param v @c EINA_TRUE to make the text force it's forces container's minimum horizontal size, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_min_x_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/**
 * @brief Sets if the text part forces the maximum horizontal size of the container to be equal to the maximum horizontal size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum horizontal size of the container to be equal (not including the state value).
 * @param value The state value.
 * @param v @c EINA_TRUE to make the text force it's forces container's maximum horizontal size, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_max_x_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/**
 * @brief Sets if the text part forces the minimum vertical size of the container to be equal to the minimum vertical size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the minimum vertical size of the container to be equal (not including the state value).
 * @param value The state value.
 * @param v @c EINA_TRUE to make the text force it's forces container's minimum vertical size, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_min_y_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/**
 * @brief Sets if the text part forces the maximum vertical size of the container to be equal to the maximum vertical size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum vertical size of the container to be equal (not including the state value).
 * @param value The state value.
 * @param v @c EINA_TRUE to make the text force it's forces container's maximum vertical size, @c EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_max_y_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/**
 * @brief Gets style name for a given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part to get the style of.
 * @param state The state of the part to get the style of.
 * @param value Value of the state.
 *
 * @return Style used by the part, or NULL if error or nothing is set.
 */
EAPI const char * edje_edit_state_text_style_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets style name for a given part state.
 *
 * Causes the part to use the default style and tags defined in the "style" block with the specified name.
 *
 * @param obj Object being edited.
 * @param part Part to set the style of.
 * @param state State in which the style is set.
 * @param value Value of the state.
 * @param style The style name to use. In case when NULL style will removed from textblock part description.
 *
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_style_set(Evas_Object *obj, const char *part, const char *state, double value, const char *style);

/**
 * @brief Gets part name, which used as text source.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum vertical size of
 * the container to be equal (not including the state value).
 * @param value Value of the state.
 *
 * @return The name of part or NULL, if text_source param not a setted.
 */
EAPI const char * edje_edit_state_text_text_source_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the source text part for a given part.
 * Causes the part to display the content text of another part and update
 * them as they change.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum vertical size of
 * the container to be equal (not including the state value).
 * @param value Value of the state.
 * @param source The text source part name.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_text_source_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source);

/**
 * @brief Gets part name, which used as style text source.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum vertical size of
 * @param value Value of the state.
 * The container to be equal (not including the state value).
 *
 * @return The name of part or NULL, if text_source param not a setted.
 */
EAPI const char * edje_edit_state_text_source_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the source part which would be used as style for text for a given part.
 * Causes the part to use the text properties (like font and size) of another
 * part and update them as they change.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum vertical size of
 * the container to be equal (not including the state value).
 * @param value Value of the state.
 * @param source The text source part name.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_source_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source);

/**
 * @brief Gets the text class of the given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get text class (not including the state value).
 * @param value The state value.
 *
 * @return The current text class.
 */
EAPI const char * edje_edit_state_text_class_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the text class of the given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set text class (not including the state value).
 * @param value The state value.
 * @param text_class The text class to assign.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_class_set(Evas_Object *obj, const char *part, const char *state, double value, const char *text_class);

/**
 * @brief Gets the replacement character string of the given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get replacement character
 * (not including the state value).
 * @param value The state value.
 *
 * @return The current replacement character.
 */
EAPI const char * edje_edit_state_text_repch_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the replacement character string of the given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get replacement character
 * (not including the state value).
 * @param value The state value.
 * @param repch The replacement character string to assign.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_repch_set(Evas_Object *obj, const char *part, const char *state, double value, const char *repch);

/**
 * @brief Gets the min and max font size allowed for the text part.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state State in which the part is set.
 * @param value Value of the state.
 * @param min Minimal value of the font size in points (pt).
 * @param max Maximum value of the font size in points (pt).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_size_range_min_max_get(Evas_Object *obj, const char *part, const char *state, double value, int *min, int *max);

/**
 * @brief Sets the min and max font size allowed for the text part.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state State in which the part is set.
 * @param value Value of the state.
 * @param min Minimal value of the font size in points (pt).
 * @param max Maximum value of the font size in points (pt).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_size_range_min_max_set(Evas_Object *obj, const char *part, const char *state, double value, int min, int max);

/**
 * @brief Gets the list of all the fonts in the given edje.
 *
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 *
 * @return A list containing all the fonts names found in the edje file.
 */
EAPI Eina_List * edje_edit_fonts_list_get(Evas_Object *obj);

/**
 * @brief Adds a new font to the edje file.
 *
 * The newly created font will be available to all the groups in the edje, not only the current one.
 *
 * @param obj Object being edited.
 * @param path The file path to load the font from.
 * @param alias The alias for file, or NULL to use filename
 *
 * @return @c EINA_TRUE if font cat be loaded, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_font_add(Evas_Object *obj, const char *path, const char* alias);

/**
 * @brief Deletes font from the edje file.
 *
 * The font will be removed from all the groups in the edje, not only the current one.
 *
 * @param obj Object being edited.
 * @param alias The font alias
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.(including the
 * case when the alias is not valid).
 */
EAPI Eina_Bool edje_edit_font_del(Evas_Object *obj, const char* alias);

/**
 * @brief Gets font path for a given font alias.
 *
 * Remember to free the string with edje_edit_string_free()
 *
 * @param obj Object being edited.
 * @param alias The font alias.
 *
 * @return The path of the given font alias.
 */
EAPI const char *edje_edit_font_path_get(Evas_Object *obj, const char *alias);

//@}
/******************************************************************************/
/************************   IMAGE SET API   ***********************************/
/******************************************************************************/
/**
 * @name Image Set API
 * Functions to deal with image objects (see @ref edcref).
 */ //@{

/**
 * @brief Checks if given image name is set of images or not.
 *
 * @param obj Object being edited.
 * @param image a name to check if it is set or not.
 *
 * @return @c EINA_TRUE in case when given name is set, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_exists(Evas_Object *obj, const char *image);

/**
 * @brief Gets id of image set.
 *
 * @param obj Object being edited.
 * @param name image set's name.
 *
 * @return The id of the given image name.
 *
 * @since 1.18
 */
EAPI int
edje_edit_image_set_id_get(Evas_Object *obj, const char *name);

/**
 * @brief Renames image set.
 *
 * @param obj Object being edited.
 * @param set image set's name.
 * @param new_set new name of image set.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_rename(Evas_Object *obj, const char *set, const char *new_set);

/**
 * @brief Gets the list of all the image sets in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 *
 * @return A List containing all image sets names found in the edje file.
 *
 * @since 1.18
 */
EAPI Eina_List *
edje_edit_image_set_list_get(Evas_Object *obj);

/**
 * @brief Gets list of (Edje_Part_Image_Use *) - group-part-state triplets where given
 * set is used
 *
 * Use edje_edit_image_usage_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param name The name of the image.
 * @param first_only If @c EINA_TRUE, return only one triplete.
 *
 * @return Eina_List containing Edje_Part_Image_Use if successful, NULL otherwise
 */
EAPI Eina_List*
edje_edit_set_usage_list_get(Evas_Object *obj, const char *name, Eina_Bool first_only);

/**
 * @brief Adds new image set.
 *
 * @param obj Object being edited.
 * @param name image set's name.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_add(Evas_Object *obj, const char *name);

/**
 * @brief Deletes image set.
 *
 * Can't delete set if it is used by any part.
 *
 * @param obj Object being edited.
 * @param name image set's name.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_del(Evas_Object *obj, const char *name);

/**
 * @brief Gets the list of all images inside of given set in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 * @param name name of image set.
 *
 * @return A List containing all images found inside of given set in the edje file.
 *
 * @since 1.18
 */
EAPI Eina_List *
edje_edit_image_set_images_list_get(Evas_Object *obj, const char *name);

/**
 * @brief Adds image to set.
 *
 * Add image to given set. If image is not exist inside of edje
 * collection then function @see edje_edit_image_add should be
 * used to get image added to edje collection.
 * This function uses only already added functions
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param name image set's name.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise (and when image is not exist).
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_add(Evas_Object *obj, const char *set_name, const char *name);

/**
 * @brief Deletes image on current position from set.
 *
 * Remove image from given set. To be sure what kind of image will be
 * deleted, firstly check it's position via
 * @see edje_edit_image_set_images_list_get function.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image to be deleted.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_del(Evas_Object *obj, const char *set_name, unsigned int place);

/**
 * @brief Gets min size of set's image.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image.
 * @param w Where to store the width min value.
 * @param h Where to store the height min value.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_min_get(Evas_Object *obj, const char *set_name, unsigned int place, int *w, int *h);

/**
 * @brief Sets min size of set's image.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image.
 * @param w New value of picture's min width.
 * @param h New value of picture's min height.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_min_set(Evas_Object *obj, const char *set_name, unsigned int place, int w, int h);

/**
 * @brief Gets max size of set's image.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image.
 * @param w Where to store the width max value.
 * @param h Where to store the height max value.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_max_get(Evas_Object *obj, const char *set_name, unsigned int place, int *w, int *h);

/**
 * @brief Sets max size of set's image.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image.
 * @param w New value of picture's max width.
 * @param h New value of picture's max height.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_max_set(Evas_Object *obj, const char *set_name, unsigned int place, int w, int h);

/**
 * @brief Gets border of set's image.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image.
 * @param l Where to store the left border value.
 * @param r Where to store the right border value.
 * @param b Where to store the bottom border value.
 * @param t Where to store the top border value.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_border_get(Evas_Object *obj, const char *set_name, unsigned int place, int *l, int *r, int *b, int *t);

/**
 * @brief Sets border of set's image.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image.
 * @param l New value of left border value.
 * @param r New value of right border value.
 * @param b New value of bottom border value.
 * @param t New value of top border value.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_border_set(Evas_Object *obj, const char *set_name, unsigned int place, int l, int r, int b, int t);

/**
 * @brief Gets border scale of set's image.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image.
 *
 * @return @c border scale value on success, @c -1 otherwise.
 *
 * @since 1.18
 */
EAPI double
edje_edit_image_set_image_border_scale_get(Evas_Object *obj, const char *set_name, unsigned int place);

/**
 * @brief Sets border scale of set's image.
 *
 * @param obj Object being edited.
 * @param set_name name of image set.
 * @param place position of image.
 * @param scale_by New border scale.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_image_set_image_border_scale_set(Evas_Object *obj, const char *set_name, unsigned int place, double scale_by);

//@}
/******************************************************************************/
/**************************   IMAGES API   ************************************/
/******************************************************************************/
/**
 * @name Images API
 * Functions to deal with image objects (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the images in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 *
 * @return A List containing all images names found in the edje file.
 */
EAPI Eina_List * edje_edit_images_list_get(Evas_Object *obj);

/**
 * @brief Adds an new image to the image collection
 *
 * This function add the given image inside the edje. Don't add a new image part
 * but only put the image inside the edje file. It actually write directly to
 * the file so you don't have to save.
 * After you have to create a new image_part that use this image. Note that all
 * the parts in the edje share the same image collection, thus you can/must use
 * the same image for different part.
 *
 * The format of the image files that can be loaded depend on the evas engine on your system
 *
 * @param obj Object being edited.
 * @param path The name of the image file to include in the edje.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_image_add(Evas_Object *obj, const char *path);

/**
 * @brief Deletes an image from the image collection
 *
 * It actually write directly to the file so you don't have to save.
 * Can't delete image if it is used by any part.
 *
 * @param obj Object being edited.
 * @param name The name of the image file to include in the edje.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.(including the
 * case when the name is not valid or image is in use).
 */
EAPI Eina_Bool edje_edit_image_del(Evas_Object *obj, const char *name);

/**
 * @brief Replaces one image in all descriptions.
 *
 * @param obj Object being edited.
 * @param name The name of the image to replace.
 * @param new_name The new_name of the image to replace with.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.(including the
 * case when one of the names is not valid)
 */
EAPI Eina_Bool edje_edit_image_replace(Evas_Object *obj, const char *name, const char *new_name);

/**
 * @brief Renames image.
 *
 * @param obj Object being edited.
 * @param name The name of the image to be renamed.
 * @param new_name The new_name of the image.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.(including the
 * case when one of the names is not valid)
 *
 * @since 1.13
 */
EAPI Eina_Bool edje_edit_image_rename(Evas_Object *obj, const char *name, const char *new_name);

/**
 * @brief Gets list of (Edje_Part_Image_Use *) - group-part-state triplets where given
 * image is used.
 *
 * Important! Image can also be used inside of set and plenty of times, so for each use
 * inside of set triplet would set "set's" name into group name, and it's state
 * value would be -1. Every other fields will be 0.
 *
 * Use edje_edit_image_usage_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param name The name of the image.
 * @param first_only If @c EINA_TRUE, return only one triplet.
 *
 * @return Eina_List containing Edje_Part_Image_Use if successful, NULL otherwise
 */
EAPI Eina_List* edje_edit_image_usage_list_get(Evas_Object *obj, const char *name, Eina_Bool first_only);

/**
 * @brief Frees an Eina_List of (Edje_Part_Image_Use *) allocated by an edje_edit_image_usage_list_get() or
 * an edje_edit_vector_usage_list_get() function.
 *
 * @param lst List of strings to free.
 */
EAPI void edje_edit_image_usage_list_free(Eina_List *lst);

/**
 * @brief Adds an image entry to the image collection.
 *
 * This function adds the given image entry to the edje image collection. The
 * image needs to be inside the eet already, with key name "images/id". After
 * you have to create a new image_part that use this image, referring to it as
 * "name". Note that all the parts in the edje share the same image collection,
 * thus you can/must use the same image for different part.
 *
 * @param obj Object being edited.
 * @param name The image entry name.
 * @param id The image id.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_image_data_add(Evas_Object *obj, const char *name, int id);

/**
 * @brief Gets normal image name for a given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the name that is being used (not including the state value).
 * @param value The state value.
 *
 * @return The name of the image used by state.
 */
EAPI const char * edje_edit_state_image_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets normal image for a given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the image that will be used (not including the state value).
 * @param value The state value.
 * @param image The name of the image (must be an image contained in the edje file).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_image_set(Evas_Object *obj, const char *part, const char *state, double value, const char *image);

/**
 * @brief Gets normal vector name for a given part state.
 * Part should be of type VECTOR
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the name that is being used (not including the state value).
 * @param value The state value.
 *
 * @return The name of the vector used by state.
 * @since 1.19
 */
EAPI const char * edje_edit_state_vector_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets normal vector name for a given part state.
 * Part should be of type VECTOR
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the vector that will be used (not including the state value).
 * @param value The state value.
 * @param image The name of the vector (must be contained in the edje file).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.19
 */
EAPI Eina_Bool edje_edit_state_vector_set(Evas_Object *obj, const char *part, const char *state, double value, const char *image);

/**
 * @brief Gets image id for a given image name.
 *
 * @param obj Object being edited.
 * @param image_name The image name.
 *
 * @return The id of the given image name.
 */
EAPI int edje_edit_image_id_get(Evas_Object *obj, const char *image_name);

/**
 * @brief Gets compression type for the given image.
 *
 * @param obj Object being edited.
 * @param image The name of the image.
 *
 * @return One of Image Compression types.
 * (EDJE_EDIT_IMAGE_COMP_RAW, EDJE_EDIT_IMAGE_COMP_USER, EDJE_EDIT_IMAGE_COMP_COMP, EDJE_EDIT_IMAGE_COMP_LOSSY[_ETC1]).
 */
EAPI Edje_Edit_Image_Comp edje_edit_image_compression_type_get(Evas_Object *obj, const char *image);

/**
 * @brief Sets compression type for the given image.
 *
 * @param obj Object being edited.
 * @param image The name of the image.
 * @param ic Edje_Edit_Image_Comp.
 * (EDJE_EDIT_IMAGE_COMP_RAW, EDJE_EDIT_IMAGE_COMP_USER, EDJE_EDIT_IMAGE_COMP_COMP, EDJE_EDIT_IMAGE_COMP_LOSSY[_ETC1]).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_image_compression_type_set(Evas_Object *obj, const char *image, Edje_Edit_Image_Comp ic);

/**
 * @brief Gets compression rate for the given image.
 *
 * @param obj Object being edited.
 * @param image The name of the image.
 *
 * @return The compression rate (if the imnage is @c
 *         EDJE_EDIT_IMAGE_COMP_LOSSY[_ETC1]) or < 0, on errors.
 */
EAPI int edje_edit_image_compression_rate_get(Evas_Object *obj, const char *image);

/**
 * @brief Gets the image border of a part state.
 *
 * Pass NULL to any of [r,g,b,a] to get only the others.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the image border (not
 *              including the state value).
 * @param value The state value.
 * @param l A pointer to store the left value
 * @param r A pointer to store the right value
 * @param t A pointer to store the top value
 * @param b A pointer to store the bottom value
 */
EAPI void edje_edit_state_image_border_get(Evas_Object *obj, const char *part, const char *state, double value, int *l, int *r, int *t, int *b);

/**
 * @brief Sets the image border of a part state.
 *
 * Pass -1 to any of [l,r,t,b] to leave the value untouched.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the image border (not
 *              including the state value).
 * @param value The state value.
 * @param l Left border value (or -1).
 * @param r Right border value (or -1).
 * @param t Top border value (or -1).
 * @param b Bottom border value (or -1).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_image_border_set(Evas_Object *obj, const char *part, const char *state, double value, int l, int r, int t, int b);

/**
 * @brief Gets the border scale value of a part state.
 *
 * This value tells Edje if the border should be scaled by
 * the object/global edje scale factors
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the image border scale (not
 *              including the state value).
 * @param value The state value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_state_image_border_scale_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the border scale value of a part state.
 *
 * This value tells Edje if the border should be scaled by
 * the object/global edje scale factors
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the image border scale (not
 *              including the state value).
 * @param value The state value.
 * @param scale New image border scale value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_state_image_border_scale_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool scale);

/**
 * @brief Gets the border scale by value of a part state.
 *
 * Valid values are: 0.0 or bigger (0.0 or 1.0 to turn it off)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the image border scale by (not
 *              including the state value).
 * @param value The state value.
 *
 * @return border scaling value.
 *
 * @since 1.18
 */
EAPI double
edje_edit_state_image_border_scale_by_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets the border scale by value of a part state.
 *
 * Valid values are: 0.0 or bigger (0.0 or 1.0 to turn it off)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the image border scale by (not
 *              including the state value).
 * @param value The state value.
 * @param scale New image border scale value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_state_image_border_scale_by_set(Evas_Object *obj, const char *part, const char *state, double value, double scale);

/**
 * @brief Gets if the image center should be draw.
 *
 * 1 or 2 means to draw the center, 0 to don't draw it.
 * If 1 - then the center will apply alpha channel.
 * If 2 (SOLID mode) - then the center of an image wont have alpha channel (Just black color).
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the image border fill (not including the state value).
 * @param value The state value.
 *
 * @return @c 2 if the center of the bordered image is draw without alpha, @c 1 drawing with alpha and @c 0 not drawing the center.
 */
EAPI unsigned char edje_edit_state_image_border_fill_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Sets if the image center should be draw.
 *
 * 1 or 2 means to draw the center, 0 to don't draw it.
 * If 1 - then the center will apply alpha channel.
 * If 2 (SOLID mode) - then the center of an image wont have alpha channel (Just black color).
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the image border fill (not including the state value).
 * @param value The state value.
 * @param fill Fill to be set. 1 or 2 if the center of the bordered image is draw, 0 otherwise.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_image_border_fill_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char fill);

/**
 * @brief Gets the list of all the tweens images in the given part state.
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the list of all the tweens images (not including the state value).
 * @param value The state value.
 *
 * @return A string list containing all the image name that form a tween animation in the given part state.
 */
EAPI Eina_List * edje_edit_state_tweens_list_get(Evas_Object *obj, const char *part, const char *state, double value);

/**
 * @brief Adds a new tween frame to the given part state.
 *
 * The tween param must be the name of an existing image.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to add a new tween frame (not including the state value).
 * @param value The state value.
 * @param tween The name of the image to add.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_tween_add(Evas_Object *obj, const char *part, const char *state, double value, const char *tween);

/**
 * @brief Inserts a new tween frame to the given part state into a specific place.
 *
 * The tween param must be the name of an existing image.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to add a new tween frame (not including the state value).
 * @param value The state value.
 * @param tween The name of the image to add.
 * @param place Place to be added. It can't be less than 0 or more than current size of tweens.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
edje_edit_state_tween_insert_at(Evas_Object *obj, const char *part, const char *state, double value, const char *tween, int place);

/**
 * @brief Removes the first tween with the given name.
 *
 * The image is not removed from the edje.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to delete the tween (not including the state value).
 * @param value The state value.
 * @param tween The name of the image to delete.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_tween_del(Evas_Object *obj, const char *part, const char *state, double value, const char *tween);

//@}
/******************************************************************************/
/**************************   VECTORS API   ************************************/
/******************************************************************************/

/**
 * @brief Gets vector id for a given vector name.
 *
 * @param obj Object being edited.
 * @param vector_name The vector name.
 *
 * @return The id of the given vector name.
 * @since 1.19
 */
EAPI int edje_edit_vector_id_get(Evas_Object *obj, const char *vector_name);

/**
 * @name Vectors API
 * Functions to deal with vector objects of images (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the vectors in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 *
 * @return A List containing all vector names found in the edje file.
 * @since 1.19
 */
EAPI Eina_List * edje_edit_vectors_list_get(Evas_Object *obj);

/**
 * @brief Deletes vector from the vector collection
 *
 * It actually write directly to the file so you don't have to save.
 * Can't delete vector if it is used by any part.
 *
 * @param obj Object being edited.
 * @param name The name of the vector file.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise (including the
 * case when the name is not valid or vector is in use).
 * @since 1.19
 */
EAPI Eina_Bool edje_edit_vector_del(Evas_Object *obj, const char *name);

/**
 * @brief Gets list of (Edje_Part_Image_Use *) - group-part-state triplets where given
 * vector is used.
 *
 * Use edje_edit_image_usage_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param name The name of the vector.
 * @param first_only If EINA_TRUE, return only one triplete.
 *
 * @return Eina_List containing Edje_Part_Image_Use if successful, NULL otherwise
 * @since 1.19
 */
EAPI Eina_List* edje_edit_vector_usage_list_get(Evas_Object *obj, const char *name, Eina_Bool first_only);

//@}
/******************************************************************************/
/**************************   SOUNDS API   ************************************/
/******************************************************************************/
/**
 * @name Sounds API
 * Functions to deal with sound objects (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the sounds samples in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 *
 * @return A List containing all sounds samples names found in the edje file.
 * @since 1.11
 */
EAPI Eina_List * edje_edit_sound_samples_list_get(Evas_Object *obj);

/**
 * @brief Gets the list of all the sounds tones in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 *
 * @return A List containing all sounds tones names found in the edje file.
 * @since 1.11
 */
EAPI Eina_List * edje_edit_sound_tones_list_get(Evas_Object *obj);

/**
 * @brief Adds new sound sample to samples collection.
 *
 * This function adds the given sound file to the edje collection.
 * The added sound sample could be used by PLAY_SAMPLE action in any program
 * of any group that is in the current collection.
 * The quality of added sound by default is uncompressed (RAW).
 *
 * The available formats list of the sound files that can be loaded depends
 * on the evas engine on your system.
 *
 * @param obj Object being edited.
 * @param name The name that will identify sample.
 * @param snd_src The name of the sound file to add.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @see edje_edit_sound_sample_del()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_sound_sample_add(Evas_Object *obj, const char* name, const char* snd_src);

/**
 * @brief Deletes sound sample from the collection.
 *
 * Deletes sound sample from collection by its name. After successful deletion
 * all PLAY_SAMPLE actions in all programs of all groups of current collection
 * that use deleted sound will be deleted.
 *
 * @param obj Object being edited.
 * @param name The name of the sound to be deleted from the edje.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @see edje_edit_sound_sample_add()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_sound_sample_del(Evas_Object *obj, const char *name);

/**
 * @brief Adds new tone to the collection.
 *
 * This function adds new tone with given frequency to the edje collection.
 * The added sound sample could be used by PLAY_TONE action in any program
 * of any group that is in the current collection.
 *
 * @param obj Object being edited.
 * @param name The name that will identify tone.
 * @param frequency Frequency of added tone. This value should be in range of 20 to 20000 inclusive.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @see edje_edit_sound_tone_del()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_sound_tone_add(Evas_Object *obj, const char* name, int frequency);

/**
 * @brief Deletes tone from the collection.
 *
 * Deletes tone from collection by its name. After successful deletion
 * all PLAY_TONE actions in all programs of all groups of current collection
 * that use deleted sound will be deleted.
 *
 * @param obj Object being edited.
 * @param name The name of the tone to be deleted from the edje.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise.
 * @see edje_edit_sound_tone_add()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_sound_tone_del(Evas_Object *obj, const char* name);

/**
 * @brief Gets the sound quality compression.
 *
 * @param obj Object being edited.
 * @param sound The name of the sample.
 *
 * @return Quality of the compression of the sample sound.
 * @since 1.11
 */
EAPI double edje_edit_sound_compression_rate_get(Evas_Object *obj, const char* sound);

/**
 * @brief Sets the sound quality compression.
 *
 * @param obj Object being edited.
 * @param sound The name of the sample.
 * @param rate Quality of the compression.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_sound_compression_rate_set(Evas_Object *obj, const char* sound, double rate);

/**
 * @brief Sets the frequency of tone.
 *
 * @param obj Object being edited.
 * @param name The name of the tone.
 * @param frequency The value of frequency of tone. This value has to be in range of 20 to 20000 inclusive.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @see edje_edit_sound_tone_frequency_get()
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_sound_tone_frequency_set(Evas_Object *obj, const char *name, int frequency);

/**
 * @brief Gets the frequency of tone.
 *
 * @param obj Object being edited.
 * @param name The name of the tone.
 *
 * @return The frequency of tone if successful, otherwise returns -1.
 * @see edje_edit_sound_tone_frequency_set()
 * @since 1.11
 */
EAPI int edje_edit_sound_tone_frequency_get(Evas_Object *obj, const char *name);

/**
 * @brief Gets the sound type compression.
 *
 * @param obj Object being edited.
 * @param name The name of the sample.
 *
 * @return Compression type of the sample sound.
 * @since 1.11
 */
EAPI Edje_Edit_Sound_Comp edje_edit_sound_compression_type_get(Evas_Object *obj, const char* name);

/**
 * @brief Sets the sound type compression.
 *
 * @param obj Object being edited.
 * @param name The name of the sample.
 * @param sc Edje_Edit_Sound_Comp
 * (@c EDJE_EDIT_SOUND_COMP_RAW, @c EDJE_EDIT_SOUND_COMP_COMP, @c EDJE_EDIT_SOUND_COMP_LOSSY, @c EDJE_EDIT_SOUND_COMP_AS_IS).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 * @since 1.11
 */
EAPI Eina_Bool edje_edit_sound_compression_type_set(Evas_Object *obj, const char* name, Edje_Edit_Sound_Comp sc);

/**
 * @brief Gets the certain sound data from the edje object.
 *
 * @param obj Object being edited.
 * @param sample_name The name of the sound.
 *
 * @return buf The buffer that contains data of the sound. To free the resources use eina_binbuf_free().
 * @since 1.11
 */
EAPI Eina_Binbuf *edje_edit_sound_samplebuffer_get(Evas_Object *obj, const char *sample_name);

/**
 * @brief Gets the name of sample source.
 *
 * @param obj Object being edited.
 * @param sample_name The name of the sample.
 *
 * @return snd_src The sample source name.
 * @since 1.11
 */
EAPI const char *edje_edit_sound_samplesource_get(Evas_Object *obj, const char *sample_name);

//@}
/******************************************************************************/
/*************************   SPECTRUM API   ***********************************/
/******************************************************************************/
/**
 * @name Spectrum API
 * Functions to manage spectrum (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the spectrum in the given edje object.
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 *
 * @return A list containing all the spectra names.
 */
EAPI Eina_List * edje_edit_spectrum_list_get(Evas_Object *obj);

/**
 * @brief Adds a new spectra in the given edje object.
 *
 * @param obj Object being edited.
 * @param name The name of the spectra to include in the edje.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_add(Evas_Object *obj, const char *name);

/**
 * @brief Deletes the given spectra from the edje object.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra to delete.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_del(Evas_Object *obj, const char *spectra);

/**
 * @brief Changes the name of the given spectra.
 *
 * @param obj Object being edited.
 * @param spectra The name of the current spectra.
 * @param name The new name to assign.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_name_set(Evas_Object *obj, const char *spectra, const char *name);

/**
 * @brief Gets the number of stops in the given spectra.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra.
 *
 * @return The number of stops (or 0 on errors).
 */
EAPI int edje_edit_spectra_stop_num_get(Evas_Object *obj, const char *spectra);

/**
 * @brief Sets the number of stops in the given spectra.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra.
 * @param num The number of stops you want
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_stop_num_set(Evas_Object *obj, const char *spectra, int num);

/**
 * @brief Gets the colors of the given stop.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra.
 * @param stop_number The number of the stop,
 * @param r Where to store the red color value,
 * @param g Where to store the green color value,
 * @param b Where to store the blue color value,
 * @param a Where to store the alpha color value,
 * @param d Where to store the delta stop value,
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_stop_color_get(Evas_Object *obj, const char *spectra, int stop_number, int *r, int *g, int *b, int *a, int *d);

/**
 * @brief Sets the colors of the given stop.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra.
 * @param stop_number The number of the stops,
 * @param r The red color value to set,
 * @param g The green color value to set,
 * @param b The blue color value to set,
 * @param a The alpha color value to set,
 * @param d The delta stop value to set,
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_stop_color_set(Evas_Object *obj, const char *spectra, int stop_number, int r, int g, int b, int a, int d);

//@}
/******************************************************************************/
/*************************   PROGRAMS API   ***********************************/
/******************************************************************************/
/**
 * @name Programs API
 * Functions to deal with programs (see @ref edcref).
 */ //@{

/**
 * @brief Gets the list of all the programs in the given edje object.
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 *
 * @return A list containing all the program names.
 */
EAPI Eina_List * edje_edit_programs_list_get(Evas_Object *obj);

/**
 * @brief Adds a new program to the edje file
 *
 * If a program with the same name just exist the function will fail.
 *
 * @param obj Object being edited.
 * @param name The name of the new program.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_add(Evas_Object *obj, const char *name);

/**
 * @brief Removes the given program from the edje file.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to remove.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_del(Evas_Object *obj, const char *prog);

/**
 * @brief Checks if a program with the given name exist in the edje object.
 *
 * @param obj Object being edited.
 * @param prog The prog of the program that will be searched.
 *
 * @return @c EINA_TRUE if the program exist, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_exist(Evas_Object *obj, const char *prog);

/**
 * @brief Runs the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to execute.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_run(Evas_Object *obj, const char *prog);

/**
 * @brief Stops all running programs.
 *
 * @param obj Object being edited.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_stop_all(Evas_Object *obj);

/**
 * @brief Sets parts into intermediate state of programs transition.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to use. Program should have action STATE_SET.
 * @param pos State of transition to be setted. Value from 0.0 to 1.0.
 * 0.0 represents the start state, 1.0 - the final state. Other values will set
 * parts to an intermediate state taking into account programs transition type.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_state_set(Evas_Object *obj, const char *prog, double pos);

/**
 * @brief Sets a new name for the given program.
 *
 * @param obj Object being edited.
 * @param prog The current program name.
 * @param new_name The new name to assign.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_name_set(Evas_Object *obj, const char *prog, const char *new_name);

/**
 * @brief Gets source of a given program.
 *
 * Remember to free the returned string using edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get source.
 *
 * @return The source value per program.
 */
EAPI const char * edje_edit_program_source_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets source of the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set source.
 * @param source The new source value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_source_set(Evas_Object *obj, const char *prog, const char *source);

/**
 * @brief Gets signal of a given program.
 *
 * Remember to free the returned string using edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the signal.
 *
 * @return The signal value for program.
 */
EAPI const char * edje_edit_program_signal_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets signal of the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the signal.
 * @param signal The new signal value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_signal_set(Evas_Object *obj, const char *prog, const char *signal);

/**
 * @brief Gets in.from of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the delay.
 *
 * @return The delay.
 */
EAPI double edje_edit_program_in_from_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets in.from of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the delay.
 * @param seconds Number of seconds to delay the program execution
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_in_from_set(Evas_Object *obj, const char *prog, double seconds);

/**
 * @brief Gets in.range of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get random delay.
 *
 * @return The delay random.
 */
EAPI double edje_edit_program_in_range_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets in.range of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set random delay.
 * @param seconds Max random number of seconds to delay.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_in_range_set(Evas_Object *obj, const char *prog, double seconds);

/**
 * @brief Gets the action of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the action.
 *
 * @return The action type, or @c -1 on errors.
 * Action can be one of EDJE_ACTION_TYPE_NONE, _STATE_SET, ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, _DRAG_VAL_STEP, _DRAG_VAL_PAGE, _SCRIPT
 */
EAPI Edje_Action_Type edje_edit_program_action_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the action of a given program.
 *
 * Action can be one of EDJE_ACTION_TYPE_NONE, _STATE_SET, ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, _DRAG_VAL_STEP, _DRAG_VAL_PAGE, _SCRIPT
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the action.
 * @param action The new action type.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_action_set(Evas_Object *obj, const char *prog, Edje_Action_Type action);

/**
 * @brief Gets the list of the targets for the given program.
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the list of the targets.
 *
 * @return A list with all the targets names, or NULL on error.
 */
EAPI Eina_List * edje_edit_program_targets_get(Evas_Object *obj, const char *prog);

/**
 * @brief Adds a new target program to the list of 'targets' in the given program.
 *
 * If program action is @c EDJE_ACTION_TYPE_ACTION_STOP, then 'target'
 * must be an existing program name. If it's @c
 * EDJE_ACTION_TYPE_STATE_SET, then 'target' must be an existing part
 * name.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to add a new target.
 * @param target The name of the new target itself.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_target_add(Evas_Object *obj, const char *prog, const char *target);

/**
 * @brief Adds a new target program to certain place in list of 'targets' in the given program.
 *
 * If program action is @c EDJE_ACTION_TYPE_ACTION_STOP, then 'target'
 * must be an existing program name. If it's @c
 * EDJE_ACTION_TYPE_STATE_SET, then 'target' must be an existing part
 * name.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to add a new target.
 * @param target The name of the new target itself.
 * @param place Specific play for target to be inserted into.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_program_target_insert_at(Evas_Object *obj, const char *prog, const char *target, int place);

/**
 * @brief Deletes a target from the list of 'targets' in the given program.
 *
 * If program action is EDJE_ACTION_TYPE_ACTION_STOP then 'target' must be an existing program name.
 * If action is EDJE_ACTION_TYPE_STATE_SET then 'target' must be an existing part name.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to del a target from the list of targets.
 * @param target The name of another program or another part.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_target_del(Evas_Object *obj, const char *prog, const char *target);

/**
 * @brief Clears the 'targets' list of the given program
 *
 * @param obj Object being edited.
 * @param prog The name of the program to clear the 'targets' list.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_targets_clear(Evas_Object *obj, const char *prog);

/**
 * @brief Gets the list of action that will be run after the give program.
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the list of actions
 *
 * @return A list with all program names, or NULL on error.
 */
EAPI Eina_List * edje_edit_program_afters_get(Evas_Object *obj, const char *prog);

/**
 * @brief Adds a new program name to the list of 'afters' in the given program.
 *
 * All the programs listed in 'afters' will be executed after program execution.
 *
 * @param obj Object being edited.
 * @param prog The name of the program that contains the list of afters
 * @param after The name of another program to add to the afters list
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_after_add(Evas_Object *obj, const char *prog, const char *after);

/**
 * @brief Adds a new program name into specific place in list of 'afters' in the given program.
 *
 * All the programs listed in 'afters' will be executed after program execution.
 *
 * @param obj Object being edited.
 * @param prog The name of the program that contains the list of afters
 * @param after The name of another program to add to the afters list
 * @param place Specific place for after to be inserted into. Note that if place is greater than total number of afters then it would append to the end of list
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 *
 * @since 1.18
 */
EAPI Eina_Bool
edje_edit_program_after_insert_at(Evas_Object *obj, const char *prog, const char *after, int place);

/**
 * @brief Deletes the given program from the list of 'afters' of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program from where to remove the after.
 * @param after The name of the program to remove from the list of afters.
 *
 * @return @c EINA_TRUE is successful or not in the list, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_after_del(Evas_Object *obj, const char *prog, const char *after);

/**
 * @brief Clears the 'afters' list of the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to clear the 'afters' list.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_afters_clear(Evas_Object *obj, const char *prog);

/**
 * @brief Get the state for the given program.
 *
 * In a STATE_SET action this is the name of state to set.
 * In a SIGNAL_EMIT action is the name of the signal to emit.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the state.
 *
 * @return The name of the state.
 */
EAPI const char * edje_edit_program_state_get(Evas_Object *obj, const char *prog);

/**
 * @brief Gets api's name of a program.
 *
 * @param obj Object being edited.
 * @param prog Name of program.
 *
 * @return Name of the api if successful, NULL otherwise.
 */
EAPI const char * edje_edit_program_api_name_get(Evas_Object *obj, const char *prog);

/**
 * @brief Gets api's description of a program.
 *
 * @param obj Object being edited.
 * @param prog Name of program.
 *
 * @return Description of the api if successful, NULL otherwise.
 */
EAPI const char * edje_edit_program_api_description_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets api's name of a program.
 *
 * @param obj Object being edited.
 * @param prog Name of the part.
 * @param name New name for the api property.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_api_name_set(Evas_Object *obj, const char *prog, const char *name);

/**
 * @brief Sets api's description of a program.
 *
 * @param obj Object being edited.
 * @param prog Name of the program.
 * @param description New description for the api property.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_api_description_set(Evas_Object *obj, const char *prog, const char *description);

/**
 * @brief Sets the state for the given program.
 *
 * In a STATE_SET action this is the name of state to set.
 * In a SIGNAL_EMIT action is the name of the signal to emit.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set a state.
 * @param state The name of the state to set (not including the state value)
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_state_set(Evas_Object *obj, const char *prog, const char *state);

/**
 * @brief Gets the value of state for the given program.
 *
 * In a STATE_SET action this is the value of state to set.
 * Not used on SIGNAL_EMIT action.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the value of state.
 *
 * @return The value of state for the program.
 */
EAPI double edje_edit_program_value_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the value of state for the given program.
 *
 * In a STATE_SET action this is the value of state to set.
 * Not used on SIGNAL_EMIT action.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the value of state.
 * @param value The vale to set.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_value_set(Evas_Object *obj, const char *prog, double value);

/**
 * @brief Gets the state2 for the given program
 *
 * In a STATE_SET action is not used
 * In a SIGNAL_EMIT action is the source of the emitted signal.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the state2.
 *
 * @return The source to emit for the program.
 */
EAPI const char * edje_edit_program_state2_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the state2 for the given program
 *
 * In a STATE_SET action is not used
 * In a SIGNAL_EMIT action is the source of the emitted signal.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the state2.
 * @param state2 The name of the state to set.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_state2_set(Evas_Object *obj, const char *prog, const char *state2);

/**
 * @brief Gets the value of state2 for the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the state2 value.
 *
 * @return The vale of the state2 for the program.
 */
EAPI double edje_edit_program_value2_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the value2 of state for the given program.
 *
 * This is used in DRAG_ACTION
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the state2 value.
 * @param value The value of the state2 to set.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_value2_set(Evas_Object *obj, const char *prog, double value);

/**
 * @brief Gets the type of transition to use when apply animations.
 *
 * Can be one of: EDJE_TWEEN_MODE_NONE, EDJE_TWEEN_MODE_LINEAR, EDJE_TWEEN_MODE_SINUSOIDAL, EDJE_TWEEN_MODE_ACCELERATE or EDJE_TWEEN_MODE_DECELERATE.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the transition.
 *
 * @return The type of transition used by program.
 */
EAPI Edje_Tween_Mode edje_edit_program_transition_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the type of transition to use when apply animations.
 *
 * Can be one of: EDJE_TWEEN_MODE_NONE, EDJE_TWEEN_MODE_LINEAR, EDJE_TWEEN_MODE_SINUSOIDAL, EDJE_TWEEN_MODE_ACCELERATE or EDJE_TWEEN_MODE_DECELERATE.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the transition.
 * @param transition The transition type to set
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_set(Evas_Object *obj, const char *prog, Edje_Tween_Mode transition);

/**
 * @brief Gets the interpolation value 1 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_ACCELERATE_FACTOR, EDJE_TWEEN_MODE_DECELERATE_FACTOR, EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR, EDJE_TWEEN_MODE_DIVISOR_INTERP, EDJE_TWEEN_MODE_BOUNCE or EDJE_TWEEN_MODE_SPRING.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 1.
 *
 * @return Interpolation value 1.
 */
EAPI double edje_edit_program_transition_value1_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the interpolation value 1 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_ACCELERATE_FACTOR, EDJE_TWEEN_MODE_DECELERATE_FACTOR, EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR, EDJE_TWEEN_MODE_DIVISOR_INTERP, EDJE_TWEEN_MODE_BOUNCE or EDJE_TWEEN_MODE_SPRING.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 1.
 * @param value The interpolation value 1 for the transition.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_value1_set(Evas_Object *obj, const char *prog, double value);

/**
 * @brief Gets the interpolation value 2 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_DIVISOR_INTERP, EDJE_TWEEN_MODE_BOUNCE or EDJE_TWEEN_MODE_SPRING.
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 2.
 *
 * @return Interpolation value 2.
 */
EAPI double edje_edit_program_transition_value2_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the interpolation value 2 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_DIVISOR_INTERP, EDJE_TWEEN_MODE_BOUNCE or EDJE_TWEEN_MODE_SPRING.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 2.
 * @param value The interpolation value 2 for the transition.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_value2_set(Evas_Object *obj, const char *prog, double value);

/**
 * @brief Gets the interpolation value 3 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_CUBIC_BEZIER.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 3.
 *
 * @return Interpolation value 3.
 */
EAPI double edje_edit_program_transition_value3_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the interpolation value 3 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_CUBIC_BEZIER.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 3.
 * @param value The interpolation value 3 for the transition.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_value3_set(Evas_Object *obj, const char *prog, double value);

/**
 * @brief Gets the interpolation value 4 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_CUBIC_BEZIER.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 4.
 *
 * @return Interpolation value 4.
 */
EAPI double edje_edit_program_transition_value4_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the interpolation value 4 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_CUBIC_BEZIER.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 4.
 * @param value The interpolation value 4 for the transition.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_value4_set(Evas_Object *obj, const char *prog, double value);

/**
 * @brief Gets the duration of the transition in seconds.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the transition time.
 *
 * @return The duration of the transition.
 */
EAPI double edje_edit_program_transition_time_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the duration of the transition in seconds.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the transition time.
 * @param seconds The duration of the transition (in seconds).
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_time_set(Evas_Object *obj, const char *prog, double seconds);

/**
 * @brief Gets sample name of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 *
 * @return const char* sample_name on success, NULL otherwise.
 */
EAPI const char * edje_edit_program_sample_name_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets sample name of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 * @param name The name of the sample.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_sample_name_set(Evas_Object *obj, const char *prog, const char *name);

/**
 * @brief Gets tone name of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 *
 * @return const char* tone_name on success, NULL otherwise.
 */
EAPI const char * edje_edit_program_tone_name_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets tone name of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 * @param name The name of the tone.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_tone_name_set(Evas_Object *obj, const char *prog, const char *name);

/**
 * @brief Gets sample speed of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 *
 * @return Double speed on success, @c -1 otherwise.
 */
EAPI double edje_edit_program_sample_speed_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets sample speed of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 * @param speed New speed value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_sample_speed_set(Evas_Object *obj, const char *prog, double speed);

/**
 * @brief Gets tone duration of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 *
 * @return Double duration on success, @c -1 otherwise.
 */
EAPI double edje_edit_program_tone_duration_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets tone duration of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 * @param duration New duration value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_tone_duration_set(Evas_Object *obj, const char *prog, double duration);

/**
 * @brief Gets sample channel of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 *
 * @return Channel on success, @c 0 otherwise.
 */
EAPI unsigned char edje_edit_program_channel_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets sample channel of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 * @param channel New channel value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_channel_set(Evas_Object *obj, const char *prog, Edje_Channel channel);

/**
 * @brief Gets filter part name of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 *
 * @return const char* part_name on success, NULL otherwise.
 */
EAPI const char * edje_edit_program_filter_part_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets filter part name of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 * @param filter_part The name of the part to be set as filter.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_filter_part_set(Evas_Object *obj, const char *prog, const char *filter_part);

/**
 * @brief Gets filter state of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 *
 * @return const char* state_name on success, NULL otherwise.
 */
EAPI const char * edje_edit_program_filter_state_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets filter state of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program.
 * @param filter_state New filter state value.
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_filter_state_set(Evas_Object *obj, const char *prog, const char *filter_state);

//@}
/******************************************************************************/
/**************************   SCRIPTS API   ***********************************/
/******************************************************************************/
/**
 * @name Scripts API
 * Functions to deal with embryo scripts (see @ref edcref).
 */ //@{

/**
 * @brief Gets the Embryo script for the group of the given object.
 *
 * Get the shared script for the group under edition. Shared script means
 * the script {} block for the group, not counting what's in each program.
 * It returns a malloc'd duplicate of the code, so users are free to modify
 * the contents directly and they should remember to free() it when done.
 * NULL will be returned if there's no script or an error occurred.
 *
 * @param obj Object being edited.
 *
 * @return The shared script code for this group.
 */
EAPI char *edje_edit_script_get(Evas_Object *obj);

/**
 * @brief Sets the code for the group script.
 *
 * Set the Embryo source code for the shared script of the edited group.
 * Note that changing the code itself will not update the running VM, you
 * need to call @see edje_edit_script_compile() for it to get updated.
 *
 * @param obj The object being edited
 * @param code The Embryo source
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_script_set(Evas_Object *obj, const char *code);

/**
 * @brief Gets the Embryo script for the given program.
 *
 * Get the script code for the given program. Like the group script, this
 * function returns a duplicate of the code that the user can modify at will
 * and must free when done using it.
 * NULL will be returned if the program doesn't exist, doesn't have any
 * script or is not of type script.
 *
 * @param obj Object being edited
 * @param prog Program name
 *
 * @return The program script code
 */
EAPI char *edje_edit_script_program_get(Evas_Object *obj, const char *prog);

/**
 * @brief Sets the Embryo script for the given program.
 *
 * Set the Embryo source code for the program @p prog. It must be an
 * existing program of type EDJE_ACTION_TYPE_SCRIPT, or the function
 * will fail and do nothing.
 * Note that changing the code itself will not update the running VM, you
 * need to call @see edje_edit_script_compile() for it to get updated.
 *
 * @param obj The object being edited
 * @param prog The program name
 * @param code The Embryo source
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_script_program_set(Evas_Object *obj, const char *prog, const char *code);

/**
 * @brief Compiles the Embryo script for the given object.
 *
 * If required, this function will process all script code for the group and
 * build the bytecode, updating the running Embryo VM Program if the build
 * is successful.
 *
 * @param obj The object being edited
 *
 * @return @c EINA_TRUE in case of success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_script_compile(Evas_Object *obj);

/**
 * @brief Gets the list of errors resulting from the last script build.
 *
 * Get the list of errors that resulted from the last attempt to rebuild
 * the Embryo script for the edited group. This will be a standard Eina_List
 * with Edje_Edit_Script_Error pointers as its data.
 * The user should not do anything else but read the contents of this list.
 * These errors can be the output of the embryo compiler, or internal errors
 * generated by Edje_Edit if the preprocessing of the scripts failed.
 *
 * @param obj The object being edited
 *
 * @return A constant list of Edje_Edit_Script_Error, or NULL if there are none
 */
EAPI const Eina_List *edje_edit_script_error_list_get(Evas_Object *obj);

//@}
/******************************************************************************/
/************************   SOURCE CODE API   *********************************/
/******************************************************************************/
/**
 * @name Scripts API
 * Functions to deal with embryo scripts (see @ref edcref).
 */ //@{

/**
 * @brief Returns source code of the current edje edit object.
 *
 * Remember to free the string with edje_edit_string_free().
 *
 * This function will return source code of the whole group, loaded previously.
 * This function also will collect all possible resources that is required and
 * mentioned in description blocks. For example: all images, fonts, data, styles,
 * and color_classes.
 *
 * @param obj The object being edited
 *
 * @return Source code containing all resources required by the object.
 */

EAPI const char *edje_edit_source_generate(Evas_Object *obj);

/**
 * @brief Returns source code of the current edje edit object.
 *
 * Remember to free the string with edje_edit_string_free().
 *
 * This function will return source code of the whole group, loaded previously.
 * This function also will collect all possible resources that is required and
 * mentioned in description blocks. For example: all images, fonts, styles and
 * color_classes.
 *
 * @note A source code will be top block 'collection'.
 *
 * @param obj The object being edited
 *
 * @return Source code containing all resources required by the object.
 */

EAPI const char * edje_edit_object_source_generate(Evas_Object *obj);
/**
 * @brief Returns source code of all collections.
 *
 * Remember to free the string with free().
 *
 * This function will generate and return source code of all collections and
 * other top level blocks.
 *
 * @param obj The object being edited
 *
 * @return Source code as char *.
 */

EAPI char *edje_edit_full_source_generate(Evas_Object *obj);

/**
 * @brief Returns source code of global block data.
 *
 * Remember to free the string with edje_edit_string_free().
 *
 * @param obj The object being edited
 *
 * @return Source code of global block data.
 */
EAPI const char * edje_edit_data_source_generate(Evas_Object *obj);

/**
 * @brief Gets a list of color classes which given object use.
 *
 * @param obj The object being edited
 *
 * @return The color classes list
 */
EAPI Eina_List *
edje_edit_object_color_class_list_get(Evas_Object *obj);

/**
 * @brief Gets the source code for given color classes.
 *
 * @param obj The object being edited
 * @param color_classes The list of color classes for generate code
 *
 * @return The color classes source code
 */
EAPI const char *
edje_edit_color_classes_source_generate(Evas_Object *obj, Eina_List *color_classes);

//@}
/******************************************************************************/
/**************************   ERROR API   ***********************************/
/******************************************************************************/
/**
 * @name Error API
 * Functions to deal with error messages (see @ref edcref).
 */ //@{

EAPI extern Eina_Error EDJE_EDIT_ERROR_GROUP_CURRENTLY_USED;
EAPI extern Eina_Error EDJE_EDIT_ERROR_GROUP_REFERENCED;
EAPI extern Eina_Error EDJE_EDIT_ERROR_GROUP_DOES_NOT_EXIST;


#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
