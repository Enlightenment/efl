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

} EVGL_Thread_Command_glGetError;

static void
_evgl_thread_glGetError(void *data)
{
   EVGL_Thread_Command_glGetError *thread_data =
      (EVGL_Thread_Command_glGetError *)data;

   thread_data->return_value = glGetError();

}

EAPI GLenum
evas_glGetError_evgl_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetError thread_data_local;
   EVGL_Thread_Command_glGetError *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glGetError();
     }


   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetError,
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

} EVGL_Thread_Command_glVertexAttribPointer;

static void
_evgl_thread_glVertexAttribPointer(void *data)
{
   EVGL_Thread_Command_glVertexAttribPointer *thread_data =
      (EVGL_Thread_Command_glVertexAttribPointer *)data;

   glVertexAttribPointer(thread_data->index,
                         thread_data->size,
                         thread_data->type,
                         thread_data->normalized,
                         thread_data->stride,
                         thread_data->pointer);

}

EAPI void
evas_glVertexAttribPointer_evgl_th(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glVertexAttribPointer thread_data_local;
   EVGL_Thread_Command_glVertexAttribPointer *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glVertexAttribPointer,
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

} EVGL_Thread_Command_glEnableVertexAttribArray;

static void
_evgl_thread_glEnableVertexAttribArray(void *data)
{
   EVGL_Thread_Command_glEnableVertexAttribArray *thread_data =
      (EVGL_Thread_Command_glEnableVertexAttribArray *)data;

   glEnableVertexAttribArray(thread_data->index);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glEnableVertexAttribArray_evgl_th(GLuint index)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glEnableVertexAttribArray thread_data_local;
   EVGL_Thread_Command_glEnableVertexAttribArray *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glEnableVertexAttribArray(index);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glEnableVertexAttribArray *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glEnableVertexAttribArray));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->index = index;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glEnableVertexAttribArray,
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

} EVGL_Thread_Command_glDisableVertexAttribArray;

static void
_evgl_thread_glDisableVertexAttribArray(void *data)
{
   EVGL_Thread_Command_glDisableVertexAttribArray *thread_data =
      (EVGL_Thread_Command_glDisableVertexAttribArray *)data;

   glDisableVertexAttribArray(thread_data->index);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDisableVertexAttribArray_evgl_th(GLuint index)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDisableVertexAttribArray thread_data_local;
   EVGL_Thread_Command_glDisableVertexAttribArray *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDisableVertexAttribArray(index);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDisableVertexAttribArray *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDisableVertexAttribArray));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->index = index;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDisableVertexAttribArray,
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

} EVGL_Thread_Command_glDrawArrays;

static void
_evgl_thread_glDrawArrays(void *data)
{
   EVGL_Thread_Command_glDrawArrays *thread_data =
      (EVGL_Thread_Command_glDrawArrays *)data;

   glDrawArrays(thread_data->mode,
                thread_data->first,
                thread_data->count);

}

EAPI void
evas_glDrawArrays_evgl_th(GLenum mode, GLint first, GLsizei count)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDrawArrays thread_data_local;
   EVGL_Thread_Command_glDrawArrays *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDrawArrays(mode, first, count);
        return;
     }

   thread_data->mode = mode;
   thread_data->first = first;
   thread_data->count = count;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDrawArrays,
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

} EVGL_Thread_Command_glDrawElements;

static void
_evgl_thread_glDrawElements(void *data)
{
   EVGL_Thread_Command_glDrawElements *thread_data =
      (EVGL_Thread_Command_glDrawElements *)data;

   glDrawElements(thread_data->mode,
                  thread_data->count,
                  thread_data->type,
                  thread_data->indices);

}

EAPI void
evas_glDrawElements_evgl_th(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDrawElements thread_data_local;
   EVGL_Thread_Command_glDrawElements *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDrawElements(mode, count, type, indices);
        return;
     }

   thread_data->mode = mode;
   thread_data->count = count;
   thread_data->type = type;
   thread_data->indices = indices;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDrawElements,
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

} EVGL_Thread_Command_glGenBuffers;

static void
_evgl_thread_glGenBuffers(void *data)
{
   EVGL_Thread_Command_glGenBuffers *thread_data =
      (EVGL_Thread_Command_glGenBuffers *)data;

   glGenBuffers(thread_data->n,
                thread_data->buffers);

}

EAPI void
evas_glGenBuffers_evgl_th(GLsizei n, GLuint *buffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGenBuffers thread_data_local;
   EVGL_Thread_Command_glGenBuffers *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGenBuffers(n, buffers);
        return;
     }

   thread_data->n = n;
   thread_data->buffers = buffers;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGenBuffers,
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

} EVGL_Thread_Command_glDeleteBuffers;

static void
_evgl_thread_glDeleteBuffers(void *data)
{
   EVGL_Thread_Command_glDeleteBuffers *thread_data =
      (EVGL_Thread_Command_glDeleteBuffers *)data;

   glDeleteBuffers(thread_data->n,
                   thread_data->buffers);


   if (thread_data->buffers_copied)
     eina_mempool_free(_mp_delete_object, thread_data->buffers_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteBuffers_evgl_th(GLsizei n, const GLuint *buffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDeleteBuffers thread_data_local;
   EVGL_Thread_Command_glDeleteBuffers *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDeleteBuffers(n, buffers);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDeleteBuffers *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDeleteBuffers));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDeleteBuffers,
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

} EVGL_Thread_Command_glBindBuffer;

static void
_evgl_thread_glBindBuffer(void *data)
{
   EVGL_Thread_Command_glBindBuffer *thread_data =
      (EVGL_Thread_Command_glBindBuffer *)data;

   glBindBuffer(thread_data->target,
                thread_data->buffer);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBindBuffer_evgl_th(GLenum target, GLuint buffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glBindBuffer thread_data_local;
   EVGL_Thread_Command_glBindBuffer *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glBindBuffer(target, buffer);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glBindBuffer *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glBindBuffer));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->buffer = buffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glBindBuffer,
                              thread_data,
                              thread_mode);
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

} EVGL_Thread_Command_glBufferData;

static void
_evgl_thread_glBufferData(void *data)
{
   EVGL_Thread_Command_glBufferData *thread_data =
      (EVGL_Thread_Command_glBufferData *)data;

   glBufferData(thread_data->target,
                thread_data->size,
                thread_data->data,
                thread_data->usage);

}

EAPI void
evas_glBufferData_evgl_th(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glBufferData thread_data_local;
   EVGL_Thread_Command_glBufferData *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glBufferData(target, size, data, usage);
        return;
     }

   thread_data->target = target;
   thread_data->size = size;
   thread_data->data = data;
   thread_data->usage = usage;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glBufferData,
                              thread_data,
                              thread_mode);
}

/*
 * GLuint
 * glCreateShader(GLenum type);
 */

typedef struct
{
   GLuint return_value;
   GLenum type;

} EVGL_Thread_Command_glCreateShader;

static void
_evgl_thread_glCreateShader(void *data)
{
   EVGL_Thread_Command_glCreateShader *thread_data =
      (EVGL_Thread_Command_glCreateShader *)data;

   thread_data->return_value = glCreateShader(thread_data->type);

}

EAPI GLuint
evas_glCreateShader_evgl_th(GLenum type)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glCreateShader thread_data_local;
   EVGL_Thread_Command_glCreateShader *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glCreateShader(type);
     }

   thread_data->type = type;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glCreateShader,
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

} EVGL_Thread_Command_glShaderSource;

static void
_evgl_thread_glShaderSource(void *data)
{
   EVGL_Thread_Command_glShaderSource *thread_data =
      (EVGL_Thread_Command_glShaderSource *)data;


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
evas_glShaderSource_evgl_th(GLuint shader, GLsizei count, const GLchar **string, const GLint *length)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glShaderSource thread_data_local;
   EVGL_Thread_Command_glShaderSource *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glShaderSource(shader, count, string, length);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glShaderSource *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glShaderSource));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glShaderSource,
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

} EVGL_Thread_Command_glCompileShader;

static void
_evgl_thread_glCompileShader(void *data)
{
   EVGL_Thread_Command_glCompileShader *thread_data =
      (EVGL_Thread_Command_glCompileShader *)data;

   glCompileShader(thread_data->shader);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glCompileShader_evgl_th(GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glCompileShader thread_data_local;
   EVGL_Thread_Command_glCompileShader *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glCompileShader(shader);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glCompileShader *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glCompileShader));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glCompileShader,
                              thread_data,
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

} EVGL_Thread_Command_glDeleteShader;

static void
_evgl_thread_glDeleteShader(void *data)
{
   EVGL_Thread_Command_glDeleteShader *thread_data =
      (EVGL_Thread_Command_glDeleteShader *)data;

   glDeleteShader(thread_data->shader);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteShader_evgl_th(GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDeleteShader thread_data_local;
   EVGL_Thread_Command_glDeleteShader *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDeleteShader(shader);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDeleteShader *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDeleteShader));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDeleteShader,
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

} EVGL_Thread_Command_glCreateProgram;

