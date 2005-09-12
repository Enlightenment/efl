#include "Edje.h"
#include "edje_private.h"

Eet_Data_Descriptor *_edje_edd_edje_file = NULL;
Eet_Data_Descriptor *_edje_edd_edje_style = NULL;
Eet_Data_Descriptor *_edje_edd_edje_style_tag = NULL;
Eet_Data_Descriptor *_edje_edd_edje_data = NULL;
Eet_Data_Descriptor *_edje_edd_edje_font_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_font_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program_target = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program_after = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_collection = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_image_id = NULL;

#define NEWD(str, typ) \
   eet_data_descriptor_new(str, sizeof(typ), \
			      (void *(*) (void *))evas_list_next, \
			      (void *(*) (void *, void *))evas_list_append, \
			      (void *(*) (void *))evas_list_data, \
			      (void *(*) (void *))evas_list_free, \
			      (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach, \
			      (void *(*) (void *, const char *, void *))evas_hash_add, \
			      (void  (*) (void *))evas_hash_free)

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
   FREED(_edje_edd_edje_data);
   FREED(_edje_edd_edje_font_directory);
   FREED(_edje_edd_edje_font_directory_entry);
   FREED(_edje_edd_edje_image_directory);
   FREED(_edje_edd_edje_image_directory_entry);
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

void
_edje_edd_setup(void)
{
   /* font directory */
   _edje_edd_edje_font_directory_entry = 
    NEWD("Edje_Font_Directory_Entry", 
	 Edje_Font_Directory_Entry);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_font_directory_entry, Edje_Font_Directory_Entry, "entry", entry, EET_T_STRING);
   
   _edje_edd_edje_font_directory = 
     NEWD("Edje_Font_Directory", 
	  Edje_Font_Directory);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_font_directory, Edje_Font_Directory, "entries", entries, _edje_edd_edje_font_directory_entry);

   /* image directory */
   _edje_edd_edje_image_directory_entry = 
    NEWD("Edje_Image_Directory_Entry", 
	 Edje_Image_Directory_Entry);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_type", source_type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_param", source_param, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "id", id, EET_T_INT);
   
   _edje_edd_edje_image_directory = 
     NEWD("Edje_Image_Directory", 
	  Edje_Image_Directory);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_image_directory, Edje_Image_Directory, "entries", entries, _edje_edd_edje_image_directory_entry);

   /* collection directory */
   _edje_edd_edje_part_collection_directory_entry = 
     NEWD("Edje_Part_Collection_Directory_Entry",
	  Edje_Part_Collection_Directory_Entry);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "id", id, EET_T_INT);
   
   _edje_edd_edje_part_collection_directory = 
     NEWD("Edje_Part_Collection_Directory",
	  Edje_Part_Collection_Directory);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection_directory, Edje_Part_Collection_Directory, "entries", entries, _edje_edd_edje_part_collection_directory_entry);

   /* generic data attachment */
   _edje_edd_edje_data =
     NEWD("Edje_Data",
	  Edje_Data);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_data, Edje_Data, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_data, Edje_Data, "value", value, EET_T_STRING);
   
   _edje_edd_edje_style_tag = 
     NEWD("Edje_Style_Tag", 
	  Edje_Style_Tag);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style_tag, Edje_Style_Tag, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style_tag, Edje_Style_Tag, "value", value, EET_T_STRING);
   
   _edje_edd_edje_style = 
     NEWD("Edje_Style", 
	  Edje_Style);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style, Edje_Style, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_style, Edje_Style, "tags", tags, _edje_edd_edje_style_tag);
   
   /* the main file directory */
   _edje_edd_edje_file = 
     NEWD("Edje_File", 
	  Edje_File);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "compiler", compiler, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "version", version, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "feature_ver", feature_ver, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "font_dir", font_dir, _edje_edd_edje_font_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "image_dir", image_dir, _edje_edd_edje_image_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "collection_dir", collection_dir, _edje_edd_edje_part_collection_directory);   
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "data", data, _edje_edd_edje_data);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "styles", styles, _edje_edd_edje_style);

   /* parts & programs - loaded induvidually */
   _edje_edd_edje_program_target = 
     NEWD("Edje_Program_Target",
	  Edje_Program_Target);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program_target, Edje_Program_Target, "id", id, EET_T_INT);

   _edje_edd_edje_program_after =
     NEWD("Edje_Program_After", Edje_Program_After);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program_after,
                                 Edje_Program_After, "id", id, EET_T_INT);

   _edje_edd_edje_program = 
     NEWD("Edje_Program",
	  Edje_Program);
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

   _edje_edd_edje_part_image_id = 
     NEWD("Edje_Part_Image_Id",
	  Edje_Part_Image_Id);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_image_id, Edje_Part_Image_Id, "id", id, EET_T_INT);
   
   _edje_edd_edje_part_description = 
     NEWD("Edje_Part_Description",
	  Edje_Part_Description);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "state.name", state.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "state.value", state.value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "visible", visible, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "align.x", align.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "align.y", align.y, EET_T_DOUBLE);
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
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.align.x", text.align.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.align.y", text.align.y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.id_source", text.id_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description, Edje_Part_Description, "text.id_text_source", text.id_text_source, EET_T_INT);
   
   _edje_edd_edje_part = 
     NEWD("Edje_Part",
	  Edje_Part);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "type", type, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "effect", effect, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "mouse_events", mouse_events, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "repeat_events", repeat_events, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "clip_to_id", clip_to_id, EET_T_INT);
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
   
   _edje_edd_edje_part_collection  = 
     NEWD("Edje_Part_Collection", 
	  Edje_Part_Collection);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection, Edje_Part_Collection, "programs", programs, _edje_edd_edje_program);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection, Edje_Part_Collection, "parts", parts, _edje_edd_edje_part);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_collection, Edje_Part_Collection, "data", data, _edje_edd_edje_data);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.min.w", prop.min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.min.h", prop.min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.max.w", prop.max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.max.h", prop.max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "id", id, EET_T_INT);
}
