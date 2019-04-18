#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#ifndef M_PI
#define M_PI 3.14159265
#endif

typedef struct _Gear Gear;
typedef struct _GLData GLData;
struct _Gear
{
   GLfloat *vertices;
   GLuint vbo;
   int count;
};

// GL related data here..
struct _GLData
{
   Evas_GL_API *glapi;
   GLuint       program;
   GLuint       vtx_shader;
   GLuint       fgmt_shader;
   int          initialized : 1;
   int          mouse_down : 1;

   // Gear Stuff
   GLfloat      view_rotx;
   GLfloat      view_roty;
   GLfloat      view_rotz;

   Gear        *gear1;
   Gear        *gear2;
   Gear        *gear3;

   GLfloat      angle;

   GLuint       proj_location;
   GLuint       light_location;
   GLuint       color_location;

   GLfloat      proj[16];
   GLfloat      light[3];
};

static void gears_init(GLData *gld);
static void free_gear(Gear *gear);
static void gears_reshape(GLData *gld, int width, int height);
static void render_gears(GLData *gld);

//--------------------------------//
// Gear Stuff....
static GLfloat *
vert(GLfloat *p, GLfloat x, GLfloat y, GLfloat z, GLfloat *n)
{
   p[0] = x;
   p[1] = y;
   p[2] = z;
   p[3] = n[0];
   p[4] = n[1];
   p[5] = n[2];

   return p + 6;
}

/*  Draw a gear wheel.  You'll probably want to call this function when
 *  building a display list since we do a lot of trig here.
 *
 *  Input:  inner_radius - radius of hole at center
 *          outer_radius - radius at center of teeth
 *          width - width of gear
 *          teeth - number of teeth
 *          tooth_depth - depth of tooth
 */
static Gear *
make_gear(GLData *gld, GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
          GLint teeth, GLfloat tooth_depth)
{
   GLint i;
   GLfloat r0, r1, r2;
   GLfloat da;
   GLfloat *v;
   Gear *gear;
   double s[5], c[5];
   GLfloat normal[3];
   const int tris_per_tooth = 20;
   Evas_GL_API *gl = gld->glapi;

   gear = (Gear*)malloc(sizeof(Gear));
   if (gear == NULL)
     return NULL;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth / 2.0;
   r2 = outer_radius + tooth_depth / 2.0;

   da = 2.0 * M_PI / teeth / 4.0;

   gear->vertices = calloc(teeth * tris_per_tooth * 3 * 6,
                           sizeof *gear->vertices);
   s[4] = 0;
   c[4] = 1;
   v = gear->vertices;
   for (i = 0; i < teeth; i++)
     {
        s[0] = s[4];
        c[0] = c[4];
        s[1] = sin(i * 2.0 * M_PI / teeth + da);
        c[1] = cos(i * 2.0 * M_PI / teeth + da);
        s[2] = sin(i * 2.0 * M_PI / teeth + da * 2);
        c[2] = cos(i * 2.0 * M_PI / teeth + da * 2);
        s[3] = sin(i * 2.0 * M_PI / teeth + da * 3);
        c[3] = cos(i * 2.0 * M_PI / teeth + da * 3);
        s[4] = sin(i * 2.0 * M_PI / teeth + da * 4);
        c[4] = cos(i * 2.0 * M_PI / teeth + da * 4);

        normal[0] = 0.0;
        normal[1] = 0.0;
        normal[2] = 1.0;

        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);

        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], width * 0.5, normal);
        v = vert(v, r0 * c[4], r0 * s[4], width * 0.5, normal);

        v = vert(v, r0 * c[4], r0 * s[4], width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], width * 0.5, normal);
        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], -width * 0.5, normal);

        normal[0] = 0.0;
        normal[1] = 0.0;
        normal[2] = -1.0;

        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);

        v = vert(v, r0 * c[4], r0 * s[4], -width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);
        v = vert(v, r0 * c[0], r0 * s[0], -width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], -width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], -width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], -width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], -width * 0.5, normal);

        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);

        v = vert(v, r1 * c[0], r1 * s[0], width * 0.5, normal);
        v = vert(v, r1 * c[0], r1 * s[0], -width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], width * 0.5, normal);
        v = vert(v, r2 * c[1], r2 * s[1], -width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], width * 0.5, normal);
        v = vert(v, r2 * c[2], r2 * s[2], -width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], width * 0.5, normal);
        v = vert(v, r1 * c[3], r1 * s[3], -width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], width * 0.5, normal);
        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);

        v = vert(v, r1 * c[4], r1 * s[4], -width * 0.5, normal);
     }

   gear->count = (v - gear->vertices) / 6;

   gl->glGenBuffers(1, &gear->vbo);
   gl->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);
   gl->glBufferData(GL_ARRAY_BUFFER, gear->count * 6 * 4,
                    gear->vertices, GL_STATIC_DRAW);

   return gear;
}

