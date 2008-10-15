/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

EAPI Eet_Data_Descriptor *_edje_edd_edje_file = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_style = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_style_tag = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_color_class = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_data = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_font_directory = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_font_directory_entry = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_image_directory = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_image_directory_entry = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_spectrum_directory = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_spectrum_directory_entry = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_spectrum_color = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_program = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_program_target = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_program_after = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory_entry = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_part_collection = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_part = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_part_description = NULL;
EAPI Eet_Data_Descriptor *_edje_edd_edje_part_image_id = NULL;

#define NEWD(str, typ) \
   { eddc.name = str; eddc.size = sizeof(typ); }

#define FREED(eed) \
   if (eed) \
   { \
      eet_data_descriptor_free((eed)); \
      (eed) = NULL; \
   }

void
_edje_edd_free(void)
{
   FREED(_edje_edd_edje_file);
   FREED(_edje_edd_edje_style);
   FREED(_edje_edd_edje_style_tag);
   FREED(_edje_edd_edje_color_class);
   FREED(_edje_edd_edje_data);
   FREED(_edje_edd_edje_font_directory);
   FREED(_edje_edd_edje_font_directory_entry);
   FREED(_edje_edd_edje_image_directory);
   FREED(_edje_edd_edje_image_directory_entry);
   FREED(_edje_edd_edje_spectrum_directory);
   FREED(_edje_edd_edje_spectrum_directory_entry);
   FREED(_edje_edd_edje_spectrum_color);
   FREED(_edje_edd_edje_program);
   FREED(_edje_edd_edje_program_target);
   FREED(_edje_edd_edje_program_after);
   FREED(_edje_edd_edje_part_collection_directory);
   FREED(_edje_edd_edje_part_collection_directory_entry);
   FREED(_edje_edd_edje_part_collection);
   FREED(_edje_edd_edje_part);
   FREED(_edje_edd_edje_part_description);
   FREED(_edje_edd_edje_part_image_id);
}

static char *
_edje_str_direct_alloc(const char *str)
{
   return (char *)str;
}

static void
_edje_str_direct_free(const char *str)
{
}

