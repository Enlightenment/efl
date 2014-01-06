#include <stdio.h>
#include <png.h>
#include "evas_gl_private.h"
#include "evas_gl_3d_private.h"

void
e3d_texture_param_update(E3D_Texture *texture)
{
   if (texture->is_imported)
     return;

   if (texture->wrap_dirty)
     {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrap_t);
        texture->wrap_dirty = EINA_FALSE;
     }

   if (texture->filter_dirty)
     {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->filter_min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->filter_mag);
        texture->filter_dirty = EINA_FALSE;
     }
}

E3D_Texture *
e3d_texture_new(void)
{
   E3D_Texture *texture = NULL;

   texture = (E3D_Texture *)malloc(sizeof(E3D_Texture));

   if (texture == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   texture->w = 0;
   texture->h = 0;

   texture->is_imported = EINA_FALSE;
   texture->tex = 0;
   texture->format = GL_RGBA;

   texture->wrap_dirty = EINA_TRUE;
   texture->wrap_s = GL_CLAMP_TO_EDGE;
   texture->wrap_t = GL_CLAMP_TO_EDGE;

   texture->filter_dirty = EINA_TRUE;
   texture->filter_min = GL_NEAREST;
   texture->filter_mag = GL_NEAREST;

   return texture;
}

void
e3d_texture_free(E3D_Texture *texture)
{
   if (texture->tex && !texture->is_imported)
     glDeleteTextures(1, &texture->tex);

   free(texture);
}

void
e3d_texture_data_set(E3D_Texture *texture,
                     Evas_3D_Color_Format color_format, Evas_3D_Pixel_Format pixel_format,
                     int w, int h, const void *data)
{
   GLenum   format;
   GLenum   iformat;
   GLenum   type;

   if (color_format == EVAS_3D_COLOR_FORMAT_RGBA)
     {
        format = GL_RGBA;
        iformat = GL_RGBA;

        if (pixel_format == EVAS_3D_PIXEL_FORMAT_8888)
          type = GL_UNSIGNED_BYTE;
        else if (pixel_format == EVAS_3D_PIXEL_FORMAT_4444)
          type = GL_UNSIGNED_SHORT_4_4_4_4;
        else if (pixel_format == EVAS_3D_PIXEL_FORMAT_5551)
          type = GL_UNSIGNED_SHORT_5_5_5_1;
        else
          {
             ERR("Texture data format mismatch.");
             return;
          }
     }
   else if (color_format == EVAS_3D_COLOR_FORMAT_RGB)
     {
        format = GL_RGB;
        iformat = GL_RGB;

        if (pixel_format == EVAS_3D_PIXEL_FORMAT_565)
          type = GL_UNSIGNED_SHORT_5_6_5;
        else if (pixel_format == EVAS_3D_PIXEL_FORMAT_888)
          type = GL_UNSIGNED_BYTE;
        else
          {
             ERR("Texture data format mismatch.");
             return;
          }
     }
   else if (color_format == EVAS_3D_COLOR_FORMAT_ALPHA)
     {
        format = GL_LUMINANCE;
        iformat = GL_LUMINANCE;

        if (pixel_format == EVAS_3D_PIXEL_FORMAT_8)
          type = GL_UNSIGNED_BYTE;
        else
          {
             ERR("Texture data format mismatch.");
             return;
          }
     }
   else
     {
        ERR("Invalid texture color format");
        return;
     }

   if (texture->tex == 0 || texture->is_imported)
     {
        glGenTextures(1, &texture->tex);
        texture->wrap_dirty = EINA_TRUE;
        texture->filter_dirty = EINA_TRUE;
     }

   glBindTexture(GL_TEXTURE_2D, texture->tex);
   glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, iformat, type, data);

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

   texture->is_imported = EINA_FALSE;
   texture->format = format;
}

