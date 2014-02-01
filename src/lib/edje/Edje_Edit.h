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


typedef enum _Edje_Edit_Image_Comp
{
   EDJE_EDIT_IMAGE_COMP_RAW,
   EDJE_EDIT_IMAGE_COMP_USER,
   EDJE_EDIT_IMAGE_COMP_COMP,
   EDJE_EDIT_IMAGE_COMP_LOSSY
} Edje_Edit_Image_Comp;

struct _Edje_Edit_Script_Error
{
   const char *program_name; /* null == group shared script */
   int line;
   const char *error_str;
};
typedef struct _Edje_Edit_Script_Error Edje_Edit_Script_Error;

struct _Edje_Part_Image_Use
{
   const char *group;
   const char *part;
   struct {
      const char     *name;
      double         value;
   } state;
};
typedef struct _Edje_Part_Image_Use Edje_Part_Image_Use;

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
/** @name General API
 *  General functions that don't fit in other cateories.
 */ //@{

/** Adds an editable Edje object to the canvas.
 *
 * An Edje_Edit object is, for the most part, a standard Edje object. Only
 * difference is you can use the Edje_Edit API on them.
 *
 * @param e Evas canvas where to add the object.
 *
 * @return An Evas_Object of type Edje_Edit, or NULL if an error occurred.
 */
EAPI Evas_Object * edje_edit_object_add(Evas *e);

/** Free a generic Eina_List of (char *) allocated by an edje_edit_*_get() function.
 *
 * @param lst List of strings to free.
 */
EAPI void edje_edit_string_list_free(Eina_List *lst);

/** Free a generic string (char *) allocated by an edje_edit_*_get() function.
 *
 * @param str String to free.
 */
EAPI void edje_edit_string_free(const char *str);

/** Get the name of the program that compiled the edje file.
  * Can be 'edje_cc' or 'edje_edit'
  *
  * @param obj Object being edited.
  *
  * @return Compiler stored in the Edje file
  */
EAPI const char * edje_edit_compiler_get(Evas_Object *obj);

/** Save the modified edje object back to his file.
 *
 * Use this function when you are done with your editing, all the change made
 * to the current loaded group will be saved back to the original file.
 *
 * @note Source for the whole file will be auto generated and will overwrite
 * any previously stored source.
 *
 * @param obj Object to save back to the file it was loaded from.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 *
 * @todo Add a way to check what the error actually was, the way Edje Load does.
 */
EAPI Eina_Bool edje_edit_save(Evas_Object *obj);

/** Saves every group back into the file.
 *
 * @param obj Object to save.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 *
 * @see edje_edit_save()
 */
EAPI Eina_Bool edje_edit_save_all(Evas_Object *obj);

/** Print on standard output many information about the internal status
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
/** @name Groups API
 *  Functions to deal with groups property (see @ref edcref).
 */ //@{

/**
 * @brief Add an edje (empty) group to an edje object's group set.
 *
 * @param obj The pointer to edje object.
 * @param name The name of the group.
 *
 * @return EINA_TRUE If it could allocate memory to the part group added
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
 * @brief Copy whole group and all it's data into separate group.
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
 * @attention This group will copy the whole group and this operation can't be undone as all references to the group will be added to the file.
 * (for example all scripts will be written to the file directly)
 *
 */
EAPI Eina_Bool edje_edit_group_copy(Evas_Object *obj, const char *group_name, const char *copy_name);

/**
 * @brief Delete the specified group from the edje file.
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
 *
 * @attention be carefull, if you deleting group, it will delete all it's aliases also,
 * if you deleting alias, then it will delete alias only.
 *
 */
EAPI Eina_Bool edje_edit_group_del(Evas_Object *obj, const char *group_name);

/** Check if a group with the given name exist in the edje.
 *
 * @param obj Object being edited.
 * @param group Group name to check for.
 *
 * @return EINA_TRUE if group exists, EINA_FALSE if not.
 */
EAPI Eina_Bool edje_edit_group_exist(Evas_Object *obj, const char *group);

/** Set a new name for the current open group.
 *
 * You can only rename a group that is currently loaded
 * Note that the relative getter function don't exist as it doesn't make sense ;)
 * @param obj Object being edited.
 * @param new_name New name for the group.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_name_set(Evas_Object *obj, const char *new_name);

/** Get the group minimum width.
 *
 * @param obj Object being edited.
 *
 * @return The minimum width set for the group. -1 if an error occurred.
 */
EAPI int edje_edit_group_min_w_get(Evas_Object *obj);

/** Set the group minimum width.
 *
 * @param obj Object being edited.
 * @param w New minimum width for the group.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_min_w_set(Evas_Object *obj, int w);

/** Get the group minimum height.
 *
 * @param obj Object being edited.
 *
 * @return The minimum height set for the group. -1 if an error occurred.
 */
EAPI int edje_edit_group_min_h_get(Evas_Object *obj);

/** Set the group minimum height.
 *
 * @param obj Object being edited.
 * @param h New minimum height for the group.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_min_h_set(Evas_Object *obj, int h);

/** Get the group maximum width.
 *
 * @param obj Object being edited.
 *
 * @return The maximum width set for the group. -1 if an error occurred.
 */
EAPI int edje_edit_group_max_w_get(Evas_Object *obj);

/** Set the group maximum width.
 *
 * @param obj Object being edited.
 * @param w New maximum width for the group.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_max_w_set(Evas_Object *obj, int w);

/** Get the group maximum height.
 *
 * @param obj Object being edited.
 *
 * @return The maximum height set for the group. -1 if an error occurred.
 */
EAPI int edje_edit_group_max_h_get(Evas_Object *obj);

/** Set the group maximum height.
 *
 * @param obj Object being edited.
 * @param h New maximum height for the group.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_max_h_set(Evas_Object *obj, int h);

//@}
/******************************************************************************/
/**************************   ALIAS API   **************************************/
/******************************************************************************/
/** @name Alias API
 *  Functions to deal with aliases that just another names of the group in the edje (see @ref edcref).
 */ //@{

/**
 * Retrieves a list of aliases for this group.
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
 * Check if this group is an alias name.
 *
 * @param obj Object being edited.
 * @param alias_name Group name that is alias.
 *
 * @return EINA_TRUE if alias, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_alias_is(Evas_Object *obj, const char *alias_name);

/**
 * Return the main group name that is aliased by given alias name.
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
 * Add new alias to the given group.
 *
 * @attention when aliasing a group, be sure that the given group_name is no an alias.
 *
 * @param obj Object being edited.
 * @param group_name Group name that is being aliased.
 * @param alias_name Group name that is alias.
 *
 * @return EINA_TRUE if success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_alias_add(Evas_Object *obj, const char *group_name, const char *alias_name);

//@}
/******************************************************************************/
/**************************   DATA API   **************************************/
/******************************************************************************/
/** @name Data API
 *  Functions to deal with data embedded in the edje (see @ref edcref).
 */ //@{

/** Retrieves a list with the item names inside the data block.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being a name entry in the global data block for the file.
 */
EAPI Eina_List * edje_edit_data_list_get(Evas_Object *obj);

/** Create a new *global* data object in the given edje file.
 *
 * If another data entry with the same name exists, nothing is created and
 * EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param itemname Name for the new data entry.
 * @param value Value for the new data entry.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_data_add(Evas_Object *obj, const char *itemname, const char *value);

/** Delete the given data object from edje.
 *
 * @param obj Object being edited.
 * @param itemname Data entry to remove from the global data block.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_data_del(Evas_Object *obj, const char *itemname);

/** Get the data associated with the given itemname.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry to fetch the value for.
 *
 * @return Value of the given entry, or NULL if not found.
 */
EAPI const char * edje_edit_data_value_get(Evas_Object *obj, const char *itemname);

/** Set the data associated with the given itemname.
 *
 * @param obj Object being edited.
 * @param itemname Name of data entry to change the value.
 * @param value New value for the entry.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_data_value_set(Evas_Object *obj, const char *itemname, const char *value);

/** Change the name of the given data object.
 *
 * @param obj Object being edited.
 * @param itemname Data entry to rename.
 * @param newname New name for the data entry.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_data_name_set(Evas_Object *obj, const char *itemname, const char *newname);

/** Retrieves a list with the item names inside the data block at the group level.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being a name entry in the data block for the group.
 */
EAPI Eina_List * edje_edit_group_data_list_get(Evas_Object *obj);

/** Create a new data object in the given edje file *belonging to the current group*.
 *
 * If another data entry with the same name exists,
 * nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param itemname Name for the new data entry.
 * @param value Value for the new data entry.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_data_add(Evas_Object *obj, const char *itemname, const char *value);

/** Delete the given data object from the group.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry to remove.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_data_del(Evas_Object *obj, const char *itemname);

/** Get the data associated with the given itemname.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry.
 *
 * @return Value of the data entry or NULL if not found.
 */
EAPI const char * edje_edit_group_data_value_get(Evas_Object *obj, const char *itemname);

/** Set the data associated with the given itemname.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry to set the value.
 * @param value Value to set for the data entry.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_data_value_set(Evas_Object *obj, const char *itemname, const char *value);

/** Change the name of the given data object.
 *
 * @param obj Object being edited.
 * @param itemname Name of the data entry to rename.
 * @param newname New name for the data entry.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_group_data_name_set(Evas_Object *obj, const char *itemname, const char *newname);


//@}
/******************************************************************************/
/***********************   COLOR CLASSES API   ********************************/
/******************************************************************************/
/** @name Color Classes API
 *  Functions to deal with Color Classes (see @ref edcref).
 */ //@{

/** Get the list of all the Color Classes in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being one color class.
 */
EAPI Eina_List * edje_edit_color_classes_list_get(Evas_Object *obj);

