#include "evas_gl_3d_private.h"

typedef enum _E3D_Uniform
{
   E3D_UNIFORM_MATRIX_MVP,
   E3D_UNIFORM_MATRIX_MV,
   E3D_UNIFORM_MATRIX_NORMAL,

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

   E3D_UNIFORM_COUNT,
} E3D_Uniform;

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
   Evas_3D_Shade_Mode   mode;

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

#define ADD_LINE(str)   _shader_string_add(shader, str"\n")

static void
_vertex_shader_string_variable_add(E3D_Shader_String *shader,
                                   Evas_3D_Shade_Mode mode, E3D_Shader_Flag flags)
{
   ADD_LINE("uniform mat4  uMatrixMvp;");

   /* Vertex attributes. */
   if (flags & E3D_SHADER_FLAG_VERTEX_POSITION)
     ADD_LINE("attribute   vec4  aPosition0;");

   if (flags & E3D_SHADER_FLAG_VERTEX_POSITION_BLEND)
     {
        ADD_LINE("attribute   vec4  aPosition1;");
        ADD_LINE("uniform     float uPositionWeight;");
     }

   if (flags & E3D_SHADER_FLAG_VERTEX_NORMAL)
     ADD_LINE("attribute   vec4  aNormal0;");

   if (flags & E3D_SHADER_FLAG_VERTEX_NORMAL_BLEND)
     {
        ADD_LINE("attribute   vec4  aNormal1;");
        ADD_LINE("uniform     float uNormalWeight;");
     }

   if (flags & E3D_SHADER_FLAG_VERTEX_TANGENT)
     ADD_LINE("attribute   vec4  aTangent0;");

   if (flags & E3D_SHADER_FLAG_VERTEX_TANGENT_BLEND)
     {
        ADD_LINE("attribute   vec4  aTangent1;");
        ADD_LINE("uniform     float uTangentWeight;");
     }

   if (flags & E3D_SHADER_FLAG_VERTEX_COLOR)
     ADD_LINE("attribute   vec4  aColor0;");

   if (flags & E3D_SHADER_FLAG_VERTEX_COLOR_BLEND)
     {
        ADD_LINE("attribute   vec4  aColor1;");
        ADD_LINE("uniform     float uColorWeight;");
     }

   if (flags & E3D_SHADER_FLAG_VERTEX_TEXCOORD)
     ADD_LINE("attribute   vec4  aTexCoord0;");

   if (flags & E3D_SHADER_FLAG_VERTEX_TEXCOORD_BLEND)
     {
        ADD_LINE("attribute   vec4  aTexCoord1;");
        ADD_LINE("uniform     float uTexCoordWeight;");
     }

   /* Texture coordinate. */
   if (_flags_need_tex_coord(flags))
     ADD_LINE("varying vec2 vTexCoord;");

   /* Variables for each shade modes. */
   if (mode == EVAS_3D_SHADE_MODE_VERTEX_COLOR)
     {
        ADD_LINE("varying     vec4  vColor;");
     }
   else if (mode == EVAS_3D_SHADE_MODE_DIFFUSE)
     {
        /* Nothing to declare. */
     }
   else if (mode == EVAS_3D_SHADE_MODE_FLAT)
     {
        ADD_LINE("uniform   mat3  uMatrixNormal;");
        ADD_LINE("uniform   mat4  uMatrixModelview;");
        ADD_LINE("uniform   vec4  uLightPosition;");

        ADD_LINE("varying   vec2  vFactor;");

        if (flags & E3D_SHADER_FLAG_LIGHT_SPOT)
          {
             ADD_LINE("uniform   vec3  uLightSpotDir;");
             ADD_LINE("uniform   float uLightSpotExp;");
             ADD_LINE("uniform   float uLightSpotCutoffCos;");
          }

        if (flags & E3D_SHADER_FLAG_SPECULAR)
          ADD_LINE("uniform float   uMaterialShininess;");

        if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
          ADD_LINE("uniform   vec3  uLightAtten;");
   }
   else if (mode == EVAS_3D_SHADE_MODE_PHONG || mode == EVAS_3D_SHADE_MODE_NORMAL_MAP)
     {
        ADD_LINE("uniform  mat3  uMatrixNormal;");
        ADD_LINE("uniform  mat4  uMatrixModelview;");
        ADD_LINE("uniform  vec4  uLightPosition;");
        ADD_LINE("varying  vec3  vLightVector;");
        ADD_LINE("varying  vec3  vLightHalfVector;");

       if (mode == EVAS_3D_SHADE_MODE_NORMAL_MAP)
          ADD_LINE("varying  vec3  vEyeVector;");

        if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
          ADD_LINE("varying  float vLightDist;");

        if (mode == EVAS_3D_SHADE_MODE_PHONG || (flags & E3D_SHADER_FLAG_VERTEX_TANGENT) == 0)
          ADD_LINE("varying   vec3  vNormal;");
     }
}

