#include "edje_private.h"

EAPI Eet_Data_Descriptor * _edje_edd_edje_file = NULL;
EAPI Eet_Data_Descriptor * _edje_edd_edje_part_collection = NULL;

Eet_Data_Descriptor *_edje_edd_edje_string = NULL;
Eet_Data_Descriptor *_edje_edd_edje_style = NULL;
Eet_Data_Descriptor *_edje_edd_edje_style_tag = NULL;
Eet_Data_Descriptor *_edje_edd_edje_color_tree_node = NULL;
Eet_Data_Descriptor *_edje_edd_edje_color_class = NULL;
Eet_Data_Descriptor *_edje_edd_edje_text_class = NULL;
Eet_Data_Descriptor *_edje_edd_edje_size_class = NULL;
Eet_Data_Descriptor *_edje_edd_edje_external_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_external_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_font_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_id_hash = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_directory_set = NULL;
Eet_Data_Descriptor *_edje_edd_edje_image_directory_set_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_vector_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_model_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_model_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_limit = NULL;
Eet_Data_Descriptor *_edje_edd_edje_limit_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_sound_sample = NULL;
Eet_Data_Descriptor *_edje_edd_edje_translation_file = NULL;
Eet_Data_Descriptor *_edje_edd_edje_sound_tone = NULL;
Eet_Data_Descriptor *_edje_edd_edje_sound_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_mo_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_vibration_sample = NULL;
Eet_Data_Descriptor *_edje_edd_edje_vibration_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program_target = NULL;
Eet_Data_Descriptor *_edje_edd_edje_program_after = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory_entry = NULL;
Eet_Data_Descriptor *_edje_edd_edje_pack_element = NULL;
Eet_Data_Descriptor *_edje_edd_edje_pack_element_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_allowed_seat = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_allowed_seat_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_variant = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_rectangle = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_snapshot = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_spacer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_swallow = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_group = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_image = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_proxy = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_text = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_textblock = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_box = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_table = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_external = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_3d_color = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_3d_offset = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_3d_vec = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_mesh_node = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_light = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_camera = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_variant_list = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_rectangle_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_snapshot_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_spacer_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_swallow_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_group_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_image_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_proxy_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_text_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_textblock_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_box_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_table_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_external_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_mesh_node_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_light_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_camera_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_filter_data = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_image_id = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_image_id_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_external_param = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_limit = NULL;
Eet_Data_Descriptor *_edje_edd_edje_physics_face = NULL;
Eet_Data_Descriptor *_edje_edd_edje_map_colors = NULL;
Eet_Data_Descriptor *_edje_edd_edje_map_colors_pointer = NULL;
Eet_Data_Descriptor *_edje_edd_edje_filter = NULL;
Eet_Data_Descriptor *_edje_edd_edje_filter_directory = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_vector = NULL;
Eet_Data_Descriptor *_edje_edd_edje_part_description_vector_pointer = NULL;


/* allocate a description struct.
 * this initializes clip_to_id as this field will not be present in most
 * edje files.
 */