/** Create a new color class object in the given edje.
 *
 * If another class with the same name exists nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param name Name for the new color class.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_add(Evas_Object *obj, const char *name);

/** Delete the given class object from edje.
 *
 * @param obj Object being edited.
 * @param name Color class to delete.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_del(Evas_Object *obj, const char *name);

/** Get all the colors that compose the class.
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
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_colors_get(Evas_Object *obj, const char *class_name, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3);

/** Set the colors for the given color class.
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
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_colors_set(Evas_Object *obj, const char *class_name, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

/** Change the name of a color class.
 *
 * @param obj Object being edited.
 * @param name Color class to rename.
 * @param newname New name for the color class.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_color_class_name_set(Evas_Object *obj, const char *name, const char *newname);

//@}


/******************************************************************************/
/**************************   TEXT STYLES *************************************/
/******************************************************************************/
/** @name Text styles API
 *  Functions to deal with text styles (see @ref edcref).
 */ //@{

/** Get the list of all the text styles in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being the name for a text style.
 */
EAPI Eina_List * edje_edit_styles_list_get(Evas_Object *obj);

/** Create a new text style object in the given edje.
 *
 * If another style with the same name exists nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param style Name for the new style.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_add(Evas_Object *obj, const char *style);

/** Delete the given text style and all the child tags.
 *
 * @param obj Object being edited.
 * @param style Style to delete.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_del(Evas_Object *obj, const char *style);

/** Get the list of all the tags name in the given text style.
 *
 * @param obj Object being edited.
 * @param style Style to get the tags for.
 *
 * @return List of strings, each being one tag in the given style.
 */
EAPI Eina_List * edje_edit_style_tags_list_get(Evas_Object *obj, const char *style);

/** Get the value of the given tag.
 *
 * @param obj Object being edited.
 * @param style Style containing the tag being.
 * @param tag Tag to get the value for.
 *
 * @return Value of the given tag.
 */
EAPI const char * edje_edit_style_tag_value_get(Evas_Object *obj, const char *style, const char *tag);

/** Set the value of the given tag.
 *
 * @param obj Object being edited.
 * @param style Style containing the tag to change.
 * @param tag Name of the tag to set the value for.
 * @param new_value Value for the tag.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_tag_value_set(Evas_Object *obj, const char *style, const char *tag, const char *new_value);

/** Set the name of the given tag.
 *
 * @param obj Object being edited.
 * @param style Style containing the tag to rename.
 * @param tag Tag to rename.
 * @param new_name New name for the tag.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_tag_name_set(Evas_Object *obj, const char *style, const char *tag, const char *new_name);

/** Add a new tag to the given text style.
 *
 * If another tag with the same name exists nothing is created and EINA_FALSE is returned.
 *
 * @param obj Object being edited.
 * @param style Style where to add the new tag.
 * @param tag_name Name for the new tag.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_tag_add(Evas_Object *obj, const char *style, const char *tag_name);

/** Delete the given tag.
 *
 * @param obj Object being edited.
 * @param style Style from where to remove the tag.
 * @param tag Tag to delete.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_style_tag_del(Evas_Object *obj, const char *style, const char *tag);


//@}
/******************************************************************************/
/************************   EXTERNALS API   ***********************************/
/******************************************************************************/
/** @name Externals API
 *  Functions to deal with list of external modules (see @ref edcref).
 */ //@{

/** Get the list of all the externals requested in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being an entry in the block of automatically loaded external modules.
 */
EAPI Eina_List * edje_edit_externals_list_get(Evas_Object *obj);

/** Add an external module to be requested on edje load.
 *
 * @param obj Object being edited.
 * @param external Name of the external module to add to the list of autoload.
 *
 * @return EINA_TRUE on success (or it was already there), EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_external_add(Evas_Object *obj, const char *external);

/** Delete the given external from the list.
 *
 * @param obj Object being edited.
 * @param external Name of the external module to remove from the autoload list.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_external_del(Evas_Object *obj, const char *external);


//@}
/******************************************************************************/
/**************************   PARTS API   *************************************/
/******************************************************************************/
/** @name Parts API
 *  Functions to deal with part objects (see @ref edcref).
 */ //@{

/** Get the list of all the parts in the given edje object.
 *
 * @param obj Object being edited.
 *
 * @return List of strings, each being the name for a part in the open group.
 */
EAPI Eina_List * edje_edit_parts_list_get(Evas_Object *obj);

/** Create a new part in the given edje.
 *
 * If another part with the same name just exists nothing is created and EINA_FALSE is returned.
 * Note that this function also create a default description for the part.
 *
 * @param obj Object being edited.
 * @param name Name for the new part.
 * @param type Type of the new part. See @ref edcref for more info on this.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_add(Evas_Object *obj, const char *name, Edje_Part_Type type);

/** Create a new part of type EXTERNAL in the given edje.
 *
 * If another part with the same name just exists nothing is created and EINA_FALSE is returned.
 * Note that this function also create a default description for the part.
 *
 * @param obj Object being edited.
 * @param name Name for the new part.
 * @param source The registered external type to use for this part.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_external_add(Evas_Object *obj, const char *name, const char *source);

/** Delete the given part from the edje.
 *
 * All the reference to this part will be zeroed.
 *
 * @param obj Object being edited.
 * @param part Name of part to delete.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_del(Evas_Object *obj, const char *part);

/** Check if a part with the given name exist in the edje object.
 *
 * @param obj Object being edited.
 * @param part Name of part to check for its existence.
 *
 * @return EINA_TRUE if the part exists, EINA_FALSE if not.
 */
EAPI Eina_Bool edje_edit_part_exist(Evas_Object *obj, const char *part);

/** Get the name of part stacked above the one passed.
 *
 * @param obj Object being edited.
 * @param part Name of part of which to check the one above.
 *
 * @return Name of the part above. NULL if an error occurred or if @p part is
 * the topmost part in the group.
 */
EAPI const char * edje_edit_part_above_get(Evas_Object *obj, const char *part);

/** Get the name of part stacked below the one passed.
 *
 * @param obj Object being edited.
 * @param part Name of part of which to check the one below.
 *
 * @return Name of the part below. NULL if an error occurred or if @p part is
 * the bottommost part in the group.
 */
EAPI const char * edje_edit_part_below_get(Evas_Object *obj, const char *part);

/** Move the given part below the previous one.
 *
 * @param obj Object being edited.
 * @param part Name of part to move one step below.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_restack_below(Evas_Object *obj, const char *part);

/** Move the given part below the part named below.
 *
 * @param obj Object being edited.
 * @param part Name of part which will be moved.
 * @param below Name of part for which will be moved 'part'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool
edje_edit_part_restack_part_below(Evas_Object *obj, const char* part, const char *below);

/** Move the given part above the next one.
 *
 * @param obj Object being edited.
 * @param part Name of part to move one step above.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_restack_above(Evas_Object *obj, const char *part);

/** Move the given part above the part named above.
 *
 * @param obj Object being edited.
 * @param part Name of part which will be moved.
 * @param above Name of part for which will be moved 'part'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool
edje_edit_part_restack_part_above(Evas_Object *obj, const char* part, const char *above);

/** Set a new name for part.
 *
 * Note that the relative getter function don't exist as it don't make sense ;)
 *
 * @param obj Object being edited.
 * @param part Name of part to rename.
 * @param new_name New name for the given part.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_name_set(Evas_Object *obj, const char *part, const char *new_name);

/** Get api's name of a part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 *
 * @return name of the api if successful, NULL otherwise.
 */
EAPI const char * edje_edit_part_api_name_get(Evas_Object *obj, const char *part);

/** Get api's description of a part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 *
 * @return description of the api if successful, NULL otherwise.
 */
EAPI const char * edje_edit_part_api_description_get(Evas_Object *obj, const char *part);

/** Set api's name of a part.
 *
 * @param obj Object being edited.
 * @param part Name of the part.
 * @param name New name for the api property.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_api_name_set(Evas_Object *obj, const char *part, const char *name);

/** Set api's description of a part.
 *
 * @param obj Object being edited.
 * @param part Name of part.
 * @param description New description for the api property.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_api_description_set(Evas_Object *obj, const char *part, const char *description);

/** Get the type of a part.
 *
 * @param obj Object being edited.
 * @param part Name of part to get the type of.
 *
 * @return Type of the part. See @ref edcref for details.
 */
EAPI Edje_Part_Type edje_edit_part_type_get(Evas_Object *obj, const char *part);

/** Get the clip_to part.
 *
 * @param obj Object being edited.
 * @param part Name of the part whose clipper to get.
 *
 * @return Name of the part @p part is clipped to. NULL is returned on errors and if the part don't have a clip.
 */
EAPI const char * edje_edit_part_clip_to_get(Evas_Object *obj, const char *part);

/** Set a part to clip part to.
 *
 * @param obj Object being edited.
 * @param part Part to set the clipper to.
 * @param clip_to Part to use as clipper, if NULL then the clipping value will be cancelled (unset clipping).
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_clip_to_set(Evas_Object *obj, const char *part, const char *clip_to);

/** Get the source of part.
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

/** Set the source of part.
 *
 * @param obj Object being edited.
 * @param part Part to set the source of.
 * @param source Value for the source parameter.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 *
 * @see edje_edit_part_source_get()
 *
 * @note You can't change the source for EXTERNAL parts, it's akin to changing
 * the type of the part.
 *
 * NOTE: This is not applied now. You must reload the edje to see the change.
 */
EAPI Eina_Bool edje_edit_part_source_set(Evas_Object *obj, const char *part, const char *source);

/** Get the effect for a given part.
 *
 * Gets the effect used for parts of type TEXT. See @ref edcref for more details.
 *
 * @param obj Object being edited.
 * @param part Part to get the effect of.
 *
 * @return The effect set for the part.
 */
EAPI Edje_Text_Effect edje_edit_part_effect_get(Evas_Object *obj, const char *part);

