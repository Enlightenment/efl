#ifndef ECTOR_GL_PRIVATE_H_
#define ECTOR_GL_PRIVATE_H_

#include "ector_private.h"
#include <Eina.h>

#define ECTOR_SHAD_VERTEX 0
#define ECTOR_SHAD_TEXUV  1

#define GL_STENCIL_HIGH_BIT  0x80

typedef struct _Ector_Renderer_Gl_Gradient_Data    Ector_Renderer_Gl_Gradient_Data;
typedef struct _Ector_Gl_Surface_Data              Ector_Gl_Surface_Data;
typedef struct _Ector_Renderer_Gl_Gradient_Data    Ector_Renderer_Gl_Gradient_Data;

typedef struct _Shader_Program                     Shader_Program;

struct _Shader_Program
{
  GLuint program;
  GLuint v_shader;
  GLuint f_shader;
  GLuint u_color;
  GLuint u_mvp;
  GLuint u_pos;
  GLuint u_texture;

  GLuint u_half_viewport_size;
  struct {
    GLuint u_linear_data;
  }linear;
  struct {
    GLuint u_fmp;
    GLuint u_bradius;
    GLuint u_fmp2_m_radius2;
    GLuint u_inverse_2_fmp2_m_radius2;
    GLuint u_sqrfr;
  }radial;
};


struct _Ector_Gl_Surface_Data
{
   struct {
     GLuint         fbo;
     int            w;
     int            h;
     int            rotation;
     Eina_Bool      flip;
     Eina_Bool      clear;
     Eina_Bool      stencil;
     Eina_Rectangle clip;
   }context;

   struct {
     Eina_Rectangle  geom;
     int             color;
   }vg;

   struct {
      int x, y;
   } ref_point;

   struct {
      int        w, h;
      GLuint     fbo;
      Eina_Bool  flip;
      int        rotation;
      Eina_Bool  clear;
   } dest_fbo;

   int vg_width;
   int vg_height;

   Eina_Rectangle clip;

   Eina_Bool  direct_draw;
};

// GRADIENT

#define GRADIENT_ARRAY_SIZE 512

typedef struct _Gl_Gradient_Linear_Data
{
   float x1, y1, x2, y2;
   float dx, dy, l;
} Gl_Gradient_Linear_Data;

typedef struct _Gl_Gradient_Radial_Data
{
   float cx, cy, fx, fy, cradius, fradius;
   float dx, dy, dr, sqrfr, a, inv2a;
   Eina_Bool extended;
} Gl_Gradient_Radial_Data;

// Gradient related structure
struct _Ector_Renderer_Gl_Gradient_Data
{
   Ector_Renderer_Gradient_Data *gd;
   union {
      Ector_Renderer_Gradient_Linear_Data *gld;
      Ector_Renderer_Gradient_Radial_Data *grd;
   };
   union {
      Gl_Gradient_Linear_Data linear;
      Gl_Gradient_Radial_Data radial;
   };
   Eina_Bool      alpha;
   unsigned int*  color_table;
   GLuint         texture;
};

void update_gradient_texture(Ector_Renderer_Gl_Gradient_Data *gdata);
void destroy_gradient_texture(Ector_Renderer_Gl_Gradient_Data *gdata);


// Ector Gl Engine
typedef struct _Ector_Gl_Engine_Data      Ector_Gl_Engine_Data;

typedef enum _Brush_Type {
  Solid,
  LinearGradient,
  RadialGradient,
} Brush_Type;

struct _Ector_Gl_Engine_Data
{
   struct {
     Shader_Program *simple;
     Shader_Program *tex;
     Shader_Program *lg;
     Shader_Program *rg;
   }shader;

   struct {
      int         w, h;
      struct {
        int       w, h;
        GLuint    id;
        GLuint    texture;
        GLuint    rbo_color;
        GLuint    rbo_stencil;
      }fbo;

      struct {
        Eina_Bool multisample;
        Eina_Bool ext_multisample;
        int       multisample_size;
        int       max_multisample;
      }capability;
   } surface;

   struct {
     Eina_Bool direct_draw; // draw directly to bounded fbo
     Eina_Bool stencil; // whether stencil buffer available in the bounded fbo
     Eina_Bool clear_surface; // whether to clear the bounded fbo
     Eina_Bool flip;
     int       rotation;
     int       color;
   }info;

   struct {
      unsigned int                      color;
      Ector_Renderer_Gl_Gradient_Data  *g;
      Brush_Type                        type;
   }brush;

   struct{
     Shader_Program *program;
     unsigned int    u_color;
     int             u_pos_x;
     int             u_pos_y;
   }state;

   struct {
     int x;
     int y;
   }offset;

   struct {
     Eina_Rectangle clip;
     struct {
       int x;
       int y;
     }offset;
   }master;

   float *mvp;

   int ref;
};


void ector_gl_engine_init();
void ector_gl_engine_shutdown();
void ector_gl_engine_info_set(Eina_Bool direct_draw, Eina_Bool stencil, Eina_Bool clear_surface, Eina_Bool flip, int rotation);
Eina_Bool ector_gl_engine_begin();
void ector_gl_engine_end();
void ector_gl_engine_surface_size_set(int width, int height);
void ector_gl_engine_master_clip_set(Eina_Rectangle *clip);
void ector_gl_engine_master_offset_set(int x, int y);
void ector_gl_engine_mul_color_set(int mul_color);
void ector_gl_engine_surface_copy(GLuint dst_surface, int dst_surface_w, int dst_surface_h,
                                  Eina_Rectangle *dst);

void ector_gl_engine_param_offset(int x, int y);
void ector_gl_engine_param_matrix(Eina_Matrix3 *m);
void ector_gl_engine_param_color(unsigned int color);

void ector_gl_engine_param_linear_gradient(Ector_Renderer_Gl_Gradient_Data* lg);
void ector_gl_engine_param_radial_gradient(Ector_Renderer_Gl_Gradient_Data* rg);

void ector_gl_engine_fill_set();

void ector_gl_engine_path_fill(float *vertex, unsigned int vertex_count,
                               int *stops, unsigned int stop_count);

void ector_gl_engine_path_stroke(float *vertex, int count);

#endif