#define EMP(Type, Minus)                            \
  EAPI Eina_Mempool *_emp_##Type = NULL;            \
                                                    \
  static void *                                     \
  mem_alloc_##Minus(size_t size)                    \
  {                                                 \
     Edje_Part_Description_Common *data;            \
                                                    \
     data = eina_mempool_malloc(_emp_##Type, size); \
     memset(data, 0, size);                         \
     data->clip_to_id = -1;                         \
     data->map.zoom.x = data->map.zoom.y = 1.0;     \
     return data;                                   \
  }                                                 \
                                                    \
  static void                                       \
    mem_free_##Minus(void *data)                    \
  {                                                 \
     eina_mempool_free(_emp_##Type, data);          \
  }

EMP(RECTANGLE, rectangle)
EMP(TEXT, text)
EMP(IMAGE, image)
EMP(PROXY, proxy)
EMP(SWALLOW, swallow)
EMP(TEXTBLOCK, textblock)
EMP(GROUP, group)
EMP(BOX, box)
EMP(TABLE, table)
EMP(EXTERNAL, external)
EMP(SPACER, spacer)
EMP(MESH_NODE, mesh_node)
EMP(LIGHT, light)
EMP(CAMERA, camera)
EMP(SNAPSHOT, snapshot)
EMP(VECTOR, vector)
#undef EMP

EAPI Eina_Mempool *_emp_part = NULL;

static void *
mem_alloc_part(size_t size)
{
   Edje_Part *ep;

   ep = eina_mempool_malloc(_emp_part, size);
   memset(ep, 0, size);
   // This value need to be defined for older file that didn't provide it
   // as it should -1 by default instead of 0.
   ep->dragable.threshold_id = -1;
   return ep;
}

static void
mem_free_part(void *data)
{
   eina_mempool_free(_emp_part, data);
}

#define FREED(eed)                      \
  if (eed)                              \
    {                                   \
       eet_data_descriptor_free((eed)); \
       (eed) = NULL;                    \
    }

struct
{
   Edje_Part_Type type;
   const char    *name;
} variant_convertion[] = {
   { EDJE_PART_TYPE_RECTANGLE, "rectangle" },
   { EDJE_PART_TYPE_SWALLOW, "swallow" },
   { EDJE_PART_TYPE_GROUP, "group" },
   { EDJE_PART_TYPE_IMAGE, "image" },
   { EDJE_PART_TYPE_TEXT, "text" },
   { EDJE_PART_TYPE_TEXTBLOCK, "textblock" },
   { EDJE_PART_TYPE_BOX, "box" },
   { EDJE_PART_TYPE_TABLE, "table" },
   { EDJE_PART_TYPE_EXTERNAL, "external" },
   { EDJE_PART_TYPE_PROXY, "proxy" },
   { EDJE_PART_TYPE_SPACER, "spacer" },
   { EDJE_PART_TYPE_MESH_NODE, "mesh_node" },
   { EDJE_PART_TYPE_LIGHT, "light" },
   { EDJE_PART_TYPE_CAMERA, "camera" },
   { EDJE_PART_TYPE_SNAPSHOT, "snapshot" },
   { EDJE_PART_TYPE_VECTOR, "vector" }
};

static const char *
_edje_description_variant_type_get(const void *data, Eina_Bool *unknow EINA_UNUSED)
{
   const unsigned char *type;
   unsigned int i;

   type = data;

   for (i = 0; i < (sizeof (variant_convertion) / sizeof (variant_convertion[0])); ++i)
     if (*type == variant_convertion[i].type)
       return variant_convertion[i].name;

   return NULL;
}

static Eina_Bool
_edje_description_variant_type_set(const char *type, void *data, Eina_Bool unknow EINA_UNUSED)
{
   unsigned char *dt;
   unsigned int i;

   dt = data;

   for (i = 0; i < (sizeof (variant_convertion) / sizeof (variant_convertion[0])); ++i)
     if (!strcmp(variant_convertion[i].name, type))
       {
          *dt = variant_convertion[i].type;
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

size_t  _edje_data_string_mapping_size = 0;
void   *_edje_data_string_mapping = NULL;

static Eina_Hash *
_edje_eina_hash_add_alloc(Eina_Hash *hash,
                          const char *key,
                          void *data)
{
   if (!hash)
     hash = eina_hash_string_small_new(free);

   if (!hash)
     return NULL;

   // XXX: ancient edje file workaround
   if ((_edje_data_string_mapping) &&
       ((key >=  (char *)_edje_data_string_mapping) &&
        (key <  ((char *)_edje_data_string_mapping + _edje_data_string_mapping_size))))
     eina_hash_direct_add(hash, key, data);
   else
     eina_hash_add(hash, key, data);
   return hash;
}

// FIXME: remove EAPI when edje_convert goes
EAPI void
_edje_edd_shutdown(void)
{
   FREED(_edje_edd_edje_string);
   FREED(_edje_edd_edje_style);
   FREED(_edje_edd_edje_style_tag);
   FREED(_edje_edd_edje_color_class);
   FREED(_edje_edd_edje_text_class);
   FREED(_edje_edd_edje_size_class);
   FREED(_edje_edd_edje_external_directory);
   FREED(_edje_edd_edje_external_directory_entry);
   FREED(_edje_edd_edje_font_directory_entry);
   FREED(_edje_edd_edje_image_id_hash);
   FREED(_edje_edd_edje_image_directory);
   FREED(_edje_edd_edje_image_directory_entry);
   FREED(_edje_edd_edje_image_directory_set);
   FREED(_edje_edd_edje_image_directory_set_entry);
   FREED(_edje_edd_edje_vector_directory_entry);
   FREED(_edje_edd_edje_model_directory);
   FREED(_edje_edd_edje_model_directory_entry);
   FREED(_edje_edd_edje_limit);
   FREED(_edje_edd_edje_limit_pointer);
   FREED(_edje_edd_edje_sound_sample);
   FREED(_edje_edd_edje_translation_file);
   FREED(_edje_edd_edje_sound_tone);
   FREED(_edje_edd_edje_sound_directory);
   FREED(_edje_edd_edje_mo_directory);
   FREED(_edje_edd_edje_vibration_sample);
   FREED(_edje_edd_edje_vibration_directory);
   FREED(_edje_edd_edje_filter);
   FREED(_edje_edd_edje_filter_directory);
   FREED(_edje_edd_edje_program);
   FREED(_edje_edd_edje_program_pointer);
   FREED(_edje_edd_edje_program_target);
   FREED(_edje_edd_edje_program_after);
   FREED(_edje_edd_edje_part_collection_directory_entry);
   FREED(_edje_edd_edje_pack_element);
   FREED(_edje_edd_edje_pack_element_pointer);
   FREED(_edje_edd_edje_part);
   FREED(_edje_edd_edje_part_pointer);
   FREED(_edje_edd_edje_part_allowed_seat);
   FREED(_edje_edd_edje_part_allowed_seat_pointer);
   FREED(_edje_edd_edje_part_description_variant);
   FREED(_edje_edd_edje_part_description_rectangle);
   FREED(_edje_edd_edje_part_description_snapshot);
   FREED(_edje_edd_edje_part_description_spacer);
   FREED(_edje_edd_edje_part_description_swallow);
   FREED(_edje_edd_edje_part_description_group);
   FREED(_edje_edd_edje_part_description_image);
   FREED(_edje_edd_edje_part_description_proxy);
   FREED(_edje_edd_edje_part_description_text);
   FREED(_edje_edd_edje_part_description_textblock);
   FREED(_edje_edd_edje_part_description_box);
   FREED(_edje_edd_edje_part_description_table);
   FREED(_edje_edd_edje_part_description_external);
   FREED(_edje_edd_edje_part_description_mesh_node);
   FREED(_edje_edd_edje_part_description_light);
   FREED(_edje_edd_edje_part_description_camera);
   FREED(_edje_edd_edje_part_description_3d_color);
   FREED(_edje_edd_edje_part_description_3d_offset);
   FREED(_edje_edd_edje_part_description_3d_vec);
   FREED(_edje_edd_edje_part_description_variant_list);
   FREED(_edje_edd_edje_part_description_rectangle_pointer);
   FREED(_edje_edd_edje_part_description_snapshot_pointer);
   FREED(_edje_edd_edje_part_description_spacer_pointer);
   FREED(_edje_edd_edje_part_description_swallow_pointer);
   FREED(_edje_edd_edje_part_description_group_pointer);
   FREED(_edje_edd_edje_part_description_image_pointer);
   FREED(_edje_edd_edje_part_description_proxy_pointer);
   FREED(_edje_edd_edje_part_description_text_pointer);
   FREED(_edje_edd_edje_part_description_textblock_pointer);
   FREED(_edje_edd_edje_part_description_box_pointer);
   FREED(_edje_edd_edje_part_description_table_pointer);
   FREED(_edje_edd_edje_part_description_external_pointer);
   FREED(_edje_edd_edje_part_description_mesh_node_pointer);
   FREED(_edje_edd_edje_part_description_light_pointer);
   FREED(_edje_edd_edje_part_description_camera_pointer);
   FREED(_edje_edd_edje_part_description_filter_data);
   FREED(_edje_edd_edje_part_image_id);
   FREED(_edje_edd_edje_part_image_id_pointer);
   FREED(_edje_edd_edje_external_param);
   FREED(_edje_edd_edje_part_limit);
   FREED(_edje_edd_edje_physics_face);
   FREED(_edje_edd_edje_map_colors);
   FREED(_edje_edd_edje_map_colors_pointer);

   FREED(_edje_edd_edje_file);
   FREED(_edje_edd_edje_part_collection);
}

#define EDJE_DEFINE_POINTER_TYPE(Type, Name)                                                                                         \
  {                                                                                                                                  \
     typedef struct _Edje_##Type##_Pointer Edje_##Type## _Pointer;                                                                   \
     struct _Edje_##Type##_Pointer                                                                                                   \
     {                                                                                                                               \
        Edje_##Type * pointer;                                                                                                       \
     };                                                                                                                              \
                                                                                                                                     \
     EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_##Type##_Pointer);                                                          \
     _edje_edd_edje_##Name##_pointer =                                                                                               \
       eet_data_descriptor_file_new(&eddc);                                                                                          \
     EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_##Name##_pointer, Edje_##Type##_Pointer, "pointer", pointer, _edje_edd_edje_##Name); \
  }

// FIXME: remove EAPI when edje_convert goes
EAPI void
_edje_edd_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   /* localisable string */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_String);
   _edje_edd_edje_string = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_string, Edje_String, "str", str, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_string, Edje_String, "id", id, EET_T_UINT);

   /* external directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_External_Directory_Entry);
   _edje_edd_edje_external_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_external_directory_entry, Edje_External_Directory_Entry, "entry", entry, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_External_Directory);
   _edje_edd_edje_external_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_external_directory, Edje_External_Directory, "entries", entries, _edje_edd_edje_external_directory_entry);

   /* font directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Font_Directory_Entry);
   _edje_edd_edje_font_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_font_directory_entry, Edje_Font_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_font_directory_entry, Edje_Font_Directory_Entry, "file", file, EET_T_STRING);

   /* image hash */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Image_Hash);
   _edje_edd_edje_image_id_hash = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_id_hash, Edje_Image_Hash, "id", id, EET_T_INT);

   /* image directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Image_Directory_Entry);
   _edje_edd_edje_image_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_type", source_type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "source_param", source_param, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_entry, Edje_Image_Directory_Entry, "external_id", external_id, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Image_Directory_Set_Entry);
   _edje_edd_edje_image_directory_set_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "min.w", size.min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "min.h", size.min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "max.w", size.max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "max.h", size.max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "border.l", border.l, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "border.r", border.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "border.t", border.t, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "border.b", border.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set_entry, Edje_Image_Directory_Set_Entry, "border.scale_by", border.scale_by, EDJE_T_FLOAT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Image_Directory_Set);
   _edje_edd_edje_image_directory_set =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set, Edje_Image_Directory_Set, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_image_directory_set, Edje_Image_Directory_Set, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_image_directory_set, Edje_Image_Directory_Set, "entries", entries, _edje_edd_edje_image_directory_set_entry);

   /* vector directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Vector_Directory_Entry);
   _edje_edd_edje_vector_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_vector_directory_entry, Edje_Vector_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_vector_directory_entry, Edje_Vector_Directory_Entry, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Image_Directory);
   _edje_edd_edje_image_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_image_directory, Edje_Image_Directory, "entries", entries, _edje_edd_edje_image_directory_entry);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_image_directory, Edje_Image_Directory, "sets", sets, _edje_edd_edje_image_directory_set);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_image_directory, Edje_Image_Directory, "vectors", vectors, _edje_edd_edje_vector_directory_entry);

   /*MO*/

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Mo);
   _edje_edd_edje_translation_file = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_translation_file, Edje_Mo, "locale", locale, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_translation_file, Edje_Mo, "mo_src", mo_src, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_translation_file, Edje_Mo, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Mo_Directory);
   _edje_edd_edje_mo_directory = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_mo_directory, Edje_Mo_Directory, "mo_entries", mo_entries, _edje_edd_edje_translation_file);

   /* model directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Model_Directory_Entry);
   _edje_edd_edje_model_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_model_directory_entry, Edje_Model_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_model_directory_entry, Edje_Model_Directory_Entry, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Model_Directory);
   _edje_edd_edje_model_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_model_directory, Edje_Model_Directory, "entries", entries, _edje_edd_edje_model_directory_entry);

   /* Sound */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Sound_Sample);
   _edje_edd_edje_sound_sample =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_sample, Edje_Sound_Sample, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_sample, Edje_Sound_Sample, "snd_src", snd_src, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_sample, Edje_Sound_Sample, "compression", compression, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_sample, Edje_Sound_Sample, "mode", mode, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_sample, Edje_Sound_Sample, "quality", quality, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_sample, Edje_Sound_Sample, "id", id, EET_T_INT);
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Sound_Tone);
   _edje_edd_edje_sound_tone =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_tone, Edje_Sound_Tone, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_tone, Edje_Sound_Tone, "value", value, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_sound_tone, Edje_Sound_Tone, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Sound_Directory);
   _edje_edd_edje_sound_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_sound_directory, Edje_Sound_Directory, "samples", samples, _edje_edd_edje_sound_sample);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_sound_directory, Edje_Sound_Directory, "tones", tones, _edje_edd_edje_sound_tone);

   /* Vibration */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Vibration_Sample);
   _edje_edd_edje_vibration_sample =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_vibration_sample, Edje_Vibration_Sample, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_vibration_sample, Edje_Vibration_Sample, "src", src, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_vibration_sample, Edje_Vibration_Sample, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Vibration_Directory);
   _edje_edd_edje_vibration_directory =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_vibration_directory, Edje_Vibration_Directory, "samples", samples, _edje_edd_edje_vibration_sample);

   /* color structure for ambient, diffuse and specular colors */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Color);
   _edje_edd_edje_part_description_3d_color =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_color, Edje_Color, "r", r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_color, Edje_Color, "g", g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_color, Edje_Color, "b", b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_color, Edje_Color, "a", a, EET_T_UCHAR);

   /* offset (vector of integer values) */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_3D_Int_Vec);
   _edje_edd_edje_part_description_3d_offset =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_offset, Edje_3D_Int_Vec, "x", x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_offset, Edje_3D_Int_Vec, "y", y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_offset, Edje_3D_Int_Vec, "z", z, EET_T_INT);

   /* vector of float values */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_3D_Vec);
   _edje_edd_edje_part_description_3d_vec =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_vec, Edje_3D_Vec, "x", x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_vec, Edje_3D_Vec, "y", y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_3d_vec, Edje_3D_Vec, "z", z, EDJE_T_FLOAT);

   /* Efl.Gfx.Filter */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Gfx_Filter);
   _edje_edd_edje_filter = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_filter, Edje_Gfx_Filter, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_filter, Edje_Gfx_Filter, "script", script, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Gfx_Filter_Directory);
   _edje_edd_edje_filter_directory = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_filter_directory, Edje_Gfx_Filter_Directory, "filters", filters, _edje_edd_edje_filter);

   /* collection directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Collection_Directory_Entry);
   _edje_edd_edje_part_collection_directory_entry =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "entry", entry, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.RECTANGLE", count.RECTANGLE, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.TEXT", count.TEXT, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.IMAGE", count.IMAGE, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.PROXY", count.PROXY, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.SWALLOW", count.SWALLOW, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.TEXTBLOCK", count.TEXTBLOCK, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.GROUP", count.GROUP, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.BOX", count.BOX, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.TABLE", count.TABLE, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.EXTERNAL", count.EXTERNAL, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.SPACER", count.SPACER, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.SNAPSHOT", count.SNAPSHOT, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.MESH_NODE", count.MESH_NODE, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.LIGHT", count.LIGHT, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.CAMERA", count.CAMERA, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.VECTOR", count.VECTOR, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "count.part", count.part, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection_directory_entry, Edje_Part_Collection_Directory_Entry, "group_alias", group_alias, EET_T_UCHAR);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Style_Tag);
   _edje_edd_edje_style_tag =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style_tag, Edje_Style_Tag, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style_tag, Edje_Style_Tag, "value", value, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Allowed_Seat);
   _edje_edd_edje_part_allowed_seat =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_allowed_seat, Edje_Part_Allowed_Seat, "name", name, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Style);
   _edje_edd_edje_style =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_style, Edje_Style, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_style, Edje_Style, "tags", tags, _edje_edd_edje_style_tag);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Color_Tree_Node);
   _edje_edd_edje_color_tree_node =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_tree_node, Edje_Color_Tree_Node, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST_STRING(_edje_edd_edje_color_tree_node, Edje_Color_Tree_Node, "color_classes", color_classes);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Color_Class);
   _edje_edd_edje_color_class =
     eet_data_descriptor_file_new(&eddc);
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
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_color_class, Edje_Color_Class, "desc", desc, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Text_Class);
   _edje_edd_edje_text_class =
      eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_text_class, Edje_Text_Class, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_text_class, Edje_Text_Class, "font", font, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_text_class, Edje_Text_Class, "size", size, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Size_Class);
   _edje_edd_edje_size_class =
      eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_size_class, Edje_Size_Class, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_size_class, Edje_Size_Class, "minw", minw, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_size_class, Edje_Size_Class, "minh", minh, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_size_class, Edje_Size_Class, "maxw", maxw, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_size_class, Edje_Size_Class, "maxh", maxh, EET_T_INT);

   /* evas filters */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Spec_Filter_Data);
   _edje_edd_edje_part_description_filter_data = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_filter_data, Edje_Part_Description_Spec_Filter_Data, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_filter_data, Edje_Part_Description_Spec_Filter_Data, "value", value, EET_T_STRING);

   /* the main file directory */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_File);
   eddc.func.hash_add = (void * (*)(void *, const char *, void *))_edje_eina_hash_add_alloc;
   _edje_edd_edje_file = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "compiler", compiler, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "version", version, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "minor", minor, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "feature_ver", feature_ver, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "efl_version.major", efl_version.major, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "efl_version.minor", efl_version.minor, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "base_scale", base_scale, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_file, Edje_File, "id", id, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY_STRING(_edje_edd_edje_file, Edje_File, "requires", requires);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "external_dir", external_dir, _edje_edd_edje_external_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "image_dir", image_dir, _edje_edd_edje_image_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "model_dir", model_dir, _edje_edd_edje_model_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "sound_dir", sound_dir, _edje_edd_edje_sound_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "mo_dir", mo_dir, _edje_edd_edje_mo_directory);
   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "filter_dir", filter_dir, _edje_edd_edje_filter_directory);

   EET_DATA_DESCRIPTOR_ADD_SUB(_edje_edd_edje_file, Edje_File, "vibration_dir", vibration_dir, _edje_edd_edje_vibration_directory);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "styles", styles, _edje_edd_edje_style);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "color_tree", color_tree, _edje_edd_edje_color_tree_node);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "color_classes", color_classes, _edje_edd_edje_color_class);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "text_classes", text_classes, _edje_edd_edje_text_class);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_file, Edje_File, "size_classes", size_classes, _edje_edd_edje_size_class);
   EET_DATA_DESCRIPTOR_ADD_HASH(_edje_edd_edje_file, Edje_File, "data", data, _edje_edd_edje_string);
   EET_DATA_DESCRIPTOR_ADD_HASH(_edje_edd_edje_file, Edje_File, "fonts", fonts, _edje_edd_edje_font_directory_entry);
   EET_DATA_DESCRIPTOR_ADD_HASH(_edje_edd_edje_file, Edje_File, "collection", collection, _edje_edd_edje_part_collection_directory_entry);
   EET_DATA_DESCRIPTOR_ADD_HASH(_edje_edd_edje_file, Edje_File, "image_id_hash", image_id_hash, _edje_edd_edje_image_id_hash);

   /* parts & limit & programs - loaded induvidually */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Limit);
   _edje_edd_edje_limit = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_limit, Edje_Limit, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_limit, Edje_Limit, "value", value, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Program_Target);
   _edje_edd_edje_program_target =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program_target, Edje_Program_Target, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Program_After);
   _edje_edd_edje_program_after =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program_after,
                                 Edje_Program_After, "id", id, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Program);
   _edje_edd_edje_program =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "signal", signal, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "source_3d_id", source_3d_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "filter_part", filter.part, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "filter_state", filter.state, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "in.from", in.from, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "in.range", in.range, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "action", action, EET_T_INT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "sample_name", sample_name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "tone_name", tone_name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "duration", duration, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "speed", speed, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "vibration_name", vibration_name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "vibration_repeat", vibration_repeat, EET_T_INT);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "seat", seat, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "state", state, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "state2", state2, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "value", value, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "value2", value2, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "tween.mode", tween.mode, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "tween.time", tween.time, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "v1", tween.v1, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "v2", tween.v2, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "v3", tween.v3, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "v4", tween.v4, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "use_duration_factor", tween.use_duration_factor, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_program, Edje_Program, "targets", targets, _edje_edd_edje_program_target);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_program, Edje_Program, "after", after, _edje_edd_edje_program_after);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "api.name", api.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "api.description", api.description, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "param.src", param.src, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "param.dst", param.dst, EET_T_INT);