static void
_vertex_shader_string_func_flat_add(E3D_Shader_String *shader,
                                    Evas_3D_Shade_Mode mode EINA_UNUSED, E3D_Shader_Flag flags)
{
   ADD_LINE("void vertexFlat(vec4 position, vec3 normal) {");

   ADD_LINE("vec3  lv;");
   ADD_LINE("float factor;");

   ADD_LINE("normal = uMatrixNormal * normal;");
   ADD_LINE("position = uMatrixModelview * position;");

   if (flags & E3D_SHADER_FLAG_NORMALIZE_NORMALS)
     ADD_LINE("normal = normalize(normal);");

   if (flags & E3D_SHADER_FLAG_LIGHT_DIRECTIONAL)
     {
        ADD_LINE("lv = uLightPosition.xyz;");
     }
   else
     {
        ADD_LINE("lv = uLightPosition.xyz - position.xyz;");
        ADD_LINE("lv = normalize(lv);");
     }

   ADD_LINE("factor = max(dot(lv, normal), 0.0);");

   if (flags & E3D_SHADER_FLAG_LIGHT_SPOT)
     {
        ADD_LINE("float f = dot(-lv, uLightSpotDir);");
        ADD_LINE("if (f > uLightSpotCutoffCos)");
        ADD_LINE("factor *= pow(f, uLightSpotExp);");
        ADD_LINE("else");
        ADD_LINE("factor = 0.0;");
     }

   ADD_LINE("if (factor > 0.0) {");

   /* Diffuse term. */
   if (flags & E3D_SHADER_FLAG_DIFFUSE)
     ADD_LINE("vFactor.x = factor;");
   else
     ADD_LINE("vFactor.x = 0.0;");

   /* Specular term. */
   if (flags & E3D_SHADER_FLAG_SPECULAR)
     {
        ADD_LINE("vec3  hv = normalize(normalize(-position.xyz) + lv);");
        ADD_LINE("factor = pow(max(dot(hv, normal), 0.0), uMaterialShininess);");
        ADD_LINE("vFactor.y = factor;");
     }

   ADD_LINE("} else {");
   ADD_LINE("vFactor = vec2(0.0, 0.0);");
   ADD_LINE("}");

   /* Light attenuation. */
   if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
     {
        ADD_LINE("float dist = length(lv);");
        ADD_LINE("vFactor /= dot(uLightAtten, vec3(1.0, dist, dist * dist));");
     }

   ADD_LINE("}");
}

static void
_vertex_shader_string_func_phong_add(E3D_Shader_String *shader,
                                     Evas_3D_Shade_Mode mode EINA_UNUSED, E3D_Shader_Flag flags)
{
   ADD_LINE("void vertexPhong(vec4 position, vec3 normal) {");

   ADD_LINE("normal = uMatrixNormal * normal;");
   ADD_LINE("position = uMatrixModelview * position;");

   if (flags & E3D_SHADER_FLAG_NORMALIZE_NORMALS)
     ADD_LINE("normal = normalize(normal);");

   if (flags & E3D_SHADER_FLAG_LIGHT_DIRECTIONAL)
     {
        ADD_LINE("vLightVector = uLightPosition.xyz;");
     }
   else
     {
        ADD_LINE("vLightVector = uLightPosition.xyz - position.xyz;");

        if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
          ADD_LINE("vLightDist = length(vLightVector);");

        ADD_LINE("vLightVector = normalize(vLightVector);");
     }

   ADD_LINE("vLightHalfVector = normalize(normalize(-position.xyz) + vLightVector);");
   ADD_LINE("vNormal = normal;");
   ADD_LINE("}");
}