/** Set the effect for a given part.
 * Effects and shadow directions can be combined.
 *
 * For effect and shadow direction list please look at Edje Part Text ref page.
 *
 * @param obj Object being edited.
 * @param part Part to set the effect to. Only makes sense on type TEXT.
 * @param effect Effect to set for the part.
 *
 * @see Edje_Part_Text
 */
EAPI Eina_Bool edje_edit_part_effect_set(Evas_Object *obj, const char *part, Edje_Text_Effect effect);

/** Get the current selected state in part.
 *
 * @param obj Object being edited.
 * @param part Part to get the selected state of.
 * @param value Pointer to a double where the value of the state will be stored.
 *
 * @return The name of the currently selected state for the part.
 */
EAPI const char * edje_edit_part_selected_state_get(Evas_Object *obj, const char *part, double *value);

/** Set the current state in part.
 *
 * @param obj Object being edited.
 * @param part Part to set the state of.
 * @param state Name of the state to set.
 * @param value Value of the state.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_selected_state_set(Evas_Object *obj, const char *part, const char *state, double value);

/** Get mouse_events for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if the mouse events is accepted.
 *
 * @return EINA_TRUE if part will accept mouse events, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_mouse_events_get(Evas_Object *obj, const char *part);

/** Set mouse_events for part.
 *
 * @param obj Object being edited.
 * @param part The part to set if the mouse events is accepted.
 * @param mouse_events EINA_TRUE if part will accept mouse events, EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_mouse_events_set(Evas_Object *obj, const char *part, Eina_Bool mouse_events);

/** Get repeat_events for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if it will pass all events to the other parts.
 *
 * @return EINA_TRUE if the events received will propagate to other parts, EINA_FALSE otherwise
 */
EAPI Eina_Bool edje_edit_part_repeat_events_get(Evas_Object *obj, const char *part);

/** Set repeat_events for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if will repeat all the received mouse events to other parts.
 * @param repeat_events EINA_TRUE if the events received will propagate to other parts, EINA_FALSE otherwise
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_repeat_events_set(Evas_Object *obj, const char *part, Eina_Bool repeat_events);

/** Get ignore_flags for part.
 *
 * @param obj Object being edited.
 * @param part Part to get which event_flags are being ignored.
 *
 * @return The Event flags set to the part.
 */
EAPI Evas_Event_Flags edje_edit_part_ignore_flags_get(Evas_Object *obj, const char *part);

/** Set ignore_flags for part.
 *
 * @param obj Object being edited.
 * @param part Part to set which event flags will be ignored.
 * @param ignore_flags The Event flags to be ignored by the part.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_ignore_flags_set(Evas_Object *obj, const char *part, Evas_Event_Flags ignore_flags);

/** Set scale property for the part.
 *
 * This property tells Edje that the given part should be scaled by the
 * Edje scale factor.
 *
 * @param obj Object being edited.
 * @param part Part to set scale for.
 * @param scale Scale value to set.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_scale_set(Evas_Object *obj, const char *part, Eina_Bool scale);

/** Get scale for the part.
 *
 * @param obj Object being edited.
 * @param part Part to get the scale value of.
 *
 * @return Whether scale is on (EINA_TRUE) or not.
 */
EAPI Eina_Bool edje_edit_part_scale_get(Evas_Object *obj, const char *part);

/** Get horizontal dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if can be dragged horizontally;
 *
 * @return 1 (or -1) if the part can be dragged horizontally, 0 otherwise.
 */
EAPI int edje_edit_part_drag_x_get(Evas_Object *obj, const char *part);

/** Set horizontal dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if should be dragged horizontally.
 * @param drag 1 (or -1) if the part should be dragged horizontally, 0 otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_x_set(Evas_Object *obj, const char *part, int drag);

/** Get vertical dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to get if can be dragged vertically.
 *
 * @return 1 (or - 1) if the part can be dragged vertically, 0 otherwise.
 */
EAPI int edje_edit_part_drag_y_get(Evas_Object *obj, const char *part);

/** Set vertical dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to set if should be dragged vertically.
 * @param drag 1 (or -1) of the part shpuld be dragged vertically, 0 otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_y_set(Evas_Object *obj, const char *part, int drag);

/** Get horizontal dragable step for part.
 *
 * @param obj Object being edited.
 * @param part Part to get the drag horizontal step value.
 *
 * @return The step value.
 */
EAPI int edje_edit_part_drag_step_x_get(Evas_Object *obj, const char *part);

/** Set horizontal dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to set the drag horizontal step value.
 * @param step The step the will be dragged.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_step_x_set(Evas_Object *obj, const char *part, int step);

/** Get vertical dragable step for part.
 *
 * @param obj Object being edited.
 * @param part Part to get the drag vertical step value.
 *
 * @return The step value.
 */
EAPI int edje_edit_part_drag_step_y_get(Evas_Object *obj, const char *part);

/** Set vertical dragable state for part.
 *
 * @param obj Object being edited.
 * @param part Part to set the drag vertical step value.
 * @param step The step the will be dragged.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_step_y_set(Evas_Object *obj, const char *part, int step);

/** Get horizontal dragable count for part.
 *
 * @param obj Object being edited.
 * @param part Part to get the drag horizontal count value.
 */
EAPI int edje_edit_part_drag_count_x_get(Evas_Object *obj, const char *part);

/** Set horizontal dragable count for part.
 *
 * @param obj Object being edited.
 * @param part Part to set the drag horizontal count value.
 * @param count The count value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_count_x_set(Evas_Object *obj, const char *part, int count);

/** Get vertical dragable count for part.
 *
 * @param obj Object being edited.
 * @param part Part to get the drag vertical count value.
 */
EAPI int edje_edit_part_drag_count_y_get(Evas_Object *obj, const char *part);

/** Set vertical dragable count for part.
 *
 * @param obj Object being edited.
 * @param part Part to set the drag vertical count value.
 * @param count The count value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_count_y_set(Evas_Object *obj, const char *part, int count);

/** Get the name of the part that is used as 'confine' for the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to get the name that is used as 'confine' for the given draggies.
 *
 * @return The name of the confine part or NULL (if unset).
 */
EAPI const char * edje_edit_part_drag_confine_get(Evas_Object *obj, const char *part);

/** Set the name of the part that is used as 'confine' for the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to set the name that is used as 'confine' for the given draggies.
 * @param confine The name of the confine part or NULL to unset confine.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_confine_set(Evas_Object *obj, const char *part, const char *confine);

/** Get the name of the part that is used as the receiver of the drag event.
 *
 * @param obj Object being edited.
 * @param part Part to get the name that is used as the receiver of the drag event.
 *
 * @return The name of the part that will receive events, or NULL (if unset).
 */
EAPI const char * edje_edit_part_drag_event_get(Evas_Object *obj, const char *part);

/** Set the name of the part that will receive events from the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to set the name that will receive events from the given draggies.
 * @param event The name of the part that will receive events, or NULL to unset.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_event_set(Evas_Object *obj, const char *part, const char *event);

/** Get the name of the part that is used as 'threshold' for the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to get the name that is used as 'threshold' for the given draggies.
 *
 * @return The name of the threshold part or NULL (if unset).
 */
EAPI const char * edje_edit_part_drag_threshold_get(Evas_Object *obj, const char *part);

/** Set the name of the part that is used as 'threshold' for the given draggies.
 *
 * @param obj Object being edited.
 * @param part Part to set the name that is used as 'threshold' for the given draggies.
 * @param confine The name of the threshold part or NULL to unset confine.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_part_drag_threshold_set(Evas_Object *obj, const char *part, const char *threshold);


//@}
/******************************************************************************/
/**************************   STATES API   ************************************/
/******************************************************************************/
/** @name States API
 *  Functions to deal with part states (see @ref edcref).
 */ //@{

/** Get the list of all the states in the given part.
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

/** Set a new name for the given state in the given part.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state Name of the state to rename.
 * @param value Value of the state to rename.
 * @param new_name The new name for the state.
 * @param new_value The new value for the state.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_name_set(Evas_Object *obj, const char *part, const char *state, double value, const char *new_name, double new_value);

/** Create a new state to the give part.
 *
 * @param obj Object being edited.
 * @param part Part to set the name of the new state.
 * @param name Name for the new state (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE if successfully, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_add(Evas_Object *obj, const char *part, const char *name, double value);

/** Delete the given part state from the edje.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The current name of the state (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE if successfully, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_del(Evas_Object *obj, const char *part, const char *state, double value);

/** Check if a part state with the given name exist.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to check (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE if the part state exist, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_exist(Evas_Object *obj, const char *part, const char *state, double value);

/** Copies the state @p from into @p to. If @p to doesn't exist it will be created.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param from State to copy from (not including state value).
 * @param val_from The value of the state to copy from.
 * @param to State to copy into (not including state value).
 * @param val_to The value of the state to copy into.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_copy(Evas_Object *obj, const char *part, const char *from, double val_from, const char *to, double val_to);

/** Get the 'rel1 relative X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel1 relative X' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel1 relative X' value of the part state.
 */
EAPI double edje_edit_state_rel1_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the 'rel1 relative Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel1 relative Y' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel1 relative Y' value of the part state.
 */
EAPI double edje_edit_state_rel1_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the 'rel2 relative X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel2 relative X' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel2 relative X' value of the part state.
 */
EAPI double edje_edit_state_rel2_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the 'rel2 relative Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel2 relative Y' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel2 relative Y' value of the part state.
 */
