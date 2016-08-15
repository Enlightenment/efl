#include "evas_gl_private.h"
#include "evas_gl_3d_private.h"

#define CHECK_LOD_DISTANCE \
   if (pd_mesh_node->lod) \
     { \
        if (pdmesh->near_lod_boundary > data->lod_distance) \
          continue; \
        else if ((pdmesh->near_lod_boundary < data->lod_distance) && \
                 (pdmesh->far_lod_boundary < data->lod_distance)) \
         continue; \
     }

#define RENDER_MESH_NODE_ITERATE_BEGIN(param)                                                      \
   Eina_Matrix4          matrix_mv;                                                                   \
   Eina_Matrix4          matrix_mvp;                                                                  \
   Eina_Iterator     *it;                                                                          \
   void              *ptr;                                                                         \
   eina_matrix4_multiply(&matrix_mv, matrix_##param, &pd_mesh_node->data.mesh.matrix_local_to_world); \
   eina_matrix4_multiply(&matrix_mvp, &pd->projection, &matrix_mv);                                   \
   it = eina_hash_iterator_data_new(pd_mesh_node->data.mesh.node_meshes);                          \
   while (eina_iterator_next(it, &ptr))                                                            \
     {                                                                                             \
        Evas_Canvas3D_Node_Mesh *nm = (Evas_Canvas3D_Node_Mesh *)ptr;                              \
        Evas_Canvas3D_Mesh_Data *pdmesh = efl_data_scope_get(nm->mesh, EVAS_CANVAS3D_MESH_CLASS);   \
        CHECK_LOD_DISTANCE

#define RENDER_MESH_NODE_ITERATE_END \
   }                                 \
   eina_iterator_free(it);

void
e3d_texture_param_update(E3D_Texture *texture)
{
   if (texture->wrap_dirty)
     {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->wrap_t);
        texture->wrap_dirty = EINA_FALSE;
     }

   if (texture->filter_dirty)
     {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->filter_min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture->filter_mag);
        texture->filter_dirty = EINA_FALSE;
     }
}

E3D_Texture *
e3d_texture_new(Eina_Bool use_atlas)
{
   E3D_Texture *texture = NULL;

   texture = (E3D_Texture *)malloc(sizeof(E3D_Texture));

   if (texture == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   eina_matrix3_identity(&texture->trans);

   texture->w = 0;
   texture->h = 0;
   texture->x = 0;
   texture->y = 0;

   texture->tex = 0;

   texture->wrap_dirty = EINA_TRUE;
   texture->wrap_s = GL_CLAMP_TO_EDGE;
   texture->wrap_t = GL_CLAMP_TO_EDGE;

   texture->filter_dirty = EINA_TRUE;
   texture->filter_min = GL_NEAREST;
   texture->filter_mag = GL_NEAREST;

   texture->atlas_enable = use_atlas;

   return texture;
}

void
e3d_texture_free(E3D_Texture *texture)
{
   if (texture)
     {
        if (texture->surface)
          evas_gl_common_image_unref(texture->surface);
     }
   free(texture);
}

void
e3d_texture_size_get(const E3D_Texture *texture, int *w, int *h)
{
   if (w) *w = texture->w;
   if (h) *h = texture->h;
}

void
e3d_texture_set(Evas_Engine_GL_Context *gc,
                E3D_Texture *texture,
                Evas_GL_Image *im)
{
   Eina_Matrix3 pt,st;
   Evas_Real pt_x, pt_y, st_x, st_y;

   texture->surface = im;
   evas_gl_common_image_ref(im);
   im->disable_atlas = !texture->atlas_enable;
   evas_gl_common_image_update(gc, im);

   texture->tex = im->tex->pt->texture;
   texture->w = im->w;
   texture->h = im->h;
   texture->x = im->tex->x;
   texture->y = im->tex->y;
   if (texture->atlas_enable)
     {
        pt_x = im->tex->pt->w ? (im->tex->x/(Evas_Real)im->tex->pt->w) : 0;
        pt_y = im->tex->pt->h ? (im->tex->y/(Evas_Real)im->tex->pt->h) : 0;

        st_x = im->tex->pt->w ? (im->w/(Evas_Real)im->tex->pt->w) : 1.0;
        st_y = im->tex->pt->h ? (im->h/(Evas_Real)im->tex->pt->h) : 1.0;
        /*Build adjusting matrix for texture unit coordinates*/
        eina_matrix3_position_transform_set(&pt, pt_x, pt_y);
        eina_matrix3_scale_transform_set(&st, st_x, st_y);
        eina_matrix3_multiply(&texture->trans, &pt, &st);
     }
}

Evas_GL_Image *
e3d_texture_get(E3D_Texture *texture)
{
   return texture ? texture->surface : NULL;
}

static inline GLenum
_to_gl_texture_wrap(Evas_Canvas3D_Wrap_Mode wrap)
{
   switch (wrap)
     {
      case EVAS_CANVAS3D_WRAP_MODE_CLAMP:
         return GL_CLAMP_TO_EDGE;
      case EVAS_CANVAS3D_WRAP_MODE_REFLECT:
         return GL_MIRRORED_REPEAT;
      case EVAS_CANVAS3D_WRAP_MODE_REPEAT:
         return GL_REPEAT;
      default:
         break;
     }

   ERR("Invalid texture wrap mode.");
   return GL_CLAMP_TO_EDGE;
}

static inline Evas_Canvas3D_Wrap_Mode
_to_e3d_texture_wrap(GLenum wrap)
{
   switch (wrap)
     {
      case GL_CLAMP_TO_EDGE:
         return EVAS_CANVAS3D_WRAP_MODE_CLAMP;
      case GL_MIRRORED_REPEAT:
         return EVAS_CANVAS3D_WRAP_MODE_REFLECT;
      case GL_REPEAT:
         return EVAS_CANVAS3D_WRAP_MODE_REPEAT;
      default:
         break;
     }

   ERR("Invalid texture wrap mode.");
   return EVAS_CANVAS3D_WRAP_MODE_CLAMP;
}

static inline GLenum
_to_gl_texture_filter(Evas_Canvas3D_Texture_Filter filter)
{
   switch (filter)
     {
      case EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST:
         return GL_NEAREST;
      case EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR:
         return GL_LINEAR;
      case EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
         return GL_NEAREST_MIPMAP_NEAREST;
      case EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
         return GL_NEAREST_MIPMAP_LINEAR;
      case EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
         return GL_LINEAR_MIPMAP_NEAREST;
      case EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
         return GL_LINEAR_MIPMAP_LINEAR;
      default:
         break;
     }

   ERR("Invalid texture wrap mode.");
   return GL_NEAREST;
}

static inline Evas_Canvas3D_Texture_Filter
_to_e3d_texture_filter(GLenum filter)
{
   switch (filter)
     {
      case GL_NEAREST:
         return EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST;
      case GL_LINEAR:
         return EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR;
      case GL_NEAREST_MIPMAP_NEAREST:
         return EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;
      case GL_NEAREST_MIPMAP_LINEAR:
         return EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
      case GL_LINEAR_MIPMAP_NEAREST:
         return EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST;
      case GL_LINEAR_MIPMAP_LINEAR:
         return EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
      default:
         break;
     }

   ERR("Invalid texture wrap mode.");
   return EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST;
}

void
e3d_texture_wrap_set(E3D_Texture *texture, Evas_Canvas3D_Wrap_Mode s, Evas_Canvas3D_Wrap_Mode t)
{
   GLenum gl_s, gl_t;

   gl_s = _to_gl_texture_wrap(s);
   gl_t = _to_gl_texture_wrap(t);

   if (gl_s == texture->wrap_s && gl_t == texture->wrap_t)
     return;

   texture->wrap_s = gl_s;
   texture->wrap_t = gl_t;
   texture->wrap_dirty = EINA_TRUE;
}

void
e3d_texture_wrap_get(const E3D_Texture *texture, Evas_Canvas3D_Wrap_Mode *s, Evas_Canvas3D_Wrap_Mode *t)
{
   if (s)
     *s = _to_e3d_texture_wrap(texture->wrap_s);

   if (t)
     *t = _to_e3d_texture_wrap(texture->wrap_t);
}

void
e3d_texture_filter_set(E3D_Texture *texture, Evas_Canvas3D_Texture_Filter min, Evas_Canvas3D_Texture_Filter mag)
{
   GLenum gl_min, gl_mag;

   gl_min = _to_gl_texture_filter(min);
   gl_mag = _to_gl_texture_filter(mag);

   if (gl_min == texture->filter_min && gl_mag == texture->filter_mag)
     return;

   texture->filter_min = gl_min;
   texture->filter_mag = gl_mag;
   texture->filter_dirty = EINA_TRUE;
}

void
e3d_texture_filter_get(const E3D_Texture *texture,
                       Evas_Canvas3D_Texture_Filter *min, Evas_Canvas3D_Texture_Filter *mag)
{
   if (min)
     *min = _to_e3d_texture_filter(texture->filter_min);

   if (mag)
     *mag = _to_e3d_texture_filter(texture->filter_mag);
}

static void
_gen_texture_2d(GLuint *texture, GLenum wrap_type, GLfloat filter_type)
{
   glGenTextures(1, texture);
   glBindTexture(GL_TEXTURE_2D, *texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_type);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_type);
}