static void
_vertex_shader_string_func_normal_map_add(E3D_Shader_String *shader,
                                          Evas_3D_Shade_Mode mode EINA_UNUSED,
                                          E3D_Shader_Flag flags)
{
   if ((flags & E3D_SHADER_FLAG_VERTEX_TANGENT) == 0)
     {
        ADD_LINE("void vertexNormalMap(vec4 position, vec3 normal) {");

        ADD_LINE("normal = uMatrixNormal * normal;");
        ADD_LINE("position = uMatrixModelview * position;");
        ADD_LINE("vEyeVector = normalize(-position.xyz);");

        if (flags & E3D_SHADER_FLAG_NORMALIZE_NORMALS)
          ADD_LINE("normal = normalize(normal);");

        if (flags & E3D_SHADER_FLAG_LIGHT_DIRECTIONAL)
          {
             ADD_LINE("vLightVector = uLightPosition.xyz;");
          }
        else
          {
             ADD_LINE("vLightVector = uLightPosition.xyz - position.xyz;");

             if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
               ADD_LINE("vLightDist = length(vLightVector);");

             ADD_LINE("vLightVector = normalize(vLightVector);");
          }

        ADD_LINE("vLightHalfVector = normalize(vEyeVector + vLightVector);");
        ADD_LINE("vNormal = normal;");
        ADD_LINE("}");
     }
   else
     {
        ADD_LINE("void vertexNormalMap(vec4 position, vec3 normal, vec3 tangent) {");

        ADD_LINE("vec3 n = normalize(uMatrixNormal * normal);");
        ADD_LINE("vec3 t = normalize(uMatrixNormal * tangent);");
        ADD_LINE("vec3 b = cross(n, t);");
        ADD_LINE("vec3 tmp;");

        ADD_LINE("position = uMatrixModelview * position;");

        if (flags & E3D_SHADER_FLAG_LIGHT_DIRECTIONAL)
          {
             ADD_LINE("vec3 lightDir = uLightPosition.xyz;");
          }
        else
          {
             ADD_LINE("vec3 lightDir = uLightPosition.xyz - position.xyz;");

             if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
               ADD_LINE("vLightDist = length(lightDir);");

             ADD_LINE("lightDir = normalize(lightDir);");
          }

        ADD_LINE("tmp.x = dot(lightDir, t);");
        ADD_LINE("tmp.y = dot(lightDir, b);");
        ADD_LINE("tmp.z = dot(lightDir, n);");
        ADD_LINE("vLightVector = tmp;");

        ADD_LINE("tmp.x = dot(position.xyz, t);");
        ADD_LINE("tmp.y = dot(position.xyz, b);");
        ADD_LINE("tmp.z = dot(position.xyz, n);");
        ADD_LINE("vEyeVector = normalize(tmp);");

        ADD_LINE("vec3 hv = normalize(normalize(-position.xyz) + lightDir);");
        ADD_LINE("tmp.x = dot(hv, t);");
        ADD_LINE("tmp.y = dot(hv, b);");
        ADD_LINE("tmp.z = dot(hv, n);");
        ADD_LINE("vLightHalfVector = tmp;");

        ADD_LINE("}");
     }
}

static void
_vertex_shader_string_get(E3D_Shader_String *shader,
                          Evas_3D_Shade_Mode mode, E3D_Shader_Flag flags)
{
   /* Add variables - vertex attributes. */
   _vertex_shader_string_variable_add(shader, mode, flags);

   /* Add functions. */
   if (mode == EVAS_3D_SHADE_MODE_FLAT)
     _vertex_shader_string_func_flat_add(shader, mode, flags);
   else if (mode == EVAS_3D_SHADE_MODE_PHONG)
     _vertex_shader_string_func_phong_add(shader, mode, flags);
   else if (mode == EVAS_3D_SHADE_MODE_NORMAL_MAP)
     _vertex_shader_string_func_normal_map_add(shader, mode, flags);

   ADD_LINE("void main() {");

   /* Process vertex attributes. */
   if (flags & E3D_SHADER_FLAG_VERTEX_POSITION_BLEND)
     {
        ADD_LINE("vec4 position = aPosition0 * uPositionWeight + ");
        ADD_LINE("aPosition1 * (1.0 - uPositionWeight);");
        ADD_LINE("position = vec4(position.xyz, 1.0);");
     }
   else if (flags & E3D_SHADER_FLAG_VERTEX_POSITION)
     {
        ADD_LINE("vec4 position = vec4(aPosition0.xyz, 1.0);");
     }

   if (flags & E3D_SHADER_FLAG_VERTEX_NORMAL_BLEND)
     {
        ADD_LINE("vec3 normal = aNormal0.xyz * uNormalWeight + ");
        ADD_LINE("aNormal1.xyz * (1.0 - uNormalWeight);");
     }
   else if (flags & E3D_SHADER_FLAG_VERTEX_NORMAL)
     {
        ADD_LINE("vec3 normal = aNormal0.xyz;");
     }

   if (flags & E3D_SHADER_FLAG_VERTEX_TANGENT_BLEND)
     {
        ADD_LINE("vec3 tangent = aTangent0.xyz * uTangentWeight + ");
        ADD_LINE("aTangent1.xyz * (1.0 - uTangentWeight);");
     }
   else if (flags & E3D_SHADER_FLAG_VERTEX_TANGENT)
     {
        ADD_LINE("vec3 tangent = aTangent0.xyz;");
     }

   if (flags & E3D_SHADER_FLAG_VERTEX_COLOR_BLEND)
     {
        ADD_LINE("vec4 color = aColor0 * uColorWeight + aColor1 * (1.0 - uColorWeight);");
     }
   else if (flags & E3D_SHADER_FLAG_VERTEX_COLOR)
     {
        ADD_LINE("vec4 color = aColor0;");
     }

   if (flags & E3D_SHADER_FLAG_VERTEX_TEXCOORD_BLEND)
     {
        ADD_LINE("vTexCoord = aTexCoord0.st * uTexCoordWeight + ");
        ADD_LINE("aTexCoord1.st * (1.0 - uTexCoordWeight);");
     }
   else if (flags & E3D_SHADER_FLAG_VERTEX_TEXCOORD)
     {
        ADD_LINE("vTexCoord = aTexCoord0.st;");
     }

   /* Transform vertex position. */
   ADD_LINE("gl_Position = uMatrixMvp * position;");

   /* Process according to the shade mode. */
   if (mode == EVAS_3D_SHADE_MODE_VERTEX_COLOR)
     {
        ADD_LINE("vColor = color;");
     }
   else if (mode == EVAS_3D_SHADE_MODE_FLAT)
     {
        ADD_LINE("vertexFlat(position, normal);");
     }
   else if (mode == EVAS_3D_SHADE_MODE_PHONG)
     {
        ADD_LINE("vertexPhong(position, normal);");
     }
   else if (mode == EVAS_3D_SHADE_MODE_NORMAL_MAP)
     {
        if (flags & E3D_SHADER_FLAG_VERTEX_TANGENT)
          ADD_LINE("vertexNormalMap(position, normal, tangent);");
        else
          ADD_LINE("vertexNormalMap(position, normal);");
     }

   ADD_LINE("}");
}

