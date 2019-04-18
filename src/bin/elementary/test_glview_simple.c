#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _GL_Data GL_Data;

// GL related data here..
struct _GL_Data
{
   Evas_GL_API *glapi;
   GLuint       program;
   GLuint       vtx_shader;
   GLuint       fgmt_shader;
   GLuint       vbo;
};

static float red = 1.0;

static void
_win_free_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   free(data);
}

static GLuint
load_shader(GL_Data *gld, GLenum type, const char *shader_src)
{
   Evas_GL_API *gl = gld->glapi;
   GLuint shader;
   GLint compiled;

   // Create the shader object
   shader = gl->glCreateShader(type);
   if (shader==0)
      return 0;

   // Load/Compile shader source
   gl->glShaderSource(shader, 1, &shader_src, NULL);
   gl->glCompileShader(shader);
   gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

   if (!compiled)
     {
        GLint info_len = 0;
        gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1)
          {
             char* info_log = malloc(sizeof(char) * info_len);

             gl->glGetShaderInfoLog(shader, info_len, NULL, info_log);
             printf("Error compiling shader:\n%s\n======\n%s\n======\n", info_log, shader_src );
             free(info_log);
          }
        gl->glDeleteShader(shader);
        return 0;
     }

   return shader;
}

// Initialize the shader and program object
static int
init_shaders(GL_Data *gld)
{
   Evas_GL_API *gl = gld->glapi;
   GLbyte vShaderStr[] =
      "attribute vec4 vPosition;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = vPosition;\n"
      "}\n";

   GLbyte fShaderStr[] =
      "#ifdef GL_ES\n"
      "precision mediump float;\n"
      "#endif\n"
      "void main()\n"
      "{\n"
      "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
      "}\n";

   GLint linked;

   // Load the vertex/fragment shaders
   gld->vtx_shader  = load_shader(gld, GL_VERTEX_SHADER, (const char*)vShaderStr);
   gld->fgmt_shader = load_shader(gld, GL_FRAGMENT_SHADER, (const char*)fShaderStr);

   // Create the program object
   gld->program = gl->glCreateProgram( );
   if (gld->program==0)
      return 0;

   gl->glAttachShader(gld->program, gld->vtx_shader);
   gl->glAttachShader(gld->program, gld->fgmt_shader);

   gl->glBindAttribLocation(gld->program, 0, "vPosition");
   gl->glLinkProgram(gld->program);
   gl->glGetProgramiv(gld->program, GL_LINK_STATUS, &linked);

   if (!linked)
     {
        GLint info_len = 0;
        gl->glGetProgramiv(gld->program, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1)
          {
             char* info_log = malloc(sizeof(char) * info_len);

             gl->glGetProgramInfoLog(gld->program, info_len, NULL, info_log);
             printf("Error linking program:\n%s\n", info_log);
             free(info_log);
          }
        gl->glDeleteProgram(gld->program);
        return 0;
     }
   return 1;
}

// Callbacks
static void
_init_gl(Evas_Object *obj)
{
   GL_Data *gld = evas_object_data_get(obj, "gld");
   gld->glapi = elm_glview_gl_api_get(obj);
   Evas_GL_API *gl = gld->glapi;
   GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f,
                           -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f };

   if (!init_shaders(gld))
     {
        printf("Error Initializing Shaders\n");
        return;
     }

   gl->glGenBuffers(1, &gld->vbo);
   gl->glBindBuffer(GL_ARRAY_BUFFER, gld->vbo);
   gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);
   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                             0, 0);
   gl->glEnableVertexAttribArray(0);
}

static void
_del_gl(Evas_Object *obj)
{
   GL_Data *gld = evas_object_data_get(obj, "gld");
   if (!gld)
     {
        printf("Unable to get GL_Data. \n");
        return;
     }
   Evas_GL_API *gl = gld->glapi;

   gl->glDeleteShader(gld->vtx_shader);
   gl->glDeleteShader(gld->fgmt_shader);
   gl->glDeleteProgram(gld->program);
   gl->glDeleteBuffers(1, &gld->vbo);

   evas_object_data_del(obj, "gld");
}

