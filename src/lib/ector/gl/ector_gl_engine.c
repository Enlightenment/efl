#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector_Gl.h>
#include "ector_private.h"
#include "ector_gl_private.h"
#include "ector_gl_shaders.x"
#include "draw.h"

static void
_init_matrix(float *m)
{
   m[0] = 1.0f;
   m[1] = 0.0f;
   m[2] = 0.0f;
   m[3] = 0.0f;

   m[4] = 0.0f;
   m[5] = 1.0f;
   m[6] = 0.0f;
   m[7] = 0.0f;

   m[8] = 0.0f;
   m[9] = 0.0f;
   m[10] = 1.0f;
   m[11] = 0.0f;

   m[12] = 0.0f;
   m[13] = 0.0f;
   m[14] = 0.0f;
   m[15] = 1.0f;
}

static void
_matrix_ortho(GLfloat *m,
             GLfloat l, GLfloat r,
             GLfloat t, GLfloat b,
             GLfloat near_, GLfloat far_,
             int rot, int vw, int vh,
             int foc, GLfloat orth)
{
   GLfloat rotf;
   GLfloat cosv, sinv;
   GLfloat tx, ty;

   rotf = (((rot / 90) & 0x3) * M_PI) / 2.0;

   tx = -0.5 * (1.0 - orth);
   ty = -0.5 * (1.0 - orth);

   if (rot == 90)
     {
        tx += -(vw * 1.0);
        ty += -(vh * 0.0);
     }
   if (rot == 180)
     {
        tx += -(vw * 1.0);
        ty += -(vh * 1.0);
     }
   if (rot == 270)
     {
        tx += -(vw * 0.0);
        ty += -(vh * 1.0);
     }

   cosv = cos(rotf);
   sinv = sin(rotf);

   m[0] = (2.0 / (r - l)) * ( cosv);
   m[1] = (2.0 / (r - l)) * ( sinv);
   m[2] = 0.0;
   m[3] = 0.0;

   m[4] = (2.0 / (t - b)) * (-sinv);
   m[5] = (2.0 / (t - b)) * ( cosv);
   m[6] = 0.0;
   m[7] = 0.0;

   m[8] = 0.0;
   m[9] = 0.0;
   m[10] = -(2.0 / (far_ - near_));
   m[11] = 1.0 / (GLfloat)foc;

   m[12] = (m[0] * tx) + (m[4] * ty) - ((r + l) / (r - l));
   m[13] = (m[1] * tx) + (m[5] * ty) - ((t + b) / (t - b));
   m[14] = (m[2] * tx) + (m[6] * ty) - ((near_ + far_) / (far_ - near_));
   m[15] = (m[3] * tx) + (m[7] * ty) + orth;
}


static Ector_Gl_Engine_Data *ENG = NULL;

static void
_destroy_shader_program(Shader_Program *shader)
{
   if (shader)
     {
        if (shader->v_shader) GL.glDeleteShader(shader->v_shader);
        if (shader->f_shader)GL.glDeleteShader(shader->f_shader);
        if (shader->program) GL.glDeleteProgram(shader->program);
        free(shader);
     }
}
static Shader_Program *
_create_shader_program(const char *v_shader, const char* f_shader)
{
   Shader_Program *shader = calloc(1, sizeof(Shader_Program));
   GLint ok = 0;

   shader->v_shader = GL.glCreateShader(GL_VERTEX_SHADER);
   GL.glShaderSource(shader->v_shader, 1, &v_shader, NULL);
   GL.glCompileShader(shader->v_shader);
   GL.glGetShaderiv(shader->v_shader, GL_COMPILE_STATUS, &ok);
   if (!ok)
     {
        ERR("vertex shader compilation failed ");
        goto fail;
     }

   shader->f_shader = GL.glCreateShader(GL_FRAGMENT_SHADER);
   GL.glShaderSource(shader->f_shader, 1, &f_shader, NULL);
   GL.glCompileShader(shader->f_shader);
   GL.glGetShaderiv(shader->f_shader, GL_COMPILE_STATUS, &ok);
   if (!ok)
     {
        ERR("fragment shader compilation failed ");
        goto fail;
     }

   shader->program = GL.glCreateProgram();
   GL.glAttachShader(shader->program, shader->v_shader);
   GL.glAttachShader(shader->program, shader->f_shader);
   GL.glBindAttribLocation(shader->program, ECTOR_SHAD_VERTEX, "vertex");
   GL.glBindAttribLocation(shader->program, ECTOR_SHAD_TEXUV,  "v_tex_coord");
   GL.glLinkProgram(shader->program);
   GL.glGetProgramiv(shader->program, GL_LINK_STATUS, &ok);
   if (!ok)
     {
        ERR("linking failed for shader program");
        goto fail;
     }
   shader->u_color = GL.glGetUniformLocation(shader->program, "color");
   shader->u_pos = GL.glGetUniformLocation(shader->program, "pos");
   shader->u_mvp = GL.glGetUniformLocation(shader->program, "mvp");
   shader->u_texture = GL.glGetUniformLocation(shader->program, "texture");
   return shader;
fail:
   _destroy_shader_program(shader);
   return NULL;
}