void
e3d_texture_file_set(E3D_Texture *texture, const char *file, const char *key EINA_UNUSED)
{
   Evas_3D_Color_Format color_format;
   Evas_3D_Pixel_Format pixel_format;

   FILE          *fp;
   png_structp    png_ptr = NULL;
   png_infop      info_ptr = NULL;
   void          *buffer = NULL;
   png_bytep     *row_pointers = NULL;

   int            w, h;
   png_byte       color_type;
   png_byte       bit_depth;
   png_byte       header[8];
   int            ret;
   int            y;
   int            stride;

   fp = fopen(file, "rb");

   if (fp == NULL)
     {
        ERR("Failed to open file %s", file);
        goto done;
     }

   ret = fread(header, 1, 8, fp);

   if (ret < 8)
     goto done;

   if (png_sig_cmp(header, 0, 8))
     {
        ERR("%s does not seem to be a png file.", file);
        goto done;
     }

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   if (png_ptr == NULL)
     goto done;

   info_ptr = png_create_info_struct(png_ptr);

   if (info_ptr == NULL)
     goto done;

   if (setjmp(png_jmpbuf(png_ptr)))
     goto done;

   png_init_io(png_ptr, fp);
   png_set_sig_bytes(png_ptr, 8);

   png_read_info(png_ptr, info_ptr);

   bit_depth = png_get_bit_depth(png_ptr, info_ptr);

   if (bit_depth != 8)
     {
        ERR("Unsupported bit depth %d.", bit_depth);
        goto done;
     }

   w = png_get_image_width(png_ptr, info_ptr);
   h = png_get_image_height(png_ptr, info_ptr);

   if (w < 1 || h < 1 || w > IMG_MAX_SIZE || h > IMG_MAX_SIZE || IMG_TOO_BIG(w, h))
     {
        ERR("Image %s(%d x %d)  is too big for texture.", file, w, h);
        goto done;
     }

   color_type = png_get_color_type(png_ptr, info_ptr);

   if (color_type == PNG_COLOR_TYPE_GRAY)
     {
        color_format = EVAS_3D_COLOR_FORMAT_ALPHA;
        pixel_format = EVAS_3D_PIXEL_FORMAT_8;
        stride = (w + 3) & ~3;
     }
   else if (color_type == PNG_COLOR_TYPE_RGB)
     {
        color_format = EVAS_3D_COLOR_FORMAT_RGB;
        pixel_format = EVAS_3D_PIXEL_FORMAT_888;
        stride = (w * 3 + 3) & ~3;
     }
   else if (color_type == PNG_COLOR_TYPE_RGBA)
     {
        color_format = EVAS_3D_COLOR_FORMAT_RGBA;
        pixel_format = EVAS_3D_PIXEL_FORMAT_8888;
        stride = w * 4;
     }
   else
     {
        ERR("Unsupported color format.");
        goto done;
     }

   row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * h);

   if (row_pointers == NULL)
     goto done;

   buffer = malloc(stride * h);

   if (buffer == NULL)
     goto done;

   for (y = 0; y < h; y++)
     row_pointers[y] = ((png_byte *)buffer) + (h - y - 1)* stride;

   png_read_image(png_ptr, row_pointers);

   e3d_texture_data_set(texture, color_format, pixel_format, w, h, buffer);

done:
   if (png_ptr)
     png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : NULL, NULL);

   if (fp)
     fclose(fp);

   if (row_pointers)
     free(row_pointers);

   if (buffer)
     free(buffer);
}

Evas_3D_Color_Format
e3d_texture_color_format_get(E3D_Texture *texture)
{
   if (texture->is_imported)
     {
        ERR("Cannot get the size of an imported texture.");
        return EVAS_3D_COLOR_FORMAT_RGBA;
     }

   switch (texture->format)
     {
      case GL_RGBA:
         return EVAS_3D_COLOR_FORMAT_RGBA;
      case GL_RGB:
         return EVAS_3D_COLOR_FORMAT_RGB;
      case GL_ALPHA:
         return EVAS_3D_COLOR_FORMAT_ALPHA;
      default:
         break;
     }

   ERR("Invalid texture format.");
   return EVAS_3D_COLOR_FORMAT_RGBA;
}

void
e3d_texture_size_get(const E3D_Texture *texture, int *w, int *h)
{
   if (texture->is_imported)
     {
        ERR("Invalid operation on an imported texture resource.");
        return;
     }

   if (w) *w = texture->w;
   if (h) *h = texture->h;
}

