/*
 * This is an automatically generated file using a python script.
 * ($EFL_HOME/src/utils/evas/generate_gl_thread_api.py)
 * Recommend that you modify data files ($EFL_HOME/src/utils/evas/gl_api_def.txt)
 * and make use of scripts if you need to fix them.
 */

/*
 * GLenum
 * glGetError(void);
 */

typedef struct
{
   GLenum return_value;

} Evas_Thread_Command_glGetError;

static void
_gl_thread_glGetError(void *data)
{
   Evas_Thread_Command_glGetError *thread_data =
      (Evas_Thread_Command_glGetError *)data;

   thread_data->return_value = glGetError();

}

EAPI GLenum
evas_glGetError_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetError thread_data_local;
   Evas_Thread_Command_glGetError *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glGetError();
     }


   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetError,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
 */

typedef struct
{
   GLuint index;
   GLint size;
   GLenum type;
   GLboolean normalized;
   GLsizei stride;
   const void *pointer;

} Evas_Thread_Command_glVertexAttribPointer;

static void
_gl_thread_glVertexAttribPointer(void *data)
{
   Evas_Thread_Command_glVertexAttribPointer *thread_data =
      (Evas_Thread_Command_glVertexAttribPointer *)data;

   glVertexAttribPointer(thread_data->index,
                         thread_data->size,
                         thread_data->type,
                         thread_data->normalized,
                         thread_data->stride,
                         thread_data->pointer);

}

EAPI void
evas_glVertexAttribPointer_th(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glVertexAttribPointer thread_data_local;
   Evas_Thread_Command_glVertexAttribPointer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        return;
     }

   thread_data->index = index;
   thread_data->size = size;
   thread_data->type = type;
   thread_data->normalized = normalized;
   thread_data->stride = stride;
   thread_data->pointer = pointer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glVertexAttribPointer,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glEnableVertexAttribArray(GLuint index);
 */

typedef struct
{
   GLuint index;
   int command_allocated;

} Evas_Thread_Command_glEnableVertexAttribArray;

static void
_gl_thread_glEnableVertexAttribArray(void *data)
{
   Evas_Thread_Command_glEnableVertexAttribArray *thread_data =
      (Evas_Thread_Command_glEnableVertexAttribArray *)data;

   glEnableVertexAttribArray(thread_data->index);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glEnableVertexAttribArray_th(GLuint index)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glEnableVertexAttribArray thread_data_local;
   Evas_Thread_Command_glEnableVertexAttribArray *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glEnableVertexAttribArray(index);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glEnableVertexAttribArray *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glEnableVertexAttribArray));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_ENQUEUE;
          }
     }

   thread_data->index = index;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glEnableVertexAttribArray,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDisableVertexAttribArray(GLuint index);
 */

typedef struct
{
   GLuint index;
   int command_allocated;

} Evas_Thread_Command_glDisableVertexAttribArray;

static void
_gl_thread_glDisableVertexAttribArray(void *data)
{
   Evas_Thread_Command_glDisableVertexAttribArray *thread_data =
      (Evas_Thread_Command_glDisableVertexAttribArray *)data;

   glDisableVertexAttribArray(thread_data->index);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDisableVertexAttribArray_th(GLuint index)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDisableVertexAttribArray thread_data_local;
   Evas_Thread_Command_glDisableVertexAttribArray *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDisableVertexAttribArray(index);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDisableVertexAttribArray *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDisableVertexAttribArray));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_ENQUEUE;
          }
     }

   thread_data->index = index;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDisableVertexAttribArray,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDrawArrays(GLenum mode, GLint first, GLsizei count);
 */

typedef struct
{
   GLenum mode;
   GLint first;
   GLsizei count;

} Evas_Thread_Command_glDrawArrays;

static void
_gl_thread_glDrawArrays(void *data)
{
   Evas_Thread_Command_glDrawArrays *thread_data =
      (Evas_Thread_Command_glDrawArrays *)data;

   glDrawArrays(thread_data->mode,
                thread_data->first,
                thread_data->count);

}

EAPI void
evas_glDrawArrays_th(GLenum mode, GLint first, GLsizei count)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDrawArrays thread_data_local;
   Evas_Thread_Command_glDrawArrays *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDrawArrays(mode, first, count);
        return;
     }

   thread_data->mode = mode;
   thread_data->first = first;
   thread_data->count = count;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDrawArrays,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
 */

typedef struct
{
   GLenum mode;
   GLsizei count;
   GLenum type;
   const void *indices;

} Evas_Thread_Command_glDrawElements;

static void
_gl_thread_glDrawElements(void *data)
{
   Evas_Thread_Command_glDrawElements *thread_data =
      (Evas_Thread_Command_glDrawElements *)data;

   glDrawElements(thread_data->mode,
                  thread_data->count,
                  thread_data->type,
                  thread_data->indices);

}

EAPI void
evas_glDrawElements_th(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDrawElements thread_data_local;
   Evas_Thread_Command_glDrawElements *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDrawElements(mode, count, type, indices);
        return;
     }

   thread_data->mode = mode;
   thread_data->count = count;
   thread_data->type = type;
   thread_data->indices = indices;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDrawElements,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGenBuffers(GLsizei n, GLuint *buffers);
 */

typedef struct
{
   GLsizei n;
   GLuint *buffers;

} Evas_Thread_Command_glGenBuffers;

static void
_gl_thread_glGenBuffers(void *data)
{
   Evas_Thread_Command_glGenBuffers *thread_data =
      (Evas_Thread_Command_glGenBuffers *)data;

   glGenBuffers(thread_data->n,
                thread_data->buffers);

}

EAPI void
evas_glGenBuffers_th(GLsizei n, GLuint *buffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGenBuffers thread_data_local;
   Evas_Thread_Command_glGenBuffers *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGenBuffers(n, buffers);
        return;
     }

   thread_data->n = n;
   thread_data->buffers = buffers;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGenBuffers,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDeleteBuffers(GLsizei n, const GLuint *buffers);
 */

typedef struct
{
   GLsizei n;
   const GLuint *buffers;
   void *buffers_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glDeleteBuffers;

static void
_gl_thread_glDeleteBuffers(void *data)
{
   Evas_Thread_Command_glDeleteBuffers *thread_data =
      (Evas_Thread_Command_glDeleteBuffers *)data;

   glDeleteBuffers(thread_data->n,
                   thread_data->buffers);


   if (thread_data->buffers_copied)
     eina_mempool_free(_mp_delete_object, thread_data->buffers_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteBuffers_th(GLsizei n, const GLuint *buffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDeleteBuffers thread_data_local;
   Evas_Thread_Command_glDeleteBuffers *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDeleteBuffers(n, buffers);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDeleteBuffers *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDeleteBuffers));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->n = n;
   thread_data->buffers = buffers;

   thread_data->buffers_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (buffers)
     {
        /* 1. check memory size */
        unsigned int copy_size = n * sizeof(GLuint);
        if (copy_size > _mp_delete_object_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->buffers_copied = eina_mempool_malloc(_mp_delete_object, copy_size);
        if (thread_data->buffers_copied)
          {
             memcpy(thread_data->buffers_copied, buffers, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->buffers = (const GLuint  *)thread_data->buffers_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDeleteBuffers,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glBindBuffer(GLenum target, GLuint buffer);
 */

typedef struct
{
   GLenum target;
   GLuint buffer;
   int command_allocated;
   GLBINDBUFFER_VARIABLE_DECLARE /* TODO */

} Evas_Thread_Command_glBindBuffer;

static void
_gl_thread_glBindBuffer(void *data)
{
   Evas_Thread_Command_glBindBuffer *thread_data =
      (Evas_Thread_Command_glBindBuffer *)data;


   GLBINDBUFFER_GLCALL_BEFORE; /* TODO */

   glBindBuffer(thread_data->target,
                thread_data->buffer);

   GLBINDBUFFER_GLCALL_AFTER; /* TODO */

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBindBuffer_th(GLenum target, GLuint buffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glBindBuffer thread_data_local;
   Evas_Thread_Command_glBindBuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glBindBuffer(target, buffer);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glBindBuffer *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glBindBuffer));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->buffer = buffer;

   GLBINDBUFFER_VARIABLE_INIT; /* TODO */

   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   GLBINDBUFFER_ASYNC_PREPARE; /* TODO */

finish:
   GLBINDBUFFER_ENQUEUE_BEFORE; /* TODO */

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glBindBuffer,
                              thread_data,
                              thread_mode);

   GLBINDBUFFER_ENQUEUE_AFTER; /* TODO */
}

/*
 * void
 * glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
 */

typedef struct
{
   GLenum target;
   GLsizeiptr size;
   const void *data;
   GLenum usage;

} Evas_Thread_Command_glBufferData;

static void
_gl_thread_glBufferData(void *data)
{
   Evas_Thread_Command_glBufferData *thread_data =
      (Evas_Thread_Command_glBufferData *)data;

   glBufferData(thread_data->target,
                thread_data->size,
                thread_data->data,
                thread_data->usage);

}

EAPI void
evas_glBufferData_th(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glBufferData thread_data_local;
   Evas_Thread_Command_glBufferData *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glBufferData(target, size, data, usage);
        return;
     }

   thread_data->target = target;
   thread_data->size = size;
   thread_data->data = data;
   thread_data->usage = usage;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glBufferData,
                              thread_data,
                              thread_mode);
}

/*
 * void *
 * glMapBuffer(GLenum target, GLenum access);
 */

typedef struct
{
   void * return_value;
   GLenum target;
   GLenum access;

} Evas_Thread_Command_glMapBuffer;

void * (*orig_evas_glMapBuffer)(GLenum target, GLenum access);

void
glMapBuffer_orig_evas_set(void *func)
{
   orig_evas_glMapBuffer = func;
}

void *
glMapBuffer_orig_evas_get(void)
{
   return orig_evas_glMapBuffer;
}

static void
_gl_thread_glMapBuffer(void *data)
{
   Evas_Thread_Command_glMapBuffer *thread_data =
      (Evas_Thread_Command_glMapBuffer *)data;

   thread_data->return_value = orig_evas_glMapBuffer(thread_data->target,
                                                     thread_data->access);

}

EAPI void *
evas_glMapBuffer_th(GLenum target, GLenum access)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glMapBuffer thread_data_local;
   Evas_Thread_Command_glMapBuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return orig_evas_glMapBuffer(target, access);
     }

   thread_data->target = target;
   thread_data->access = access;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glMapBuffer,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * GLboolean
 * glUnmapBuffer(GLenum target);
 */

typedef struct
{
   GLboolean return_value;
   GLenum target;

} Evas_Thread_Command_glUnmapBuffer;

GLboolean (*orig_evas_glUnmapBuffer)(GLenum target);

void
glUnmapBuffer_orig_evas_set(void *func)
{
   orig_evas_glUnmapBuffer = func;
}

void *
glUnmapBuffer_orig_evas_get(void)
{
   return orig_evas_glUnmapBuffer;
}

static void
_gl_thread_glUnmapBuffer(void *data)
{
   Evas_Thread_Command_glUnmapBuffer *thread_data =
      (Evas_Thread_Command_glUnmapBuffer *)data;

   thread_data->return_value = orig_evas_glUnmapBuffer(thread_data->target);

}

EAPI GLboolean
evas_glUnmapBuffer_th(GLenum target)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUnmapBuffer thread_data_local;
   Evas_Thread_Command_glUnmapBuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return orig_evas_glUnmapBuffer(target);
     }

   thread_data->target = target;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUnmapBuffer,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * GLuint
 * glCreateShader(GLenum type);
 */

typedef struct
{
   GLuint return_value;
   GLenum type;

} Evas_Thread_Command_glCreateShader;

static void
_gl_thread_glCreateShader(void *data)
{
   Evas_Thread_Command_glCreateShader *thread_data =
      (Evas_Thread_Command_glCreateShader *)data;

   thread_data->return_value = glCreateShader(thread_data->type);

}

EAPI GLuint
evas_glCreateShader_th(GLenum type)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glCreateShader thread_data_local;
   Evas_Thread_Command_glCreateShader *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glCreateShader(type);
     }

   thread_data->type = type;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glCreateShader,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
 */

typedef struct
{
   GLuint shader;
   GLsizei count;
   const GLchar **string;
   const GLint *length;
   int command_allocated;
   GLSHADERSOURCE_VARIABLE_DECLARE /* TODO */

} Evas_Thread_Command_glShaderSource;

static void
_gl_thread_glShaderSource(void *data)
{
   Evas_Thread_Command_glShaderSource *thread_data =
      (Evas_Thread_Command_glShaderSource *)data;


   GLSHADERSOURCE_GLCALL_BEFORE; /* TODO */

   glShaderSource(thread_data->shader,
                  thread_data->count,
                  thread_data->string,
                  thread_data->length);

   GLSHADERSOURCE_GLCALL_AFTER; /* TODO */

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glShaderSource_th(GLuint shader, GLsizei count, const GLchar **string, const GLint *length)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glShaderSource thread_data_local;
   Evas_Thread_Command_glShaderSource *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glShaderSource(shader, count, string, length);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glShaderSource *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glShaderSource));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->shader = shader;
   thread_data->count = count;
   thread_data->string = string;
   thread_data->length = length;

   GLSHADERSOURCE_VARIABLE_INIT; /* TODO */

   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   GLSHADERSOURCE_ASYNC_PREPARE; /* TODO */

