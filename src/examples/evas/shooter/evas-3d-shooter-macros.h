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

#define KEY_MOTION(way)                                      \
   {                                                         \
      for (i = 0; (motion_vec[i] > 0) && (!pressed); i++)    \
        pressed = (motion_vec[i] == way);                    \
                                                             \
      if (!pressed)                                          \
        motion_vec[i] = way;                                 \
                                                             \
      if ((way == 1) || (way == 3))                          \
        camera_move = CAMERA_MOVE;                           \
      else                                                   \
        camera_move = -CAMERA_MOVE;                          \
                                                             \
      if (way < 3)                                           \
        lateral_motion_indicator = 0;                        \
      else                                                   \
        lateral_motion_indicator = 1;                        \
   }

#define KEY_MOTION_DOWN(way)                                         \
   {                                                                 \
      while ((motion_vec[i] != way) && (i < 4))                      \
        i++;                                                         \
      for (j = i; j < 3; j++)                                        \
        motion_vec[j] = motion_vec[j + 1];                           \
      motion_vec[3] = 0;                                             \
                                                                     \
      i = 0;                                                         \
      while (motion_vec[i] > 0)                                      \
        i++;                                                         \
      if (i > 0)                                                     \
        {                                                            \
           if ((motion_vec[i - 1] == 1) || (motion_vec[i - 1] == 3)) \
             camera_move = CAMERA_MOVE;                              \
           else                                                      \
             camera_move = -CAMERA_MOVE;                             \
                                                                     \
           if (motion_vec[i-1] < 3)                                  \
             lateral_motion_indicator = 0;                           \
           else                                                      \
             lateral_motion_indicator = 1;                           \
        }                                                            \
      else                                                           \
        camera_move = 0;                                             \
   }


#define ENABLE_FOG(mesh)                                      \
   evas_canvas3d_mesh_fog_enable_set(mesh, EINA_TRUE); \
   evas_canvas3d_mesh_fog_color_set(mesh, FOG_COLOR, FOG_FACTOR);


#define DISABLE_FOG(mesh)                                      \
   evas_canvas3d_mesh_fog_enable_set(mesh, EINA_FALSE);


#define ENABLE_NORMAL(mesh)                                                  \
   evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_NORMAL_MAP);


#define DISABLE_NORMAL(mesh)                                               \
   evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);


