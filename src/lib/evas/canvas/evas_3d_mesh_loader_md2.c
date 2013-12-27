#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "evas_common_private.h"
#include "evas_private.h"

#define PACKED __attribute__((__packed__))

#define MD2_MAGIC_NUMBER   844121161
#define MD2_VERSION        8
#define MD2_FRAME_SCALE    256

/* Structures for reading data from file. */
typedef struct _MD2_Header    MD2_Header;
typedef struct _MD2_Vertex    MD2_Vertex;
typedef struct _MD2_Frame     MD2_Frame;
typedef struct _MD2_Triangle  MD2_Triangle;
typedef struct _MD2_Texcoord  MD2_Texcoord;

struct PACKED _MD2_Header
{
   int   magic;
   int   version;

   int   skin_width;
   int   skin_height;

   int   frame_size;

   int   skins_count;
   int   vertex_count;
   int   texcoord_count;
   int   triangle_count;
   int   glcmd_count;
   int   frame_count;

   int   offset_skins;
   int   offset_texcoords;
   int   offset_triangles;
   int   offset_frames;
   int   offset_glcmds;
   int   offset_end;
};

struct PACKED _MD2_Vertex
{
   unsigned char  pos[3];
   unsigned char  normal_idx;
};

struct PACKED _MD2_Frame
{
   float       scale[3];
   float       trans[3];
   char        name[16];
   MD2_Vertex  vertices[1];
};

struct PACKED _MD2_Triangle
{
   unsigned short vertex_idx[3];
   unsigned short texcoord_idx[3];
};

struct PACKED _MD2_Texcoord
{
   short s;
   short t;
};

typedef struct _MD2_Loader
{
   Eina_File     *file;
   char          *map;
   int            size;

   int            skin_width;
   int            skin_height;

   int            frame_count;
   int            frame_size;
   char          *frames;

   int            vertex_count;
   int            triangle_count;
   int            texcoord_count;

   MD2_Triangle  *triangles;
   MD2_Texcoord   *texcoords;
} MD2_Loader;