#ifdef HAVE_EPHYSICS
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "physics.w", physics.w, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "physics.x", physics.x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "physics.y", physics.y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "physics.z", physics.z, EET_T_DOUBLE);
#endif
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_program, Edje_Program, "channel", channel, EET_T_UCHAR);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Image_Id);
   _edje_edd_edje_part_image_id =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_image_id, Edje_Part_Image_Id, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_image_id, Edje_Part_Image_Id, "set", set, EET_T_UCHAR);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_External_Param);
   _edje_edd_edje_external_param =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_external_param, Edje_External_Param, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_external_param, Edje_External_Param, "type", type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_external_param, Edje_External_Param, "i", i, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_external_param, Edje_External_Param, "d", d, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_external_param, Edje_External_Param, "s", s, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Physics_Face);
   _edje_edd_edje_physics_face = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_physics_face, Edje_Physics_Face, "type", type, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_physics_face, Edje_Physics_Face, "source", source, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Map_Color);
   _edje_edd_edje_map_colors = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_map_colors, Edje_Map_Color,
                                 "idx", idx, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_map_colors, Edje_Map_Color,
                                 "r", r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_map_colors, Edje_Map_Color,
                                 "g", g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_map_colors, Edje_Map_Color,
                                 "b", b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_map_colors, Edje_Map_Color,
                                 "a", a, EET_T_UCHAR);

   EDJE_DEFINE_POINTER_TYPE(Map_Color, map_colors);