static void
_update_lg_uniform()
{
  ENG->shader.lg->u_half_viewport_size = GL.glGetUniformLocation(ENG->shader.lg->program, "half_viewport_size");
  ENG->shader.lg->linear.u_linear_data = GL.glGetUniformLocation(ENG->shader.lg->program, "linear_data");
}

static void
_update_rg_uniform()
{
  ENG->shader.rg->u_half_viewport_size = GL.glGetUniformLocation(ENG->shader.rg->program, "half_viewport_size");
  ENG->shader.rg->radial.u_fmp = GL.glGetUniformLocation(ENG->shader.rg->program, "fmp");
  ENG->shader.rg->radial.u_bradius = GL.glGetUniformLocation(ENG->shader.rg->program, "bradius");
  ENG->shader.rg->radial.u_fmp2_m_radius2 = GL.glGetUniformLocation(ENG->shader.rg->program, "fmp2_m_radius2");
  ENG->shader.rg->radial.u_inverse_2_fmp2_m_radius2 = GL.glGetUniformLocation(ENG->shader.rg->program, "inverse_2_fmp2_m_radius2");
  ENG->shader.rg->radial.u_sqrfr = GL.glGetUniformLocation(ENG->shader.rg->program, "sqrfr");
}

void
update_gradient_texture(Ector_Renderer_Gl_Gradient_Data *gdata)
{
   if (gdata->color_table) return;

   GLenum wrapMode = GL_CLAMP_TO_EDGE;

   gdata->color_table = malloc(GRADIENT_ARRAY_SIZE * 4);
   gdata->alpha = efl_draw_generate_gradient_color_table(gdata->gd->colors, gdata->gd->colors_count, gdata->color_table, GRADIENT_ARRAY_SIZE);

   if (gdata->gd->s == EFL_GFX_GRADIENT_SPREAD_REPEAT)
     wrapMode = GL_REPEAT;
   else if (gdata->gd->s == EFL_GFX_GRADIENT_SPREAD_REFLECT)
     wrapMode = GL_MIRRORED_REPEAT;

   GL.glGenTextures(1, &(gdata->texture));
   GL.glBindTexture(GL_TEXTURE_2D, gdata->texture);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
   GL.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRADIENT_ARRAY_SIZE, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, gdata->color_table);

   GL.glBindTexture(GL_TEXTURE_2D, 0);
}

void
destroy_gradient_texture(Ector_Renderer_Gl_Gradient_Data *gdata)
{
   if (gdata->color_table)
     {
        free(gdata->color_table);
        gdata->color_table = NULL;
        GL.glDeleteTextures(1, &(gdata->texture));
        gdata->texture = 0;
     }
}

static void
_ector_gl_engine_destroy_surface()
{
   if (!ENG->surface.fbo.id) return;

   if (ENG->surface.fbo.rbo_color)
     GL.glDeleteRenderbuffers(1, &(ENG->surface.fbo.rbo_color));
   if (ENG->surface.fbo.rbo_stencil)
     GL.glDeleteRenderbuffers(1, &(ENG->surface.fbo.rbo_stencil));
   if (ENG->surface.fbo.texture)
     GL.glDeleteTextures(1, &(ENG->surface.fbo.texture));
   if (ENG->surface.fbo.id)
     GL.glDeleteFramebuffers(1, &(ENG->surface.fbo.id));

   ENG->surface.fbo.id = 0;
   ENG->surface.fbo.rbo_color = 0;
   ENG->surface.fbo.rbo_stencil = 0;
   ENG->surface.fbo.texture = 0;
}

