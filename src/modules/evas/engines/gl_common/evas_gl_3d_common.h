#ifndef EVAS_GL_COMMON_H
# error You shall not include this header directly
#endif

typedef struct _E3D_Texture   E3D_Texture;
typedef struct _E3D_Drawable  E3D_Drawable;
typedef struct _E3D_Renderer  E3D_Renderer;

/* Texture */
E3D_Texture         *e3d_texture_new(void);
void                 e3d_texture_free(E3D_Texture *texture);

void                 e3d_texture_data_set(E3D_Texture *texture, Evas_3D_Color_Format format, Evas_3D_Pixel_Format pixel_format, int w, int h, const void *data);
void                 e3d_texture_file_set(E3D_Texture *texture, const char *file, const char *key);
Evas_3D_Color_Format e3d_texture_color_format_get(E3D_Texture *texture);
void                 e3d_texture_size_get(const E3D_Texture *texture, int *w, int *h);

void                 e3d_texture_import(E3D_Texture *texture, GLuint tex);
Eina_Bool            e3d_texture_is_imported_get(const E3D_Texture *texture);

void                 e3d_texture_wrap_set(E3D_Texture *texture, Evas_3D_Wrap_Mode s, Evas_3D_Wrap_Mode t);
void                 e3d_texture_wrap_get(const E3D_Texture *texture, Evas_3D_Wrap_Mode *s, Evas_3D_Wrap_Mode *t);

void                 e3d_texture_filter_set(E3D_Texture *texture, Evas_3D_Texture_Filter min, Evas_3D_Texture_Filter mag);
void                 e3d_texture_filter_get(const E3D_Texture *texture, Evas_3D_Texture_Filter *min, Evas_3D_Texture_Filter *mag);

/* Drawable */
E3D_Drawable        *e3d_drawable_new(int w, int h, int alpha, GLenum depth_format, GLenum stencil_format);
void                 e3d_drawable_free(E3D_Drawable *drawable);
void                 e3d_drawable_scene_render(E3D_Drawable *drawable, E3D_Renderer *renderer, Evas_3D_Scene_Data *data);
void                 e3d_drawable_size_get(E3D_Drawable *drawable, int *w, int *h);
GLuint               e3d_drawable_texture_id_get(E3D_Drawable *drawable);
GLenum               e3d_drawable_format_get(E3D_Drawable *drawable);

/* Renderer */
E3D_Renderer        *e3d_renderer_new(void);
void                 e3d_renderer_free(E3D_Renderer *renderer);