finish:
   GLSHADERSOURCE_ENQUEUE_BEFORE; /* TODO */

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glShaderSource,
                              thread_data,
                              thread_mode);

   GLSHADERSOURCE_ENQUEUE_AFTER; /* TODO */
}

/*
 * void
 * glCompileShader(GLuint shader);
 */

typedef struct
{
   GLuint shader;
   int command_allocated;

} Evas_Thread_Command_glCompileShader;

static void
_gl_thread_glCompileShader(void *data)
{
   Evas_Thread_Command_glCompileShader *thread_data =
      (Evas_Thread_Command_glCompileShader *)data;

   glCompileShader(thread_data->shader);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glCompileShader_th(GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glCompileShader thread_data_local;
   Evas_Thread_Command_glCompileShader *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glCompileShader(shader);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glCompileShader *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glCompileShader));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glCompileShader,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glReleaseShaderCompiler(void);
 */

void (*orig_evas_glReleaseShaderCompiler)(void);

void
glReleaseShaderCompiler_orig_evas_set(void *func)
{
   orig_evas_glReleaseShaderCompiler = func;
}

void *
glReleaseShaderCompiler_orig_evas_get(void)
{
   return orig_evas_glReleaseShaderCompiler;
}

static void
_gl_thread_glReleaseShaderCompiler(void *data EINA_UNUSED)
{
   orig_evas_glReleaseShaderCompiler();

}

EAPI void
evas_glReleaseShaderCompiler_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glReleaseShaderCompiler();
        return;
     }

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glReleaseShaderCompiler,
                              NULL,
                              thread_mode);
}

/*
 * void
 * glDeleteShader(GLuint shader);
 */

typedef struct
{
   GLuint shader;
   int command_allocated;

} Evas_Thread_Command_glDeleteShader;

static void
_gl_thread_glDeleteShader(void *data)
{
   Evas_Thread_Command_glDeleteShader *thread_data =
      (Evas_Thread_Command_glDeleteShader *)data;

   glDeleteShader(thread_data->shader);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteShader_th(GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDeleteShader thread_data_local;
   Evas_Thread_Command_glDeleteShader *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDeleteShader(shader);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDeleteShader *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDeleteShader));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDeleteShader,
                              thread_data,
                              thread_mode);
}

/*
 * GLuint
 * glCreateProgram(void);
 */

typedef struct
{
   GLuint return_value;

} Evas_Thread_Command_glCreateProgram;

static void
_gl_thread_glCreateProgram(void *data)
{
   Evas_Thread_Command_glCreateProgram *thread_data =
      (Evas_Thread_Command_glCreateProgram *)data;

   thread_data->return_value = glCreateProgram();

}

EAPI GLuint
evas_glCreateProgram_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glCreateProgram thread_data_local;
   Evas_Thread_Command_glCreateProgram *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glCreateProgram();
     }


   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glCreateProgram,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glAttachShader(GLuint program, GLuint shader);
 */

typedef struct
{
   GLuint program;
   GLuint shader;
   int command_allocated;

} Evas_Thread_Command_glAttachShader;

static void
_gl_thread_glAttachShader(void *data)
{
   Evas_Thread_Command_glAttachShader *thread_data =
      (Evas_Thread_Command_glAttachShader *)data;

   glAttachShader(thread_data->program,
                  thread_data->shader);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glAttachShader_th(GLuint program, GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glAttachShader thread_data_local;
   Evas_Thread_Command_glAttachShader *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glAttachShader(program, shader);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glAttachShader *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glAttachShader));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;
   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glAttachShader,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDetachShader(GLuint program, GLuint shader);
 */

typedef struct
{
   GLuint program;
   GLuint shader;
   int command_allocated;

} Evas_Thread_Command_glDetachShader;

static void
_gl_thread_glDetachShader(void *data)
{
   Evas_Thread_Command_glDetachShader *thread_data =
      (Evas_Thread_Command_glDetachShader *)data;

   glDetachShader(thread_data->program,
                  thread_data->shader);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDetachShader_th(GLuint program, GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDetachShader thread_data_local;
   Evas_Thread_Command_glDetachShader *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDetachShader(program, shader);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDetachShader *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDetachShader));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;
   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDetachShader,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glLinkProgram(GLuint program);
 */

typedef struct
{
   GLuint program;
   int command_allocated;

} Evas_Thread_Command_glLinkProgram;

static void
_gl_thread_glLinkProgram(void *data)
{
   Evas_Thread_Command_glLinkProgram *thread_data =
      (Evas_Thread_Command_glLinkProgram *)data;

   glLinkProgram(thread_data->program);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glLinkProgram_th(GLuint program)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glLinkProgram thread_data_local;
   Evas_Thread_Command_glLinkProgram *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glLinkProgram(program);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glLinkProgram *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glLinkProgram));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glLinkProgram,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUseProgram(GLuint program);
 */

typedef struct
{
   GLuint program;
   int command_allocated;

} Evas_Thread_Command_glUseProgram;

static void
_gl_thread_glUseProgram(void *data)
{
   Evas_Thread_Command_glUseProgram *thread_data =
      (Evas_Thread_Command_glUseProgram *)data;

   glUseProgram(thread_data->program);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUseProgram_th(GLuint program)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUseProgram thread_data_local;
   Evas_Thread_Command_glUseProgram *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUseProgram(program);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUseProgram *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUseProgram));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUseProgram,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glProgramParameteri(GLuint program, GLenum pname, GLint value);
 */

typedef struct
{
   GLuint program;
   GLenum pname;
   GLint value;
   int command_allocated;

} Evas_Thread_Command_glProgramParameteri;

void (*orig_evas_glProgramParameteri)(GLuint program, GLenum pname, GLint value);

void
glProgramParameteri_orig_evas_set(void *func)
{
   orig_evas_glProgramParameteri = func;
}

void *
glProgramParameteri_orig_evas_get(void)
{
   return orig_evas_glProgramParameteri;
}

static void
_gl_thread_glProgramParameteri(void *data)
{
   Evas_Thread_Command_glProgramParameteri *thread_data =
      (Evas_Thread_Command_glProgramParameteri *)data;

   orig_evas_glProgramParameteri(thread_data->program,
                                 thread_data->pname,
                                 thread_data->value);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glProgramParameteri_th(GLuint program, GLenum pname, GLint value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glProgramParameteri thread_data_local;
   Evas_Thread_Command_glProgramParameteri *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glProgramParameteri(program, pname, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glProgramParameteri *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glProgramParameteri));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;
   thread_data->pname = pname;
   thread_data->value = value;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glProgramParameteri,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDeleteProgram(GLuint program);
 */

typedef struct
{
   GLuint program;
   int command_allocated;

} Evas_Thread_Command_glDeleteProgram;

static void
_gl_thread_glDeleteProgram(void *data)
{
   Evas_Thread_Command_glDeleteProgram *thread_data =
      (Evas_Thread_Command_glDeleteProgram *)data;

   glDeleteProgram(thread_data->program);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteProgram_th(GLuint program)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDeleteProgram thread_data_local;
   Evas_Thread_Command_glDeleteProgram *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDeleteProgram(program);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDeleteProgram *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDeleteProgram));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDeleteProgram,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
 */

typedef struct
{
   GLuint program;
   GLsizei bufSize;
   GLsizei *length;
   GLenum *binaryFormat;
   void *binary;

} Evas_Thread_Command_glGetProgramBinary;

void (*orig_evas_glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);

void
glGetProgramBinary_orig_evas_set(void *func)
{
   orig_evas_glGetProgramBinary = func;
}

void *
glGetProgramBinary_orig_evas_get(void)
{
   return orig_evas_glGetProgramBinary;
}

static void
_gl_thread_glGetProgramBinary(void *data)
{
   Evas_Thread_Command_glGetProgramBinary *thread_data =
      (Evas_Thread_Command_glGetProgramBinary *)data;

   orig_evas_glGetProgramBinary(thread_data->program,
                                thread_data->bufSize,
                                thread_data->length,
                                thread_data->binaryFormat,
                                thread_data->binary);

}

EAPI void
evas_glGetProgramBinary_th(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetProgramBinary thread_data_local;
   Evas_Thread_Command_glGetProgramBinary *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
        return;
     }

   thread_data->program = program;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->binaryFormat = binaryFormat;
   thread_data->binary = binary;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetProgramBinary,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLint length);
 */

typedef struct
{
   GLuint program;
   GLenum binaryFormat;
   const void *binary;
   GLint length;
   void *binary_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glProgramBinary;

void (*orig_evas_glProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLint length);

void
glProgramBinary_orig_evas_set(void *func)
{
   orig_evas_glProgramBinary = func;
}

void *
glProgramBinary_orig_evas_get(void)
{
   return orig_evas_glProgramBinary;
}

static void
_gl_thread_glProgramBinary(void *data)
{
   Evas_Thread_Command_glProgramBinary *thread_data =
      (Evas_Thread_Command_glProgramBinary *)data;

   orig_evas_glProgramBinary(thread_data->program,
                             thread_data->binaryFormat,
                             thread_data->binary,
                             thread_data->length);


   if (thread_data->binary_copied)
     eina_mempool_free(_mp_default, thread_data->binary_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glProgramBinary_th(GLuint program, GLenum binaryFormat, const void *binary, GLint length)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glProgramBinary thread_data_local;
   Evas_Thread_Command_glProgramBinary *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glProgramBinary(program, binaryFormat, binary, length);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glProgramBinary *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glProgramBinary));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;
   thread_data->binaryFormat = binaryFormat;
   thread_data->binary = binary;
   thread_data->length = length;

   thread_data->binary_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (binary)
     {
        /* 1. check memory size */
        unsigned int copy_size = length;
        if (copy_size > _mp_default_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->binary_copied = eina_mempool_malloc(_mp_default, copy_size);
        if (thread_data->binary_copied)
          {
             memcpy(thread_data->binary_copied, binary, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->binary = (const void  *)thread_data->binary_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glProgramBinary,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
 */

typedef struct
{
   GLuint program;
   GLuint index;
   GLsizei bufSize;
   GLsizei *length;
   GLint *size;
   GLenum *type;
   GLchar *name;

} Evas_Thread_Command_glGetActiveAttrib;

static void
_gl_thread_glGetActiveAttrib(void *data)
{
   Evas_Thread_Command_glGetActiveAttrib *thread_data =
      (Evas_Thread_Command_glGetActiveAttrib *)data;

   glGetActiveAttrib(thread_data->program,
                     thread_data->index,
                     thread_data->bufSize,
                     thread_data->length,
                     thread_data->size,
                     thread_data->type,
                     thread_data->name);

}

EAPI void
evas_glGetActiveAttrib_th(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetActiveAttrib thread_data_local;
   Evas_Thread_Command_glGetActiveAttrib *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetActiveAttrib(program, index, bufSize, length, size, type, name);
        return;
     }

   thread_data->program = program;
   thread_data->index = index;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->size = size;
   thread_data->type = type;
   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetActiveAttrib,
                              thread_data,
                              thread_mode);
}

/*
 * GLint
 * glGetAttribLocation(GLuint program, const GLchar *name);
 */

typedef struct
{
   GLint return_value;
   GLuint program;
   const GLchar *name;

} Evas_Thread_Command_glGetAttribLocation;

static void
_gl_thread_glGetAttribLocation(void *data)
{
   Evas_Thread_Command_glGetAttribLocation *thread_data =
      (Evas_Thread_Command_glGetAttribLocation *)data;

   thread_data->return_value = glGetAttribLocation(thread_data->program,
                                                   thread_data->name);

}

EAPI GLint
evas_glGetAttribLocation_th(GLuint program, const GLchar *name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetAttribLocation thread_data_local;
   Evas_Thread_Command_glGetAttribLocation *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glGetAttribLocation(program, name);
     }

   thread_data->program = program;
   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetAttribLocation,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
 */

typedef struct
{
   GLuint program;
   GLuint index;
   const GLchar *name;

} Evas_Thread_Command_glBindAttribLocation;

static void
_gl_thread_glBindAttribLocation(void *data)
{
   Evas_Thread_Command_glBindAttribLocation *thread_data =
      (Evas_Thread_Command_glBindAttribLocation *)data;

   glBindAttribLocation(thread_data->program,
                        thread_data->index,
                        thread_data->name);

}

EAPI void
evas_glBindAttribLocation_th(GLuint program, GLuint index, const GLchar *name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glBindAttribLocation thread_data_local;
   Evas_Thread_Command_glBindAttribLocation *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glBindAttribLocation(program, index, name);
        return;
     }

   thread_data->program = program;
   thread_data->index = index;
   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glBindAttribLocation,
                              thread_data,
                              thread_mode);
}

/*
 * GLint
 * glGetUniformLocation(GLuint program, const GLchar *name);
 */

typedef struct
{
   GLint return_value;
   GLuint program;
   const GLchar *name;

} Evas_Thread_Command_glGetUniformLocation;

static void
_gl_thread_glGetUniformLocation(void *data)
{
   Evas_Thread_Command_glGetUniformLocation *thread_data =
      (Evas_Thread_Command_glGetUniformLocation *)data;

   thread_data->return_value = glGetUniformLocation(thread_data->program,
                                                    thread_data->name);

}

EAPI GLint
evas_glGetUniformLocation_th(GLuint program, const GLchar *name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetUniformLocation thread_data_local;
   Evas_Thread_Command_glGetUniformLocation *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glGetUniformLocation(program, name);
     }

   thread_data->program = program;
   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetUniformLocation,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glUniform1f(GLint location, GLfloat v0);
 */

typedef struct
{
   GLint location;
   GLfloat v0;
   int command_allocated;

} Evas_Thread_Command_glUniform1f;

static void
_gl_thread_glUniform1f(void *data)
{
   Evas_Thread_Command_glUniform1f *thread_data =
      (Evas_Thread_Command_glUniform1f *)data;

   glUniform1f(thread_data->location,
               thread_data->v0);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform1f_th(GLint location, GLfloat v0)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform1f thread_data_local;
   Evas_Thread_Command_glUniform1f *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform1f(location, v0);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform1f *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform1f));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform1f,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform1i(GLint location, GLint v0);
 */