void
e3d_texture_import(E3D_Texture *texture, GLuint tex)
{
   if (tex == 0)
     {
        ERR("Cannot import an invalid texture ID.");
        return;
     }

   if (texture->tex && !texture->is_imported)
     glDeleteTextures(1, &texture->tex);

   texture->tex = tex;
   texture->is_imported = EINA_TRUE;
}

Eina_Bool
e3d_texture_is_imported_get(const E3D_Texture *texture)
{
   return texture->is_imported;
}

static inline GLenum
_to_gl_texture_wrap(Evas_3D_Wrap_Mode wrap)
{
   switch (wrap)
     {
      case EVAS_3D_WRAP_MODE_CLAMP:
         return GL_CLAMP_TO_EDGE;
      case EVAS_3D_WRAP_MODE_REFLECT:
         return GL_MIRRORED_REPEAT;
      case EVAS_3D_WRAP_MODE_REPEAT:
         return GL_REPEAT;
      default:
         break;
     }

   ERR("Invalid texture wrap mode.");
   return GL_CLAMP_TO_EDGE;
}

static inline Evas_3D_Wrap_Mode
_to_e3d_texture_wrap(GLenum wrap)
{
   switch (wrap)
     {
      case GL_CLAMP_TO_EDGE:
         return EVAS_3D_WRAP_MODE_CLAMP;
      case GL_MIRRORED_REPEAT:
         return EVAS_3D_WRAP_MODE_REFLECT;
      case GL_REPEAT:
         return EVAS_3D_WRAP_MODE_REPEAT;
      default:
         break;
     }

   ERR("Invalid texture wrap mode.");
   return EVAS_3D_WRAP_MODE_CLAMP;
}

static inline GLenum
_to_gl_texture_filter(Evas_3D_Texture_Filter filter)
{
   switch (filter)
     {
      case EVAS_3D_TEXTURE_FILTER_NEAREST:
         return GL_NEAREST;
      case EVAS_3D_TEXTURE_FILTER_LINEAR:
         return GL_LINEAR;
      case EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
         return GL_NEAREST_MIPMAP_NEAREST;
      case EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
         return GL_NEAREST_MIPMAP_LINEAR;
      case EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
         return GL_LINEAR_MIPMAP_NEAREST;
      case EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
         return GL_LINEAR_MIPMAP_LINEAR;
      default:
         break;
     }

   ERR("Invalid texture wrap mode.");
   return GL_NEAREST;
}

static inline Evas_3D_Texture_Filter
_to_e3d_texture_filter(GLenum filter)
{
   switch (filter)
     {
      case GL_NEAREST:
         return EVAS_3D_TEXTURE_FILTER_NEAREST;
      case GL_LINEAR:
         return EVAS_3D_TEXTURE_FILTER_LINEAR;
      case GL_NEAREST_MIPMAP_NEAREST:
         return EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;
      case GL_NEAREST_MIPMAP_LINEAR:
         return EVAS_3D_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
      case GL_LINEAR_MIPMAP_NEAREST:
         return EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST;
      case GL_LINEAR_MIPMAP_LINEAR:
         return EVAS_3D_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
      default:
         break;
     }

   ERR("Invalid texture wrap mode.");
   return EVAS_3D_TEXTURE_FILTER_NEAREST;
}

void
e3d_texture_wrap_set(E3D_Texture *texture, Evas_3D_Wrap_Mode s, Evas_3D_Wrap_Mode t)
{
   GLenum gl_s, gl_t;

   if (texture->is_imported)
     {
        ERR("Invalid operation on an imported texture resource.");
        return;
     }

   gl_s = _to_gl_texture_wrap(s);
   gl_t = _to_gl_texture_wrap(t);

   if (gl_s == texture->wrap_s && gl_t == texture->wrap_t)
     return;

   texture->wrap_s = gl_s;
   texture->wrap_t = gl_t;
   texture->wrap_dirty = EINA_TRUE;
}