static void
_gen_depth_stencil_buffer(GLuint *buffer, GLenum format, GLenum attach,
                          int size_w, int size_h)
{
   glGenRenderbuffers(1, buffer);
   glBindRenderbuffer(GL_RENDERBUFFER, *buffer);
   glRenderbufferStorage(GL_RENDERBUFFER, format, size_w, size_h);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach, GL_RENDERBUFFER, *buffer);
}

E3D_Drawable *
e3d_drawable_new(int w, int h, int alpha, GLenum depth_format, GLenum stencil_format)
{
   E3D_Drawable  *drawable = NULL;
   GLuint         tex, fbo, texDepth, texcolorpick, color_pick_fb_id;
   GLuint         depth_stencil_buf = 0;
   GLuint         depth_buf = 0;
   GLuint         stencil_buf = 0;
   Eina_Bool      depth_stencil = EINA_FALSE;

   _gen_texture_2d(&tex, GL_CLAMP_TO_EDGE, GL_NEAREST);
   if (alpha)
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   else
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

   _gen_texture_2d(&texDepth, GL_CLAMP_TO_EDGE, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

   glGenFramebuffers(1, &color_pick_fb_id);
   _gen_texture_2d(&texcolorpick, GL_CLAMP_TO_EDGE, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

   glGenFramebuffers(1, &fbo);
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

#ifdef GL_GLES
   if (depth_format == GL_DEPTH_STENCIL_OES)
     {
        _gen_texture_2d(&depth_stencil_buf, GL_CLAMP_TO_EDGE, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL_OES, w, h, 0,
                     GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, depth_stencil_buf, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D, depth_stencil_buf, 0);

        depth_stencil = EINA_TRUE;
     }
#else
   if (depth_format == GL_DEPTH24_STENCIL8)
     {
        _gen_depth_stencil_buffer(&depth_stencil_buf, GL_DEPTH24_STENCIL8,
                                  GL_DEPTH_STENCIL_ATTACHMENT, w, h);
        depth_stencil = EINA_TRUE;
     }
#endif

   if ((!depth_stencil) && (depth_format))
     _gen_depth_stencil_buffer(&depth_buf, depth_format, GL_DEPTH_ATTACHMENT, w, h);

   if ((!depth_stencil) && (stencil_format))
     _gen_depth_stencil_buffer(&stencil_buf, stencil_format, GL_STENCIL_ATTACHMENT, w, h);

   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
     goto error;

   drawable = (E3D_Drawable *)calloc(1, sizeof(E3D_Drawable));
   if (drawable == NULL)
     goto error;

   drawable->w = w;
   drawable->h = h;
   drawable->alpha = alpha;

   if (alpha)
     drawable->format = GL_RGBA;
   else
     drawable->format = GL_RGB;

   drawable->depth_format = depth_format;
   drawable->stencil_format = stencil_format;
   drawable->tex = tex;
   drawable->fbo = fbo;
   drawable->texcolorpick = texcolorpick;
   drawable->color_pick_fb_id = color_pick_fb_id;
   drawable->depth_stencil_buf = depth_stencil_buf;
   drawable->depth_buf = depth_buf;
   drawable->stencil_buf = stencil_buf;
   drawable->texDepth = texDepth;

   return drawable;

error:
   ERR("Drawable creation failed.");

   if (tex)
     glDeleteTextures(1, &tex);
   if (texcolorpick)
     glDeleteTextures(1, &texcolorpick);
   if (texDepth)
     glDeleteTextures(1, &texDepth);

   if (fbo)
     glDeleteFramebuffers(1, &fbo);
   if (color_pick_fb_id)
     glDeleteFramebuffers(1, &color_pick_fb_id);

   if (depth_stencil_buf)
     {
#ifdef GL_GLES
        glDeleteTextures(1, &depth_stencil_buf);
#else
        glDeleteRenderbuffers(1, &depth_stencil_buf);
#endif
     }

   if (depth_buf)
     glDeleteRenderbuffers(1, &depth_buf);

   if (stencil_buf)
     glDeleteRenderbuffers(1, &stencil_buf);


   return NULL;
}

void
e3d_drawable_free(E3D_Drawable *drawable)
{
   if (drawable == NULL)
     return;

   if (drawable->tex)
     glDeleteTextures(1, &drawable->tex);

   if (drawable->fbo)
     glDeleteFramebuffers(1, &drawable->fbo);

   if (drawable->depth_stencil_buf)
     {
#ifdef GL_GLES
        glDeleteTextures(1, &drawable->depth_stencil_buf);
#else
        glDeleteRenderbuffers(1, &drawable->depth_stencil_buf);
#endif
     }

   if (drawable->depth_buf)
     glDeleteRenderbuffers(1, &drawable->depth_buf);

   if (drawable->stencil_buf)
     glDeleteRenderbuffers(1, &drawable->stencil_buf);

   free(drawable);
}

void
e3d_drawable_size_get(E3D_Drawable *drawable, int *w, int *h)
{
   if (drawable)
     {
        if (w) *w = drawable->w;
        if (h) *h = drawable->h;
     }
   else
     {
        if (w) *w = 0;
        if (h) *h = 0;
     }
}

GLuint
e3d_drawable_texture_id_get(E3D_Drawable *drawable)
{
   return drawable->tex;
}

GLuint
e3d_drawable_texture_color_pick_id_get(E3D_Drawable *drawable)
{
   return drawable->texcolorpick;
}

GLenum
e3d_drawable_format_get(E3D_Drawable *drawable)
{
   return drawable->format;
}

static inline void
_mesh_frame_find(Evas_Canvas3D_Mesh *mesh, int frame,
                 Eina_List **l, Eina_List **r)
{
   Eina_List *left, *right;
   Evas_Canvas3D_Mesh_Frame *f0 = NULL, *f1;
   Evas_Canvas3D_Mesh_Data *pdmesh = efl_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);

   left = pdmesh->frames;
   right = eina_list_next(left);

   while (right)
     {
        f0 = (Evas_Canvas3D_Mesh_Frame *)eina_list_data_get(left);
        f1 = (Evas_Canvas3D_Mesh_Frame *)eina_list_data_get(right);

        if (frame >= f0->frame && frame <= f1->frame)
          break;

        left = right;
        right = eina_list_next(left);
     }

   if (right == NULL)
     {
        if (f0 && frame <= f0->frame)
          {
             *l = NULL;
             *r = left;
          }
        else
          {
             *l = left;
             *r = NULL;
          }
     }

   *l = left;
   *r = right;
}

static inline void
_vertex_attrib_flag_add(E3D_Draw_Data *data,
                        Evas_Canvas3D_Vertex_Attrib attrib,
                        Eina_Bool blend)
{
   switch (attrib)
     {
      case EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION:
         data->flags |= E3D_SHADER_FLAG_VERTEX_POSITION;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_POSITION_BLEND;
         break;
      case EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL:
         data->flags |= E3D_SHADER_FLAG_VERTEX_NORMAL;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_NORMAL_BLEND;
         break;
      case EVAS_CANVAS3D_VERTEX_ATTRIB_TANGENT:
         data->flags |= E3D_SHADER_FLAG_VERTEX_TANGENT;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_TANGENT_BLEND;
         break;
      case EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR:
         data->flags |= E3D_SHADER_FLAG_VERTEX_COLOR;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_COLOR_BLEND;
         break;
      case EVAS_CANVAS3D_VERTEX_ATTRIB_TEXCOORD:
         data->flags |= E3D_SHADER_FLAG_VERTEX_TEXCOORD;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_TEXCOORD_BLEND;
         break;
      default:
         ERR("Invalid vertex attrib.");
         break;
     }
}

static inline void
_material_color_flag_add(E3D_Draw_Data *data, Evas_Canvas3D_Material_Attrib attrib)
{
   switch (attrib)
     {
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT:
         data->flags |= E3D_SHADER_FLAG_AMBIENT;
         break;
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE:
         data->flags |= E3D_SHADER_FLAG_DIFFUSE;
         break;
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR:
         data->flags |= E3D_SHADER_FLAG_SPECULAR;
         break;
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION:
         data->flags |= E3D_SHADER_FLAG_EMISSION;
         break;
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL:
         ERR("Material attribute normal should not be used with color values.");
         break;
      default:
         ERR("Invalid material attrib.");
         break;
     }
}

static inline void
_material_texture_flag_add(E3D_Draw_Data *data, Evas_Canvas3D_Material_Attrib attrib, Eina_Bool blend)
{
   switch (attrib)
     {
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT:
         data->flags |= E3D_SHADER_FLAG_AMBIENT;
         data->flags |= E3D_SHADER_FLAG_AMBIENT_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_AMBIENT_TEXTURE_BLEND;
         break;
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE:
         data->flags |= E3D_SHADER_FLAG_DIFFUSE;
         data->flags |= E3D_SHADER_FLAG_DIFFUSE_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_DIFFUSE_TEXTURE_BLEND;
         break;
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR:
         data->flags |= E3D_SHADER_FLAG_SPECULAR;
         data->flags |= E3D_SHADER_FLAG_SPECULAR_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_SPECULAR_TEXTURE_BLEND;
         break;
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION:
         data->flags |= E3D_SHADER_FLAG_EMISSION;
         data->flags |= E3D_SHADER_FLAG_EMISSION_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_EMISSION_TEXTURE_BLEND;
         break;
      case EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL:
         data->flags |= E3D_SHADER_FLAG_NORMAL_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_NORMAL_TEXTURE_BLEND;
         break;
      default:
         ERR("Invalid material attrib.");
         break;
     }
}

static inline Eina_Bool
_vertex_attrib_build(E3D_Draw_Data *data, int frame,
                     const Eina_List *l, const Eina_List *r,
                     Evas_Canvas3D_Vertex_Attrib attrib)
{
   const Evas_Canvas3D_Mesh_Frame *f0 = NULL, *f1 = NULL;

   while (l)
     {
        f0 = (const Evas_Canvas3D_Mesh_Frame *)eina_list_data_get(l);

        if (f0->vertices[attrib].data != NULL)
          break;

        l = eina_list_prev(l);
        f0 = NULL;
     }

   while (r)
     {
        f1 = (const Evas_Canvas3D_Mesh_Frame *)eina_list_data_get(r);

        if (f1->vertices[attrib].data != NULL)
          break;

        r = eina_list_next(r);
        f1 = NULL;
     }

   if (f0 == NULL && f1 == NULL)
     return EINA_FALSE;

   if (f0 == NULL)
     {
        f0 = f1;
        f1 = NULL;
     }
   else if (f1 != NULL)
     {
        if (frame == f0->frame)
          {
             f1 = NULL;
          }
        else if (frame == f1->frame)
          {
             f0 = f1;
             f1 = NULL;
          }
     }

   data->vertices[attrib].vertex0 = f0->vertices[attrib];
   data->vertices[attrib].vertex0.owns_data = EINA_FALSE;

   if (f1)
     {
        data->vertices[attrib].vertex1 = f1->vertices[attrib];
        data->vertices[attrib].vertex1.owns_data = EINA_FALSE;
        data->vertices[attrib].weight = (f1->frame - frame) / (Evas_Real)(f1->frame - f0->frame);
        _vertex_attrib_flag_add(data, attrib, EINA_TRUE);
     }
   else
     {
        _vertex_attrib_flag_add(data, attrib, EINA_FALSE);
     }

   return EINA_TRUE;
}

static inline Eina_Bool
_material_color_build(E3D_Draw_Data *data, int frame,
                      const Eina_List *l, const Eina_List *r,
                      Evas_Canvas3D_Material_Attrib attrib)
{
   const Evas_Canvas3D_Mesh_Frame *f0 = NULL, *f1 = NULL;

   while (l)
     {
        f0 = (const Evas_Canvas3D_Mesh_Frame *)eina_list_data_get(l);

        if (f0->material)
          {
             Evas_Canvas3D_Material_Data *pdm = efl_data_scope_get(f0->material, EVAS_CANVAS3D_MATERIAL_CLASS);
             if (pdm->attribs[attrib].enable)
                break;
          }

        l = eina_list_prev(l);
        f0 = NULL;
     }

   while (r)
     {
        f1 = (const Evas_Canvas3D_Mesh_Frame *)eina_list_data_get(r);

        if (f1->material)
          {
             Evas_Canvas3D_Material_Data *pdm = efl_data_scope_get(f1->material, EVAS_CANVAS3D_MATERIAL_CLASS);
             if (pdm->attribs[attrib].enable)
                break;
          }

        r = eina_list_next(r);
        f1 = NULL;
     }

   if (f0 == NULL && f1 == NULL)
     return EINA_FALSE;

   if (f0 == NULL)
     {
        f0 = f1;
        f1 = NULL;
     }
   else if (f1 != NULL)
     {
        if (frame == f0->frame)
          {
             f1 = NULL;
          }
        else if (frame == f1->frame)
          {
             f0 = f1;
             f1 = NULL;
          }
     }
   Evas_Canvas3D_Material_Data *pdmf0 = efl_data_scope_get(f0->material, EVAS_CANVAS3D_MATERIAL_CLASS);
   if (f1 == NULL)
     {
        data->materials[attrib].color = pdmf0->attribs[attrib].color;

        if (attrib == EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR)
          data->shininess = pdmf0->shininess;
     }
   else
     {
        Evas_Real weight;
        Evas_Canvas3D_Material_Data *pdmf1 = efl_data_scope_get(f1->material, EVAS_CANVAS3D_MATERIAL_CLASS);

        weight = (f1->frame - frame) / (Evas_Real)(f1->frame - f0->frame);
        evas_color_blend(&data->materials[attrib].color,
                         &pdmf0->attribs[attrib].color,
                         &pdmf0->attribs[attrib].color,
                         weight);

        if (attrib == EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR)
          {
             data->shininess = pdmf0->shininess * weight +
                pdmf1->shininess * (1.0 - weight);
          }
     }

   _material_color_flag_add(data, attrib);
   return EINA_TRUE;
}

static inline Eina_Bool
_material_texture_build(E3D_Draw_Data *data, int frame,
                        const Eina_List *l, const Eina_List *r,
                        Evas_Canvas3D_Material_Attrib attrib)
{
   const Evas_Canvas3D_Mesh_Frame *f0 = NULL, *f1 = NULL;

   while (l)
     {
        f0 = (const Evas_Canvas3D_Mesh_Frame *)eina_list_data_get(l);

        if (f0->material)
          {
             Evas_Canvas3D_Material_Data *pdm = efl_data_scope_get(f0->material, EVAS_CANVAS3D_MATERIAL_CLASS);
             if (pdm->attribs[attrib].enable && pdm->attribs[attrib].texture != NULL)
                break;
          }

        l = eina_list_prev(l);
        f0 = NULL;
     }

   while (r)
     {
        f1 = (const Evas_Canvas3D_Mesh_Frame *)eina_list_data_get(r);

        if (f1->material)
          {
             Evas_Canvas3D_Material_Data *pdm = efl_data_scope_get(f1->material, EVAS_CANVAS3D_MATERIAL_CLASS);
             if (pdm->attribs[attrib].enable && pdm->attribs[attrib].texture != NULL)
                break;
          }

        r = eina_list_next(r);
        f1 = NULL;
     }

   if (f0 == NULL && f1 == NULL)
     return EINA_FALSE;

   if (f0 == NULL)
     {
        f0 = f1;
        f1 = NULL;
     }
   else if (f1 != NULL)
     {
        if (frame == f0->frame)
          {
             f1 = NULL;
          }
        else if (frame == f1->frame)
          {
             f0 = f1;
             f1 = NULL;
          }
     }

   Evas_Canvas3D_Material_Data *pdmf0 = efl_data_scope_get(f0->material, EVAS_CANVAS3D_MATERIAL_CLASS);
   data->materials[attrib].sampler0 = data->texture_count++;
   Evas_Canvas3D_Texture_Data *pd = efl_data_scope_get(pdmf0->attribs[attrib].texture, EVAS_CANVAS3D_TEXTURE_CLASS);
   data->materials[attrib].tex0 = (E3D_Texture *)pd->engine_data;

   if (f1)
     {
        Evas_Canvas3D_Material_Data *pdmf1 = efl_data_scope_get(f1->material, EVAS_CANVAS3D_MATERIAL_CLASS);
        Evas_Real weight = (f1->frame - frame) / (Evas_Real)(f1->frame - f0->frame);

        data->materials[attrib].sampler1 = data->texture_count++;
        pd = efl_data_scope_get(pdmf1->attribs[attrib].texture, EVAS_CANVAS3D_TEXTURE_CLASS);
        data->materials[attrib].tex1 = (E3D_Texture *)pd->engine_data;

        data->materials[attrib].texture_weight = weight;

        if (attrib == EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR)
          {
             data->shininess = pdmf0->shininess * weight +
                pdmf1->shininess * (1.0 - weight);
          }

        _material_texture_flag_add(data, attrib, EINA_TRUE);
     }
   else
     {
        if (attrib == EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR)
          data->shininess = pdmf0->shininess;

        _material_texture_flag_add(data, attrib, EINA_FALSE);
     }

   return EINA_TRUE;
}

static inline void
_light_build(E3D_Draw_Data *data,
             const Evas_Canvas3D_Node *light,
             const Eina_Matrix4    *matrix_eye)
{
   Evas_Canvas3D_Node_Data *pd_light_node = efl_data_scope_get(light, EVAS_CANVAS3D_NODE_CLASS);
   Evas_Canvas3D_Light *l = pd_light_node ? pd_light_node->data.light.light : NULL;
   Evas_Canvas3D_Light_Data *pdl = l ? efl_data_scope_get(l, EVAS_CANVAS3D_LIGHT_CLASS) : NULL;
   Eina_Vector3      pos, dir;

   if (pdl == NULL)
     return;

   /* Get light node's position. */
   if (pdl->directional)
     {
        data->flags |= E3D_SHADER_FLAG_LIGHT_DIRECTIONAL;

        /* Negative Z. */
        eina_vector3_set(&dir, 0.0, 0.0, 1.0);
        eina_vector3_quaternion_rotate(&dir, &dir, &pd_light_node->orientation);

        /* Transform to eye space. */
        eina_vector3_homogeneous_direction_transform(&dir, matrix_eye, &dir);
        eina_vector3_normalize(&dir, &dir);

        data->light.position.x = dir.x;
        data->light.position.y = dir.y;
        data->light.position.z = dir.z;
        data->light.position.w = 0.0;
     }
   else
     {
        eina_vector3_copy(&pos, &pd_light_node->position_world);
        eina_vector3_homogeneous_position_transform(&pos, matrix_eye, &pos);

        data->light.position.x = pos.x;
        data->light.position.y = pos.y;
        data->light.position.z = pos.z;
        data->light.position.w = 1.0;

        if (pdl->enable_attenuation)
          {
             data->flags |= E3D_SHADER_FLAG_LIGHT_ATTENUATION;

             data->light.atten.x = pdl->atten_const;
             data->light.atten.y = pdl->atten_linear;
             data->light.atten.z = pdl->atten_quad;
          }

        if (pdl->spot_cutoff < 180.0)
          {
             data->flags |= E3D_SHADER_FLAG_LIGHT_SPOT;
             eina_vector3_set(&dir, 0.0, 0.0, -1.0);
             eina_vector3_quaternion_rotate(&dir, &dir, &pd_light_node->orientation);
             eina_vector3_homogeneous_direction_transform(&dir, matrix_eye, &dir);

             data->light.spot_dir = dir;
             data->light.spot_exp = pdl->spot_exp;
             data->light.spot_cutoff_cos = pdl->spot_cutoff_cos;
          }
     }

   data->light.ambient = pdl->ambient;
   data->light.diffuse = pdl->diffuse;
   data->light.specular = pdl->specular;
}

static inline Eina_Bool
_mesh_draw_data_build(E3D_Draw_Data *data,
                      Evas_Canvas3D_Mesh *mesh, int frame,
                      const Eina_Matrix4 *matrix_eye,
                      const Eina_Matrix4 *matrix_mv,
                      const Eina_Matrix4 *matrix_mvp,
                      const Eina_Matrix4 *matrix_light,
                      const Evas_Canvas3D_Node *light)
{
   Eina_List *l, *r;
   Evas_Canvas3D_Mesh_Data *pdmesh = efl_data_scope_get(mesh, EVAS_CANVAS3D_MESH_CLASS);

   if (pdmesh->frames == NULL)
     return EINA_FALSE;

   if (pdmesh->fog_enabled)
     {
        data->flags |= E3D_SHADER_FLAG_FOG_ENABLED;
        data->fog_color = pdmesh->fog_color;
     }
   if (pdmesh->alpha_test_enabled)
     data->flags |= E3D_SHADER_FLAG_ALPHA_TEST_ENABLED;

   if (pdmesh->shadowed)
     {
        data->flags |= E3D_SHADER_FLAG_SHADOWED;
        data->pcf_size = 1 / pdmesh->shadows_edges_size;
        data->pcf_step = (Evas_Real)pdmesh->shadows_edges_filtering_level;
        data->constant_bias = pdmesh->shadows_constant_bias;
     }

   if (pdmesh->color_pick_enabled)
     data->color_pick_key = pdmesh->color_pick_key;

   data->alpha_comparison = pdmesh->alpha_comparison;
   data->alpha_ref_value = pdmesh->alpha_ref_value;
   data->alpha_test_enabled =pdmesh->alpha_test_enabled;

   data->blending = pdmesh->blending;
   data->blend_sfactor = pdmesh->blend_sfactor;
   data->blend_dfactor = pdmesh->blend_dfactor;
   data->mode = pdmesh->shader_mode;
   data->assembly = pdmesh->assembly;
   data->vertex_count = pdmesh->vertex_count;
   data->index_count = pdmesh->index_count;
   data->index_format = pdmesh->index_format;
   data->indices = pdmesh->indices;

   eina_matrix4_copy(&data->matrix_mvp, matrix_mvp);
   eina_matrix4_copy(&data->matrix_mv, matrix_mv);
   if (matrix_light != NULL)
     eina_matrix4_copy(&data->matrix_light, matrix_light);

   _mesh_frame_find(mesh, frame, &l, &r);

#define BUILD(type, arg, check)                                               \
   do {                                                                       \
        Eina_Bool ret = _##type##_build(data, frame, l, r, EVAS_CANVAS3D_##arg);    \
        if (check && !ret)                                                    \
          {                                                                   \
             ERR("Missing attribute : " #arg);                                \
             return EINA_FALSE;                                               \
          }                                                                   \
   } while (0)

   if (pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_VERTEX_COLOR)
     {
        BUILD(vertex_attrib,     VERTEX_ATTRIB_POSITION,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_ATTRIB_COLOR,        EINA_TRUE);
     }
   else if (pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_SHADOW_MAP_RENDER)
     {
        BUILD(vertex_attrib,     VERTEX_ATTRIB_POSITION,     EINA_TRUE);
        if (pdmesh->alpha_test_enabled)
          {
             BUILD(material_texture,  MATERIAL_ATTRIB_DIFFUSE,    EINA_FALSE);

             if (_flags_need_tex_coord(data->flags))
               BUILD(vertex_attrib,     VERTEX_ATTRIB_TEXCOORD,     EINA_FALSE);
          }
     }
   else if (pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_COLOR_PICK)
     {
        BUILD(vertex_attrib,     VERTEX_ATTRIB_POSITION,     EINA_TRUE);
     }
   else if (pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_DIFFUSE)
     {
        BUILD(vertex_attrib,     VERTEX_ATTRIB_POSITION,     EINA_TRUE);
        BUILD(material_color,    MATERIAL_ATTRIB_DIFFUSE,    EINA_TRUE);
        BUILD(material_texture,  MATERIAL_ATTRIB_DIFFUSE,    EINA_FALSE);

        if (_flags_need_tex_coord(data->flags))
          BUILD(vertex_attrib,     VERTEX_ATTRIB_TEXCOORD,     EINA_FALSE);
     }
   else if (pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_FLAT)
     {
        BUILD(vertex_attrib,     VERTEX_ATTRIB_POSITION,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_ATTRIB_NORMAL,       EINA_TRUE);

        BUILD(material_color,    MATERIAL_ATTRIB_AMBIENT,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_DIFFUSE,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_SPECULAR,   EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_EMISSION,   EINA_FALSE);

        BUILD(material_texture,  MATERIAL_ATTRIB_AMBIENT,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_DIFFUSE,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_SPECULAR,   EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_EMISSION,   EINA_FALSE);

        _light_build(data, light, matrix_eye);
        eina_normal3_matrix_get(&data->matrix_normal, matrix_mv);

        if (_flags_need_tex_coord(data->flags))
          BUILD(vertex_attrib,     VERTEX_ATTRIB_TEXCOORD,     EINA_FALSE);
     }
   else if (pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_PHONG)
     {
        BUILD(vertex_attrib,     VERTEX_ATTRIB_POSITION,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_ATTRIB_NORMAL,       EINA_TRUE);

        BUILD(material_color,    MATERIAL_ATTRIB_AMBIENT,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_DIFFUSE,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_SPECULAR,   EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_EMISSION,   EINA_FALSE);

        BUILD(material_texture,  MATERIAL_ATTRIB_AMBIENT,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_DIFFUSE,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_SPECULAR,   EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_EMISSION,   EINA_FALSE);

        _light_build(data, light, matrix_eye);
        eina_normal3_matrix_get(&data->matrix_normal, matrix_mv);

        if (_flags_need_tex_coord(data->flags))
          BUILD(vertex_attrib,     VERTEX_ATTRIB_TEXCOORD,     EINA_FALSE);
     }
   else if ((pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_NORMAL_MAP) ||
            (pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_PARALLAX_OCCLUSION))
     {
        BUILD(vertex_attrib,     VERTEX_ATTRIB_POSITION,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_ATTRIB_NORMAL,       EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_ATTRIB_TEXCOORD,     EINA_TRUE);
        BUILD(material_texture,  MATERIAL_ATTRIB_NORMAL,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_ATTRIB_TANGENT,      EINA_FALSE);


        if (pdmesh->shader_mode == EVAS_CANVAS3D_SHADER_MODE_NORMAL_MAP)
          BUILD(vertex_attrib,     VERTEX_ATTRIB_TANGENT,      EINA_FALSE);
        else BUILD(vertex_attrib,     VERTEX_ATTRIB_TANGENT,      EINA_TRUE);

        BUILD(material_color,    MATERIAL_ATTRIB_AMBIENT,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_DIFFUSE,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_SPECULAR,   EINA_FALSE);
        BUILD(material_color,    MATERIAL_ATTRIB_EMISSION,   EINA_FALSE);

        BUILD(material_texture,  MATERIAL_ATTRIB_AMBIENT,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_DIFFUSE,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_SPECULAR,   EINA_FALSE);
        BUILD(material_texture,  MATERIAL_ATTRIB_EMISSION,   EINA_FALSE);

        _light_build(data, light, matrix_eye);
        eina_normal3_matrix_get(&data->matrix_normal, matrix_mv);
     }
   /*Check possible quantity of texture units*/
   int num, count = 0;
   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num);
   data->smap_sampler = num - 1;

   if (data->render_to_texture)
     {
        count++;
        data->colortex_sampler = num - (count + 1);
     }
   if (data->texture_count + count >= num)
     if ((data->flags & E3D_SHADER_FLAG_SHADOWED) ||
         (data->texture_count + count > num))
       {
          ERR("Too many textures for your graphics configuration.");
          return EINA_FALSE;
       }

   return EINA_TRUE;
}

static inline void
_mesh_draw(E3D_Renderer *renderer, Evas_Canvas3D_Mesh *mesh, int frame, Evas_Canvas3D_Node *light,
           const Eina_Matrix4 *matrix_eye, const Eina_Matrix4 *matrix_mv, const Eina_Matrix4 *matrix_mvp, const Eina_Matrix4 *matrix_light)
{
   E3D_Draw_Data   data;

   memset(&data, 0x00, sizeof(E3D_Draw_Data));
   data.render_to_texture = e3d_renderer_rendering_to_texture_get(renderer);

   if (_mesh_draw_data_build(&data, mesh, frame, matrix_eye, matrix_mv, matrix_mvp, matrix_light, light))
     e3d_renderer_draw(renderer, &data);
}

void _shadowmap_render(E3D_Drawable *drawable, E3D_Renderer *renderer,
                       Evas_Canvas3D_Scene_Public_Data *data, Eina_Matrix4 *matrix_light_eye,
                       Evas_Canvas3D_Node *light)
{
   Eina_List        *l;
   Evas_Canvas3D_Node     *n;
   Evas_Canvas3D_Shader_Mode shader_mode;
   Eina_Bool       blend_enabled;
   Evas_Color      c = {1.0, 1.0, 1.0, 1.0};
   Eina_Matrix4 matrix_vp;

   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(data->depth_offset, data->depth_constant);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                          drawable->texDepth, 0);

   e3d_renderer_clear(renderer, &c);

   Evas_Canvas3D_Node_Data *pd_light_node = efl_data_scope_get(light, EVAS_CANVAS3D_NODE_CLASS);
   Evas_Canvas3D_Light_Data *pd = efl_data_scope_get(pd_light_node->data.light.light,
                                              EVAS_CANVAS3D_LIGHT_CLASS);

   Eina_Quaternion planes[6];
   eina_matrix4_multiply(&matrix_vp, &pd->projection, matrix_light_eye);
   evas_frustum_calculate(planes, &matrix_vp);

   EINA_LIST_FOREACH(data->mesh_nodes, l, n)
     {
        Evas_Canvas3D_Node_Data *pd_mesh_node = efl_data_scope_get(n, EVAS_CANVAS3D_NODE_CLASS);

        if (evas_is_sphere_in_frustum(&pd_mesh_node->bsphere, planes))
          {
             RENDER_MESH_NODE_ITERATE_BEGIN(light_eye)
               {
                  shader_mode = pdmesh->shader_mode;
                  blend_enabled = pdmesh->blending;
                  pdmesh->blending = EINA_FALSE;
                  pdmesh->shader_mode = EVAS_CANVAS3D_SHADER_MODE_SHADOW_MAP_RENDER;
                  _mesh_draw(renderer, nm->mesh, nm->frame, light, matrix_light_eye,
                             &matrix_mv, &matrix_mvp, &matrix_mvp);
                  pdmesh->shader_mode = shader_mode;
                  pdmesh->blending = blend_enabled;
               }
             RENDER_MESH_NODE_ITERATE_END
          }
     }

   glDisable(GL_POLYGON_OFFSET_FILL);

   if (data->render_to_texture)
     {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               drawable->texcolorpick, 0);
        e3d_renderer_clear(renderer, &data->bg_color);
     }
   else
     {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               drawable->tex, 0);
        e3d_renderer_clear(renderer, &data->bg_color);
     }
}

