/* ECTOR - EFL retained mode drawing library
 * Copyright (C) 2014 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>
#include "ector_private.h"

Ector_GL_API GL;
int _ector_log_dom_global = 0;

static int _ector_main_count = 0;

EAPI int
ector_init(void)
{
   if (EINA_LIKELY(_ector_main_count > 0))
     return ++_ector_main_count;

   eina_init();
   efl_object_init();

   _ector_log_dom_global = eina_log_domain_register("ector", ECTOR_DEFAULT_LOG_COLOR);
   if (_ector_log_dom_global < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ector");
        goto on_error;
     }

   _ector_main_count = 1;

   eina_log_timing(_ector_log_dom_global, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);
   GL.init = 0;

   return _ector_main_count;

 on_error:
   efl_object_shutdown();
   eina_shutdown();

   return 0;
}

static int
_gl_version()
{
   char *str;

   str = (char *)GL.glGetString(GL_VERSION);
   if (!str)
     {
        return 0;
     }

   if (strstr(str, "OpenGL ES 3"))
     {
        return 3;
     }

   if (strstr(str, "OpenGL ES 2"))
     {
        return 2;
     }

    if (strstr(str, "3.") || strstr(str, "4."))
     {
        return 3;
     }

    if (strstr(str, "2."))
     {
        return 2;
     }
   return 0;
}

static int
_gl_extension(const char * extension)
{
   char *extensions;
   extensions = (char *)GL.glGetString(GL_EXTENSIONS);
   if (strstr(extensions, extension))
      return 1;
   return 0;
}

static Eina_Bool
gl_func_exist(void *fp)
{
   if (!fp) return EINA_FALSE;

   return EINA_TRUE;
}

static void
_gl_finalize()
{
   if (GL.init) return;
   GL.init = EINA_TRUE;
   GL.version = _gl_version();

   if (GL.version == 2)
     {
        GL.glRenderbufferStorageMultisample = NULL;
        GL.glBlitFramebuffer = NULL;
        GL.glRenderbufferStorageMultisampleEXT = NULL;
        GL.glFramebufferTexture2DMultisampleEXT = NULL;
        if (_gl_extension("GL_EXT_multisampled_render_to_texture"))
          {
             if (GL.eglGetProcAddress)
               {
                  GL.glRenderbufferStorageMultisampleEXT = GL.eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
                  GL.glFramebufferTexture2DMultisampleEXT = GL.eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
               }
             if (GL.glXGetProcAddress)
               {
                  GL.glRenderbufferStorageMultisampleEXT = GL.glXGetProcAddress("glRenderbufferStorageMultisampleEXT");
                  GL.glFramebufferTexture2DMultisampleEXT = GL.glXGetProcAddress("glFramebufferTexture2DMultisampleEXT");
               }
             GL.ext_ms_render_to_tex = 1;
          }
     }
   if (GL.version == 3)
     {
        GL.glRenderbufferStorageMultisampleEXT = 0;
        GL.glFramebufferTexture2DMultisampleEXT = 0;
     }

}

EAPI Eina_Bool
ector_glsym_set(void *(*glsym)(void *lib, const char *name), void *lib)
{
   GL.init = 0;

   if (!glsym) return EINA_FALSE;

#define ORD(a) GL.a = glsym(lib, #a);

   ORD(glActiveTexture);
   ORD(glAttachShader);
   ORD(glBindAttribLocation);
   ORD(glBindBuffer);
   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);
   ORD(glBindTexture);
   ORD(glBlendColor);
   ORD(glBlendEquation);
   ORD(glBlendEquationSeparate);
   ORD(glBlendFunc);
   ORD(glBlendFuncSeparate);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glCheckFramebufferStatus);
   ORD(glClear);
   ORD(glClearColor);
   ORD(glClearDepthf);
   ORD(glClearStencil);
   ORD(glColorMask);
   ORD(glCompileShader);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCreateProgram);
   ORD(glCreateShader);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteFramebuffers);
   ORD(glDeleteProgram);
   ORD(glDeleteRenderbuffers);
   ORD(glDeleteShader);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
   ORD(glDepthRangef);
   ORD(glDetachShader);
   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableVertexAttribArray);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFramebufferRenderbuffer);
   ORD(glFramebufferTexture2D);
   ORD(glFrontFace);
   ORD(glGenBuffers);
   ORD(glGenerateMipmap);
   ORD(glGenFramebuffers);
   ORD(glGenRenderbuffers);
   ORD(glGenTextures);
   ORD(glGetActiveAttrib);
   ORD(glGetActiveUniform);
   ORD(glGetAttachedShaders);
   ORD(glGetAttribLocation);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetError);
   ORD(glGetFloatv);
   ORD(glGetFramebufferAttachmentParameteriv);
   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetProgramBinary);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
   ORD(glGetString);
   ORD(glGetTexParameterfv);
   ORD(glGetTexParameteriv);
   ORD(glGetUniformfv);
   ORD(glGetUniformiv);
   ORD(glGetUniformLocation);
   ORD(glGetVertexAttribfv);
   ORD(glGetVertexAttribiv);
   ORD(glGetVertexAttribPointerv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsFramebuffer);
   ORD(glIsProgram);
   ORD(glIsRenderbuffer);
   ORD(glIsShader);
   ORD(glIsTexture);
   ORD(glLineWidth);
   ORD(glLinkProgram);
   ORD(glPixelStorei);
   ORD(glPolygonOffset);
   ORD(glProgramBinary);
   ORD(glProgramParameteri);
   ORD(glReadPixels);
   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
   ORD(glScissor);
   ORD(glShaderBinary);
   ORD(glShaderSource);
   ORD(glStencilFunc);
   ORD(glStencilFuncSeparate);
   ORD(glStencilMask);
   ORD(glStencilMaskSeparate);
   ORD(glStencilOp);
   ORD(glStencilOpSeparate);
   ORD(glTexImage2D);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTexParameteri);
   ORD(glTexParameteriv);
   ORD(glTexSubImage2D);
   ORD(glUniform1f);
   ORD(glUniform1fv);
   ORD(glUniform1i);
   ORD(glUniform1iv);
   ORD(glUniform2f);
   ORD(glUniform2fv);
   ORD(glUniform2i);
   ORD(glUniform2iv);
   ORD(glUniform3f);
   ORD(glUniform3fv);
   ORD(glUniform3i);
   ORD(glUniform3iv);
   ORD(glUniform4f);
   ORD(glUniform4fv);
   ORD(glUniform4i);
   ORD(glUniform4iv);
   ORD(glUniformMatrix2fv);
   ORD(glUniformMatrix3fv);
   ORD(glUniformMatrix4fv);
   ORD(glUseProgram);
   ORD(glValidateProgram);
   ORD(glVertexAttrib1f);
   ORD(glVertexAttrib1fv);
   ORD(glVertexAttrib2f);
   ORD(glVertexAttrib2fv);
   ORD(glVertexAttrib3f);
   ORD(glVertexAttrib3fv);
   ORD(glVertexAttrib4f);
   ORD(glVertexAttrib4fv);
   ORD(glVertexAttribPointer);
   ORD(glViewport);
   ORD(eglGetProcAddress);
   ORD(glXGetProcAddress);

   ORD(glRenderbufferStorageMultisample);
   ORD(glBlitFramebuffer);
   GL.finalize = _gl_finalize;
   GL.glFuncExist = gl_func_exist;
   GL.ext_ms_render_to_tex = 0;

   return EINA_TRUE;
}

EAPI int
ector_shutdown(void)
{
   if (_ector_main_count <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown of ector.");
        return 0;
     }

   _ector_main_count--;
   if (EINA_LIKELY(_ector_main_count > 0))
     return _ector_main_count;

   GL.init = 0;
   eina_log_timing(_ector_log_dom_global,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   efl_object_shutdown();

   eina_log_domain_unregister(_ector_log_dom_global);

   eina_shutdown();
   return _ector_main_count;
}