#define EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_FIELDS(Edd, Type)                                              \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "state.name", state.name, EET_T_STRING);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "state.value", state.value, EET_T_DOUBLE);                          \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "visible", visible, EET_T_CHAR);                                    \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "no_render", no_render, EET_T_UCHAR);                               \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "limit", limit, EET_T_CHAR);                                        \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "align.x", align.x, EDJE_T_FLOAT);                                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "align.y", align.y, EDJE_T_FLOAT);                                  \
  EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(Edd, Type, "align_3d", align_3d, _edje_edd_edje_part_description_3d_vec); \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "fixed.w", fixed.w, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "fixed.h", fixed.h, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "minmul.have", minmul.have, EET_T_UCHAR);                           \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "minmul.w", minmul.w, EDJE_T_FLOAT);                                \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "minmul.h", minmul.h, EDJE_T_FLOAT);                                \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "min.w", min.w, EET_T_INT);                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "min.h", min.h, EET_T_INT);                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "min.limit", min.limit, EET_T_UCHAR);                               \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "max.w", max.w, EET_T_INT);                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "max.h", max.h, EET_T_INT);                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "max.limit", max.limit, EET_T_UCHAR);                               \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "step.x", step.x, EET_T_INT);                                       \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "step.y", step.y, EET_T_INT);                                       \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "aspect.min", aspect.min, EDJE_T_FLOAT);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "aspect.max", aspect.max, EDJE_T_FLOAT);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "aspect.prefer", aspect.prefer, EET_T_CHAR);                        \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.relative_x", rel1.relative_x, EDJE_T_FLOAT);                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.relative_y", rel1.relative_y, EDJE_T_FLOAT);                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.offset_x", rel1.offset_x, EET_T_INT);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.offset_y", rel1.offset_y, EET_T_INT);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.id_x", rel1.id_x, EET_T_INT);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.id_y", rel1.id_y, EET_T_INT);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.relative_x", rel2.relative_x, EDJE_T_FLOAT);                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.relative_y", rel2.relative_y, EDJE_T_FLOAT);                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.offset_x", rel2.offset_x, EET_T_INT);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.offset_y", rel2.offset_y, EET_T_INT);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.id_x", rel2.id_x, EET_T_INT);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.id_y", rel2.id_y, EET_T_INT);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "clip_to_id", clip_to_id, EET_T_INT);                               \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "size_class", size_class, EET_T_STRING);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color_class", color_class, EET_T_STRING);                          \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color.r", color.r, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color.g", color.g, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color.b", color.b, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color.a", color.a, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color2.r", color2.r, EET_T_UCHAR);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color2.g", color2.g, EET_T_UCHAR);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color2.b", color2.b, EET_T_UCHAR);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color2.a", color2.a, EET_T_UCHAR);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.id_persp", map.id_persp, EET_T_INT);                           \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.id_light", map.id_light, EET_T_INT);                           \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.rot.id_center", map.rot.id_center, EET_T_INT);                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.rot.x", map.rot.x, EDJE_T_FLOAT);                              \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.rot.y", map.rot.y, EDJE_T_FLOAT);                              \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.rot.z", map.rot.z, EDJE_T_FLOAT);                              \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.on", map.on, EET_T_UCHAR);                                     \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.smooth", map.smooth, EET_T_UCHAR);                             \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.alpha", map.alpha, EET_T_UCHAR);                               \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.persp_on", map.persp_on, EET_T_UCHAR);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.backcull", map.backcull, EET_T_UCHAR);                         \
  EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(Edd, Type, "map.color", map.colors, _edje_edd_edje_map_colors_pointer);    \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.zoom.x", map.zoom.x, EDJE_T_FLOAT);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.zoom.y", map.zoom.y, EDJE_T_FLOAT);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "persp.zplane", persp.zplane, EET_T_INT);                           \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "persp.focal", persp.focal, EET_T_INT);                             \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "offset_is_scaled", offset_is_scaled, EET_T_UCHAR);

