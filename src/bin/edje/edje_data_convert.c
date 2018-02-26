#include "edje_private.h"
#include "edje_convert.h"

Eet_Data_Descriptor *_edje_edd_old_edje_file = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_style = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_style_tag = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_color_class = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_data = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_external_directory = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_external_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_font_directory = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_font_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_image_directory = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_image_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_image_directory_set = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_image_directory_set_entry = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_program = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_program_target = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_program_after = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_part_collection_directory = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_part_collection_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_pack_element = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_part_collection = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_part = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_part_description = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_part_image_id = NULL;
Eet_Data_Descriptor *_edje_edd_old_edje_external_param = NULL;

#define FREED(eed)                      \
  if (eed)                              \
    {                                   \
       eet_data_descriptor_free((eed)); \
       (eed) = NULL;                    \
    }

void
_edje_edd_old_shutdown(void)
{
   FREED(_edje_edd_old_edje_file);
   FREED(_edje_edd_old_edje_style);
   FREED(_edje_edd_old_edje_style_tag);
   FREED(_edje_edd_old_edje_color_class);
   FREED(_edje_edd_old_edje_data);
   FREED(_edje_edd_old_edje_external_directory);
   FREED(_edje_edd_old_edje_external_directory_entry);
   FREED(_edje_edd_old_edje_font_directory);
   FREED(_edje_edd_old_edje_font_directory_entry);
   FREED(_edje_edd_old_edje_image_directory);
   FREED(_edje_edd_old_edje_image_directory_entry);
   FREED(_edje_edd_old_edje_program);
   FREED(_edje_edd_old_edje_program_target);
   FREED(_edje_edd_old_edje_program_after);
   FREED(_edje_edd_old_edje_part_collection_directory);
   FREED(_edje_edd_old_edje_part_collection_directory_entry);
   FREED(_edje_edd_old_edje_pack_element);
   FREED(_edje_edd_old_edje_part_collection);
   FREED(_edje_edd_old_edje_part);
   FREED(_edje_edd_old_edje_part_description);
   FREED(_edje_edd_old_edje_part_image_id);
   FREED(_edje_edd_old_edje_external_param);
   FREED(_edje_edd_old_edje_image_directory_set);
   FREED(_edje_edd_old_edje_image_directory_set_entry);
}