static void
free_gear(Gear *gear)
{
    if (!gear) return;
    free(gear->vertices);
    free(gear);
}

static void
multiply(GLfloat *m, const GLfloat *n)
{
   GLfloat tmp[16];
   const GLfloat *row, *column;
   div_t d;
   int i, j;

   for (i = 0; i < 16; i++)
     {
        tmp[i] = 0;
        d = div(i, 4);
        row = n + d.quot * 4;
        column = m + d.rem;
        for (j = 0; j < 4; j++)
          tmp[i] += row[j] * column[j * 4];
     }
   memcpy(m, &tmp, sizeof tmp);
}

static void
rotate(GLfloat *m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   double s, c;

   s = sin(angle);
   c = cos(angle);
   GLfloat r[16] =
     {
        x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
        x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0,
        x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
        0, 0, 0, 1
     };

   multiply(m, r);
}

static void
translate(GLfloat *m, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat t[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  x, y, z, 1 };

   multiply(m, t);
}

static void
draw_gear(GLData *gld, Gear *gear, GLfloat *m,
          GLfloat x, GLfloat y, GLfloat angle, const GLfloat *color)
{
   Evas_GL_API *gl = gld->glapi;
   GLfloat tmp[16];

   memcpy(tmp, m, sizeof tmp);
   translate(tmp, x, y, 0);
   rotate(tmp, 2 * M_PI * angle / 360.0, 0, 0, 1);
   gl->glUniformMatrix4fv(gld->proj_location, 1, GL_FALSE, tmp);
   gl->glUniform3fv(gld->light_location, 1, gld->light);
   gl->glUniform4fv(gld->color_location, 1, color);

   gl->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                             6 * sizeof(GLfloat), NULL);
   gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                             6 * sizeof(GLfloat), (GLfloat *)(0 + 3 * sizeof(GLfloat)));
   gl->glEnableVertexAttribArray(0);
   gl->glEnableVertexAttribArray(1);
   gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, gear->count);
}

static void
gears_draw(GLData *gld)
{
   Evas_GL_API *gl = gld->glapi;

   static const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
   static const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
   static const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };
   GLfloat m[16];

   gl->glClearColor(0x25 / 255., 0x13 / 255., 0.0, 1.0);
   gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   memcpy(m, gld->proj, sizeof m);
   rotate(m, 2 * M_PI * gld->view_rotx / 360.0, 1, 0, 0);
   rotate(m, 2 * M_PI * gld->view_roty / 360.0, 0, 1, 0);
   rotate(m, 2 * M_PI * gld->view_rotz / 360.0, 0, 0, 1);

   draw_gear(gld, gld->gear1, m, -3.0, -2.0, gld->angle, red);
   draw_gear(gld, gld->gear2, m, 3.1, -2.0, -2 * gld->angle - 9.0, green);
   draw_gear(gld, gld->gear3, m, -3.1, 4.2, -2 * gld->angle - 25.0, blue);
}

static void render_gears(GLData *gld)
{
   gears_draw(gld);

   gld->angle += 2.0;
}

