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
   E3D_UNIFORM_RENDER_TO_TEXTURE,
   E3D_UNIFORM_FRAME_SIZE_H,
   E3D_UNIFORM_FRAME_SIZE_W,
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
   Evas_Canvas3D_Shader_Mode   mode;

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
   GLint ok = 0;

   evas_glShaderSource_th(shader, 1, &src, NULL);
   evas_glCompileShader_th(shader);
   evas_glGetShaderiv_th(shader, GL_COMPILE_STATUS, &ok);

   if (!ok)
     {
        GLchar   *log_str;
        GLint     len;
        GLsizei   info_len;

        evas_glGetShaderiv_th(shader, GL_INFO_LOG_LENGTH, &len);
        log_str = (GLchar *)malloc(len);
        evas_glGetShaderInfoLog_th(shader, len, &info_len, log_str);
        ERR("Shader compilation failed: %s", log_str);
        DBG("Shader source was:\n%s", src);
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
     evas_glBindAttribLocation_th(program->prog, index++, "aPosition0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_POSITION_BLEND)
     evas_glBindAttribLocation_th(program->prog, index++, "aPosition1");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_NORMAL)
     evas_glBindAttribLocation_th(program->prog, index++, "aNormal0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_NORMAL_BLEND)
     evas_glBindAttribLocation_th(program->prog, index++, "aNormal1");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_TANGENT)
     evas_glBindAttribLocation_th(program->prog, index++, "aTangent0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_TANGENT_BLEND)
     evas_glBindAttribLocation_th(program->prog, index++, "aTangent1");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_COLOR)
     evas_glBindAttribLocation_th(program->prog, index++, "aColor0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_COLOR_BLEND)
     evas_glBindAttribLocation_th(program->prog, index++, "aColor1");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_TEXCOORD)
     evas_glBindAttribLocation_th(program->prog, index++, "aTexCoord0");

   if (program->flags & E3D_SHADER_FLAG_VERTEX_TEXCOORD_BLEND)
     evas_glBindAttribLocation_th(program->prog, index++, "aTexCoord1");
}

static inline Eina_Bool
_program_build(E3D_Program *program, const char *vert_src, const char *frag_src)
{
   GLint ok = 0;

   /* Create OpenGL vertex & fragment shader object. */
   program->vert = evas_glCreateShader_th(GL_VERTEX_SHADER);
   program->frag = evas_glCreateShader_th(GL_FRAGMENT_SHADER);

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
   program->prog = evas_glCreateProgram_th();

   /* Attach shaders. */
   evas_glAttachShader_th(program->prog, program->vert);
   evas_glAttachShader_th(program->prog, program->frag);

   _program_vertex_attrib_bind(program);
   /* Link program. */
   evas_glLinkProgram_th(program->prog);

   /* Check link status. */
   evas_glGetProgramiv_th(program->prog, GL_LINK_STATUS, &ok);

   if (!ok)
     {
        GLchar   *log_str;
        GLint     len = 0;
        GLsizei   info_len;

        evas_glGetProgramiv_th(program->prog, GL_INFO_LOG_LENGTH, &len);
        log_str = (GLchar *)malloc(len);
        evas_glGetProgramInfoLog_th(program->prog, len, &info_len, log_str);
        ERR("Shader link failed.\n%s", log_str);
        free(log_str);
        return EINA_FALSE;
     }

   return EINA_TRUE;
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
   "uColorTexture",
   "uFrameSizeH",
   "uFrameSizeW"
};

static inline void
_program_uniform_init(E3D_Program *program)
{
   int i;
   for (i = 0; i < E3D_UNIFORM_COUNT; i++)
     {
        program->uniform_locations[i] = evas_glGetUniformLocation_th(program->prog, uniform_names[i]);
     }
}

#define UNIFORM_MATRIX3_FOREACH(m, data)     \
        m[0] = data.xx;                      \
        m[1] = data.xy;                      \
        m[2] = data.xz;                      \
        m[3] = data.yx;                      \
        m[4] = data.yy;                      \
        m[5] = data.yz;                      \
        m[6] = data.zx;                      \
        m[7] = data.zy;                      \
        m[8] = data.zz;

#define UNIFORM_MATRIX4_FOREACH(m, data)     \
        m[0] = data.xx;                      \
        m[1] = data.xy;                      \
        m[2] = data.xz;                      \
        m[3] = data.xw;                      \
        m[4] = data.yx;                      \
        m[5] = data.yy;                      \
        m[6] = data.yz;                      \
        m[7] = data.yw;                      \
        m[8] = data.zx;                      \
        m[9] = data.zy;                      \
        m[10] = data.zz;                     \
        m[11] = data.zw;                     \
        m[12] = data.wx;                     \
        m[13] = data.wy;                     \
        m[14] = data.wz;                     \
        m[15] = data.ww;