EAPI double edje_edit_state_rel2_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the 'rel1 relative X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel1 relative X' (not including the state value).
 * @param value The state value.
 * @param x The new 'rel1 relative X' value to set'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the 'rel1 relative Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel1 relative Y' (not including the state value).
 * @param value The state value.
 * @param y The new 'rel1 relative Y' value to set'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/** Set the 'rel2 relative X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel2 relative X' (not including the state value).
 * @param value The state value.
 * @param x The new 'rel2 relative X' value to set'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the 'rel2 relative Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel2 relative Y' (not including the state value).
 * @param value The state value.
 * @param y The new 'rel2 relative Y' value to set'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/** Get the 'rel1 offset X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel1 offset X' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel1 offset X' value of the part state.
 */
EAPI int edje_edit_state_rel1_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the 'rel1 offset Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel1 offset Y' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel1 offset Y' value of the part state.
 */
EAPI int edje_edit_state_rel1_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the 'rel2 offset X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel2 offset X' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel2 offset X' value of the part state.
 */
EAPI int edje_edit_state_rel2_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the 'rel2 offset Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get 'rel2 offset Y' (not including the state value).
 * @param value The state value.
 *
 * @return The 'rel2 offset Y' value of the part state.
 */
EAPI int edje_edit_state_rel2_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the 'rel1 offset X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel1 offset X' (not including the state value).
 * @param value The state value.
 * @param x The new 'rel1 offset X' value to set'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the 'rel1 offset Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel1 offset Y' (not including the state value).
 * @param value The state value.
 * @param y The new 'rel1 offset Y' value to set'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/** Set the 'rel2 offset X' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel2 offset X' (not including the state value).
 * @param value The state value.
 * @param x The new 'rel2 offset X' value to set'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the 'rel2 offset Y' value of state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set 'rel2 offset Y' (not including the state value).
 * @param value The state value.
 * @param y The new 'rel2 offset Y' value to set'.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/** Get the part name rel1x is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The state that contain which the part name rel1x is relative to (not including the state value).
 * @param value The state value.
 *
 * @return The part name rel1x is relative to or NULL if the part is relative to the whole interface.
 */
EAPI const char * edje_edit_state_rel1_to_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the part name rel1y is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The state that contain which the part name rel1y is relative to (not including the state value).
 * @param value The state value.
 *
 * @return The part name rel1y is relative to or NULL if the part is relative to the whole interface.
 */
EAPI const char * edje_edit_state_rel1_to_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the part name rel2x is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The state that contain which the part name rel2x is relative to (not including the state value).
 * @param value The state value.
 *
 * @return The part name rel2x is relative to or NULL if the part is relative to the whole interface.
 */
EAPI const char * edje_edit_state_rel2_to_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the part name rel2y is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The state that contain which the part name rel2y is relative to (not including the state value).
 * @param value The state value.
 *
 * @return The part name rel2y is relative to or NULL if the part is relative to the whole interface.
 */
EAPI const char * edje_edit_state_rel2_to_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the part rel1x is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set rel1x is relative to (not including the state value).
 * @param value The state value.
 * @param rel_to The name of the part that is used as container/parent (NULL make the part relative to the whole interface).
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_to_x_set(Evas_Object *obj, const char *part, const char *state, double value, const char *rel_to);

/** Set the part rel1y is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set rel1y is relative to (not including the state value).
 * @param value The state value.
 * @param rel_to The name of the part that is used as container/parent (NULL make the part relative to the whole interface).
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel1_to_y_set(Evas_Object *obj, const char *part, const char *state, double value, const char *rel_to);

/** Set the part rel2x is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set rel2x is relative to (not including the state value).
 * @param value The state value.
 * @param rel_to The name of the part that is used as container/parent (NULL make the part relative to the whole interface).
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_to_x_set(Evas_Object *obj, const char *part, const char *state, double value, const char *rel_to);

/** Set the part rel2y is relative to.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set rel2y is relative to (not including the state value).
 * @param value The state value.
 * @param rel_to The name of the part that is used as container/parent (NULL make the part relative to the whole interface).
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_rel2_to_y_set(Evas_Object *obj, const char *part, const char *state, double value, const char *rel_to);

/** Get the color of a part state.
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

/** Get the color2 of a part state.
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

/** Get the color3 of a part state.
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

/** Set the color of a part state.
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
 * @return EINA_TRUE If successfull, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_color_set(Evas_Object *obj, const char *part, const char *state, double value, int r, int g, int b, int a);

/** Set the color2 of a part state.
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
 * @return EINA_TRUE If successfull, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_color2_set(Evas_Object *obj, const char *part, const char *state, double value, int r, int g, int b, int a);

/** Set the color3 of a part state.
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
 * @return EINA_TRUE If successfull, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_color3_set(Evas_Object *obj, const char *part, const char *state, double value, int r, int g, int b, int a);

/** Get the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get horizontal align (not including the state value).
 * @param value The state value.
 *
 * @return The horizontal align value for the given state
 */
EAPI double edje_edit_state_align_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get horizontal align (not including the state value).
 * @param value The state value.
 *
 * @return The vertical align value for the given state
 */
EAPI double edje_edit_state_align_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the horizontal align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get horizontal align (not including the state value).
 * @param value The state value.
 * @param align The new vertical align value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_align_x_set(Evas_Object *obj, const char *part, const char *state, double value,  double align);

/** Set the vertical align value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get vertical align (not including the state value).
 * @param value The state value.
 * @param align The new vertical align value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_align_y_set(Evas_Object *obj, const char *part, const char *state, double value,  double align);

/** Get the minimum width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get minimum width (not including the state value).
 * @param value The state value.
 *
 * @return The minimum width value.
 */
EAPI int edje_edit_state_min_w_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the minimum width value of a part state.
 * The minimum width should be greater than 0.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set minimum width (not including the state value).
 * @param value The state value.
 * @param min_w Minimum width value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_min_w_set(Evas_Object *obj, const char *part, const char *state, double value, int min_w);

/** Get the minimum height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get minimum height (not including the state value).
 * @param value The state value.
 *
 * @return The minimum height value.
 */
EAPI int edje_edit_state_min_h_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the minimum height value of a part state.
 * The minimum height should be greater than 0.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set minimum height (not including the state value).
 * @param value The state value.
 * @param min_h Minimum height value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_min_h_set(Evas_Object *obj, const char *part, const char *state, double value, int min_h);

/** Get the maximum width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get maximum width (not including the state value).
 * @param value The state value.
 *
 * @return The maximum width value.
 */
EAPI int edje_edit_state_max_w_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the maximum width value of a part state.
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
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_max_w_set(Evas_Object *obj, const char *part, const char *state, double value, int max_w);

/** Get the maximum height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get maximum height (not including the state value).
 * @param value The state value.
 *
 * @return The maximum height value.
 */
EAPI int edje_edit_state_max_h_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the maximum height value of a part state.
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
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_max_h_set(Evas_Object *obj, const char *part, const char *state, double value, int max_h);

/** Get the fixed width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fixed width value (not including the state value).
 * @param value The state value.
 *
 * @return The fixed width value.
 */
EAPI Eina_Bool edje_edit_state_fixed_w_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the fixed width value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fixed width value (not including the state value).
 * @param value The state value.
 * @param fixed Fixed width value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fixed_w_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool fixed);

/** Get the fixed height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fixed height value (not including the state value).
 * @param value The state value.
 *
 * @return The fixed height value.
 */
EAPI Eina_Bool edje_edit_state_fixed_h_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the fixed height value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set maximum height (not including the state value).
 * @param value The state value.
 * @param fixed Fixed height value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fixed_h_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool fixed);

/** Get the minimum aspect value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get minimum aspect (not including the state value).
 * @param value The state value.
 *
 * @return The minimum aspect
 */
EAPI double edje_edit_state_aspect_min_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the maximum aspect value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get maximum aspect (not including the state value).
 * @param value The state value.
 *
 * @return The maximum aspect
 */
EAPI double edje_edit_state_aspect_max_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the minimum aspect value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set minimum aspect (not including the state value).
 * @param value The state value.
 * @param aspect Minimum aspect value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_aspect_min_set(Evas_Object *obj, const char *part, const char *state, double value, double aspect);

/** Set the maximum aspect value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set maximum aspect (not including the state value).
 * @param value The state value.
 * @param aspect Maximum aspect value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_aspect_max_set(Evas_Object *obj, const char *part, const char *state, double value, double aspect);

/** Get the aspect preference of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get aspect preference (not including the state value).
 * @param value The state value.
 *
 * @return The aspect preference (0 = None, 1 = Vertical, 2 = Horizontal, 3 = Both)
 */
EAPI unsigned char edje_edit_state_aspect_pref_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the aspect preference of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set aspect preference (not
 *              including the state value).
 * @param value The state value.
 * @param pref The aspect preference to set (0 = None, 1 = Vertical, 2
 *             = Horizontal, 3 = Both)
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_aspect_pref_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char pref);

/** Get the smooth property for given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the fill horizontal origin relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The smooth value.
 */
EAPI Eina_Bool
edje_edit_state_fill_smooth_get(Evas_Object *obj, const char *part, const char *state, double value);


/** Set the smooth property for given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal origin relative to area (not including the state value).
 * @param value The state value.
 * @param smooth The smooth value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool
edje_edit_state_fill_smooth_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool smooth);

/** Get the fill horizontal origin relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the fill horizontal origin relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill horizontal origin relative to area.
 */
EAPI double edje_edit_state_fill_origin_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the fill vertical origin relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill vertical origin relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill vertical origin relative to area.
 */
EAPI double edje_edit_state_fill_origin_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the fill horizontal origin offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill horizontal origin offset relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill horizontal origin offset relative to area.
 */
EAPI int edje_edit_state_fill_origin_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the fill vertical origin offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill vertical origin offset relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill vertical origin offset value.
 */
