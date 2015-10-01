#include "evas_gl_3d_private.h"
#include "shader_3d/evas_gl_3d_shaders.x"

typedef enum _E3D_Uniform
{
   E3D_UNIFORM_MATRIX_MVP,
   E3D_UNIFORM_MATRIX_MV,
   E3D_UNIFORM_MATRIX_NORMAL,
   E3D_UNIFORM_MATRIX_LIGHT,

   E3D_UNIFORM_POSITION_WEIGHT,
   E3D_UNIFORM_NORMAL_WEIGHT,
   E3D_UNIFORM_TANGENT_WEIGHT,
   E3D_UNIFORM_COLOR_WEIGHT,
   E3D_UNIFORM_TEXCOORD_WEIGHT,

   E3D_UNIFORM_TEXTURE_WEIGHT_AMBIENT,
   E3D_UNIFORM_TEXTURE_WEIGHT_DIFFUSE,
   E3D_UNIFORM_TEXTURE_WEIGHT_SPECULAR,
   E3D_UNIFORM_TEXTURE_WEIGHT_EMISSION,
   E3D_UNIFORM_TEXTURE_WEIGHT_NORMAL,

   E3D_UNIFORM_TEXTURE_AMBIENT0,
   E3D_UNIFORM_TEXTURE_DIFFUSE0,
   E3D_UNIFORM_TEXTURE_SPECULAR0,
   E3D_UNIFORM_TEXTURE_EMISSION0,
   E3D_UNIFORM_TEXTURE_NORMAL0,

   E3D_UNIFORM_TEXTURE_AMBIENT1,
   E3D_UNIFORM_TEXTURE_DIFFUSE1,
   E3D_UNIFORM_TEXTURE_SPECULAR1,
   E3D_UNIFORM_TEXTURE_EMISSION1,
   E3D_UNIFORM_TEXTURE_NORMAL1,

   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_AMBIENT0,
   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_DIFFUSE0,
   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_SPECULAR0,
   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_EMISSION0,
   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_NORMAL0,

   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_AMBIENT1,
   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_DIFFUSE1,
   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_SPECULAR1,
   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_EMISSION1,
   E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_NORMAL1,

   E3D_UNIFORM_SHADOWMAP,
   E3D_UNIFORM_SHADOWS_PCF_STEP,
   E3D_UNIFORM_SHADOWS_PCF_SIZE,
   E3D_UNIFORM_SHADOWS_CONSTANT_BIAS,

   E3D_UNIFORM_LIGHT_POSITION,
   E3D_UNIFORM_LIGHT_SPOT_DIR,
   E3D_UNIFORM_LIGHT_SPOT_EXP,
   E3D_UNIFORM_LIGHT_SPOT_CUTOFF_COS,
   E3D_UNIFORM_LIGHT_ATTENUATION,
   E3D_UNIFORM_LIGHT_AMBIENT,
   E3D_UNIFORM_LIGHT_DIFFUSE,
   E3D_UNIFORM_LIGHT_SPECULAR,

   E3D_UNIFORM_MATERIAL_AMBIENT,
   E3D_UNIFORM_MATERIAL_DIFFUSE,
   E3D_UNIFORM_MATERIAL_SPECULAR,
   E3D_UNIFORM_MATERIAL_EMISSION,
   E3D_UNIFORM_MATERIAL_SHININESS,
   E3D_UNIFORM_FOG_FACTOR,
   E3D_UNIFORM_FOG_COLOR,
   E3D_UNIFORM_COLOR_PICK,
   E3D_UNIFORM_ALPHATEST_COMPARISON,
   E3D_UNIFORM_ALPHATEST_REFVALUE,

   E3D_UNIFORM_COUNT,
} E3D_Uniform;