void
_edje_edd_old_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   /* external directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_External_Directory_Entry);
   _edje_edd_old_edje_external_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_external_directory_entry, Edje_External_Directory_Entry, "entry", entry, EET_T_STRING);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc),
                                           "Edje_External_Directory",
                                           sizeof (Old_Edje_External_Directory));
   _edje_edd_old_edje_external_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_external_directory, Edje_External_Directory, "entries", entries, _edje_edd_old_edje_external_directory_entry);

   /* font directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Font_Directory_Entry);
   _edje_edd_old_edje_font_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_font_directory_entry, Edje_Font_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_font_directory_entry, Edje_Font_Directory_Entry, "file", file, EET_T_STRING);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc),
                                           "Edje_Font_Directory",
                                           sizeof (Old_Edje_Font_Directory));
   _edje_edd_old_edje_font_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_font_directory, Old_Edje_Font_Directory, "entries", entries, _edje_edd_old_edje_font_directory_entry);

   /* image directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Image_Directory_Entry);
   _edje_edd_old_edje_image_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_entry, Edje_Image_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_type", source_type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_param", source_param, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_entry, Edje_Image_Directory_Entry, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Image_Directory_Set_Entry);
   _edje_edd_old_edje_image_directory_set_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "min.w", size.min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "min.h", size.min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "max.w", size.max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "max.h", size.max.h, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Image_Directory_Set);
   _edje_edd_old_edje_image_directory_set =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_set, Edje_Image_Directory_Set, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_image_directory_set, Edje_Image_Directory_Set, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_image_directory_set, Edje_Image_Directory_Set, "entries", entries, _edje_edd_old_edje_image_directory_set_entry);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc),
                                           "Edje_Image_Directory",
                                           sizeof (Old_Edje_Image_Directory));
   _edje_edd_old_edje_image_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_image_directory, Old_Edje_Image_Directory, "entries", entries, _edje_edd_old_edje_image_directory_entry);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_image_directory, Old_Edje_Image_Directory, "sets", sets, _edje_edd_old_edje_image_directory_set);

   /* collection directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Collection_Directory_Entry);
   _edje_edd_old_edje_part_collection_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "id", id, EET_T_INT);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc),
                                           "Edje_Part_Collection_Directory",
                                           sizeof (Old_Edje_Part_Collection_Directory));
   _edje_edd_old_edje_part_collection_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_part_collection_directory, Old_Edje_Part_Collection_Directory, "entries", entries, _edje_edd_old_edje_part_collection_directory_entry);

   /* generic data attachment */
   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc),
                                           "Edje_Data", sizeof (Old_Edje_Data));
   _edje_edd_old_edje_data =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_data, Old_Edje_Data, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_data, Old_Edje_Data, "value", value, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Style_Tag);
   _edje_edd_old_edje_style_tag =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_style_tag, Edje_Style_Tag, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_style_tag, Edje_Style_Tag, "value", value, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Style);
   _edje_edd_old_edje_style =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_style, Edje_Style, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_style, Edje_Style, "tags", tags, _edje_edd_old_edje_style_tag);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Color_Class);
   _edje_edd_old_edje_color_class =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "r", r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "g", g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "b", b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "a", a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "r2", r2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "g2", g2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "b2", b2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "a2", a2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "r3", r3, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "g3", g3, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "b3", b3, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_color_class, Edje_Color_Class, "a3", a3, EET_T_UCHAR);

   /* the main file directory */
   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc),
                                           "Edje_File", sizeof (Old_Edje_File));
   _edje_edd_old_edje_file =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_file, Old_Edje_File, "compiler", compiler, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_file, Old_Edje_File, "version", version, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_file, Old_Edje_File, "feature_ver", feature_ver, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_old_edje_file, Old_Edje_File, "external_dir", external_dir, _edje_edd_old_edje_external_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_old_edje_file, Old_Edje_File, "font_dir", font_dir, _edje_edd_old_edje_font_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_old_edje_file, Old_Edje_File, "image_dir", image_dir, _edje_edd_old_edje_image_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_old_edje_file, Old_Edje_File, "collection_dir", collection_dir, _edje_edd_old_edje_part_collection_directory);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_file, Old_Edje_File, "data", data, _edje_edd_old_edje_data);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_file, Old_Edje_File, "styles", styles, _edje_edd_old_edje_style);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_file, Old_Edje_File, "color_classes", color_classes, _edje_edd_old_edje_color_class);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_file, Old_Edje_File, "text_classes", text_classes, _edje_edd_old_edje_text_class);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_file, Old_Edje_File, "size_classes", size_classes, _edje_edd_old_edje_size_class);

   /* parts & programs - loaded induvidually */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Program_Target);
   _edje_edd_old_edje_program_target =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program_target, Edje_Program_Target, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Program_After);
   _edje_edd_old_edje_program_after =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program_after,
                                 Edje_Program_After, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Program);
   _edje_edd_old_edje_program =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "signal", signal, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "filter_part", filter.part, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "filter_state", filter.state, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "in.from", in.from, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "in.range", in.range, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "action", action, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "state", state, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "state2", state2, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "value", value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "value2", value2, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "tween.mode", tween.mode, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "tween.time", tween.time, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_program, Edje_Program, "targets", targets, _edje_edd_old_edje_program_target);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_program, Edje_Program, "after", after, _edje_edd_old_edje_program_after);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "api.name", api.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "api.description", api.description, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "param.src", param.src, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_program, Edje_Program, "param.dst", param.dst, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Image_Id);
   _edje_edd_old_edje_part_image_id =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_image_id, Edje_Part_Image_Id, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_image_id, Edje_Part_Image_Id, "set", set, EET_T_UCHAR);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_External_Param);
   _edje_edd_old_edje_external_param =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_external_param, Edje_External_Param, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_external_param, Edje_External_Param, "type", type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_external_param, Edje_External_Param, "i", i, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_external_param, Edje_External_Param, "d", d, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_external_param, Edje_External_Param, "s", s, EET_T_STRING);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc), "Edje_Part_Description", sizeof (Old_Edje_Part_Description));
   _edje_edd_old_edje_part_description =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "state.name", common.state.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "state.value", common.state.value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "visible", common.visible, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "align.x", common.align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "align.y", common.align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fixed.w", common.fixed.w, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fixed.h", common.fixed.h, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "min.w", common.min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "min.h", common.min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "max.w", common.max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "max.h", common.max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "step.x", common.step.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "step.y", common.step.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "aspect.min", common.aspect.min, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "aspect.max", common.aspect.max, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "aspect.prefer", common.aspect.prefer, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel1.relative_x", common.rel1.relative_x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel1.relative_y", common.rel1.relative_y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel1.offset_x", common.rel1.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel1.offset_y", common.rel1.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel1.id_x", common.rel1.id_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel1.id_y", common.rel1.id_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel2.relative_x", common.rel2.relative_x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel2.relative_y", common.rel2.relative_y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel2.offset_x", common.rel2.offset_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel2.offset_y", common.rel2.offset_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel2.id_x", common.rel2.id_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "rel2.id_y", common.rel2.id_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "image.id", image.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "image.set", image.set, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "image.tween_list", image.tween_list, _edje_edd_old_edje_part_image_id);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "border.l", image.border.l, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "border.r", image.border.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "border.t", image.border.t, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "border.b", image.border.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "border.no_fill", image.border.no_fill, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "border.scale", image.border.scale, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.smooth", image.fill.smooth, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.pos_rel_x", image.fill.pos_rel_x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.pos_abs_x", image.fill.pos_abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.rel_x", image.fill.rel_x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.abs_x", image.fill.abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.pos_rel_y", image.fill.pos_rel_y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.pos_abs_y", image.fill.pos_abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.rel_y", image.fill.rel_y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.abs_y", image.fill.abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.angle", image.fill.angle, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.spread", image.fill.spread, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "fill.type", image.fill.type, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color_class", common.color_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color.r", common.color.r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color.g", common.color.g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color.b", common.color.b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color.a", common.color.a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color2.r", common.color2.r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color2.g", common.color2.g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color2.b", common.color2.b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color2.a", common.color2.a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color3.r", text.color3.r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color3.g", text.color3.g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color3.b", text.color3.b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "color3.a", text.color3.a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.text", text.text.str, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.text_class", text.text_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.style", text.style.str, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.font", text.font.str, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.repch", text.repch.str, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.size", text.size, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.fit_x", text.fit_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.fit_y", text.fit_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.min_x", text.min_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.min_y", text.min_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.max_x", text.max_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.max_y", text.max_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.align.x", text.align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.align.y", text.align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.id_source", text.id_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.id_text_source", text.id_text_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "text.ellipsis", text.ellipsis, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "box.layout", box.layout, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "box.alt_layout", box.alt_layout, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "box.align.x", box.align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "box.align.y", box.align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "box.padding.x", box.padding.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "box.padding.y", box.padding.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "box.min.h", box.min.h, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "box.min.v", box.min.v, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "table.homogeneous", table.homogeneous, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "table.align.x", table.align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "table.align.y", table.align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "table.padding.x", table.padding.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "table.padding.y", table.padding.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.id_persp", common.map.id_persp, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.id_light", common.map.id_light, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.rot.id_center", common.map.rot.id_center, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.rot.x", common.map.rot.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.rot.y", common.map.rot.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.rot.z", common.map.rot.z, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.on", common.map.on, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.smooth", common.map.smooth, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.alpha", common.map.alpha, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.persp_on", common.map.persp_on, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "map.backcull", common.map.backcull, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "persp.zplane", common.persp.zplane, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "persp.focal", common.persp.focal, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "size_class", common.size_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_part_description, Old_Edje_Part_Description, "external_params", external_params, _edje_edd_old_edje_external_param);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Pack_Element);
   _edje_edd_old_edje_pack_element =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "type", type, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "min.w", min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "min.h", min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "prefer.w", prefer.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "prefer.h", prefer.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "max.w", max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "max.h", max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "padding.l", padding.l, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "padding.r", padding.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "padding.t", padding.t, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "padding.b", padding.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "align.x", align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "align.y", align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "weight.x", weight.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "weight.y", weight.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "aspect.w", aspect.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "aspect.h", aspect.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "aspect.mode", aspect.mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "options", options, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "col", col, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "row", row, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "colspan", colspan, EET_T_USHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_pack_element, Edje_Pack_Element, "rowspan", rowspan, EET_T_USHORT);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc), "Edje_Part", sizeof (Old_Edje_Part));
   _edje_edd_old_edje_part =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "type", type, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "effect", effect, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "mouse_events", mouse_events, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "anti_alias", anti_alias, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "repeat_events", repeat_events, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "ignore_flags", ignore_flags, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "mask_flags", mask_flags, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "scale", scale, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "pointer_mode", pointer_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "precise_is_inside", precise_is_inside, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "clip_to_id", clip_to_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "use_alternate_font_metrics", use_alternate_font_metrics, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_old_edje_part, Old_Edje_Part, "default_desc", default_desc, _edje_edd_old_edje_part_description);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_part, Old_Edje_Part, "other_desc", other_desc, _edje_edd_old_edje_part_description);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "dragable.x", dragable.x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "dragable.step_x", dragable.step_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "dragable.count_x", dragable.count_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "dragable.y", dragable.y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "dragable.step_y", dragable.step_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "dragable.count_y", dragable.count_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "dragable.counfine_id", dragable.confine_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "dragable.events_id", dragable.event_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "entry_mode", entry_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "select_mode", select_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "multiline", multiline, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "source2", source2, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "source3", source3, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "source4", source4, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "source5", source5, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "source6", source6, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_part, Old_Edje_Part, "items", items, _edje_edd_old_edje_pack_element);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "api.name", api.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "api.description", api.description, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part, Old_Edje_Part, "required", required, EET_T_UCHAR);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc), "Edje_Part_Collection", sizeof (Old_Edje_Part_Collection));
   _edje_edd_old_edje_part_collection =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "programs", programs, _edje_edd_old_edje_program);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "parts", parts, _edje_edd_old_edje_part);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "data", data, _edje_edd_old_edje_data);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "prop.min.w", prop.min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "prop.min.h", prop.min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "prop.max.w", prop.max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "prop.max.h", prop.max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_old_edje_part_collection, Old_Edje_Part_Collection, "lua_script_only", lua_script_only, EET_T_UCHAR);

   {
      Old_Edje_Part_Collection epc;

      eet_data_descriptor_element_add(_edje_edd_old_edje_part_collection,
                                      "alias", EET_T_STRING, EET_G_HASH,
                                      (char *)(&(epc.alias)) - (char *)(&(epc)),
                                      0, /* 0,  */ NULL, NULL);
   }
}