void
_scene_render(E3D_Drawable *drawable, E3D_Renderer *renderer, Evas_Canvas3D_Scene_Public_Data *data)
{
   Eina_List        *l;
   Evas_Canvas3D_Node     *n;
   const Eina_Matrix4  *matrix_eye;
   Evas_Canvas3D_Node     *light;
   Eina_Matrix4        matrix_light_eye, matrix_vp;;
   Evas_Canvas3D_Light_Data *ld = NULL;
   Evas_Canvas3D_Node_Data *pd_light_node;
   Eina_Quaternion planes[6];

   /* Get eye matrix. */
   Evas_Canvas3D_Node_Data *pd_camera_node = efl_data_scope_get(data->camera_node, EVAS_CANVAS3D_NODE_CLASS);
   matrix_eye = &pd_camera_node->data.camera.matrix_world_to_eye;

   Evas_Canvas3D_Camera_Data *pd = efl_data_scope_get(pd_camera_node->data.camera.camera, EVAS_CANVAS3D_CAMERA_CLASS);

   light = eina_list_data_get(data->light_nodes);

   if (data->shadows_enabled)
     {
        pd_light_node = efl_data_scope_get(light, EVAS_CANVAS3D_NODE_CLASS);
        evas_mat4_inverse_build(&matrix_light_eye,
           &pd_light_node->position_world, &pd_light_node->orientation_world, &pd_light_node->scale_world);
        ld = efl_data_scope_get(pd_light_node->data.light.light, EVAS_CANVAS3D_LIGHT_CLASS);
         _shadowmap_render(drawable, renderer, data, &matrix_light_eye, light);
     }

   eina_matrix4_multiply(&matrix_vp, &pd->projection, matrix_eye);
   evas_frustum_calculate(planes, &matrix_vp);

   EINA_LIST_FOREACH(data->mesh_nodes, l, n)
     {
        Eina_Matrix4          matrix_mv;
        Eina_Matrix4          matrix_light;
        Eina_Matrix4          matrix_mvp;
        Eina_Iterator     *it;
        void              *ptr;
        Evas_Canvas3D_Node_Data *pd_mesh_node = efl_data_scope_get(n, EVAS_CANVAS3D_NODE_CLASS);

        // TODO Add other frustum shapes
        if (evas_is_sphere_in_frustum(&pd_mesh_node->bsphere, planes))
          {

             if (data->shadows_enabled)
               {
                  eina_matrix4_multiply(&matrix_mv, &matrix_light_eye,
                      &pd_mesh_node->data.mesh.matrix_local_to_world);
                  eina_matrix4_multiply(&matrix_light, &ld->projection,
                      &matrix_mv);
               }

             eina_matrix4_multiply(&matrix_mv, matrix_eye, &pd_mesh_node->data.mesh.matrix_local_to_world);
             eina_matrix4_multiply(&matrix_mvp, &pd->projection,
                                &matrix_mv);

             it = eina_hash_iterator_data_new(pd_mesh_node->data.mesh.node_meshes);
             while (eina_iterator_next(it, &ptr))
               {
                  Evas_Canvas3D_Node_Mesh *nm = (Evas_Canvas3D_Node_Mesh *)ptr;
                  Evas_Canvas3D_Mesh_Data *pdmesh = efl_data_scope_get(nm->mesh, EVAS_CANVAS3D_MESH_CLASS);
                  /*In case LOD enable pass in render only LOD meshes in dependences of the
                  distance to the camera node:
                  near_boundary <= distance <= far_boundary*/
                  CHECK_LOD_DISTANCE
                  if (data->shadows_enabled)
                    {
                       pdmesh->shadowed = EINA_TRUE;
                       _mesh_draw(renderer, nm->mesh, nm->frame, light, matrix_eye, &matrix_mv, &matrix_mvp, &matrix_light);
                       pdmesh->shadowed = EINA_FALSE;
                    }
                  else _mesh_draw(renderer, nm->mesh, nm->frame, light, matrix_eye, &matrix_mv, &matrix_mvp, NULL);
               }
             eina_iterator_free(it);
          }
     }
   e3d_renderer_flush(renderer);
}