static void
_evgl_thread_glCreateProgram(void *data)
{
   EVGL_Thread_Command_glCreateProgram *thread_data =
      (EVGL_Thread_Command_glCreateProgram *)data;

   thread_data->return_value = glCreateProgram();

}

EAPI GLuint
evas_glCreateProgram_evgl_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glCreateProgram thread_data_local;
   EVGL_Thread_Command_glCreateProgram *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glCreateProgram();
     }


   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glCreateProgram,
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

} EVGL_Thread_Command_glAttachShader;

static void
_evgl_thread_glAttachShader(void *data)
{
   EVGL_Thread_Command_glAttachShader *thread_data =
      (EVGL_Thread_Command_glAttachShader *)data;

   glAttachShader(thread_data->program,
                  thread_data->shader);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glAttachShader_evgl_th(GLuint program, GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glAttachShader thread_data_local;
   EVGL_Thread_Command_glAttachShader *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glAttachShader(program, shader);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glAttachShader *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glAttachShader));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;
   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glAttachShader,
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

} EVGL_Thread_Command_glDetachShader;

static void
_evgl_thread_glDetachShader(void *data)
{
   EVGL_Thread_Command_glDetachShader *thread_data =
      (EVGL_Thread_Command_glDetachShader *)data;

   glDetachShader(thread_data->program,
                  thread_data->shader);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDetachShader_evgl_th(GLuint program, GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDetachShader thread_data_local;
   EVGL_Thread_Command_glDetachShader *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDetachShader(program, shader);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDetachShader *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDetachShader));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;
   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDetachShader,
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

} EVGL_Thread_Command_glLinkProgram;

static void
_evgl_thread_glLinkProgram(void *data)
{
   EVGL_Thread_Command_glLinkProgram *thread_data =
      (EVGL_Thread_Command_glLinkProgram *)data;

   glLinkProgram(thread_data->program);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glLinkProgram_evgl_th(GLuint program)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glLinkProgram thread_data_local;
   EVGL_Thread_Command_glLinkProgram *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glLinkProgram(program);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glLinkProgram *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glLinkProgram));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glLinkProgram,
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

} EVGL_Thread_Command_glUseProgram;

static void
_evgl_thread_glUseProgram(void *data)
{
   EVGL_Thread_Command_glUseProgram *thread_data =
      (EVGL_Thread_Command_glUseProgram *)data;

   glUseProgram(thread_data->program);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUseProgram_evgl_th(GLuint program)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUseProgram thread_data_local;
   EVGL_Thread_Command_glUseProgram *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUseProgram(program);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUseProgram *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUseProgram));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUseProgram,
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

} EVGL_Thread_Command_glDeleteProgram;

static void
_evgl_thread_glDeleteProgram(void *data)
{
   EVGL_Thread_Command_glDeleteProgram *thread_data =
      (EVGL_Thread_Command_glDeleteProgram *)data;

   glDeleteProgram(thread_data->program);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteProgram_evgl_th(GLuint program)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDeleteProgram thread_data_local;
   EVGL_Thread_Command_glDeleteProgram *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDeleteProgram(program);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDeleteProgram *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDeleteProgram));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->program = program;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDeleteProgram,
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

} EVGL_Thread_Command_glGetProgramBinary;

void (*orig_evgl_glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);

void
glGetProgramBinary_orig_evgl_set(void *func)
{
   orig_evgl_glGetProgramBinary = func;
}

void *
glGetProgramBinary_orig_evgl_get(void)
{
   return orig_evgl_glGetProgramBinary;
}

static void
_evgl_thread_glGetProgramBinary(void *data)
{
   EVGL_Thread_Command_glGetProgramBinary *thread_data =
      (EVGL_Thread_Command_glGetProgramBinary *)data;

   orig_evgl_glGetProgramBinary(thread_data->program,
                                thread_data->bufSize,
                                thread_data->length,
                                thread_data->binaryFormat,
                                thread_data->binary);

}

EAPI void
evas_glGetProgramBinary_evgl_th(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetProgramBinary thread_data_local;
   EVGL_Thread_Command_glGetProgramBinary *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        orig_evgl_glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
        return;
     }

   thread_data->program = program;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->binaryFormat = binaryFormat;
   thread_data->binary = binary;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetProgramBinary,
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

} EVGL_Thread_Command_glProgramBinary;

void (*orig_evgl_glProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLint length);

void
glProgramBinary_orig_evgl_set(void *func)
{
   orig_evgl_glProgramBinary = func;
}

void *
glProgramBinary_orig_evgl_get(void)
{
   return orig_evgl_glProgramBinary;
}

static void
_evgl_thread_glProgramBinary(void *data)
{
   EVGL_Thread_Command_glProgramBinary *thread_data =
      (EVGL_Thread_Command_glProgramBinary *)data;

   orig_evgl_glProgramBinary(thread_data->program,
                             thread_data->binaryFormat,
                             thread_data->binary,
                             thread_data->length);


   if (thread_data->binary_copied)
     eina_mempool_free(_mp_default, thread_data->binary_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glProgramBinary_evgl_th(GLuint program, GLenum binaryFormat, const void *binary, GLint length)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glProgramBinary thread_data_local;
   EVGL_Thread_Command_glProgramBinary *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        orig_evgl_glProgramBinary(program, binaryFormat, binary, length);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glProgramBinary *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glProgramBinary));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glProgramBinary,
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

} EVGL_Thread_Command_glGetActiveAttrib;

static void
_evgl_thread_glGetActiveAttrib(void *data)
{
   EVGL_Thread_Command_glGetActiveAttrib *thread_data =
      (EVGL_Thread_Command_glGetActiveAttrib *)data;

   glGetActiveAttrib(thread_data->program,
                     thread_data->index,
                     thread_data->bufSize,
                     thread_data->length,
                     thread_data->size,
                     thread_data->type,
                     thread_data->name);

}

EAPI void
evas_glGetActiveAttrib_evgl_th(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetActiveAttrib thread_data_local;
   EVGL_Thread_Command_glGetActiveAttrib *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetActiveAttrib,
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

} EVGL_Thread_Command_glGetAttribLocation;

static void
_evgl_thread_glGetAttribLocation(void *data)
{
   EVGL_Thread_Command_glGetAttribLocation *thread_data =
      (EVGL_Thread_Command_glGetAttribLocation *)data;

   thread_data->return_value = glGetAttribLocation(thread_data->program,
                                                   thread_data->name);

}

EAPI GLint
evas_glGetAttribLocation_evgl_th(GLuint program, const GLchar *name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetAttribLocation thread_data_local;
   EVGL_Thread_Command_glGetAttribLocation *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glGetAttribLocation(program, name);
     }

   thread_data->program = program;
   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetAttribLocation,
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

} EVGL_Thread_Command_glBindAttribLocation;

static void
_evgl_thread_glBindAttribLocation(void *data)
{
   EVGL_Thread_Command_glBindAttribLocation *thread_data =
      (EVGL_Thread_Command_glBindAttribLocation *)data;

   glBindAttribLocation(thread_data->program,
                        thread_data->index,
                        thread_data->name);

}

EAPI void
evas_glBindAttribLocation_evgl_th(GLuint program, GLuint index, const GLchar *name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glBindAttribLocation thread_data_local;
   EVGL_Thread_Command_glBindAttribLocation *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glBindAttribLocation(program, index, name);
        return;
     }

   thread_data->program = program;
   thread_data->index = index;
   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glBindAttribLocation,
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

} EVGL_Thread_Command_glGetUniformLocation;

static void
_evgl_thread_glGetUniformLocation(void *data)
{
   EVGL_Thread_Command_glGetUniformLocation *thread_data =
      (EVGL_Thread_Command_glGetUniformLocation *)data;

   thread_data->return_value = glGetUniformLocation(thread_data->program,
                                                    thread_data->name);

}

EAPI GLint
evas_glGetUniformLocation_evgl_th(GLuint program, const GLchar *name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetUniformLocation thread_data_local;
   EVGL_Thread_Command_glGetUniformLocation *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glGetUniformLocation(program, name);
     }

   thread_data->program = program;
   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetUniformLocation,
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

} EVGL_Thread_Command_glUniform1f;

static void
_evgl_thread_glUniform1f(void *data)
{
   EVGL_Thread_Command_glUniform1f *thread_data =
      (EVGL_Thread_Command_glUniform1f *)data;

   glUniform1f(thread_data->location,
               thread_data->v0);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform1f_evgl_th(GLint location, GLfloat v0)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform1f thread_data_local;
   EVGL_Thread_Command_glUniform1f *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform1f(location, v0);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform1f *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform1f));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform1f,
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

} EVGL_Thread_Command_glUniform1i;