static void
_fragment_shader_string_variable_add(E3D_Shader_String *shader,
                                     Evas_3D_Shade_Mode mode, E3D_Shader_Flag flags)
{
   /* Texture coordinate. */
   if (_flags_need_tex_coord(flags))
     ADD_LINE("varying vec2   vTexCoord;");

   /* Materials. */
   if (flags & E3D_SHADER_FLAG_DIFFUSE)
     {
        ADD_LINE("uniform   vec4        uMaterialDiffuse;");

        if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE)
          {
             ADD_LINE("uniform sampler2D  uTextureDiffuse0;");
          }

        if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE_BLEND)
          {
             ADD_LINE("uniform sampler2D  uTextureDiffuse1;");
             ADD_LINE("uniform float      uTextureDiffuseWeight;");
          }
     }

   if (flags & E3D_SHADER_FLAG_SPECULAR)
     {
        ADD_LINE("uniform  vec4        uMaterialSpecular;");
        ADD_LINE("uniform  float       uMaterialShininess;");

        if (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE)
          {
             ADD_LINE("uniform sampler2D  uTextureSpecular0;");
          }

        if (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE_BLEND)
          {
             ADD_LINE("uniform sampler2D  uTextureSpecular1;");
             ADD_LINE("uniform float      uTextureSpecularWeight;");
          }
     }

   if (flags & E3D_SHADER_FLAG_AMBIENT)
     {
        ADD_LINE("uniform  vec4        uMaterialAmbient;");

        if (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE)
          {
             ADD_LINE("uniform sampler2D  uTextureAmbient0;");
          }

        if (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE_BLEND)
          {
             ADD_LINE("uniform sampler2D  uTextureAmbient1;");
             ADD_LINE("uniform float      uTextureAmbientWeight;");
          }
     }

   if (flags & E3D_SHADER_FLAG_EMISSION)
     {
        ADD_LINE("uniform vec4       uMaterialEmission;");

        if (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE)
          {
             ADD_LINE("uniform sampler2D  uTextureEmission0;");
          }

        if (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE_BLEND)
          {
             ADD_LINE("uniform sampler2D  uTextureEmission1;");
             ADD_LINE("uniform float      uTextureEmissionWeight;");
          }
     }

   if (mode == EVAS_3D_SHADE_MODE_VERTEX_COLOR)
     {
        ADD_LINE("varying  vec4        vColor;");
     }
   else if (mode == EVAS_3D_SHADE_MODE_DIFFUSE)
     {
        /* Nothing to declare. */
     }
   else if (mode == EVAS_3D_SHADE_MODE_FLAT)
     {
        ADD_LINE("varying vec2   vFactor;");

        if (flags & E3D_SHADER_FLAG_DIFFUSE)
          ADD_LINE("uniform   vec4        uLightDiffuse;");

        if (flags & E3D_SHADER_FLAG_SPECULAR)
          ADD_LINE("uniform  vec4        uLightSpecular;");

        if (flags & E3D_SHADER_FLAG_AMBIENT)
          ADD_LINE("uniform  vec4        uLightAmbient;");
     }
   else if (mode == EVAS_3D_SHADE_MODE_PHONG || mode == EVAS_3D_SHADE_MODE_NORMAL_MAP)
     {
        ADD_LINE("varying  vec3        vLightVector;");
        ADD_LINE("varying  vec3        vLightHalfVector;");

        if (flags & E3D_SHADER_FLAG_LIGHT_SPOT)
          {
             ADD_LINE("uniform   vec3  uLightSpotDir;");
             ADD_LINE("uniform   float uLightSpotExp;");
             ADD_LINE("uniform   float uLightSpotCutoffCos;");
          }

        if (flags & E3D_SHADER_FLAG_DIFFUSE)
          ADD_LINE("uniform   vec4     uLightDiffuse;");

        if (flags & E3D_SHADER_FLAG_SPECULAR)
          ADD_LINE("uniform   vec4     uLightSpecular;");

        if (flags & E3D_SHADER_FLAG_AMBIENT)
          ADD_LINE("uniform   vec4     uLightAmbient;");

        if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
          ADD_LINE("varying   float    vLightDist;");

        if (mode == EVAS_3D_SHADE_MODE_PHONG)
          {
             ADD_LINE("varying  vec3        vNormal;");
          }
        else /* Normal map. */
          {
             ADD_LINE("uniform  sampler2D   uTextureNormal0;");

             if (flags & E3D_SHADER_FLAG_NORMAL_TEXTURE_BLEND)
               {
                  ADD_LINE("uniform  sampler2D  uTextureNormal1;");
                  ADD_LINE("uniform  float      uTextureNormalWeight;");
               }

             ADD_LINE("varying  vec3        vEyeVector;");

            if ((flags & E3D_SHADER_FLAG_VERTEX_TANGENT) == 0)
              ADD_LINE("varying  vec3        vNormal;");
          }
     }
}