static inline void
_uniform_upload(E3D_Uniform u, GLint loc, const E3D_Draw_Data *data)
{
#define SET_TEX_COORD_TRANSFORM_MATRIX(attrib, tn)                             \
   if (data->materials[attrib].tex##tn)                                        \
     {                                                                         \
        float   m[9];                                                          \
        UNIFORM_MATRIX3_FOREACH(m, data->materials[attrib].tex##tn->trans);    \
        evas_glUniformMatrix3fv_th(loc, 1, EINA_FALSE, &m[0]);                 \
     }

   switch (u)
     {
      case E3D_UNIFORM_MATRIX_MVP: {
         float   m[16];
         UNIFORM_MATRIX4_FOREACH(m, data->matrix_mvp);
         evas_glUniformMatrix4fv_th(loc, 1, EINA_FALSE, &m[0]);
         break;
      }
      case E3D_UNIFORM_MATRIX_MV: {
         float   m[16];
         UNIFORM_MATRIX4_FOREACH(m, data->matrix_mv);
         evas_glUniformMatrix4fv_th(loc, 1, EINA_FALSE, &m[0]);
         break;
      }
      case E3D_UNIFORM_MATRIX_NORMAL: {
         float   m[9];
         UNIFORM_MATRIX3_FOREACH(m, data->matrix_normal);
         evas_glUniformMatrix3fv_th(loc, 1, EINA_FALSE, &m[0]);
         break;
      }
      case E3D_UNIFORM_MATRIX_LIGHT: {
         float   m[16];
         UNIFORM_MATRIX4_FOREACH(m, data->matrix_light);
         evas_glUniformMatrix4fv_th(loc, 1, EINA_FALSE, &m[0]);
         break;
      }
      case E3D_UNIFORM_POSITION_WEIGHT:
         evas_glUniform1f_th(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].weight);
         break;
      case E3D_UNIFORM_NORMAL_WEIGHT:
         evas_glUniform1f_th(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL].weight);
         break;
      case E3D_UNIFORM_TANGENT_WEIGHT:
         evas_glUniform1f_th(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT].weight);
         break;
      case E3D_UNIFORM_COLOR_WEIGHT:
         evas_glUniform1f_th(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR].weight);
         break;
      case E3D_UNIFORM_TEXCOORD_WEIGHT:
         evas_glUniform1f_th(loc, data->vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD].weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_AMBIENT:
         evas_glUniform1f_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_DIFFUSE:
         evas_glUniform1f_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_SPECULAR:
         evas_glUniform1f_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_EMISSION:
         evas_glUniform1f_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_NORMAL:
         evas_glUniform1f_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_AMBIENT0:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_DIFFUSE0:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_SPECULAR0:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_EMISSION0:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_NORMAL0:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_AMBIENT1:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_DIFFUSE1:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_SPECULAR1:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_EMISSION1:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_NORMAL1:
         evas_glUniform1i_th(loc, data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL].sampler1);
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
         evas_glUniform1i_th(loc, data->smap_sampler);
         break;
      case E3D_UNIFORM_SHADOWS_PCF_STEP:
         evas_glUniform1f_th(loc, data->pcf_step);
         break;
      case E3D_UNIFORM_SHADOWS_PCF_SIZE:
         evas_glUniform1f_th(loc, data->pcf_size);
         break;
      case E3D_UNIFORM_SHADOWS_CONSTANT_BIAS:
         evas_glUniform1f_th(loc, data->constant_bias);
         break;
      case E3D_UNIFORM_LIGHT_POSITION:
         evas_glUniform4f_th(loc, data->light.position.x, data->light.position.y,
                                  data->light.position.z, data->light.position.w);
         break;
      case E3D_UNIFORM_LIGHT_SPOT_DIR:
         evas_glUniform3f_th(loc, data->light.spot_dir.x, data->light.spot_dir.y,
                                  data->light.spot_dir.z);
         break;
      case E3D_UNIFORM_LIGHT_SPOT_EXP:
         evas_glUniform1f_th(loc, data->light.spot_exp);
         break;
      case E3D_UNIFORM_LIGHT_SPOT_CUTOFF_COS:
         evas_glUniform1f_th(loc, data->light.spot_cutoff_cos);
         break;
      case E3D_UNIFORM_LIGHT_ATTENUATION:
         evas_glUniform3f_th(loc, data->light.atten.x, data->light.atten.y,
                                  data->light.atten.z);
         break;
      case E3D_UNIFORM_LIGHT_AMBIENT:
         evas_glUniform4f_th(loc,
                                data->light.ambient.r, data->light.ambient.g,
                                data->light.ambient.b, data->light.ambient.a);
         break;
      case E3D_UNIFORM_LIGHT_DIFFUSE:
         evas_glUniform4f_th(loc,
                                data->light.diffuse.r, data->light.diffuse.g,
                                data->light.diffuse.b, data->light.diffuse.a);
         break;
      case E3D_UNIFORM_LIGHT_SPECULAR:
         evas_glUniform4f_th(loc,
                                data->light.specular.r, data->light.specular.g,
                                data->light.specular.b, data->light.specular.a);
         break;
      case E3D_UNIFORM_MATERIAL_AMBIENT:
         evas_glUniform4f_th(loc,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.r,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.g,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.b,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_DIFFUSE:
         evas_glUniform4f_th(loc,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.r,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.g,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.b,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_SPECULAR:
         evas_glUniform4f_th(loc,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.r,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.g,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.b,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_EMISSION:
         evas_glUniform4f_th(loc,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color.r,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color.g,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color.b,
                                data->materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_SHININESS:
         evas_glUniform1f_th(loc, data->shininess);
         break;
      case E3D_UNIFORM_FOG_FACTOR:
         evas_glUniform1f_th(loc, data->fog_color.a);
         break;
      case E3D_UNIFORM_FOG_COLOR:
         evas_glUniform4f_th(loc, data->fog_color.r, data->fog_color.g, data->fog_color.b, 1);
         break;
      case E3D_UNIFORM_COLOR_PICK:
         evas_glUniform4f_th(loc, data->color_pick_key.r, data->color_pick_key.g,
                                  data->color_pick_key.b, 1.0);
         break;
      case E3D_UNIFORM_ALPHATEST_COMPARISON:
         evas_glUniform1i_th(loc,
                               (data->alpha_comparison ? data->alpha_comparison : EVAS_CANVAS3D_COMPARISON_GREATER));
         break;
      case E3D_UNIFORM_ALPHATEST_REFVALUE:
         evas_glUniform1f_th(loc, (data->alpha_ref_value ? data->alpha_ref_value : 0.0));
         break;
      case E3D_UNIFORM_RENDER_TO_TEXTURE:
         evas_glUniform1i_th(loc, data->colortex_sampler);
         break;
      case E3D_UNIFORM_FRAME_SIZE_H:
         evas_glUniform1f_th(loc, data->frame_size_h);
         break;
      case E3D_UNIFORM_FRAME_SIZE_W:
         evas_glUniform1f_th(loc, data->frame_size_w);
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
e3d_program_new(Evas_Canvas3D_Shader_Mode mode, E3D_Shader_Flag flags)
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

   program->prog = evas_glCreateProgram_th();
   program->vert = evas_glCreateShader_th(GL_VERTEX_SHADER);
   program->frag = evas_glCreateShader_th(GL_FRAGMENT_SHADER);
   program->mode = mode;
   program->flags = flags;

   _shader_flags_add(&vert, flags);
   _shader_string_add(&frag, vert.str);

   _shader_string_add(&vert, vertex_shaders[mode]);
   _shader_string_add(&frag, fragment_shaders[mode]);

   if (! _program_build(program, vert.str, frag.str))
     goto error;

   _program_uniform_init(program);

   _shader_string_fini(&vert);
   _shader_string_fini(&frag);

   return program;

error:
   if (program->prog)
     evas_glDeleteProgram_th(program->prog);

   if (program->vert)
     evas_glDeleteShader_th(program->vert);

   if (program->frag)
     evas_glDeleteShader_th(program->frag);

   _shader_string_fini(&vert);
   _shader_string_fini(&frag);

   free(program);

   return NULL;
}

void
e3d_program_free(E3D_Program *program)
{
   evas_glDeleteProgram_th(program->prog);
   evas_glDeleteShader_th(program->vert);
   evas_glDeleteShader_th(program->frag);
   free(program);
}

GLuint
e3d_program_id_get(const E3D_Program *program)
{
   return program->prog;
}

Evas_Canvas3D_Shader_Mode
e3d_program_shader_mode_get(const E3D_Program *program)
{
   return program->mode;
}

E3D_Shader_Flag
e3d_program_shader_flags_get(const E3D_Program *program)
{
   return program->flags;
}