typedef struct
{
   GLint location;
   GLint v0;
   int command_allocated;

} Evas_Thread_Command_glUniform1i;

static void
_gl_thread_glUniform1i(void *data)
{
   Evas_Thread_Command_glUniform1i *thread_data =
      (Evas_Thread_Command_glUniform1i *)data;

   glUniform1i(thread_data->location,
               thread_data->v0);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform1i_th(GLint location, GLint v0)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform1i thread_data_local;
   Evas_Thread_Command_glUniform1i *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform1i(location, v0);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform1i *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform1i));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform1i,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform2f(GLint location, GLfloat v0, GLfloat v1);
 */

typedef struct
{
   GLint location;
   GLfloat v0;
   GLfloat v1;
   int command_allocated;

} Evas_Thread_Command_glUniform2f;

static void
_gl_thread_glUniform2f(void *data)
{
   Evas_Thread_Command_glUniform2f *thread_data =
      (Evas_Thread_Command_glUniform2f *)data;

   glUniform2f(thread_data->location,
               thread_data->v0,
               thread_data->v1);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform2f_th(GLint location, GLfloat v0, GLfloat v1)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform2f thread_data_local;
   Evas_Thread_Command_glUniform2f *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform2f(location, v0, v1);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform2f *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform2f));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;
   thread_data->v1 = v1;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform2f,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform2i(GLint location, GLint v0, GLint v1);
 */

typedef struct
{
   GLint location;
   GLint v0;
   GLint v1;
   int command_allocated;

} Evas_Thread_Command_glUniform2i;

static void
_gl_thread_glUniform2i(void *data)
{
   Evas_Thread_Command_glUniform2i *thread_data =
      (Evas_Thread_Command_glUniform2i *)data;

   glUniform2i(thread_data->location,
               thread_data->v0,
               thread_data->v1);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform2i_th(GLint location, GLint v0, GLint v1)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform2i thread_data_local;
   Evas_Thread_Command_glUniform2i *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform2i(location, v0, v1);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform2i *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform2i));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;
   thread_data->v1 = v1;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform2i,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
 */

typedef struct
{
   GLint location;
   GLfloat v0;
   GLfloat v1;
   GLfloat v2;
   int command_allocated;

} Evas_Thread_Command_glUniform3f;

static void
_gl_thread_glUniform3f(void *data)
{
   Evas_Thread_Command_glUniform3f *thread_data =
      (Evas_Thread_Command_glUniform3f *)data;

   glUniform3f(thread_data->location,
               thread_data->v0,
               thread_data->v1,
               thread_data->v2);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform3f_th(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform3f thread_data_local;
   Evas_Thread_Command_glUniform3f *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform3f(location, v0, v1, v2);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform3f *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform3f));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;
   thread_data->v1 = v1;
   thread_data->v2 = v2;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform3f,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
 */

typedef struct
{
   GLint location;
   GLint v0;
   GLint v1;
   GLint v2;
   int command_allocated;

} Evas_Thread_Command_glUniform3i;

static void
_gl_thread_glUniform3i(void *data)
{
   Evas_Thread_Command_glUniform3i *thread_data =
      (Evas_Thread_Command_glUniform3i *)data;

   glUniform3i(thread_data->location,
               thread_data->v0,
               thread_data->v1,
               thread_data->v2);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform3i_th(GLint location, GLint v0, GLint v1, GLint v2)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform3i thread_data_local;
   Evas_Thread_Command_glUniform3i *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform3i(location, v0, v1, v2);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform3i *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform3i));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;
   thread_data->v1 = v1;
   thread_data->v2 = v2;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform3i,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
 */

typedef struct
{
   GLint location;
   GLfloat v0;
   GLfloat v1;
   GLfloat v2;
   GLfloat v3;
   int command_allocated;

} Evas_Thread_Command_glUniform4f;

static void
_gl_thread_glUniform4f(void *data)
{
   Evas_Thread_Command_glUniform4f *thread_data =
      (Evas_Thread_Command_glUniform4f *)data;

   glUniform4f(thread_data->location,
               thread_data->v0,
               thread_data->v1,
               thread_data->v2,
               thread_data->v3);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform4f_th(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform4f thread_data_local;
   Evas_Thread_Command_glUniform4f *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform4f(location, v0, v1, v2, v3);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform4f *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform4f));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;
   thread_data->v1 = v1;
   thread_data->v2 = v2;
   thread_data->v3 = v3;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform4f,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
 */

typedef struct
{
   GLint location;
   GLint v0;
   GLint v1;
   GLint v2;
   GLint v3;
   int command_allocated;

} Evas_Thread_Command_glUniform4i;

