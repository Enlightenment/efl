#include "evas_gl_common.h"

typedef struct
{
   void *(*gl_generated_func_get)(void);

   void (*GL_TH_FN(glTexSubImage2DEVAS))(int thread_push, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
} Evas_GL_Thread_GL_Func;

#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC

#define GLSHADERSOURCE_VARIABLE_DECLARE \
   GLchar **string_copied;

#define GLSHADERSOURCE_GLCALL_BEFORE
#define GLSHADERSOURCE_GLCALL_AFTER \
   if (thread_data->string_copied) \
     { \
        int i; \
        for (i = 0; i < thread_data->count; i++) \
          { \
             if (thread_data->string_copied[i]) \
                eina_mempool_free(_mp_default, thread_data->string_copied[i]); \
          }\
        eina_mempool_free(_mp_default, thread_data->string_copied); \
     }

#define GLSHADERSOURCE_VARIABLE_INIT \
   thread_data->string_copied = NULL;

#define GLSHADERSOURCE_ASYNC_PREPARE \
   if (string) \
     { \
        /* 1. check memory size */ \
        if ((unsigned int)(sizeof(char *) * count) > _mp_default_memory_size) \
          { \
             thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
             goto finish; \
          } \
        int i, len = 0; \
        for (i = 0; i < count; i++) \
          { \
             if (length) len = length[i]; \
             else        len = strlen(string[i]); \
             if ((unsigned int)len + 1 > _mp_default_memory_size) \
               { \
                  thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
                  goto finish; \
               } \
          } \
        /* 2. malloc & copy */ \
        thread_data->string_copied = eina_mempool_malloc(_mp_default, sizeof(char *) * count); \
        if (thread_data->string_copied) \
          { \
             memset(thread_data->string_copied, 0x00, sizeof(char *) * count); \
             for (i = 0, len = 0; i < count; i++) \
               { \
                  if (length) len = length[i]; \
                  else        len = strlen(string[i]); \
                  thread_data->string_copied[i] = eina_mempool_malloc(_mp_default, len + 1); \
                  if (thread_data->string_copied[i]) \
                    { \
                       memcpy(thread_data->string_copied[i], string[i], len + 1); \
                    } \
                  else \
                    { \
                       thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
                       goto finish; \
                    } \
               } \
          } \
        else \
          { \
             thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
             goto finish; \
          } \
        /* 3. replace */ \
        thread_data->string = (const GLchar **)thread_data->string_copied; \
     }

#define GLSHADERSOURCE_ENQUEUE_BEFORE
#define GLSHADERSOURCE_ENQUEUE_AFTER

/* Unpack Caches */
static GLint   _cache_glPixelStorei_unpack_row_length = 0;
static GLint   _cache_glPixelStorei_unpack_alignment  = 4;

/* Buffer Cache */
static GLuint  _cache_glBindBuffer_pixel_unpack_buffer_idx = 0;

/*
 * n : number of elements in a group (comp)
 * l : number of groups in the row (width)
 *     if   UNPACK_ROW_LENGTH is 0, l is width
 *     else l = UNPACK_ROW_LENGTH
 * a : value of UNPACK_ALIGNMENT (1,2,4, or 8)
 * s : size, in units of GL ubytes, of an element (csize)
 *
 * if, (s >= a)
 *    k = n * l
 * else if (s < a)
 *    k = (a / s) * ceil( (s * n * l)/a )
 *
 */

static int
get_size(GLenum format, GLenum type, GLsizei width, GLsizei height)
{
   int csize = 0, comp = 0, k =0;
   int l = _cache_glPixelStorei_unpack_row_length;
   int a = _cache_glPixelStorei_unpack_alignment;

   if (l == 0) l = width;

   switch (type)
    {
#ifdef GL_UNSIGNED_BYTE_3_3_2
      case GL_UNSIGNED_BYTE_3_3_2:         csize = sizeof(GLubyte); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_BYTE_2_3_3
      case GL_UNSIGNED_BYTE_2_3_3:         csize = sizeof(GLubyte); comp = 1; break;
#endif

#ifdef GL_UNSIGNED_SHORT_5_6_5
      case GL_UNSIGNED_SHORT_5_6_5:        csize = sizeof(GLushort); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_SHORT_5_6_5_REV
      case GL_UNSIGNED_SHORT_5_6_5_REV:    csize = sizeof(GLushort); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_SHORT_4_4_4_4
      case GL_UNSIGNED_SHORT_4_4_4_4:      csize = sizeof(GLushort); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_SHORT_4_4_4_4_REV
      case GL_UNSIGNED_SHORT_4_4_4_4_REV:  csize = sizeof(GLushort); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_SHORT_5_5_5_1
      case GL_UNSIGNED_SHORT_5_5_5_1:      csize = sizeof(GLushort); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_SHORT_1_5_5_5_REV
      case GL_UNSIGNED_SHORT_1_5_5_5_REV:  csize = sizeof(GLushort); comp = 1; break;
#endif

#ifdef GL_UNSIGNED_INT_8_8_8_8
      case GL_UNSIGNED_INT_8_8_8_8:        csize = sizeof(GLuint); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_INT_8_8_8_8_REV
      case GL_UNSIGNED_INT_8_8_8_8_REV:    csize = sizeof(GLuint); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_INT_10_10_10_2
      case GL_UNSIGNED_INT_10_10_10_2:     csize = sizeof(GLuint); comp = 1; break;
#endif
#ifdef GL_UNSIGNED_INT_2_10_10_10_REV
      case GL_UNSIGNED_INT_2_10_10_10_REV: csize = sizeof(GLuint); comp = 1; break;
#endif

      case GL_UNSIGNED_BYTE:
      case GL_BYTE:
         csize = sizeof(GLbyte);
         break;

      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
         csize = sizeof(GLshort);
         break;

      case GL_UNSIGNED_INT:
      case GL_INT:
         csize = sizeof(GLint);
         break;

      case GL_FLOAT:
         csize = sizeof(GLfloat);
         break;

      default:
         ERR("Not supported type for TexSubImage2D Threading : 0x%X", type);
         return -1;
    }

   if (comp == 0)
    {
      switch (format)
        {
          case GL_RED:
          case GL_RED_INTEGER:
#ifdef GL_R8_EXT
          case GL_R8_EXT:
#endif
#ifdef GL_R16_EXT
          case GL_R16_EXT:
#endif
          case GL_ALPHA:
          case GL_LUMINANCE:
            comp = 1;
            break;

          case GL_RG:
          case GL_RG_INTEGER:
#ifdef GL_RG8_EXT
          case GL_RG8_EXT:
#endif
#ifdef GL_RG16_EXT
          case GL_RG16_EXT:
#endif
          case GL_LUMINANCE_ALPHA:
            comp = 2;
            break;

          case GL_RGB:
          case GL_RGB_INTEGER:
          case GL_BGR:
#ifdef GL_BGR_INTEGER
          case GL_BGR_INTEGER:
#endif
            comp = 3;
            break;

          case GL_RGBA:
          case GL_BGRA:
          case GL_RGBA_INTEGER:
#ifdef GL_BGRA8_EXT
          case GL_BGRA8_EXT:
#endif
#ifdef GL_BGRA_INTEGER
          case GL_BGRA_INTEGER:
#endif
            comp = 4;
            break;

          default:
            ERR("Not supported format for TexSubImage2D Threading : 0x%X", format);
            return -1;
        }
    }

   if (csize >= a)
      k = comp * l;
   else /* csize < a */
      k = (a / csize) * ceil( (csize * comp * l) / (double)a);

   return k * height;
}

#define GLPIXELSTOREI_VARIABLE_DECLARE \
   ;
#define GLPIXELSTOREI_GLCALL_BEFORE
#define GLPIXELSTOREI_GLCALL_AFTER
#define GLPIXELSTOREI_VARIABLE_INIT
#define GLPIXELSTOREI_ASYNC_PREPARE
#define GLPIXELSTOREI_ENQUEUE_BEFORE \
   switch (pname) \
     { \
        case GL_UNPACK_ROW_LENGTH: \
         if (param >= 0) _cache_glPixelStorei_unpack_row_length = param; \
         break; \
        case GL_UNPACK_ALIGNMENT: \
           if (param == 1 || param == 2 || param == 4 || param == 8) \
              _cache_glPixelStorei_unpack_alignment = param; \
           break; \
     }

#define GLPIXELSTOREI_ENQUEUE_AFTER

#define GLBINDBUFFER_VARIABLE_DECLARE \
   ;
#define GLBINDBUFFER_GLCALL_BEFORE
#define GLBINDBUFFER_GLCALL_AFTER
#define GLBINDBUFFER_VARIABLE_INIT
#define GLBINDBUFFER_ASYNC_PREPARE
#define GLBINDBUFFER_ENQUEUE_BEFORE \
   /* pixel unpack buffer id caching */ \
   if (target == GL_PIXEL_UNPACK_BUFFER) \
     { \
        _cache_glBindBuffer_pixel_unpack_buffer_idx = buffer; \
     }
#define GLBINDBUFFER_ENQUEUE_AFTER



#define GLTEXIMAGE2D_VARIABLE_DECLARE \
   void *pixels_copied;

#define GLTEXIMAGE2D_GLCALL_BEFORE
#define GLTEXIMAGE2D_GLCALL_AFTER \
   if (thread_data->pixels_copied) \
      eina_mempool_free(_mp_texture, thread_data->pixels_copied);

#define GLTEXIMAGE2D_VARIABLE_INIT \
   thread_data->pixels_copied = NULL;

#define GLTEXIMAGE2D_ASYNC_PREPARE \
   /* 1. check pixel_ubpack_buffer_index */ \
   if (_cache_glBindBuffer_pixel_unpack_buffer_idx == 0) \
     { \
       /* 2. check memory size */ \
       int copy_size = get_size(format, type, width + (border *2), height + (border *2)); \
          if (copy_size < 0 || (unsigned int)copy_size > _mp_texture_memory_size) \
            { \
               thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
               goto finish; \
            } \
          if (pixels) \
            { \
               /* 3. malloc & copy */ \
               thread_data->pixels_copied = eina_mempool_malloc(_mp_texture, copy_size); \
               if (thread_data->pixels_copied) \
                 { \
                    memcpy(thread_data->pixels_copied, pixels, copy_size); \
                 } \
               else \
                 { \
                    thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
                    goto finish; \
                 } \
               /* 3. replace */ \
               thread_data->pixels = (const void *)thread_data->pixels_copied; \
            } \
     }

#define GLTEXIMAGE2D_ENQUEUE_BEFORE
#define GLTEXIMAGE2D_ENQUEUE_AFTER

#define GLTEXSUBIMAGE2D_VARIABLE_DECLARE \
   void *pixels_copied;

#define GLTEXSUBIMAGE2D_GLCALL_BEFORE
#define GLTEXSUBIMAGE2D_GLCALL_AFTER \
   if (thread_data->pixels_copied) \
      eina_mempool_free(_mp_texture, thread_data->pixels_copied);

#define GLTEXSUBIMAGE2D_VARIABLE_INIT \
   thread_data->pixels_copied = NULL;

#define GLTEXSUBIMAGE2D_ASYNC_PREPARE \
   /* 1. check pixel_ubpack_buffer_index */ \
   if (_cache_glBindBuffer_pixel_unpack_buffer_idx == 0) \
     { \
       /* 2. check memory size */ \
       int copy_size = get_size(format, type, width, height); \
       if (copy_size < 0 || (unsigned int)copy_size > _mp_texture_memory_size) \
         { \
            thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
            goto finish; \
         } \
       if (pixels) \
         { \
            /* 3. malloc & copy */ \
            thread_data->pixels_copied = eina_mempool_malloc(_mp_texture, copy_size); \
            if (thread_data->pixels_copied) \
              { \
                 memcpy(thread_data->pixels_copied, pixels, copy_size); \
              } \
            else \
              { \
                 thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
                 goto finish; \
              } \
            /* 4. replace */ \
            thread_data->pixels = (const void *)thread_data->pixels_copied; \
         } \
     }

#define GLTEXSUBIMAGE2D_ENQUEUE_BEFORE
#define GLTEXSUBIMAGE2D_ENQUEUE_AFTER

#define GLCOMPRESSEDTEXIMAGE2D_VARIABLE_DECLARE \
   void *data_copied;

#define GLCOMPRESSEDTEXIMAGE2D_GLCALL_BEFORE
#define GLCOMPRESSEDTEXIMAGE2D_GLCALL_AFTER \
   if (thread_data->data_copied) \
      eina_mempool_free(_mp_texture, thread_data->data_copied);

#define GLCOMPRESSEDTEXIMAGE2D_VARIABLE_INIT \
   thread_data->data_copied = NULL;

#define GLCOMPRESSEDTEXIMAGE2D_ASYNC_PREPARE \
   /* 1. check pixel_ubpack_buffer_index */ \
   if (_cache_glBindBuffer_pixel_unpack_buffer_idx == 0) \
     { \
        /* 2. check memory size */ \
        if ((unsigned int)imageSize > _mp_texture_memory_size) \
          { \
             thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
             goto finish; \
          } \
        if (data) \
          { \
             /* 3. malloc & copy */ \
             thread_data->data_copied = eina_mempool_malloc(_mp_texture, imageSize); \
             if (thread_data->data_copied) \
               { \
                  memcpy(thread_data->data_copied, data, imageSize); \
               } \
             else \
               { \
                  thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
                  goto finish; \
               } \
             /* 4. replace */ \
             thread_data->data = (const void *)thread_data->data_copied; \
          } \
     }

#define GLCOMPRESSEDTEXIMAGE2D_ENQUEUE_BEFORE
#define GLCOMPRESSEDTEXIMAGE2D_ENQUEUE_AFTER \

#define GLCOMPRESSEDTEXSUBIMAGE2D_VARIABLE_DECLARE \
   void *data_copied;

#define GLCOMPRESSEDTEXSUBIMAGE2D_GLCALL_BEFORE
#define GLCOMPRESSEDTEXSUBIMAGE2D_GLCALL_AFTER \
   if (thread_data->data_copied) \
      eina_mempool_free(_mp_texture, thread_data->data_copied);

#define GLCOMPRESSEDTEXSUBIMAGE2D_VARIABLE_INIT \
   thread_data->data_copied = NULL;

#define GLCOMPRESSEDTEXSUBIMAGE2D_ASYNC_PREPARE \
   /* 1. check pixel_ubpack_buffer_index */ \
   if (_cache_glBindBuffer_pixel_unpack_buffer_idx == 0) \
      { \
        /* 2. check memory size */ \
        if ((unsigned int)imageSize > _mp_texture_memory_size) \
          { \
             thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
             goto finish; \
          } \
        if (data) \
          { \
             /* 3. malloc & copy */ \
             thread_data->data_copied = eina_mempool_malloc(_mp_texture, imageSize); \
             if (thread_data->data_copied) \
               { \
                  memcpy(thread_data->data_copied, data, imageSize); \
               } \
             else \
               { \
                  thread_mode = EVAS_GL_THREAD_MODE_FINISH; \
                  goto finish; \
               } \
             /* 4. replace */ \
             thread_data->data = (const void *)thread_data->data_copied; \
          } \
     }

#define GLCOMPRESSEDTEXSUBIMAGE2D_ENQUEUE_BEFORE
#define GLCOMPRESSEDTEXSUBIMAGE2D_ENQUEUE_AFTER




#include "evas_gl_thread_gl_generated.c"

void
GL_TH_FN(glTexSubImage2DEVAS)(int thread_push, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glTexSubImage2D) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(EVAS_GL_THREAD_TYPE_GL))
     {
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(EVAS_GL_THREAD_TYPE_GL, sizeof(GL_TH_ST(glTexSubImage2D) *) + sizeof(GL_TH_ST(glTexSubImage2D)), &thcmd_ref);
   *thread_data_ptr = (void *)((char *)thread_data_ptr + sizeof(GL_TH_ST(glTexSubImage2D) *));
   thread_data = *thread_data_ptr;

   if (!evas_gl_thread_force_finish())
      thread_mode = EVAS_GL_THREAD_MODE_FLUSH;

   thread_data->target = target;
   thread_data->level = level;
   thread_data->xoffset = xoffset;
   thread_data->yoffset = yoffset;
   thread_data->width = width;
   thread_data->height = height;
   thread_data->format = format;
   thread_data->type = type;
   thread_data->pixels = pixels;
   thread_data->orig_func = glTexSubImage2D;

   /* TODO : Can be deleted after patching evas map issue */
   GLTEXSUBIMAGE2D_VARIABLE_INIT; /* TODO */
   if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)
     goto finish;

   if (thread_push)
     {
        /* NON COPY FASTPATH. Must finish before end of functional logic */
        thread_mode = EVAS_GL_THREAD_MODE_ENQUEUE;
     }
   else
     {
        GLTEXSUBIMAGE2D_ASYNC_PREPARE; /* TODO */
     }