void
e3d_drawable_scene_render(E3D_Drawable *drawable, E3D_Renderer *renderer, Evas_Canvas3D_Scene_Public_Data *data)
{
   /* Set up render target. */
   e3d_renderer_target_set(renderer, drawable);
   e3d_renderer_clear(renderer, &data->bg_color);

   if (data->post_processing)
     e3d_drawable_scene_render_to_texture(drawable, renderer, data);//Render to additional texture
   else
     _scene_render(drawable, renderer, data);//Common main render
}

void
_scene_post_render(E3D_Drawable *drawable, E3D_Renderer *renderer, Evas_Canvas3D_Scene_Public_Data *data)
{
   E3D_Draw_Data   drawable_data;
   float vertices_of_square[] = {-1.0, 1.0, 0.0, 1.0, 1.0, 0.0,
                                 -1.0, -1.0, 0.0, 1.0, -1.0, 0.0};
   unsigned short indices_of_square[] = {0, 1, 2, 2, 1, 3};

   memset(&drawable_data, 0x00, sizeof(E3D_Draw_Data));

   drawable_data.flags |= E3D_SHADER_FLAG_VERTEX_POSITION;
   drawable_data.mode = data->post_processing_type;
   drawable_data.assembly = EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES;
   drawable_data.indices = indices_of_square;
   drawable_data.vertex_count = 4;
   drawable_data.index_count = 6;
   drawable_data.index_format = EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT;
   eina_matrix4_identity(&drawable_data.matrix_mv);
   eina_matrix4_identity(&drawable_data.matrix_mvp);
   drawable_data.frame_size_h = (Evas_Real)drawable->h;
   drawable_data.frame_size_w = (Evas_Real)drawable->w;
   /*Initialize Evas_Canvas3D_Vertex_Buffer for full screen quard*/
   drawable_data.vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].vertex0.data = (void *)vertices_of_square;
   drawable_data.vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].vertex0.element_count = 3;
   drawable_data.vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].vertex0.stride = 3 * sizeof(float);
   drawable_data.vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].vertex0.owns_data = EINA_FALSE;
   drawable_data.vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION].vertex0.size = 0;

   e3d_renderer_clear(renderer, &data->bg_color);

   /*Initialize texture units*/
   drawable_data.smap_sampler = e3d_renderer_sampler_shadowmap_get(renderer);
   drawable_data.colortex_sampler = e3d_renderer_sampler_colortexture_get(renderer);

   /*Render full screen quard with color pick texture unit*/
   e3d_renderer_draw(renderer, &drawable_data);
   e3d_renderer_flush(renderer);
}