/* new window size or exposure */
static void
gears_reshape(GLData *gld, int width, int height)
{
   Evas_GL_API *gl = gld->glapi;

   GLfloat ar, m[16] = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.1, 0.0,
      0.0, 0.0, 0.0, 1.0
   };

   if (width < height)
     ar = width;
   else
     ar = height;

   m[0] = 0.1 * ar / width;
   m[5] = 0.1 * ar / height;
   memcpy(gld->proj, m, sizeof gld->proj);
   gl->glViewport(0, 0, (GLint) width, (GLint) height);
}

static const char vertex_shader[] =
   "uniform mat4 proj;\n"
   "attribute vec4 position;\n"
   "attribute vec4 normal;\n"
   "varying vec3 rotated_normal;\n"
   "varying vec3 rotated_position;\n"
   "vec4 tmp;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = proj * position;\n"
   "   rotated_position = gl_Position.xyz;\n"
   "   tmp = proj * normal;\n"
   "   rotated_normal = tmp.xyz;\n"
   "}\n";

 static const char fragment_shader[] =
   "#ifdef GL_ES\n"
   "precision mediump float;\n"
   "#endif\n"
   "uniform vec4 color;\n"
   "uniform vec3 light;\n"
   "varying vec3 rotated_normal;\n"
   "varying vec3 rotated_position;\n"
   "vec3 light_direction;\n"
   "vec4 white = vec4(0.5, 0.5, 0.5, 1.0);\n"
   "void main()\n"
   "{\n"
   "   light_direction = normalize(light - rotated_position);\n"
   "   gl_FragColor = color + white * dot(light_direction, rotated_normal);\n"
   "}\n";

static void
_print_gl_log(Evas_GL_API *gl, GLuint id)
{
   GLint log_len = 0;
   char *log_info;

   if (gl->glIsShader(id))
     gl->glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_len);
   else if (gl->glIsProgram(id))
     gl->glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_len);
   if (!log_len) return;

   log_info = malloc(log_len * sizeof(char));

   if (gl->glIsShader(id))
     gl->glGetShaderInfoLog(id, log_len, NULL, log_info);
   else if (gl->glIsProgram(id))
     gl->glGetProgramInfoLog(id, log_len, NULL, log_info);

   printf("%s", log_info);
   free(log_info);
}

static void
gears_init(GLData *gld)
{
   Evas_GL_API *gl = gld->glapi;

   const char *p;

   gl->glEnable(GL_CULL_FACE);
   gl->glEnable(GL_DEPTH_TEST);

   p = vertex_shader;
   gld->vtx_shader = gl->glCreateShader(GL_VERTEX_SHADER);
   gl->glShaderSource(gld->vtx_shader, 1, &p, NULL);
   gl->glCompileShader(gld->vtx_shader);
   _print_gl_log(gl, gld->vtx_shader);

   p = fragment_shader;
   gld->fgmt_shader = gl->glCreateShader(GL_FRAGMENT_SHADER);
   gl->glShaderSource(gld->fgmt_shader, 1, &p, NULL);
   gl->glCompileShader(gld->fgmt_shader);
   _print_gl_log(gl, gld->fgmt_shader);

   gld->program = gl->glCreateProgram();
   gl->glAttachShader(gld->program, gld->vtx_shader);
   gl->glAttachShader(gld->program, gld->fgmt_shader);
   gl->glBindAttribLocation(gld->program, 0, "position");
   gl->glBindAttribLocation(gld->program, 1, "normal");

   gl->glLinkProgram(gld->program);
   _print_gl_log(gl, gld->program);

   gl->glUseProgram(gld->program);
   gld->proj_location  = gl->glGetUniformLocation(gld->program, "proj");
   gld->light_location = gl->glGetUniformLocation(gld->program, "light");
   gld->color_location = gl->glGetUniformLocation(gld->program, "color");

   /* make the gears */
   gld->gear1 = make_gear(gld, 1.0, 4.0, 1.0, 20, 0.7);
   gld->gear2 = make_gear(gld, 0.5, 2.0, 2.0, 10, 0.7);
   gld->gear3 = make_gear(gld, 1.3, 2.0, 0.5, 10, 0.7);
}