static void
_evgl_thread_glUniform1i(void *data)
{
   EVGL_Thread_Command_glUniform1i *thread_data =
      (EVGL_Thread_Command_glUniform1i *)data;

   glUniform1i(thread_data->location,
               thread_data->v0);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform1i_evgl_th(GLint location, GLint v0)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform1i thread_data_local;
   EVGL_Thread_Command_glUniform1i *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform1i(location, v0);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform1i *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform1i));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->location = location;
   thread_data->v0 = v0;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform1i,
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

} EVGL_Thread_Command_glUniform2f;

static void
_evgl_thread_glUniform2f(void *data)
{
   EVGL_Thread_Command_glUniform2f *thread_data =
      (EVGL_Thread_Command_glUniform2f *)data;

   glUniform2f(thread_data->location,
               thread_data->v0,
               thread_data->v1);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform2f_evgl_th(GLint location, GLfloat v0, GLfloat v1)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform2f thread_data_local;
   EVGL_Thread_Command_glUniform2f *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform2f(location, v0, v1);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform2f *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform2f));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform2f,
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

} EVGL_Thread_Command_glUniform2i;

static void
_evgl_thread_glUniform2i(void *data)
{
   EVGL_Thread_Command_glUniform2i *thread_data =
      (EVGL_Thread_Command_glUniform2i *)data;

   glUniform2i(thread_data->location,
               thread_data->v0,
               thread_data->v1);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform2i_evgl_th(GLint location, GLint v0, GLint v1)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform2i thread_data_local;
   EVGL_Thread_Command_glUniform2i *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform2i(location, v0, v1);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform2i *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform2i));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform2i,
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

} EVGL_Thread_Command_glUniform3f;

static void
_evgl_thread_glUniform3f(void *data)
{
   EVGL_Thread_Command_glUniform3f *thread_data =
      (EVGL_Thread_Command_glUniform3f *)data;

   glUniform3f(thread_data->location,
               thread_data->v0,
               thread_data->v1,
               thread_data->v2);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform3f_evgl_th(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform3f thread_data_local;
   EVGL_Thread_Command_glUniform3f *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform3f(location, v0, v1, v2);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform3f *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform3f));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform3f,
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

} EVGL_Thread_Command_glUniform3i;

static void
_evgl_thread_glUniform3i(void *data)
{
   EVGL_Thread_Command_glUniform3i *thread_data =
      (EVGL_Thread_Command_glUniform3i *)data;

   glUniform3i(thread_data->location,
               thread_data->v0,
               thread_data->v1,
               thread_data->v2);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform3i_evgl_th(GLint location, GLint v0, GLint v1, GLint v2)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform3i thread_data_local;
   EVGL_Thread_Command_glUniform3i *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform3i(location, v0, v1, v2);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform3i *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform3i));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform3i,
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

} EVGL_Thread_Command_glUniform4f;

static void
_evgl_thread_glUniform4f(void *data)
{
   EVGL_Thread_Command_glUniform4f *thread_data =
      (EVGL_Thread_Command_glUniform4f *)data;

   glUniform4f(thread_data->location,
               thread_data->v0,
               thread_data->v1,
               thread_data->v2,
               thread_data->v3);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform4f_evgl_th(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform4f thread_data_local;
   EVGL_Thread_Command_glUniform4f *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform4f(location, v0, v1, v2, v3);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform4f *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform4f));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform4f,
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

} EVGL_Thread_Command_glUniform4i;

static void
_evgl_thread_glUniform4i(void *data)
{
   EVGL_Thread_Command_glUniform4i *thread_data =
      (EVGL_Thread_Command_glUniform4i *)data;

   glUniform4i(thread_data->location,
               thread_data->v0,
               thread_data->v1,
               thread_data->v2,
               thread_data->v3);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform4i_evgl_th(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform4i thread_data_local;
   EVGL_Thread_Command_glUniform4i *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform4i(location, v0, v1, v2, v3);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform4i *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform4i));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform4i,
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

} EVGL_Thread_Command_glUniform1fv;

