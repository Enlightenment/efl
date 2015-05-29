#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define  FOG_COLOR        0.5, 0.5, 0.5
#define  FOG_FACTOR       0.01
#define  WIDTH            1920
#define  HEIGHT           1080
#define  SOLDIER_RADIUS 5.3                   /* soldier couldn't get сloser to camera than SOLDIER_RADIUS */
#define  COLUMN_RADIUS    3.0                 /* warrior couldn't get сloser to any column than WARRIOR_COLUMN */
#define  WALL_RADIUS      2.2                 /* camera couldn't get сloser to wall than WALL_RADIUS */
#define  JUMP_HIGHT       4
#define  GUN_DIR_X        0
#define  GUN_DIR_Y        -2
#define  GUN_DIR_Z        105
#define  ROCKET_POS_X     0.4
#define  ROCKET_POS_Y     -3.1
#define  ROCKET_POS_Z     0.0
#define  TOMMY_POS_X      1.0
#define  TOMMY_POS_Y      -3.3
#define  TOMMY_POS_Z      -9.0
#define  TOMMY_JUMP_X     0.7
#define  TOMMY_JUMP_Y     3.3
#define  TOMMY_JUMP_Z     -4.1


#define  GUN_SPEED        0.001
#define  WARRIOR_SPEED    0.001
#define  WARRIOR_SPEED2   0.003
#define  SNAKE_SPEED      0.1
#define  SNAKE_SPEED2     0.001
#define  ROCKET_SPEED     2
#define  TO_RADIAN        M_PI / 180
#define  JUMP             0.2
#define  STEP             2.5
#define  FALL_SPEED_UP    0.04
#define  CAMERA_SPEED     2
#define  CAMERA_MOVE      0.3

#define  SOLDIER_FRAME_X1 -43
#define  SOLDIER_FRAME_X2 76
#define  SOLDIER_FRAME_Z1 -55
#define  SOLDIER_FRAME_Z2 30.3
#define  ROCKET_FRAME_X1  -47
#define  ROCKET_FRAME_X2  80
#define  ROCKET_FRAME_Z1  -59
#define  ROCKET_FRAME_Z2  34
#define  SNAKE_FRAME_X1   -47
#define  SNAKE_FRAME_X2   79
#define  SNAKE_FRAME_Z1   -59
#define  SNAKE_FRAME_Z2   32

typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

typedef struct _vec2
{
    float   x;
    float   y;
} vec2;

#define KEY_MOTION(lateral, camera)               \
   {                                              \
      lateral_motion_indicator = lateral;         \
      camera_move = camera;                       \
   }


#define ENABLE_FOG(mesh)                                      \
   eo_do(mesh, evas_3d_mesh_fog_enable_set(EINA_TRUE),        \
         evas_3d_mesh_fog_color_set(FOG_COLOR, FOG_FACTOR));


#define DISABLE_FOG(mesh)                                      \
   eo_do(mesh, evas_3d_mesh_fog_enable_set(EINA_FALSE));


#define ENABLE_NORMAL(mesh)                                                  \
   eo_do(mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP));


#define DISABLE_NORMAL(mesh)                                               \
   eo_do(mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));