void
e3d_texture_wrap_get(const E3D_Texture *texture, Evas_3D_Wrap_Mode *s, Evas_3D_Wrap_Mode *t)
{
   if (texture->is_imported)
     {
        ERR("Invalid operation on an imported texture resource.");
        return;
     }

   if (s)
     *s = _to_e3d_texture_wrap(texture->wrap_s);

   if (t)
     *t = _to_e3d_texture_wrap(texture->wrap_t);
}

void
e3d_texture_filter_set(E3D_Texture *texture, Evas_3D_Texture_Filter min, Evas_3D_Texture_Filter mag)
{
   GLenum gl_min, gl_mag;

   if (texture->is_imported)
     {
        ERR("Invalid operation on an imported texture resource.");
        return;
     }

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
                       Evas_3D_Texture_Filter *min, Evas_3D_Texture_Filter *mag)
{
   if (texture->is_imported)
     {
        ERR("Invalid operation on an imported texture resource.");
        return;
     }

   if (min)
     *min = _to_e3d_texture_filter(texture->filter_min);

   if (mag)
     *mag = _to_e3d_texture_filter(texture->filter_mag);
}

E3D_Drawable *
e3d_drawable_new(int w, int h, int alpha, GLenum depth_format, GLenum stencil_format)
{
   E3D_Drawable  *drawable;
   GLuint         tex, fbo;
   GLuint         depth_stencil_buf = 0;
   GLuint         depth_buf = 0;
   GLuint         stencil_buf = 0;
   Eina_Bool      depth_stencil = EINA_FALSE;

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   if (alpha)
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   else
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

   glGenFramebuffers(1, &fbo);
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

#if GL_GLES
   if (depth_format == GL_DEPTH_STENCIL_OES)
     {
        glGenTextures(1, &depth_stencil_buf);
        glBindTexture(GL_TEXTURE_2D, depth_stencil_buf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
        glGenRenderbuffers(1, &depth_stencil_buf);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_buf);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, depth_stencil_buf);

        depth_stencil = EINA_TRUE;
     }
#endif

   if ((!depth_stencil) && (depth_format))
     {
        glGenRenderbuffers(1, &depth_buf);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buf);
        glRenderbufferStorage(GL_RENDERBUFFER, depth_format, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, depth_buf);
     }

   if ((!depth_stencil) && (stencil_format))
     {
        glGenRenderbuffers(1, &stencil_buf);
        glBindRenderbuffer(GL_RENDERBUFFER, stencil_buf);
        glRenderbufferStorage(GL_RENDERBUFFER, stencil_format, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, stencil_buf);
     }

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
   drawable->depth_stencil_buf = depth_stencil_buf;
   drawable->depth_buf = depth_buf;
   drawable->stencil_buf = stencil_buf;

   return drawable;

error:
   ERR("Drawable creation failed.");

   if (tex)
     glDeleteTextures(1, &tex);

   if (fbo)
     glDeleteFramebuffers(1, &fbo);

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

   if (drawable)
     free(drawable);

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

GLenum
e3d_drawable_format_get(E3D_Drawable *drawable)
{
   return drawable->format;
}

static inline GLuint
_texture_id_get(Evas_3D_Texture *texture)
{
   E3D_Texture *tex = (E3D_Texture *)texture->engine_data;

   return tex->tex;
}