static void
_fragment_shader_string_func_flat_add(E3D_Shader_String *shader,
                                      Evas_3D_Shade_Mode mode EINA_UNUSED, E3D_Shader_Flag flags)
{
   ADD_LINE("void fragmentFlat() {");
   ADD_LINE("vec4 color;");

   if (flags & E3D_SHADER_FLAG_DIFFUSE)
     {
        if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureDiffuse0, vTexCoord) * uTextureDiffuseWeight +");
             ADD_LINE("texture2D(uTextureDiffuse1, vTexCoord) * (1.0 - uTextureDiffuseWeight);");
             ADD_LINE("color *= uMaterialDiffuse;");
          }
        else if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureDiffuse0, vTexCoord) * uMaterialDiffuse;");
          }
        else
          {
             ADD_LINE("color = uMaterialDiffuse;");
          }

        ADD_LINE("gl_FragColor = uLightDiffuse * color * vFactor.x;");
     }
   else
     {
        ADD_LINE("gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);");
     }

   if (flags & E3D_SHADER_FLAG_SPECULAR)
     {
        if (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureSpecular0, vTexCoord) * uTextureSpecularWeight +");
             ADD_LINE("texture2D(uTextureSpecular1, vTexCoord) * (1.0 - uTextureSpecularWeight);");
             ADD_LINE("color *= uMaterialSpecular;");
          }
        else if (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureSpecular0, vTexCoord) * uMaterialSpecular;");
          }
        else
          {
             ADD_LINE("color = uMaterialSpecular;");
          }

        ADD_LINE("gl_FragColor += uLightSpecular * color * vFactor.y;");
     }

   if (flags & E3D_SHADER_FLAG_AMBIENT)
     {
        if (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureAmbient0, vTexCoord) * uTextureAmbientWeight +");
             ADD_LINE("texture2D(uTextureAmbient1, vTexCoord) * (1.0 - uTextureAmbientWeight);");
             ADD_LINE("color *= uMaterialAmbient;");
          }
        else if (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureAmbient0, vTexCoord) * uMaterialAmbient;");
          }
        else
          {
             ADD_LINE("color = uMaterialAmbient;");
          }

        ADD_LINE("gl_FragColor += uLightAmbient * color;");
     }

   if (flags & E3D_SHADER_FLAG_EMISSION)
     {
        if (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureEmission0, vTexCoord) * uTextureEmissionWeight +");
             ADD_LINE("texture2D(uTextureEmission1, vTexCoord) * (1.0 - uTextureEmissionWeight);");
             ADD_LINE("color *= uMaterialEmission;");
          }
        else if (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureEmission0, vTexCoord) * uMaterialEmission;");
          }
        else
          {
             ADD_LINE("color = uMaterialEmission;");
          }

        ADD_LINE("gl_FragColor += color;");
     }

   ADD_LINE("}");
}

