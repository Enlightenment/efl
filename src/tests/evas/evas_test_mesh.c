#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>

#include "../../lib/evas/include/evas_common_private.h"
#include "../../lib/evas/include/evas_private.h"

#include "evas_suite.h"
#include "evas_tests_helpers.h"

#define TESTS_MESH_DIR TESTS_SRC_DIR"/meshes"
#define TESTS_OBJ_MESH_DIR TESTS_MESH_DIR"/obj"
#define TESTS_MD2_MESH_DIR TESTS_MESH_DIR"/md2"
#define TESTS_PLY_MESH_DIR TESTS_MESH_DIR"/ply"

#define COMPARE_GEOMETRICS(a)                                    \
   vb1 = &f1->vertices[a];                                       \
   vb2 = &f2->vertices[a];                                       \
   if ((a == EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION) &&            \
       ((vb1->data == NULL) || (vb2->data == NULL)))             \
      return 1;                                                  \
   if ((vb1->data != NULL) && (vb2->data != NULL))               \
     {                                                           \
        src1 = (float *)vb1->data;                               \
        src2 = (float *)vb2->data;                               \
        for (i = 0; i < pd1->vertex_count; i++)                  \
           {                                                     \
              if ((src1[0] != src2[0]) || (src1[1] != src2[1]))  \
                return 1;                                        \
              if (a != EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD)     \
              if (src1[2] != src2[2])                            \
                return 1;                                        \
              src1 += f1->vertices[a].element_count;             \
              src2 += f2->vertices[a].element_count;             \
           }                                                     \
     }

#define CHECK_MESHES_IN_FOLDER(folder, ext)                             \
  it = eina_file_direct_ls(folder);                                     \
  EINA_ITERATOR_FOREACH(it, file)                                       \
    {                                                                   \
       int set_ok, save_ok;                                             \
       Eina_File *f; \
       const Eina_File *f_get = NULL; \
       const char *filename = NULL; \
       const char *key = NULL; \
       mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, e);                      \
       mesh2 = efl_add(EVAS_CANVAS3D_MESH_CLASS, e);                     \
       fail_if(mesh == NULL);                                           \
       fail_if(mesh2 == NULL);                                          \
       snprintf(buffer, PATH_MAX, "%s%s", tmp, ext);                    \
       set_ok = efl_file_simple_load(mesh, file->path, NULL); \
       save_ok = efl_file_save(mesh, buffer, NULL, NULL);              \
       fail_if(!set_ok);                                                \
       fail_if(!save_ok);                                               \
       set_ok = efl_file_simple_load(mesh2, buffer, NULL);               \
       fail_if(!set_ok);                                                \
       efl_file_simple_get(mesh2, &filename, &key);               \
       fail_if(!filename);                                                \
       fail_if(!!key);                                                \
       res = _compare_meshes(mesh, mesh2);                              \
       fail_if(res == 1);                                               \
       f = eina_file_open(file->path, 0); \
       fail_if(!f); \
       set_ok = efl_file_simple_mmap_load(mesh, f, NULL); \
       efl_file_simple_mmap_get(mesh, &f_get, &key); \
       fail_if(!f_get); \
       fail_if(!!key); \
       eina_file_close(f); \
       save_ok = efl_file_save(mesh, buffer, NULL, NULL);              \
       fail_if(!set_ok);                                                \
       fail_if(!save_ok);                                               \
       f = eina_file_open(buffer, 0); \
       fail_if(!f); \
       set_ok = efl_file_simple_mmap_load(mesh2, f, NULL); \
       eina_file_close(f); \
       fail_if(!set_ok);                                                \
       res = _compare_meshes(mesh, mesh2);                              \
       fail_if(res == 1);                                               \
       efl_del(mesh2);                                                   \
       efl_del(mesh);                                                    \
       unlink(buffer);                                                  \
    } \
  eina_iterator_free(it);

static Evas_Canvas3D_Mesh_Frame *
return_zero_frame(Evas_Canvas3D_Mesh_Data *pd)
{
   Eina_List *l;
   Evas_Canvas3D_Mesh_Frame *f;

   EINA_LIST_FOREACH(pd->frames, l, f)
     {
        if (f->frame == 0)
          return f;
     }

   return NULL;
}

static int _compare_meshes(Evas_Canvas3D_Mesh *mesh1, Evas_Canvas3D_Mesh *mesh2)
{
   Evas_Canvas3D_Mesh_Data *pd1, *pd2;
   Evas_Canvas3D_Mesh_Frame *f1, *f2;
   int i;
   float *src1, *src2;
   Evas_Canvas3D_Vertex_Buffer *vb1, *vb2;

   pd1 = efl_data_scope_get(mesh1, EVAS_CANVAS3D_MESH_CLASS);
   f1 = return_zero_frame(pd1);
   if ((pd1 == NULL) || (f1 == NULL))
      return 1;

   pd2 = efl_data_scope_get(mesh2, EVAS_CANVAS3D_MESH_CLASS);
   f2 = return_zero_frame(pd2);
   if ((pd2 == NULL) || (f2 == NULL))
      return 1;

   if ((pd1->vertex_count) != (pd2->vertex_count))
      return 1;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#endif

   COMPARE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION)
   COMPARE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL)
   COMPARE_GEOMETRICS(EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD)

#ifdef __clang__
#pragma clang diagnostic pop
#endif

   return 0;
}

EFL_START_TEST(evas_object_mesh_loader_saver)
{
   char buffer[PATH_MAX];
   Evas *e = _setup_evas();
   Eina_Tmpstr *tmp;
   Eo *mesh, *mesh2;
   Eina_Iterator *it;
   char *file_mask = strdup("evas_test_mesh_XXXXXX");
   int res = 0, tmpfd;
   const Eina_File_Direct_Info *file;
   struct
   {
      const char *dir;
      const char *fmt;
   } values[4] =
   {
     { TESTS_OBJ_MESH_DIR, ".eet" },
     { TESTS_MD2_MESH_DIR, ".eet" },
     { TESTS_PLY_MESH_DIR, ".eet" },
     { TESTS_PLY_MESH_DIR, ".ply" },
   };

   /* create tmp file name, assume tmp.eet and tmp.ply also work */
   tmpfd = eina_file_mkstemp(file_mask, &tmp);
   fail_if(tmpfd == -1);
   fail_if(!!close(tmpfd));

   CHECK_MESHES_IN_FOLDER(values[_i].dir, values[_i].fmt)

   unlink(tmp);

   free(file_mask);
   evas_free(e);
}
EFL_END_TEST

void evas_test_mesh(TCase *tc)
{
   tcase_add_loop_test(tc, evas_object_mesh_loader_saver, 0, 1);
}

void evas_test_mesh1(TCase *tc)
{
   tcase_add_loop_test(tc, evas_object_mesh_loader_saver, 1, 2);
}

void evas_test_mesh2(TCase *tc)
{
   tcase_add_loop_test(tc, evas_object_mesh_loader_saver, 2, 3);
}

void evas_test_mesh3(TCase *tc)
{
   tcase_add_loop_test(tc, evas_object_mesh_loader_saver, 3, 4);
}