Eina_Bool
e3d_drawable_scene_render_to_texture(E3D_Drawable *drawable, E3D_Renderer *renderer,
                                     Evas_Canvas3D_Scene_Public_Data *data)
{
   const Eina_Matrix4  *matrix_eye;
   Evas_Canvas3D_Shader_Mode shader_mode;
   Eina_Stringshare *tmp;
   Eina_Iterator *itmn;
   void *ptrmn;
   Eina_List *repeat_node = NULL;
   Evas_Color *unic_color = NULL;

   e3d_renderer_color_pick_target_set(renderer, drawable);
   e3d_renderer_clear(renderer, &data->bg_color);

   if (data->color_pick_enabled) //Use rendering to texture in color pick mechanism
     {
         Evas_Canvas3D_Node_Data *pd_camera_node = efl_data_scope_get(data->camera_node, EVAS_CANVAS3D_NODE_CLASS);
         matrix_eye = &pd_camera_node->data.camera.matrix_world_to_eye;
         Evas_Canvas3D_Camera_Data *pd = efl_data_scope_get(pd_camera_node->data.camera.camera, EVAS_CANVAS3D_CAMERA_CLASS);

         itmn = eina_hash_iterator_data_new(data->colors_node_mesh);

         while (eina_iterator_next(itmn, &ptrmn))
           {
              Evas_Canvas3D_Node      *n;
              Eina_Array *arr = NULL;

              arr = (Eina_Array *)ptrmn;
              n = (Evas_Canvas3D_Node *)eina_array_data_get(arr, 0);
              /*To avoid repeatedly render mesh*/
              if (!repeat_node)
                repeat_node = eina_list_append(repeat_node, (void*)n);
              else
                {
                   if (eina_list_data_find(repeat_node, (void *)n))
                     continue;
                   else
                     repeat_node = eina_list_append(repeat_node, (void *)n);
                }
              Evas_Canvas3D_Node_Data *pd_mesh_node = efl_data_scope_get(n, EVAS_CANVAS3D_NODE_CLASS);
              RENDER_MESH_NODE_ITERATE_BEGIN(eye)
                {
                   if (pdmesh->color_pick_enabled)
                     {
                        tmp = eina_stringshare_printf("%p %p", n, nm->mesh);
                        unic_color = (Evas_Color *)eina_hash_find(data->node_mesh_colors, tmp);
                        if (unic_color)
                          {
                             pdmesh->color_pick_key.r = unic_color->r;
                             pdmesh->color_pick_key.g = unic_color->g;
                             pdmesh->color_pick_key.b = unic_color->b;
                             shader_mode = pdmesh->shader_mode;
                             pdmesh->shader_mode = EVAS_CANVAS3D_SHADER_MODE_COLOR_PICK;
                             _mesh_draw(renderer, nm->mesh, nm->frame, NULL, matrix_eye, &matrix_mv,
                                        &matrix_mvp, NULL);
                             pdmesh->shader_mode = shader_mode;
                          }
                        eina_stringshare_del(tmp);
                     }
                }
              RENDER_MESH_NODE_ITERATE_END
           }
        eina_iterator_free(itmn);
        eina_list_free(repeat_node);
     }
   else
     {
        _scene_render(drawable, renderer, data); //Just render scene in texture
        glBindFramebuffer(GL_FRAMEBUFFER, drawable->fbo);
        /*Render full screen quard*/
        if (data->post_processing)
          _scene_post_render(drawable, renderer, data);
     }

   return EINA_TRUE;
}