#ifdef HAVE_EPHYSICS
#define EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON(Edd, Type)                                                          \
  {                                                                                                                 \
     EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_FIELDS(Edd, Type)                                                      \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mass", physics.mass, EET_T_DOUBLE);                          \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.restitution", physics.restitution, EET_T_DOUBLE);            \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.friction", physics.friction, EET_T_DOUBLE);                  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.damping.linear", physics.damping.linear, EET_T_DOUBLE);      \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.damping.angular", physics.damping.angular, EET_T_DOUBLE);    \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.sleep.linear", physics.sleep.linear, EET_T_DOUBLE);          \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.sleep.angular", physics.sleep.angular, EET_T_DOUBLE);        \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.material", physics.material, EET_T_UCHAR);                   \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.density", physics.density, EET_T_DOUBLE);                    \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.hardness", physics.hardness, EET_T_DOUBLE);                  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.z", physics.z, EET_T_INT);                                   \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.depth", physics.depth, EET_T_INT);                           \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.ignore_part_pos", physics.ignore_part_pos, EET_T_UCHAR);     \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.light_on", physics.light_on, EET_T_UCHAR);                   \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.backcull", physics.backcull, EET_T_UCHAR);                   \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.lin.x", physics.mov_freedom.lin.x, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.lin.y", physics.mov_freedom.lin.y, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.lin.z", physics.mov_freedom.lin.z, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.ang.x", physics.mov_freedom.ang.x, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.ang.y", physics.mov_freedom.ang.y, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.ang.z", physics.mov_freedom.ang.z, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_LIST(Edd, Type, "physics.faces", physics.faces, _edje_edd_edje_physics_face);          \
  }
#else
#define EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON(Edd, Type)     \
  {                                                            \
     EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_FIELDS(Edd, Type) \
  }
#endif

#define EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB_FIELDS(Edd, Type, Dec)                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "state.name", Dec.state.name, EET_T_STRING);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "state.value", Dec.state.value, EET_T_DOUBLE);                          \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "visible", Dec.visible, EET_T_CHAR);                                    \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "align.x", Dec.align.x, EDJE_T_FLOAT);                                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "align.y", Dec.align.y, EDJE_T_FLOAT);                                  \
  EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(Edd, Type, "align_3d", Dec.align_3d, _edje_edd_edje_part_description_3d_vec); \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "fixed.w", Dec.fixed.w, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "fixed.h", Dec.fixed.h, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "min.w", Dec.min.w, EET_T_INT);                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "min.h", Dec.min.h, EET_T_INT);                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "max.w", Dec.max.w, EET_T_INT);                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "max.h", Dec.max.h, EET_T_INT);                                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "step.x", Dec.step.x, EET_T_INT);                                       \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "step.y", Dec.step.y, EET_T_INT);                                       \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "aspect.min", Dec.aspect.min, EDJE_T_FLOAT);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "aspect.max", Dec.aspect.max, EDJE_T_FLOAT);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "aspect.prefer", Dec.aspect.prefer, EET_T_CHAR);                        \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.relative_x", Dec.rel1.relative_x, EDJE_T_FLOAT);                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.relative_y", Dec.rel1.relative_y, EDJE_T_FLOAT);                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.offset_x", Dec.rel1.offset_x, EET_T_INT);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.offset_y", Dec.rel1.offset_y, EET_T_INT);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.id_x", Dec.rel1.id_x, EET_T_INT);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel1.id_y", Dec.rel1.id_y, EET_T_INT);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.relative_x", Dec.rel2.relative_x, EDJE_T_FLOAT);                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.relative_y", Dec.rel2.relative_y, EDJE_T_FLOAT);                  \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.offset_x", Dec.rel2.offset_x, EET_T_INT);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.offset_y", Dec.rel2.offset_y, EET_T_INT);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.id_x", Dec.rel2.id_x, EET_T_INT);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "rel2.id_y", Dec.rel2.id_y, EET_T_INT);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "clip_to_id", Dec.clip_to_id, EET_T_INT);                               \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "no_render", Dec.no_render, EET_T_UCHAR);                               \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "size_class", Dec.size_class, EET_T_STRING);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color_class", Dec.color_class, EET_T_STRING);                          \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color.r", Dec.color.r, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color.g", Dec.color.g, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color.b", Dec.color.b, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color.a", Dec.color.a, EET_T_UCHAR);                                   \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color2.r", Dec.color2.r, EET_T_UCHAR);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color2.g", Dec.color2.g, EET_T_UCHAR);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color2.b", Dec.color2.b, EET_T_UCHAR);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "color2.a", Dec.color2.a, EET_T_UCHAR);                                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.id_persp", Dec.map.id_persp, EET_T_INT);                           \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.id_light", Dec.map.id_light, EET_T_INT);                           \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.rot.id_center", Dec.map.rot.id_center, EET_T_INT);                 \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.rot.x", Dec.map.rot.x, EDJE_T_FLOAT);                              \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.rot.y", Dec.map.rot.y, EDJE_T_FLOAT);                              \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.rot.z", Dec.map.rot.z, EDJE_T_FLOAT);                              \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.on", Dec.map.on, EET_T_UCHAR);                                     \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.smooth", Dec.map.smooth, EET_T_UCHAR);                             \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.alpha", Dec.map.alpha, EET_T_UCHAR);                               \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.persp_on", Dec.map.persp_on, EET_T_UCHAR);                         \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.backcull", Dec.map.backcull, EET_T_UCHAR);                         \
  EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(Edd, Type, "map.color", Dec.map.colors, _edje_edd_edje_map_colors_pointer);    \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.zoom.x", Dec.map.zoom.x, EDJE_T_FLOAT);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "map.zoom.y", Dec.map.zoom.y, EDJE_T_FLOAT);                            \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "persp.zplane", Dec.persp.zplane, EET_T_INT);                           \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "persp.focal", Dec.persp.focal, EET_T_INT);                             \
  EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "offset_is_scaled", Dec.offset_is_scaled, EET_T_UCHAR);

#ifdef HAVE_EPHYSICS
#define EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(Edd, Type, Dec)                                                     \
  {                                                                                                                     \
     EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB_FIELDS(Edd, Type, Dec)                                                 \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mass", Dec.physics.mass, EET_T_DOUBLE);                          \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.restitution", Dec.physics.restitution, EET_T_DOUBLE);            \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.friction", Dec.physics.friction, EET_T_DOUBLE);                  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.damping.linear", Dec.physics.damping.linear, EET_T_DOUBLE);      \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.damping.angular", Dec.physics.damping.angular, EET_T_DOUBLE);    \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.sleep.linear", Dec.physics.sleep.linear, EET_T_DOUBLE);          \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.sleep.angular", Dec.physics.sleep.angular, EET_T_DOUBLE);        \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.material", Dec.physics.material, EET_T_UCHAR);                   \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.density", Dec.physics.density, EET_T_DOUBLE);                    \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.hardness", Dec.physics.hardness, EET_T_DOUBLE);                  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.z", Dec.physics.z, EET_T_INT);                                   \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.depth", Dec.physics.depth, EET_T_INT);                           \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.ignore_part_pos", Dec.physics.ignore_part_pos, EET_T_UCHAR);     \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.light_on", Dec.physics.light_on, EET_T_UCHAR);                   \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.backcull", Dec.physics.backcull, EET_T_UCHAR);                   \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.lin.x", Dec.physics.mov_freedom.lin.x, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.lin.y", Dec.physics.mov_freedom.lin.y, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.lin.z", Dec.physics.mov_freedom.lin.z, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.ang.x", Dec.physics.mov_freedom.ang.x, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.ang.y", Dec.physics.mov_freedom.ang.y, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "physics.mov_freedom.ang.z", Dec.physics.mov_freedom.ang.z, EET_T_UCHAR); \
     EET_DATA_DESCRIPTOR_ADD_LIST(Edd, Type, "physics.faces", Dec.physics.faces, _edje_edd_edje_physics_face);          \
  }
