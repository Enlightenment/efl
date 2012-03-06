/**
 * @defgroup GLView GLView
 *
 * A GLView widget allows for simple GL rendering in elementary environment.
 * GLView hides all the complicated evas_gl details so that the user only
 * has to deal with registering a few callback functions for rendering
 * to a surface using OpenGL APIs.
 *
 * Below is an illustrative example of how to use GLView and and OpenGL
 * to render in elementary environment.
 * @code
// Simple GLView example
#include <Elementary.h>
#include <Evas_GL.h>
#include <stdio.h>

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
// a helper funtion to load shaders from a shader source
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
// intialize callback that gets called once for intialization
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

// delete callback gets called when glview is deleted
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

// resize callback gets called every time object is resized
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


// draw callback is where all the main GL rendering happens
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

// just need to notify that glview has changed so it can render
static Eina_Bool
_anim(void *data)
{
   elm_glview_changed_set(data);
   return EINA_TRUE;
}

static void
_on_done(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del((Evas_Object*)data);
   elm_exit();
}

static void
_del(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Ecore_Animator *ani = evas_object_data_get(obj, "ani");
   ecore_animator_del(ani);
}


EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bx, *bt, *gl;
   Ecore_Animator *ani;
   GLData *gld = NULL;

   if (!(gld = calloc(1, sizeof(GLData)))) return 1;

   // set the engine to opengl_x11
   // if commented out, ELM will choose one
   elm_engine_set("opengl_x11");

   win = elm_win_add(NULL, "glview simple", ELM_WIN_BASIC);
   elm_win_title_set(win, "GLView Simple");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   //-//-//-// THIS IS WHERE GL INIT STUFF HAPPENS (ALA EGL)
   //-//   
   // create a new glview object
   gl = elm_glview_add(win);
   gld->glapi = elm_glview_gl_api_get(gl);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   // mode is simply for supporting alpha, depth buffering, and stencil
   // buffering.
   elm_glview_mode_set(gl, ELM_GLVIEW_ALPHA | ELM_GLVIEW_DEPTH);
   // resize policy tells glview what to do with the surface when it
   // reisizes.  ELM_GLVIEW_RESIZE_POLICY_RECREATE will tell it to 
   // destroy the current surface and recreate it to the new size
   elm_glview_resize_policy_set(gl, ELM_GLVIEW_RESIZE_POLICY_RECREATE);
   // render poicy tells glview how it would like glview to render
   // gl code. ELM_GLVIEW_RENDER_POLICY_ON_DEMAND will have the gl
   // calls called in the pixel_get callback, which only gets called 
   // if the object is visible, hence ON_DEMAND.  ALWAYS mode renders
   // it despite the visibility of the object.
   elm_glview_render_policy_set(gl, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);
   // initialize callback function gets registered here
   elm_glview_init_func_set(gl, _init_gl);
   // delete callback function gets registered here
   elm_glview_del_func_set(gl, _del_gl);
   elm_glview_resize_func_set(gl, _resize_gggl);
   elm_glview_render_func_set(gl, _draw_gl);
   //-//
   //-//-//-// END GL INIT BLOB  
   
   elm_box_pack_end(bx, gl);
   evas_object_show(gl);

   elm_object_focus_set(gl, EINA_TRUE);

   // animating - just a demo. as long as you trigger an update on the image
   // object via elm_glview_changed_set() it will be updated.
   //
   // NOTE: if you delete gl, this animator will keep running trying to access
   // gl so you'd better delete this animator with ecore_animator_del().
   ani = ecore_animator_add(_anim, gl);

   evas_object_data_set(gl, "ani", ani);
   evas_object_data_set(gl, "gld", gld);
   evas_object_event_callback_add(gl, EVAS_CALLBACK_DEL, _del, gl);

   // add an 'OK' button to end the program
   bt = elm_button_add(win);
   elm_object_text_set(bt, "OK");
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _on_done, win);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);

   // run the mainloop and process events and callbacks
   elm_run();
   return 0;
}
ELM_MAIN()
 * @endcode
 */



/**
 * @addtogroup GLView
 * @{
 */

typedef void (*Elm_GLView_Func_Cb)(Evas_Object *obj);

typedef enum _Elm_GLView_Mode
{
   ELM_GLVIEW_NONE    = 0,       
   ELM_GLVIEW_ALPHA   = (1<<1), /**< Alpha channel enabled rendering mode */
   ELM_GLVIEW_DEPTH   = (1<<2), /**< Depth buffer enabled rendering mode */
   ELM_GLVIEW_STENCIL = (1<<3), /**< Stencil buffer enabled rendering mode */
   ELM_GLVIEW_DIRECT  = (1<<4)  /**< Direct rendering optimization hint */
} Elm_GLView_Mode;

/**
 * Defines a policy for the glview resizing. 
 *
 * The resizing policy tells glview what to do with the underlying
 * surface when resize happens. ELM_GLVIEW_RESIZE_POLICY_RECREATE
 * will destroy the current surface and recreate the surface to the
 * new size.  ELM_GLVIEW_RESIZE_POLICY_SCALE will instead keep the
 * current surface but only display the result at the desired size
 * scaled.
 *
 * @note Default is ELM_GLVIEW_RESIZE_POLICY_RECREATE
 */