EAPI int edje_edit_state_fill_origin_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the fill horizontal origin relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal origin relative to area (not including the state value).
 * @param value The state value.
 * @param x The fill horizontal origin value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_origin_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the fill horizontal origin relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill vertical origin relative to area (not including the state value).
 * @param value The state value.
 * @param y The fill vertical origin value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_origin_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/** Set the fill horizontal origin offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal origin offset relative to area (not including the state value).
 * @param value The state value.
 * @param x The fill horizontal origin offset value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_origin_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the fill vertical origin offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill vertical origin offset relative to area (not including the state value).
 * @param value The state value.
 * @param y The fill vertical origin offset value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_origin_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/** Get the fill horizontal size relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill horizontal size relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill horizontal size relative to area.
 */
EAPI double edje_edit_state_fill_size_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the fill vertical size relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get fill vertical size relative to area (not including the state value).
 * @param value The state value.
 *
 * @return The fill vertical size relative to area.
 */
EAPI double edje_edit_state_fill_size_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the fill horizontal size offset value of a part state.
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

/** Get the fill vertical size offset value of a part state.
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

/** Set the fill horizontal size relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal size
 * relative value (not including the state value).
 * @param value The state value.
 * @param x The horizontal size relative value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_size_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the fill vertical size relative value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill vertical size
 * relative value (not including the state value).
 * @param value The state value.
 * @param x The vertical size relative value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_size_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the fill horizontal size offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill horizontal size
 * offset relative value (not including the state value).
 * @param value The state value.
 * @param x The horizontal size offset value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_size_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, double x);

/** Set the fill vertical size offset value of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set fill vertical size offset
 * relative value (not including the state value).
 * @param value The state value.
 * @param y The vertical size offset value.
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_fill_size_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, double y);

/** Get the visibility of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get visibility (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE if the state is visible, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_visible_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the visibility of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set visibility (not including the state value).
 * @param value The state value.
 * @param visible To set state visible (EINA_TRUE if the state is visible, EINA_FALSE otherwise)
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_visible_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool visible);

/** Get the color class of the given part state.
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

/** Set the color class of the given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set color class (not including the state value).
 * @param value The state value.
 * @param color_class The color class to assign.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_color_class_set(Evas_Object *obj, const char *part, const char *state, double value, const char *color_class);

/** Get the list of parameters for an external part.
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

/** Get the external parameter type and value.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter to look for.
 * @param type The type of the parameter will be stored here.
 * @param val Pointer to value will be stored here - DO NOT FREE IT!
 *
 * @return EINA_TRUE if the parameter was found, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Edje_External_Param_Type *type, void **val);

/** Get external parameter of type INT.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type INT (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val The value of the parameter.
 *
 * @return EINA_TRUE if successful. EINA_FALSE if not found or is of different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_int_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, int *val);

/** Get external parameter of type BOOL.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type BOOL (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val The value of the parameter.
 *
 * @return EINA_TRUE if successful. EINA_FALSE if not found or is of different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_bool_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Eina_Bool *val);

/** Get external parameter of type DOUBLE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type DOUBLE (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val The value of the parameter.
 *
 * @return EINA_TRUE if successful. EINA_FALSE if not found or is of different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_double_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, double *val);

/** Get external parameter of type STRING.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of
 *              type STRING (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val The value of the parameter.
 *
 * @return EINA_TRUE if successful. EINA_FALSE if not found or is of
 * different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_string_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char **val);

/** Get external parameter of type CHOICE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of
 *        type CHOICE (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val The value of the parameter.
 *
 * @return EINA_TRUE if successful. EINA_FALSE if not found or is of
 * different type.
 */
EAPI Eina_Bool edje_edit_state_external_param_choice_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char **val);

/** Set the external parameter type and value, adding it if it didn't
 * exist before.
 *
 * @param obj Object being edited.

 * @param part Part that contain state.
 * @param state The name of the state to get external parameter (not
 *              including the state value).
 * @param value The state value.
 * @param param The name of the paramter set.
 * @param type The type of the parameter.
 *
 * @return EINA_TRUE if it was set, EINA_FALSE otherwise.
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

/** Set external parameter of type INT.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of
 *              type INT (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val Value will be stored here.
 *
 * @return EINA_TRUE if it was set, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_int_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, int val);

/** Set external parameter of type BOOL.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type BOOL (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val Value will be stored here.
 *
 * @return EINA_TRUE if it was set, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_bool_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Eina_Bool val);

/** Set external parameter of type DOUBLE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type DOUBLE (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val Value will be stored here.
 *
 * @return EINA_TRUE if it was set, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_double_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, double val);

/** Set external parameter of type STRING.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type STRING (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val Value will be stored here.
 *
 * @return EINA_TRUE if it was set, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_string_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char *val);

/** Set external parameter of type CHOICE.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get external parameter of type CHOICE (not including the state value).
 * @param value The state value.
 * @param param The name of the paramter.
 * @param val Value will be stored here.
 *
 * @return EINA_TRUE if it was set, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_external_param_choice_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char *val);


//@}
/******************************************************************************/
/**************************   TEXT API   ************************************/
/******************************************************************************/
/** @name Text API
 *  Functions to deal with text objects (see @ref edcref).
 */ //@{

/** Get the text of a part state.
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

/** Set the text of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set text (not including the state value).
 * @param value The state value.
 * @param text The new text to assign.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_set(Evas_Object *obj, const char *part, const char *state, double value,const char *text);

/** Get font name for a given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part to get the font of.
 * @param state The state of the part to get the font of.
 * @param value Value of the state.
 *
 * @return Font used by the part or NULL if error or nothing is set.
 */
EAPI const char * edje_edit_state_font_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set font name for a given part state.
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
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_font_set(Evas_Object *obj, const char *part, const char *state, double value, const char *font);

/** Get the text size of a part state
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get text size (not including the state value).
 * @param value The state value.
 *
 * @return The text size or -1 on errors.
 */
EAPI int edje_edit_state_text_size_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the text size of a part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set text size (not including the state value).
 * @param value The state value.
 * @param size The new font size to set (in pixel)
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_size_set(Evas_Object *obj, const char *part, const char *state, double value, int size);

/** Get the text horizontal align of a part state.
 *
 * The value range is from 0.0(right) to 1.0(left)
 * If the value is between -1.0 and 0.0 then it uses align automatically.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the text horizontal align (not including the state value).
 * @param value The state value.
 *
 * @return The text horizont align value
 */
EAPI double edje_edit_state_text_align_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the text vertical align of a part state.
 *
 * The value range is from 0.0(top) to 1.0(bottom)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the text vertical align (not including the state value).
 * @param value The state value.
 *
 * @return The text horizont align value
 */
EAPI double edje_edit_state_text_align_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the text horizontal align of a part state.
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
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_align_x_set(Evas_Object *obj, const char *part, const char *state, double value, double align);

/** Set the text vertical align of a part state.
 *
 * The value range is from 0.0(top) to 1.0(bottom)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the text vertical align (not including the state value).
 * @param value The state value.
 * @param align The new text vertical align value
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_align_y_set(Evas_Object *obj, const char *part, const char *state, double value, double align);

/** Get the text elipsis of a part state.
 *
 * The value range is from 0.0(right) to 1.0(left), and -1.0 (if disabled)
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the text elipses value (not including the state value).
 * @param value The state value.
 *
 * @return The text elipsis value
 */
EAPI double edje_edit_state_text_elipsis_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the text vertical align of a part state.
 *
 * The value range is from 0.0(right) to 1.0(left)
 * If the value is in range from -1.0 to 0.0 then ellipsis is disabled.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the text elipses value (not including the state value).
 * @param value The state value.
 * @param balance The position where to cut the string
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_elipsis_set(Evas_Object *obj, const char *part, const char *state, double value, double balance);

/** Get if the text part fit it's container horizontally
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the if the text part fit it's container horizontally (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE If the part fit it's container horizontally, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_fit_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set if the text part should fit it's container horizontally
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the if the text part fit it's container horizontally (not including the state value).
 * @param value The state value.
 * @param fit EINA_TRUE to make the text fit it's container horizontally, EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_fit_x_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool fit);

/** Get if the text part fit it's container vertically
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the if the text part fit it's container vertically (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE If the part fit it's container vertically, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_fit_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set if the text part should fit it's container vertically
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the if the text part fit it's container vertically (not including the state value).
 * @param value The state value.
 * @param fit EINA_TRUE to make the text fit it's container vertically, EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_fit_y_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool fit);

/** Get if the text part forces the minimum horizontal size of the container to be equal to the minimum horizontal size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the the minimum horizontal size of the container to be equal (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE If the part forces container's minimum horizontal size, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_min_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get if the text part forces the maximum horizontal size of the container to be equal to the maximum horizontal size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the the minimum horizontal size of the container to be equal (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE If the part forces container's maximum horizontal size, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_max_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get if the text part forces the minimum vertical size of the container to be equal to the minimum vertical size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the the minimum vertical size of the container to be equal (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE If the part forces container's minimum vertical size, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_min_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get if the text part forces the maximum vertical size of the container to be equal to the maximum vertical size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the the maximum vertical size of the container to be equal (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE If the part forces container's maximum vertical size, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_max_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set if the text part forces the minimum horizontal size of the container to be equal to the minimum horizontal size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the minimum horizontal size of the container to be equal (not including the state value).
 * @param value The state value.
 * @param v EINA_TRUE to make the text force it's forces container's minimum horizontal size, EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_min_x_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/** Set if the text part forces the maximum horizontal size of the container to be equal to the maximum horizontal size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum horizontal size of the container to be equal (not including the state value).
 * @param value The state value.
 * @param v EINA_TRUE to make the text force it's forces container's maximum horizontal size, EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_max_x_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/** Set if the text part forces the minimum vertical size of the container to be equal to the minimum vertical size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the minimum vertical size of the container to be equal (not including the state value).
 * @param value The state value.
 * @param v EINA_TRUE to make the text force it's forces container's minimum vertical size, EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_min_y_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/** Set if the text part forces the maximum vertical size of the container to be equal to the maximum vertical size of the text part
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum vertical size of the container to be equal (not including the state value).
 * @param value The state value.
 * @param v EINA_TRUE to make the text force it's forces container's maximum vertical size, EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_text_max_y_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool v);

/** Get style name for a given part state.
 *
 * @param obj Object being edited.
 * @param part The name of the part to get the style of.
 * @param state The state of the part to get the style of.
 * @param value Value of the state.
 *
 * @return Style used by the part or NULL if error or nothing is set.
 */