#else
#define EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(Edd, Type, Dec)     \
  {                                                                     \
     EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB_FIELDS(Edd, Type, Dec) \
  }
#endif

#define EET_DATA_DESCRIPTOR_ADD_SUB_NESTED_LOOK(Edd, Type, Dec)                                                     \
  {                                                                                                                 \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "Dec##.orientation.look1.x", Dec.orientation.data[0], EDJE_T_FLOAT);  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "Dec##.orientation.look1.y", Dec.orientation.data[1], EDJE_T_FLOAT);  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "Dec##.orientation.look1.z", Dec.orientation.data[2], EDJE_T_FLOAT);  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "Dec##.orientation.look2.x", Dec.orientation.data[3], EDJE_T_FLOAT);  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "Dec##.orientation.look2.y", Dec.orientation.data[4], EDJE_T_FLOAT);  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, "Dec##.orientation.look2.z", Dec.orientation.data[5], EDJE_T_FLOAT);  \
     EET_DATA_DESCRIPTOR_ADD_BASIC(Edd, Type, #Dec ".orientation.look_to", Dec.orientation.look_to, EET_T_INT);     \
  }

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Common);
   eddc.func.mem_free = mem_free_rectangle;
   eddc.func.mem_alloc = mem_alloc_rectangle;
   _edje_edd_edje_part_description_rectangle =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON(_edje_edd_edje_part_description_rectangle, Edje_Part_Description_Common);

   // SVG, since 1.18
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Vector);
   eddc.func.mem_free = mem_free_vector;
   eddc.func.mem_alloc = mem_alloc_vector;
   _edje_edd_edje_part_description_vector =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_vector, Edje_Part_Description_Vector, common);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_vector, Edje_Part_Description_Vector, "vg.id", vg.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_vector, Edje_Part_Description_Vector, "vg.set", vg.set, EET_T_UCHAR);


   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Common);
   eddc.func.mem_free = mem_free_spacer;
   eddc.func.mem_alloc = mem_alloc_spacer;
   _edje_edd_edje_part_description_spacer =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON(_edje_edd_edje_part_description_spacer, Edje_Part_Description_Common);

   // SNAPSHOT, since 1.16
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Snapshot);
   eddc.func.mem_free = mem_free_snapshot;
   eddc.func.mem_alloc = mem_alloc_snapshot;
   _edje_edd_edje_part_description_snapshot =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_snapshot, Edje_Part_Description_Snapshot, common);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_snapshot, Edje_Part_Description_Snapshot, "filter.code", filter.code, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST_STRING(_edje_edd_edje_part_description_snapshot, Edje_Part_Description_Snapshot, "filter.sources", filter.sources);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_description_snapshot, Edje_Part_Description_Snapshot, "filter.data", filter.data, _edje_edd_edje_part_description_filter_data);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Common);
   eddc.func.mem_free = mem_free_swallow;
   eddc.func.mem_alloc = mem_alloc_swallow;
   _edje_edd_edje_part_description_swallow =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON(_edje_edd_edje_part_description_swallow, Edje_Part_Description_Common);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Common);
   eddc.func.mem_free = mem_free_group;
   eddc.func.mem_alloc = mem_alloc_group;
   _edje_edd_edje_part_description_group =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON(_edje_edd_edje_part_description_group, Edje_Part_Description_Common);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Image);
   eddc.func.mem_free = mem_free_image;
   eddc.func.mem_alloc = mem_alloc_image;
   _edje_edd_edje_part_description_image =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, common);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Mesh_Node);
   eddc.func.mem_free = mem_free_mesh_node;
   eddc.func.mem_alloc = mem_alloc_mesh_node;
   _edje_edd_edje_part_description_mesh_node =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, common);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.position.point", mesh_node.position.point, _edje_edd_edje_part_description_3d_vec);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.position.space", mesh_node.position.space, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.properties.ambient", mesh_node.properties.ambient, _edje_edd_edje_part_description_3d_color);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.properties.diffuse", mesh_node.properties.diffuse, _edje_edd_edje_part_description_3d_color);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.properties.specular", mesh_node.properties.specular, _edje_edd_edje_part_description_3d_color);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.properties.material_attrib", mesh_node.properties.material_attrib, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.properties.normal", mesh_node.properties.normal, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.properties.shininess", mesh_node.properties.shininess, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.properties.shade", mesh_node.properties.shade, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.aabb1.relative", mesh_node.aabb1.relative, _edje_edd_edje_part_description_3d_vec);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.aabb1.offset", mesh_node.aabb1.offset, _edje_edd_edje_part_description_3d_offset);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.aabb1.rel_to", mesh_node.aabb1.rel_to, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.aabb2.relative", mesh_node.aabb2.relative, _edje_edd_edje_part_description_3d_vec);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.aabb2.offset", mesh_node.aabb2.offset, _edje_edd_edje_part_description_3d_offset);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.aabb2.rel_to", mesh_node.aabb2.rel_to, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.orientation.type", mesh_node.orientation.type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED_LOOK(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, mesh_node);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.scale_3d", mesh_node.scale_3d, _edje_edd_edje_part_description_3d_vec);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.texture.id", mesh_node.texture.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.texture.set", mesh_node.texture.set, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.texture.wrap1", mesh_node.texture.wrap1, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.texture.wrap2", mesh_node.texture.wrap2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.texture.filter1", mesh_node.texture.filter1, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.texture.filter2", mesh_node.texture.filter2, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.texture.need_texture", mesh_node.texture.need_texture, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.texture.textured", mesh_node.texture.textured, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.mesh.primitive", mesh_node.mesh.primitive, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.mesh.assembly", mesh_node.mesh.assembly, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.mesh.id", mesh_node.mesh.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.mesh.set", mesh_node.mesh.set, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_mesh_node, Edje_Part_Description_Mesh_Node, "mesh_node.mesh.frame", mesh_node.mesh.frame, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Light);
   eddc.func.mem_free = mem_free_light;
   eddc.func.mem_alloc = mem_alloc_light;
   _edje_edd_edje_part_description_light =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, common);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.position.point", light.position.point, _edje_edd_edje_part_description_3d_vec);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.position.space", light.position.space, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.orientation.type", light.orientation.type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED_LOOK(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, light);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.properties.ambient", light.properties.ambient, _edje_edd_edje_part_description_3d_color);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.properties.diffuse", light.properties.diffuse, _edje_edd_edje_part_description_3d_color);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.properties.specular", light.properties.specular, _edje_edd_edje_part_description_3d_color);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.light.perspective.fovy", light.light.fovy, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.light.perspective.aspect", light.light.aspect, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.light.perspective.near", light.light.frustum_near, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_light, Edje_Part_Description_Light, "light.light.perspective.far", light.light.frustum_far, EDJE_T_FLOAT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Camera);
   eddc.func.mem_free = mem_free_camera;
   eddc.func.mem_alloc = mem_alloc_camera;
   _edje_edd_edje_part_description_camera =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, common);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, "camera.position.point", camera.position.point, _edje_edd_edje_part_description_3d_vec);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, "camera.position.space", camera.position.space, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, "camera.orientation.type", camera.orientation.type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB_NESTED_LOOK(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, camera);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, "camera.camera.perspective.fovy", camera.camera.fovy, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, "camera.camera.perspective.aspect", camera.camera.aspect, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, "camera.camera.perspective.near", camera.camera.frustum_near, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_camera, Edje_Part_Description_Camera, "camera.camera.perspective.far", camera.camera.frustum_far, EDJE_T_FLOAT);

   EDJE_DEFINE_POINTER_TYPE(Part_Image_Id, part_image_id);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.tweens", image.tweens, _edje_edd_edje_part_image_id_pointer);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.id", image.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.set", image.set, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.border.l", image.border.l, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.border.r", image.border.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.border.t", image.border.t, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.border.b", image.border.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.border.no_fill", image.border.no_fill, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.border.scale", image.border.scale, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.border.scale_by", image.border.scale_by, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.smooth", image.fill.smooth, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.pos_rel_x", image.fill.pos_rel_x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.pos_abs_x", image.fill.pos_abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.rel_x", image.fill.rel_x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.abs_x", image.fill.abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.pos_rel_y", image.fill.pos_rel_y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.pos_abs_y", image.fill.pos_abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.rel_y", image.fill.rel_y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.abs_y", image.fill.abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.fill.type", image.fill.type, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.filter.code", filter.code, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST_STRING(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.filter.sources", filter.sources); // @since 1.15
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_description_image, Edje_Part_Description_Image, "image.filter.data", filter.data, _edje_edd_edje_part_description_filter_data); // @since 1.15

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Proxy);
   eddc.func.mem_free = mem_free_proxy;
   eddc.func.mem_alloc = mem_alloc_proxy;
   _edje_edd_edje_part_description_proxy =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, common);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.id", proxy.id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.smooth", proxy.fill.smooth, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.pos_rel_x", proxy.fill.pos_rel_x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.pos_abs_x", proxy.fill.pos_abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.rel_x", proxy.fill.rel_x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.abs_x", proxy.fill.abs_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.pos_rel_y", proxy.fill.pos_rel_y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.pos_abs_y", proxy.fill.pos_abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.rel_y", proxy.fill.rel_y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.abs_y", proxy.fill.abs_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.fill.type", proxy.fill.type, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.source_visible", proxy.source_visible, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.source_clip", proxy.source_clip, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.filter.code", filter.code, EET_T_STRING); // @since 1.16
   EET_DATA_DESCRIPTOR_ADD_LIST_STRING(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.filter.sources", filter.sources); // @since 1.16
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_description_proxy, Edje_Part_Description_Proxy, "proxy.filter.data", filter.data, _edje_edd_edje_part_description_filter_data); // @since 1.16

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Text);
   eddc.func.mem_free = mem_free_text;
   eddc.func.mem_alloc = mem_alloc_text;
   _edje_edd_edje_part_description_text =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, common);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.color3.r", text.color3.r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.color3.g", text.color3.g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.color3.b", text.color3.b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.color3.a", text.color3.a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.text", text.text, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.domain", text.domain, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.text_class", text.text_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.style", text.style, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.font", text.font, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.size_range_min", text.size_range_min, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.size_range_max", text.size_range_max, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.repch", text.repch, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.size", text.size, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.fit_x", text.fit_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.fit_y", text.fit_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.min_x", text.min_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.min_y", text.min_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.max_x", text.max_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.max_y", text.max_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.align.x", text.align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.align.y", text.align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.id_source", text.id_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.id_source_part", text.id_source_part, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.id_text_source", text.id_text_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.id_text_source_part", text.id_text_source_part, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.ellipsis", text.ellipsis, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.filter", filter.code, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST_STRING(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.filter_sources", filter.sources);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_description_text, Edje_Part_Description_Text, "text.filter.data", filter.data, _edje_edd_edje_part_description_filter_data); // @since 1.15

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Text);
   eddc.func.mem_free = mem_free_textblock;
   eddc.func.mem_alloc = mem_alloc_textblock;
   _edje_edd_edje_part_description_textblock =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, common);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.color3.r", text.color3.r, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.color3.g", text.color3.g, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.color3.b", text.color3.b, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.color3.a", text.color3.a, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.text", text.text, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.domain", text.domain, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.text_class", text.text_class, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.style", text.style, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.font", text.font, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.repch", text.repch, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.size", text.size, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.size_range_min", text.size_range_min, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.size_range_max", text.size_range_max, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.fit_x", text.fit_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.fit_y", text.fit_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.min_x", text.min_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.min_y", text.min_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.max_x", text.max_x, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.max_y", text.max_y, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.align.x", text.align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.align.y", text.align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.id_source", text.id_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.id_source_part", text.id_source_part, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.id_text_source", text.id_text_source, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.id_text_source_part", text.id_text_source_part, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_textblock, Edje_Part_Description_Text, "text.ellipsis", text.ellipsis, EET_T_DOUBLE);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Box);
   eddc.func.mem_free = mem_free_box;
   eddc.func.mem_alloc = mem_alloc_box;
   _edje_edd_edje_part_description_box =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, common);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, "box.layout", box.layout, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, "box.alt_layout", box.alt_layout, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, "box.align.x", box.align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, "box.align.y", box.align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, "box.padding.x", box.padding.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, "box.padding.y", box.padding.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, "box.min.h", box.min.h, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_box, Edje_Part_Description_Box, "box.min.v", box.min.v, EET_T_UCHAR);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_Table);
   eddc.func.mem_free = mem_free_table;
   eddc.func.mem_alloc = mem_alloc_table;
   _edje_edd_edje_part_description_table =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_table, Edje_Part_Description_Table, common);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_table, Edje_Part_Description_Table, "table.homogeneous", table.homogeneous, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_table, Edje_Part_Description_Table, "table.align.x", table.align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_table, Edje_Part_Description_Table, "table.align.y", table.align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_table, Edje_Part_Description_Table, "table.padding.x", table.padding.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_table, Edje_Part_Description_Table, "table.padding.y", table.padding.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_table, Edje_Part_Description_Table, "table.min.h", table.min.h, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_description_table, Edje_Part_Description_Table, "table.min.v", table.min.v, EET_T_UCHAR);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Description_External);
   eddc.func.mem_free = mem_free_external;
   eddc.func.mem_alloc = mem_alloc_external;
   _edje_edd_edje_part_description_external =
     eet_data_descriptor_file_new(&eddc);
   EDJE_DATA_DESCRIPTOR_DESCRIPTION_COMMON_SUB(_edje_edd_edje_part_description_external, Edje_Part_Description_External, common);
   EET_DATA_DESCRIPTOR_ADD_LIST(_edje_edd_edje_part_description_external, Edje_Part_Description_External, "external_params", external_params, _edje_edd_edje_external_param);

   EDJE_DEFINE_POINTER_TYPE(Part_Description_Common, part_description_spacer);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Common, part_description_rectangle);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Common, part_description_swallow);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Common, part_description_group);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Snapshot, part_description_snapshot);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Image, part_description_image);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Proxy, part_description_proxy);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Text, part_description_text);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Text, part_description_textblock);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Box, part_description_box);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Table, part_description_table);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_External, part_description_external);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Mesh_Node, part_description_mesh_node);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Camera, part_description_camera);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Light, part_description_light);
   EDJE_DEFINE_POINTER_TYPE(Part_Description_Vector, part_description_vector);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _edje_description_variant_type_get;
   eddc.func.type_set = _edje_description_variant_type_set;
   _edje_edd_edje_part_description_variant = eet_data_descriptor_file_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "spacer", _edje_edd_edje_part_description_spacer);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "rectangle", _edje_edd_edje_part_description_rectangle);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "snapshot", _edje_edd_edje_part_description_snapshot);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "swallow", _edje_edd_edje_part_description_swallow);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "group", _edje_edd_edje_part_description_group);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "image", _edje_edd_edje_part_description_image);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "proxy", _edje_edd_edje_part_description_proxy);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "text", _edje_edd_edje_part_description_text);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "textblock", _edje_edd_edje_part_description_textblock);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "box", _edje_edd_edje_part_description_box);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "table", _edje_edd_edje_part_description_table);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "external", _edje_edd_edje_part_description_external);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "mesh_node", _edje_edd_edje_part_description_mesh_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "light", _edje_edd_edje_part_description_light);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "camera", _edje_edd_edje_part_description_camera);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(_edje_edd_edje_part_description_variant, "vector", _edje_edd_edje_part_description_vector);

