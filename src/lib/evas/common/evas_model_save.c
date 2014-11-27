#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_options.h"

#include "evas_common_private.h"
#include "evas_private.h"


void
evas_common_save_model_to_file(Evas_3D_Mesh *model,
                               const char *file,
                               Evas_3D_Mesh_Frame *f,
                               Evas_3D_Mesh_File_Type type)
{
   switch (type)
     {
      case EVAS_3D_MESH_FILE_TYPE_OBJ:
        evas_model_save_file_obj(model, file, f);
        break;
      case EVAS_3D_MESH_FILE_TYPE_EET:
        evas_model_save_file_eet(model, file, f);
        break;
      case EVAS_3D_MESH_FILE_TYPE_PLY:
        evas_model_save_file_ply(model, file, f);
        break;
      default:
        ERR("Invalid mesh file type.");
        break;
     }
}