static void
_evgl_thread_glUniform1fv(void *data)
{
   EVGL_Thread_Command_glUniform1fv *thread_data =
      (EVGL_Thread_Command_glUniform1fv *)data;

   glUniform1fv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform1fv_evgl_th(GLint location, GLsizei count, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform1fv thread_data_local;
   EVGL_Thread_Command_glUniform1fv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform1fv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform1fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform1fv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform1fv,
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

} EVGL_Thread_Command_glUniform1iv;

static void
_evgl_thread_glUniform1iv(void *data)
{
   EVGL_Thread_Command_glUniform1iv *thread_data =
      (EVGL_Thread_Command_glUniform1iv *)data;

   glUniform1iv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform1iv_evgl_th(GLint location, GLsizei count, const GLint *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform1iv thread_data_local;
   EVGL_Thread_Command_glUniform1iv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform1iv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform1iv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform1iv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform1iv,
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

} EVGL_Thread_Command_glUniform2fv;

static void
_evgl_thread_glUniform2fv(void *data)
{
   EVGL_Thread_Command_glUniform2fv *thread_data =
      (EVGL_Thread_Command_glUniform2fv *)data;

   glUniform2fv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform2fv_evgl_th(GLint location, GLsizei count, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform2fv thread_data_local;
   EVGL_Thread_Command_glUniform2fv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform2fv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform2fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform2fv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform2fv,
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

} EVGL_Thread_Command_glUniform2iv;

static void
_evgl_thread_glUniform2iv(void *data)
{
   EVGL_Thread_Command_glUniform2iv *thread_data =
      (EVGL_Thread_Command_glUniform2iv *)data;

   glUniform2iv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform2iv_evgl_th(GLint location, GLsizei count, const GLint *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform2iv thread_data_local;
   EVGL_Thread_Command_glUniform2iv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform2iv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform2iv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform2iv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform2iv,
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

} EVGL_Thread_Command_glUniform3fv;

static void
_evgl_thread_glUniform3fv(void *data)
{
   EVGL_Thread_Command_glUniform3fv *thread_data =
      (EVGL_Thread_Command_glUniform3fv *)data;

   glUniform3fv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform3fv_evgl_th(GLint location, GLsizei count, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform3fv thread_data_local;
   EVGL_Thread_Command_glUniform3fv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform3fv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform3fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform3fv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform3fv,
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

} EVGL_Thread_Command_glUniform3iv;

static void
_evgl_thread_glUniform3iv(void *data)
{
   EVGL_Thread_Command_glUniform3iv *thread_data =
      (EVGL_Thread_Command_glUniform3iv *)data;

   glUniform3iv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform3iv_evgl_th(GLint location, GLsizei count, const GLint *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform3iv thread_data_local;
   EVGL_Thread_Command_glUniform3iv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform3iv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform3iv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform3iv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform3iv,
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

} EVGL_Thread_Command_glUniform4fv;

static void
_evgl_thread_glUniform4fv(void *data)
{
   EVGL_Thread_Command_glUniform4fv *thread_data =
      (EVGL_Thread_Command_glUniform4fv *)data;

   glUniform4fv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform4fv_evgl_th(GLint location, GLsizei count, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform4fv thread_data_local;
   EVGL_Thread_Command_glUniform4fv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform4fv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform4fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform4fv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform4fv,
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

} EVGL_Thread_Command_glUniform4iv;

static void
_evgl_thread_glUniform4iv(void *data)
{
   EVGL_Thread_Command_glUniform4iv *thread_data =
      (EVGL_Thread_Command_glUniform4iv *)data;

   glUniform4iv(thread_data->location,
                thread_data->count,
                thread_data->value);


   if (thread_data->value_copied)
     eina_mempool_free(_mp_uniform, thread_data->value_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glUniform4iv_evgl_th(GLint location, GLsizei count, const GLint *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniform4iv thread_data_local;
   EVGL_Thread_Command_glUniform4iv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniform4iv(location, count, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniform4iv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniform4iv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniform4iv,
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

} EVGL_Thread_Command_glUniformMatrix2fv;

static void
_evgl_thread_glUniformMatrix2fv(void *data)
{
   EVGL_Thread_Command_glUniformMatrix2fv *thread_data =
      (EVGL_Thread_Command_glUniformMatrix2fv *)data;

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
evas_glUniformMatrix2fv_evgl_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniformMatrix2fv thread_data_local;
   EVGL_Thread_Command_glUniformMatrix2fv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniformMatrix2fv(location, count, transpose, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniformMatrix2fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniformMatrix2fv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniformMatrix2fv,
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

} EVGL_Thread_Command_glUniformMatrix3fv;

static void
_evgl_thread_glUniformMatrix3fv(void *data)
{
   EVGL_Thread_Command_glUniformMatrix3fv *thread_data =
      (EVGL_Thread_Command_glUniformMatrix3fv *)data;

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
evas_glUniformMatrix3fv_evgl_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniformMatrix3fv thread_data_local;
   EVGL_Thread_Command_glUniformMatrix3fv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniformMatrix3fv(location, count, transpose, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniformMatrix3fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniformMatrix3fv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniformMatrix3fv,
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

} EVGL_Thread_Command_glUniformMatrix4fv;

static void
_evgl_thread_glUniformMatrix4fv(void *data)
{
   EVGL_Thread_Command_glUniformMatrix4fv *thread_data =
      (EVGL_Thread_Command_glUniformMatrix4fv *)data;

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
evas_glUniformMatrix4fv_evgl_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glUniformMatrix4fv thread_data_local;
   EVGL_Thread_Command_glUniformMatrix4fv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glUniformMatrix4fv(location, count, transpose, value);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glUniformMatrix4fv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glUniformMatrix4fv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glUniformMatrix4fv,
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

} EVGL_Thread_Command_glViewport;

static void
_evgl_thread_glViewport(void *data)
{
   EVGL_Thread_Command_glViewport *thread_data =
      (EVGL_Thread_Command_glViewport *)data;

   glViewport(thread_data->x,
              thread_data->y,
              thread_data->width,
              thread_data->height);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glViewport_evgl_th(GLint x, GLint y, GLsizei width, GLsizei height)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glViewport thread_data_local;
   EVGL_Thread_Command_glViewport *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glViewport(x, y, width, height);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glViewport *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glViewport));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glViewport,
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

} EVGL_Thread_Command_glEnable;

static void
_evgl_thread_glEnable(void *data)
{
   EVGL_Thread_Command_glEnable *thread_data =
      (EVGL_Thread_Command_glEnable *)data;

   glEnable(thread_data->cap);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glEnable_evgl_th(GLenum cap)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glEnable thread_data_local;
   EVGL_Thread_Command_glEnable *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glEnable(cap);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glEnable *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glEnable));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->cap = cap;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glEnable,
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

} EVGL_Thread_Command_glDisable;

static void
_evgl_thread_glDisable(void *data)
{
   EVGL_Thread_Command_glDisable *thread_data =
      (EVGL_Thread_Command_glDisable *)data;

   glDisable(thread_data->cap);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDisable_evgl_th(GLenum cap)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDisable thread_data_local;
   EVGL_Thread_Command_glDisable *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDisable(cap);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDisable *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDisable));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->cap = cap;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDisable,
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

} EVGL_Thread_Command_glLineWidth;

static void
_evgl_thread_glLineWidth(void *data)
{
   EVGL_Thread_Command_glLineWidth *thread_data =
      (EVGL_Thread_Command_glLineWidth *)data;

   glLineWidth(thread_data->width);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glLineWidth_evgl_th(GLfloat width)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glLineWidth thread_data_local;
   EVGL_Thread_Command_glLineWidth *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glLineWidth(width);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glLineWidth *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glLineWidth));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->width = width;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glLineWidth,
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

} EVGL_Thread_Command_glPolygonOffset;

static void
_evgl_thread_glPolygonOffset(void *data)
{
   EVGL_Thread_Command_glPolygonOffset *thread_data =
      (EVGL_Thread_Command_glPolygonOffset *)data;

   glPolygonOffset(thread_data->factor,
                   thread_data->units);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glPolygonOffset_evgl_th(GLfloat factor, GLfloat units)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glPolygonOffset thread_data_local;
   EVGL_Thread_Command_glPolygonOffset *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glPolygonOffset(factor, units);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glPolygonOffset *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glPolygonOffset));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->factor = factor;
   thread_data->units = units;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glPolygonOffset,
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

} EVGL_Thread_Command_glPixelStorei;

static void
_evgl_thread_glPixelStorei(void *data)
{
   EVGL_Thread_Command_glPixelStorei *thread_data =
      (EVGL_Thread_Command_glPixelStorei *)data;

   glPixelStorei(thread_data->pname,
                 thread_data->param);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glPixelStorei_evgl_th(GLenum pname, GLint param)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glPixelStorei thread_data_local;
   EVGL_Thread_Command_glPixelStorei *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glPixelStorei(pname, param);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glPixelStorei *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glPixelStorei));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->pname = pname;
   thread_data->param = param;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glPixelStorei,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glActiveTexture(GLenum texture);
 */

typedef struct
{
   GLenum texture;
   int command_allocated;

} EVGL_Thread_Command_glActiveTexture;

static void
_evgl_thread_glActiveTexture(void *data)
{
   EVGL_Thread_Command_glActiveTexture *thread_data =
      (EVGL_Thread_Command_glActiveTexture *)data;

   glActiveTexture(thread_data->texture);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glActiveTexture_evgl_th(GLenum texture)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glActiveTexture thread_data_local;
   EVGL_Thread_Command_glActiveTexture *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glActiveTexture(texture);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glActiveTexture *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glActiveTexture));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->texture = texture;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glActiveTexture,
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

} EVGL_Thread_Command_glGenTextures;

static void
_evgl_thread_glGenTextures(void *data)
{
   EVGL_Thread_Command_glGenTextures *thread_data =
      (EVGL_Thread_Command_glGenTextures *)data;

   glGenTextures(thread_data->n,
                 thread_data->textures);

}

EAPI void
evas_glGenTextures_evgl_th(GLsizei n, GLuint *textures)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGenTextures thread_data_local;
   EVGL_Thread_Command_glGenTextures *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGenTextures(n, textures);
        return;
     }

   thread_data->n = n;
   thread_data->textures = textures;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGenTextures,
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

} EVGL_Thread_Command_glBindTexture;

static void
_evgl_thread_glBindTexture(void *data)
{
   EVGL_Thread_Command_glBindTexture *thread_data =
      (EVGL_Thread_Command_glBindTexture *)data;

   glBindTexture(thread_data->target,
                 thread_data->texture);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBindTexture_evgl_th(GLenum target, GLuint texture)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glBindTexture thread_data_local;
   EVGL_Thread_Command_glBindTexture *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glBindTexture(target, texture);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glBindTexture *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glBindTexture));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->texture = texture;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glBindTexture,
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

} EVGL_Thread_Command_glDeleteTextures;

static void
_evgl_thread_glDeleteTextures(void *data)
{
   EVGL_Thread_Command_glDeleteTextures *thread_data =
      (EVGL_Thread_Command_glDeleteTextures *)data;

   glDeleteTextures(thread_data->n,
                    thread_data->textures);


   if (thread_data->textures_copied)
     eina_mempool_free(_mp_delete_object, thread_data->textures_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteTextures_evgl_th(GLsizei n, const GLuint *textures)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDeleteTextures thread_data_local;
   EVGL_Thread_Command_glDeleteTextures *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDeleteTextures(n, textures);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDeleteTextures *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDeleteTextures));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDeleteTextures,
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

} EVGL_Thread_Command_glTexImage2D;

static void
_evgl_thread_glTexImage2D(void *data)
{
   EVGL_Thread_Command_glTexImage2D *thread_data =
      (EVGL_Thread_Command_glTexImage2D *)data;


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
evas_glTexImage2D_evgl_th(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glTexImage2D thread_data_local;
   EVGL_Thread_Command_glTexImage2D *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glTexImage2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glTexImage2D));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glTexImage2D,
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

} EVGL_Thread_Command_glTexSubImage2D;

static void
_evgl_thread_glTexSubImage2D(void *data)
{
   EVGL_Thread_Command_glTexSubImage2D *thread_data =
      (EVGL_Thread_Command_glTexSubImage2D *)data;


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
evas_glTexSubImage2D_evgl_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glTexSubImage2D thread_data_local;
   EVGL_Thread_Command_glTexSubImage2D *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glTexSubImage2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glTexSubImage2D));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glTexSubImage2D,
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

} EVGL_Thread_Command_glCompressedTexImage2D;

static void
_evgl_thread_glCompressedTexImage2D(void *data)
{
   EVGL_Thread_Command_glCompressedTexImage2D *thread_data =
      (EVGL_Thread_Command_glCompressedTexImage2D *)data;


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
evas_glCompressedTexImage2D_evgl_th(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glCompressedTexImage2D thread_data_local;
   EVGL_Thread_Command_glCompressedTexImage2D *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glCompressedTexImage2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glCompressedTexImage2D));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glCompressedTexImage2D,
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

} EVGL_Thread_Command_glCompressedTexSubImage2D;

static void
_evgl_thread_glCompressedTexSubImage2D(void *data)
{
   EVGL_Thread_Command_glCompressedTexSubImage2D *thread_data =
      (EVGL_Thread_Command_glCompressedTexSubImage2D *)data;


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
evas_glCompressedTexSubImage2D_evgl_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glCompressedTexSubImage2D thread_data_local;
   EVGL_Thread_Command_glCompressedTexSubImage2D *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glCompressedTexSubImage2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glCompressedTexSubImage2D));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glCompressedTexSubImage2D,
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

} EVGL_Thread_Command_glTexParameterf;

