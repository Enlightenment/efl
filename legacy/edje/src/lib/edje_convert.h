#ifndef EDJE_CONVERT_H__
# define EDJE_CONVERT_H__

typedef struct _Edje_Part_Collection Old_Edje_Part_Collection;

struct _Old_Edje_File
{
   const char                     *path;
   time_t                          mtime;

   Edje_External_Directory        *external_dir;
   Edje_Font_Directory            *font_dir;
   Edje_Image_Directory           *image_dir;
   Edje_Spectrum_Directory        *spectrum_dir;
   Edje_Part_Collection_Directory *collection_dir;
   Eina_List                      *data;
   Eina_List                      *styles;
   Eina_List                      *color_classes;

   char                           *compiler;
   int                             version;
   int                             feature_ver;
};

Edje_File *_edje_file_convert(Eet_File *file, Old_Edje_File *oedf);
Edje_Part_Collection *_edje_collection_convert(Edje_File *file,
					       Old_Edje_Part_Collection *oedc);
const Edje_File *_edje_file_get(void);
void _edje_file_set(const Edje_File *edf);

#endif