#define ADD_MESH(Object, Name, a, d, s)                                                   \
   data->material_##Object = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);                        \
                                                                                          \
   evas_canvas3d_material_enable_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE); \
   evas_canvas3d_material_enable_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE); \
   evas_canvas3d_material_enable_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE); \
   evas_canvas3d_material_enable_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE); \
   evas_canvas3d_material_color_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, a, a, a, 1.0); \
   evas_canvas3d_material_color_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, d, d, d, 1.0); \
   evas_canvas3d_material_color_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, s, s, s, 1.0); \
   evas_canvas3d_material_shininess_set(data->material_##Object, 50.0);                                           \
                                                                                          \
   data->mesh_##Name = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);

#define SETUP_DEFAULT_MESH(Object, Name, Shade_Mode)                                      \
   evas_canvas3d_mesh_shader_mode_set(data->mesh_##Name, EVAS_CANVAS3D_SHADER_MODE_##Shade_Mode); \
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_##Name, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES); \
   evas_canvas3d_mesh_frame_material_set(data->mesh_##Name, 0, data->material_##Object);


#define SETUP_MESH_NODE(Name)                                                             \
   data->mesh_node_##Name = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));     \
                                                                                          \
   evas_canvas3d_node_mesh_add(data->mesh_node_##Name, data->mesh_##Name);


#define MATERIAL_TEXTURE_SET(Object, Name, file, image)                                   \
   efl_file_set(data->mesh_##Name, file, NULL);                                                       \
                                                                                          \
   SETUP_DEFAULT_MESH(Object, Name, PHONG)                                                \
   data->texture_diffuse_##Object = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);                  \
                                                                                          \
   evas_canvas3d_texture_atlas_enable_set(data->texture_diffuse_##Object, EINA_FALSE); \
   efl_file_set(data->texture_diffuse_##Object, image, NULL); \
   evas_canvas3d_texture_filter_set(data->texture_diffuse_##Object, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, \
                                    EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST); \
   evas_canvas3d_texture_wrap_set(data->texture_diffuse_##Object, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);   \
                                                                                          \
   evas_canvas3d_material_texture_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, \
                                      data->texture_diffuse_##Object); \
   evas_canvas3d_material_texture_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, \
                                      data->texture_diffuse_##Object);


#define CUBE_TEXTURE_SET(Object, Name, vertex, image)                                           \
   evas_canvas3d_mesh_from_primitive_set(data->mesh_##Name, 0, data->cube_primitive); \
   evas_canvas3d_mesh_frame_vertex_data_set(data->mesh_##Name, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD, \
                                            2 * sizeof(float), vertex);                        \
   SETUP_DEFAULT_MESH(Object, Name, NORMAL_MAP)                                                 \
   data->texture_diffuse_##Object = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);                        \
                                                                                                \
   evas_canvas3d_texture_atlas_enable_set(data->texture_diffuse_##Object, EINA_FALSE); \
   efl_file_set(data->texture_diffuse_##Object, image, NULL); \
   evas_canvas3d_texture_filter_set(data->texture_diffuse_##Object, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, \
                                    EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST); \
   evas_canvas3d_texture_wrap_set(data->texture_diffuse_##Object, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);         \
                                                                                                \
   evas_canvas3d_material_texture_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, \
                                      data->texture_diffuse_##Object); \
   evas_canvas3d_material_texture_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, \
                                      data->texture_diffuse_##Object);


#define NORMAL_SET(Object, Name, normal)                                                  \
   data->texture_normal_##Object = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);                   \
                                                                                          \
   evas_canvas3d_texture_atlas_enable_set(data->texture_normal_##Object, EINA_FALSE); \
   efl_file_set(data->texture_normal_##Object, normal, NULL); \
   evas_canvas3d_texture_filter_set(data->texture_normal_##Object, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, \
                                    EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST); \
   evas_canvas3d_texture_wrap_set(data->texture_normal_##Object, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);   \
                                                                                          \
   evas_canvas3d_material_texture_set(data->material_##Object, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, \
                                      data->texture_normal_##Object);                    \
                                                                                          \
   evas_canvas3d_mesh_shader_mode_set(data->mesh_##Name, EVAS_CANVAS3D_SHADER_MODE_NORMAL_MAP);

#define TEXTCOORDS_SET(Name, fb1, fb2, lr1, lr2, tb1, tb2)                      \
   static float  Name##_textcoords[] =                                          \
   {                                                                            \
   /* Front */                                                                  \
   0.0,  0.0,                                                                   \
   fb1,  0.0,                                                                   \
   fb1,  fb2,                                                                   \
   0.0,  fb2,                                                                   \
                                                                                \
   /* Left */                                                                   \
   lr1,  0.0,                                                                   \
   lr1,  lr2,                                                                   \
   0.0,  lr2,                                                                   \
   0.0,  0.0,                                                                   \
                                                                                \
   /* Back */                                                                   \
   0.0,  0.0,                                                                   \
   fb1,  0.0,                                                                   \
   fb1,  fb2,                                                                   \
   0.0,  fb2,                                                                   \
                                                                                \
   /* Right */                                                                  \
   0.0,  lr2,                                                                   \
   0.0,  0.0,                                                                   \
   lr1,  0.0,                                                                   \
   lr1,  lr2,                                                                   \
                                                                                \
   /* Top */                                                                    \
   0.0,  0.0,                                                                   \
   0.0,  tb2,                                                                   \
   tb1,  tb2,                                                                   \
   tb1,  0.0,                                                                   \
                                                                                \
   /* Bottom */                                                                 \
   tb1,  0.0,                                                                   \
   tb1,  tb2,                                                                   \
   0.0,  tb2,                                                                   \
   0.0,  0.0,                                                                   \
   };

