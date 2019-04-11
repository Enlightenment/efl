#ifndef EDJE_CONVERT_H__
# define EDJE_CONVERT_H__

typedef struct _Old_Edje_Image_Directory	Old_Edje_Image_Directory; /**< An old Edje image directory */
typedef struct _Old_Edje_Font_Directory         Old_Edje_Font_Directory; /**< An old Edje font directory */
typedef struct _Old_Edje_External_Directory	Old_Edje_External_Directory; /**< An old Edje external directory */
typedef struct _Old_Edje_Part			Old_Edje_Part; /**< An old Edje part */
typedef struct _Old_Edje_Part_Collection	Old_Edje_Part_Collection; /**< An old Edje part collection */
typedef struct _Old_Edje_Part_Description       Old_Edje_Part_Description; /**< An old Edje part description */
typedef struct _Old_Edje_Part_Description_Spec_Image Old_Edje_Part_Description_Spec_Image; /**< An old Edje part description image */
typedef struct _Edje_Data                       Edje_Data; /**< A key/value tuple */


/**
 * @struct _Edje_Data
 * @brief A structure that stores a key/value tuple.
 */
struct _Edje_Data
{
   const char *key; /**< key name */
   char *value; /**< contents value */
};

/*----------*/

/**
 * @struct _Old_Edje_Font_Directory
 * @brief A structure that stores old font directory entries.
 */
struct _Old_Edje_Font_Directory
{
   Eina_List *entries; /**< a list of Edje_Font_Directory_Entry */
};

/**
 * @struct _Old_Edje_Image_Directory
 * @brief A structure that stores old image directory entries.
 */
struct _Old_Edje_Image_Directory
{
   Eina_List *entries; /**< a list of Edje_Image_Directory_Entry */
   Eina_List *sets; /**< a list of Edje_Image_Directory_Set */
};

/**
 * @struct _Old_Edje_External_Directory
 * @brief A structure that stores old external directory entries.
 */
struct _Old_Edje_External_Directory
{
   Eina_List *entries; /**< a list of Edje_External_Directory_Entry */
};

/**
 * @struct _Old_Edje_File
 * @brief A structure that stores old Edje files information.
 */
struct _Old_Edje_File
{
   const char                     *path; /**< path */
   time_t                          mtime; /**< modification time */

   Old_Edje_External_Directory    *external_dir; /**< external directory */
   Old_Edje_Font_Directory        *font_dir; /**< fonts directory */
   Old_Edje_Image_Directory       *image_dir; /**< images directory */
   Edje_Part_Collection_Directory *collection_dir; /**< collection directory */
   Eina_List                      *data; /**< list of Edje_Data */
   Eina_List                      *styles; /**< list of Edje_Style */
   Eina_List                      *color_classes; /**< list of Edje_Color_Class */
   Eina_List                      *text_classes; /**< list of Edje_Text_Class */
   Eina_List                      *size_classes; /**< list of Edje_Size_Class */

   const char                     *compiler; /**< compiler name */
   int                             version; /**< Edje version */
   int                             feature_ver; /**< features version */
};

/**
 * @struct _Old_Edje_Part_Collection
 * @brief A structure that stores old Edje part collection information.
 */
struct _Old_Edje_Part_Collection
{
   Eina_List *programs; /**< a list of Edje_Program */
   Eina_List *parts; /**< a list of Edje_Part */
   Eina_List *data; /**< a list of Edje_Data */

   int        id; /**< the collection id */

   Eina_Hash *alias; /**< aliasing part*/

   struct {
      Edje_Size min; /**< minimum size */
      Edje_Size max; /**< maximum size */
   } prop;

   int        references; /**< references count */
#ifdef EDJE_PROGRAM_CACHE
   struct {
      Eina_Hash                   *no_matches;
      Eina_Hash                   *matches;
   } prog_cache;
#endif

   Embryo_Program   *script; /**< all the embryo script code for this group */
   const char       *part; /**< part name */

   unsigned char    lua_script_only; /** LUA script only */

   unsigned char    checked : 1; /**< contents checked and registered */
};