static Eina_Bool
_ector_gl_engine_create_fbo_rbo_surface()
{
   GL.glGenRenderbuffers(1, &ENG->surface.fbo.rbo_color);
   GL.glBindRenderbuffer(GL_RENDERBUFFER, ENG->surface.fbo.rbo_color);
   GL.glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                       ENG->surface.capability.multisample_size,
                                       GL_RGBA,
                                       ENG->surface.fbo.w, ENG->surface.fbo.h);

   // create a 4x MSAA renderbuffer object for depthbuffer
   GL.glGenRenderbuffers(1, &ENG->surface.fbo.rbo_stencil);
   GL.glBindRenderbuffer(GL_RENDERBUFFER, ENG->surface.fbo.rbo_stencil);
   GL.glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                       ENG->surface.capability.multisample_size,
                                       GL_STENCIL_INDEX8,
                                       ENG->surface.fbo.w, ENG->surface.fbo.h);


   GL.glGenFramebuffers(1, &ENG->surface.fbo.id);
   GL.glBindFramebuffer(GL_FRAMEBUFFER, ENG->surface.fbo.id);

   // attach colorbuffer image to FBO
   GL.glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                GL_COLOR_ATTACHMENT0,
                                GL_RENDERBUFFER,
                                ENG->surface.fbo.rbo_color);

   // attach stencil buffer to FBO
   GL.glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                GL_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER,
                                ENG->surface.fbo.rbo_stencil);

   // check FBO status
   GLenum status = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if(status != GL_FRAMEBUFFER_COMPLETE)
     {
        ERR("cretaing MSAA fbo rbo failed");
        GL.glDeleteRenderbuffers(1, &(ENG->surface.fbo.rbo_color));
        GL.glDeleteRenderbuffers(1, &(ENG->surface.fbo.rbo_stencil));
        GL.glDeleteFramebuffers(1, &(ENG->surface.fbo.id));
        ENG->surface.fbo.id = 0;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_gl_engine_create_fbo_text_ext_surface()
{
   GL.glGenTextures(1, &ENG->surface.fbo.texture);
   GL.glBindTexture(GL_TEXTURE_2D, ENG->surface.fbo.texture);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GL.glTexImage2D(GL_TEXTURE_2D, 0,
                   GL_RGBA, ENG->surface.fbo.w, ENG->surface.fbo.h, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, NULL);
   GL.glBindTexture(GL_TEXTURE_2D, 0);


   // create a MSAA renderbuffer object for stencil buffer
   GL.glGenRenderbuffers(1, &ENG->surface.fbo.rbo_stencil);
   GL.glBindRenderbuffer(GL_RENDERBUFFER, ENG->surface.fbo.rbo_stencil);
   GL.glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
                                          ENG->surface.capability.multisample_size,
                                          GL_STENCIL_INDEX8,
                                          ENG->surface.fbo.w, ENG->surface.fbo.h);


   GL.glGenFramebuffers(1, &ENG->surface.fbo.id);
   GL.glBindFramebuffer(GL_FRAMEBUFFER, ENG->surface.fbo.id);

   // attach texture to FBO
   GL.glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER,
                                           GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_2D,
                                           ENG->surface.fbo.texture,
                                           0,
                                           ENG->surface.capability.multisample_size);

   // attach stencil buffer to FBO
   GL.glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                GL_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER,
                                ENG->surface.fbo.rbo_stencil);

   // check FBO status
   GLenum status = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if(status != GL_FRAMEBUFFER_COMPLETE)
     {
        ERR("cretaing MSAA fbo texture failed");
        GL.glDeleteTextures(1, &(ENG->surface.fbo.texture));
        GL.glDeleteRenderbuffers(1, &(ENG->surface.fbo.rbo_stencil));
        GL.glDeleteFramebuffers(1, &(ENG->surface.fbo.id));
        ENG->surface.fbo.id = 0;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_gl_engine_create_fbo_text_surface()
{
   GL.glGenTextures(1, &ENG->surface.fbo.texture);
   GL.glBindTexture(GL_TEXTURE_2D, ENG->surface.fbo.texture);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GL.glTexImage2D(GL_TEXTURE_2D, 0,
                   GL_RGBA, ENG->surface.fbo.w, ENG->surface.fbo.h, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, NULL);
   GL.glBindTexture(GL_TEXTURE_2D, 0);


   GL.glGenRenderbuffers(1, &ENG->surface.fbo.rbo_stencil);
   GL.glBindRenderbuffer(GL_RENDERBUFFER, ENG->surface.fbo.rbo_stencil);
   GL.glRenderbufferStorage(GL_RENDERBUFFER,
                            GL_STENCIL_INDEX8,
                            ENG->surface.fbo.w, ENG->surface.fbo.h);


   GL.glGenFramebuffers(1, &ENG->surface.fbo.id);
   GL.glBindFramebuffer(GL_FRAMEBUFFER, ENG->surface.fbo.id);

   // attach texture to FBO
   GL.glFramebufferTexture2D(GL_FRAMEBUFFER,
                             GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D,
                             ENG->surface.fbo.texture,
                             0);

   // attach stencil buffer to FBO
   GL.glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                GL_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER,
                                ENG->surface.fbo.rbo_stencil);

   // check FBO status
   GLenum status = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if(status != GL_FRAMEBUFFER_COMPLETE)
     {
        ERR("cretaing normal fbo texture failed");
        GL.glDeleteTextures(1, &(ENG->surface.fbo.texture));
        GL.glDeleteRenderbuffers(1, &(ENG->surface.fbo.rbo_stencil));
        GL.glDeleteFramebuffers(1, &(ENG->surface.fbo.id));
        ENG->surface.fbo.id = 0;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
_ector_gl_engine_update_surface()
{
   if (ENG->info.direct_draw)
     return EINA_TRUE;

   if (ENG->surface.fbo.w >= ENG->surface.w && ENG->surface.fbo.h >= ENG->surface.h)
     {
        // no need to update the scratch buffer
        GL.glBindFramebuffer(GL_FRAMEBUFFER, ENG->surface.fbo.id);
        return EINA_TRUE;
     }

   // delete old buffer if any
   _ector_gl_engine_destroy_surface();

   // update the new size
   ENG->surface.fbo.w = ENG->surface.w > ENG->surface.fbo.w ? ENG->surface.w : ENG->surface.fbo.w;
   ENG->surface.fbo.h = ENG->surface.h > ENG->surface.fbo.h ? ENG->surface.h : ENG->surface.fbo.h;

   if (ENG->surface.capability.multisample)
     {
        return _ector_gl_engine_create_fbo_rbo_surface();
     }
   else if (ENG->surface.capability.ext_multisample)
     {
        return _ector_gl_engine_create_fbo_text_ext_surface();
     }
   else
     {
       return _ector_gl_engine_create_fbo_text_surface();
     }
}

static void
_update_viewport(int width, int height)
{
   if ((ENG->info.rotation == 0) || (ENG->info.rotation == 180))
   {
     GL.glViewport(0, 0, width, height);
   }
   else
   {
     GL.glViewport(0, 0, height, width);
   }

   _init_matrix(ENG->mvp);

   if (ENG->info.flip)
   {
     _matrix_ortho(ENG->mvp,
                   0, width,
                   height, 0,
                   -1000000.0, 1000000.0, ENG->info.rotation,
                   width, height,
                   1, 1.0);
   }
   else
   {
     _matrix_ortho(ENG->mvp,
                   0, width,
                   0, height,
                   -1000000.0, 1000000.0, ENG->info.rotation,
                   width, height,
                   1, 1.0);
   }
}

Eina_Bool
ector_gl_engine_begin()
{
   if (!_ector_gl_engine_update_surface()) return EINA_FALSE;

   _update_viewport(ENG->surface.w, ENG->surface.h);

   GL.glEnable(GL_SCISSOR_TEST);
   GL.glScissor(ENG->master.clip.x, ENG->master.clip.y, ENG->master.clip.w, ENG->master.clip.h);
   GL.glClearStencil(0);
   GL.glClearColor(0, 0, 0, 0);
   if (!(ENG->info.direct_draw) || ENG->info.clear_surface)
   {
     GL.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }
   GL.glDisable(GL_DEPTH_TEST);
   GL.glEnable(GL_BLEND);
   GL.glDisable(GL_STENCIL_TEST);
   GL.glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

   return EINA_TRUE;
}

void
ector_gl_engine_end()
{
   GL.glDisable(GL_DEPTH_TEST);
   GL.glDisable(GL_STENCIL_TEST);
   GL.glDisable(GL_BLEND);
   GL.glDisable(GL_SCISSOR_TEST);
   GL.glScissor(0, 0, 0, 0);

   ENG->state.program = NULL;
   ENG->state.u_color = 0;
   ENG->state.u_pos_x = 0;
   ENG->state.u_pos_y = 0;
}

static inline void
_update_color_uniform(GLuint pos, unsigned int color)
{
   GL.glUniform4f(pos, (R_VAL(&color)/255.0), (G_VAL(&color)/255.0), (B_VAL(&color)/255.0), (A_VAL(&color)/255.0));
}

static void
_load_simple_program()
{
   //Already the program is used
   if (ENG->state.program == ENG->shader.simple)
     {
        GL.glUniform4f(ENG->shader.simple->u_pos, ENG->master.offset.x + ENG->offset.x, ENG->master.offset.y + ENG->offset.y, 0, 0);
        _update_color_uniform(ENG->shader.simple->u_color, DRAW_MUL4_SYM(ENG->brush.color, ENG->info.color));
     }
   else
     {
        GL.glUseProgram(ENG->shader.simple->program);
        GL.glEnableVertexAttribArray(ECTOR_SHAD_VERTEX);
        GL.glUniformMatrix4fv(ENG->shader.simple->u_mvp, 1, GL_FALSE, ENG->mvp);
        GL.glUniform4f(ENG->shader.simple->u_pos, ENG->master.offset.x + ENG->offset.x, ENG->master.offset.y + ENG->offset.y, 0, 0);
        _update_color_uniform(ENG->shader.simple->u_color, DRAW_MUL4_SYM(ENG->brush.color, ENG->info.color));

        // keep the state
        ENG->state.program = ENG->shader.simple;
        ENG->state.u_pos_x = ENG->master.offset.x + ENG->offset.x;
        ENG->state.u_pos_y = ENG->master.offset.y + ENG->offset.y;
        ENG->state.u_color = ENG->brush.color;
     }
}

static void
_load_lg_program()
{
   //Already the program is used
   GL.glUseProgram(ENG->shader.lg->program);
   GL.glEnableVertexAttribArray(ECTOR_SHAD_VERTEX);
   GL.glUniformMatrix4fv(ENG->shader.lg->u_mvp, 1, GL_FALSE, ENG->mvp);
   GL.glUniform4f(ENG->shader.lg->u_pos, ENG->master.offset.x + ENG->offset.x, ENG->master.offset.y + ENG->offset.y, 0, 0);
   GL.glUniform2f(ENG->shader.lg->u_half_viewport_size, ENG->surface.w * 0.5, ENG->surface.h * 0.5);
   GL.glUniform3f(ENG->shader.lg->linear.u_linear_data, ENG->brush.g->linear.dx, ENG->brush.g->linear.dy, ENG->brush.g->linear.l);

   GL.glActiveTexture(GL_TEXTURE0);
   GL.glBindTexture(GL_TEXTURE_2D, ENG->brush.g->texture);
   GL.glUniform1i(ENG->shader.lg->u_texture, 0);

   // keep the state
   ENG->state.program = ENG->shader.lg;
   ENG->state.u_pos_x = ENG->master.offset.x + ENG->offset.x;
   ENG->state.u_pos_y = ENG->master.offset.y + ENG->offset.y;
   ENG->state.u_color = 0;
}

static void
_load_rg_program()
{
   //Already the program is used
   GL.glUseProgram(ENG->shader.rg->program);
   GL.glEnableVertexAttribArray(ECTOR_SHAD_VERTEX);
   GL.glUniformMatrix4fv(ENG->shader.rg->u_mvp, 1, GL_FALSE, ENG->mvp);
   GL.glUniform4f(ENG->shader.rg->u_pos, ENG->master.offset.x + ENG->offset.x, ENG->master.offset.y + ENG->offset.y, 0, 0);
   GL.glUniform2f(ENG->shader.rg->u_half_viewport_size, ENG->surface.w * 0.5, ENG->surface.h * 0.5);

   GL.glUniform2f(ENG->shader.rg->radial.u_fmp, ENG->brush.g->radial.dx, ENG->brush.g->radial.dy);
   GL.glUniform1f(ENG->shader.rg->radial.u_fmp2_m_radius2, ENG->brush.g->radial.a);
   GL.glUniform1f(ENG->shader.rg->radial.u_inverse_2_fmp2_m_radius2, ENG->brush.g->radial.inv2a);
   GL.glUniform1f(ENG->shader.rg->radial.u_sqrfr, ENG->brush.g->radial.fradius * ENG->brush.g->radial.fradius);

   GL.glUniform3f(ENG->shader.rg->radial.u_bradius,
                  2 * ENG->brush.g->radial.dr * ENG->brush.g->radial.fradius,
                  ENG->brush.g->radial.fradius,
                  ENG->brush.g->radial.dr);


   GL.glActiveTexture(GL_TEXTURE0);
   GL.glBindTexture(GL_TEXTURE_2D, ENG->brush.g->texture);
   GL.glUniform1i(ENG->shader.rg->u_texture, 0);

   // keep the state
   ENG->state.program = ENG->shader.rg;
   ENG->state.u_pos_x = ENG->master.offset.x + ENG->offset.x;
   ENG->state.u_pos_y = ENG->master.offset.y + ENG->offset.y;
   ENG->state.u_color = 0;
}

static void
_update_brush()
{
   switch(ENG->brush.type)
     {
        case Solid:
           {
             _load_simple_program();
             break;
           }
        case LinearGradient:
           {
             _load_lg_program();
             break;
           }
        case RadialGradient:
           {
             _load_rg_program();
             break;
           }
        default:
          break;
     }
}

static Eina_Bool
_is_stencil_available()
{
   if (ENG->info.direct_draw && !(ENG->info.stencil))
     return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_is_solid_brush()
{
   if ((ENG->brush.type == Solid) && (A_VAL(&(ENG->brush.color)) == 255))
     return EINA_TRUE;
   else if ((ENG->brush.g) && (!ENG->brush.g->alpha))
     return EINA_TRUE;

   return EINA_FALSE;
}

void
ector_gl_engine_path_stroke(float *vertex, int count)
{
   if (!(_is_stencil_available()) || _is_solid_brush())
     {
        _load_simple_program();
        GL.glVertexAttribPointer(ECTOR_SHAD_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, vertex);
        GL.glDrawArrays(GL_TRIANGLE_STRIP, 0, count/2);
        return;
     }
   else
     {
        _load_simple_program();
        GL.glVertexAttribPointer(ECTOR_SHAD_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, vertex);

        GL.glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable color writes

        GL.glEnable(GL_STENCIL_TEST);
        GL.glStencilMask(0xff);
        GL.glClear(GL_STENCIL_BUFFER_BIT);
        GL.glStencilMask(GL_STENCIL_HIGH_BIT);

        GL.glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        GL.glStencilFunc(GL_ALWAYS, GL_STENCIL_HIGH_BIT, 0xff);

        GL.glDrawArrays(GL_TRIANGLE_STRIP, 0, count/2); // here the shape is drawn in the stencil

        // Enable color writes & disable stencil writes
        GL.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        GL.glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
        // Pass when high bit is set, replace stencil value with 0
        GL.glStencilFunc(GL_NOTEQUAL, 0, GL_STENCIL_HIGH_BIT);

         // draw a rectangle
         _update_brush();
        float rect_bound[10] = {0, 0 ,
                                ENG->surface.w, 0,
                                ENG->surface.w, ENG->surface.h,
                                0, ENG->surface.h};
         GL.glVertexAttribPointer(ECTOR_SHAD_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, rect_bound);
         GL.glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // reset
        GL.glStencilFunc(GL_ALWAYS, 0, 0xff);
        GL.glDisable(GL_STENCIL_TEST);
     }
}


void
ector_gl_engine_path_fill(float *vertex,
                            unsigned int vertex_count EINA_UNUSED,
                            int *stops,
                            unsigned int stop_count)
{
   unsigned int i, previous_stop = 0, stop = 0;

   if (!_is_stencil_available())
     {
        _update_brush();
        GL.glVertexAttribPointer(ECTOR_SHAD_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, vertex);
        for (i = 0 ; i < stop_count; i++)
          {
             stop = stops[i];
             GL.glDrawArrays(GL_TRIANGLE_FAN, previous_stop/2, (stop - previous_stop)/2);
             previous_stop = stop;
          }
        return;
     }

   _load_simple_program();

   GL.glVertexAttribPointer(ECTOR_SHAD_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, vertex);

   // 1. Disable color writes
   GL.glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

   // 2. Enable Stencil and draw the shape in stencil
   GL.glEnable(GL_STENCIL_TEST);
   GL.glStencilMask(0xff);
   GL.glClear(GL_STENCIL_BUFFER_BIT);
   GL.glStencilMask(GL_STENCIL_HIGH_BIT);

   // 3. set the Op for odd/even fill
   GL.glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // Simply invert the stencil bit

   // 4. draw the shape
   for (i = 0 ; i < stop_count; i++)
     {
        stop = stops[i];
        GL.glDrawArrays(GL_TRIANGLE_FAN, previous_stop/2, (stop - previous_stop)/2);
        previous_stop = stop;
      }

   // 5. Enable color writes
   GL.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

   // 6. setup stencil op
   GL.glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
   GL.glStencilFunc(GL_NOTEQUAL, 0, GL_STENCIL_HIGH_BIT);

   // 7.draw a bounding rectangle covering the shape
   float rect_bound[10] = {0, 0 ,
                           ENG->surface.w, 0,
                           ENG->surface.w, ENG->surface.h,
                           0, ENG->surface.h};
   GL.glVertexAttribPointer(ECTOR_SHAD_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, rect_bound);

   _update_brush();

   GL.glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

   // reset
   GL.glStencilFunc(GL_ALWAYS, 0, 0xff);
   GL.glDisable(GL_STENCIL_TEST);
}


void
ector_gl_engine_surface_copy(GLuint dst_surface, int dst_surface_w, int dst_surface_h,
                             Eina_Rectangle *dest)
{
   if (ENG->info.direct_draw) return;

   GL.glDisable(GL_SCISSOR_TEST);

   if (ENG->surface.capability.multisample)
     {
        GL.glBindFramebuffer(GL_READ_FRAMEBUFFER, ENG->surface.fbo.id);
        GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_surface);
        GL.glBlitFramebuffer(ENG->master.clip.x, ENG->master.clip.y, ENG->master.clip.w, ENG->master.clip.h,
                             dest->x, dest->y, dest->w, dest->h,
                             GL_COLOR_BUFFER_BIT,
                             GL_LINEAR);
        return;
     }

   // copy texture by drawing a quad
   GL.glBindFramebuffer(GL_FRAMEBUFFER, dst_surface);

   _update_viewport(dst_surface_w, dst_surface_h);

   if (ENG->info.clear_surface)
     GL.glClear(GL_COLOR_BUFFER_BIT);

   GL.glUseProgram(ENG->shader.tex->program);
   GL.glUniformMatrix4fv(ENG->shader.tex->u_mvp, 1, GL_FALSE, ENG->mvp);
   GL.glUniform4f(ENG->shader.tex->u_pos, 0, 0, 0, 0);
   _update_color_uniform(ENG->shader.tex->u_color, ENG->info.color);

   GL.glEnableVertexAttribArray(ECTOR_SHAD_VERTEX);
   GL.glEnableVertexAttribArray(ECTOR_SHAD_TEXUV);

   GL.glActiveTexture(GL_TEXTURE0);
   GL.glBindTexture(GL_TEXTURE_2D, ENG->surface.fbo.texture);
   GL.glUniform1i(ENG->shader.tex->u_texture, 0);

   float rect_bound[10] = {dest->x, dest->y ,
                           dest->x + dest->w, dest->y,
                           dest->x + dest->w, dest->y + dest->h,
                           dest->x, dest->y + dest->h};
   float uv_w = (float)(ENG->master.clip.w)/ ENG->surface.fbo.w;
   float uv_h = (float)(ENG->master.clip.h)/ ENG->surface.fbo.h;
   float uv_x = (float)(ENG->master.clip.x)/ ENG->surface.fbo.w;
   float uv_y = (float)(ENG->master.clip.y)/ ENG->surface.fbo.h;
   float tex_uv[10] = {uv_x, uv_y ,
                       uv_x + uv_w, uv_y,
                       uv_x + uv_w, uv_y + uv_h,
                       uv_x, uv_y + uv_h};

   GL.glVertexAttribPointer(ECTOR_SHAD_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, rect_bound);
   GL.glVertexAttribPointer(ECTOR_SHAD_TEXUV, 2, GL_FLOAT, GL_FALSE, 0, tex_uv);

   GL.glEnable(GL_BLEND);

   GL.glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void
ector_gl_engine_param_color(unsigned int color)
{
   ENG->brush.color = color;
   ENG->brush.g = NULL;
   ENG->brush.type = Solid;
}

void
ector_gl_engine_param_linear_gradient(Ector_Renderer_Gl_Gradient_Data* lg)
{
   ENG->brush.g = lg;
   ENG->brush.type = LinearGradient;
}

void
ector_gl_engine_param_radial_gradient(Ector_Renderer_Gl_Gradient_Data* rg)
{
   ENG->brush.g = rg;
   ENG->brush.type = RadialGradient;
}

void
ector_gl_engine_param_offset(int x, int y)
{
   ENG->offset.x = x;
   ENG->offset.y = y;
}

void
ector_gl_engine_surface_size_set(int width, int height)
{
   ENG->surface.w = width;
   ENG->surface.h = height;
}

void
ector_gl_engine_info_set(Eina_Bool direct_draw, Eina_Bool stencil, Eina_Bool clear_surface, Eina_Bool flip, int rotation)
{
   ENG->info.direct_draw = direct_draw;
   ENG->info.stencil = stencil;
   ENG->info.clear_surface = clear_surface;
   ENG->info.flip = flip;
   ENG->info.rotation = rotation;
}

void
ector_gl_engine_master_clip_set(Eina_Rectangle *clip)
{
   ENG->master.clip.x = clip->x;
   ENG->master.clip.y = clip->y;
   ENG->master.clip.w = clip->w;
   ENG->master.clip.h = clip->h;
}

void
ector_gl_engine_master_offset_set(int x, int y)
{
   ENG->master.offset.x = x;
   ENG->master.offset.y = y;
}

void
ector_gl_engine_mul_color_set(int mul_color)
{
   ENG->info.color = mul_color;
}


void
ector_gl_engine_init()
{
   const char *ector_msaa = NULL;

   if (ENG)
     {
        ENG->ref++;
        return;
     }

   GL.finalize();

   ENG = calloc(1,sizeof(Ector_Gl_Engine_Data));
   ENG->ref = 1;
   GL.finalize();

   ENG->shader.simple = _create_shader_program(vertex_shader, fragment_shader);
   ENG->shader.tex = _create_shader_program(tex_vertex_shader, tex_fragment_shader);
   ENG->shader.lg = _create_shader_program(linear_gradient_vertex_shader, linear_gradient_fragment_shader);
   ENG->shader.rg = _create_shader_program(radial_gradient_vertex_shader, radial_gradient_fragment_shader);
   _update_lg_uniform();
   _update_rg_uniform();

   ENG->mvp = malloc(16 * sizeof(float));

   if (GL.version == 3) ENG->surface.capability.multisample = EINA_TRUE;

   if (GL.ext_ms_render_to_tex) ENG->surface.capability.ext_multisample = EINA_TRUE;

   ector_msaa = getenv("ECTOR_MSAA");

   if (ector_msaa)
     {
        //@TODO parse the msaa level;
        ENG->surface.capability.multisample_size = 4;
     }
   else
     {
        ENG->surface.capability.multisample_size = 4;
     }
}

void
ector_gl_engine_shutdown()
{
   if (ENG)
     {
        ENG->ref--;
        if (ENG->ref) return;

        _destroy_shader_program(ENG->shader.simple);
        _destroy_shader_program(ENG->shader.tex);
        _destroy_shader_program(ENG->shader.lg);
        _destroy_shader_program(ENG->shader.rg);
        _ector_gl_engine_destroy_surface();
        free(ENG->mvp);
        free(ENG);
        ENG = NULL;
     }

}