static void
_gl_thread_glUniform4i(void *data)
{
   Evas_Thread_Command_glUniform4i *thread_data =
      (Evas_Thread_Command_glUniform4i *)data;

   glUniform4i(thread_data->location,
               thread_data->v0,
               thread_data->v1,
               thread_data->v2,
               thread_data->v3);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform4i_th(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform4i thread_data_local;
   Evas_Thread_Command_glUniform4i *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform4i(location, v0, v1, v2, v3);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform4i *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform4i));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;
   thread_data->v1 = v1;
   thread_data->v2 = v2;
   thread_data->v3 = v3;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform4i,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform1fv(GLint location, GLsizei count, const GLfloat *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   const GLfloat *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniform1fv;

static void
_gl_thread_glUniform1fv(void *data)
{
   Evas_Thread_Command_glUniform1fv *thread_data =
      (Evas_Thread_Command_glUniform1fv *)data;

   glUniform1fv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform1fv_th(GLint location, GLsizei count, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform1fv thread_data_local;
   Evas_Thread_Command_glUniform1fv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform1fv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform1fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform1fv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 1 * count * sizeof(GLfloat);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLfloat  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform1fv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform1iv(GLint location, GLsizei count, const GLint *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   const GLint *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniform1iv;

static void
_gl_thread_glUniform1iv(void *data)
{
   Evas_Thread_Command_glUniform1iv *thread_data =
      (Evas_Thread_Command_glUniform1iv *)data;

   glUniform1iv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform1iv_th(GLint location, GLsizei count, const GLint *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform1iv thread_data_local;
   Evas_Thread_Command_glUniform1iv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform1iv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform1iv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform1iv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 1 * count * sizeof(GLint);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLint  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform1iv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   const GLfloat *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniform2fv;

static void
_gl_thread_glUniform2fv(void *data)
{
   Evas_Thread_Command_glUniform2fv *thread_data =
      (Evas_Thread_Command_glUniform2fv *)data;

   glUniform2fv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform2fv_th(GLint location, GLsizei count, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform2fv thread_data_local;
   Evas_Thread_Command_glUniform2fv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform2fv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform2fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform2fv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 2 * count * sizeof(GLfloat);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLfloat  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform2fv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform2iv(GLint location, GLsizei count, const GLint *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   const GLint *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniform2iv;

static void
_gl_thread_glUniform2iv(void *data)
{
   Evas_Thread_Command_glUniform2iv *thread_data =
      (Evas_Thread_Command_glUniform2iv *)data;

   glUniform2iv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform2iv_th(GLint location, GLsizei count, const GLint *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform2iv thread_data_local;
   Evas_Thread_Command_glUniform2iv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform2iv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform2iv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform2iv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 2 * count * sizeof(GLint);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLint  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform2iv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   const GLfloat *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniform3fv;

static void
_gl_thread_glUniform3fv(void *data)
{
   Evas_Thread_Command_glUniform3fv *thread_data =
      (Evas_Thread_Command_glUniform3fv *)data;

   glUniform3fv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform3fv_th(GLint location, GLsizei count, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform3fv thread_data_local;
   Evas_Thread_Command_glUniform3fv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform3fv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform3fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform3fv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 3 * count * sizeof(GLfloat);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLfloat  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform3fv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform3iv(GLint location, GLsizei count, const GLint *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   const GLint *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniform3iv;

static void
_gl_thread_glUniform3iv(void *data)
{
   Evas_Thread_Command_glUniform3iv *thread_data =
      (Evas_Thread_Command_glUniform3iv *)data;

   glUniform3iv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform3iv_th(GLint location, GLsizei count, const GLint *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform3iv thread_data_local;
   Evas_Thread_Command_glUniform3iv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform3iv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform3iv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform3iv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 3 * count * sizeof(GLint);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLint  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform3iv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   const GLfloat *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniform4fv;

static void
_gl_thread_glUniform4fv(void *data)
{
   Evas_Thread_Command_glUniform4fv *thread_data =
      (Evas_Thread_Command_glUniform4fv *)data;

   glUniform4fv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform4fv_th(GLint location, GLsizei count, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform4fv thread_data_local;
   Evas_Thread_Command_glUniform4fv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform4fv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform4fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform4fv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 4 * count * sizeof(GLfloat);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLfloat  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform4fv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniform4iv(GLint location, GLsizei count, const GLint *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   const GLint *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniform4iv;

static void
_gl_thread_glUniform4iv(void *data)
{
   Evas_Thread_Command_glUniform4iv *thread_data =
      (Evas_Thread_Command_glUniform4iv *)data;

   glUniform4iv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform4iv_th(GLint location, GLsizei count, const GLint *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniform4iv thread_data_local;
   Evas_Thread_Command_glUniform4iv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniform4iv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniform4iv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniform4iv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 4 * count * sizeof(GLint);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLint  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniform4iv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   GLboolean transpose;
   const GLfloat *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniformMatrix2fv;

static void
_gl_thread_glUniformMatrix2fv(void *data)
{
   Evas_Thread_Command_glUniformMatrix2fv *thread_data =
      (Evas_Thread_Command_glUniformMatrix2fv *)data;

   glUniformMatrix2fv(thread_data->location,
                      thread_data->count,
                      thread_data->transpose,
                      thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniformMatrix2fv_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniformMatrix2fv thread_data_local;
   Evas_Thread_Command_glUniformMatrix2fv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniformMatrix2fv(location, count, transpose, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniformMatrix2fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniformMatrix2fv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->transpose = transpose;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 4 * count * sizeof(GLfloat);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLfloat  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniformMatrix2fv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   GLboolean transpose;
   const GLfloat *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniformMatrix3fv;

static void
_gl_thread_glUniformMatrix3fv(void *data)
{
   Evas_Thread_Command_glUniformMatrix3fv *thread_data =
      (Evas_Thread_Command_glUniformMatrix3fv *)data;

   glUniformMatrix3fv(thread_data->location,
                      thread_data->count,
                      thread_data->transpose,
                      thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniformMatrix3fv_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniformMatrix3fv thread_data_local;
   Evas_Thread_Command_glUniformMatrix3fv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniformMatrix3fv(location, count, transpose, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniformMatrix3fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniformMatrix3fv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->transpose = transpose;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 9 * count * sizeof(GLfloat);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLfloat  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniformMatrix3fv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
 */

typedef struct
{
   GLint location;
   GLsizei count;
   GLboolean transpose;
   const GLfloat *value;
   void *value_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glUniformMatrix4fv;

static void
_gl_thread_glUniformMatrix4fv(void *data)
{
   Evas_Thread_Command_glUniformMatrix4fv *thread_data =
      (Evas_Thread_Command_glUniformMatrix4fv *)data;

   glUniformMatrix4fv(thread_data->location,
                      thread_data->count,
                      thread_data->transpose,
                      thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniformMatrix4fv_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glUniformMatrix4fv thread_data_local;
   Evas_Thread_Command_glUniformMatrix4fv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glUniformMatrix4fv(location, count, transpose, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glUniformMatrix4fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glUniformMatrix4fv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->count = count;
   thread_data->transpose = transpose;
   thread_data->value = value;

   thread_data->value_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (value)
     {
        /* 1. check memory size */
        unsigned int copy_size = 16 * count * sizeof(GLfloat);
        if (copy_size > _mp_uniform_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->value_copied = eina_mempool_malloc(_mp_uniform, copy_size);
        if (thread_data->value_copied)
          {
             memcpy(thread_data->value_copied, value, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->value = (const GLfloat  *)thread_data->value_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glUniformMatrix4fv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
 */

typedef struct
{
   GLint x;
   GLint y;
   GLsizei width;
   GLsizei height;
   int command_allocated;

} Evas_Thread_Command_glViewport;

static void
_gl_thread_glViewport(void *data)
{
   Evas_Thread_Command_glViewport *thread_data =
      (Evas_Thread_Command_glViewport *)data;

   glViewport(thread_data->x,
              thread_data->y,
              thread_data->width,
              thread_data->height);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glViewport_th(GLint x, GLint y, GLsizei width, GLsizei height)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glViewport thread_data_local;
   Evas_Thread_Command_glViewport *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glViewport(x, y, width, height);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glViewport *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glViewport));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->x = x;
   thread_data->y = y;
   thread_data->width = width;
   thread_data->height = height;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glViewport,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glEnable(GLenum cap);
 */

typedef struct
{
   GLenum cap;
   int command_allocated;

} Evas_Thread_Command_glEnable;

static void
_gl_thread_glEnable(void *data)
{
   Evas_Thread_Command_glEnable *thread_data =
      (Evas_Thread_Command_glEnable *)data;

   glEnable(thread_data->cap);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glEnable_th(GLenum cap)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glEnable thread_data_local;
   Evas_Thread_Command_glEnable *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glEnable(cap);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glEnable *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glEnable));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_ENQUEUE;
          }
     }

   thread_data->cap = cap;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glEnable,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDisable(GLenum cap);
 */

typedef struct
{
   GLenum cap;
   int command_allocated;

} Evas_Thread_Command_glDisable;

static void
_gl_thread_glDisable(void *data)
{
   Evas_Thread_Command_glDisable *thread_data =
      (Evas_Thread_Command_glDisable *)data;

   glDisable(thread_data->cap);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDisable_th(GLenum cap)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDisable thread_data_local;
   Evas_Thread_Command_glDisable *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDisable(cap);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDisable *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDisable));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_ENQUEUE;
          }
     }

   thread_data->cap = cap;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDisable,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glLineWidth(GLfloat width);
 */

typedef struct
{
   GLfloat width;
   int command_allocated;

} Evas_Thread_Command_glLineWidth;

static void
_gl_thread_glLineWidth(void *data)
{
   Evas_Thread_Command_glLineWidth *thread_data =
      (Evas_Thread_Command_glLineWidth *)data;

   glLineWidth(thread_data->width);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glLineWidth_th(GLfloat width)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glLineWidth thread_data_local;
   Evas_Thread_Command_glLineWidth *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glLineWidth(width);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glLineWidth *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glLineWidth));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->width = width;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glLineWidth,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glPolygonOffset(GLfloat factor, GLfloat units);
 */

typedef struct
{
   GLfloat factor;
   GLfloat units;
   int command_allocated;

} Evas_Thread_Command_glPolygonOffset;

static void
_gl_thread_glPolygonOffset(void *data)
{
   Evas_Thread_Command_glPolygonOffset *thread_data =
      (Evas_Thread_Command_glPolygonOffset *)data;

   glPolygonOffset(thread_data->factor,
                   thread_data->units);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glPolygonOffset_th(GLfloat factor, GLfloat units)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glPolygonOffset thread_data_local;
   Evas_Thread_Command_glPolygonOffset *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glPolygonOffset(factor, units);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glPolygonOffset *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glPolygonOffset));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->factor = factor;
   thread_data->units = units;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glPolygonOffset,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glPixelStorei(GLenum pname, GLint param);
 */

typedef struct
{
   GLenum pname;
   GLint param;
   int command_allocated;
   GLPIXELSTOREI_VARIABLE_DECLARE /* TODO */

} Evas_Thread_Command_glPixelStorei;

static void
_gl_thread_glPixelStorei(void *data)
{
   Evas_Thread_Command_glPixelStorei *thread_data =
      (Evas_Thread_Command_glPixelStorei *)data;


   GLPIXELSTOREI_GLCALL_BEFORE; /* TODO */

   glPixelStorei(thread_data->pname,
                 thread_data->param);

   GLPIXELSTOREI_GLCALL_AFTER; /* TODO */

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glPixelStorei_th(GLenum pname, GLint param)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glPixelStorei thread_data_local;
   Evas_Thread_Command_glPixelStorei *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glPixelStorei(pname, param);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glPixelStorei *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glPixelStorei));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->pname = pname;
   thread_data->param = param;

   GLPIXELSTOREI_VARIABLE_INIT; /* TODO */

   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   GLPIXELSTOREI_ASYNC_PREPARE; /* TODO */

finish:
   GLPIXELSTOREI_ENQUEUE_BEFORE; /* TODO */

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glPixelStorei,
                              thread_data,
                              thread_mode);

   GLPIXELSTOREI_ENQUEUE_AFTER; /* TODO */
}

/*
 * void
 * glActiveTexture(GLenum texture);
 */

typedef struct
{
   GLenum texture;
   int command_allocated;

} Evas_Thread_Command_glActiveTexture;

static void
_gl_thread_glActiveTexture(void *data)
{
   Evas_Thread_Command_glActiveTexture *thread_data =
      (Evas_Thread_Command_glActiveTexture *)data;

   glActiveTexture(thread_data->texture);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glActiveTexture_th(GLenum texture)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glActiveTexture thread_data_local;
   Evas_Thread_Command_glActiveTexture *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glActiveTexture(texture);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glActiveTexture *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glActiveTexture));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->texture = texture;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glActiveTexture,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGenTextures(GLsizei n, GLuint *textures);
 */

typedef struct
{
   GLsizei n;
   GLuint *textures;

} Evas_Thread_Command_glGenTextures;

static void
_gl_thread_glGenTextures(void *data)
{
   Evas_Thread_Command_glGenTextures *thread_data =
      (Evas_Thread_Command_glGenTextures *)data;

   glGenTextures(thread_data->n,
                 thread_data->textures);

}

EAPI void
evas_glGenTextures_th(GLsizei n, GLuint *textures)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGenTextures thread_data_local;
   Evas_Thread_Command_glGenTextures *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGenTextures(n, textures);
        return;
     }

   thread_data->n = n;
   thread_data->textures = textures;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGenTextures,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glBindTexture(GLenum target, GLuint texture);
 */

typedef struct
{
   GLenum target;
   GLuint texture;
   int command_allocated;

} Evas_Thread_Command_glBindTexture;

static void
_gl_thread_glBindTexture(void *data)
{
   Evas_Thread_Command_glBindTexture *thread_data =
      (Evas_Thread_Command_glBindTexture *)data;

   glBindTexture(thread_data->target,
                 thread_data->texture);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBindTexture_th(GLenum target, GLuint texture)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glBindTexture thread_data_local;
   Evas_Thread_Command_glBindTexture *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glBindTexture(target, texture);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glBindTexture *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glBindTexture));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->texture = texture;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glBindTexture,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDeleteTextures(GLsizei n, const GLuint *textures);
 */

typedef struct
{
   GLsizei n;
   const GLuint *textures;
   void *textures_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glDeleteTextures;

static void
_gl_thread_glDeleteTextures(void *data)
{
   Evas_Thread_Command_glDeleteTextures *thread_data =
      (Evas_Thread_Command_glDeleteTextures *)data;

   glDeleteTextures(thread_data->n,
                    thread_data->textures);


   if (thread_data->textures_copied)
     eina_mempool_free(_mp_delete_object, thread_data->textures_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteTextures_th(GLsizei n, const GLuint *textures)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDeleteTextures thread_data_local;
   Evas_Thread_Command_glDeleteTextures *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDeleteTextures(n, textures);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDeleteTextures *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDeleteTextures));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->n = n;
   thread_data->textures = textures;

   thread_data->textures_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (textures)
     {
        /* 1. check memory size */
        unsigned int copy_size = n * sizeof(GLuint);
        if (copy_size > _mp_delete_object_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->textures_copied = eina_mempool_malloc(_mp_delete_object, copy_size);
        if (thread_data->textures_copied)
          {
             memcpy(thread_data->textures_copied, textures, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->textures = (const GLuint  *)thread_data->textures_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDeleteTextures,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
 */

typedef struct
{
   GLenum target;
   GLint level;
   GLint internalformat;
   GLsizei width;
   GLsizei height;
   GLint border;
   GLenum format;
   GLenum type;
   const void *pixels;
   int command_allocated;
   GLTEXIMAGE2D_VARIABLE_DECLARE /* TODO */

} Evas_Thread_Command_glTexImage2D;

static void
_gl_thread_glTexImage2D(void *data)
{
   Evas_Thread_Command_glTexImage2D *thread_data =
      (Evas_Thread_Command_glTexImage2D *)data;


   GLTEXIMAGE2D_GLCALL_BEFORE; /* TODO */

   glTexImage2D(thread_data->target,
                thread_data->level,
                thread_data->internalformat,
                thread_data->width,
                thread_data->height,
                thread_data->border,
                thread_data->format,
                thread_data->type,
                thread_data->pixels);

   GLTEXIMAGE2D_GLCALL_AFTER; /* TODO */

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexImage2D_th(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glTexImage2D thread_data_local;
   Evas_Thread_Command_glTexImage2D *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glTexImage2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glTexImage2D));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->level = level;
   thread_data->internalformat = internalformat;
   thread_data->width = width;
   thread_data->height = height;
   thread_data->border = border;
   thread_data->format = format;
   thread_data->type = type;
   thread_data->pixels = pixels;

   GLTEXIMAGE2D_VARIABLE_INIT; /* TODO */

   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   GLTEXIMAGE2D_ASYNC_PREPARE; /* TODO */

finish:
   GLTEXIMAGE2D_ENQUEUE_BEFORE; /* TODO */

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glTexImage2D,
                              thread_data,
                              thread_mode);

   GLTEXIMAGE2D_ENQUEUE_AFTER; /* TODO */
}

/*
 * void
 * glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
 */

typedef struct
{
   GLenum target;
   GLint level;
   GLint xoffset;
   GLint yoffset;
   GLsizei width;
   GLsizei height;
   GLenum format;
   GLenum type;
   const void *pixels;
   int command_allocated;
   GLTEXSUBIMAGE2D_VARIABLE_DECLARE /* TODO */

} Evas_Thread_Command_glTexSubImage2D;

static void
_gl_thread_glTexSubImage2D(void *data)
{
   Evas_Thread_Command_glTexSubImage2D *thread_data =
      (Evas_Thread_Command_glTexSubImage2D *)data;


   GLTEXSUBIMAGE2D_GLCALL_BEFORE; /* TODO */

   glTexSubImage2D(thread_data->target,
                   thread_data->level,
                   thread_data->xoffset,
                   thread_data->yoffset,
                   thread_data->width,
                   thread_data->height,
                   thread_data->format,
                   thread_data->type,
                   thread_data->pixels);

   GLTEXSUBIMAGE2D_GLCALL_AFTER; /* TODO */

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexSubImage2D_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glTexSubImage2D thread_data_local;
   Evas_Thread_Command_glTexSubImage2D *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glTexSubImage2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glTexSubImage2D));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->level = level;
   thread_data->xoffset = xoffset;
   thread_data->yoffset = yoffset;
   thread_data->width = width;
   thread_data->height = height;
   thread_data->format = format;
   thread_data->type = type;
   thread_data->pixels = pixels;

   GLTEXSUBIMAGE2D_VARIABLE_INIT; /* TODO */

   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   GLTEXSUBIMAGE2D_ASYNC_PREPARE; /* TODO */

finish:
   GLTEXSUBIMAGE2D_ENQUEUE_BEFORE; /* TODO */

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glTexSubImage2D,
                              thread_data,
                              thread_mode);

   GLTEXSUBIMAGE2D_ENQUEUE_AFTER; /* TODO */
}

/*
 * void
 * glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
 */

typedef struct
{
   GLenum target;
   GLint level;
   GLenum internalformat;
   GLsizei width;
   GLsizei height;
   GLint border;
   GLsizei imageSize;
   const void *data;
   int command_allocated;
   GLCOMPRESSEDTEXIMAGE2D_VARIABLE_DECLARE /* TODO */

} Evas_Thread_Command_glCompressedTexImage2D;

static void
_gl_thread_glCompressedTexImage2D(void *data)
{
   Evas_Thread_Command_glCompressedTexImage2D *thread_data =
      (Evas_Thread_Command_glCompressedTexImage2D *)data;


   GLCOMPRESSEDTEXIMAGE2D_GLCALL_BEFORE; /* TODO */

   glCompressedTexImage2D(thread_data->target,
                          thread_data->level,
                          thread_data->internalformat,
                          thread_data->width,
                          thread_data->height,
                          thread_data->border,
                          thread_data->imageSize,
                          thread_data->data);

   GLCOMPRESSEDTEXIMAGE2D_GLCALL_AFTER; /* TODO */

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glCompressedTexImage2D_th(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glCompressedTexImage2D thread_data_local;
   Evas_Thread_Command_glCompressedTexImage2D *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glCompressedTexImage2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glCompressedTexImage2D));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->level = level;
   thread_data->internalformat = internalformat;
   thread_data->width = width;
   thread_data->height = height;
   thread_data->border = border;
   thread_data->imageSize = imageSize;
   thread_data->data = data;

   GLCOMPRESSEDTEXIMAGE2D_VARIABLE_INIT; /* TODO */

   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   GLCOMPRESSEDTEXIMAGE2D_ASYNC_PREPARE; /* TODO */

finish:
   GLCOMPRESSEDTEXIMAGE2D_ENQUEUE_BEFORE; /* TODO */

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glCompressedTexImage2D,
                              thread_data,
                              thread_mode);

   GLCOMPRESSEDTEXIMAGE2D_ENQUEUE_AFTER; /* TODO */
}

/*
 * void
 * glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
 */

typedef struct
{
   GLenum target;
   GLint level;
   GLint xoffset;
   GLint yoffset;
   GLsizei width;
   GLsizei height;
   GLenum format;
   GLsizei imageSize;
   const void *data;
   int command_allocated;
   GLCOMPRESSEDTEXSUBIMAGE2D_VARIABLE_DECLARE /* TODO */

} Evas_Thread_Command_glCompressedTexSubImage2D;

static void
_gl_thread_glCompressedTexSubImage2D(void *data)
{
   Evas_Thread_Command_glCompressedTexSubImage2D *thread_data =
      (Evas_Thread_Command_glCompressedTexSubImage2D *)data;


   GLCOMPRESSEDTEXSUBIMAGE2D_GLCALL_BEFORE; /* TODO */

   glCompressedTexSubImage2D(thread_data->target,
                             thread_data->level,
                             thread_data->xoffset,
                             thread_data->yoffset,
                             thread_data->width,
                             thread_data->height,
                             thread_data->format,
                             thread_data->imageSize,
                             thread_data->data);

   GLCOMPRESSEDTEXSUBIMAGE2D_GLCALL_AFTER; /* TODO */

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glCompressedTexSubImage2D_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glCompressedTexSubImage2D thread_data_local;
   Evas_Thread_Command_glCompressedTexSubImage2D *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glCompressedTexSubImage2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glCompressedTexSubImage2D));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->level = level;
   thread_data->xoffset = xoffset;
   thread_data->yoffset = yoffset;
   thread_data->width = width;
   thread_data->height = height;
   thread_data->format = format;
   thread_data->imageSize = imageSize;
   thread_data->data = data;

   GLCOMPRESSEDTEXSUBIMAGE2D_VARIABLE_INIT; /* TODO */

   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   GLCOMPRESSEDTEXSUBIMAGE2D_ASYNC_PREPARE; /* TODO */

finish:
   GLCOMPRESSEDTEXSUBIMAGE2D_ENQUEUE_BEFORE; /* TODO */

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glCompressedTexSubImage2D,
                              thread_data,
                              thread_mode);

   GLCOMPRESSEDTEXSUBIMAGE2D_ENQUEUE_AFTER; /* TODO */
}

/*
 * void
 * glTexParameterf(GLenum target, GLenum pname, GLfloat param);
 */

typedef struct
{
   GLenum target;
   GLenum pname;
   GLfloat param;
   int command_allocated;

} Evas_Thread_Command_glTexParameterf;

static void
_gl_thread_glTexParameterf(void *data)
{
   Evas_Thread_Command_glTexParameterf *thread_data =
      (Evas_Thread_Command_glTexParameterf *)data;

   glTexParameterf(thread_data->target,
                   thread_data->pname,
                   thread_data->param);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexParameterf_th(GLenum target, GLenum pname, GLfloat param)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glTexParameterf thread_data_local;
   Evas_Thread_Command_glTexParameterf *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glTexParameterf(target, pname, param);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glTexParameterf *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glTexParameterf));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->param = param;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glTexParameterf,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
 */

typedef struct
{
   GLenum target;
   GLenum pname;
   const GLfloat *params;
   void *params_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glTexParameterfv;

static void
_gl_thread_glTexParameterfv(void *data)
{
   Evas_Thread_Command_glTexParameterfv *thread_data =
      (Evas_Thread_Command_glTexParameterfv *)data;

   glTexParameterfv(thread_data->target,
                    thread_data->pname,
                    thread_data->params);


   if (thread_data->params_copied)
     eina_mempool_free(_mp_default, thread_data->params_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexParameterfv_th(GLenum target, GLenum pname, const GLfloat *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glTexParameterfv thread_data_local;
   Evas_Thread_Command_glTexParameterfv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glTexParameterfv(target, pname, params);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glTexParameterfv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glTexParameterfv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->params = params;

   thread_data->params_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (params)
     {
        /* 1. check memory size */
        unsigned int copy_size = sizeof(GLfloat);
        if (copy_size > _mp_default_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->params_copied = eina_mempool_malloc(_mp_default, copy_size);
        if (thread_data->params_copied)
          {
             memcpy(thread_data->params_copied, params, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->params = (const GLfloat  *)thread_data->params_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glTexParameterfv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glTexParameteri(GLenum target, GLenum pname, GLint param);
 */

typedef struct
{
   GLenum target;
   GLenum pname;
   GLint param;
   int command_allocated;

} Evas_Thread_Command_glTexParameteri;

static void
_gl_thread_glTexParameteri(void *data)
{
   Evas_Thread_Command_glTexParameteri *thread_data =
      (Evas_Thread_Command_glTexParameteri *)data;

   glTexParameteri(thread_data->target,
                   thread_data->pname,
                   thread_data->param);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexParameteri_th(GLenum target, GLenum pname, GLint param)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glTexParameteri thread_data_local;
   Evas_Thread_Command_glTexParameteri *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glTexParameteri(target, pname, param);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glTexParameteri *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glTexParameteri));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->param = param;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glTexParameteri,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
 */

typedef struct
{
   GLenum target;
   GLenum pname;
   const GLint *params;
   void *params_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glTexParameteriv;

static void
_gl_thread_glTexParameteriv(void *data)
{
   Evas_Thread_Command_glTexParameteriv *thread_data =
      (Evas_Thread_Command_glTexParameteriv *)data;

   glTexParameteriv(thread_data->target,
                    thread_data->pname,
                    thread_data->params);


   if (thread_data->params_copied)
     eina_mempool_free(_mp_default, thread_data->params_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexParameteriv_th(GLenum target, GLenum pname, const GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glTexParameteriv thread_data_local;
   Evas_Thread_Command_glTexParameteriv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glTexParameteriv(target, pname, params);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glTexParameteriv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glTexParameteriv));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->params = params;

   thread_data->params_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (params)
     {
        /* 1. check memory size */
        unsigned int copy_size = sizeof(GLint);
        if (copy_size > _mp_default_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->params_copied = eina_mempool_malloc(_mp_default, copy_size);
        if (thread_data->params_copied)
          {
             memcpy(thread_data->params_copied, params, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->params = (const GLint  *)thread_data->params_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glTexParameteriv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
 */

typedef struct
{
   GLint x;
   GLint y;
   GLsizei width;
   GLsizei height;
   int command_allocated;

} Evas_Thread_Command_glScissor;

static void
_gl_thread_glScissor(void *data)
{
   Evas_Thread_Command_glScissor *thread_data =
      (Evas_Thread_Command_glScissor *)data;

   glScissor(thread_data->x,
             thread_data->y,
             thread_data->width,
             thread_data->height);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glScissor_th(GLint x, GLint y, GLsizei width, GLsizei height)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glScissor thread_data_local;
   Evas_Thread_Command_glScissor *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glScissor(x, y, width, height);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glScissor *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glScissor));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->x = x;
   thread_data->y = y;
   thread_data->width = width;
   thread_data->height = height;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glScissor,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glBlendFunc(GLenum sfactor, GLenum dfactor);
 */

typedef struct
{
   GLenum sfactor;
   GLenum dfactor;
   int command_allocated;

} Evas_Thread_Command_glBlendFunc;

static void
_gl_thread_glBlendFunc(void *data)
{
   Evas_Thread_Command_glBlendFunc *thread_data =
      (Evas_Thread_Command_glBlendFunc *)data;

   glBlendFunc(thread_data->sfactor,
               thread_data->dfactor);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBlendFunc_th(GLenum sfactor, GLenum dfactor)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glBlendFunc thread_data_local;
   Evas_Thread_Command_glBlendFunc *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glBlendFunc(sfactor, dfactor);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glBlendFunc *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glBlendFunc));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->sfactor = sfactor;
   thread_data->dfactor = dfactor;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glBlendFunc,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
 */

typedef struct
{
   GLfloat red;
   GLfloat green;
   GLfloat blue;
   GLfloat alpha;
   int command_allocated;

} Evas_Thread_Command_glBlendColor;

static void
_gl_thread_glBlendColor(void *data)
{
   Evas_Thread_Command_glBlendColor *thread_data =
      (Evas_Thread_Command_glBlendColor *)data;

   glBlendColor(thread_data->red,
                thread_data->green,
                thread_data->blue,
                thread_data->alpha);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBlendColor_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glBlendColor thread_data_local;
   Evas_Thread_Command_glBlendColor *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glBlendColor(red, green, blue, alpha);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glBlendColor *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glBlendColor));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->red = red;
   thread_data->green = green;
   thread_data->blue = blue;
   thread_data->alpha = alpha;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glBlendColor,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDepthMask(GLboolean flag);
 */

typedef struct
{
   GLboolean flag;
   int command_allocated;

} Evas_Thread_Command_glDepthMask;

static void
_gl_thread_glDepthMask(void *data)
{
   Evas_Thread_Command_glDepthMask *thread_data =
      (Evas_Thread_Command_glDepthMask *)data;

   glDepthMask(thread_data->flag);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDepthMask_th(GLboolean flag)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDepthMask thread_data_local;
   Evas_Thread_Command_glDepthMask *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDepthMask(flag);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDepthMask *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDepthMask));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->flag = flag;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDepthMask,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glClear(GLbitfield mask);
 */

typedef struct
{
   GLbitfield mask;
   int command_allocated;

} Evas_Thread_Command_glClear;

static void
_gl_thread_glClear(void *data)
{
   Evas_Thread_Command_glClear *thread_data =
      (Evas_Thread_Command_glClear *)data;

   glClear(thread_data->mask);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glClear_th(GLbitfield mask)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glClear thread_data_local;
   Evas_Thread_Command_glClear *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glClear(mask);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glClear *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glClear));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->mask = mask;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glClear,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
 */

typedef struct
{
   GLfloat red;
   GLfloat green;
   GLfloat blue;
   GLfloat alpha;
   int command_allocated;

} Evas_Thread_Command_glClearColor;

static void
_gl_thread_glClearColor(void *data)
{
   Evas_Thread_Command_glClearColor *thread_data =
      (Evas_Thread_Command_glClearColor *)data;

   glClearColor(thread_data->red,
                thread_data->green,
                thread_data->blue,
                thread_data->alpha);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glClearColor_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glClearColor thread_data_local;
   Evas_Thread_Command_glClearColor *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glClearColor(red, green, blue, alpha);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glClearColor *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glClearColor));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->red = red;
   thread_data->green = green;
   thread_data->blue = blue;
   thread_data->alpha = alpha;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glClearColor,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
 */

typedef struct
{
   GLint x;
   GLint y;
   GLsizei width;
   GLsizei height;
   GLenum format;
   GLenum type;
   void *pixels;

} Evas_Thread_Command_glReadPixels;

static void
_gl_thread_glReadPixels(void *data)
{
   Evas_Thread_Command_glReadPixels *thread_data =
      (Evas_Thread_Command_glReadPixels *)data;

   glReadPixels(thread_data->x,
                thread_data->y,
                thread_data->width,
                thread_data->height,
                thread_data->format,
                thread_data->type,
                thread_data->pixels);

}

EAPI void
evas_glReadPixels_th(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glReadPixels thread_data_local;
   Evas_Thread_Command_glReadPixels *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glReadPixels(x, y, width, height, format, type, pixels);
        return;
     }

   thread_data->x = x;
   thread_data->y = y;
   thread_data->width = width;
   thread_data->height = height;
   thread_data->format = format;
   thread_data->type = type;
   thread_data->pixels = pixels;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glReadPixels,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGenFramebuffers(GLsizei n, GLuint *framebuffers);
 */

typedef struct
{
   GLsizei n;
   GLuint *framebuffers;

} Evas_Thread_Command_glGenFramebuffers;

void (*orig_evas_glGenFramebuffers)(GLsizei n, GLuint *framebuffers);

void
glGenFramebuffers_orig_evas_set(void *func)
{
   orig_evas_glGenFramebuffers = func;
}

void *
glGenFramebuffers_orig_evas_get(void)
{
   return orig_evas_glGenFramebuffers;
}

static void
_gl_thread_glGenFramebuffers(void *data)
{
   Evas_Thread_Command_glGenFramebuffers *thread_data =
      (Evas_Thread_Command_glGenFramebuffers *)data;

   orig_evas_glGenFramebuffers(thread_data->n,
                               thread_data->framebuffers);

}

EAPI void
evas_glGenFramebuffers_th(GLsizei n, GLuint *framebuffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGenFramebuffers thread_data_local;
   Evas_Thread_Command_glGenFramebuffers *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glGenFramebuffers(n, framebuffers);
        return;
     }

   thread_data->n = n;
   thread_data->framebuffers = framebuffers;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGenFramebuffers,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glBindFramebuffer(GLenum target, GLuint framebuffer);
 */

typedef struct
{
   GLenum target;
   GLuint framebuffer;
   int command_allocated;

} Evas_Thread_Command_glBindFramebuffer;

void (*orig_evas_glBindFramebuffer)(GLenum target, GLuint framebuffer);

void
glBindFramebuffer_orig_evas_set(void *func)
{
   orig_evas_glBindFramebuffer = func;
}

void *
glBindFramebuffer_orig_evas_get(void)
{
   return orig_evas_glBindFramebuffer;
}

static void
_gl_thread_glBindFramebuffer(void *data)
{
   Evas_Thread_Command_glBindFramebuffer *thread_data =
      (Evas_Thread_Command_glBindFramebuffer *)data;

   orig_evas_glBindFramebuffer(thread_data->target,
                               thread_data->framebuffer);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBindFramebuffer_th(GLenum target, GLuint framebuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glBindFramebuffer thread_data_local;
   Evas_Thread_Command_glBindFramebuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glBindFramebuffer(target, framebuffer);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glBindFramebuffer *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glBindFramebuffer));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->framebuffer = framebuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glBindFramebuffer,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);
 */

typedef struct
{
   GLsizei n;
   const GLuint *framebuffers;
   void *framebuffers_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glDeleteFramebuffers;

void (*orig_evas_glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers);

void
glDeleteFramebuffers_orig_evas_set(void *func)
{
   orig_evas_glDeleteFramebuffers = func;
}

void *
glDeleteFramebuffers_orig_evas_get(void)
{
   return orig_evas_glDeleteFramebuffers;
}

static void
_gl_thread_glDeleteFramebuffers(void *data)
{
   Evas_Thread_Command_glDeleteFramebuffers *thread_data =
      (Evas_Thread_Command_glDeleteFramebuffers *)data;

   orig_evas_glDeleteFramebuffers(thread_data->n,
                                  thread_data->framebuffers);


   if (thread_data->framebuffers_copied)
     eina_mempool_free(_mp_delete_object, thread_data->framebuffers_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteFramebuffers_th(GLsizei n, const GLuint *framebuffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDeleteFramebuffers thread_data_local;
   Evas_Thread_Command_glDeleteFramebuffers *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glDeleteFramebuffers(n, framebuffers);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDeleteFramebuffers *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDeleteFramebuffers));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->n = n;
   thread_data->framebuffers = framebuffers;

   thread_data->framebuffers_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (framebuffers)
     {
        /* 1. check memory size */
        unsigned int copy_size = n * sizeof(GLuint);
        if (copy_size > _mp_delete_object_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->framebuffers_copied = eina_mempool_malloc(_mp_delete_object, copy_size);
        if (thread_data->framebuffers_copied)
          {
             memcpy(thread_data->framebuffers_copied, framebuffers, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->framebuffers = (const GLuint  *)thread_data->framebuffers_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDeleteFramebuffers,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
 */

typedef struct
{
   GLsizei n;
   GLuint *renderbuffers;

} Evas_Thread_Command_glGenRenderbuffers;

static void
_gl_thread_glGenRenderbuffers(void *data)
{
   Evas_Thread_Command_glGenRenderbuffers *thread_data =
      (Evas_Thread_Command_glGenRenderbuffers *)data;

   glGenRenderbuffers(thread_data->n,
                      thread_data->renderbuffers);

}

EAPI void
evas_glGenRenderbuffers_th(GLsizei n, GLuint *renderbuffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGenRenderbuffers thread_data_local;
   Evas_Thread_Command_glGenRenderbuffers *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGenRenderbuffers(n, renderbuffers);
        return;
     }

   thread_data->n = n;
   thread_data->renderbuffers = renderbuffers;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGenRenderbuffers,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glBindRenderbuffer(GLenum target, GLuint renderbuffer);
 */

typedef struct
{
   GLenum target;
   GLuint renderbuffer;
   int command_allocated;

} Evas_Thread_Command_glBindRenderbuffer;

static void
_gl_thread_glBindRenderbuffer(void *data)
{
   Evas_Thread_Command_glBindRenderbuffer *thread_data =
      (Evas_Thread_Command_glBindRenderbuffer *)data;

   glBindRenderbuffer(thread_data->target,
                      thread_data->renderbuffer);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBindRenderbuffer_th(GLenum target, GLuint renderbuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glBindRenderbuffer thread_data_local;
   Evas_Thread_Command_glBindRenderbuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glBindRenderbuffer(target, renderbuffer);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glBindRenderbuffer *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glBindRenderbuffer));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->renderbuffer = renderbuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glBindRenderbuffer,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);
 */

typedef struct
{
   GLsizei n;
   const GLuint *renderbuffers;
   void *renderbuffers_copied; /* COPIED */
   int command_allocated;

} Evas_Thread_Command_glDeleteRenderbuffers;

static void
_gl_thread_glDeleteRenderbuffers(void *data)
{
   Evas_Thread_Command_glDeleteRenderbuffers *thread_data =
      (Evas_Thread_Command_glDeleteRenderbuffers *)data;

   glDeleteRenderbuffers(thread_data->n,
                         thread_data->renderbuffers);


   if (thread_data->renderbuffers_copied)
     eina_mempool_free(_mp_delete_object, thread_data->renderbuffers_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteRenderbuffers_th(GLsizei n, const GLuint *renderbuffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glDeleteRenderbuffers thread_data_local;
   Evas_Thread_Command_glDeleteRenderbuffers *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glDeleteRenderbuffers(n, renderbuffers);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glDeleteRenderbuffers *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glDeleteRenderbuffers));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->n = n;
   thread_data->renderbuffers = renderbuffers;

   thread_data->renderbuffers_copied = NULL;
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   /* copy variable */
   if (renderbuffers)
     {
        /* 1. check memory size */
        unsigned int copy_size = n * sizeof(GLuint);
        if (copy_size > _mp_delete_object_memory_size)
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 2. malloc & copy */
        thread_data->renderbuffers_copied = eina_mempool_malloc(_mp_delete_object, copy_size);
        if (thread_data->renderbuffers_copied)
          {
             memcpy(thread_data->renderbuffers_copied, renderbuffers, copy_size);
          }
        else
          {
             thread_mode = EVAS_GL_THREAD_MODE_FINISH;
             goto finish;
          }
        /* 3. replace */
        thread_data->renderbuffers = (const GLuint  *)thread_data->renderbuffers_copied;
     }
   /* end of copy variable */

finish:
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glDeleteRenderbuffers,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
 */

typedef struct
{
   GLenum target;
   GLenum internalformat;
   GLsizei width;
   GLsizei height;
   int command_allocated;

} Evas_Thread_Command_glRenderbufferStorage;

static void
_gl_thread_glRenderbufferStorage(void *data)
{
   Evas_Thread_Command_glRenderbufferStorage *thread_data =
      (Evas_Thread_Command_glRenderbufferStorage *)data;

   glRenderbufferStorage(thread_data->target,
                         thread_data->internalformat,
                         thread_data->width,
                         thread_data->height);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glRenderbufferStorage_th(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glRenderbufferStorage thread_data_local;
   Evas_Thread_Command_glRenderbufferStorage *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glRenderbufferStorage(target, internalformat, width, height);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glRenderbufferStorage *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glRenderbufferStorage));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->internalformat = internalformat;
   thread_data->width = width;
   thread_data->height = height;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glRenderbufferStorage,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
 */

typedef struct
{
   GLenum target;
   GLenum attachment;
   GLenum renderbuffertarget;
   GLuint renderbuffer;
   int command_allocated;

} Evas_Thread_Command_glFramebufferRenderbuffer;

static void
_gl_thread_glFramebufferRenderbuffer(void *data)
{
   Evas_Thread_Command_glFramebufferRenderbuffer *thread_data =
      (Evas_Thread_Command_glFramebufferRenderbuffer *)data;

   glFramebufferRenderbuffer(thread_data->target,
                             thread_data->attachment,
                             thread_data->renderbuffertarget,
                             thread_data->renderbuffer);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glFramebufferRenderbuffer_th(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glFramebufferRenderbuffer thread_data_local;
   Evas_Thread_Command_glFramebufferRenderbuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glFramebufferRenderbuffer *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glFramebufferRenderbuffer));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->attachment = attachment;
   thread_data->renderbuffertarget = renderbuffertarget;
   thread_data->renderbuffer = renderbuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glFramebufferRenderbuffer,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
 */

typedef struct
{
   GLenum target;
   GLenum attachment;
   GLenum textarget;
   GLuint texture;
   GLint level;
   int command_allocated;

} Evas_Thread_Command_glFramebufferTexture2D;

void (*orig_evas_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

void
glFramebufferTexture2D_orig_evas_set(void *func)
{
   orig_evas_glFramebufferTexture2D = func;
}

void *
glFramebufferTexture2D_orig_evas_get(void)
{
   return orig_evas_glFramebufferTexture2D;
}

static void
_gl_thread_glFramebufferTexture2D(void *data)
{
   Evas_Thread_Command_glFramebufferTexture2D *thread_data =
      (Evas_Thread_Command_glFramebufferTexture2D *)data;

   orig_evas_glFramebufferTexture2D(thread_data->target,
                                    thread_data->attachment,
                                    thread_data->textarget,
                                    thread_data->texture,
                                    thread_data->level);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glFramebufferTexture2D_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glFramebufferTexture2D thread_data_local;
   Evas_Thread_Command_glFramebufferTexture2D *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glFramebufferTexture2D(target, attachment, textarget, texture, level);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glFramebufferTexture2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glFramebufferTexture2D));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->attachment = attachment;
   thread_data->textarget = textarget;
   thread_data->texture = texture;
   thread_data->level = level;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glFramebufferTexture2D,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glFramebufferTexture2DMultisample(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
 */

typedef struct
{
   GLenum target;
   GLenum attachment;
   GLenum textarget;
   GLuint texture;
   GLint level;
   GLsizei samples;
   int command_allocated;

} Evas_Thread_Command_glFramebufferTexture2DMultisample;

void (*orig_evas_glFramebufferTexture2DMultisample)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);

void
glFramebufferTexture2DMultisample_orig_evas_set(void *func)
{
   orig_evas_glFramebufferTexture2DMultisample = func;
}

void *
glFramebufferTexture2DMultisample_orig_evas_get(void)
{
   return orig_evas_glFramebufferTexture2DMultisample;
}

static void
_gl_thread_glFramebufferTexture2DMultisample(void *data)
{
   Evas_Thread_Command_glFramebufferTexture2DMultisample *thread_data =
      (Evas_Thread_Command_glFramebufferTexture2DMultisample *)data;

   orig_evas_glFramebufferTexture2DMultisample(thread_data->target,
                                               thread_data->attachment,
                                               thread_data->textarget,
                                               thread_data->texture,
                                               thread_data->level,
                                               thread_data->samples);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glFramebufferTexture2DMultisample_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glFramebufferTexture2DMultisample thread_data_local;
   Evas_Thread_Command_glFramebufferTexture2DMultisample *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glFramebufferTexture2DMultisample(target, attachment, textarget, texture, level, samples);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glFramebufferTexture2DMultisample *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glFramebufferTexture2DMultisample));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->attachment = attachment;
   thread_data->textarget = textarget;
   thread_data->texture = texture;
   thread_data->level = level;
   thread_data->samples = samples;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glFramebufferTexture2DMultisample,
                              thread_data,
                              thread_mode);
}

/*
 * GLenum
 * glCheckFramebufferStatus(GLenum target);
 */

typedef struct
{
   GLenum return_value;
   GLenum target;

} Evas_Thread_Command_glCheckFramebufferStatus;

static void
_gl_thread_glCheckFramebufferStatus(void *data)
{
   Evas_Thread_Command_glCheckFramebufferStatus *thread_data =
      (Evas_Thread_Command_glCheckFramebufferStatus *)data;

   thread_data->return_value = glCheckFramebufferStatus(thread_data->target);

}

EAPI GLenum
evas_glCheckFramebufferStatus_th(GLenum target)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glCheckFramebufferStatus thread_data_local;
   Evas_Thread_Command_glCheckFramebufferStatus *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glCheckFramebufferStatus(target);
     }

   thread_data->target = target;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glCheckFramebufferStatus,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glFlush(void);
 */

static void
_gl_thread_glFlush(void *data EINA_UNUSED)
{
   glFlush();

}

EAPI void
evas_glFlush_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   if (!evas_gl_thread_enabled())
     {
        glFlush();
        return;
     }

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glFlush,
                              NULL,
                              thread_mode);
}

/*
 * void
 * glFinish(void);
 */

static void
_gl_thread_glFinish(void *data EINA_UNUSED)
{
   glFinish();

}

EAPI void
evas_glFinish_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   if (!evas_gl_thread_enabled())
     {
        glFinish();
        return;
     }

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glFinish,
                              NULL,
                              thread_mode);
}

/*
 * void
 * glHint(GLenum target, GLenum mode);
 */

typedef struct
{
   GLenum target;
   GLenum mode;
   int command_allocated;

} Evas_Thread_Command_glHint;

static void
_gl_thread_glHint(void *data)
{
   Evas_Thread_Command_glHint *thread_data =
      (Evas_Thread_Command_glHint *)data;

   glHint(thread_data->target,
          thread_data->mode);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glHint_th(GLenum target, GLenum mode)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glHint thread_data_local;
   Evas_Thread_Command_glHint *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glHint(target, mode);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glHint *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glHint));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->mode = mode;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glHint,
                              thread_data,
                              thread_mode);
}

/*
 * const GLubyte *
 * glGetString(GLenum name);
 */

typedef struct
{
   const GLubyte * return_value;
   GLenum name;

} Evas_Thread_Command_glGetString;

static void
_gl_thread_glGetString(void *data)
{
   Evas_Thread_Command_glGetString *thread_data =
      (Evas_Thread_Command_glGetString *)data;

   thread_data->return_value = glGetString(thread_data->name);

}

EAPI const GLubyte *
evas_glGetString_th(GLenum name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetString thread_data_local;
   Evas_Thread_Command_glGetString *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glGetString(name);
     }

   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetString,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glGetBooleanv(GLenum pname, GLboolean *data);
 */

typedef struct
{
   GLenum pname;
   GLboolean *data;

} Evas_Thread_Command_glGetBooleanv;

static void
_gl_thread_glGetBooleanv(void *data)
{
   Evas_Thread_Command_glGetBooleanv *thread_data =
      (Evas_Thread_Command_glGetBooleanv *)data;

   glGetBooleanv(thread_data->pname,
                 thread_data->data);

}

EAPI void
evas_glGetBooleanv_th(GLenum pname, GLboolean *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetBooleanv thread_data_local;
   Evas_Thread_Command_glGetBooleanv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetBooleanv(pname, data);
        return;
     }

   thread_data->pname = pname;
   thread_data->data = data;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetBooleanv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetFloatv(GLenum pname, GLfloat *data);
 */

typedef struct
{
   GLenum pname;
   GLfloat *data;

} Evas_Thread_Command_glGetFloatv;

static void
_gl_thread_glGetFloatv(void *data)
{
   Evas_Thread_Command_glGetFloatv *thread_data =
      (Evas_Thread_Command_glGetFloatv *)data;

   glGetFloatv(thread_data->pname,
               thread_data->data);

}

EAPI void
evas_glGetFloatv_th(GLenum pname, GLfloat *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetFloatv thread_data_local;
   Evas_Thread_Command_glGetFloatv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetFloatv(pname, data);
        return;
     }

   thread_data->pname = pname;
   thread_data->data = data;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetFloatv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetIntegerv(GLenum pname, GLint *data);
 */

typedef struct
{
   GLenum pname;
   GLint *data;

} Evas_Thread_Command_glGetIntegerv;

static void
_gl_thread_glGetIntegerv(void *data)
{
   Evas_Thread_Command_glGetIntegerv *thread_data =
      (Evas_Thread_Command_glGetIntegerv *)data;

   glGetIntegerv(thread_data->pname,
                 thread_data->data);

}

EAPI void
evas_glGetIntegerv_th(GLenum pname, GLint *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetIntegerv thread_data_local;
   Evas_Thread_Command_glGetIntegerv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetIntegerv(pname, data);
        return;
     }

   thread_data->pname = pname;
   thread_data->data = data;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetIntegerv,
                              thread_data,
                              thread_mode);
}

/*
 * GLboolean
 * glIsBuffer(GLint buffer);
 */

typedef struct
{
   GLboolean return_value;
   GLint buffer;

} Evas_Thread_Command_glIsBuffer;

static void
_gl_thread_glIsBuffer(void *data)
{
   Evas_Thread_Command_glIsBuffer *thread_data =
      (Evas_Thread_Command_glIsBuffer *)data;

   thread_data->return_value = glIsBuffer(thread_data->buffer);

}

EAPI GLboolean
evas_glIsBuffer_th(GLint buffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glIsBuffer thread_data_local;
   Evas_Thread_Command_glIsBuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glIsBuffer(buffer);
     }

   thread_data->buffer = buffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glIsBuffer,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);
 */

typedef struct
{
   GLenum target;
   GLenum pname;
   GLint *params;

} Evas_Thread_Command_glGetBufferParameteriv;

static void
_gl_thread_glGetBufferParameteriv(void *data)
{
   Evas_Thread_Command_glGetBufferParameteriv *thread_data =
      (Evas_Thread_Command_glGetBufferParameteriv *)data;

   glGetBufferParameteriv(thread_data->target,
                          thread_data->pname,
                          thread_data->params);

}

EAPI void
evas_glGetBufferParameteriv_th(GLenum target, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetBufferParameteriv thread_data_local;
   Evas_Thread_Command_glGetBufferParameteriv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetBufferParameteriv(target, pname, params);
        return;
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetBufferParameteriv,
                              thread_data,
                              thread_mode);
}

/*
 * GLboolean
 * glIsShader(GLuint shader);
 */

typedef struct
{
   GLboolean return_value;
   GLuint shader;

} Evas_Thread_Command_glIsShader;

static void
_gl_thread_glIsShader(void *data)
{
   Evas_Thread_Command_glIsShader *thread_data =
      (Evas_Thread_Command_glIsShader *)data;

   thread_data->return_value = glIsShader(thread_data->shader);

}

EAPI GLboolean
evas_glIsShader_th(GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glIsShader thread_data_local;
   Evas_Thread_Command_glIsShader *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glIsShader(shader);
     }

   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glIsShader,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
 */

typedef struct
{
   GLuint shader;
   GLenum pname;
   GLint *params;

} Evas_Thread_Command_glGetShaderiv;

static void
_gl_thread_glGetShaderiv(void *data)
{
   Evas_Thread_Command_glGetShaderiv *thread_data =
      (Evas_Thread_Command_glGetShaderiv *)data;

   glGetShaderiv(thread_data->shader,
                 thread_data->pname,
                 thread_data->params);

}

EAPI void
evas_glGetShaderiv_th(GLuint shader, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetShaderiv thread_data_local;
   Evas_Thread_Command_glGetShaderiv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetShaderiv(shader, pname, params);
        return;
     }

   thread_data->shader = shader;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetShaderiv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
 */

typedef struct
{
   GLuint program;
   GLsizei maxCount;
   GLsizei *count;
   GLuint *shaders;

} Evas_Thread_Command_glGetAttachedShaders;

static void
_gl_thread_glGetAttachedShaders(void *data)
{
   Evas_Thread_Command_glGetAttachedShaders *thread_data =
      (Evas_Thread_Command_glGetAttachedShaders *)data;

   glGetAttachedShaders(thread_data->program,
                        thread_data->maxCount,
                        thread_data->count,
                        thread_data->shaders);

}

EAPI void
evas_glGetAttachedShaders_th(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetAttachedShaders thread_data_local;
   Evas_Thread_Command_glGetAttachedShaders *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetAttachedShaders(program, maxCount, count, shaders);
        return;
     }

   thread_data->program = program;
   thread_data->maxCount = maxCount;
   thread_data->count = count;
   thread_data->shaders = shaders;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetAttachedShaders,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
 */

typedef struct
{
   GLuint shader;
   GLsizei bufSize;
   GLsizei *length;
   GLchar *infoLog;

} Evas_Thread_Command_glGetShaderInfoLog;

static void
_gl_thread_glGetShaderInfoLog(void *data)
{
   Evas_Thread_Command_glGetShaderInfoLog *thread_data =
      (Evas_Thread_Command_glGetShaderInfoLog *)data;

   glGetShaderInfoLog(thread_data->shader,
                      thread_data->bufSize,
                      thread_data->length,
                      thread_data->infoLog);

}

EAPI void
evas_glGetShaderInfoLog_th(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetShaderInfoLog thread_data_local;
   Evas_Thread_Command_glGetShaderInfoLog *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetShaderInfoLog(shader, bufSize, length, infoLog);
        return;
     }

   thread_data->shader = shader;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->infoLog = infoLog;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetShaderInfoLog,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
 */

typedef struct
{
   GLuint shader;
   GLsizei bufSize;
   GLsizei *length;
   GLchar *source;

} Evas_Thread_Command_glGetShaderSource;

static void
_gl_thread_glGetShaderSource(void *data)
{
   Evas_Thread_Command_glGetShaderSource *thread_data =
      (Evas_Thread_Command_glGetShaderSource *)data;

   glGetShaderSource(thread_data->shader,
                     thread_data->bufSize,
                     thread_data->length,
                     thread_data->source);

}

EAPI void
evas_glGetShaderSource_th(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetShaderSource thread_data_local;
   Evas_Thread_Command_glGetShaderSource *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetShaderSource(shader, bufSize, length, source);
        return;
     }

   thread_data->shader = shader;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->source = source;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetShaderSource,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
 */

typedef struct
{
   GLenum shadertype;
   GLenum precisiontype;
   GLint *range;
   GLint *precision;

} Evas_Thread_Command_glGetShaderPrecisionFormat;

static void
_gl_thread_glGetShaderPrecisionFormat(void *data)
{
   Evas_Thread_Command_glGetShaderPrecisionFormat *thread_data =
      (Evas_Thread_Command_glGetShaderPrecisionFormat *)data;

   glGetShaderPrecisionFormat(thread_data->shadertype,
                              thread_data->precisiontype,
                              thread_data->range,
                              thread_data->precision);

}

EAPI void
evas_glGetShaderPrecisionFormat_th(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetShaderPrecisionFormat thread_data_local;
   Evas_Thread_Command_glGetShaderPrecisionFormat *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
        return;
     }

   thread_data->shadertype = shadertype;
   thread_data->precisiontype = precisiontype;
   thread_data->range = range;
   thread_data->precision = precision;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetShaderPrecisionFormat,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params);
 */

typedef struct
{
   GLuint index;
   GLenum pname;
   GLfloat *params;

} Evas_Thread_Command_glGetVertexAttribfv;

static void
_gl_thread_glGetVertexAttribfv(void *data)
{
   Evas_Thread_Command_glGetVertexAttribfv *thread_data =
      (Evas_Thread_Command_glGetVertexAttribfv *)data;

   glGetVertexAttribfv(thread_data->index,
                       thread_data->pname,
                       thread_data->params);

}

EAPI void
evas_glGetVertexAttribfv_th(GLuint index, GLenum pname, GLfloat *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetVertexAttribfv thread_data_local;
   Evas_Thread_Command_glGetVertexAttribfv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetVertexAttribfv(index, pname, params);
        return;
     }

   thread_data->index = index;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetVertexAttribfv,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params);
 */

typedef struct
{
   GLuint index;
   GLenum pname;
   GLint *params;

} Evas_Thread_Command_glGetVertexAttribiv;

static void
_gl_thread_glGetVertexAttribiv(void *data)
{
   Evas_Thread_Command_glGetVertexAttribiv *thread_data =
      (Evas_Thread_Command_glGetVertexAttribiv *)data;

   glGetVertexAttribiv(thread_data->index,
                       thread_data->pname,
                       thread_data->params);

}

EAPI void
evas_glGetVertexAttribiv_th(GLuint index, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetVertexAttribiv thread_data_local;
   Evas_Thread_Command_glGetVertexAttribiv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetVertexAttribiv(index, pname, params);
        return;
     }

   thread_data->index = index;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetVertexAttribiv,
                              thread_data,
                              thread_mode);
}

/*
 * GLboolean
 * glIsProgram(GLuint program);
 */

typedef struct
{
   GLboolean return_value;
   GLuint program;

} Evas_Thread_Command_glIsProgram;

static void
_gl_thread_glIsProgram(void *data)
{
   Evas_Thread_Command_glIsProgram *thread_data =
      (Evas_Thread_Command_glIsProgram *)data;

   thread_data->return_value = glIsProgram(thread_data->program);

}

EAPI GLboolean
evas_glIsProgram_th(GLuint program)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glIsProgram thread_data_local;
   Evas_Thread_Command_glIsProgram *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glIsProgram(program);
     }

   thread_data->program = program;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glIsProgram,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
 */

typedef struct
{
   GLuint program;
   GLsizei bufSize;
   GLsizei *length;
   GLchar *infoLog;

} Evas_Thread_Command_glGetProgramInfoLog;

static void
_gl_thread_glGetProgramInfoLog(void *data)
{
   Evas_Thread_Command_glGetProgramInfoLog *thread_data =
      (Evas_Thread_Command_glGetProgramInfoLog *)data;

   glGetProgramInfoLog(thread_data->program,
                       thread_data->bufSize,
                       thread_data->length,
                       thread_data->infoLog);

}

EAPI void
evas_glGetProgramInfoLog_th(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetProgramInfoLog thread_data_local;
   Evas_Thread_Command_glGetProgramInfoLog *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetProgramInfoLog(program, bufSize, length, infoLog);
        return;
     }

   thread_data->program = program;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->infoLog = infoLog;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetProgramInfoLog,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetProgramiv(GLuint program, GLenum pname, GLint *params);
 */

typedef struct
{
   GLuint program;
   GLenum pname;
   GLint *params;

} Evas_Thread_Command_glGetProgramiv;

static void
_gl_thread_glGetProgramiv(void *data)
{
   Evas_Thread_Command_glGetProgramiv *thread_data =
      (Evas_Thread_Command_glGetProgramiv *)data;

   glGetProgramiv(thread_data->program,
                  thread_data->pname,
                  thread_data->params);

}

EAPI void
evas_glGetProgramiv_th(GLuint program, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetProgramiv thread_data_local;
   Evas_Thread_Command_glGetProgramiv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetProgramiv(program, pname, params);
        return;
     }

   thread_data->program = program;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetProgramiv,
                              thread_data,
                              thread_mode);
}

/*
 * GLboolean
 * glIsFramebuffer(GLint framebuffer);
 */

typedef struct
{
   GLboolean return_value;
   GLint framebuffer;

} Evas_Thread_Command_glIsFramebuffer;

static void
_gl_thread_glIsFramebuffer(void *data)
{
   Evas_Thread_Command_glIsFramebuffer *thread_data =
      (Evas_Thread_Command_glIsFramebuffer *)data;

   thread_data->return_value = glIsFramebuffer(thread_data->framebuffer);

}

EAPI GLboolean
evas_glIsFramebuffer_th(GLint framebuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glIsFramebuffer thread_data_local;
   Evas_Thread_Command_glIsFramebuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glIsFramebuffer(framebuffer);
     }

   thread_data->framebuffer = framebuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glIsFramebuffer,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint *params);
 */

typedef struct
{
   GLenum target;
   GLenum pname;
   GLint *params;

} Evas_Thread_Command_glGetFramebufferParameteriv;

void (*orig_evas_glGetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params);

void
glGetFramebufferParameteriv_orig_evas_set(void *func)
{
   orig_evas_glGetFramebufferParameteriv = func;
}

void *
glGetFramebufferParameteriv_orig_evas_get(void)
{
   return orig_evas_glGetFramebufferParameteriv;
}

static void
_gl_thread_glGetFramebufferParameteriv(void *data)
{
   Evas_Thread_Command_glGetFramebufferParameteriv *thread_data =
      (Evas_Thread_Command_glGetFramebufferParameteriv *)data;

   orig_evas_glGetFramebufferParameteriv(thread_data->target,
                                         thread_data->pname,
                                         thread_data->params);

}

EAPI void
evas_glGetFramebufferParameteriv_th(GLenum target, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetFramebufferParameteriv thread_data_local;
   Evas_Thread_Command_glGetFramebufferParameteriv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glGetFramebufferParameteriv(target, pname, params);
        return;
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetFramebufferParameteriv,
                              thread_data,
                              thread_mode);
}

/*
 * GLboolean
 * glIsRenderbuffer(GLint renderbuffer);
 */

typedef struct
{
   GLboolean return_value;
   GLint renderbuffer;

} Evas_Thread_Command_glIsRenderbuffer;

static void
_gl_thread_glIsRenderbuffer(void *data)
{
   Evas_Thread_Command_glIsRenderbuffer *thread_data =
      (Evas_Thread_Command_glIsRenderbuffer *)data;

   thread_data->return_value = glIsRenderbuffer(thread_data->renderbuffer);

}

EAPI GLboolean
evas_glIsRenderbuffer_th(GLint renderbuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glIsRenderbuffer thread_data_local;
   Evas_Thread_Command_glIsRenderbuffer *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glIsRenderbuffer(renderbuffer);
     }

   thread_data->renderbuffer = renderbuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glIsRenderbuffer,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params);
 */

typedef struct
{
   GLenum target;
   GLenum pname;
   GLint *params;

} Evas_Thread_Command_glGetRenderbufferParameteriv;

static void
_gl_thread_glGetRenderbufferParameteriv(void *data)
{
   Evas_Thread_Command_glGetRenderbufferParameteriv *thread_data =
      (Evas_Thread_Command_glGetRenderbufferParameteriv *)data;

   glGetRenderbufferParameteriv(thread_data->target,
                                thread_data->pname,
                                thread_data->params);

}

EAPI void
evas_glGetRenderbufferParameteriv_th(GLenum target, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetRenderbufferParameteriv thread_data_local;
   Evas_Thread_Command_glGetRenderbufferParameteriv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetRenderbufferParameteriv(target, pname, params);
        return;
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetRenderbufferParameteriv,
                              thread_data,
                              thread_mode);
}