static void
gldata_init(GLData *gld)
{
   gld->initialized = 0;
   gld->mouse_down = 0;

   gld->view_rotx = -20.0;
   gld->view_roty = -30.0;
   gld->view_rotz = 0.0;
   gld->angle = 0.0;

   gld->light[0] = 1.0;
   gld->light[1] = 1.0;
   gld->light[2] = -5.0;
}

//-------------------------//

static void
_init_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "gld");

   gld->glapi = elm_glview_gl_api_get(obj);
   printf("GL_VERSION: %s\n", gld->glapi->glGetString(GL_VERSION));
   fflush(stdout);

   gears_init(gld);
}

static void
_del_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld)
     {
        printf("Unable to get GLData.\n");
        fflush(stdout);
        return;
     }
   Evas_GL_API *gl = gld->glapi;

   if (gl)
     {
        gl->glDeleteShader(gld->vtx_shader);
        gl->glDeleteShader(gld->fgmt_shader);
        gl->glDeleteProgram(gld->program);
        gl->glDeleteBuffers(1, &gld->gear1->vbo);
        gl->glDeleteBuffers(1, &gld->gear2->vbo);
        gl->glDeleteBuffers(1, &gld->gear3->vbo);
     }

   free_gear(gld->gear1);
   free_gear(gld->gear2);
   free_gear(gld->gear3);

   evas_object_data_del((Evas_Object*)obj, "..gld");
   free(gld);
}

static void
_resize_gl(Evas_Object *obj)
{
   int w, h;
   GLData *gld = evas_object_data_get(obj, "gld");

   elm_glview_size_get(obj, &w, &h);

   // GL Viewport stuff. you can avoid doing this if viewport is all the
   // same as last frame if you want
   gears_reshape(gld, w,h);
}

static void
_draw_gl(Evas_Object *obj)
{
   Evas_GL_API *gl = elm_glview_gl_api_get(obj);
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld) return;

   render_gears(gld);
   gl->glFinish();
}

static Eina_Bool
_anim(void *data)
{
   elm_glview_changed_set(data);
   return EINA_TRUE;
}

static Eina_Bool
_quit_idler(void *data)
{
   evas_object_del(data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_done(void *data,
         Evas_Object *obj EINA_UNUSED,
         void *event_info EINA_UNUSED)
{
   ecore_idler_add(_quit_idler, data);
}

static void
_on_direct(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   if (!data) return;

   // ON_DEMAND is necessary for Direct Rendering
   elm_glview_render_policy_set(data, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);
   elm_glview_mode_set(data, 0
                       | ELM_GLVIEW_ALPHA
                       | ELM_GLVIEW_DEPTH
                       | ELM_GLVIEW_DIRECT
                      );
}

static void
_on_indirect(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   if (!data) return;

   // note that with policy ALWAYS the window will flicker on resize
   elm_glview_render_policy_set(data, ELM_GLVIEW_RENDER_POLICY_ALWAYS);
   elm_glview_mode_set(data, 0
                       | ELM_GLVIEW_ALPHA
                       | ELM_GLVIEW_DEPTH
                      );
}

static void
_del(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Ecore_Animator *ani = evas_object_data_get(obj, "ani");
   ecore_animator_del(ani);
}

static void
_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   ev = (Evas_Event_Key_Down *)event_info;
   GLData *gld = evas_object_data_get(obj, "gld");

   if (strcmp(ev->key, "Left") == 0)
     {
        gld->view_roty += 5.0;
        return;
     }

   if (strcmp(ev->key, "Right") == 0)
     {
        gld->view_roty -= 5.0;
        return;
     }

   if (strcmp(ev->key, "Up") == 0)
     {
        gld->view_rotx += 5.0;
        return;
     }

   if (strcmp(ev->key, "Down") == 0)
     {
        gld->view_rotx -= 5.0;
        return;
     }
   if ((strcmp(ev->key, "Escape") == 0) ||
       (strcmp(ev->key, "Return") == 0))
     {
        //_on_done(data, obj, event_info);
        return;
     }
}

static void
_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   gld->mouse_down = 1;
}