EAPI const char *
edje_edit_state_text_style_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set style name for a given part state.
 *
 * Causes the part to use the default style and tags defined in the "style" block with the specified name.
 *
 * @param obj Object being edited.
 * @param part Part to set the style of.
 * @param state State in which the style is set.
 * @param value Value of the state.
 * @param style The style name to use.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool
edje_edit_state_text_style_set(Evas_Object *obj, const char *part, const char *state, double value, const char *style);

/** Get part name, which used as text source.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum vertical size of
 * the container to be equal (not including the state value).
 *
 * @return The name of part or NULL, if text_source param not a setted.
 */
EAPI const char *
edje_edit_state_text_source_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the source text part for a given part.
 * Causes the part to display the text content of another part and update them
 * as they change.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the the maximum vertical size of
 * the container to be equal (not including the state value).
 * @param source The text source part name.
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool
edje_edit_state_text_source_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source);

/** Get the list of all the fonts in the given edje.
 *
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 *
 * @return A list containing all the fonts names found in the edje file.
 */
EAPI Eina_List * edje_edit_fonts_list_get(Evas_Object *obj);

/** Add a new font to the edje file.
 *
 * The newly created font will be available to all the groups in the edje, not only the current one.
 *
 * @param obj Object being edited.
 * @param path The file path to load the font from.
 * @param alias The alias for file, or NULL to use filename
 *
 * @return EINA_TRUE if font cat be loaded, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_font_add(Evas_Object *obj, const char *path, const char* alias);

/** Delete font from the edje file.
 *
 * The font will be removed from all the groups in the edje, not only the current one.
 *
 * @param obj Object being edited.
 * @param alias The font alias
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise (including the
 * case when the alias is not valid).
 */
EAPI Eina_Bool edje_edit_font_del(Evas_Object *obj, const char* alias);

/** Get font path for a given font alias.
 *
 * Remember to free the string with edje_edit_string_free()
 *
 * @param obj Object being edited.
 * @param alias The font alias.
 *
 * @return The path of the given font alias.
 */
EAPI const char *edje_edit_font_path_get(Evas_Object *obj, const char *alias);


/** Get font name for a given part state.
 *
 * Remember to free the returned string using edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the name of the font used (not including the state value).
 * @param value The state value.
 *
 * @return The name of the font used in the given part state.
 */
EAPI const char * edje_edit_state_font_get(Evas_Object *obj, const char *part, const char *state, double value);

//@}
/******************************************************************************/
/**************************   IMAGES API   ************************************/
/******************************************************************************/
/** @name Images API
 *  Functions to deal with image objects (see @ref edcref).
 */ //@{

/** Get the list of all the images in the given edje.
 * Use edje_edit_string_list_free() when you don't need the list anymore.
 *
 * @param obj Object being edited.
 *
 * @return A List containing all images names found in the edje file.
 */
EAPI Eina_List * edje_edit_images_list_get(Evas_Object *obj);

/** Add an new image to the image collection
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
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_image_add(Evas_Object *obj, const char *path);

/** Delete an image from the image collection
 *
 * It actually write directly to the file so you don't have to save.
 * Can't delete image if it is used by any part.
 *
 * @param obj Object being edited.
 * @param name The name of the image file to include in the edje.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise (including the
 * case when the name is not valid or image is in use).
 */
EAPI Eina_Bool edje_edit_image_del(Evas_Object *obj, const char *name);

/** Replace one image in all descriptions
 *
 * @param obj Object being edited.
 * @param name The name of the image to replace.
 * @param name The new_name of the image to replace with.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise (including the
 * case when one of the names is not valid)
 */
EAPI Eina_Bool edje_edit_image_replace(Evas_Object *obj, const char *name, const char *new_name);

/** Get list of (Edje_Part_Image_Use *) - group-part-state triplets where given
 * image is used
 *
 * Use edje_edit_image_usage_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param name The name of the image.
 * @param first_only If EINA_TRUE, return only one triplete.
 *
 * @return Eina_List containing Edje_Part_Image_Use if successful, NULL otherwise
 */
EAPI Eina_List* edje_edit_image_usage_list_get(Evas_Object *obj, const char *name, Eina_Bool first_only);

/** Free an Eina_List of (Edje_Part_Image_Use *) allocated by an edje_edit_image_usage_list_get() function.
 *
 * @param lst List of strings to free.
 */
EAPI void edje_edit_image_usage_list_free(Eina_List *lst);

/** Add an image entry to the image collection
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
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_image_data_add(Evas_Object *obj, const char *name, int id);

/** Get normal image name for a given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to get the name that is being used (not including the state value).
 * @param value The state value.
 *
 * @return The name of the image used by state.
 */
EAPI const char * edje_edit_state_image_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set normal image for a given part state.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to set the image that will be used (not including the state value).
 * @param value The state value.
 * @param image The name of the image (must be an image contained in the edje file).
 *
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_image_set(Evas_Object *obj, const char *part, const char *state, double value, const char *image);

/** Get image id for a given image name.
 *
 * @param obj Object being edited.
 * @param image_name The image name.
 *
 * @return The id of the given image name.
 */
EAPI int edje_edit_image_id_get(Evas_Object *obj, const char *image_name);

/** Get compression type for the given image.
 *
 * @param obj Object being edited.
 * @param image The name of the image.
 *
 * @return One of Image Compression types.
 * (EDJE_EDIT_IMAGE_COMP_RAW, EDJE_EDIT_IMAGE_COMP_USER, EDJE_EDIT_IMAGE_COMP_COMP, EDJE_EDIT_IMAGE_COMP_LOSSY).
 */
EAPI Edje_Edit_Image_Comp edje_edit_image_compression_type_get(Evas_Object *obj, const char *image);

/** Get compression rate for the given image.
 *
 * @param obj Object being edited.
 * @param image The name of the image.
 *
 * @return The compression rate (if the imnage is @c
 *         EDJE_EDIT_IMAGE_COMP_LOSSY) or < 0, on errors.
 */
EAPI int edje_edit_image_compression_rate_get(Evas_Object *obj, const char *image);

/** Get the image border of a part state.
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

/** Set the image border of a part state.
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
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_image_border_set(Evas_Object *obj, const char *part, const char *state, double value, int l, int r, int t, int b);

/** Get if the image center should be draw.
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
 * @return 2 if the center of the bordered image is draw without alpha, 1 dawing with alpha and 0 not drawing the center.
 */
EAPI unsigned char edje_edit_state_image_border_fill_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set if the image center should be draw.
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
 * @return EINA_TRUE if successful, EINA_FALSE - otherwise.
 */
EAPI Eina_Bool edje_edit_state_image_border_fill_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char fill);

/** Get the list of all the tweens images in the given part state.
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

/** Add a new tween frame to the given part state.
 *
 * The tween param must be the name of an existing image.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to add a new tween frame (not including the state value).
 * @param value The state value.
 * @param tween The name of the image to add.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_tween_add(Evas_Object *obj, const char *part, const char *state, double value, const char *tween);

/** Remove the first tween with the given name.
 *
 * The image is not removed from the edje.
 *
 * @param obj Object being edited.
 * @param part Part that contain state.
 * @param state The name of the state to del the tween (not including the state value).
 * @param value The state value.
 * @param tween The name of the image to del.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_tween_del(Evas_Object *obj, const char *part, const char *state, double value, const char *tween);


//@}
/******************************************************************************/
/*************************   SPECTRUM API   ***********************************/
/******************************************************************************/
/** @name Spectrum API
 *  Functions to manage spectrum (see @ref edcref).
 */ //@{

/** Get the list of all the spectrum in the given edje object.
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 *
 * @return A list containing all the spectra names.
 */
EAPI Eina_List * edje_edit_spectrum_list_get(Evas_Object *obj);

/** Add a new spectra in the given edje object.
 *
 * @param obj Object being edited.
 * @param name The name of the spectra to include in the edje.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_add(Evas_Object *obj, const char *name);

/** Delete the given spectra from the edje object.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra to delete.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_del(Evas_Object *obj, const char *spectra);

/** Change the name of the given spectra.
 *
 * @param obj Object being edited.
 * @param spectra The name of the current spectra.
 * @param name The new name to assign.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_name_set(Evas_Object *obj, const char *spectra, const char *name);

/** Get the number of stops in the given spectra.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra.
 *
 * @return The number of stops (or 0 on errors).
 */
EAPI int edje_edit_spectra_stop_num_get(Evas_Object *obj, const char *spectra);

/** Set the number of stops in the given spectra.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra.
 * @param num The number of stops you want
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_stop_num_set(Evas_Object *obj, const char *spectra, int num);

/** Get the colors of the given stop.
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
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_spectra_stop_color_get(Evas_Object *obj, const char *spectra, int stop_number, int *r, int *g, int *b, int *a, int *d);

/** Set the colors of the given stop.
 *
 * @param obj Object being edited.
 * @param spectra The name of the spectra.
 * @param stop_number The number of the stops,
 * @param r The red color value to set,
 * @param g The green color value to set,
 * @param b The blue color value to set,
 * @param a The alpha color value to set,
 * @param d The delta stop value to set,
 */
EAPI Eina_Bool edje_edit_spectra_stop_color_set(Evas_Object *obj, const char *spectra, int stop_number, int r, int g, int b, int a, int d);