/*
 * GLboolean
 * glIsTexture(GLint texture);
 */

typedef struct
{
   GLboolean return_value;
   GLint texture;

} Evas_Thread_Command_glIsTexture;

static void
_gl_thread_glIsTexture(void *data)
{
   Evas_Thread_Command_glIsTexture *thread_data =
      (Evas_Thread_Command_glIsTexture *)data;

   thread_data->return_value = glIsTexture(thread_data->texture);

}

EAPI GLboolean
evas_glIsTexture_th(GLint texture)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glIsTexture thread_data_local;
   Evas_Thread_Command_glIsTexture *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return glIsTexture(texture);
     }

   thread_data->texture = texture;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glIsTexture,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glStartTiling(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e);
 */

typedef struct
{
   GLuint a;
   GLuint b;
   GLuint c;
   GLuint d;
   GLuint e;
   int command_allocated;

} Evas_Thread_Command_glStartTiling;

void (*orig_evas_glStartTiling)(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e);

void
glStartTiling_orig_evas_set(void *func)
{
   orig_evas_glStartTiling = func;
}

void *
glStartTiling_orig_evas_get(void)
{
   return orig_evas_glStartTiling;
}

static void
_gl_thread_glStartTiling(void *data)
{
   Evas_Thread_Command_glStartTiling *thread_data =
      (Evas_Thread_Command_glStartTiling *)data;

   orig_evas_glStartTiling(thread_data->a,
                           thread_data->b,
                           thread_data->c,
                           thread_data->d,
                           thread_data->e);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glStartTiling_th(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glStartTiling thread_data_local;
   Evas_Thread_Command_glStartTiling *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glStartTiling(a, b, c, d, e);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glStartTiling *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glStartTiling));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->a = a;
   thread_data->b = b;
   thread_data->c = c;
   thread_data->d = d;
   thread_data->e = e;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glStartTiling,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glEndTiling(GLuint a);
 */