static void
_mouse_move(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Event_Mouse_Move *ev;
   ev = (Evas_Event_Mouse_Move *)event_info;
   GLData *gld = evas_object_data_get(obj, "gld");
   float dx = 0, dy = 0;

   if (gld->mouse_down)
     {
        dx = ev->cur.canvas.x - ev->prev.canvas.x;
        dy = ev->cur.canvas.y - ev->prev.canvas.y;

        gld->view_roty += -1.0 * dx;
        gld->view_rotx += -1.0 * dy;
     }
}

static void
_mouse_up(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   gld->mouse_down = 0;
}

static void
_test_glview_do(Evas_GL_Context_Version version)
{
   Evas_Object *win, *bx, *bt, *gl, *lb;
   Ecore_Animator *ani;
   GLData *gld = NULL;

   // alloc a data struct to hold our relevant gl info in
   if (!(gld = calloc(1, sizeof(GLData)))) return;
   gldata_init(gld);

#if 1
   // add a Z-depth buffer to the window and try to use GL
   Eina_Stringshare *accel;
   accel = eina_stringshare_add(elm_config_accel_preference_get());
   elm_config_accel_preference_set("gl:depth");

   // new window - do the usual and give it a name, title and delete handler
   win = elm_win_util_standard_add("glview", "GLView");
   elm_win_autodel_set(win, EINA_TRUE);

   // restore previous accel preference
   elm_config_accel_preference_set(accel);
   eina_stringshare_del(accel);
#else
   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                efl_ui_win_name_set(efl_added, "glview"),
                efl_text_set(efl_added, "GLView"),
                efl_ui_win_accel_preference_set(efl_added, "gl:depth"));
   elm_win_autodel_set(win, EINA_TRUE);
#endif

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   // Add a GLView
   gl = elm_glview_version_add(win, version);
   if (gl)
     {
        evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_glview_mode_set(gl, 0
                            | ELM_GLVIEW_ALPHA
                            | ELM_GLVIEW_DEPTH
                           );
        elm_glview_resize_policy_set(gl, ELM_GLVIEW_RESIZE_POLICY_RECREATE);
        elm_glview_render_policy_set(gl, ELM_GLVIEW_RENDER_POLICY_ALWAYS);
        elm_glview_init_func_set(gl, _init_gl);
        elm_glview_del_func_set(gl, _del_gl);
        elm_glview_resize_func_set(gl, _resize_gl);
        elm_glview_render_func_set(gl, _draw_gl);
        elm_box_pack_end(bx, gl);
        evas_object_show(gl);

        // Add Mouse/Key Event Callbacks
        elm_object_focus_set(gl, EINA_TRUE);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_KEY_DOWN, _key_down, gl);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, gl);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_MOUSE_UP, _mouse_up, gl);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, gl);

        // Animator and other vars
        ani = ecore_animator_add(_anim, gl);
        evas_object_data_set(gl, "ani", ani);
        evas_object_data_set(gl, "gld", gld);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_DEL, _del, gl);

        bt = elm_button_add(win);
        elm_object_text_set(bt, "Direct Mode");
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
        evas_object_smart_callback_add(bt, "clicked", _on_direct, gl);

        bt = elm_button_add(win);
        elm_object_text_set(bt, "Indirect Mode");
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
        evas_object_smart_callback_add(bt, "clicked", _on_indirect, gl);
     }
   else
     {
        lb = elm_label_add(bx);
        elm_object_text_set(lb, "<align=left> GL backend engine is not supported.<br/>"
                            " 1. Check your back-end engine or<br/>"
                            " 2. Run elementary_test with engine option or<br/>"
                            "    ex) $ <b>ELM_ACCEL=gl</b> elementary_test<br/>"
                            " 3. Change your back-end engine from elementary_config.<br/></align>");
        evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(bx, lb);
        evas_object_show(lb);
        free(gld);
     }

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Close");
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _on_done, win);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

void
test_glview(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _test_glview_do(EVAS_GL_GLES_2_X);
}

void
test_glview_gles3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _test_glview_do(EVAS_GL_GLES_3_X);
}