static void
_fragment_shader_string_func_phong_add(E3D_Shader_String *shader,
                                       Evas_3D_Shade_Mode mode EINA_UNUSED, E3D_Shader_Flag flags)
{
   ADD_LINE("void fragmentPhong() {");
   ADD_LINE("vec3  normal = normalize(vNormal);");
   ADD_LINE("vec3  lv = normalize(vLightVector);");
   ADD_LINE("float factor = dot(lv, normal);");
   ADD_LINE("vec4  color;");

   if (flags & E3D_SHADER_FLAG_LIGHT_SPOT)
     {
        ADD_LINE("float f = dot(-lv, normalize(uLightSpotDir));");

        ADD_LINE("if (f > uLightSpotCutoffCos)");
        ADD_LINE("factor *= pow(f, uLightSpotExp);");
        ADD_LINE("else");
        ADD_LINE("factor = 0.0;");
     }

   ADD_LINE("if (factor > 0.0) {");

   /* Diffuse term. */
   if (flags & E3D_SHADER_FLAG_DIFFUSE)
     {
        if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureDiffuse0, vTexCoord) * uTextureDiffuseWeight +");
             ADD_LINE("texture2D(uTextureDiffuse1, vTexCoord) * (1.0 - uTextureDiffuseWeight);");
          }
        else if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureDiffuse0, vTexCoord);");
          }
        else
          {
             ADD_LINE("color = uMaterialDiffuse;");
          }

        ADD_LINE("gl_FragColor = uLightDiffuse * color * factor;");
     }
   else
     {
        ADD_LINE("gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);");
     }

   /* Specular term. */
   if (flags & E3D_SHADER_FLAG_SPECULAR)
     {
        ADD_LINE("factor = dot(normalize(vLightHalfVector), normal);");
        ADD_LINE("if (factor > 0.0) {");
        ADD_LINE("factor = pow(factor, uMaterialShininess);");

        if (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureSpecular0, vTexCoord) * uTextureSpecularWeight +");
             ADD_LINE("texture2D(uTextureSpecular1, vTexCoord) * (1.0 - uTextureSpecularWeight);");
          }
        else if (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureSpecular0, vTexCoord);");
          }
        else
          {
             ADD_LINE("color = uMaterialSpecular;");
          }

        ADD_LINE("gl_FragColor += uLightSpecular * color * factor;");
        ADD_LINE("}");
     }

   ADD_LINE("} else {");
   ADD_LINE("gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);");
   ADD_LINE("}");

   /* Ambient term. */
   if (flags & E3D_SHADER_FLAG_AMBIENT)
     {
        if (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureAmbient0, vTexCoord) * uTextureAmbientWeight +");
             ADD_LINE("texture2D(uTextureAmbient1 * vTexCoord) * (1.0 - uTextureAmbientWeight);");
          }
        else if (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureAmbient0, vTexCoord);");
          }
        else
          {
             ADD_LINE("color = uMaterialAmbient;");
          }

        ADD_LINE("gl_FragColor += uLightAmbient * color;");
     }

   /* Light attenuation. */
   if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
     ADD_LINE("gl_FragColor /= dot(uLightAtten, vec3(1.0, vLightDist, vLightDist * vLightDist));");

   /* Emission term. */
   if (flags & E3D_SHADER_FLAG_EMISSION)
     {
        if (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureEmission0, vTexCoord) * uTextureEmissionWeight +");
             ADD_LINE("texture2D(uTextureEmission1, vTexCoord) * (1.0 - uTextureEmissionWeight);");
          }
        else if (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureEmission0, vTexCoord);");
          }
        else
          {
             ADD_LINE("color = uMaterialEmission;");
          }

        ADD_LINE("gl_FragColor += color;");
     }

   ADD_LINE("gl_FragColor.a = 1.0;");
   ADD_LINE("}");
}

