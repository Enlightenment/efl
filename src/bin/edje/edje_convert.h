#ifndef EDJE_CONVERT_H__
# define EDJE_CONVERT_H__


typedef struct _Old_Edje_File			Old_Edje_File;
typedef struct _Old_Edje_Image_Directory	Old_Edje_Image_Directory;
typedef struct _Old_Edje_Font_Directory         Old_Edje_Font_Directory;
typedef struct _Old_Edje_External_Directory	Old_Edje_External_Directory;
typedef struct _Old_Edje_Part			Old_Edje_Part;
typedef struct _Old_Edje_Part_Collection	Old_Edje_Part_Collection;
typedef struct _Old_Edje_Part_Collection_Directory Old_Edje_Part_Collection_Directory;
typedef struct _Old_Edje_Part_Description       Old_Edje_Part_Description;
typedef struct _Old_Edje_Part_Description_Spec_Image Old_Edje_Part_Description_Spec_Image;
typedef struct _Old_Edje_Data                   Old_Edje_Data;

struct _Old_Edje_Data
{
   const char *key;
   char *value;
};

/*----------*/

struct _Old_Edje_Font_Directory
{
   Eina_List *entries; /* a list of Edje_Font_Directory_Entry */
};

struct _Old_Edje_Image_Directory
{
   Eina_List *entries; /* a list of Edje_Image_Directory_Entry */
   Eina_List *sets; /* a list of Edje_Image_Directory_Set */
};

struct _Old_Edje_External_Directory
{
   Eina_List *entries; /* a list of Edje_External_Directory_Entry */
};

struct _Old_Edje_File
{
   const char                     *path;
   time_t                          mtime;

   Old_Edje_External_Directory    *external_dir;
   Old_Edje_Font_Directory        *font_dir;
   Old_Edje_Image_Directory       *image_dir;
   Old_Edje_Part_Collection_Directory *collection_dir;
   Eina_List                      *data;
   Eina_List                      *styles;
   Eina_List                      *color_classes;
   Eina_List                      *text_classes;
   Eina_List                      *size_classes;

   const char                     *compiler;
   int                             version;
   int                             feature_ver;
};

struct _Old_Edje_Part_Collection
{
   Eina_List *programs; /* a list of Edje_Program */
   Eina_List *parts; /* a list of Edje_Part */
   Eina_List *data;

   int        id; /* the collection id */

   Eina_Hash *alias; /* aliasing part*/

   struct {
      Edje_Size min, max;
   } prop;

   int        references;
#ifdef EDJE_PROGRAM_CACHE
   struct {
      Eina_Hash                   *no_matches;
      Eina_Hash                   *matches;
   } prog_cache;
#endif

   Embryo_Program   *script; /* all the embryo script code for this group */
   const char       *part;

   unsigned char    script_only;

   unsigned char    lua_script_only;

   unsigned char    checked : 1;
};

struct _Old_Edje_Part
{
   const char            *name; /* the name if any of the part */
   Old_Edje_Part_Description *default_desc; /* the part descriptor for default */
   Eina_List             *other_desc; /* other possible descriptors */
   const char            *source, *source2, *source3, *source4, *source5, *source6;
   int                    id; /* its id number */
   int                    clip_to_id; /* the part id to clip this one to */
   Edje_Part_Dragable     dragable;
   Eina_List             *items; /* packed items for box and table */
   unsigned char          type; /* what type (image, rect, text) */
   unsigned char          effect; /* 0 = plain... */
   unsigned char          mouse_events; /* it will affect/respond to mouse events */
   unsigned char          repeat_events; /* it will repeat events to objects below */
   Evas_Event_Flags       ignore_flags;
   Evas_Event_Flags       mask_flags;
   unsigned char          scale; /* should certain properties scale with edje scale factor? */
   unsigned char          precise_is_inside;
   unsigned char          use_alternate_font_metrics;
   unsigned char          pointer_mode;
   unsigned char          entry_mode;
   unsigned char          select_mode;
   unsigned char          multiline;
   Edje_Part_Api	  api;
   unsigned char          required;
};

struct _Old_Edje_Part_Description_Spec_Image
{
   Eina_List     *tween_list; /* list of Edje_Part_Image_Id */
   int            id; /* the image id to use */
   int            scale_hint; /* evas scale hint */
   Eina_Bool      set; /* if image condition it's content */

   Edje_Part_Description_Spec_Border border;
   Edje_Part_Description_Spec_Fill   fill;
};

struct _Old_Edje_Part_Description
{
   Edje_Part_Description_Common common;
   Old_Edje_Part_Description_Spec_Image image;
   Edje_Part_Description_Spec_Text text;
   Edje_Part_Description_Spec_Box box;
   Edje_Part_Description_Spec_Table table;

   Eina_List *external_params; /* parameters for external objects */
};

struct _Old_Edje_Part_Collection_Directory
{
   Eina_List *entries; /* a list of Edje_Part_Collection_Directory_Entry */

   int        references;
};

Edje_File *_edje_file_convert(Eet_File *ef, Old_Edje_File *oedf);
Edje_Part_Collection *_edje_collection_convert(Eet_File *ef,
					       Edje_Part_Collection_Directory_Entry *ce,
					       Old_Edje_Part_Collection *oedc);
Edje_Part_Description_Common *_edje_description_convert(int type,
							Edje_Part_Collection_Directory_Entry *ce,
							Old_Edje_Part_Description *oed);
const Edje_File *_edje_file_get(void);
void _edje_file_set(const Edje_File *edf);

#endif
