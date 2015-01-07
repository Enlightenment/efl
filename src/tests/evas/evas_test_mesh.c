#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>

#include "../../lib/evas/include/evas_common_private.h"
#include "../../lib/evas/include/evas_private.h"
#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"

#define TESTS_MESH_DIR TESTS_SRC_DIR"/meshes"
#define TESTS_OBJ_MESH_DIR TESTS_MESH_DIR"/obj"
#define TESTS_MD2_MESH_DIR TESTS_MESH_DIR"/md2"
#define TESTS_PLY_MESH_DIR TESTS_MESH_DIR"/ply"

#define COMPARE_GEOMETRICS(a)                               \
   vb1 = &f1->vertices[a];                                  \
   vb2 = &f2->vertices[a];                                  \
   if ((vb1->data == NULL) || (vb2->data == NULL))          \
      return 1;                                             \
   src1 = (float *)vb1->data;                               \
   src2 = (float *)vb2->data;                               \
   for (i = 0; i < pd1->vertex_count; i++)                  \
     {                                                      \
        if ((src1[0] != src2[0]) || (src1[1] != src2[1]))   \
           return 1;                                        \
        if (a != EVAS_3D_VERTEX_TEXCOORD)                   \
           if (src1[2] != src2[2])                          \
              return 1;                                     \
        src1 += f1->vertices[a].element_count;              \
        src2 += f2->vertices[a].element_count;              \
     }

#define CHECK_MESHES_IN_FOLDER(folder, ext)                                                                     \
   it = eina_file_direct_ls(folder);                                                                            \
   EINA_ITERATOR_FOREACH(it, file)                                                                              \
     {                                                                                                          \
        mesh = eo_add(EVAS_3D_MESH_CLASS, e);                                                                   \
        mesh2 = eo_add(EVAS_3D_MESH_CLASS, e);                                                                  \
        fail_if(mesh == NULL);                                                                                  \
        fail_if(mesh2 == NULL);                                                                                 \
        snprintf(buffer, PATH_MAX, "%s", ext);                                                                  \
        eo_do(mesh, efl_file_set(file->path, NULL),                                                             \
                    efl_file_save(buffer, NULL, NULL));                                                         \
        eo_do(mesh2, efl_file_set(buffer, NULL));                                                               \
        res = _compare_meshes(mesh, mesh2);                                                                     \
        fail_if(res == 1);                                                                                      \
        eo_do(mesh, evas_3d_mesh_mmap_set(eina_file_open(file->path, 0), NULL),                                 \
                    efl_file_save(buffer, NULL, NULL));                                                         \
        eo_do(mesh2, evas_3d_mesh_mmap_set(eina_file_open(buffer, 0), NULL));                                   \
        res = _compare_meshes(mesh, mesh2);                                                                     \
        fail_if(res == 1);                                                                                      \
        eo_del(mesh2);                                                                                          \
        eo_del(mesh);                                                                                           \
        unlink(buffer);                                                                                         \
     }

static Evas_3D_Mesh_Frame *
return_zero_frame(Evas_3D_Mesh_Data *pd)
{
   Eina_List *l;
   Evas_3D_Mesh_Frame *f;

   EINA_LIST_FOREACH(pd->frames, l, f)
     {
        if (f->frame == 0)
          return f;
     }

   return NULL;
}

static int _compare_meshes(Evas_3D_Mesh *mesh1, Evas_3D_Mesh *mesh2)
{
   Evas_3D_Mesh_Data *pd1, *pd2;
   Evas_3D_Mesh_Frame *f1, *f2;
   int i;
   float *src1, *src2;
   Evas_3D_Vertex_Buffer *vb1, *vb2;

   pd1 = eo_data_scope_get(mesh1, EVAS_3D_MESH_CLASS);
   f1 = return_zero_frame(pd1);
   if ((pd1 == NULL) || (f1 == NULL))
      return 1;

   pd2 = eo_data_scope_get(mesh2, EVAS_3D_MESH_CLASS);
   f2 = return_zero_frame(pd2);
   if ((pd2 == NULL) || (f2 == NULL))
      return 1;

   if ((pd1->vertex_count) != (pd2->vertex_count))
      return 1;

   COMPARE_GEOMETRICS(EVAS_3D_VERTEX_POSITION)
   COMPARE_GEOMETRICS(EVAS_3D_VERTEX_NORMAL)
   COMPARE_GEOMETRICS(EVAS_3D_VERTEX_TEXCOORD)

   return 0;
}

START_TEST(evas_object_mesh_loader_saver)
{
   char buffer[PATH_MAX];
   Evas *e = _setup_evas();
   Eina_Tmpstr *tmp;
   Eo *mesh, *mesh2;
   Eina_Iterator *it;
   char *file_mask = strdup("evas_test_mesh_XXXXXX");
   int res = 0, tmpfd;
   const Eina_File_Direct_Info *file;

   tmpfd = eina_file_mkstemp(file_mask, &tmp);
   fail_if(tmpfd == -1);
   fail_if(!!close(tmpfd));

   snprintf(buffer, PATH_MAX, "%s", tmp);

   CHECK_MESHES_IN_FOLDER(TESTS_OBJ_MESH_DIR, ".eet")
   CHECK_MESHES_IN_FOLDER(TESTS_MD2_MESH_DIR, ".eet")
   CHECK_MESHES_IN_FOLDER(TESTS_PLY_MESH_DIR, ".eet")
   CHECK_MESHES_IN_FOLDER(TESTS_PLY_MESH_DIR, ".ply")

   eina_iterator_free(it);

   evas_free(e);
   evas_shutdown();
}
END_TEST

void evas_test_mesh(TCase *tc)
{
   tcase_add_test(tc, evas_object_mesh_loader_saver);
}