finish:
   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glTexSubImage2D),
                              thread_mode);
}



static Evas_GL_Thread_GL_Func th_gl_func;
Eina_Bool th_gl_func_initialized = EINA_FALSE;

void *
evas_gl_thread_gl_func_get(void)
{
   if (!th_gl_func_initialized)
     {
#define THREAD_FUNCTION_ASSIGN(func) th_gl_func.func = func;

        THREAD_FUNCTION_ASSIGN(GL_TH_FN(glTexSubImage2DEVAS));

        THREAD_FUNCTION_ASSIGN(gl_generated_func_get);
#undef THREAD_FUNCTION_ASSIGN

        th_gl_func_initialized = EINA_TRUE;
     }

   return &th_gl_func;
}



#else  /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */


#include <dlfcn.h>
#include "evas_gl_thread_gl_link_generated.c"

void (*GL_TH_FN(glTexSubImage2DEVAS))(int thread_push, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) = NULL;

void
_gl_thread_link_init(void *func_ptr)
{
   const Evas_GL_Thread_GL_Func *th_gl_func = func_ptr;

   if (!th_gl_func)
     {
        ERR("Thread functions (GL BASE) are not exist");
        return;
     }

#define THREAD_FUNCTION_ASSIGN(func) func = th_gl_func->func;

   THREAD_FUNCTION_ASSIGN(GL_TH_FN(glTexSubImage2DEVAS));

#undef THREAD_FUNCTION_ASSIGN

   if (th_gl_func->gl_generated_func_get)
      _gl_thread_link_gl_generated_init(th_gl_func->gl_generated_func_get());
   else
      ERR("Thread functions (GL-generated) are not exist");

}

#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */
