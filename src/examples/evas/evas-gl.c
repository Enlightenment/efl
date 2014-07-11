//Add Evas_GL.h for Evas GL APIs access.
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas_GL.h>
#include <math.h>

#define WIDTH  500
#define HEIGHT 500

//GL related data here...
typedef struct _GLData
{
   Evas_Object     *win;
   Evas_GL         *evasgl;
   Evas_GL_API     *glapi;
   Evas_GL_Context *ctx;
   Evas_GL_Surface *sfc;
   Evas_Object     *img;

   unsigned int     program;
   unsigned int     vtx_shader;
   unsigned int     fgmt_shader;
   unsigned int     vbo;
   float            view[16];

   float xangle;
   float yangle;
   Eina_Bool mouse_down : 1;
   Eina_Bool initialized : 1;

} GLData;

static GLData gldata;

//Define the cube's vertices
//Each vertex consist of x, y, z, r, g, b
const float cube_vertices[] =
{
   //front surface is blue
   0.5,  0.5,  0.5,  0.0, 0.0, 1.0,
   -0.5, -0.5,  0.5, 0.0, 0.0, 1.0,
   0.5, -0.5,  0.5,  0.0, 0.0, 1.0,
   0.5,  0.5,  0.5,  0.0, 0.0, 1.0,
   -0.5,  0.5,  0.5, 0.0, 0.0, 1.0,
   -0.5, -0.5,  0.5, 0.0, 0.0, 1.0,
   //left surface is green
   -0.5,  0.5,  0.5, 0.0, 1.0, 0.0,
   -0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
   -0.5, -0.5,  0.5, 0.0, 1.0, 0.0,
   -0.5,  0.5,  0.5, 0.0, 1.0, 0.0,
   -0.5,  0.5, -0.5, 0.0, 1.0, 0.0,
   -0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
   //top surface is red
   -0.5,  0.5,  0.5, 1.0, 0.0, 0.0,
   0.5,  0.5, -0.5,  1.0, 0.0, 0.0,
   -0.5,  0.5, -0.5, 1.0, 0.0, 0.0,
   -0.5,  0.5,  0.5, 1.0, 0.0, 0.0,
   0.5,  0.5,  0.5,  1.0, 0.0, 0.0,
   0.5,  0.5, -0.5,  1.0, 0.0, 0.0,
   //right surface is yellow
   0.5,  0.5, -0.5,  1.0, 1.0, 0.0,
   0.5, -0.5,  0.5,  1.0, 1.0, 0.0,
   0.5, -0.5, -0.5,  1.0, 1.0, 0.0,
   0.5,  0.5, -0.5,  1.0, 1.0, 0.0,
   0.5,  0.5,  0.5,  1.0, 1.0, 0.0,
   0.5, -0.5,  0.5,  1.0, 1.0, 0.0,
   //back surface is cyan
   -0.5,  0.5, -0.5, 0.0, 1.0, 1.0,
   0.5, -0.5, -0.5,  0.0, 1.0, 1.0,
   -0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
   -0.5,  0.5, -0.5, 0.0, 1.0, 1.0,
   0.5,  0.5, -0.5,  0.0, 1.0, 1.0,
   0.5, -0.5, -0.5,  0.0, 1.0, 1.0,
   //bottom surface is magenta
   -0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
   0.5, -0.5,  0.5,  1.0, 0.0, 1.0,
   -0.5, -0.5,  0.5, 1.0, 0.0, 1.0,
   -0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
   0.5, -0.5, -0.5,  1.0, 0.0, 1.0,
   0.5, -0.5,  0.5,  1.0, 0.0, 1.0
};

//Vertext Shader Source
static const char vertex_shader[] =
   "attribute vec4 vPosition;\n"
   "attribute vec3 inColor;\n"
   "uniform mat4 mvpMatrix;"
   "varying vec3 outColor;\n"
   "void main()\n"
   "{\n"
   "   outColor = inColor;\n"
   "   gl_Position = mvpMatrix * vPosition;\n"
   "}\n";

//Fragment Shader Source
static const char fragment_shader[] =
   "#ifdef GL_ES\n"
   "precision mediump float;\n"
   "#endif\n"
   "varying vec3 outColor;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = vec4 ( outColor, 1.0 );\n"
   "}\n";

//Rotation Operation related functions here...
static void
init_matrix(float matrix[16])
{
   matrix[0] = 1.0f;
   matrix[1] = 0.0f;
   matrix[2] = 0.0f;
   matrix[3] = 0.0f;

   matrix[4] = 0.0f;
   matrix[5] = 1.0f;
   matrix[6] = 0.0f;
   matrix[7] = 0.0f;

   matrix[8] = 0.0f;
   matrix[9] = 0.0f;
   matrix[10] = 1.0f;
   matrix[11] = 0.0f;

   matrix[12] = 0.0f;
   matrix[13] = 0.0f;
   matrix[14] = 0.0f;
   matrix[15] = 1.0f;
}

static void
multiply_matrix(float matrix[16], const float matrix0[16],
                const float matrix1[16])
{
   int i;
   int row;
   int column;
   float temp[16];

   for (column = 0; column < 4; column++)
     {
        for (row = 0; row < 4; row++)
          {
             temp[(column * 4) + row] = 0.0f;

             for (i = 0; i < 4; i++)
               temp[(column * 4) + row] += matrix0[(i * 4) + row] * matrix1[(column * 4) + i];
          }
     }

   for (i = 0; i < 16; i++)
        matrix[i] = temp[i];
}

static void
rotate_xyz(float matrix[16], const float anglex, const float angley,
           const float anglez)
{
   const float pi = 3.141592f;
   float temp[16];
   float rx = 2.0f * pi * anglex / 360.0f;
   float ry = 2.0f * pi * angley / 360.0f;
   float rz = 2.0f * pi * anglez / 360.0f;
   float sy = sinf(ry);
   float cy = cosf(ry);
   float sx = sinf(rx);
   float cx = cosf(rx);
   float sz = sinf(rz);
   float cz = cosf(rz);

   init_matrix(temp);

   temp[0] = (cy * cz) - (sx * sy * sz);
   temp[1] = (cz * sx * sy) + (cy * sz);
   temp[2] = -cx * sy;

   temp[4] = -cx * sz;
   temp[5] = cx * cz;
   temp[6] = sx;

   temp[8] = (cz * sy) + (cy * sx * sz);
   temp[9] = (-cy * cz * sx) + (sy * sz);
   temp[10] = cx * cy;

   multiply_matrix(matrix, matrix, temp);
}

static int
view_set_ortho(float result[16], const float left, const float right,
               const float bottom, const float top, const float near,
               const float far)
{
   if ((right - left) == 0.0f || (top - bottom) == 0.0f || (far - near) == 0.0f)
        return 0;

    result[0] = 2.0f / (right - left);
    result[1] = 0.0f;
    result[2] = 0.0f;
    result[3] = 0.0f;
    result[4] = 0.0f;
    result[5] = 2.0f / (top - bottom);
    result[6] = 0.0f;
    result[7] = 0.0f;
    result[8] = 0.0f;
    result[9] = 0.0f;
    result[10] = -2.0f / (far - near);
    result[11] = 0.0f;
    result[12] = -(right + left) / (right - left);
    result[13] = -(top + bottom) / (top - bottom);
    result[14] = -(far + near) / (far - near);
    result[15] = 1.0f;

    return 1;
}

static void
init_shaders(GLData *gl_data)
{
   Evas_GL_API *gl = gl_data->glapi;
   const char *p;

   p = vertex_shader;
   gl_data->vtx_shader = gl->glCreateShader(GL_VERTEX_SHADER);
   gl->glShaderSource(gl_data->vtx_shader, 1, &p, NULL);
   gl->glCompileShader(gl_data->vtx_shader);

   p = fragment_shader;
   gl_data->fgmt_shader = gl->glCreateShader(GL_FRAGMENT_SHADER);
   gl->glShaderSource(gl_data->fgmt_shader, 1, &p, NULL);
   gl->glCompileShader(gl_data->fgmt_shader);

   gl_data->program = gl->glCreateProgram();
   gl->glAttachShader(gl_data->program, gl_data->vtx_shader);
   gl->glAttachShader(gl_data->program, gl_data->fgmt_shader);
   gl->glBindAttribLocation(gl_data->program, 0, "vPosition");
   gl->glBindAttribLocation(gl_data->program, 1, "inColor");

   gl->glLinkProgram(gl_data->program);
   gl->glUseProgram(gl_data->program);
   gl->glEnable(GL_DEPTH_TEST);
}

static void
img_pixel_cb(void *data, Evas_Object *obj)
{
   //Define the model view projection matrix
   float model[16], mvp[16];
   GLData *gl_data = data;
   Evas_GL_API *glapi = gl_data->glapi;

   Evas_Coord w, h;
   evas_object_image_size_get(obj, &w, &h);

   //Set up the context and surface as the current one
   evas_gl_make_current(gl_data->evasgl, gl_data->sfc, gl_data->ctx);

   //Initialize gl stuff just one time.
   if (!gl_data->initialized)
     {
        float aspect;
        init_shaders(gl_data);
        glapi->glGenBuffers(1, &gl_data->vbo);
        glapi->glBindBuffer(GL_ARRAY_BUFFER, gl_data->vbo);
        glapi->glBufferData(GL_ARRAY_BUFFER, 3 * 72 * 4, cube_vertices,GL_STATIC_DRAW);
        init_matrix(gl_data->view);
        if(w > h)
          {
             aspect = (float)w/h;
             view_set_ortho(gl_data->view, -1.0 * aspect, 1.0 * aspect, -1.0, 1.0, -1.0, 1.0);
          }
        else
          {
             aspect = (float)h/w;
             view_set_ortho(gl_data->view, -1.0, 1.0, -1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
          }
        gl_data->initialized = EINA_TRUE;
     }

   glapi->glViewport(0, 0, w, h);
   glapi->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glapi->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   init_matrix(model);
   rotate_xyz(model, gl_data->xangle, gl_data->yangle, 0.0f);
   multiply_matrix(mvp, gl_data->view, model);

   glapi->glUseProgram(gl_data->program);
   glapi->glBindBuffer(GL_ARRAY_BUFFER, gl_data->vbo);
   glapi->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
   glapi->glEnableVertexAttribArray(0);

   glapi->glBindBuffer(GL_ARRAY_BUFFER, gl_data->vbo);
   glapi->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float)*3));
   glapi->glEnableVertexAttribArray(1);

   glapi->glUniformMatrix4fv( glapi->glGetUniformLocation(gl_data->program, "mvpMatrix"), 1, GL_FALSE, mvp);
   glapi->glDrawArrays(GL_TRIANGLES, 0, 36);

   glapi->glFinish();
}

static void
img_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   GLData *gl_data = data;
   Evas_GL_API *glapi = gl_data->glapi;

   //Free the gl resources when image object is deleted.
   evas_gl_make_current(gl_data->evasgl, gl_data->sfc, gl_data->ctx);

   glapi->glDeleteShader(gl_data->vtx_shader);
   glapi->glDeleteShader(gl_data->fgmt_shader);
   glapi->glDeleteProgram(gl_data->program);
   glapi->glDeleteBuffers(1, &gl_data->vbo);

   evas_gl_surface_destroy(gl_data->evasgl, gl_data->sfc);
   evas_gl_context_destroy(gl_data->evasgl, gl_data->ctx);

   evas_gl_free(gl_data->evasgl);
}

static Eina_Bool
_animator_cb(void *data)
{
   Evas_Object *img = data;

   //Animate here whenever an animation tick happens and then mark the image as
   //"dirty" meaning it needs an update next time evas renders. it will call the
   //pixel get callback then.
   evas_object_image_pixels_dirty_set(img, EINA_TRUE);

   return ECORE_CALLBACK_RENEW;
}

static void
_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   GLData *gl_data = data;
   gl_data->mouse_down = EINA_TRUE;
}

