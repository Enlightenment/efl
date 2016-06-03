#ifndef EVAS_GL_COMMON_H
# error You shall not include this header directly
#endif

typedef struct _E3D_Texture   E3D_Texture;
typedef struct _E3D_Drawable  E3D_Drawable;
typedef struct _E3D_Renderer  E3D_Renderer;

/* Texture */
E3D_Texture         *e3d_texture_new(Eina_Bool use_atlas);
void                 e3d_texture_free(E3D_Texture *texture);

void                 e3d_texture_size_get(const E3D_Texture *texture, int *w, int *h);
void                 e3d_texture_set(Evas_Engine_GL_Context *gc, E3D_Texture *texture, Evas_GL_Image *im);
Evas_GL_Image       *e3d_texture_get(E3D_Texture *texture);

void                 e3d_texture_import(E3D_Texture *texture, GLuint tex);
Eina_Bool            e3d_texture_is_imported_get(const E3D_Texture *texture);

void                 e3d_texture_wrap_set(E3D_Texture *texture, Evas_Canvas3D_Wrap_Mode s, Evas_Canvas3D_Wrap_Mode t);
void                 e3d_texture_wrap_get(const E3D_Texture *texture, Evas_Canvas3D_Wrap_Mode *s, Evas_Canvas3D_Wrap_Mode *t);

void                 e3d_texture_filter_set(E3D_Texture *texture, Evas_Canvas3D_Texture_Filter min, Evas_Canvas3D_Texture_Filter mag);
void                 e3d_texture_filter_get(const E3D_Texture *texture, Evas_Canvas3D_Texture_Filter *min, Evas_Canvas3D_Texture_Filter *mag);

/* Drawable */
E3D_Drawable        *e3d_drawable_new(int w, int h, int alpha, GLenum depth_format, GLenum stencil_format);
void                 e3d_drawable_free(E3D_Drawable *drawable);
void                 e3d_drawable_scene_render(E3D_Drawable *drawable, E3D_Renderer *renderer, Evas_Canvas3D_Scene_Public_Data *data);
Eina_Bool            e3d_drawable_scene_render_to_texture(E3D_Drawable *drawable, E3D_Renderer *renderer, Evas_Canvas3D_Scene_Public_Data *data);
void                 e3d_drawable_size_get(E3D_Drawable *drawable, int *w, int *h);
GLuint               e3d_drawable_texture_id_get(E3D_Drawable *drawable);
GLuint               e3d_drawable_texture_color_pick_id_get(E3D_Drawable *drawable);
void                 e3d_drawable_texture_pixel_color_get(GLuint tex EINA_UNUSED, int x, int y, Evas_Color *color, void *drawable);
GLenum               e3d_drawable_format_get(E3D_Drawable *drawable);
void                 e3d_drawable_texture_rendered_pixels_get(GLuint tex EINA_UNUSED, int x, int y, int w, int h,
                                                              void *drawable EINA_UNUSED, void *data);
/* Renderer */
E3D_Renderer        *e3d_renderer_new(void);
void                 e3d_renderer_free(E3D_Renderer *renderer);