static void
_evgl_thread_glTexParameterf(void *data)
{
   EVGL_Thread_Command_glTexParameterf *thread_data =
      (EVGL_Thread_Command_glTexParameterf *)data;

   glTexParameterf(thread_data->target,
                   thread_data->pname,
                   thread_data->param);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexParameterf_evgl_th(GLenum target, GLenum pname, GLfloat param)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glTexParameterf thread_data_local;
   EVGL_Thread_Command_glTexParameterf *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glTexParameterf(target, pname, param);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glTexParameterf *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glTexParameterf));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glTexParameterf,
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

} EVGL_Thread_Command_glTexParameterfv;

static void
_evgl_thread_glTexParameterfv(void *data)
{
   EVGL_Thread_Command_glTexParameterfv *thread_data =
      (EVGL_Thread_Command_glTexParameterfv *)data;

   glTexParameterfv(thread_data->target,
                    thread_data->pname,
                    thread_data->params);


   if (thread_data->params_copied)
     eina_mempool_free(_mp_default, thread_data->params_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexParameterfv_evgl_th(GLenum target, GLenum pname, const GLfloat *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glTexParameterfv thread_data_local;
   EVGL_Thread_Command_glTexParameterfv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glTexParameterfv(target, pname, params);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glTexParameterfv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glTexParameterfv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glTexParameterfv,
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

} EVGL_Thread_Command_glTexParameteri;

static void
_evgl_thread_glTexParameteri(void *data)
{
   EVGL_Thread_Command_glTexParameteri *thread_data =
      (EVGL_Thread_Command_glTexParameteri *)data;

   glTexParameteri(thread_data->target,
                   thread_data->pname,
                   thread_data->param);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexParameteri_evgl_th(GLenum target, GLenum pname, GLint param)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glTexParameteri thread_data_local;
   EVGL_Thread_Command_glTexParameteri *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glTexParameteri(target, pname, param);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glTexParameteri *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glTexParameteri));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glTexParameteri,
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

} EVGL_Thread_Command_glTexParameteriv;

static void
_evgl_thread_glTexParameteriv(void *data)
{
   EVGL_Thread_Command_glTexParameteriv *thread_data =
      (EVGL_Thread_Command_glTexParameteriv *)data;

   glTexParameteriv(thread_data->target,
                    thread_data->pname,
                    thread_data->params);


   if (thread_data->params_copied)
     eina_mempool_free(_mp_default, thread_data->params_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glTexParameteriv_evgl_th(GLenum target, GLenum pname, const GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glTexParameteriv thread_data_local;
   EVGL_Thread_Command_glTexParameteriv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glTexParameteriv(target, pname, params);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glTexParameteriv *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glTexParameteriv));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glTexParameteriv,
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

} EVGL_Thread_Command_glScissor;

static void
_evgl_thread_glScissor(void *data)
{
   EVGL_Thread_Command_glScissor *thread_data =
      (EVGL_Thread_Command_glScissor *)data;

   glScissor(thread_data->x,
             thread_data->y,
             thread_data->width,
             thread_data->height);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glScissor_evgl_th(GLint x, GLint y, GLsizei width, GLsizei height)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glScissor thread_data_local;
   EVGL_Thread_Command_glScissor *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glScissor(x, y, width, height);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glScissor *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glScissor));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glScissor,
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

} EVGL_Thread_Command_glBlendFunc;

static void
_evgl_thread_glBlendFunc(void *data)
{
   EVGL_Thread_Command_glBlendFunc *thread_data =
      (EVGL_Thread_Command_glBlendFunc *)data;

   glBlendFunc(thread_data->sfactor,
               thread_data->dfactor);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBlendFunc_evgl_th(GLenum sfactor, GLenum dfactor)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glBlendFunc thread_data_local;
   EVGL_Thread_Command_glBlendFunc *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glBlendFunc(sfactor, dfactor);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glBlendFunc *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glBlendFunc));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->sfactor = sfactor;
   thread_data->dfactor = dfactor;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glBlendFunc,
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

} EVGL_Thread_Command_glBlendColor;

static void
_evgl_thread_glBlendColor(void *data)
{
   EVGL_Thread_Command_glBlendColor *thread_data =
      (EVGL_Thread_Command_glBlendColor *)data;

   glBlendColor(thread_data->red,
                thread_data->green,
                thread_data->blue,
                thread_data->alpha);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBlendColor_evgl_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glBlendColor thread_data_local;
   EVGL_Thread_Command_glBlendColor *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glBlendColor(red, green, blue, alpha);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glBlendColor *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glBlendColor));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glBlendColor,
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

} EVGL_Thread_Command_glDepthMask;

static void
_evgl_thread_glDepthMask(void *data)
{
   EVGL_Thread_Command_glDepthMask *thread_data =
      (EVGL_Thread_Command_glDepthMask *)data;

   glDepthMask(thread_data->flag);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDepthMask_evgl_th(GLboolean flag)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDepthMask thread_data_local;
   EVGL_Thread_Command_glDepthMask *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDepthMask(flag);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDepthMask *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDepthMask));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->flag = flag;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDepthMask,
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

} EVGL_Thread_Command_glClear;

static void
_evgl_thread_glClear(void *data)
{
   EVGL_Thread_Command_glClear *thread_data =
      (EVGL_Thread_Command_glClear *)data;

   glClear(thread_data->mask);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glClear_evgl_th(GLbitfield mask)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glClear thread_data_local;
   EVGL_Thread_Command_glClear *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glClear(mask);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glClear *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glClear));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->mask = mask;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glClear,
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

} EVGL_Thread_Command_glClearColor;

static void
_evgl_thread_glClearColor(void *data)
{
   EVGL_Thread_Command_glClearColor *thread_data =
      (EVGL_Thread_Command_glClearColor *)data;

   glClearColor(thread_data->red,
                thread_data->green,
                thread_data->blue,
                thread_data->alpha);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glClearColor_evgl_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glClearColor thread_data_local;
   EVGL_Thread_Command_glClearColor *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glClearColor(red, green, blue, alpha);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glClearColor *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glClearColor));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glClearColor,
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

} EVGL_Thread_Command_glReadPixels;

static void
_evgl_thread_glReadPixels(void *data)
{
   EVGL_Thread_Command_glReadPixels *thread_data =
      (EVGL_Thread_Command_glReadPixels *)data;

   glReadPixels(thread_data->x,
                thread_data->y,
                thread_data->width,
                thread_data->height,
                thread_data->format,
                thread_data->type,
                thread_data->pixels);

}

EAPI void
evas_glReadPixels_evgl_th(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glReadPixels thread_data_local;
   EVGL_Thread_Command_glReadPixels *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glReadPixels,
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

} EVGL_Thread_Command_glGenFramebuffers;

static void
_evgl_thread_glGenFramebuffers(void *data)
{
   EVGL_Thread_Command_glGenFramebuffers *thread_data =
      (EVGL_Thread_Command_glGenFramebuffers *)data;

   glGenFramebuffers(thread_data->n,
                     thread_data->framebuffers);

}

EAPI void
evas_glGenFramebuffers_evgl_th(GLsizei n, GLuint *framebuffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGenFramebuffers thread_data_local;
   EVGL_Thread_Command_glGenFramebuffers *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGenFramebuffers(n, framebuffers);
        return;
     }

   thread_data->n = n;
   thread_data->framebuffers = framebuffers;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGenFramebuffers,
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

} EVGL_Thread_Command_glBindFramebuffer;

static void
_evgl_thread_glBindFramebuffer(void *data)
{
   EVGL_Thread_Command_glBindFramebuffer *thread_data =
      (EVGL_Thread_Command_glBindFramebuffer *)data;

   glBindFramebuffer(thread_data->target,
                     thread_data->framebuffer);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBindFramebuffer_evgl_th(GLenum target, GLuint framebuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glBindFramebuffer thread_data_local;
   EVGL_Thread_Command_glBindFramebuffer *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glBindFramebuffer(target, framebuffer);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glBindFramebuffer *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glBindFramebuffer));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->framebuffer = framebuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glBindFramebuffer,
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

} EVGL_Thread_Command_glDeleteFramebuffers;

static void
_evgl_thread_glDeleteFramebuffers(void *data)
{
   EVGL_Thread_Command_glDeleteFramebuffers *thread_data =
      (EVGL_Thread_Command_glDeleteFramebuffers *)data;

   glDeleteFramebuffers(thread_data->n,
                        thread_data->framebuffers);


   if (thread_data->framebuffers_copied)
     eina_mempool_free(_mp_delete_object, thread_data->framebuffers_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteFramebuffers_evgl_th(GLsizei n, const GLuint *framebuffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDeleteFramebuffers thread_data_local;
   EVGL_Thread_Command_glDeleteFramebuffers *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDeleteFramebuffers(n, framebuffers);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDeleteFramebuffers *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDeleteFramebuffers));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDeleteFramebuffers,
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

} EVGL_Thread_Command_glGenRenderbuffers;