#define EDJE_ADD_ARRAY_MAPPING(Variant, Type, Minus)                                     \
  {                                                                                      \
     Edje_Part_Description_List tmp;                                                     \
                                                                                         \
     eet_data_descriptor_element_add(Variant, Type,                                      \
                                     EET_T_UNKNOW, EET_G_VAR_ARRAY,                      \
                                     0, (char *)(&tmp.desc_count) - (char *)(&tmp),      \
                                     NULL,                                               \
                                     _edje_edd_edje_part_description_##Minus##_pointer); \
  }

   _edje_edd_edje_part_description_variant_list = eet_data_descriptor_file_new(&eddc);

   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "rectangle", rectangle);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "snapshot", snapshot);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "spacer", spacer);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "swallow", swallow);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "group", group);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "image", image);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "proxy", proxy);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "text", text);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "textblock", textblock);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "box", box);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "table", table);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "external", external);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "mesh_node", mesh_node);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "light", light);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "camera", camera);
   EDJE_ADD_ARRAY_MAPPING(_edje_edd_edje_part_description_variant_list, "vector", vector);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Pack_Element);
   _edje_edd_edje_pack_element =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "type", type, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "min.w", min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "min.h", min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "spread.w", spread.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "spread.h", spread.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "prefer.w", prefer.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "prefer.h", prefer.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "max.w", max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "max.h", max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "padding.l", padding.l, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "padding.r", padding.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "padding.t", padding.t, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "padding.b", padding.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "align.x", align.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "align.y", align.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "weight.x", weight.x, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "weight.y", weight.y, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "aspect.w", aspect.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "aspect.h", aspect.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "aspect.mode", aspect.mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "options", options, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "col", col, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "row", row, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "colspan", colspan, EET_T_USHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_pack_element, Edje_Pack_Element, "rowspan", rowspan, EET_T_USHORT);

   EDJE_DEFINE_POINTER_TYPE(Pack_Element, pack_element);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part);
   eddc.func.mem_free = mem_free_part;
   eddc.func.mem_alloc = mem_alloc_part;
   _edje_edd_edje_part =
     eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_VARIANT(_edje_edd_edje_part, Edje_Part, "default_desc", default_desc, type, _edje_edd_edje_part_description_variant);
   EET_DATA_DESCRIPTOR_ADD_VARIANT(_edje_edd_edje_part, Edje_Part, "other", other, type, _edje_edd_edje_part_description_variant_list);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source", source, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source2", source2, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source3", source3, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source4", source4, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source5", source5, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "source6", source6, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "clip_to_id", clip_to_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "no_render", no_render, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.x", dragable.x, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.step_x", dragable.step_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.count_x", dragable.count_x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.y", dragable.y, EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.step_y", dragable.step_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.count_y", dragable.count_y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.counfine_id", dragable.confine_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.threshold_id", dragable.threshold_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "dragable.events_id", dragable.event_id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part, Edje_Part, "items", items, _edje_edd_edje_pack_element_pointer);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "type", type, EET_T_UCHAR);
