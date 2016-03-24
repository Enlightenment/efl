#include <Elementary.hh>

#include <Evas_GL.h>
#include <stdio.h>

typedef struct _GLData GLData;

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

static GLuint
load_shader(GLData *gld, GLenum type, const char *shader_src )
{
   Evas_GL_API *gl = gld->glapi;
   GLuint shader;
   GLint compiled;

   shader = gl->glCreateShader(type);
   if (shader==0)
     return 0;

   gl->glShaderSource(shader, 1, &shader_src, NULL);
   gl->glCompileShader(shader);
   gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

   if (!compiled)
     {
        GLint info_len = 0;
        gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1)
          {
             char* info_log = new char[sizeof(char) * info_len];

             gl->glGetShaderInfoLog(shader, info_len, NULL, info_log);
             printf("Error compiling shader:\n%s\n======\n%s\n======\n", info_log, shader_src );
             delete [] info_log;
          }
        gl->glDeleteShader(shader);
        return 0;
     }

   return shader;
}

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
     "#ifdef GL_ES                                 \n"
     "precision mediump float;                     \n"
     "#endif                                       \n"
     "void main()                                  \n"
     "{                                            \n"
     "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
     "}                                            \n";

   GLint linked;

   gld->vtx_shader  = load_shader(gld, GL_VERTEX_SHADER, (const char*)vShaderStr);
   gld->fgmt_shader = load_shader(gld, GL_FRAGMENT_SHADER, (const char*)fShaderStr);

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
             char* info_log = new char[sizeof(char) * info_len];

             gl->glGetProgramInfoLog(gld->program, info_len, NULL, info_log);
             printf("Error linking program:\n%s\n", info_log);
             delete [] info_log;
          }
        gl->glDeleteProgram(gld->program);
        return 0;
     }
   return 1;
}

static void
_init_gl(Evas_Object *obj)
{
   GLData *gld = static_cast<GLData*>(evas_object_data_get(obj, "gld"));
   assert(gld != 0);
   ::elm::glview glv(eo_ref(obj));
   Evas_GL_API *gl = glv.gl_api_get();
   GLfloat vVertices[] = {
     0.0f,  0.5f, 0.0f,
     -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f };

   if (!init_shaders(gld))
     {
        std::cout << "Error Initializing Shaders" << std::endl;
        return;
     }

   gl->glGenBuffers(1, &gld->vbo);
   gl->glBindBuffer(GL_ARRAY_BUFFER, gld->vbo);
   gl->glBufferData(GL_ARRAY_BUFFER, 3 * 3 * 4, vVertices, GL_STATIC_DRAW);

}

static void
_del_gl(Evas_Object *obj)
{
   GLData *gld = static_cast<GLData*>(evas_object_data_get(obj, "gld"));
   if (!gld)
     {
        std::cout << "Unable to get GLData. " << std::endl;
        return;
     }
   ::elm::glview glv(eo_ref(obj));
   Evas_GL_API *gl = glv.gl_api_get();

   gl->glDeleteShader(gld->vtx_shader);
   gl->glDeleteShader(gld->fgmt_shader);
   gl->glDeleteProgram(gld->program);
   gl->glDeleteBuffers(1, &gld->vbo);

   evas_object_data_del(obj, "..gld");
   free(gld);
}

static void
_resize_gl(Evas_Object *obj)
{
   int w, h;
   ::elm::glview glv(eo_ref(obj));
   Evas_GL_API *gl = glv.gl_api_get();

   glv.size_get(&w, &h);

   gl->glViewport(0, 0, w, h);
}

static void
_draw_gl(Evas_Object *obj)
{
   GLData *gld = static_cast<GLData*>(evas_object_data_get(obj, "gld"));
   ::elm::glview glv(eo_ref(obj));
   Evas_GL_API *gl = glv.gl_api_get();

   if (!gld) return;
   int w, h;

   glv.size_get(&w, &h);

   gl->glViewport(0, 0, w, h);
   gl->glClearColor(red,0.8,0.3,1);
   gl->glClear(GL_COLOR_BUFFER_BIT);

   gl->glEnable(GL_BLEND);

   gl->glUseProgram(gld->program);

   gl->glBindBuffer(GL_ARRAY_BUFFER, gld->vbo);
   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
   gl->glEnableVertexAttribArray(0);

   gl->glDrawArrays(GL_TRIANGLES, 0, 3);

   gl->glFinish();

   red -= 0.1;
   if (red < 0.0) red = 1.0;
}

static Eina_Bool
_anim(void* data)
{
   static_cast<elm::glview*>(data)->changed_set();

   return EINA_TRUE;
}

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   GLData *gld = NULL;

   if (!(gld = static_cast<GLData*>(calloc(1, sizeof(GLData))))) return 1;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win win(elm_win_util_standard_add("glview simple", "GLView Simple"));
   win.autohide_set(true);

   ::elm::box bx(efl::eo::parent = win);
   bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bx);
   bx.visible_set(true);

   ::elm::glview gl(efl::eo::parent = win,
                   gl.version_constructor(EVAS_GL_GLES_2_X));
   gld->glapi = gl.gl_api_get();
   gl.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   gl.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);

   int gl_mode = ELM_GLVIEW_ALPHA | ELM_GLVIEW_DEPTH;
   gl.mode_set(static_cast<Elm_GLView_Mode>(gl_mode));
   gl.resize_policy_set(ELM_GLVIEW_RESIZE_POLICY_RECREATE);
   gl.render_policy_set(ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);
   gl.init_func_set(_init_gl);
   gl.del_func_set(_del_gl);
   gl.resize_func_set(_resize_gl);
   gl.render_func_set(_draw_gl);

   bx.pack_end(gl);
   gl.visible_set(true);

   gl.object::focus_set(true);

   Ecore_Animator *ani = ecore_animator_add(_anim, &gl);

   evas_object_data_set(gl._eo_ptr(), "ani", ani);
   evas_object_data_set(gl._eo_ptr(), "gld", gld);
   gl.object::callback_del_add(std::bind([&] () { ecore_animator_del(ani); }));

   ::elm::button bt(efl::eo::parent = win);
   bt.text_set(nullptr, "OK");
   bt.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   bt.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
   bx.pack_end(bt);
   bt.visible_set(true);
   bt.callback_clicked_add(std::bind([] () { elm_exit(); }));

   win.size_set(320, 480);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