static void
_resize_gl(Evas_Object *obj)
{
   int w, h;
   GL_Data *gld = evas_object_data_get(obj, "gld");
   Evas_GL_API *gl = gld->glapi;

   elm_glview_size_get(obj, &w, &h);

   // GL Viewport stuff. you can avoid doing this if viewport is all the
   // same as last frame if you want
   gl->glViewport(0, 0, w, h);
}

static void
_draw_gl(Evas_Object *obj)
{
   Evas_GL_API *gl = elm_glview_gl_api_get(obj);
   GL_Data *gld = evas_object_data_get(obj, "gld");
   if (!gld) return;
   int w, h;

   elm_glview_size_get(obj, &w, &h);

   gl->glViewport(0, 0, w, h);
   gl->glClearColor(red, 0.8, 0.3, 1);
   gl->glClear(GL_COLOR_BUFFER_BIT);

   // Draw a Triangle
   gl->glEnable(GL_BLEND);

   gl->glUseProgram(gld->program);

   gl->glBindBuffer(GL_ARRAY_BUFFER, gld->vbo);

   gl->glDrawArrays(GL_TRIANGLES, 0, 3);

   // Optional - Flush the GL pipeline
   gl->glFinish();

   red -= 0.1;
   if (red < 0.0) red = 1.0;
}

static Eina_Bool
_anim(void *data)
{
   elm_glview_changed_set(data);
   return ECORE_CALLBACK_RENEW;
}

static void
_close_cb(void *data, Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_gl_del_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ecore_animator_del(data);
}

void
test_glview_simple(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *gl, *lb;
   Ecore_Animator *ani;
   GL_Data *gld = NULL;

   if (!(gld = calloc(1, sizeof(GL_Data)))) return;

   // add a Z-depth buffer to the window and try to use GL
   Eina_Stringshare *accel;
   accel = eina_stringshare_add(elm_config_accel_preference_get());
   elm_config_accel_preference_set("gl:depth");

   win = elm_win_util_standard_add("glview-simple", "GLView Simple");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _win_free_cb, gld);

   // restore previous accel preference
   elm_config_accel_preference_set(accel);
   eina_stringshare_del(accel);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_glview_add(win);
   if (gl)
     {
        evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_glview_mode_set(gl, ELM_GLVIEW_ALPHA | ELM_GLVIEW_DEPTH);
        elm_glview_resize_policy_set(gl, ELM_GLVIEW_RESIZE_POLICY_RECREATE);
        elm_glview_render_policy_set(gl, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);
        elm_glview_init_func_set(gl, _init_gl);
        elm_glview_del_func_set(gl, _del_gl);
        elm_glview_resize_func_set(gl, _resize_gl);
        elm_glview_render_func_set(gl, _draw_gl);
        elm_box_pack_end(bx, gl);
        evas_object_show(gl);

        elm_object_focus_set(gl, EINA_TRUE);

        ani = ecore_animator_add(_anim, gl);
        evas_object_data_set(gl, "gld", gld);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_DEL, _gl_del_cb, ani);
     }
   else
     {
        lb = elm_label_add(bx);
        elm_object_text_set(lb, "<align=left> GL backend engine is not supported.<br/>"
                            " 1. Check your back-end engine or<br/>"
                            " 2. Run elementary_test with engine option or<br/>"
                            "    ex) $ <b>ELM_ACCEL=gl</b> elementary_test<br/>"
                            " 3. Change your back-end engine from elementary_config.<br/></align>");
        evas_object_size_hint_weight_set(lb, 0.0, 0.0);
        evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(bx, lb);
        evas_object_show(lb);
     }

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Close");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _close_cb, win);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