//@}
/******************************************************************************/
/*************************   GRADIENT API   ***********************************/
/******************************************************************************/
/** @name Gradient API
 *  Functions to deal with gradient objects (see @ref edcref).
 */ //@{

/** Get the type of gradient.
 *
 * Remember to free the string with edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get the gradient type (not including the state value).
 * @param value The state value.
 *
 * @return The type of gradient used in state.
 * (linear, linear.diag, linear.codiag, radial, rectangular, angular, sinosoidal)
 */
EAPI const char * edje_edit_state_gradient_type_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the type of gradient.
 *
 * Gradient type can be on of the following: linear, linear.diag, linear.codiag, radial, rectangular, angular, sinusoidal
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set the gradient type (not including the state value).
 * @param value The state value.
 * @param type The type of gradient to use.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_gradient_type_set(Evas_Object *obj, const char *part, const char *state, double value, const char *type);

/** Get if the current gradient use the fill properties or the gradient_rel as params.
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set the gradient type (not including the state value).
 * @param value The state value.
 *
 * @return EINA_TRUE if gradient use the fill properties, EINA_FALSE otherwise.
 * */
EAPI Eina_Bool edje_edit_state_gradient_use_fill_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the spectra used by part state.
 *
 * Remember to free the string with edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get the spectra name used (not including the state value).
 * @param value The state value.
 *
 * @return The spectra name used in state.
 */
EAPI const char * edje_edit_state_gradient_spectra_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the spectra used by part state.
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set the spectra (not including the state value).
 * @param value The state value.
 * @param spectra The spectra name to assign
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_gradient_spectra_set(Evas_Object *obj, const char *part, const char *state, double value, const char *spectra);

/** Get the angle of the gradient.
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get the angle (not including the state value).
 * @param value The state value.
 *
 * @return The angle of the gradient.
 */
EAPI int edje_edit_state_gradient_angle_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the angle of the gradient.
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set the angle (not including the state value).
 * @param value The state value.
 * @param angle The angle to set.
 */
EAPI void edje_edit_state_gradient_angle_set(Evas_Object *obj, const char *part, const char *state, double value, int angle);

/** Get the gradient rel1 horizontal relative value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get rel1 relative x value (not including the state value).
 * @param value The state value.
 *
 * @return The gradient rel1 horizontal relative value.
 */
EAPI double edje_edit_state_gradient_rel1_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the gradient rel1 vertical relative value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get rel1 relative y value (not including the state value).
 * @param value The state value.
 *
 * @return The gradient rel1 vertical relative value.
 */
EAPI double edje_edit_state_gradient_rel1_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the gradient rel2 horizontal relative value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get rel2 relative x value (not including the state value).
 * @param value The state value.
 *
 * @return The gradient rel2 horizontal relative value.
 */
EAPI double edje_edit_state_gradient_rel2_relative_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the gradient rel2 vertical relative value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get rel2 relative y value (not including the state value).
 * @param value The state value.
 *
 * @return The gradient rel2 vertical relative value.
 */
EAPI double edje_edit_state_gradient_rel2_relative_y_get(Evas_Object *obj, const char *part, const char *state, double value);


/** Set the gradient rel1 horizontal relative value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set rel1 relative x value (not including the state value).
 * @param value The state value.
 * @param val The rel1 relative x to be set,
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise..
 */
EAPI Eina_Bool edje_edit_state_gradient_rel1_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double val);


/** Set the gradient rel1 vertical relative value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set rel1 relative y value (not including the state value).
 * @param value The state value.
 * @param val The rel1 relative y to be set,
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise..
 */
EAPI Eina_Bool edje_edit_state_gradient_rel1_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double val);

/** Set the gradient rel2 horizontal relative value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set rel2 relative x value (not including the state value).
 * @param value The state value.
 * @param val The rel2 relative x to be set,
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise..
 */
EAPI Eina_Bool edje_edit_state_gradient_rel2_relative_x_set(Evas_Object *obj, const char *part, const char *state, double value, double val);

/** Set the gradient rel2 vertical relative value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set rel2 relative y value (not including the state value).
 * @param value The state value.
 * @param val The rel2 relative y to be set,
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise..
 */
EAPI Eina_Bool edje_edit_state_gradient_rel2_relative_y_set(Evas_Object *obj, const char *part, const char *state, double value, double val);

/** Get the gradient rel1 horizontal offset value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get rel1 offset x value (not including the state value).
 * @param value The state value.
 *
 * @return The gradient rel1 horizontal offset value.
 */
EAPI int edje_edit_state_gradient_rel1_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the gradient rel1 vertical offset value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get rel1 offset y value (not including the state value).
 * @param value The state value.
 *
 * @return The gradient rel1 vertical offset value.
 */
EAPI int edje_edit_state_gradient_rel1_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the gradient rel2 horizontal offset value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get rel2 offset x value (not including the state value).
 * @param value The state value.
 *
 * @return The gradient rel2 horizontal offset value.
 */
EAPI int edje_edit_state_gradient_rel2_offset_x_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Get the gradient rel2 vertical offset value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to get rel2 offset y value (not including the state value).
 * @param value The state value.
 *
 * @return The gradient rel2 vertical offset value.
 */
EAPI int edje_edit_state_gradient_rel2_offset_y_get(Evas_Object *obj, const char *part, const char *state, double value);

/** Set the gradient rel1 horizontal offset value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set rel1 offset x value (not including the state value).
 * @param value The state value.
 * @param val The rel1 offset x value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_gradient_rel1_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, int val);

/** Set the gradient rel1 vertical offset value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set rel1 offset y value (not including the state value).
 * @param value The state value.
 * @param val The rel1 offset y value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_gradient_rel1_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, int val);

/** Set the gradient rel2 horizontal offset value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set rel2 offset x value (not including the state value).
 * @param value The state value.
 * @param val The rel2 offset x value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_gradient_rel2_offset_x_set(Evas_Object *obj, const char *part, const char *state, double value, int val);

/** Set the gradient rel2 vertical offset value
 *
 * @param obj Object being edited.
 * @param part The part that contain state.
 * @param state The name of the state to set rel2 offset y value (not including the state value).
 * @param value The state value.
 * @param val The rel2 offset y value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_state_gradient_rel2_offset_y_set(Evas_Object *obj, const char *part, const char *state, double value, int val);


//@}
/******************************************************************************/
/*************************   PROGRAMS API   ***********************************/
/******************************************************************************/
/** @name Programs API
 *  Functions to deal with programs (see @ref edcref).
 */ //@{

/** Get the list of all the programs in the given edje object.
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 *
 * @return A list containing all the program names.
 */
EAPI Eina_List * edje_edit_programs_list_get(Evas_Object *obj);

/** Add a new program to the edje file
 *
 * If a program with the same name just exist the function will fail.
 *
 * @param obj Object being edited.
 * @param name The name of the new program.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_add(Evas_Object *obj, const char *name);

/** Remove the given program from the edje file.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to remove.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_del(Evas_Object *obj, const char *prog);

/** Check if a program with the given name exist in the edje object.
 *
 * @param obj Object being edited.
 * @param prog The prog of the program that will be searched.
 *
 * @return EINA_TRUE if the program exist, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_exist(Evas_Object *obj, const char *prog);

/** Run the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to execute.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_run(Evas_Object *obj, const char *prog);

/** Set a new name for the given program
 *
 * @param obj Object being edited.
 * @param prog The current program name.
 * @param new_name The new name to assign.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_name_set(Evas_Object *obj, const char *prog, const char *new_name);

/** Get source of a given program.
 *
 * Remember to free the returned string using edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get source.
 *
 * @return The source value por program.
 */
EAPI const char * edje_edit_program_source_get(Evas_Object *obj, const char *prog);

/** Set source of the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set source.
 * @param source The new source value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_source_set(Evas_Object *obj, const char *prog, const char *source);

/** Get signal of a given program.
 *
 * Remember to free the returned string using edje_edit_string_free().
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the signal.
 *
 * @return The signal value for program.
 */
EAPI const char * edje_edit_program_signal_get(Evas_Object *obj, const char *prog);

/** Set signal of the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the signal.
 * @param signal The new signal value.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_signal_set(Evas_Object *obj, const char *prog, const char *signal);

/** Get in.from of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the delay.
 *
 * @return The delay.
 */
EAPI double edje_edit_program_in_from_get(Evas_Object *obj, const char *prog);

/** Set in.from of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the delay.
 * @param seconds Number of seconds to delay the program execution
 *
 * */
EAPI Eina_Bool edje_edit_program_in_from_set(Evas_Object *obj, const char *prog, double seconds);

/** Get in.range of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get random delay.
 *
 * @return The delay random.
 */
EAPI double edje_edit_program_in_range_get(Evas_Object *obj, const char *prog);

/** Set in.range of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set random delay.
 * @param seconds Max random number of seconds to delay.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_in_range_set(Evas_Object *obj, const char *prog, double seconds);

/** Get the action of a given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the action.
 *
 * @return The action type, or -1 on errors.
 * Action can be one of EDJE_ACTION_TYPE_NONE, _STATE_SET, ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, _DRAG_VAL_STEP, _DRAG_VAL_PAGE, _SCRIPT
 */
EAPI Edje_Action_Type edje_edit_program_action_get(Evas_Object *obj, const char *prog);

/** Set the action of a given program.
 *
 * Action can be one of EDJE_ACTION_TYPE_NONE, _STATE_SET, ACTION_STOP, SIGNAL_EMIT, DRAG_VAL_SET, _DRAG_VAL_STEP, _DRAG_VAL_PAGE, _SCRIPT
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the action.
 * @param action The new action type.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_action_set(Evas_Object *obj, const char *prog, Edje_Action_Type action);

/** Get the list of the targets for the given program.
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param prog The name of the progrem to get the list of the targets.
 *
 * @return A list with all the targets names, or NULL on error.
 */