static const char *shader_flag_names[] =
{
   "NORMALIZE_NORMALS",
   "VERTEX_POSITION",
   "VERTEX_POSITION_BLEND",
   "VERTEX_NORMAL",
   "VERTEX_NORMAL_BLEND",
   "VERTEX_TANGENT",
   "VERTEX_TANGENT_BLEND",
   "VERTEX_COLOR",
   "VERTEX_COLOR_BLEND",
   "VERTEX_TEXCOORD",
   "VERTEX_TEXCOORD_BLEND",
   "LIGHT_DIRECTIONAL",
   "LIGHT_SPOT",
   "LIGHT_ATTENUATION",
   "AMBIENT",
   "DIFFUSE",
   "SPECULAR",
   "EMISSION",
   "DIFFUSE_TEXTURE",
   "AMBIENT_TEXTURE",
   "SPECULAR_TEXTURE",
   "EMISSION_TEXTURE",
   "NORMAL_TEXTURE",
   "DIFFUSE_TEXTURE_BLEND",
   "AMBIENT_TEXTURE_BLEND",
   "SPECULAR_TEXTURE_BLEND",
   "EMISSION_TEXTURE_BLEND",
   "NORMAL_TEXTURE_BLEND",
   "FOG_ENABLED",
   "ALPHA_TEST_ENABLED",
   "SHADOWED"
};

typedef struct _E3D_Shader_String
{
   char *str;
   int   size;
   int   count;
} E3D_Shader_String;

struct _E3D_Program
{
   GLuint               vert;
   GLuint               frag;
   GLuint               prog;

   E3D_Shader_Flag      flags;
   Evas_Canvas3D_Shade_Mode   mode;

   GLint                uniform_locations[E3D_UNIFORM_COUNT];
};

static void _shader_string_init(E3D_Shader_String *string)
{
   string->str = NULL;
   string->size = 0;
   string->count = 0;
}

static void _shader_string_fini(E3D_Shader_String *string)
{
   if (string->str)
     {
        free(string->str);
        _shader_string_init(string);
     }
}

void _shader_string_add(E3D_Shader_String *shader, const char *str)
{
   int len;

   if (str == NULL)
     return;

   len = strlen(str);

   if ((shader->size - shader->count) < len)
     {
        int new_size = (shader->count + len) * 2;
        char *new_buf = (char *)malloc(new_size + 1);

        if (shader->str)
          {
             memcpy(new_buf, shader->str, sizeof(char) * shader->count);
             free(shader->str);
          }

        shader->str = new_buf;
        shader->size = new_size;
     }

   memcpy(&shader->str[shader->count], str, len + 1);
   shader->count += len;
}

void _shader_flags_add(E3D_Shader_String *shader, E3D_Shader_Flag flags)
{
   int i;

   for (i = 0; i < E3D_SHADER_FLAG_COUNT; i++)
     if (flags & (1 << i))
       {
          int len = strlen("#define ") + strlen(shader_flag_names[i]) + 2;
          char str[len];
          snprintf(str, len, "#define %s\n", shader_flag_names[i]);
          _shader_string_add(shader, str);
       }

   if(_flags_need_tex_coord(flags))
     _shader_string_add(shader, "#define NEED_TEX_COORD\n");
}