static inline void
_mesh_frame_find(Evas_3D_Mesh *mesh, int frame,
                 Eina_List **l, Eina_List **r)
{
   Eina_List *left, *right;
   Evas_3D_Mesh_Frame *f0, *f1;

   left = mesh->frames;
   right = eina_list_next(left);

   while (right)
     {
        f0 = (Evas_3D_Mesh_Frame *)eina_list_data_get(left);
        f1 = (Evas_3D_Mesh_Frame *)eina_list_data_get(right);

        if (frame >= f0->frame && frame <= f1->frame)
          break;

        left = right;
        right = eina_list_next(left);
     }

   if (right == NULL)
     {
        if (frame <= f0->frame)
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
                        Evas_3D_Vertex_Attrib attrib,
                        Eina_Bool blend)
{
   switch (attrib)
     {
      case EVAS_3D_VERTEX_POSITION:
         data->flags |= E3D_SHADER_FLAG_VERTEX_POSITION;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_POSITION_BLEND;
         break;
      case EVAS_3D_VERTEX_NORMAL:
         data->flags |= E3D_SHADER_FLAG_VERTEX_NORMAL;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_NORMAL_BLEND;
         break;
      case EVAS_3D_VERTEX_TANGENT:
         data->flags |= E3D_SHADER_FLAG_VERTEX_TANGENT;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_TANGENT_BLEND;
         break;
      case EVAS_3D_VERTEX_COLOR:
         data->flags |= E3D_SHADER_FLAG_VERTEX_COLOR;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_VERTEX_COLOR_BLEND;
         break;
      case EVAS_3D_VERTEX_TEXCOORD:
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
_material_color_flag_add(E3D_Draw_Data *data, Evas_3D_Material_Attrib attrib)
{
   switch (attrib)
     {
      case EVAS_3D_MATERIAL_AMBIENT:
         data->flags |= E3D_SHADER_FLAG_AMBIENT;
         break;
      case EVAS_3D_MATERIAL_DIFFUSE:
         data->flags |= E3D_SHADER_FLAG_DIFFUSE;
         break;
      case EVAS_3D_MATERIAL_SPECULAR:
         data->flags |= E3D_SHADER_FLAG_SPECULAR;
         break;
      case EVAS_3D_MATERIAL_EMISSION:
         data->flags |= E3D_SHADER_FLAG_EMISSION;
         break;
      case EVAS_3D_MATERIAL_NORMAL:
         ERR("Material attribute normal should not be used with color values.");
         break;
      default:
         ERR("Invalid material attrib.");
         break;
     }
}

static inline void
_material_texture_flag_add(E3D_Draw_Data *data, Evas_3D_Material_Attrib attrib, Eina_Bool blend)
{
   switch (attrib)
     {
      case EVAS_3D_MATERIAL_AMBIENT:
         data->flags |= E3D_SHADER_FLAG_AMBIENT;
         data->flags |= E3D_SHADER_FLAG_AMBIENT_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_AMBIENT_TEXTURE_BLEND;
         break;
      case EVAS_3D_MATERIAL_DIFFUSE:
         data->flags |= E3D_SHADER_FLAG_DIFFUSE;
         data->flags |= E3D_SHADER_FLAG_DIFFUSE_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_DIFFUSE_TEXTURE_BLEND;
         break;
      case EVAS_3D_MATERIAL_SPECULAR:
         data->flags |= E3D_SHADER_FLAG_SPECULAR;
         data->flags |= E3D_SHADER_FLAG_SPECULAR_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_SPECULAR_TEXTURE_BLEND;
         break;
      case EVAS_3D_MATERIAL_EMISSION:
         data->flags |= E3D_SHADER_FLAG_EMISSION;
         data->flags |= E3D_SHADER_FLAG_EMISSION_TEXTURE;

         if (blend)
           data->flags |= E3D_SHADER_FLAG_EMISSION_TEXTURE_BLEND;
         break;
      case EVAS_3D_MATERIAL_NORMAL:
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
                     Evas_3D_Vertex_Attrib attrib)
{
   const Evas_3D_Mesh_Frame *f0 = NULL, *f1 = NULL;

   while (l)
     {
        f0 = (const Evas_3D_Mesh_Frame *)eina_list_data_get(l);

        if (f0->vertices[attrib].data != NULL)
          break;

        l = eina_list_prev(l);
        f0 = NULL;
     }

   while (r)
     {
        f1 = (const Evas_3D_Mesh_Frame *)eina_list_data_get(r);

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
                      Evas_3D_Material_Attrib attrib)
{
   const Evas_3D_Mesh_Frame *f0 = NULL, *f1 = NULL;

   while (l)
     {
        f0 = (const Evas_3D_Mesh_Frame *)eina_list_data_get(l);

        if (f0->material && f0->material->attribs[attrib].enable)
          break;

        l = eina_list_prev(l);
        f0 = NULL;
     }

   while (r)
     {
        f1 = (const Evas_3D_Mesh_Frame *)eina_list_data_get(r);

        if (f1->material && f1->material->attribs[attrib].enable)
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

   if (f1 == NULL)
     {
        data->materials[attrib].color = f0->material->attribs[attrib].color;

        if (attrib == EVAS_3D_MATERIAL_SPECULAR)
          data->shininess = f0->material->shininess;
     }
   else
     {
        Evas_Real weight;

        weight = (f1->frame - frame) / (Evas_Real)(f1->frame - f0->frame);
        evas_color_blend(&data->materials[attrib].color,
                         &f0->material->attribs[attrib].color,
                         &f1->material->attribs[attrib].color,
                         weight);

        if (attrib == EVAS_3D_MATERIAL_SPECULAR)
          {
             data->shininess = f0->material->shininess * weight +
                f1->material->shininess * (1.0 - weight);
          }
     }

   _material_color_flag_add(data, attrib);
   return EINA_TRUE;
}

static inline Eina_Bool
_material_texture_build(E3D_Draw_Data *data, int frame,
                        const Eina_List *l, const Eina_List *r,
                        Evas_3D_Material_Attrib attrib)
{
   const Evas_3D_Mesh_Frame *f0 = NULL, *f1 = NULL;

   while (l)
     {
        f0 = (const Evas_3D_Mesh_Frame *)eina_list_data_get(l);

        if (f0->material &&
            f0->material->attribs[attrib].enable &&
            f0->material->attribs[attrib].texture != NULL)
          break;

        l = eina_list_prev(l);
        f0 = NULL;
     }

   while (r)
     {
        f1 = (const Evas_3D_Mesh_Frame *)eina_list_data_get(r);

        if (f1->material &&
            f1->material->attribs[attrib].enable &&
            f1->material->attribs[attrib].texture != NULL)
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

   data->materials[attrib].sampler0 = data->texture_count++;
   data->materials[attrib].tex0 =
      (E3D_Texture *)f0->material->attribs[attrib].texture->engine_data;

   if (f1)
     {
        Evas_Real weight = (f1->frame - frame) / (Evas_Real)(f1->frame - f0->frame);

        data->materials[attrib].sampler1 = data->texture_count++;
        data->materials[attrib].tex1 =
           (E3D_Texture *)f1->material->attribs[attrib].texture->engine_data;

        data->materials[attrib].texture_weight = weight;

        if (attrib == EVAS_3D_MATERIAL_SPECULAR)
          {
             data->shininess = f0->material->shininess * weight +
                f1->material->shininess * (1.0 - weight);
          }

        _material_texture_flag_add(data, attrib, EINA_TRUE);
     }
   else
     {
        if (attrib == EVAS_3D_MATERIAL_SPECULAR)
          data->shininess = f0->material->shininess;

        _material_texture_flag_add(data, attrib, EINA_FALSE);
     }

   return EINA_TRUE;
}

static inline void
_light_build(E3D_Draw_Data *data,
             const Evas_3D_Node *light,
             const Evas_Mat4    *matrix_eye)
{
   Evas_3D_Light *l = light->data.light.light;
   Evas_Vec3      pos, dir;

   if (l == NULL)
     return;

   /* Get light node's position. */
   if (l->directional)
     {
        data->flags |= E3D_SHADER_FLAG_LIGHT_DIRECTIONAL;

        /* Negative Z. */
        evas_vec3_set(&dir, 0.0, 0.0, 1.0);
        evas_vec3_quaternion_rotate(&dir, &dir, &light->orientation);

        /* Transform to eye space. */
        evas_vec3_homogeneous_direction_transform(&dir, &dir, matrix_eye);
        evas_vec3_normalize(&dir, &dir);

        data->light.position.x = dir.x;
        data->light.position.y = dir.y;
        data->light.position.z = dir.z;
        data->light.position.w = 0.0;
     }
   else
     {
        evas_vec3_copy(&pos, &light->position_world);
        evas_vec3_homogeneous_position_transform(&pos, &pos, matrix_eye);

        data->light.position.x = pos.x;
        data->light.position.y = pos.y;
        data->light.position.z = pos.z;
        data->light.position.w = 1.0;

        if (l->enable_attenuation)
          {
             data->flags |= E3D_SHADER_FLAG_LIGHT_ATTENUATION;

             data->light.atten.x = l->atten_const;
             data->light.atten.x = l->atten_linear;
             data->light.atten.x = l->atten_quad;
          }

        if (l->spot_cutoff < 180.0)
          {
             data->flags |= E3D_SHADER_FLAG_LIGHT_SPOT;
             evas_vec3_set(&dir, 0.0, 0.0, -1.0);
             evas_vec3_quaternion_rotate(&dir, &dir, &light->orientation);
             evas_vec3_homogeneous_direction_transform(&dir, &dir, matrix_eye);

             data->light.spot_dir = dir;
             data->light.spot_exp = l->spot_exp;
             data->light.spot_cutoff_cos = l->spot_cutoff_cos;
          }
     }

   data->light.ambient = l->ambient;
   data->light.diffuse = l->diffuse;
   data->light.specular = l->specular;
}

static inline Eina_Bool
_mesh_draw_data_build(E3D_Draw_Data *data,
                      Evas_3D_Mesh *mesh, int frame,
                      const Evas_Mat4 *matrix_eye,
                      const Evas_Mat4 *matrix_mv,
                      const Evas_Mat4 *matrix_mvp,
                      const Evas_3D_Node *light)
{
   Eina_List *l, *r;

   if (mesh->frames == NULL)
     return EINA_FALSE;

   data->mode = mesh->shade_mode;
   data->assembly = mesh->assembly;
   data->vertex_count = mesh->vertex_count;
   data->index_count = mesh->index_count;
   data->index_format = mesh->index_format;
   data->indices = mesh->indices;

   evas_mat4_copy(&data->matrix_mvp, matrix_mvp);
   evas_mat4_copy(&data->matrix_mv, matrix_mv);

   _mesh_frame_find(mesh, frame, &l, &r);

#define BUILD(type, arg, check)                                               \
   do {                                                                       \
        Eina_Bool ret = _##type##_build(data, frame, l, r, EVAS_3D_##arg);    \
        if (check && !ret)                                                    \
          {                                                                   \
             ERR("Missing attribute : " #arg);                                \
             return EINA_FALSE;                                               \
          }                                                                   \
   } while (0)

   if (mesh->shade_mode == EVAS_3D_SHADE_MODE_VERTEX_COLOR)
     {
        BUILD(vertex_attrib,     VERTEX_POSITION,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_COLOR,        EINA_TRUE);
     }
   else if (mesh->shade_mode == EVAS_3D_SHADE_MODE_DIFFUSE)
     {
        BUILD(vertex_attrib,     VERTEX_POSITION,     EINA_TRUE);
        BUILD(material_color,    MATERIAL_DIFFUSE,    EINA_TRUE);
        BUILD(material_texture,  MATERIAL_DIFFUSE,    EINA_FALSE);

        if (_flags_need_tex_coord(data->flags))
          BUILD(vertex_attrib,     VERTEX_TEXCOORD,     EINA_FALSE);
     }
   else if (mesh->shade_mode == EVAS_3D_SHADE_MODE_FLAT)
     {
        BUILD(vertex_attrib,     VERTEX_POSITION,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_NORMAL,       EINA_TRUE);

        BUILD(material_color,    MATERIAL_AMBIENT,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_DIFFUSE,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_SPECULAR,   EINA_FALSE);
        BUILD(material_color,    MATERIAL_EMISSION,   EINA_FALSE);

        BUILD(material_texture,  MATERIAL_AMBIENT,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_DIFFUSE,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_SPECULAR,   EINA_FALSE);
        BUILD(material_texture,  MATERIAL_EMISSION,   EINA_FALSE);

        _light_build(data, light, matrix_eye);
        evas_normal_matrix_get(&data->matrix_normal, matrix_mv);

        if (_flags_need_tex_coord(data->flags))
          BUILD(vertex_attrib,     VERTEX_TEXCOORD,     EINA_FALSE);
     }
   else if (mesh->shade_mode == EVAS_3D_SHADE_MODE_PHONG)
     {
        BUILD(vertex_attrib,     VERTEX_POSITION,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_NORMAL,       EINA_TRUE);

        BUILD(material_color,    MATERIAL_AMBIENT,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_DIFFUSE,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_SPECULAR,   EINA_FALSE);
        BUILD(material_color,    MATERIAL_EMISSION,   EINA_FALSE);

        BUILD(material_texture,  MATERIAL_AMBIENT,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_DIFFUSE,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_SPECULAR,   EINA_FALSE);
        BUILD(material_texture,  MATERIAL_EMISSION,   EINA_FALSE);

        _light_build(data, light, matrix_eye);
        evas_normal_matrix_get(&data->matrix_normal, matrix_mv);

        if (_flags_need_tex_coord(data->flags))
          BUILD(vertex_attrib,     VERTEX_TEXCOORD,     EINA_FALSE);
     }
   else if (mesh->shade_mode == EVAS_3D_SHADE_MODE_NORMAL_MAP)
     {
        BUILD(vertex_attrib,     VERTEX_POSITION,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_NORMAL,       EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_TEXCOORD,     EINA_TRUE);
        BUILD(material_texture,  MATERIAL_NORMAL,     EINA_TRUE);
        BUILD(vertex_attrib,     VERTEX_TANGENT,      EINA_FALSE);

        BUILD(material_color,    MATERIAL_AMBIENT,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_DIFFUSE,    EINA_FALSE);
        BUILD(material_color,    MATERIAL_SPECULAR,   EINA_FALSE);
        BUILD(material_color,    MATERIAL_EMISSION,   EINA_FALSE);

        BUILD(material_texture,  MATERIAL_AMBIENT,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_DIFFUSE,    EINA_FALSE);
        BUILD(material_texture,  MATERIAL_SPECULAR,   EINA_FALSE);
        BUILD(material_texture,  MATERIAL_EMISSION,   EINA_FALSE);

        _light_build(data, light, matrix_eye);
        evas_normal_matrix_get(&data->matrix_normal, matrix_mv);
     }

   return EINA_TRUE;
}

static inline void
_mesh_draw(E3D_Renderer *renderer, Evas_3D_Mesh *mesh, int frame, Evas_3D_Node *light,
           const Evas_Mat4 *matrix_eye, const Evas_Mat4 *matrix_mv, const Evas_Mat4 *matrix_mvp)
{
   E3D_Draw_Data   data;

   memset(&data, 0x00, sizeof(E3D_Draw_Data));

   if (_mesh_draw_data_build(&data, mesh, frame, matrix_eye, matrix_mv, matrix_mvp, light))
     e3d_renderer_draw(renderer, &data);
}

void
e3d_drawable_scene_render(E3D_Drawable *drawable, E3D_Renderer *renderer, Evas_3D_Scene_Data *data)
{
   Eina_List        *l;
   Evas_3D_Node     *n;
   const Evas_Mat4  *matrix_eye;
   Evas_3D_Node     *light;

   /* Set up render target. */
   e3d_renderer_target_set(renderer, drawable);
   e3d_renderer_clear(renderer, &data->bg_color);

   /* Get eye matrix. */
   matrix_eye = &data->camera_node->data.camera.matrix_world_to_eye;

   EINA_LIST_FOREACH(data->mesh_nodes, l, n)
     {
        Evas_Mat4          matrix_mv;
        Evas_Mat4          matrix_mvp;
        Eina_Iterator     *it;
        void              *ptr;

        evas_mat4_multiply(&matrix_mv, matrix_eye, &n->data.mesh.matrix_local_to_world);
        evas_mat4_multiply(&matrix_mvp, &data->camera_node->data.camera.camera->projection,
                           &matrix_mv);

        /* TODO: Get most effective light node. */
        light = eina_list_data_get(data->light_nodes);

        it = eina_hash_iterator_data_new(n->data.mesh.node_meshes);

        while (eina_iterator_next(it, &ptr))
          {
             Evas_3D_Node_Mesh *nm = (Evas_3D_Node_Mesh *)ptr;
             _mesh_draw(renderer, nm->mesh, nm->frame, light, matrix_eye, &matrix_mv, &matrix_mvp);
          }

        eina_iterator_free(it);
     }

   e3d_renderer_flush(renderer);
}
