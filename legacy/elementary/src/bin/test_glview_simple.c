#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct _GLData GLData;

// GL related data here..
struct _GLData
{
   Evas_GL_API *glapi;
   GLuint       program;
   GLuint       vtx_shader;
   GLuint       fgmt_shader;
   GLuint       vbo;
   int          initialized : 1;
};


static float red = 1.0;

//--------------------------------//
static GLuint
load_shader( GLData *gld, GLenum type, const char *shader_src )
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
             printf("Error compiling shader:\n%s\n", info_log );            
             free(info_log);
          }
        gl->glDeleteShader(shader);
        return 0;
     }

   return shader;
}

// Initialize the shader and program object
static int 
init_shaders(GLData *gld)
{
   Evas_GL_API *gl = gld->glapi;
   GLbyte vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";

   GLbyte fShaderStr[] =  
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
      "}                                            \n";

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
   GLData *gld = evas_object_data_get(obj, "gld");
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
   gl->glBufferData(GL_ARRAY_BUFFER, 3 * 3 * 4, vVertices, GL_STATIC_DRAW);
}

static void
_del_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld)
     {
        printf("Unable to get GLData. \n");
        return;
     }
   Evas_GL_API *gl = gld->glapi;

   gl->glDeleteShader(gld->vtx_shader);
   gl->glDeleteShader(gld->fgmt_shader);
   gl->glDeleteProgram(gld->program);
   gl->glDeleteBuffers(1, &gld->vbo); 

   evas_object_data_del((Evas_Object*)obj, "..gld");
   free(gld);
}


static void
_resize_gl(Evas_Object *obj)
{
   int w, h;
   GLData *gld = evas_object_data_get(obj, "gld");
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
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld) return;
   int w, h;

   elm_glview_size_get(obj, &w, &h);

   gl->glViewport(0, 0, w, h);
   gl->glClearColor(red,0.8,0.3,1);
   gl->glClear(GL_COLOR_BUFFER_BIT);

   // Draw a Triangle
   gl->glEnable(GL_BLEND);

   gl->glUseProgram(gld->program);

   gl->glBindBuffer(GL_ARRAY_BUFFER, gld->vbo);
   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                             0, 0);
   gl->glEnableVertexAttribArray(0);

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
   return EINA_TRUE;
}

static void
_on_done(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_del((Evas_Object*)data);
}


static void
_del(void *data __UNUSED__, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Ecore_Animator *ani = evas_object_data_get(obj, "ani");
   ecore_animator_del(ani);
}


void
test_glview_simple(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt, *gl;
   Ecore_Animator *ani;
   GLData *gld = NULL;

   if (!(gld = calloc(1, sizeof(GLData)))) return;

   // Set the engine to opengl_x11
   elm_engine_set("opengl_x11");

   win = elm_win_add(NULL, "glview simple", ELM_WIN_BASIC);
   elm_win_title_set(win, "GLView Simple");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_glview_add(win);
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

   elm_object_focus(gl);

   ani = ecore_animator_add(_anim, gl);
   gld->glapi = elm_glview_gl_api_get(gl);
   evas_object_data_set(gl, "ani", ani);
   evas_object_data_set(gl, "gld", gld);
   evas_object_event_callback_add(gl, EVAS_CALLBACK_DEL, _del, gl);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "OK");
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _on_done, win);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