static void
_evgl_thread_glGenRenderbuffers(void *data)
{
   EVGL_Thread_Command_glGenRenderbuffers *thread_data =
      (EVGL_Thread_Command_glGenRenderbuffers *)data;

   glGenRenderbuffers(thread_data->n,
                      thread_data->renderbuffers);

}

EAPI void
evas_glGenRenderbuffers_evgl_th(GLsizei n, GLuint *renderbuffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGenRenderbuffers thread_data_local;
   EVGL_Thread_Command_glGenRenderbuffers *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGenRenderbuffers(n, renderbuffers);
        return;
     }

   thread_data->n = n;
   thread_data->renderbuffers = renderbuffers;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGenRenderbuffers,
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

} EVGL_Thread_Command_glBindRenderbuffer;

static void
_evgl_thread_glBindRenderbuffer(void *data)
{
   EVGL_Thread_Command_glBindRenderbuffer *thread_data =
      (EVGL_Thread_Command_glBindRenderbuffer *)data;

   glBindRenderbuffer(thread_data->target,
                      thread_data->renderbuffer);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glBindRenderbuffer_evgl_th(GLenum target, GLuint renderbuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glBindRenderbuffer thread_data_local;
   EVGL_Thread_Command_glBindRenderbuffer *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glBindRenderbuffer(target, renderbuffer);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glBindRenderbuffer *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glBindRenderbuffer));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->renderbuffer = renderbuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glBindRenderbuffer,
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

} EVGL_Thread_Command_glDeleteRenderbuffers;

static void
_evgl_thread_glDeleteRenderbuffers(void *data)
{
   EVGL_Thread_Command_glDeleteRenderbuffers *thread_data =
      (EVGL_Thread_Command_glDeleteRenderbuffers *)data;

   glDeleteRenderbuffers(thread_data->n,
                         thread_data->renderbuffers);


   if (thread_data->renderbuffers_copied)
     eina_mempool_free(_mp_delete_object, thread_data->renderbuffers_copied);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glDeleteRenderbuffers_evgl_th(GLsizei n, const GLuint *renderbuffers)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDeleteRenderbuffers thread_data_local;
   EVGL_Thread_Command_glDeleteRenderbuffers *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDeleteRenderbuffers(n, renderbuffers);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glDeleteRenderbuffers *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glDeleteRenderbuffers));
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
   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDeleteRenderbuffers,
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

} EVGL_Thread_Command_glRenderbufferStorage;

static void
_evgl_thread_glRenderbufferStorage(void *data)
{
   EVGL_Thread_Command_glRenderbufferStorage *thread_data =
      (EVGL_Thread_Command_glRenderbufferStorage *)data;

   glRenderbufferStorage(thread_data->target,
                         thread_data->internalformat,
                         thread_data->width,
                         thread_data->height);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glRenderbufferStorage_evgl_th(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glRenderbufferStorage thread_data_local;
   EVGL_Thread_Command_glRenderbufferStorage *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glRenderbufferStorage(target, internalformat, width, height);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glRenderbufferStorage *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glRenderbufferStorage));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glRenderbufferStorage,
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

} EVGL_Thread_Command_glFramebufferRenderbuffer;

static void
_evgl_thread_glFramebufferRenderbuffer(void *data)
{
   EVGL_Thread_Command_glFramebufferRenderbuffer *thread_data =
      (EVGL_Thread_Command_glFramebufferRenderbuffer *)data;

   glFramebufferRenderbuffer(thread_data->target,
                             thread_data->attachment,
                             thread_data->renderbuffertarget,
                             thread_data->renderbuffer);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glFramebufferRenderbuffer_evgl_th(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glFramebufferRenderbuffer thread_data_local;
   EVGL_Thread_Command_glFramebufferRenderbuffer *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glFramebufferRenderbuffer *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glFramebufferRenderbuffer));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glFramebufferRenderbuffer,
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

} EVGL_Thread_Command_glFramebufferTexture2D;

static void
_evgl_thread_glFramebufferTexture2D(void *data)
{
   EVGL_Thread_Command_glFramebufferTexture2D *thread_data =
      (EVGL_Thread_Command_glFramebufferTexture2D *)data;

   glFramebufferTexture2D(thread_data->target,
                          thread_data->attachment,
                          thread_data->textarget,
                          thread_data->texture,
                          thread_data->level);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glFramebufferTexture2D_evgl_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glFramebufferTexture2D thread_data_local;
   EVGL_Thread_Command_glFramebufferTexture2D *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glFramebufferTexture2D(target, attachment, textarget, texture, level);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glFramebufferTexture2D *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glFramebufferTexture2D));
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

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glFramebufferTexture2D,
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

} EVGL_Thread_Command_glCheckFramebufferStatus;

static void
_evgl_thread_glCheckFramebufferStatus(void *data)
{
   EVGL_Thread_Command_glCheckFramebufferStatus *thread_data =
      (EVGL_Thread_Command_glCheckFramebufferStatus *)data;

   thread_data->return_value = glCheckFramebufferStatus(thread_data->target);

}

EAPI GLenum
evas_glCheckFramebufferStatus_evgl_th(GLenum target)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glCheckFramebufferStatus thread_data_local;
   EVGL_Thread_Command_glCheckFramebufferStatus *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glCheckFramebufferStatus(target);
     }

   thread_data->target = target;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glCheckFramebufferStatus,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glFlush(void);
 */

static void
_evgl_thread_glFlush(void *data EINA_UNUSED)
{
   glFlush();

}

EAPI void
evas_glFlush_evgl_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   if (!evas_evgl_thread_enabled())
     {
        glFlush();
        return;
     }

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glFlush,
                              NULL,
                              thread_mode);
}

/*
 * void
 * glFinish(void);
 */

static void
_evgl_thread_glFinish(void *data EINA_UNUSED)
{
   glFinish();

}

EAPI void
evas_glFinish_evgl_th(void)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   if (!evas_evgl_thread_enabled())
     {
        glFinish();
        return;
     }

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glFinish,
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

} EVGL_Thread_Command_glHint;

static void
_evgl_thread_glHint(void *data)
{
   EVGL_Thread_Command_glHint *thread_data =
      (EVGL_Thread_Command_glHint *)data;

   glHint(thread_data->target,
          thread_data->mode);

   if (thread_data->command_allocated)
     eina_mempool_free(_mp_command, thread_data);
}

EAPI void
evas_glHint_evgl_th(GLenum target, GLenum mode)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glHint thread_data_local;
   EVGL_Thread_Command_glHint *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glHint(target, mode);
        return;
     }

   /* command_allocated flag init. */
   thread_data->command_allocated = 0;

   if (!evas_gl_thread_force_finish())
     { /* _flush */
        EVGL_Thread_Command_glHint *thread_data_new;
        thread_data_new = eina_mempool_malloc(_mp_command,
                                              sizeof(EVGL_Thread_Command_glHint));
        if (thread_data_new)
          {
             thread_data = thread_data_new;
             thread_data->command_allocated = 1;
             thread_mode = EVAS_GL_THREAD_MODE_FLUSH;
          }
     }

   thread_data->target = target;
   thread_data->mode = mode;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glHint,
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

} EVGL_Thread_Command_glGetString;

static void
_evgl_thread_glGetString(void *data)
{
   EVGL_Thread_Command_glGetString *thread_data =
      (EVGL_Thread_Command_glGetString *)data;

   thread_data->return_value = glGetString(thread_data->name);

}

EAPI const GLubyte *
evas_glGetString_evgl_th(GLenum name)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetString thread_data_local;
   EVGL_Thread_Command_glGetString *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glGetString(name);
     }

   thread_data->name = name;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetString,
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

} EVGL_Thread_Command_glGetBooleanv;

static void
_evgl_thread_glGetBooleanv(void *data)
{
   EVGL_Thread_Command_glGetBooleanv *thread_data =
      (EVGL_Thread_Command_glGetBooleanv *)data;

   glGetBooleanv(thread_data->pname,
                 thread_data->data);

}

EAPI void
evas_glGetBooleanv_evgl_th(GLenum pname, GLboolean *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetBooleanv thread_data_local;
   EVGL_Thread_Command_glGetBooleanv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetBooleanv(pname, data);
        return;
     }

   thread_data->pname = pname;
   thread_data->data = data;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetBooleanv,
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

} EVGL_Thread_Command_glGetFloatv;

static void
_evgl_thread_glGetFloatv(void *data)
{
   EVGL_Thread_Command_glGetFloatv *thread_data =
      (EVGL_Thread_Command_glGetFloatv *)data;

   glGetFloatv(thread_data->pname,
               thread_data->data);

}