static void
_fragment_shader_string_func_normal_map_add(E3D_Shader_String *shader,
                                            Evas_3D_Shade_Mode mode EINA_UNUSED,
                                            E3D_Shader_Flag flags)
{
   if ((flags & E3D_SHADER_FLAG_VERTEX_TANGENT) == 0)
     {
        ADD_LINE("mat3 cotangent_frame(vec3 n, vec3 p, vec2 uv) {");
        ADD_LINE("vec3 dp1 = dFdx(p);");
        ADD_LINE("vec3 dp2 = dFdy(p);");
        ADD_LINE("vec2 duv1 = dFdx(uv);");
        ADD_LINE("vec2 duv2 = dFdy(uv);");
        ADD_LINE("vec3 dp2perp = cross(dp2, n);");
        ADD_LINE("vec3 dp1perp = cross(n, dp1);");
        ADD_LINE("vec3 t = dp2perp * duv1.x + dp1perp * duv2.x;");
        ADD_LINE("vec3 b = dp2perp * duv1.y + dp1perp * duv2.y;");
        ADD_LINE("float invmax = inversesqrt(max(dot(t, t), dot(b, b)));");
        ADD_LINE("return mat3(t * invmax, b * invmax, n);");
        ADD_LINE("}");

        ADD_LINE("vec3 perturb_normal(vec3 normal) {");
        ADD_LINE("mat3 tbn = cotangent_frame(vNormal, -vEyeVector, vTexCoord);");
        ADD_LINE("return normalize(tbn * normal);");
        ADD_LINE("}");
     }

   ADD_LINE("void fragmentNormalMap() {");
   ADD_LINE("float factor;");
   ADD_LINE("vec3  normal;");
   ADD_LINE("vec4  color;");

   if (flags & E3D_SHADER_FLAG_NORMAL_TEXTURE_BLEND)
     {
        ADD_LINE("normal = texture2D(uTextureNormal0, vTexCoord).rgb * uTextureNormalWeight;");
        ADD_LINE("normal += texture2D(uTextureNormal1, vTexCoord).rgb * ");
        ADD_LINE("(1.0 - uTextureNormalWeight);");
     }
   else
     {
        ADD_LINE("normal = texture2D(uTextureNormal0, vTexCoord).rgb;");
     }

   ADD_LINE("normal = 2.0 * normal - 1.0;");

   if ((flags & E3D_SHADER_FLAG_VERTEX_TANGENT) == 0)
     {
        ADD_LINE("normal = perturb_normal(normal);");
     }

   /* Can we skip this normalization?? */
   ADD_LINE("vec3  lv = normalize(vLightVector);");
   ADD_LINE("normal = normalize(normal);");

   ADD_LINE("factor = dot(lv, normal);");

   if (flags & E3D_SHADER_FLAG_LIGHT_SPOT)
     {
        ADD_LINE("float f = dot(-lv, normalize(uLightSpotDir));");

        ADD_LINE("if (f > uLightSpotCutoffCos)");
        ADD_LINE("factor *= pow(f, uLightSpotExp);");
        ADD_LINE("else");
        ADD_LINE("factor = 0.0;");
     }

   ADD_LINE("if (factor > 0.0) {");

   /* Diffuse term. */
   if (flags & E3D_SHADER_FLAG_DIFFUSE)
     {
        if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureDiffuse0, vTexCoord) * uTextureDiffuseWeight +");
             ADD_LINE("texture2D(uTextureDiffuse1, vTexCoord) * (1.0 - uTextureDiffuseWeight);");
          }
        else if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureDiffuse0, vTexCoord);");
          }
        else
          {
             ADD_LINE("color = uMaterialDiffuse;");
          }

        ADD_LINE("gl_FragColor = uLightDiffuse * color * factor;");
     }
   else
     {
        ADD_LINE("gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);");
     }

   /* Specular term. */
   if (flags & E3D_SHADER_FLAG_SPECULAR)
     {
        ADD_LINE("factor = dot(normalize(vLightHalfVector), normal);");
        ADD_LINE("if (factor > 0.0) {");
        ADD_LINE("factor = pow(factor, uMaterialShininess);");

        if (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureSpecular0, vTexCoord) * uTextureSpecularWeight +");
             ADD_LINE("texture2D(uTextureSpecular1, vTexCoord) * (1.0 - uTextureSpecularWeight);");
          }
        else if (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureSpecular0, vTexCoord);");
          }
        else
          {
             ADD_LINE("color = uMaterialSpecular;");
          }

        ADD_LINE("gl_FragColor += uLightSpecular * color * factor;");
        ADD_LINE("}");
     }

   ADD_LINE("} else {");
   ADD_LINE("gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);");
   ADD_LINE("}");

   /* Ambient term. */
   if (flags & E3D_SHADER_FLAG_AMBIENT)
     {
        if (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureAmbient0, vTexCoord) * uTextureAmbientWeight +");
             ADD_LINE("texture2D(uTextureAmbient1, vTexCoord) * (1.0 - uTextureAmbientWeight);");
          }
        else if (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureAmbient0, vTexCoord);");
          }
        else
          {
             ADD_LINE("color = uMaterialAmbient;");
          }

        ADD_LINE("gl_FragColor += uLightAmbient * color;");
     }

   /* Light attenuation. */
   if (flags & E3D_SHADER_FLAG_LIGHT_ATTENUATION)
     ADD_LINE("gl_FragColor /= dot(uLightAtten, vec3(1.0, vLightDist, vLightDist * vLightDist));");

   /* Emission term. */
   if (flags & E3D_SHADER_FLAG_EMISSION)
     {
        if (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE_BLEND)
          {
             ADD_LINE("color = texture2D(uTextureEmission0, vTexCoord) * uTextureEmissionWeight +");
             ADD_LINE("texture2D(uTextureEmission1, vTexCoord) * (1.0 - uTextureEmissionWeight);");
          }
        else if (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE)
          {
             ADD_LINE("color = texture2D(uTextureEmission0, vTexCoord);");
          }
        else
          {
             ADD_LINE("color = uMaterialEmission;");
          }

        ADD_LINE("gl_FragColor += color;");
     }

   ADD_LINE("gl_FragColor.a = 1.0;");
   ADD_LINE("}");
}

static void
_fragment_shader_string_get(E3D_Shader_String *shader,
                            Evas_3D_Shade_Mode mode, E3D_Shader_Flag flags)
{
   /* Add variables - vertex attributes. */
   _fragment_shader_string_variable_add(shader, mode, flags);

   /* Add functions. */
   if (mode == EVAS_3D_SHADE_MODE_FLAT)
     _fragment_shader_string_func_flat_add(shader, mode, flags);
   else if (mode == EVAS_3D_SHADE_MODE_PHONG)
     _fragment_shader_string_func_phong_add(shader, mode, flags);
   else if (mode == EVAS_3D_SHADE_MODE_NORMAL_MAP)
     _fragment_shader_string_func_normal_map_add(shader, mode, flags);

   /* Add main function. */
   ADD_LINE("void main() {");

   if (mode == EVAS_3D_SHADE_MODE_VERTEX_COLOR)
     {
        ADD_LINE("gl_FragColor = vColor;");
     }
   else if (mode == EVAS_3D_SHADE_MODE_DIFFUSE)
     {
        if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE_BLEND)
          {
             ADD_LINE("gl_FragColor = (texture2D(uTextureDiffuse0, vTexCoord) *");
             ADD_LINE("uTextureDiffuseWeight + texture2D(uTextureDiffuse1, vTexCoord) *");
             ADD_LINE("(1.0 - uTextureDiffuseWeight)) * uMaterialDiffuse;");
          }
        else if (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE)
          {
             ADD_LINE("gl_FragColor = texture2D(uTextureDiffuse0, vTexCoord) * uMaterialDiffuse;");
          }
        else
          {
             ADD_LINE("gl_FragColor = uMaterialDiffuse;");
          }
     }
   else if (mode == EVAS_3D_SHADE_MODE_FLAT)
     {
        ADD_LINE("fragmentFlat();");
     }
   else if (mode == EVAS_3D_SHADE_MODE_PHONG)
     {
        ADD_LINE("fragmentPhong();");
     }
   else if (mode == EVAS_3D_SHADE_MODE_NORMAL_MAP)
     {
        ADD_LINE("fragmentNormalMap();");
     }

   ADD_LINE("}");
}