#define SETUP_MESH(Object, Name, a, d, s)                                                 \
   data->material_##Object = eo_add(EVAS_3D_MATERIAL_CLASS, evas);                        \
                                                                                          \
   eo_do(data->material_##Object,                                                         \
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),                \
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),                \
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),               \
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),                 \
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, a, a, a, 1.0),              \
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,  d, d, d, 1.0),             \
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, s, s, s, 1.0),             \
         evas_3d_material_shininess_set(50.0));                                           \
                                                                                          \
   data->mesh_##Name = eo_add(EVAS_3D_MESH_CLASS, evas);                                  \
                                                                                          \
   eo_do(data->mesh_##Name,                                                               \
         evas_3d_mesh_frame_add(0),                                                       \
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),                         \
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),             \
         evas_3d_mesh_frame_material_set(0, data->material_##Object));


#define SETUP_MESH_NODE(Name)                                                             \
   data->mesh_node_##Name = eo_add(EVAS_3D_NODE_CLASS, evas,                              \
                                   evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));     \
                                                                                          \
   eo_do(data->mesh_node_##Name,                                                          \
         evas_3d_node_mesh_add(data->mesh_##Name));


#define MATERIAL_TEXTURE_SET(Object, Name, file, image)                                   \
   eo_do(data->mesh_##Name,                                                               \
         efl_file_set(file, NULL),                                                        \
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),                           \
         evas_3d_mesh_frame_material_set(0, data->material_##Object));                    \
                                                                                          \
   data->texture_diffuse_##Object = eo_add(EVAS_3D_TEXTURE_CLASS, evas);                  \
                                                                                          \
   eo_do(data->texture_diffuse_##Object,                                                  \
         evas_3d_texture_file_set(image, NULL),                                           \
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,                       \
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),                      \
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));   \
                                                                                          \
   eo_do(data->material_##Object,                                                         \
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE,                           \
                                      data->texture_diffuse_##Object),                    \
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_AMBIENT,                           \
                                      data->texture_diffuse_##Object));


#define CUBE_TEXTURE_SET(Object, Name, v_count, vertex1, vertex2, i_count, index, image)        \
   int textcoords_count = 12;                                                                   \
   int pos_count = 12;                                                                          \
   if (&vertex1[10] != vertex2)                                                                 \
     {                                                                                          \
        textcoords_count =2;                                                                    \
        pos_count = 10;                                                                         \
     }                                                                                          \
   eo_do(data->mesh_##Name,                                                                     \
         evas_3d_mesh_vertex_count_set(v_count),                                                \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,                         \
                                            pos_count * sizeof(float), &vertex1[ 0]),           \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,                           \
                                            pos_count * sizeof(float), &vertex1[ 3]),           \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,                            \
                                            pos_count * sizeof(float), &vertex1[ 6]),           \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,                         \
                                            textcoords_count * sizeof(float), vertex2),         \
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,                       \
                                     i_count, &index[0]),                                       \
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP));                           \
                                                                                                \
   data->texture_diffuse_##Object = eo_add(EVAS_3D_TEXTURE_CLASS, evas);                        \
                                                                                                \
   eo_do(data->texture_diffuse_##Object,                                                        \
         evas_3d_texture_file_set(image, NULL),                                                 \
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,                             \
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),                            \
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));         \
                                                                                                \
   eo_do(data->material_##Object,                                                               \
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE,                                 \
                                      data->texture_diffuse_##Object),                          \
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_AMBIENT,                                 \
                                      data->texture_diffuse_##Object));


#define NORMAL_SET(Object, Name, normal)                                                  \
   data->texture_normal_##Object = eo_add(EVAS_3D_TEXTURE_CLASS, evas);                   \
                                                                                          \
   eo_do(data->texture_normal_##Object,                                                   \
         evas_3d_texture_file_set(normal, NULL),                                          \
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,                       \
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),                      \
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));   \
                                                                                          \
   eo_do(data->material_##Object,                                                         \
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL,                            \
                                      data->texture_normal_##Object));                    \
                                                                                          \
   eo_do(data->mesh_##Name,                                                               \
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP));

#define SPHERE_SET(Name)                                                                   \
   eo_do(data->mesh_##Name,                                                                \
         evas_3d_mesh_vertex_count_set(vertex_count),                                      \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,                    \
                                            sizeof(vertex), &vertices[0].position),        \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,                      \
                                            sizeof(vertex), &vertices[0].normal),          \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TANGENT,                     \
                                            sizeof(vertex), &vertices[0].tangent),         \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,                       \
                                            sizeof(vertex), &vertices[0].color);           \
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,                    \
                                            sizeof(vertex), &vertices[0].texcoord),        \
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,                  \
                                     index_count, &indices[0]));

#define TEXTCOORDS_SET(Name, fb1, fb2, lr1, lr2, tb1, tb2)                      \
   static float  Name##_textcoords[] =                                          \
   {                                                                            \
   /* Front */                                                                  \
   0.0,  0.0,                                                                   \
   fb1,  0.0,                                                                   \
   0.0,  fb2,                                                                   \
   fb1,  fb2,                                                                   \
                                                                                \
   /* Back */                                                                   \
   0.0,  fb2,                                                                   \
   fb1,  fb2,                                                                   \
   0.0,  0.0,                                                                   \
   fb1,  0.0,                                                                   \
                                                                                \
   /* Left */                                                                   \
   0.0,  lr2,                                                                   \
   lr1,  lr2,                                                                   \
   0.0,  0.0,                                                                   \
   lr1,  0.0,                                                                   \
                                                                                \
   /* Right */                                                                  \
   0.0,  lr2,                                                                   \
   lr1,  lr2,                                                                   \
   0.0,  0.0,                                                                   \
   lr1,  0.0,                                                                   \
                                                                                \
   /* Top */                                                                    \
   0.0,  tb2,                                                                   \
   tb1,  tb2,                                                                   \
   0.0,  0.0,                                                                   \
   tb1,  0.0,                                                                   \
                                                                                \
   /* Bottom */                                                                 \
   0.0,  tb2,                                                                   \
   tb1,  tb2,                                                                   \
   0.0,  0.0,                                                                   \
   tb1,  0.0,                                                                   \
   };