static const float normal_table[162][3] =
{
     {-0.525731f,  0.000000f,  0.850651f},
     {-0.442863f,  0.238856f,  0.864188f},
     {-0.295242f,  0.000000f,  0.955423f},
     {-0.309017f,  0.500000f,  0.809017f},
     {-0.162460f,  0.262866f,  0.951056f},
     { 0.000000f,  0.000000f,  1.000000f},
     { 0.000000f,  0.850651f,  0.525731f},
     {-0.147621f,  0.716567f,  0.681718f},
     { 0.147621f,  0.716567f,  0.681718f},
     { 0.000000f,  0.525731f,  0.850651f},
     { 0.309017f,  0.500000f,  0.809017f},
     { 0.525731f,  0.000000f,  0.850651f},
     { 0.295242f,  0.000000f,  0.955423f},
     { 0.442863f,  0.238856f,  0.864188f},
     { 0.162460f,  0.262866f,  0.951056f},
     {-0.681718f,  0.147621f,  0.716567f},
     {-0.809017f,  0.309017f,  0.500000f},
     {-0.587785f,  0.425325f,  0.688191f},
     {-0.850651f,  0.525731f,  0.000000f},
     {-0.864188f,  0.442863f,  0.238856f},
     {-0.716567f,  0.681718f,  0.147621f},
     {-0.688191f,  0.587785f,  0.425325f},
     {-0.500000f,  0.809017f,  0.309017f},
     {-0.238856f,  0.864188f,  0.442863f},
     {-0.425325f,  0.688191f,  0.587785f},
     {-0.716567f,  0.681718f, -0.147621f},
     {-0.500000f,  0.809017f, -0.309017f},
     {-0.525731f,  0.850651f,  0.000000f},
     { 0.000000f,  0.850651f, -0.525731f},
     {-0.238856f,  0.864188f, -0.442863f},
     { 0.000000f,  0.955423f, -0.295242f},
     {-0.262866f,  0.951056f, -0.162460f},
     { 0.000000f,  1.000000f,  0.000000f},
     { 0.000000f,  0.955423f,  0.295242f},
     {-0.262866f,  0.951056f,  0.162460f},
     { 0.238856f,  0.864188f,  0.442863f},
     { 0.262866f,  0.951056f,  0.162460f},
     { 0.500000f,  0.809017f,  0.309017f},
     { 0.238856f,  0.864188f, -0.442863f},
     { 0.262866f,  0.951056f, -0.162460f},
     { 0.500000f,  0.809017f, -0.309017f},
     { 0.850651f,  0.525731f,  0.000000f},
     { 0.716567f,  0.681718f,  0.147621f},
     { 0.716567f,  0.681718f, -0.147621f},
     { 0.525731f,  0.850651f,  0.000000f},
     { 0.425325f,  0.688191f,  0.587785f},
     { 0.864188f,  0.442863f,  0.238856f},
     { 0.688191f,  0.587785f,  0.425325f},
     { 0.809017f,  0.309017f,  0.500000f},
     { 0.681718f,  0.147621f,  0.716567f},
     { 0.587785f,  0.425325f,  0.688191f},
     { 0.955423f,  0.295242f,  0.000000f},
     { 1.000000f,  0.000000f,  0.000000f},
     { 0.951056f,  0.162460f,  0.262866f},
     { 0.850651f, -0.525731f,  0.000000f},
     { 0.955423f, -0.295242f,  0.000000f},
     { 0.864188f, -0.442863f,  0.238856f},
     { 0.951056f, -0.162460f,  0.262866f},
     { 0.809017f, -0.309017f,  0.500000f},
     { 0.681718f, -0.147621f,  0.716567f},
     { 0.850651f,  0.000000f,  0.525731f},
     { 0.864188f,  0.442863f, -0.238856f},
     { 0.809017f,  0.309017f, -0.500000f},
     { 0.951056f,  0.162460f, -0.262866f},
     { 0.525731f,  0.000000f, -0.850651f},
     { 0.681718f,  0.147621f, -0.716567f},
     { 0.681718f, -0.147621f, -0.716567f},
     { 0.850651f,  0.000000f, -0.525731f},
     { 0.809017f, -0.309017f, -0.500000f},
     { 0.864188f, -0.442863f, -0.238856f},
     { 0.951056f, -0.162460f, -0.262866f},
     { 0.147621f,  0.716567f, -0.681718f},
     { 0.309017f,  0.500000f, -0.809017f},
     { 0.425325f,  0.688191f, -0.587785f},
     { 0.442863f,  0.238856f, -0.864188f},
     { 0.587785f,  0.425325f, -0.688191f},
     { 0.688191f,  0.587785f, -0.425325f},
     {-0.147621f,  0.716567f, -0.681718f},
     {-0.309017f,  0.500000f, -0.809017f},
     { 0.000000f,  0.525731f, -0.850651f},
     {-0.525731f,  0.000000f, -0.850651f},
     {-0.442863f,  0.238856f, -0.864188f},
     {-0.295242f,  0.000000f, -0.955423f},
     {-0.162460f,  0.262866f, -0.951056f},
     { 0.000000f,  0.000000f, -1.000000f},
     { 0.295242f,  0.000000f, -0.955423f},
     { 0.162460f,  0.262866f, -0.951056f},
     {-0.442863f, -0.238856f, -0.864188f},
     {-0.309017f, -0.500000f, -0.809017f},
     {-0.162460f, -0.262866f, -0.951056f},
     { 0.000000f, -0.850651f, -0.525731f},
     {-0.147621f, -0.716567f, -0.681718f},
     { 0.147621f, -0.716567f, -0.681718f},
     { 0.000000f, -0.525731f, -0.850651f},
     { 0.309017f, -0.500000f, -0.809017f},
     { 0.442863f, -0.238856f, -0.864188f},
     { 0.162460f, -0.262866f, -0.951056f},
     { 0.238856f, -0.864188f, -0.442863f},
     { 0.500000f, -0.809017f, -0.309017f},
     { 0.425325f, -0.688191f, -0.587785f},
     { 0.716567f, -0.681718f, -0.147621f},
     { 0.688191f, -0.587785f, -0.425325f},
     { 0.587785f, -0.425325f, -0.688191f},
     { 0.000000f, -0.955423f, -0.295242f},
     { 0.000000f, -1.000000f,  0.000000f},
     { 0.262866f, -0.951056f, -0.162460f},
     { 0.000000f, -0.850651f,  0.525731f},
     { 0.000000f, -0.955423f,  0.295242f},
     { 0.238856f, -0.864188f,  0.442863f},
     { 0.262866f, -0.951056f,  0.162460f},
     { 0.500000f, -0.809017f,  0.309017f},
     { 0.716567f, -0.681718f,  0.147621f},
     { 0.525731f, -0.850651f,  0.000000f},
     {-0.238856f, -0.864188f, -0.442863f},
     {-0.500000f, -0.809017f, -0.309017f},
     {-0.262866f, -0.951056f, -0.162460f},
     {-0.850651f, -0.525731f,  0.000000f},
     {-0.716567f, -0.681718f, -0.147621f},
     {-0.716567f, -0.681718f,  0.147621f},
     {-0.525731f, -0.850651f,  0.000000f},
     {-0.500000f, -0.809017f,  0.309017f},
     {-0.238856f, -0.864188f,  0.442863f},
     {-0.262866f, -0.951056f,  0.162460f},
     {-0.864188f, -0.442863f,  0.238856f},
     {-0.809017f, -0.309017f,  0.500000f},
     {-0.688191f, -0.587785f,  0.425325f},
     {-0.681718f, -0.147621f,  0.716567f},
     {-0.442863f, -0.238856f,  0.864188f},
     {-0.587785f, -0.425325f,  0.688191f},
     {-0.309017f, -0.500000f,  0.809017f},
     {-0.147621f, -0.716567f,  0.681718f},
     {-0.425325f, -0.688191f,  0.587785f},
     {-0.162460f, -0.262866f,  0.951056f},
     { 0.442863f, -0.238856f,  0.864188f},
     { 0.162460f, -0.262866f,  0.951056f},
     { 0.309017f, -0.500000f,  0.809017f},
     { 0.147621f, -0.716567f,  0.681718f},
     { 0.000000f, -0.525731f,  0.850651f},
     { 0.425325f, -0.688191f,  0.587785f},
     { 0.587785f, -0.425325f,  0.688191f},
     { 0.688191f, -0.587785f,  0.425325f},
     {-0.955423f,  0.295242f,  0.000000f},
     {-0.951056f,  0.162460f,  0.262866f},
     {-1.000000f,  0.000000f,  0.000000f},
     {-0.850651f,  0.000000f,  0.525731f},
     {-0.955423f, -0.295242f,  0.000000f},
     {-0.951056f, -0.162460f,  0.262866f},
     {-0.864188f,  0.442863f, -0.238856f},
     {-0.951056f,  0.162460f, -0.262866f},
     {-0.809017f,  0.309017f, -0.500000f},
     {-0.864188f, -0.442863f, -0.238856f},
     {-0.951056f, -0.162460f, -0.262866f},
     {-0.809017f, -0.309017f, -0.500000f},
     {-0.681718f,  0.147621f, -0.716567f},
     {-0.681718f, -0.147621f, -0.716567f},
     {-0.850651f,  0.000000f, -0.525731f},
     {-0.688191f,  0.587785f, -0.425325f},
     {-0.587785f,  0.425325f, -0.688191f},
     {-0.425325f,  0.688191f, -0.587785f},
     {-0.425325f, -0.688191f, -0.587785f},
     {-0.587785f, -0.425325f, -0.688191f},
     {-0.688191f, -0.587785f, -0.425325f},
};