#ifdef HAVE_EPHYSICS
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "physics_body", physics_body, EET_T_UCHAR);
#endif
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "effect", effect, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "mouse_events", mouse_events, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "anti_alias", anti_alias, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "repeat_events", repeat_events, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "ignore_flags", ignore_flags, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "mask_flags", mask_flags, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "scale", scale, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "precise_is_inside", precise_is_inside, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "use_alternate_font_metrics", use_alternate_font_metrics, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "pointer_mode", pointer_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "entry_mode", entry_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "select_mode", select_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "cursor_mode", cursor_mode, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "multiline", multiline, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "access", access, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "api.name", api.name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "api.description", api.description, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "nested_children_count", nested_children_count, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part, Edje_Part, "required", required, EET_T_UCHAR);

   EDJE_DEFINE_POINTER_TYPE(Part_Allowed_Seat, part_allowed_seat);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part, Edje_Part, "allowed_seats", allowed_seats, _edje_edd_edje_part_allowed_seat_pointer);


   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Limit);
   _edje_edd_edje_part_limit = eet_data_descriptor_file_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_limit, Edje_Part_Limit, "part", part, EET_T_INT);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Edje_Part_Collection);
   _edje_edd_edje_part_collection =
     eet_data_descriptor_file_new(&eddc);

   EDJE_DEFINE_POINTER_TYPE(Program, program);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "programs.fnmatch", programs.fnmatch, _edje_edd_edje_program_pointer);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "programs.strcmp", programs.strcmp, _edje_edd_edje_program_pointer);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "programs.strncmp", programs.strncmp, _edje_edd_edje_program_pointer);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "programs.strrncmp", programs.strrncmp, _edje_edd_edje_program_pointer);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "programs.nocmp", programs.nocmp, _edje_edd_edje_program_pointer);

   EDJE_DEFINE_POINTER_TYPE(Part, part);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "parts", parts, _edje_edd_edje_part_pointer);

   EDJE_DEFINE_POINTER_TYPE(Limit, limit);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "limits.vertical", limits.vertical, _edje_edd_edje_limit_pointer);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "limits.horizontal", limits.horizontal, _edje_edd_edje_limit_pointer);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(_edje_edd_edje_part_collection, Edje_Part_Collection, "limits.parts", limits.parts, _edje_edd_edje_part_limit);

   EET_DATA_DESCRIPTOR_ADD_HASH(_edje_edd_edje_part_collection, Edje_Part_Collection, "data", data, _edje_edd_edje_string);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "id", id, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_HASH_STRING(_edje_edd_edje_part_collection, Edje_Part_Collection, "alias", alias);
   EET_DATA_DESCRIPTOR_ADD_HASH_STRING(_edje_edd_edje_part_collection, Edje_Part_Collection, "aliased", aliased);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "scene_size.width", scene_size.width, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "scene_size.height", scene_size.height, EDJE_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.min.w", prop.min.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.min.h", prop.min.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.max.w", prop.max.w, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.max.h", prop.max.h, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "lua_script_only", lua_script_only, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "prop.orientation", prop.orientation, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "broadcast_signal", broadcast_signal, EET_T_UCHAR);
#ifdef HAVE_EPHYSICS
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "physics.world.rate", physics.world.rate, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "physics.world.gravity.x", physics.world.gravity.x, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "physics.world.gravity.y", physics.world.gravity.y, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "physics.world.gravity.z", physics.world.gravity.z, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "physics.world.z", physics.world.z, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "physics.world.depth", physics.world.depth, EET_T_INT);
#endif
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "physics_enabled", physics_enabled, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "script_recursion", script_recursion, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_edje_edd_edje_part_collection, Edje_Part_Collection, "use_custom_seat_names", use_custom_seat_names, EET_T_UCHAR);
}

EAPI void
_edje_data_font_list_desc_make(Eet_Data_Descriptor **_font_list_edd,
                               Eet_Data_Descriptor **_font_edd)
{  /* User have to free: _font_list_edd, _font_edd */
  Eet_Data_Descriptor_Class eddc;

  eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc),
                                            "font", sizeof (Edje_Font));
  *_font_edd = eet_data_descriptor_stream_new(&eddc);
  EET_DATA_DESCRIPTOR_ADD_BASIC(*_font_edd, Edje_Font,
                                "file", file, EET_T_INLINED_STRING);
  EET_DATA_DESCRIPTOR_ADD_BASIC(*_font_edd, Edje_Font,
                                "name", name, EET_T_INLINED_STRING);

  eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc),
                                            "font_list", sizeof (Edje_Font_List));
  *_font_list_edd = eet_data_descriptor_stream_new(&eddc);
  EET_DATA_DESCRIPTOR_ADD_LIST(*_font_list_edd, Edje_Font_List,
                               "list", list, *_font_edd);
}