EAPI Eina_List * edje_edit_program_targets_get(Evas_Object *obj, const char *prog);

/** Add a new target program to the list of 'targets' in the given program.
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
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_target_add(Evas_Object *obj, const char *prog, const char *target);

/** Deletes a target from the list of 'targets' in the given program.
 *
 * If program action is EDJE_ACTION_TYPE_ACTION_STOP then 'target' must be an existing program name.
 * If action is EDJE_ACTION_TYPE_STATE_SET then 'target' must be an existing part name.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to del a target from the list of targets.
 * @param target The name of another program or another part.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_target_del(Evas_Object *obj, const char *prog, const char *target);

/** Clear the 'targets' list of the given program
 *
 * @param obj Object being edited.
 * @param prog The name of the program to cleaar the 'targets' list.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_targets_clear(Evas_Object *obj, const char *prog);

/** Get the list of action that will be run after the give program
 *
 * Use edje_edit_string_list_free() when you don't need it anymore.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the list of actions
 *
 * @return A list with all program names. or NULL on error.
 */
EAPI Eina_List * edje_edit_program_afters_get(Evas_Object *obj, const char *prog);

/** Add a new program name to the list of 'afters' in the given program.
 *
 * All the programs listed in 'afters' will be executed after program execution.
 *
 * @param obj Object being edited.
 * @param prog The name of the program that contains the list of afters
 * @param after The name of another program to add to the afters list
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_after_add(Evas_Object *obj, const char *prog, const char *after);

/** Delete the given program from the list of 'afters' of the program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program from where to remove the after.
 * @param after The name of the program to remove from the list of afters.
 *
 * @return EINA_TRUE is successful or not in the list, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_after_del(Evas_Object *obj, const char *prog, const char *after);

/** Clear the 'afters' list of the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to clear the 'afters' list.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_afters_clear(Evas_Object *obj, const char *prog);

/** Get the state for the given program
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

/** Get api's name of a program.
 *
 * @param obj Object being edited.
 * @param prog Name of program.
 *
 * @return name of the api if successful, NULL otherwise.
 */
EAPI const char * edje_edit_program_api_name_get(Evas_Object *obj, const char *prog);

/** Get api's description of a program.
 *
 * @param obj Object being edited.
 * @param prog Name of program.
 *
 * @return description of the api if successful, NULL otherwise.
 */
EAPI const char * edje_edit_program_api_description_get(Evas_Object *obj, const char *prog);

/** Set api's name of a program.
 *
 * @param obj Object being edited.
 * @param prog Name of the part.
 * @param name New name for the api property.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_api_name_set(Evas_Object *obj, const char *prog, const char *name);

/** Set api's description of a program.
 *
 * @param obj Object being edited.
 * @param prog Name of the program.
 * @param description New description for the api property.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_api_description_set(Evas_Object *obj, const char *prog, const char *description);

/** Set the state for the given program
 *
 * In a STATE_SET action this is the name of state to set.
 * In a SIGNAL_EMIT action is the name of the signal to emit.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set a state.
 * @param state The nameo of the state to set (not including the state value)
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_state_set(Evas_Object *obj, const char *prog, const char *state);

/** Get the value of state for the given program.
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

/** Set the value of state for the given program.
 *
 * In a STATE_SET action this is the value of state to set.
 * Not used on SIGNAL_EMIT action.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the value of state.
 * @param value The vale to set.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_value_set(Evas_Object *obj, const char *prog, double value);

/** Get the state2 for the given program
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

/** Set the state2 for the given program
 *
 * In a STATE_SET action is not used
 * In a SIGNAL_EMIT action is the source of the emitted signal.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the state2.
 * @param state2 The name of the state to set.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_state2_set(Evas_Object *obj, const char *prog, const char *state2);

/** Get the value of state2 for the given program.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the state2 value.
 *
 * @return The vale of the state2 for the program.
 */
EAPI double edje_edit_program_value2_get(Evas_Object *obj, const char *prog);

/** Set the value2 of state for the given program.
 *
 * This is used in DRAG_ACTION
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the state2 value.
 * @param value The value of the state2 to set.
 */
EAPI Eina_Bool edje_edit_program_value2_set(Evas_Object *obj, const char *prog, double value);

/** Get the type of transition to use when apply animations.
 *
 * Can be one of: EDJE_TWEEN_MODE_NONE, EDJE_TWEEN_MODE_LINEAR, EDJE_TWEEN_MODE_SINUSOIDAL, EDJE_TWEEN_MODE_ACCELERATE or EDJE_TWEEN_MODE_DECELERATE.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the transition.
 *
 * @return The type of transition used by program.
 */
EAPI Edje_Tween_Mode edje_edit_program_transition_get(Evas_Object *obj, const char *prog);

/** Set the type of transition to use when apply animations.
 *
 * Can be one of: EDJE_TWEEN_MODE_NONE, EDJE_TWEEN_MODE_LINEAR, EDJE_TWEEN_MODE_SINUSOIDAL, EDJE_TWEEN_MODE_ACCELERATE or EDJE_TWEEN_MODE_DECELERATE.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the transition.
 * @param transition The transition type to set
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_set(Evas_Object *obj, const char *prog, Edje_Tween_Mode transition);

/** Get the interpolation value 1 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_ACCELERATE_FACTOR, EDJE_TWEEN_MODE_DECELERATE_FACTOR, EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR, EDJE_TWEEN_MODE_DIVISOR_INTERP, EDJE_TWEEN_MODE_BOUNCE or EDJE_TWEEN_MODE_SPRING.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 1.
 *
 * @return interpolation value 1.
 */
EAPI double
edje_edit_program_transition_value1_get(Evas_Object *obj, const char *prog);

/** Set the interpolation value 1 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_ACCELERATE_FACTOR, EDJE_TWEEN_MODE_DECELERATE_FACTOR, EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR, EDJE_TWEEN_MODE_DIVISOR_INTERP, EDJE_TWEEN_MODE_BOUNCE or EDJE_TWEEN_MODE_SPRING.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 1.
 * @param value The interpolation value 1 for the transition.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool
edje_edit_program_transition_value1_set(Evas_Object *obj, const char *prog, double value);

/** Get the interpolation value 2 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_DIVISOR_INTERP, EDJE_TWEEN_MODE_BOUNCE or EDJE_TWEEN_MODE_SPRING.
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 2.
 *
 * @return interpolation value 2.
 */
EAPI double
edje_edit_program_transition_value2_get(Evas_Object *obj, const char *prog);

/** Set the interpolation value 2 of the transition.
 *  Can be used with one of transition type: EDJE_TWEEN_MODE_DIVISOR_INTERP, EDJE_TWEEN_MODE_BOUNCE or EDJE_TWEEN_MODE_SPRING.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the interpolation value 2.
 * @param value The interpolation value 2 for the transition.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool
edje_edit_program_transition_value2_set(Evas_Object *obj, const char *prog, double value);

/** Get the duration of the transition in seconds.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to get the transition time.
 *
 * @return The duration of the transition.
 */
EAPI double edje_edit_program_transition_time_get(Evas_Object *obj, const char *prog);

/** Set the duration of the transition in seconds.
 *
 * @param obj Object being edited.
 * @param prog The name of the program to set the transition time.
 * @param seconds The duration of the transition (in seconds).
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_program_transition_time_set(Evas_Object *obj, const char *prog, double seconds);

EAPI const char * edje_edit_program_filter_part_get(Evas_Object *obj, const char *prog);
EAPI Eina_Bool edje_edit_program_filter_part_set(Evas_Object *obj, const char *prog, const char *filter_part);

//@}
/******************************************************************************/
/**************************   SCRIPTS API   ***********************************/
/******************************************************************************/
/** @name Scripts API
 *  Functions to deal with embryo scripts (see @ref edcref).
 */ //@{

/**
 * Get the Embryo script for the group of the given object.
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
 * Set the code for the group script.
 *
 * Set the Embryo source code for the shared script of the edited group.
 * Note that changing the code itself will not update the running VM, you
 * need to call edje_edit_script_compile for it to get updated.
 *
 * @param obj The object being edited
 * @param code The Embryo source
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_script_set(Evas_Object *obj, const char *code);

/**
 * Get the Embryo script for the given program.
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
 * Set the Embryo script for the given program.
 *
 * Set the Embryo source code for the program @p prog. It must be an
 * existing program of type EDJE_ACTION_TYPE_SCRIPT, or the function
 * will fail and do nothing.
 * Note that changing the code itself will not update the running VM, you
 * need to call edje_edit_script_compile for it to get updated.
 *
 * @param obj The object being edited
 * @param prog The program name.
 * @param code The Embryo source
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EAPI Eina_Bool edje_edit_script_program_set(Evas_Object *obj, const char *prog, const char *code);

/**
 * Compile the Embryo script for the given object
 *
 * If required, this function will process all script code for the group and
 * build the bytecode, updating the running Embryo VM Program if the build
 * is succesful.
 *
 * @param obj The object being edited
 *
 */
EAPI Eina_Bool edje_edit_script_compile(Evas_Object *obj);

/**
 * Get the list of errors resulting from the last script build
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
/**************************   ERROR API   ***********************************/
/******************************************************************************/
/** @name Error API
 *   to deal with error messages (see @ref edcref).
 */ //@{

EAPI extern Eina_Error EDJE_EDIT_ERROR_GROUP_CURRENTLY_USED;
EAPI extern Eina_Error EDJE_EDIT_ERROR_GROUP_REFERENCED;
EAPI extern Eina_Error EDJE_EDIT_ERROR_GROUP_DOES_NOT_EXIST;


#ifdef __cplusplus
}
#endif

#endif