void
e3d_drawable_texture_pixel_color_get(GLuint tex EINA_UNUSED, int x, int y,
                                     Evas_Color *color, void *drawable)
{
   E3D_Drawable *d = (E3D_Drawable *)drawable;

   glBindFramebuffer(GL_FRAMEBUFFER, d->color_pick_fb_id);

   GLubyte pixel[4] = {0, 0, 0, 0};
   glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
   color->r = (double)pixel[0] / 255;
   color->g = (double)pixel[1] / 255;
   color->b = (double)pixel[2] / 255;

   glBindFramebuffer(GL_FRAMEBUFFER, d->fbo);
}

void
e3d_drawable_texture_rendered_pixels_get(GLuint tex EINA_UNUSED, int x, int y, int w, int h,
                                         void *drawable EINA_UNUSED, void *data)
{
   DATA32 *buffer = (DATA32 *)data;
   DATA32 *datarowup = NULL, *datarowlow = NULL;
   DATA32 pixel;
   int i, j, width = 0, up, bellow;

   glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLubyte *)buffer);

  /*Due to returned pixels buffer filled as from the
   bottom left of the screen going up to the top right*/
   datarowup = malloc(w * sizeof(DATA32));
   datarowlow = malloc(w * sizeof(DATA32));

   if (!datarowup || !datarowlow)
     {
        ERR("Not enough memory");
        if (datarowup) free(datarowup);
        if (datarowlow) free(datarowlow);
        return;
     }
   for (j = 0; j < h / 2; j++)
     {
        bellow = h * w - width;
        up = w + width;

        for (i = w - 1; i >= 0; i--)
          {
             pixel = buffer[bellow];
             datarowlow[i] = ((pixel & 0x000000ff) << 16) +
                             ((pixel & 0x00ff0000) >> 16)  +
                             ((pixel & 0xff00ff00));
             pixel = buffer[up];
             datarowup[i] = ((pixel & 0x000000ff) << 16) +
                             ((pixel & 0x00ff0000) >> 16)  +
                             ((pixel & 0xff00ff00));
             bellow--;
             up--;
          }
        memcpy(buffer + width, datarowlow, w * sizeof(DATA32));
        width += w;
        memcpy(buffer + (h * w - width), datarowup, w * sizeof(DATA32));
     }
   free(datarowup);
   free(datarowlow);
}
#undef CHECK_LOD_DISTANCE
#undef RENDER_MESH_NODE_ITERATE_BEGIN
#undef RENDER_MESH_NODE_ITERATE_END