static inline void
_md2_loader_fini(MD2_Loader *loader)
{
   if (loader->map)
     {
        eina_file_map_free(loader->file, loader->map);
        loader->map = NULL;
     }

   if (loader->file)
     {
        eina_file_close(loader->file);
        loader->file = NULL;
     }
}

static inline Eina_Bool
_md2_loader_init(MD2_Loader *loader, const char *file)
{
   MD2_Header header;

   memset(loader, 0x00, sizeof(MD2_Loader));

   /* Open given file. */
   loader->file = eina_file_open(file, 0);

   if (loader->file == NULL)
     {
        ERR("Failed to open file %s\n", file);
        goto error;
     }

   /* Check file size. We require a file larger than MD2 header size. */
   loader->size = eina_file_size_get(loader->file);

   if (loader->size < (int)sizeof(MD2_Header))
     goto error;

   /* Map the file. */
   loader->map = eina_file_map_all(loader->file, EINA_FILE_SEQUENTIAL);

   if (loader->map == NULL)
     goto error;

   /* Read header. */
   memcpy(&header, loader->map, sizeof(MD2_Header));

   /* Check identity */
   if (header.magic != MD2_MAGIC_NUMBER || header.version != MD2_VERSION)
     goto error;

   /* Check offsets */
   if (header.offset_skins > header.offset_end)
     goto error;

   if (header.offset_texcoords > header.offset_end)
     goto error;

   if (header.offset_triangles > header.offset_end)
     goto error;

   if (header.offset_frames > header.offset_end)
     goto error;

   if (header.offset_glcmds > header.offset_end)
     goto error;

   if (header.offset_end > loader->size)
     goto error;

   loader->skin_width = header.skin_width;
   loader->skin_height = header.skin_height;

   loader->frame_count = header.frame_count;
   loader->frame_size = header.frame_size;
   loader->frames = loader->map + header.offset_frames;

   loader->vertex_count = header.vertex_count;
   loader->triangle_count = header.triangle_count;
   loader->texcoord_count = header.texcoord_count;

   loader->triangles = (MD2_Triangle *)(loader->map + header.offset_triangles);
   loader->texcoords = (MD2_Texcoord *)(loader->map + header.offset_texcoords);
   return EINA_TRUE;

error:
   _md2_loader_fini(loader);
   return EINA_FALSE;
}