typedef enum
{
   ELM_GLVIEW_RESIZE_POLICY_RECREATE = 1, /**< Resize the internal surface along with the image */
   ELM_GLVIEW_RESIZE_POLICY_SCALE    = 2  /**< Only resize the internal image and not the surface */
} Elm_GLView_Resize_Policy;

/**
 * Defines a policy for gl rendering. 
 *
 * The rendering policy tells glview where to run the gl rendering code.
 * ELM_GLVIEW_RENDER_POLICY_ON_DEMAND tells glview to call the rendering
 * calls on demand, which means that the rendering code gets called
 * only when it is visible. 
 *
 * @note Default is ELM_GLVIEW_RENDER_POLICY_ON_DEMAND
 */
typedef enum
{
   ELM_GLVIEW_RENDER_POLICY_ON_DEMAND = 1, /**< Render only when there is a need for redrawing */
   ELM_GLVIEW_RENDER_POLICY_ALWAYS    = 2  /**< Render always even when it is not visible */
} Elm_GLView_Render_Policy;

/**
 * Add a new glview to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup GLView
 */
EAPI Evas_Object *elm_glview_add(Evas_Object *parent);

/**
 * Sets the size of the glview
 *
 * @param obj The glview object
 * @param w width of the glview object
 * @param h height of the glview object
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * Gets the size of the glview.
 *
 * @param obj The glview object
 * @param w width of the glview object
 * @param h height of the glview object
 *
 * Note that this function returns the actual image size of the
 * glview.  This means that when the scale policy is set to
 * ELM_GLVIEW_RESIZE_POLICY_SCALE, it'll return the non-scaled
 * size.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * Gets the gl api struct for gl rendering
 *
 * @param obj The glview object
 * @return The api object or NULL if it cannot be created
 *
 * @ingroup GLView
 */
EAPI Evas_GL_API *elm_glview_gl_api_get(const Evas_Object *obj);

/**
 * Set the mode of the GLView. Supports alpha, depth, stencil.
 *
 * @param obj The glview object
 * @param mode The mode Options OR'ed enabling Alpha, Depth, Stencil, Direct.
 * @return True if set properly.
 *
 * Direct is a hint for the elm_glview to render directly to the window
 * given that the right conditions are met. Otherwise it falls back
 * to rendering to an offscreen buffer before it gets composited to the 
 * window.
 *
 * @ingroup GLView
 */
EAPI Eina_Bool    elm_glview_mode_set(Evas_Object *obj, Elm_GLView_Mode mode);

/**
 * Set the resize policy for the glview object.
 *
 * @param obj The glview object.
 * @param policy The scaling policy.
 *
 * By default, the resize policy is set to ELM_GLVIEW_RESIZE_POLICY_RECREATE.  
 * When resize is called it destroys the previous surface and recreates the 
 * newly specified size. If the policy is set to 
 * ELM_GLVIEW_RESIZE_POLICY_SCALE, however, glview only scales the image 
 * object and not the underlying GL Surface.
 *
 * @ingroup GLView
 */
EAPI Eina_Bool    elm_glview_resize_policy_set(Evas_Object *obj, Elm_GLView_Resize_Policy policy);

/**
 * Set the render policy for the glview object.
 *
 * @param obj The glview object.
 * @param policy The render policy.
 *
 * By default, the render policy is set to ELM_GLVIEW_RENDER_POLICY_ON_DEMAND.
 * This policy is set such that during the render loop, glview is only 
 * redrawn if it needs to be redrawn. (i.e. when it is visible) If the policy
 * is set to ELM_GLVIEWW_RENDER_POLICY_ALWAYS, it redraws regardless of
 * whether it is visible or needs redrawing.
 *
 * @ingroup GLView
 */
EAPI Eina_Bool    elm_glview_render_policy_set(Evas_Object *obj, Elm_GLView_Render_Policy policy);

/**
 * Set the init function that runs once in the main loop.
 *
 * @param obj The glview object.
 * @param func The init function to be registered.
 *
 * The registered init function gets called once during the render loop. 
 * This function allows glview to hide all the redering context/surface
 * details and have the user just call GL calls that they desire
 * for initialization GL calls.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_init_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the render function that runs in the main loop.
 *
 * @param obj The glview object.
 * @param func The delete function to be registered.
 *
 * The registered del function gets called when GLView object is deleted.
 * This function allows glview to hide all the redering context/surface
 * details and have the user just call GL calls that they desire
 * when delete happends.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_del_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the resize function that gets called when resize happens.
 *
 * @param obj The glview object.
 * @param func The resize function to be registered.
 *
 * The resize function getes called during the render loop. 
 * This function allows glview to hide all the redering context/surface
 * details and have the user just call GL calls that they desire
 * when resize happends.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_resize_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Set the render function that runs in the main loop.
 *
 * The render function gets called in the main loop but whether it runs
 * depends on the rendering policy and whether elm_glview_changed_set() 
 * gets called.
 * 
 * @param obj The glview object.
 * @param func The render function to be registered.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_render_func_set(Evas_Object *obj, Elm_GLView_Func_Cb func);

/**
 * Notifies that there has been changes in the GLView.
 *
 * @param obj The glview object.
 *
 * @ingroup GLView
 */
EAPI void         elm_glview_changed_set(Evas_Object *obj);

/**
 * @}
 */