static inline Eina_Bool
_shader_compile(GLuint shader, const char *src)
{
   GLint ok;

   glShaderSource(shader, 1, &src, NULL);
   glCompileShader(shader);
   glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

   if (!ok)
     {
        GLchar   *log_str;
        GLint     len;
        GLsizei   info_len;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        log_str = (GLchar *)malloc(len);
        glGetShaderInfoLog(shader, len, &info_len, log_str);
        ERR("Shader compilation failed.\n%s", log_str);
        free(log_str);

        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static inline Eina_Bool
_program_build(E3D_Program *program, const char *vert_src, const char *frag_src)
{
   GLint ok;

   /* Create OpenGL vertex & fragment shader object. */
   program->vert = glCreateShader(GL_VERTEX_SHADER);
   program->frag = glCreateShader(GL_FRAGMENT_SHADER);

   /* Commpile vertex shader. */
   if (!_shader_compile(program->vert, vert_src))
     {
        ERR("Faield to compile vertex shader.");
        return EINA_FALSE;
     }

   /* Compile fragment shader. */
   if (!_shader_compile(program->frag, frag_src))
     {
        ERR("Failed to compile fragment shader.");
        return EINA_FALSE;
     }

   /* Create OpenGL program object. */
   program->prog = glCreateProgram();

   /* Attach shaders. */
   glAttachShader(program->prog, program->vert);
   glAttachShader(program->prog, program->frag);

   /* Link program. */
   glLinkProgram(program->prog);

   /* Check link status. */
   glGetProgramiv(program->prog, GL_LINK_STATUS, &ok);

   if (!ok)
     {
        GLchar   *log_str;
        GLint     len;
        GLsizei   info_len;

        glGetProgramiv(program->prog, GL_INFO_LOG_LENGTH, &len);
        log_str = (GLchar *)malloc(len);
        glGetProgramInfoLog(program->prog, len, &info_len, log_str);
        ERR("Shader link failed.\n%s", log_str);
        free(log_str);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static inline void
_program_vertex_attrib_bind(E3D_Program *program)
{
   GLint index = 0;

   if (program->flags & E3D_SHADER_FLAG_VERTEX_POSITION)
     glBindAttribLocation(program->prog, index++, "aPosition0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_POSITION_BLEND)
     glBindAttribLocation(program->prog, index++, "aPosition1");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_NORMAL)
     glBindAttribLocation(program->prog, index++, "aNormal0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_NORMAL_BLEND)
     glBindAttribLocation(program->prog, index++, "aNormal1");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_TANGENT)
     glBindAttribLocation(program->prog, index++, "aTangent0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_TANGENT_BLEND)
     glBindAttribLocation(program->prog, index++, "aTangent1");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_COLOR)
     glBindAttribLocation(program->prog, index++, "aColor0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_COLOR_BLEND)
     glBindAttribLocation(program->prog, index++, "aColor1");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_TEXCOORD)
     glBindAttribLocation(program->prog, index++, "aTexCoord0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_TEXCOORD_BLEND)
     glBindAttribLocation(program->prog, index++, "aTexCoord1");
}

static const char *uniform_names[] =
{
   "uMatrixMvp",
   "uMatrixModelview",
   "uMatrixNormal",
   "uMatrixLight",
   "uPositionWeight",
   "uNormalWeight",
   "uTangentWeight",
   "uColorWeight",
   "uTexCoordWeight",
   "uTextureAmbientWeight",
   "uTextureDiffuseWeight",
   "uTextureSpecularWeight",
   "uTextureEmissionWeight",
   "uTextureNormalWeight",
   "uTextureAmbient0",
   "uTextureDiffuse0",
   "uTextureSpecular0",
   "uTextureEmission0",
   "uTextureNormal0",
   "uTextureAmbient1",
   "uTextureDiffuse1",
   "uTextureSpecular1",
   "uTextureEmission1",
   "uTextureNormal1",
   "uTextureMatrixTransformAmbient0",
   "uTextureMatrixTransformDiffuse0",
   "uTextureMatrixTransformSpecular0",
   "uTextureMatrixTransformEmission0",
   "uTextureMatrixTransformNormal0",
   "uTextureMatrixTransformAmbient1",
   "uTextureMatrixTransformDiffuse1",
   "uTextureMatrixTransformSpecular1",
   "uTextureMatrixTransformEmission1",
   "uTextureMatrixTransformNormal1",
   "uShadowMap",
   "uShadowsPCFStep",
   "uShadowsPCFSize",
   "uShadowsConstantBias",
   "uLightPosition",
   "uLightSpotDir",
   "uLightSpotExp",
   "uLightSpotCutoffCos",
   "uLightAtten",
   "uLightAmbient",
   "uLightDiffuse",
   "uLightSpecular",
   "uMaterialAmbient",
   "uMaterialDiffuse",
   "uMaterialSpecular",
   "uMaterialEmission",
   "uMaterialShininess",
   "uFogFactor",
   "uFogColor",
   "uColorPick",
   "uAlphaTestComparison",
   "uAlphaTestRefValue",
};

static inline void
_program_uniform_init(E3D_Program *program)
{
   int i;
   for (i = 0; i < E3D_UNIFORM_COUNT; i++)
     {
        program->uniform_locations[i] = glGetUniformLocation(program->prog, uniform_names[i]);
     }
}

static inline void
_uniform_upload(E3D_Uniform u, GLint loc, const E3D_Draw_Data *data)
{
#define SET_TEX_COORD_TRANSFORM_MATRIX(attrib, tn)                             \
   if (data->materials[attrib].tex##tn)                                        \
     {                                                                         \
        float   m[9];                                                          \
        for(int i = 0 ; i < 9 ; i++)                                           \
          m[i] = data->materials[attrib].tex##tn->trans.m[i];                  \
        glUniformMatrix3fv(loc, 1, EINA_FALSE, &m[0]);                         \
     }

   switch (u)
     {
      case E3D_UNIFORM_MATRIX_MVP: {
         float   m[16];
         for(int i = 0 ; i <16 ; i++)
            m[i] = data->matrix_mvp.m[i];
         glUniformMatrix4fv(loc, 1, EINA_FALSE, &m[0]);
         break;
      }
      case E3D_UNIFORM_MATRIX_MV: {
         float   m[16];
         for(int i = 0 ; i <16 ; i++)
            m[i] = data->matrix_mv.m[i];
         glUniformMatrix4fv(loc, 1, EINA_FALSE, &m[0]);
         break;
      }
      case E3D_UNIFORM_MATRIX_NORMAL: {
         float   m[9];
         for(int i = 0 ; i <9 ; i++)
            m[i] = data->matrix_normal.m[i];
         glUniformMatrix3fv(loc, 1, EINA_FALSE, &m[0]);
         break;
      }
      case E3D_UNIFORM_MATRIX_LIGHT: {
         float   m[16];
         for(int i = 0 ; i <16 ; i++)
            m[i] = data->matrix_light.m[i];
         glUniformMatrix4fv(loc, 1, EINA_FALSE, &m[0]);
         break;
      }
      case E3D_UNIFORM_POSITION_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].weight);
         break;
      case E3D_UNIFORM_NORMAL_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL].weight);
         break;
      case E3D_UNIFORM_TANGENT_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT].weight);
         break;
      case E3D_UNIFORM_COLOR_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR].weight);
         break;
      case E3D_UNIFORM_TEXCOORD_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD].weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_AMBIENT:
         glUniform1f(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_DIFFUSE:
         glUniform1f(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_SPECULAR:
         glUniform1f(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_EMISSION:
         glUniform1f(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_NORMAL:
         glUniform1f(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_AMBIENT0:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_DIFFUSE0:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_SPECULAR0:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_EMISSION0:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_NORMAL0:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_AMBIENT1:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_DIFFUSE1:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_SPECULAR1:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_EMISSION1:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_NORMAL1:
         glUniform1i(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_AMBIENT0: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_DIFFUSE0: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_SPECULAR0: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 0)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_EMISSION0: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION, 0)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_NORMAL0: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, 0)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_AMBIENT1: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 1)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_DIFFUSE1: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_SPECULAR1: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_EMISSION1: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION, 1)
         break;
      }
      case E3D_UNIFORM_TEXTURE_MATRIX_TRANSFORM_NORMAL1: {
         SET_TEX_COORD_TRANSFORM_MATRIX(EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, 1)
         break;
      }
      case E3D_UNIFORM_SHADOWMAP:
         glUniform1i(loc, data->smap_sampler);
         break;
      case E3D_UNIFORM_SHADOWS_PCF_STEP:
         glUniform1f(loc, data->pcf_step);
         break;
      case E3D_UNIFORM_SHADOWS_PCF_SIZE:
         glUniform1f(loc, data->pcf_size);
         break;
      case E3D_UNIFORM_SHADOWS_CONSTANT_BIAS:
         glUniform1f(loc, data->constant_bias);
         break;
      case E3D_UNIFORM_LIGHT_POSITION:
         glUniform4f(loc, data->light.position.x, data->light.position.y,
                     data->light.position.z, data->light.position.w);
         break;
      case E3D_UNIFORM_LIGHT_SPOT_DIR:
         glUniform3f(loc, data->light.spot_dir.x, data->light.spot_dir.y, data->light.spot_dir.z);
         break;
      case E3D_UNIFORM_LIGHT_SPOT_EXP:
         glUniform1f(loc, data->light.spot_exp);
         break;
      case E3D_UNIFORM_LIGHT_SPOT_CUTOFF_COS:
         glUniform1f(loc, data->light.spot_cutoff_cos);
         break;
      case E3D_UNIFORM_LIGHT_ATTENUATION:
         glUniform3f(loc, data->light.atten.x, data->light.atten.y, data->light.atten.z);
         break;
      case E3D_UNIFORM_LIGHT_AMBIENT:
         glUniform4f(loc,
                     data->light.ambient.r, data->light.ambient.g,
                     data->light.ambient.b, data->light.ambient.a);
         break;
      case E3D_UNIFORM_LIGHT_DIFFUSE:
         glUniform4f(loc,
                     data->light.diffuse.r, data->light.diffuse.g,
                     data->light.diffuse.b, data->light.diffuse.a);
         break;
      case E3D_UNIFORM_LIGHT_SPECULAR:
         glUniform4f(loc,
                     data->light.specular.r, data->light.specular.g,
                     data->light.specular.b, data->light.specular.a);
         break;
      case E3D_UNIFORM_MATERIAL_AMBIENT:
         glUniform4f(loc,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.r,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.g,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.b,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_DIFFUSE:
         glUniform4f(loc,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.r,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.g,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.b,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_SPECULAR:
         glUniform4f(loc,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.r,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.g,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.b,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_EMISSION:
         glUniform4f(loc,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color.r,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color.g,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color.b,
                     data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_SHININESS:
         glUniform1f(loc, data->shininess);
         break;
      case E3D_UNIFORM_FOG_FACTOR:
         glUniform1f(loc, data->fog_color.a);
         break;
      case E3D_UNIFORM_FOG_COLOR:
         glUniform4f(loc, data->fog_color.r, data->fog_color.g, data->fog_color.b, 1);
         break;
#ifndef GL_GLES
      case E3D_UNIFORM_COLOR_PICK:
         glUniform1f(loc, data->color_pick_key);
         break;
#else
      case E3D_UNIFORM_COLOR_PICK:
         glUniform4f(loc, data->color_pick_key.r, data->color_pick_key.g,
                     data->color_pick_key.b, 1.0);
         break;
#endif
      case E3D_UNIFORM_ALPHATEST_COMPARISON:
         glUniform1i(loc,
                    (data->alpha_comparison ? data->alpha_comparison : EVAS_CANVAS3D_COMPARISON_GREATER));
         break;
      case E3D_UNIFORM_ALPHATEST_REFVALUE:
         glUniform1f(loc, (data->alpha_ref_value ? data->alpha_ref_value : 0.0));
         break;
      default:
         ERR("Invalid uniform ID.");
         break;
     }
#undef SET_TEX_SHIFT
}

void
e3d_program_uniform_upload(E3D_Program *program, const E3D_Draw_Data *data)
{
   int i;
   for (i = 0; i < E3D_UNIFORM_COUNT; i++)
     {
        if (program->uniform_locations[i] != -1)
          {
             _uniform_upload(i, program->uniform_locations[i], data);
          }
     }
}

E3D_Program *
e3d_program_new(Evas_Canvas3D_Shade_Mode mode, E3D_Shader_Flag flags)
{
   E3D_Shader_String vert, frag;
   E3D_Program *program = NULL;

   _shader_string_init(&vert);
   _shader_string_init(&frag);

   program = (E3D_Program *)calloc(1, sizeof(E3D_Program));

   if (program == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   program->prog = glCreateProgram();
   program->vert = glCreateShader(GL_VERTEX_SHADER);
   program->frag = glCreateShader(GL_FRAGMENT_SHADER);
   program->mode = mode;
   program->flags = flags;

   _shader_flags_add(&vert, flags);
   _shader_string_add(&frag, vert.str);

   _shader_string_add(&vert, vertex_shaders[mode]);
   _shader_string_add(&frag, fragment_shaders[mode]);

   if (! _program_build(program, vert.str, frag.str))
     goto error;

   _program_vertex_attrib_bind(program);
   _program_uniform_init(program);

   _shader_string_fini(&vert);
   _shader_string_fini(&frag);

   return program;

error:
   if (program->prog)
     glDeleteProgram(program->prog);

   if (program->vert)
     glDeleteShader(program->vert);

   if (program->frag)
     glDeleteShader(program->frag);

   _shader_string_fini(&vert);
   _shader_string_fini(&frag);

   free(program);

   return NULL;
}

void
e3d_program_free(E3D_Program *program)
{
   glDeleteProgram(program->prog);
   glDeleteShader(program->vert);
   glDeleteShader(program->frag);
   free(program);
}

GLuint
e3d_program_id_get(const E3D_Program *program)
{
   return program->prog;
}

Evas_Canvas3D_Shade_Mode
e3d_program_shade_mode_get(const E3D_Program *program)
{
   return program->mode;
}

E3D_Shader_Flag
e3d_program_shader_flags_get(const E3D_Program *program)
{
   return program->flags;
}