typedef struct
{
   GLuint a;
   int command_allocated;

} Evas_Thread_Command_glEndTiling;

void (*orig_evas_glEndTiling)(GLuint a);

void
glEndTiling_orig_evas_set(void *func)
{
   orig_evas_glEndTiling = func;
}

void *
glEndTiling_orig_evas_get(void)
{
   return orig_evas_glEndTiling;
}

static void
_gl_thread_glEndTiling(void *data)
{
   Evas_Thread_Command_glEndTiling *thread_data =
      (Evas_Thread_Command_glEndTiling *)data;

   orig_evas_glEndTiling(thread_data->a);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glEndTiling_th(GLuint a)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glEndTiling thread_data_local;
   Evas_Thread_Command_glEndTiling *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glEndTiling(a);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glEndTiling *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glEndTiling));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->a = a;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glEndTiling,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glActivateTile(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e);
 */

typedef struct
{
   GLuint a;
   GLuint b;
   GLuint c;
   GLuint d;
   GLuint e;
   int command_allocated;

} Evas_Thread_Command_glActivateTile;

void (*orig_evas_glActivateTile)(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e);

void
glActivateTile_orig_evas_set(void *func)
{
   orig_evas_glActivateTile = func;
}

void *
glActivateTile_orig_evas_get(void)
{
   return orig_evas_glActivateTile;
}