static void
_mouse_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   ev = (Evas_Event_Mouse_Move *)event_info;
   GLData *gl_data = data;
   float dx = 0, dy = 0;

   if( gl_data->mouse_down)
     {
        dx = (ev->cur.canvas.x - ev->prev.canvas.x);
        dy = (ev->cur.canvas.y - ev->prev.canvas.y);
        gl_data->xangle += dy;
        gl_data->yangle += dx;
     }
}

static void
_mouse_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   GLData *gl_data = data;
   gl_data->mouse_down = EINA_FALSE;
}

static void
_win_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   float aspect;
   Evas_Coord w,h;
   evas_object_geometry_get( obj, NULL, NULL, &w, &h);

   GLData *gl_data = data;
   evas_object_resize(gl_data->img, w, h);

   init_matrix(gl_data->view);
   if(w > h)
     {
        aspect = (float)w/h;
        view_set_ortho(gl_data->view, (-1.0 * aspect), (1.0 * aspect), -1.0, 1.0, -1.0, 1.0);
     }
   else
     {
        aspect = (float)h/w;
        view_set_ortho(gl_data->view, -1.0, 1.0, (-1.0 * aspect), (1.0 * aspect), -1.0, 1.0);
     }
}

static void
evas_gl_exam(Evas_Object *win)
{
   Evas_Native_Surface ns;

   //Config for the surface for evas gl
   Evas_GL_Config config =
     {
        EVAS_GL_RGBA_8888,
        EVAS_GL_DEPTH_BIT_32,
        EVAS_GL_STENCIL_NONE
     };

   //Get the window size
   Evas_Coord w,h;
   evas_object_geometry_get(win, NULL, NULL, &w, &h);

   //Get the evas gl handle for doing gl things
   gldata.evasgl = evas_gl_new(evas_object_evas_get(win));
   gldata.glapi = evas_gl_api_get(gldata.evasgl);

   //Create a surface and context
   gldata.sfc = evas_gl_surface_create(gldata.evasgl, &config, w, h);
   gldata.ctx = evas_gl_context_create(gldata.evasgl, NULL);

   //Set rotation variables
   gldata.xangle = 45.0f;
   gldata.yangle = 45.0f;
   gldata.mouse_down = EINA_FALSE;

   //Set up the image object. A filled one by default.
   gldata.img = evas_object_image_filled_add(evas_object_evas_get(win));
   evas_object_event_callback_add(gldata.img, EVAS_CALLBACK_DEL, img_del_cb, &gldata);

   //Set up an actual pixel size for the buffer data. It may be different to the
   //output size. Any windowing sysmtem has something like this, just evas has 2
   //sizes, a pixel size and the output object size.
   evas_object_image_size_set(gldata.img, w, h);

   //Set up the native surface info to use the context and surface created
   //above.
   evas_gl_native_surface_get(gldata.evasgl, gldata.sfc, &ns);
   evas_object_image_native_surface_set(gldata.img, &ns);
   evas_object_image_pixels_get_callback_set(gldata.img, img_pixel_cb, &gldata);

   evas_object_resize(gldata.img, w, h);
   evas_object_show(gldata.img);

   //Add Mouse Event Callbacks
   evas_object_event_callback_add(gldata.img, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, &gldata);
   evas_object_event_callback_add(gldata.img, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, &gldata);
   evas_object_event_callback_add(gldata.img, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, &gldata);
   //Add Window Resize Event Callback
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _win_resize_cb, &gldata);

   ecore_animator_add(_animator_cb, gldata.img);
}

static void
_on_delete_cb(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(gldata.win, w, h);
}

int
main(void)
{
   if (!ecore_evas_init()) return 0;

   Ecore_Evas *ecore_evas = ecore_evas_new("opengl_x11", 0, 0, WIDTH, HEIGHT, NULL);

   if(!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete_cb);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize_cb);
   ecore_evas_show(ecore_evas);

   Evas_Object *evas = ecore_evas_get(ecore_evas);

   gldata.win = evas_object_rectangle_add(evas);
   evas_object_color_set(gldata.win, 255, 255, 255, 255);
   evas_object_resize(gldata.win, WIDTH, HEIGHT);
   evas_object_show(gldata.win);

   evas_gl_exam(gldata.win);

   ecore_main_loop_begin();
   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