EAPI void
evas_glGetFloatv_evgl_th(GLenum pname, GLfloat *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetFloatv thread_data_local;
   EVGL_Thread_Command_glGetFloatv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetFloatv(pname, data);
        return;
     }

   thread_data->pname = pname;
   thread_data->data = data;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetFloatv,
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

} EVGL_Thread_Command_glGetIntegerv;

static void
_evgl_thread_glGetIntegerv(void *data)
{
   EVGL_Thread_Command_glGetIntegerv *thread_data =
      (EVGL_Thread_Command_glGetIntegerv *)data;

   glGetIntegerv(thread_data->pname,
                 thread_data->data);

}

EAPI void
evas_glGetIntegerv_evgl_th(GLenum pname, GLint *data)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetIntegerv thread_data_local;
   EVGL_Thread_Command_glGetIntegerv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetIntegerv(pname, data);
        return;
     }

   thread_data->pname = pname;
   thread_data->data = data;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetIntegerv,
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

} EVGL_Thread_Command_glIsBuffer;

static void
_evgl_thread_glIsBuffer(void *data)
{
   EVGL_Thread_Command_glIsBuffer *thread_data =
      (EVGL_Thread_Command_glIsBuffer *)data;

   thread_data->return_value = glIsBuffer(thread_data->buffer);

}

EAPI GLboolean
evas_glIsBuffer_evgl_th(GLint buffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glIsBuffer thread_data_local;
   EVGL_Thread_Command_glIsBuffer *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glIsBuffer(buffer);
     }

   thread_data->buffer = buffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glIsBuffer,
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

} EVGL_Thread_Command_glGetBufferParameteriv;

static void
_evgl_thread_glGetBufferParameteriv(void *data)
{
   EVGL_Thread_Command_glGetBufferParameteriv *thread_data =
      (EVGL_Thread_Command_glGetBufferParameteriv *)data;

   glGetBufferParameteriv(thread_data->target,
                          thread_data->pname,
                          thread_data->params);

}

EAPI void
evas_glGetBufferParameteriv_evgl_th(GLenum target, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetBufferParameteriv thread_data_local;
   EVGL_Thread_Command_glGetBufferParameteriv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetBufferParameteriv(target, pname, params);
        return;
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetBufferParameteriv,
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

} EVGL_Thread_Command_glIsShader;

static void
_evgl_thread_glIsShader(void *data)
{
   EVGL_Thread_Command_glIsShader *thread_data =
      (EVGL_Thread_Command_glIsShader *)data;

   thread_data->return_value = glIsShader(thread_data->shader);

}

EAPI GLboolean
evas_glIsShader_evgl_th(GLuint shader)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glIsShader thread_data_local;
   EVGL_Thread_Command_glIsShader *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glIsShader(shader);
     }

   thread_data->shader = shader;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glIsShader,
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

} EVGL_Thread_Command_glGetShaderiv;

static void
_evgl_thread_glGetShaderiv(void *data)
{
   EVGL_Thread_Command_glGetShaderiv *thread_data =
      (EVGL_Thread_Command_glGetShaderiv *)data;

   glGetShaderiv(thread_data->shader,
                 thread_data->pname,
                 thread_data->params);

}

EAPI void
evas_glGetShaderiv_evgl_th(GLuint shader, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetShaderiv thread_data_local;
   EVGL_Thread_Command_glGetShaderiv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetShaderiv(shader, pname, params);
        return;
     }

   thread_data->shader = shader;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetShaderiv,
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

} EVGL_Thread_Command_glGetAttachedShaders;

static void
_evgl_thread_glGetAttachedShaders(void *data)
{
   EVGL_Thread_Command_glGetAttachedShaders *thread_data =
      (EVGL_Thread_Command_glGetAttachedShaders *)data;

   glGetAttachedShaders(thread_data->program,
                        thread_data->maxCount,
                        thread_data->count,
                        thread_data->shaders);

}

EAPI void
evas_glGetAttachedShaders_evgl_th(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetAttachedShaders thread_data_local;
   EVGL_Thread_Command_glGetAttachedShaders *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetAttachedShaders(program, maxCount, count, shaders);
        return;
     }

   thread_data->program = program;
   thread_data->maxCount = maxCount;
   thread_data->count = count;
   thread_data->shaders = shaders;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetAttachedShaders,
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

} EVGL_Thread_Command_glGetShaderInfoLog;

static void
_evgl_thread_glGetShaderInfoLog(void *data)
{
   EVGL_Thread_Command_glGetShaderInfoLog *thread_data =
      (EVGL_Thread_Command_glGetShaderInfoLog *)data;

   glGetShaderInfoLog(thread_data->shader,
                      thread_data->bufSize,
                      thread_data->length,
                      thread_data->infoLog);

}

EAPI void
evas_glGetShaderInfoLog_evgl_th(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetShaderInfoLog thread_data_local;
   EVGL_Thread_Command_glGetShaderInfoLog *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetShaderInfoLog(shader, bufSize, length, infoLog);
        return;
     }

   thread_data->shader = shader;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->infoLog = infoLog;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetShaderInfoLog,
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

} EVGL_Thread_Command_glGetShaderSource;

static void
_evgl_thread_glGetShaderSource(void *data)
{
   EVGL_Thread_Command_glGetShaderSource *thread_data =
      (EVGL_Thread_Command_glGetShaderSource *)data;

   glGetShaderSource(thread_data->shader,
                     thread_data->bufSize,
                     thread_data->length,
                     thread_data->source);

}

EAPI void
evas_glGetShaderSource_evgl_th(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetShaderSource thread_data_local;
   EVGL_Thread_Command_glGetShaderSource *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetShaderSource(shader, bufSize, length, source);
        return;
     }

   thread_data->shader = shader;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->source = source;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetShaderSource,
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

} EVGL_Thread_Command_glGetShaderPrecisionFormat;

static void
_evgl_thread_glGetShaderPrecisionFormat(void *data)
{
   EVGL_Thread_Command_glGetShaderPrecisionFormat *thread_data =
      (EVGL_Thread_Command_glGetShaderPrecisionFormat *)data;

   glGetShaderPrecisionFormat(thread_data->shadertype,
                              thread_data->precisiontype,
                              thread_data->range,
                              thread_data->precision);

}

EAPI void
evas_glGetShaderPrecisionFormat_evgl_th(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetShaderPrecisionFormat thread_data_local;
   EVGL_Thread_Command_glGetShaderPrecisionFormat *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
        return;
     }

   thread_data->shadertype = shadertype;
   thread_data->precisiontype = precisiontype;
   thread_data->range = range;
   thread_data->precision = precision;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetShaderPrecisionFormat,
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

} EVGL_Thread_Command_glGetVertexAttribfv;

static void
_evgl_thread_glGetVertexAttribfv(void *data)
{
   EVGL_Thread_Command_glGetVertexAttribfv *thread_data =
      (EVGL_Thread_Command_glGetVertexAttribfv *)data;

   glGetVertexAttribfv(thread_data->index,
                       thread_data->pname,
                       thread_data->params);

}

EAPI void
evas_glGetVertexAttribfv_evgl_th(GLuint index, GLenum pname, GLfloat *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetVertexAttribfv thread_data_local;
   EVGL_Thread_Command_glGetVertexAttribfv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetVertexAttribfv(index, pname, params);
        return;
     }

   thread_data->index = index;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetVertexAttribfv,
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

} EVGL_Thread_Command_glGetVertexAttribiv;

static void
_evgl_thread_glGetVertexAttribiv(void *data)
{
   EVGL_Thread_Command_glGetVertexAttribiv *thread_data =
      (EVGL_Thread_Command_glGetVertexAttribiv *)data;

   glGetVertexAttribiv(thread_data->index,
                       thread_data->pname,
                       thread_data->params);

}

EAPI void
evas_glGetVertexAttribiv_evgl_th(GLuint index, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetVertexAttribiv thread_data_local;
   EVGL_Thread_Command_glGetVertexAttribiv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetVertexAttribiv(index, pname, params);
        return;
     }

   thread_data->index = index;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetVertexAttribiv,
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

} EVGL_Thread_Command_glIsProgram;

static void
_evgl_thread_glIsProgram(void *data)
{
   EVGL_Thread_Command_glIsProgram *thread_data =
      (EVGL_Thread_Command_glIsProgram *)data;

   thread_data->return_value = glIsProgram(thread_data->program);

}

EAPI GLboolean
evas_glIsProgram_evgl_th(GLuint program)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glIsProgram thread_data_local;
   EVGL_Thread_Command_glIsProgram *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glIsProgram(program);
     }

   thread_data->program = program;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glIsProgram,
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

} EVGL_Thread_Command_glGetProgramInfoLog;

