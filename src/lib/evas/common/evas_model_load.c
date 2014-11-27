#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_options.h"

#include "evas_common_private.h"
#include "evas_private.h"


void
evas_common_load_model_to_file(Evas_3D_Mesh *model,
                               const char *file,
                               Evas_3D_Mesh_File_Type type)
{
   switch (type)
     {
      case EVAS_3D_MESH_FILE_TYPE_MD2:
        evas_model_load_file_md2(model, file);
        break;
      case EVAS_3D_MESH_FILE_TYPE_OBJ:
        evas_model_load_file_obj(model, file);
        break;
      case EVAS_3D_MESH_FILE_TYPE_EET:
        evas_model_load_file_eet(model, file);
        break;
      case EVAS_3D_MESH_FILE_TYPE_PLY:
        evas_model_load_file_ply(model, file);
        break;
      default:
        ERR("Invalid mesh file type.");
        break;
     }
}