void
_edje_edd_setup(void)
{
   Eet_Data_Descriptor_Class eddc;

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.mem_alloc = NULL;
   eddc.func.mem_free = NULL;
   eddc.func.str_alloc = (char *(*)(const char *))eina_stringshare_add;
   eddc.func.str_free = eina_stringshare_del;
   eddc.func.list_next = (void *(*)(void *))evas_list_next;
   eddc.func.list_append = (void *(*)(void *, void *))evas_list_append;
   eddc.func.list_data = (void *(*)(void *))evas_list_data;
   eddc.func.list_free = (void *(*)(void *))evas_list_free;
   eddc.func.hash_foreach = (void (*)(void *, int (*)(void *, const char *, void *, void *), void *))evas_hash_foreach;
   eddc.func.hash_add = (void *(*)(void *, const char *, void *))evas_hash_add;
   eddc.func.hash_free = (void (*)(void *))evas_hash_free;
   eddc.func.str_direct_alloc = _edje_str_direct_alloc;
   eddc.func.str_direct_free = _edje_str_direct_free;

   /* font directory */
   NEWD("Edje_Font_Directory_Entry",
	Edje_Font_Directory_Entry);
   _edje_edd_edje_font_directory_entry =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_font_directory_entry, Edje_Font_Directory_Entry, "entry", entry, EET_T_STRING);

   NEWD("Edje_Font_Directory",
	Edje_Font_Directory);
   _edje_edd_edje_font_directory =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_font_directory, Edje_Font_Directory, "entries", entries, _edje_edd_edje_font_directory_entry);

   /* image directory */
   NEWD("Edje_Image_Directory_Entry",
	Edje_Image_Directory_Entry);
   _edje_edd_edje_image_directory_entry =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_type", source_type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_param", source_param, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "id", id, EET_T_INT);

   NEWD("Edje_Image_Directory",
	Edje_Image_Directory);
   _edje_edd_edje_image_directory =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_image_directory, Edje_Image_Directory, "entries", entries, _edje_edd_edje_image_directory_entry);

   /* spectrum directory */
   NEWD("Edje_Spectrum_Color",
	Edje_Spectrum_Color);
   _edje_edd_edje_spectrum_color =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_spectrum_color, Edje_Spectrum_Color, "r", r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_spectrum_color, Edje_Spectrum_Color, "g", g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_spectrum_color, Edje_Spectrum_Color, "b", b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_spectrum_color, Edje_Spectrum_Color, "a", a, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_spectrum_color, Edje_Spectrum_Color, "d", d, EET_T_INT);

   NEWD("Edje_Spectrum_Directory_Entry",
	Edje_Spectrum_Directory_Entry);
   _edje_edd_edje_spectrum_directory_entry =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_spectrum_directory_entry, Edje_Spectrum_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_spectrum_directory_entry, Edje_Spectrum_Directory_Entry, "filename", filename, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_spectrum_directory_entry, Edje_Spectrum_Directory_Entry, "color_list", color_list, _edje_edd_edje_spectrum_color);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_spectrum_directory_entry, Edje_Spectrum_Directory_Entry, "id", id, EET_T_INT);

   NEWD("Edje_Spectrum_Directory",
	Edje_Spectrum_Directory);
   _edje_edd_edje_spectrum_directory =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_spectrum_directory, Edje_Spectrum_Directory, "entries", entries, _edje_edd_edje_spectrum_directory_entry);

   /* collection directory */
   NEWD("Edje_Part_Collection_Directory_Entry",
	Edje_Part_Collection_Directory_Entry);
   _edje_edd_edje_part_collection_directory_entry =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "id", id, EET_T_INT);

   NEWD("Edje_Part_Collection_Directory",
	Edje_Part_Collection_Directory);
   _edje_edd_edje_part_collection_directory =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection_directory, Edje_Part_Collection_Directory, "entries", entries, _edje_edd_edje_part_collection_directory_entry);

   /* generic data attachment */
   NEWD("Edje_Data",
	Edje_Data);
   _edje_edd_edje_data =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_data, Edje_Data, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_data, Edje_Data, "value", value, EET_T_STRING);

   NEWD("Edje_Style_Tag",
	Edje_Style_Tag);
   _edje_edd_edje_style_tag =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style_tag, Edje_Style_Tag, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style_tag, Edje_Style_Tag, "value", value, EET_T_STRING);

   NEWD("Edje_Style",
	Edje_Style);
   _edje_edd_edje_style =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style, Edje_Style, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_style, Edje_Style, "tags", tags, _edje_edd_edje_style_tag);

   NEWD("Edje_Color_Class",
	Edje_Color_Class);
   _edje_edd_edje_color_class =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "r", r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "g", g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "b", b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "a", a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "r2", r2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "g2", g2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "b2", b2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "a2", a2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "r3", r3, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "g3", g3, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "b3", b3, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "a3", a3, EET_T_UCHAR);

   /* the main file directory */
   NEWD("Edje_File",
	Edje_File);
   _edje_edd_edje_file =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "compiler", compiler, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "version", version, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "feature_ver", feature_ver, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "font_dir", font_dir, _edje_edd_edje_font_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "image_dir", image_dir, _edje_edd_edje_image_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "spectrum_dir", spectrum_dir, _edje_edd_edje_spectrum_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "collection_dir", collection_dir, _edje_edd_edje_part_collection_directory);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "data", data, _edje_edd_edje_data);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "styles", styles, _edje_edd_edje_style);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "color_classes", color_classes, _edje_edd_edje_color_class);

   /* parts & programs - loaded induvidually */
   NEWD("Edje_Program_Target",
	Edje_Program_Target);
   _edje_edd_edje_program_target =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program_target, Edje_Program_Target, "id", id, EET_T_INT);

   NEWD("Edje_Program_After",
	Edje_Program_After);
   _edje_edd_edje_program_after =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program_after,
                                 Edje_Program_After, "id", id, EET_T_INT);

   NEWD("Edje_Program",
	Edje_Program);
   _edje_edd_edje_program =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "signal", signal, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "in.from", in.from, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "in.range", in.range, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "action", action, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "state", state, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "state2", state2, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "value", value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "value2", value2, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "tween.mode", tween.mode, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "tween.time", tween.time, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_program, Edje_Program, "targets", targets, _edje_edd_edje_program_target);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_program, Edje_Program, "after", after, _edje_edd_edje_program_after);

   NEWD("Edje_Part_Image_Id",
	Edje_Part_Image_Id);
   _edje_edd_edje_part_image_id =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_image_id, Edje_Part_Image_Id, "id", id, EET_T_INT);

   NEWD("Edje_Part_Description",
	Edje_Part_Description);
   _edje_edd_edje_part_description =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "state.name", state.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "state.value", state.value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "visible", visible, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "align.x", align.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "align.y", align.y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fixed.w", fixed.w, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fixed.h", fixed.h, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "min.w", min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "min.h", min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "max.w", max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "max.h", max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "step.x", step.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "step.y", step.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "aspect.min", aspect.min, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "aspect.max", aspect.max, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "aspect.prefer", aspect.prefer, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.relative_x", rel1.relative_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.relative_y", rel1.relative_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.offset_x", rel1.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.offset_y", rel1.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.id_x", rel1.id_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel1.id_y", rel1.id_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.relative_x", rel2.relative_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.relative_y", rel2.relative_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.offset_x", rel2.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.offset_y", rel2.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.id_x", rel2.id_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "rel2.id_y", rel2.id_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "image.id", image.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_description, Edje_Part_Description, "image.tween_list", image.tween_list, _edje_edd_edje_part_image_id);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.l", border.l, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.r", border.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.t", border.t, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.b", border.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "border.no_fill", border.no_fill, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.smooth", fill.smooth, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.pos_rel_x", fill.pos_rel_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.pos_abs_x", fill.pos_abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.rel_x", fill.rel_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.abs_x", fill.abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.pos_rel_y", fill.pos_rel_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.pos_abs_y", fill.pos_abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.rel_y", fill.rel_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.abs_y", fill.abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.angle", fill.angle, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.spread", fill.spread, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "fill.type", fill.type, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color_class", color_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color.r", color.r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color.g", color.g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color.b", color.b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color.a", color.a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color2.r", color2.r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color2.g", color2.g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color2.b", color2.b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color2.a", color2.a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color3.r", color3.r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color3.g", color3.g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color3.b", color3.b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "color3.a", color3.a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.text", text.text, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.text_class", text.text_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.style", text.style, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.font", text.font, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.size", text.size, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.fit_x", text.fit_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.fit_y", text.fit_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.min_x", text.min_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.min_y", text.min_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.max_x", text.max_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.max_y", text.max_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.align.x", text.align.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.align.y", text.align.y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.id_source", text.id_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.id_text_source", text.id_text_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.elipsis", text.elipsis, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.id", gradient.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.type", gradient.type, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.use_rel", gradient.use_rel, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.rel1.relative_x", gradient.rel1.relative_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.rel1.relative_y", gradient.rel1.relative_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.rel1.offset_x", gradient.rel1.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.rel1.offset_y", gradient.rel1.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.rel2.relative_x", gradient.rel2.relative_x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.rel2.relative_y", gradient.rel2.relative_y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.rel2.offset_x", gradient.rel2.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "gradient.rel2.offset_y", gradient.rel2.offset_y, EET_T_INT);

   NEWD("Edje_Part",
	Edje_Part);
   _edje_edd_edje_part =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "type", type, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "effect", effect, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "mouse_events", mouse_events, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "repeat_events", repeat_events, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "ignore_flags", ignore_flags, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "scale", scale, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "pointer_mode", pointer_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "precise_is_inside", precise_is_inside, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "clip_to_id", clip_to_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "use_alternate_font_metrics", use_alternate_font_metrics, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_part, Edje_Part, "default_desc", default_desc, _edje_edd_edje_part_description);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part, Edje_Part, "other_desc", other_desc, _edje_edd_edje_part_description);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.x", dragable.x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.step_x", dragable.step_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.count_x", dragable.count_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.y", dragable.y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.step_y", dragable.step_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.count_y", dragable.count_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.counfine_id", dragable.confine_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.events_id", dragable.events_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "entry_mode", entry_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "multiline", multiline, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source2", source2, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source3", source3, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source4", source4, EET_T_STRING);

   NEWD("Edje_Part_Collection",
	Edje_Part_Collection);
   _edje_edd_edje_part_collection  =
     eet_data_descriptor3_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection, Edje_Part_Collection, "programs", programs, _edje_edd_edje_program);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection, Edje_Part_Collection, "parts", parts, _edje_edd_edje_part);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection, Edje_Part_Collection, "data", data, _edje_edd_edje_data);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.min.w", prop.min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.min.h", prop.min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.max.w", prop.max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.max.h", prop.max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "script_only", script_only, EET_T_UCHAR);
}