static void
_evgl_thread_glGetProgramInfoLog(void *data)
{
   EVGL_Thread_Command_glGetProgramInfoLog *thread_data =
      (EVGL_Thread_Command_glGetProgramInfoLog *)data;

   glGetProgramInfoLog(thread_data->program,
                       thread_data->bufSize,
                       thread_data->length,
                       thread_data->infoLog);

}

EAPI void
evas_glGetProgramInfoLog_evgl_th(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetProgramInfoLog thread_data_local;
   EVGL_Thread_Command_glGetProgramInfoLog *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetProgramInfoLog(program, bufSize, length, infoLog);
        return;
     }

   thread_data->program = program;
   thread_data->bufSize = bufSize;
   thread_data->length = length;
   thread_data->infoLog = infoLog;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetProgramInfoLog,
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

} EVGL_Thread_Command_glGetProgramiv;

static void
_evgl_thread_glGetProgramiv(void *data)
{
   EVGL_Thread_Command_glGetProgramiv *thread_data =
      (EVGL_Thread_Command_glGetProgramiv *)data;

   glGetProgramiv(thread_data->program,
                  thread_data->pname,
                  thread_data->params);

}

EAPI void
evas_glGetProgramiv_evgl_th(GLuint program, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetProgramiv thread_data_local;
   EVGL_Thread_Command_glGetProgramiv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetProgramiv(program, pname, params);
        return;
     }

   thread_data->program = program;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetProgramiv,
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

} EVGL_Thread_Command_glIsFramebuffer;

static void
_evgl_thread_glIsFramebuffer(void *data)
{
   EVGL_Thread_Command_glIsFramebuffer *thread_data =
      (EVGL_Thread_Command_glIsFramebuffer *)data;

   thread_data->return_value = glIsFramebuffer(thread_data->framebuffer);

}

EAPI GLboolean
evas_glIsFramebuffer_evgl_th(GLint framebuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glIsFramebuffer thread_data_local;
   EVGL_Thread_Command_glIsFramebuffer *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glIsFramebuffer(framebuffer);
     }

   thread_data->framebuffer = framebuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glIsFramebuffer,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * GLboolean
 * glIsRenderbuffer(GLint renderbuffer);
 */

typedef struct
{
   GLboolean return_value;
   GLint renderbuffer;

} EVGL_Thread_Command_glIsRenderbuffer;

static void
_evgl_thread_glIsRenderbuffer(void *data)
{
   EVGL_Thread_Command_glIsRenderbuffer *thread_data =
      (EVGL_Thread_Command_glIsRenderbuffer *)data;

   thread_data->return_value = glIsRenderbuffer(thread_data->renderbuffer);

}

EAPI GLboolean
evas_glIsRenderbuffer_evgl_th(GLint renderbuffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glIsRenderbuffer thread_data_local;
   EVGL_Thread_Command_glIsRenderbuffer *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glIsRenderbuffer(renderbuffer);
     }

   thread_data->renderbuffer = renderbuffer;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glIsRenderbuffer,
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

} EVGL_Thread_Command_glGetRenderbufferParameteriv;

static void
_evgl_thread_glGetRenderbufferParameteriv(void *data)
{
   EVGL_Thread_Command_glGetRenderbufferParameteriv *thread_data =
      (EVGL_Thread_Command_glGetRenderbufferParameteriv *)data;

   glGetRenderbufferParameteriv(thread_data->target,
                                thread_data->pname,
                                thread_data->params);

}

EAPI void
evas_glGetRenderbufferParameteriv_evgl_th(GLenum target, GLenum pname, GLint *params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetRenderbufferParameteriv thread_data_local;
   EVGL_Thread_Command_glGetRenderbufferParameteriv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetRenderbufferParameteriv(target, pname, params);
        return;
     }

   thread_data->target = target;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetRenderbufferParameteriv,
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

} EVGL_Thread_Command_glIsTexture;

static void
_evgl_thread_glIsTexture(void *data)
{
   EVGL_Thread_Command_glIsTexture *thread_data =
      (EVGL_Thread_Command_glIsTexture *)data;

   thread_data->return_value = glIsTexture(thread_data->texture);

}

EAPI GLboolean
evas_glIsTexture_evgl_th(GLint texture)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glIsTexture thread_data_local;
   EVGL_Thread_Command_glIsTexture *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        return glIsTexture(texture);
     }

   thread_data->texture = texture;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glIsTexture,
                              thread_data,
                              thread_mode);

   return thread_data->return_value;
}

/*
 * void
 * glClearDepthf(GLclampf depth);
 */

typedef struct
{
   GLclampf depth;

} EVGL_Thread_Command_glClearDepthf;

static void
_evgl_thread_glClearDepthf(void *data)
{
   EVGL_Thread_Command_glClearDepthf *thread_data =
      (EVGL_Thread_Command_glClearDepthf *)data;

   glClearDepthf(thread_data->depth);

}

EAPI void
evas_glClearDepthf_evgl_th(GLclampf depth)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glClearDepthf thread_data_local;
   EVGL_Thread_Command_glClearDepthf *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glClearDepthf(depth);
        return;
     }

   thread_data->depth = depth;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glClearDepthf,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDepthRangef(GLclampf zNear, GLclampf zFar);
 */

typedef struct
{
   GLclampf zNear;
   GLclampf zFar;

} EVGL_Thread_Command_glDepthRangef;

static void
_evgl_thread_glDepthRangef(void *data)
{
   EVGL_Thread_Command_glDepthRangef *thread_data =
      (EVGL_Thread_Command_glDepthRangef *)data;

   glDepthRangef(thread_data->zNear,
                 thread_data->zFar);

}

EAPI void
evas_glDepthRangef_evgl_th(GLclampf zNear, GLclampf zFar)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDepthRangef thread_data_local;
   EVGL_Thread_Command_glDepthRangef *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDepthRangef(zNear, zFar);
        return;
     }

   thread_data->zNear = zNear;
   thread_data->zFar = zFar;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDepthRangef,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glClearDepth(GLclampf depth);
 */

typedef struct
{
   GLclampf depth;

} EVGL_Thread_Command_glClearDepth;

static void
_evgl_thread_glClearDepth(void *data)
{
   EVGL_Thread_Command_glClearDepth *thread_data =
      (EVGL_Thread_Command_glClearDepth *)data;

   glClearDepth(thread_data->depth);

}

EAPI void
evas_glClearDepth_evgl_th(GLclampf depth)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glClearDepth thread_data_local;
   EVGL_Thread_Command_glClearDepth *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glClearDepth(depth);
        return;
     }

   thread_data->depth = depth;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glClearDepth,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glDepthRange(GLclampf zNear, GLclampf zFar);
 */

typedef struct
{
   GLclampf zNear;
   GLclampf zFar;

} EVGL_Thread_Command_glDepthRange;

static void
_evgl_thread_glDepthRange(void *data)
{
   EVGL_Thread_Command_glDepthRange *thread_data =
      (EVGL_Thread_Command_glDepthRange *)data;

   glDepthRange(thread_data->zNear,
                thread_data->zFar);

}

EAPI void
evas_glDepthRange_evgl_th(GLclampf zNear, GLclampf zFar)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glDepthRange thread_data_local;
   EVGL_Thread_Command_glDepthRange *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glDepthRange(zNear, zFar);
        return;
     }

   thread_data->zNear = zNear;
   thread_data->zFar = zFar;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glDepthRange,
                              thread_data,
                              thread_mode);
}

/*
 * void
 * glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params);
 */

typedef struct
{
   GLenum target;
   GLenum attachment;
   GLenum pname;
   GLint* params;

} EVGL_Thread_Command_glGetFramebufferAttachmentParameteriv;

static void
_evgl_thread_glGetFramebufferAttachmentParameteriv(void *data)
{
   EVGL_Thread_Command_glGetFramebufferAttachmentParameteriv *thread_data =
      (EVGL_Thread_Command_glGetFramebufferAttachmentParameteriv *)data;

   glGetFramebufferAttachmentParameteriv(thread_data->target,
                                         thread_data->attachment,
                                         thread_data->pname,
                                         thread_data->params);

}

EAPI void
evas_glGetFramebufferAttachmentParameteriv_evgl_th(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;

   EVGL_Thread_Command_glGetFramebufferAttachmentParameteriv thread_data_local;
   EVGL_Thread_Command_glGetFramebufferAttachmentParameteriv *thread_data = &thread_data_local;

   if (!evas_evgl_thread_enabled())
     {
        glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
        return;
     }

   thread_data->target = target;
   thread_data->attachment = attachment;
   thread_data->pname = pname;
   thread_data->params = params;

   evas_gl_thread_cmd_enqueue(EVAS_GL_THREAD_TYPE_EVGL,
                              _evgl_thread_glGetFramebufferAttachmentParameteriv,
                              thread_data,
                              thread_mode);
}