static void
_gl_thread_glActivateTile(void *data)
{
   Evas_Thread_Command_glActivateTile *thread_data =
      (Evas_Thread_Command_glActivateTile *)data;

   orig_evas_glActivateTile(thread_data->a,
                            thread_data->b,
                            thread_data->c,
                            thread_data->d,
                            thread_data->e);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glActivateTile_th(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glActivateTile thread_data_local;
   Evas_Thread_Command_glActivateTile *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glActivateTile(a, b, c, d, e);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glActivateTile *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glActivateTile));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->a = a;
   thread_data->b = b;
   thread_data->c = c;
   thread_data->d = d;
   thread_data->e = e;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glActivateTile,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glEGLImageTargetTexture2DOES(GLenum target, void *image);
 */

typedef struct
{
   GLenum target;
   void *image;

} Evas_Thread_Command_glEGLImageTargetTexture2DOES;

void (*orig_evas_glEGLImageTargetTexture2DOES)(GLenum target, void *image);

void
glEGLImageTargetTexture2DOES_orig_evas_set(void *func)
{
   orig_evas_glEGLImageTargetTexture2DOES = func;
}

void *
glEGLImageTargetTexture2DOES_orig_evas_get(void)
{
   return orig_evas_glEGLImageTargetTexture2DOES;
}