static inline Eina_Bool
_shader_compile(GLuint shader, const char *src)
{
   GLint ok;

   glShaderSource(shader, 1, &src, NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glCompileShader(shader);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

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
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glAttachShader(program->prog, program->frag);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   /* Link program. */
   glLinkProgram(program->prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   /* Check link status. */
   glGetProgramiv(program->prog, GL_LINK_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

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
      case E3D_UNIFORM_POSITION_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_3D_VERTEX_POSITION].weight);
         break;
      case E3D_UNIFORM_NORMAL_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_3D_VERTEX_NORMAL].weight);
         break;
      case E3D_UNIFORM_TANGENT_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_3D_VERTEX_TANGENT].weight);
         break;
      case E3D_UNIFORM_COLOR_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_3D_VERTEX_COLOR].weight);
         break;
      case E3D_UNIFORM_TEXCOORD_WEIGHT:
         glUniform1f(loc, data->vertices[EVAS_3D_VERTEX_TEXCOORD].weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_AMBIENT:
         glUniform1f(loc, data->materials[EVAS_3D_MATERIAL_AMBIENT].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_DIFFUSE:
         glUniform1f(loc, data->materials[EVAS_3D_MATERIAL_DIFFUSE].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_SPECULAR:
         glUniform1f(loc, data->materials[EVAS_3D_MATERIAL_SPECULAR].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_EMISSION:
         glUniform1f(loc, data->materials[EVAS_3D_MATERIAL_EMISSION].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_WEIGHT_NORMAL:
         glUniform1f(loc, data->materials[EVAS_3D_MATERIAL_NORMAL].texture_weight);
         break;
      case E3D_UNIFORM_TEXTURE_AMBIENT0:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_AMBIENT].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_DIFFUSE0:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_DIFFUSE].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_SPECULAR0:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_SPECULAR].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_EMISSION0:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_EMISSION].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_NORMAL0:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_NORMAL].sampler0);
         break;
      case E3D_UNIFORM_TEXTURE_AMBIENT1:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_AMBIENT].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_DIFFUSE1:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_DIFFUSE].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_SPECULAR1:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_SPECULAR].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_EMISSION1:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_EMISSION].sampler1);
         break;
      case E3D_UNIFORM_TEXTURE_NORMAL1:
         glUniform1i(loc, data->materials[EVAS_3D_MATERIAL_NORMAL].sampler1);
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
                     data->materials[EVAS_3D_MATERIAL_AMBIENT].color.r,
                     data->materials[EVAS_3D_MATERIAL_AMBIENT].color.g,
                     data->materials[EVAS_3D_MATERIAL_AMBIENT].color.b,
                     data->materials[EVAS_3D_MATERIAL_AMBIENT].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_DIFFUSE:
         glUniform4f(loc,
                     data->materials[EVAS_3D_MATERIAL_DIFFUSE].color.r,
                     data->materials[EVAS_3D_MATERIAL_DIFFUSE].color.g,
                     data->materials[EVAS_3D_MATERIAL_DIFFUSE].color.b,
                     data->materials[EVAS_3D_MATERIAL_DIFFUSE].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_SPECULAR:
         glUniform4f(loc,
                     data->materials[EVAS_3D_MATERIAL_SPECULAR].color.r,
                     data->materials[EVAS_3D_MATERIAL_SPECULAR].color.g,
                     data->materials[EVAS_3D_MATERIAL_SPECULAR].color.b,
                     data->materials[EVAS_3D_MATERIAL_SPECULAR].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_EMISSION:
         glUniform4f(loc,
                     data->materials[EVAS_3D_MATERIAL_EMISSION].color.r,
                     data->materials[EVAS_3D_MATERIAL_EMISSION].color.g,
                     data->materials[EVAS_3D_MATERIAL_EMISSION].color.b,
                     data->materials[EVAS_3D_MATERIAL_EMISSION].color.a);
         break;
      case E3D_UNIFORM_MATERIAL_SHININESS:
         glUniform1f(loc, data->shininess);
         break;
      default:
         ERR("Invalid uniform ID.");
         break;
     }
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
e3d_program_new(Evas_3D_Shade_Mode mode, E3D_Shader_Flag flags)
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

   _vertex_shader_string_get(&vert, mode, flags);
   _fragment_shader_string_get(&frag, mode, flags);

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

Evas_3D_Shade_Mode
e3d_program_shade_mode_get(const E3D_Program *program)
{
   return program->mode;
}

E3D_Shader_Flag
e3d_program_shader_flags_get(const E3D_Program *program)
{
   return program->flags;
}