/**
 * @struct _Old_Edje_Part
 * @brief A structure that stores old Edje part information.
 */
struct _Old_Edje_Part
{
   const char            *name; /**< the name if any of the part */
   Old_Edje_Part_Description *default_desc; /**< the part descriptor for default */
   Eina_List             *other_desc; /**< other possible descriptors */
   const char            *source, *source2, *source3, *source4, *source5, *source6;
   int                    id; /**< its id number */
   int                    clip_to_id; /**< the part id to clip this one to */
   Edje_Part_Dragable     dragable; /**< dragable part */
   Eina_List             *items; /**< packed items for box and table */
   unsigned char          type; /**< what type (image, rect, text) */
   unsigned char          effect; /**< 0 = plain... */
   unsigned char          mouse_events; /**< it will affect/respond to mouse events */
   unsigned char          repeat_events; /**< it will repeat events to objects below */
   Evas_Event_Flags       ignore_flags; /**< ignore flags */
   Evas_Event_Flags       mask_flags; /**< mask flags */
   unsigned char          scale; /**< should certain properties scale with edje scale factor? */
   unsigned char          precise_is_inside; /**< whether is precisely inside */
   unsigned char          use_alternate_font_metrics; /**< use alternate font metrics */
   unsigned char          pointer_mode; /**< pointer mode */
   unsigned char          entry_mode; /**< entry mode */
   unsigned char          select_mode; /**< entry selection mode */
   unsigned char          multiline; /**< multiline enabled */
   Edje_Part_Api          api; /**< part API */
   unsigned char          required; /**< required */
};

/**
 * @struct _Old_Edje_Part_Description_Spec_Image
 * @brief A structure that stores old Edje part description image information.
 */
struct _Old_Edje_Part_Description_Spec_Image
{
   Eina_List     *tween_list; /**< list of Edje_Part_Image_Id */
   int            id; /**< the image id to use */
   int            scale_hint; /**< evas scale hint */
   Eina_Bool      set; /**< if image condition it's content */

   Edje_Part_Description_Spec_Border border; /**< border settings */
   Edje_Part_Description_Spec_Fill   fill; /**< fill settings */
};

/**
 * @struct _Old_Edje_Part_Description
 * @brief A structure that stores old Edje part description information.
 */
struct _Old_Edje_Part_Description
{
   Edje_Part_Description_Common common; /**< common part description */
   Old_Edje_Part_Description_Spec_Image image; /**< image part description */
   Edje_Part_Description_Spec_Text text; /**< text part description */
   Edje_Part_Description_Spec_Box box; /**< box part description */
   Edje_Part_Description_Spec_Table table; /**< table part description */

   Eina_List *external_params; /**< list of Edje_External_Param */
};

/**
 * Convert old Edje files into new Edje files.
 *
 * @param file an Eet_File to write the new Edje file to
 * @param oedf the old Edje file
 *
 * @return an Edje_File pointer to the converted file
 */
Edje_File *_edje_file_convert(Eet_File *file, Old_Edje_File *oedf);

/**
 * Convert old edje part collection into the new Edje file.
 *
 * @param file an Eet_File to write the new Edje file to
 * @param oedc The Old edje part collection
 *
 * @return a new Edje part collection
 */
Edje_Part_Collection *_edje_collection_convert(Edje_File *file,
                                               Old_Edje_Part_Collection *oedc);

/**
 * Convert old Edje part description into new format.
 *
 * @param type The edje par description common type
 * @param ce an edje collection directory entry
 * @param the old edje part description
 *
 * @return a new edje part description common
 */
Edje_Part_Description_Common *_edje_description_convert(int type,
                                                        Edje_Part_Collection_Directory_Entry *ce,
                                                        Old_Edje_Part_Description *oed);

/**
 * Get the current Edje file.
 *
 * @return the current Edje file
 */
const Edje_File *_edje_file_get(void);

/**
 * Set the current Edje file.
 *
 * @param edf the Edje file to set
 *
 */
void _edje_file_set(const Edje_File *edf);

#endif