static void
_gl_thread_glEGLImageTargetTexture2DOES(void *data)
{
   Evas_Thread_Command_glEGLImageTargetTexture2DOES *thread_data =
      (Evas_Thread_Command_glEGLImageTargetTexture2DOES *)data;

   orig_evas_glEGLImageTargetTexture2DOES(thread_data->target,
                                          thread_data->image);

}

EAPI void
evas_glEGLImageTargetTexture2DOES_th(GLenum target, void *image)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glEGLImageTargetTexture2DOES thread_data_local;
   Evas_Thread_Command_glEGLImageTargetTexture2DOES *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glEGLImageTargetTexture2DOES(target, image);
        return;
     }

   thread_data->target = target;
   thread_data->image = image;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glEGLImageTargetTexture2DOES,
                              thread_data,
                              thread_mode);
}

#ifndef GL_GLES

/*
 * void
 * glAlphaFunc(GLenum func, GLclampf ref);
 */

typedef struct
{
   GLenum func;
   GLclampf ref;
   int command_allocated;

} Evas_Thread_Command_glAlphaFunc;

static void
_gl_thread_glAlphaFunc(void *data)
{
   Evas_Thread_Command_glAlphaFunc *thread_data =
      (Evas_Thread_Command_glAlphaFunc *)data;

   glAlphaFunc(thread_data->func,
               thread_data->ref);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glAlphaFunc_th(GLenum func, GLclampf ref)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glAlphaFunc thread_data_local;
   Evas_Thread_Command_glAlphaFunc *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glAlphaFunc(func, ref);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        Evas_Thread_Command_glAlphaFunc *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(Evas_Thread_Command_glAlphaFunc));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->func = func;
   thread_data->ref = ref;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glAlphaFunc,
                              thread_data,
                              thread_mode);
}
#endif

#ifndef GL_GLES

/*
 * void
 * glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params);
 */

typedef struct
{
   GLenum target;
   GLint level;
   GLenum pname;
   GLint *params;

} Evas_Thread_Command_glGetTexLevelParameteriv;

static void
_gl_thread_glGetTexLevelParameteriv(void *data)
{
   Evas_Thread_Command_glGetTexLevelParameteriv *thread_data =
      (Evas_Thread_Command_glGetTexLevelParameteriv *)data;

   glGetTexLevelParameteriv(thread_data->target,
                            thread_data->level,
                            thread_data->pname,
                            thread_data->params);

}

EAPI void
evas_glGetTexLevelParameteriv_th(GLenum target, GLint level, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetTexLevelParameteriv thread_data_local;
   Evas_Thread_Command_glGetTexLevelParameteriv *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        glGetTexLevelParameteriv(target, level, pname, params);
        return;
     }

   thread_data->target = target;
   thread_data->level = level;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetTexLevelParameteriv,
                              thread_data,
                              thread_mode);
}
#endif

#ifndef GL_GLES

/*
 * void
 * glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
 */

typedef struct
{
   GLenum target;
   GLsizei samples;
   GLenum internalformat;
   GLsizei width;
   GLsizei height;

} Evas_Thread_Command_glRenderbufferStorageMultisample;

void (*orig_evas_glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

void
glRenderbufferStorageMultisample_orig_evas_set(void *func)
{
   orig_evas_glRenderbufferStorageMultisample = func;
}

void *
glRenderbufferStorageMultisample_orig_evas_get(void)
{
   return orig_evas_glRenderbufferStorageMultisample;
}

static void
_gl_thread_glRenderbufferStorageMultisample(void *data)
{
   Evas_Thread_Command_glRenderbufferStorageMultisample *thread_data =
      (Evas_Thread_Command_glRenderbufferStorageMultisample *)data;

   orig_evas_glRenderbufferStorageMultisample(thread_data->target,
                                              thread_data->samples,
                                              thread_data->internalformat,
                                              thread_data->width,
                                              thread_data->height);

}

EAPI void
evas_glRenderbufferStorageMultisample_th(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glRenderbufferStorageMultisample thread_data_local;
   Evas_Thread_Command_glRenderbufferStorageMultisample *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        orig_evas_glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
        return;
     }

   thread_data->target = target;
   thread_data->samples = samples;
   thread_data->internalformat = internalformat;
   thread_data->width = width;
   thread_data->height = height;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glRenderbufferStorageMultisample,
                              thread_data,
                              thread_mode);
}
#endif

/*
 * const GLubyte *
 * glGetStringi(GLenum name, GLuint index);
 */

typedef struct
{
   const GLubyte * return_value;
   GLenum name;
   GLuint index;

} Evas_Thread_Command_glGetStringi;

const GLubyte * (*orig_evas_glGetStringi)(GLenum name, GLuint index);

void
glGetStringi_orig_evas_set(void *func)
{
   orig_evas_glGetStringi = func;
}

void *
glGetStringi_orig_evas_get(void)
{
   return orig_evas_glGetStringi;
}

static void
_gl_thread_glGetStringi(void *data)
{
   Evas_Thread_Command_glGetStringi *thread_data =
      (Evas_Thread_Command_glGetStringi *)data;

   thread_data->return_value = orig_evas_glGetStringi(thread_data->name,
                                                      thread_data->index);

}

EAPI const GLubyte *
evas_glGetStringi_th(GLenum name, GLuint index)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   Evas_Thread_Command_glGetStringi thread_data_local;
   Evas_Thread_Command_glGetStringi *thread_data = &thread_data_local;

   if (!evas_gl_thread_enabled())
     {
        return orig_evas_glGetStringi(name, index);
     }

   thread_data->name = name;
   thread_data->index = index;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_GL,
                              _gl_thread_glGetStringi,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}