void
evas_3d_mesh_file_md2_set(Evas_3D_Mesh *mesh, const char *file)
{
   MD2_Loader           loader;
   int                  i, j, k;
   float               *pos, *nor, *tex;
   int                  stride_pos, stride_nor, stride_tex;
   float                s_scale, t_scale;

   /* Initialize MD2 loader (Open file and read MD2 head ant etc) */
   if (!_md2_loader_init(&loader, file))
     {
        ERR("Failed to initialize MD2 loader.");
        return;
     }

   s_scale = 1.0 / (float)(loader.skin_width - 1);
   t_scale = 1.0 / (float)(loader.skin_height - 1);

   evas_3d_mesh_vertex_count_set(mesh, loader.triangle_count * 3);
   evas_3d_mesh_vertex_assembly_set(mesh, EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES);

   /* Load frames */
   for (i = 0; i < loader.frame_count; i++)
     {
        const MD2_Frame *frame = (const MD2_Frame *)(loader.frames + loader.frame_size * i);
        int              f = i * MD2_FRAME_SCALE;

        /* Add a mesh frame. */
        evas_3d_mesh_frame_add(mesh, f);

        /* Allocate vertex buffer for the frame. */
        evas_3d_mesh_frame_vertex_data_copy_set(mesh, f, EVAS_3D_VERTEX_POSITION, 0, NULL);
        evas_3d_mesh_frame_vertex_data_copy_set(mesh, f, EVAS_3D_VERTEX_NORMAL,   0, NULL);
        evas_3d_mesh_frame_vertex_data_copy_set(mesh, f, EVAS_3D_VERTEX_TEXCOORD, 0, NULL);

        /* Map vertex buffer. */
        pos = (float *)evas_3d_mesh_frame_vertex_data_map(mesh, f, EVAS_3D_VERTEX_POSITION);
        nor = (float *)evas_3d_mesh_frame_vertex_data_map(mesh, f, EVAS_3D_VERTEX_NORMAL);
        tex = (float *)evas_3d_mesh_frame_vertex_data_map(mesh, f, EVAS_3D_VERTEX_TEXCOORD);

        stride_pos = evas_3d_mesh_frame_vertex_stride_get(mesh, f, EVAS_3D_VERTEX_POSITION);
        stride_nor = evas_3d_mesh_frame_vertex_stride_get(mesh, f, EVAS_3D_VERTEX_NORMAL);
        stride_tex = evas_3d_mesh_frame_vertex_stride_get(mesh, f, EVAS_3D_VERTEX_TEXCOORD);

        if (stride_pos == 0)
          stride_pos = sizeof(float) * 3;

        if (stride_nor == 0)
          stride_nor = sizeof(float) * 3;

        if (stride_tex == 0)
          stride_tex = sizeof(float) * 2;

        for (j = 0; j < loader.triangle_count; j++)
          {
             const MD2_Triangle *tri   = &loader.triangles[j];

             for (k = 0; k < 3; k++)
               {
                  unsigned int tidx, vidx;
                  float *p, *n, *t;

                  tidx = tri->texcoord_idx[k];
                  vidx = tri->vertex_idx[k];

                  p = (float *)((char *)pos + stride_pos * (j * 3 + k));
                  n = (float *)((char *)nor + stride_nor * (j * 3 + k));
                  t = (float *)((char *)tex + stride_tex * (j * 3 + k));

                  p[0] = frame->vertices[vidx].pos[0] * frame->scale[0] + frame->trans[0];
                  p[1] = frame->vertices[vidx].pos[1] * frame->scale[1] + frame->trans[1];
                  p[2] = frame->vertices[vidx].pos[2] * frame->scale[2] + frame->trans[2];

                  n[0] = normal_table[frame->vertices[vidx].normal_idx][0];
                  n[1] = normal_table[frame->vertices[vidx].normal_idx][1];
                  n[2] = normal_table[frame->vertices[vidx].normal_idx][2];

                  t[0] = loader.texcoords[tidx].s * s_scale;
                  t[1] = 1.0 - loader.texcoords[tidx].t * t_scale;
               }
          }

        /* Unmap vertex buffer. */
        evas_3d_mesh_frame_vertex_data_unmap(mesh, f, EVAS_3D_VERTEX_POSITION);
        evas_3d_mesh_frame_vertex_data_unmap(mesh, f, EVAS_3D_VERTEX_NORMAL);
        evas_3d_mesh_frame_vertex_data_unmap(mesh, f, EVAS_3D_VERTEX_TEXCOORD);
     }

   _md2_loader_fini(&loader);
}
