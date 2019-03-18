/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Ported to GLES2.
 * Kristian Høgsberg <krh@bitplanet.net>
 * May 3, 2010
 *
 * Improve GLES2 port:
 *   * Refactor gear drawing.
 *   * Use correct normals for surfaces.
 *   * Improve shader.
 *   * Use perspective projection transformation.
 *   * Add FPS count.
 *   * Add comments.
 * Alexandros Frantzis <alexandros.frantzis@linaro.org>
 * Jul 13, 2010
 *
 * Ported to Elm_GLView and added features for testing purposes:
 *    * Has 5 gears
 *    * Number of teeth can be increased to test under many vertices
 *    * Up/Down button increases the teeth number
 * Sung W. Park <sungwoo@gmail.com>
 * (Some positioning and numbers taken from KHCho's example)
 * Oct 27, 2011
 *
 */
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define STRIPS_PER_TOOTH 7
#define VERTICES_PER_TOOTH 34
#define GEAR_VERTEX_STRIDE 6

/* A set of macros for making the creation of the gears easier */
#define  GEAR_POINT(r, da) { (r) * c[(da)], (r) * s[(da)] }
#define  SET_NORMAL(x, y, z) do { \
     normal[0] = (x); normal[1] = (y); normal[2] = (z); \
} while(0);

#define  GEAR_VERT(v, point, sign) vert((v), p[(point)].x, p[(point)].y, \
                                        (sign) * width * 0.5, normal);

#define START_STRIP do { \
     gear->strips[cur_strip].first = v - gear->vertices; \
} while(0);

#define END_STRIP do { \
     int _tmp = (v - gear->vertices); \
     gear->strips[cur_strip].count = _tmp - gear->strips[cur_strip].first; \
     cur_strip++; \
} while (0);

#define QUAD_WITH_NORMAL(p1, p2) do { \
     SET_NORMAL((p[(p1)].y - p[(p2)].y), -(p[(p1)].x - p[(p2)].x), 0); \
     v = GEAR_VERT(v, (p1), -1); \
     v = GEAR_VERT(v, (p1), 1); \
     v = GEAR_VERT(v, (p2), -1); \
     v = GEAR_VERT(v, (p2), 1); \
} while(0);

// Struct describing a point
typedef struct _Point
{
   GLfloat x;
   GLfloat y;
} Point;

// Struct describing the vertices in triangle strip
typedef struct _VertexStrip
{
   GLint first;
   GLint count;
} VertexStrip;

// Each vertex consist of GEAR_VERTEX_STRIDE GLfloat attributes
typedef GLfloat GearVertex[GEAR_VERTEX_STRIDE];

// Struct representing a gear.
typedef struct _Gear
{
   GearVertex  *vertices;
   int          nvertices;
   VertexStrip *strips;
   int          nstrips;
   GLuint       vbo;
} Gear;

// GL related data here..
typedef struct _GLData
{
   Evas_GL_API *glapi;
   GLuint       program;
   GLuint       vtx_shader;
   GLuint       fgmt_shader;
   int          initialized : 1;
   int          mouse_down : 1;

   // Gear Stuff
   Gear        *gear[6];
   GLfloat      view_rot[3];
   GLfloat      light_pos[4];
   GLfloat      proj_mat[16];

   GLfloat      angle;

   GLuint       mvp_loc;         // ModelViewPorjection Matrix Loc
   GLuint       norm_mat_loc;
   GLuint       light_pos_loc;
   GLuint       material_loc;

   GLuint       gear_teeth;
   GLuint       tot_vertices;

   int          recreate_gears;
} GLData;

static void gears_init(GLData *gld);
static void create_gears(GLData *gld);
static void render_gears(GLData *gld);
static void gears_reshape(GLData *gld, int width, int height);

//--------------------------------//
// Fills a gear vertex.
static GearVertex *
vert(GearVertex *v, GLfloat x, GLfloat y, GLfloat z, GLfloat n[3])
{
   v[0][0] = x;
   v[0][1] = y;
   v[0][2] = z;
   v[0][3] = n[0];
   v[0][4] = n[1];
   v[0][5] = n[2];

   return v + 1;
}

// Create a gear wheel.
static Gear *
create_gear(GLData *gld, GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
            GLint teeth, GLfloat tooth_depth)
{
   GLfloat r0, r1, r2;
   GLfloat da;
   Gear *gear;
   GearVertex *v;
   double s[5], c[5];
   GLfloat normal[3];
   int cur_strip = 0;
   int i;
   Evas_GL_API *gl = gld->glapi;

   gear = (Gear*)malloc(sizeof(Gear));
   if (gear == NULL)
      return NULL;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth / 2.0;
   r2 = outer_radius + tooth_depth / 2.0;

   da = 2.0 * M_PI / teeth / 4.0;

   // Allocate memory for the triangle strip information
   gear->nstrips = STRIPS_PER_TOOTH * teeth;
   gear->strips = calloc(gear->nstrips, sizeof (*gear->strips));

   // Allocate memory for the vertices
   gear->vertices = calloc(VERTICES_PER_TOOTH * teeth,
                           sizeof(*gear->vertices));

   v = gear->vertices;
   for (i = 0; i < teeth; i++)
     {
        // Calculate needed sin/cos for varius angles
        s[0] = sin(i * 2.0 * M_PI / teeth);
        c[0] = cos(i * 2.0 * M_PI / teeth);
        s[1] = sin(i * 2.0 * M_PI / teeth + da);
        c[1] = cos(i * 2.0 * M_PI / teeth + da);
        s[2] = sin(i * 2.0 * M_PI / teeth + da * 2);
        c[2] = cos(i * 2.0 * M_PI / teeth + da * 2);
        s[3] = sin(i * 2.0 * M_PI / teeth + da * 3);
        c[3] = cos(i * 2.0 * M_PI / teeth + da * 3);
        s[4] = sin(i * 2.0 * M_PI / teeth + da * 4);
        c[4] = cos(i * 2.0 * M_PI / teeth + da * 4);

        // Create the 7 points (only x,y coords) used to draw a tooth
        Point p[7] =
          {
             GEAR_POINT(r2, 1), // 0
             GEAR_POINT(r2, 2), // 1
             GEAR_POINT(r1, 0), // 2
             GEAR_POINT(r1, 3), // 3
             GEAR_POINT(r0, 0), // 4
             GEAR_POINT(r1, 4), // 5
             GEAR_POINT(r0, 4), // 6
          };

        // Front face
        START_STRIP;
        SET_NORMAL(0, 0, 1.0);
        v = GEAR_VERT(v, 0, +1);
        v = GEAR_VERT(v, 1, +1);
        v = GEAR_VERT(v, 2, +1);
        v = GEAR_VERT(v, 3, +1);
        v = GEAR_VERT(v, 4, +1);
        v = GEAR_VERT(v, 5, +1);
        v = GEAR_VERT(v, 6, +1);
        END_STRIP;

        // Inner face
        START_STRIP;
        QUAD_WITH_NORMAL(4, 6);
        END_STRIP;

        // Back face
        START_STRIP;
        SET_NORMAL(0, 0, -1.0);
        v = GEAR_VERT(v, 6, -1);
        v = GEAR_VERT(v, 5, -1);
        v = GEAR_VERT(v, 4, -1);
        v = GEAR_VERT(v, 3, -1);
        v = GEAR_VERT(v, 2, -1);
        v = GEAR_VERT(v, 1, -1);
        v = GEAR_VERT(v, 0, -1);
        END_STRIP;

        // Outer face
        START_STRIP;
        QUAD_WITH_NORMAL(0, 2);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(1, 0);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(3, 1);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(5, 3);
        END_STRIP;
     }

   gear->nvertices = (v - gear->vertices);

   // Store the vertices in a vertex buffer object (VBO)
   gl->glGenBuffers(1, &gear->vbo);
   gl->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);
   gl->glBufferData(GL_ARRAY_BUFFER, gear->nvertices * sizeof(GearVertex),
                    gear->vertices, GL_STATIC_DRAW);

   return gear;
}

static void
free_gear(GLData *gld, Gear *gear)
{
   Evas_GL_API *gl = gld->glapi;

   gl->glDeleteBuffers(1, &gear->vbo);
   free(gear->strips);
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
identity(GLfloat *m)
{
   GLfloat t[16] =
     {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
     };

   memcpy(m, t, sizeof(t));
}

// Transposes a 4x4 matrix.
static void
transpose(GLfloat *m)
{
   GLfloat t[16] =
     {
        m[0], m[4], m[8],  m[12],
        m[1], m[5], m[9],  m[13],
        m[2], m[6], m[10], m[14],
        m[3], m[7], m[11], m[15]
     };

   memcpy(m, t, sizeof(t));
}

/*
 Inverts a 4x4 matrix.

 This function can currently handle only pure translation-rotation matrices.
 Read http://www.gamedev.net/community/forums/topic.asp?topic_id=425118
 for an explanation.
 */
static void
invert(GLfloat *m)
{
   GLfloat t[16];
   identity(t);

   // Extract and invert the translation part 't'. The inverse of a
   // translation matrix can be calculated by negating the translation
   // coordinates.
   t[12] = -m[12]; t[13] = -m[13]; t[14] = -m[14];

   // Invert the rotation part 'r'. The inverse of a rotation matrix is
   // equal to its transpose.
   m[12] = m[13] = m[14] = 0;
   transpose(m);

   // inv(m) = inv(r) * inv(t)
   multiply(m, t);
}

// Calculate a perspective projection transformation.
static void
perspective(GLfloat *m, GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
   GLfloat tmp[16];
   identity(tmp);

   double sine, cosine, cotangent, deltaZ;
   GLfloat radians = fovy / 2 * M_PI / 180;

   deltaZ = zFar - zNear;
   sine = sin(radians);
   cosine = cos(radians);

   if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
      return;

   cotangent = cosine / sine;

   tmp[0] = cotangent / aspect;
   tmp[5] = cotangent;
   tmp[10] = -(zFar + zNear) / deltaZ;
   tmp[11] = -1;
   tmp[14] = -2 * zNear * zFar / deltaZ;
   tmp[15] = 0;

   memcpy(m, tmp, sizeof(tmp));
}

// Draws a gear
static void
draw_gear(GLData *gld, Gear *gear, GLfloat *transform,
          GLfloat tx, GLfloat ty, GLfloat tz,
          GLfloat angle, int rotate_gear, const GLfloat color[4])
{
   Evas_GL_API *gl = gld->glapi;
   GLfloat model_view[16];
   GLfloat normal_matrix[16];
   GLfloat model_view_projection[16];
   int i;

   // Translate and rotate the gear
   memcpy(model_view, transform, sizeof (model_view));
   translate(model_view, tx, ty, tz);
   if (rotate_gear)
      rotate(model_view, 2 * M_PI * 90.0 / 360.0, 0, 1, 0);
   rotate(model_view, 2 * M_PI * angle / 360.0, 0, 0, 1);

   // Create and set the ModelViewProjectionMatrix
   memcpy(model_view_projection, gld->proj_mat, sizeof(model_view_projection));
   multiply(model_view_projection, model_view);

   gl->glUniformMatrix4fv(gld->mvp_loc, 1, GL_FALSE,
                          model_view_projection);

   // Create/set normal matrix: inverse transpose of the mvp
   memcpy(normal_matrix, model_view, sizeof (normal_matrix));
   invert(normal_matrix);
   transpose(normal_matrix);

   gl->glUniformMatrix4fv(gld->norm_mat_loc, 1, GL_FALSE, normal_matrix);

   // Set the gear color
   gl->glUniform4fv(gld->material_loc, 1, color);

   // Set the vertex buffer object to use
   gl->glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

   // Set up the position of the attributes in the vertex buffer object
   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), NULL);
   gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLfloat *)(0 + 3 * sizeof(GLfloat)));

   // Enable the attributes
   gl->glEnableVertexAttribArray(0);
   gl->glEnableVertexAttribArray(1);

   // Draw the triangle strips that comprise the gear
   for (i = 0; i < gear->nstrips; i++)
     {
        gl->glDrawArrays(GL_TRIANGLE_STRIP, gear->strips[i].first, gear->strips[i].count);
     }


   // Disable the attributes
   gl->glDisableVertexAttribArray(1);
   gl->glDisableVertexAttribArray(0);
}

static void
gears_draw(GLData *gld)
{
   Evas_GL_API *gl = gld->glapi;

   static const GLfloat red[4]    = { 0.8, 0.1, 0.0, 1.0 };
   static const GLfloat green[4]  = { 0.0, 0.8, 0.2, 1.0 };
   static const GLfloat blue[4]   = { 0.2, 0.2, 1.0, 1.0 };
   static const GLfloat yellow[4] = { 1.0, 1.0, 0.2, 1.0 };
   static const GLfloat purple[4] = { 0.2, 1.0, 1.0, 1.0 };
   static const GLfloat cyan[4]   = { 1.0, 0.2, 1.0, 1.0 };
   GLfloat transform[16];
   identity(transform);

   gl->glClearColor(0.0, 0.0, 0.0, 0.0);
   gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Translate and rotate the view
   translate(transform, 0, -2.5, -20);
   rotate(transform, 2 * M_PI * gld->view_rot[0] / 360.0, 1, 0, 0);
   rotate(transform, 2 * M_PI * gld->view_rot[1] / 360.0, 0, 1, 0);
   rotate(transform, 2 * M_PI * gld->view_rot[2] / 360.0, 0, 0, 1);

   draw_gear(gld, gld->gear[0], transform, -2.0, -4.0,  0.0, gld->angle            , 0, red);
   draw_gear(gld, gld->gear[1], transform,  4.1, -4.0,  0.0, -2 * gld->angle - 9.0 , 0, green);
   draw_gear(gld, gld->gear[2], transform, -2.1,  2.2,  0.0, -2 * gld->angle - 25.0, 0, blue);
   draw_gear(gld, gld->gear[3], transform, -2.1,  8.4,  0.0, gld->angle            , 0, yellow);
   draw_gear(gld, gld->gear[4], transform,  6.4, -4.0, -4.5, gld->angle            , 1, purple);
   draw_gear(gld, gld->gear[5], transform,  6.4,  4.4, -4.5, -1 * gld->angle - 23.0, 1, cyan);
}

//-------------------------//

static void render_gears(GLData *gld)
{
   if (gld->recreate_gears)
     {
        create_gears(gld);
        gld->recreate_gears = 0;
     }

   gears_draw(gld);

   gld->angle += 2.0;
}

// new window size or exposure
static void
gears_reshape(GLData *gld, int width, int height)
{
   Evas_GL_API *gl = gld->glapi;

   // Update the projection matrix
   perspective(gld->proj_mat, 60.0, width / (float)height, 1.0, 1024.0);

   // Set the viewport
   gl->glViewport(0, 0, (GLint) width, (GLint) height);

}

static const char vertex_shader[] =
   "attribute vec3 position;\n"
   "attribute vec3 normal;\n"
   "uniform mat4 mvp;\n"
   "uniform mat4 norm_mat;\n"
   "uniform vec4 light_pos;\n"
   "uniform vec4 material;\n"
   "varying vec4 color;\n"
   "\n"
   "void main(void)\n"
   "{\n"
   "    // Transform the normal to eye coordinates\n"
   "    vec3 N = normalize(vec3(norm_mat * vec4(normal, 1.0)));\n"
   "\n"
   "    // The LightSourcePosition is actually its direction for directional ight\n"
   "    vec3 L = normalize(light_pos.xyz);\n"
   "\n"
   "    // Multiply the diffuse value by the vertex color (which is fixed in this ase)\n"
   "    // to get the actual color that we will use to draw this vertex with\n"
   "    float diffuse = clamp(dot(N, L), 0.0, 1.0);\n"
   "    color = material*0.2 + diffuse * material;\n"
   "    color.a = 1.0; \n"
   "\n"
   "    // Transform the position to clip coordinates\n"
   "    gl_Position = mvp * vec4(position, 1.0);\n"
   "}";

static const char fragment_shader[] =
   "#ifdef GL_ES\n"
   "precision mediump float;\n"
   "#endif\n"
   "varying vec4 color;\n"
   "\n"
   "void main(void)\n"
   "{\n"
   "    gl_FragColor = color;\n"
   "}";

static void
create_gears(GLData *gld)
{
   int i;

   for (i = 0; i < 6; i++)
      if (gld->gear[i]) free_gear(gld, gld->gear[i]);

   // make the gears
   gld->gear[0] = create_gear(gld, 1.0, 4.0, 1.0, gld->gear_teeth*2, 0.7);
   gld->gear[1] = create_gear(gld, 0.5, 2.0, 2.0, gld->gear_teeth  , 0.7);
   gld->gear[2] = create_gear(gld, 1.3, 2.0, 0.5, gld->gear_teeth  , 0.7);
   gld->gear[3] = create_gear(gld, 2.0, 4.0, 2.0, gld->gear_teeth*2, 0.7);
   gld->gear[4] = create_gear(gld, 1.5, 4.0, 0.5, gld->gear_teeth*2, 1.0);
   gld->gear[5] = create_gear(gld, 2.5, 4.0, 2.5, gld->gear_teeth*2, 1.0);

   gld->tot_vertices = 0;
   for (i = 0; i < 6; i++)
      gld->tot_vertices += gld->gear[i]->nvertices;

   //printf("Teeth: %d, Total Number of vertices %d\n", gld->gear_teeth, gld->tot_vertices);
}

static void
gears_init(GLData *gld)
{
   Evas_GL_API *gl = gld->glapi;

   const char *p;
   char msg[512] = {};

   gl->glEnable(GL_CULL_FACE);
   gl->glEnable(GL_DEPTH_TEST);

   p = vertex_shader;
   gld->vtx_shader = gl->glCreateShader(GL_VERTEX_SHADER);
   gl->glShaderSource(gld->vtx_shader, 1, &p, NULL);
   gl->glCompileShader(gld->vtx_shader);
   gl->glGetShaderInfoLog(gld->vtx_shader, sizeof msg, NULL, msg);
   printf("vertex shader info: %.512s\n", msg);

   p = fragment_shader;
   gld->fgmt_shader = gl->glCreateShader(GL_FRAGMENT_SHADER);
   gl->glShaderSource(gld->fgmt_shader, 1, &p, NULL);
   gl->glCompileShader(gld->fgmt_shader);
   gl->glGetShaderInfoLog(gld->fgmt_shader, sizeof msg, NULL, msg);
   printf("fragment shader info: %.512s\n", msg);

   gld->program = gl->glCreateProgram();
   gl->glAttachShader(gld->program, gld->vtx_shader);
   gl->glAttachShader(gld->program, gld->fgmt_shader);
   gl->glBindAttribLocation(gld->program, 0, "position");
   gl->glBindAttribLocation(gld->program, 1, "normal");

   gl->glLinkProgram(gld->program);
   gl->glGetProgramInfoLog(gld->program, sizeof msg, NULL, msg);
   printf("info: %.512s\n", msg);

   gl->glUseProgram(gld->program);
   gld->mvp_loc  = gl->glGetUniformLocation(gld->program, "mvp");
   gld->norm_mat_loc = gl->glGetUniformLocation(gld->program, "norm_mat");
   gld->light_pos_loc = gl->glGetUniformLocation(gld->program, "light_pos");
   gld->material_loc = gl->glGetUniformLocation(gld->program, "material");
   gl->glUniform4fv(gld->light_pos_loc, 1, gld->light_pos);

   create_gears(gld);
}

static void
gldata_init(GLData *gld)
{
   gld->initialized = 0;
   gld->mouse_down = 0;

   gld->view_rot[0] = 20.0;
   gld->view_rot[1] = 30.0;
   gld->view_rot[2] = 0.0;

   gld->light_pos[0] = 5.0;
   gld->light_pos[1] = 5.0;
   gld->light_pos[2] = 10.0;
   gld->light_pos[3] = 1.0;

   gld->angle = 0.0;

   gld->tot_vertices = 0;
   gld->gear_teeth   = 10;
}

//-------------------------//

static void
_init_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld) return;
   gld->glapi = elm_glview_gl_api_get(obj);

   gears_init(gld);
}

static void
_del_gl(Evas_Object *obj)
{
   int i;
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

   for (i = 0; i < 6; i++)
      if (gld->gear[i]) free_gear(gld, gld->gear[i]);

   evas_object_data_del((Evas_Object*)obj, "gld");
   free(gld);

   Ecore_Animator *ani = evas_object_data_get(obj, "ani");
   ecore_animator_del(ani);

}

static void
_resize_gl(Evas_Object *obj)
{
   int w, h;
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld) return;

   elm_glview_size_get(obj, &w, &h);
   gears_reshape(gld, w, h);
}


static void
_draw_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld) return;
   Evas_GL_API *gl = gld->glapi;

   render_gears(gld);
   gl->glFinish();
}

static Eina_Bool
_anim(void *data)
{
   elm_glview_changed_set((Evas_Object*)data);
   return EINA_TRUE;
}

static Eina_Bool
_quit_idler(void *data)
{
   evas_object_del(data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ecore_idler_add(_quit_idler, data);
}

static void
_on_plus(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   GLData *gld = evas_object_data_get(data, "gld");
   gld->gear_teeth += 1;
   gld->recreate_gears = 1;
}

static void
_on_minus(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   GLData *gld = evas_object_data_get(data, "gld");
   if (gld->gear_teeth > 0)
     {
        gld->gear_teeth -= 1;
        gld->recreate_gears -= 1;
     }
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

   if (strcmp(ev->keyname, "Up") == 0)
     {
        gld->gear_teeth += 1;
        gld->recreate_gears = 1;
        return;
     }

   if (strcmp(ev->keyname, "Down") == 0)
     {
        gld->gear_teeth -= 1;
        gld->recreate_gears = 1;
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
_mouse_move(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   ev = (Evas_Event_Mouse_Move *)event_info;
   GLData *gld = evas_object_data_get(obj, "gld");
   float dx = 0, dy = 0;

   if (gld->mouse_down)
     {
        dx = ev->cur.canvas.x - ev->prev.canvas.x;
        dy = ev->cur.canvas.y - ev->prev.canvas.y;

        gld->view_rot[1] += 1.0 * dx;
        gld->view_rot[0] += 1.0 * dy;
     }
}

static void
_mouse_up(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   gld->mouse_down = 0;
}

//---------------------------//
void
test_glview_manygears(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *bx1, *bx2, *bt_ok, *bt_up, *bt_dn, *gl, *lb;
   Ecore_Animator *ani;
   GLData *gld = NULL;
   char buf[PATH_MAX];

   // alloc a data struct to hold our relevant gl info in
   if (!(gld = calloc(1, sizeof(GLData)))) return;
   gldata_init(gld);

   // new window - do the usual and give it a name, title and delete handler
   win = elm_win_util_standard_add("glview_manygears", "GLView Many Gears");
   elm_win_autodel_set(win, EINA_TRUE);


   // add an image bg
   bg = elm_bg_add(win);
   evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_bg_color_set(bg, 150, 150, 150);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx1 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx1);
   evas_object_show(bx1);


   // Add a GLView
   gl = elm_glview_add(win);
   if (gl)
     {
        evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

        // Add a glview
        elm_glview_mode_set(gl, 0
                            | ELM_GLVIEW_ALPHA
                            | ELM_GLVIEW_DEPTH
                            );
        elm_glview_resize_policy_set(gl, ELM_GLVIEW_RESIZE_POLICY_RECREATE);
        elm_glview_render_policy_set(gl, ELM_GLVIEW_RENDER_POLICY_ALWAYS);
        elm_glview_init_func_set(gl, (Elm_GLView_Func_Cb)_init_gl);
        elm_glview_del_func_set(gl, (Elm_GLView_Func_Cb)_del_gl);
        elm_glview_resize_func_set(gl, (Elm_GLView_Func_Cb)_resize_gl);
        elm_glview_render_func_set(gl, (Elm_GLView_Func_Cb)_draw_gl);
        elm_box_pack_end(bx1, gl);
        evas_object_show(gl);

        // Add Mouse/Key Event Callbacks
        elm_object_focus_set(gl, EINA_TRUE);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_KEY_DOWN, _key_down, gl);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, gl);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_MOUSE_UP, _mouse_up, gl);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, gl);

        // Add animator for rendering
        ani = ecore_animator_add(_anim, gl);
        evas_object_data_set(gl, "ani", ani);
        evas_object_data_set(gl, "gld", gld);
        evas_object_event_callback_add(gl, EVAS_CALLBACK_DEL, _del, gl);

        // Add Up/Down Buttons
        bx2 = elm_box_add(win);
        elm_box_horizontal_set(bx2, EINA_TRUE);
        evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
        evas_object_show(bx2);

        bt_up = elm_button_add(win);
        elm_object_text_set(bt_up, "+ Teeth");
        evas_object_size_hint_align_set(bt_up, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(bt_up, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_box_pack_end(bx2, bt_up);
        evas_object_show(bt_up);
        evas_object_smart_callback_add(bt_up, "clicked", _on_plus, gl);

        bt_dn = elm_button_add(win);
        elm_object_text_set(bt_dn, "- Teeth");
        evas_object_size_hint_align_set(bt_dn, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(bt_dn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_box_pack_end(bx2, bt_dn);
        evas_object_show(bt_dn);
        evas_object_smart_callback_add(bt_dn, "clicked", _on_minus, gl);

        elm_box_pack_end(bx1, bx2);
     }
   else
     {
        lb = elm_label_add(bx1);
        elm_object_text_set(lb, "<align=left> GL backend engine is not supported.<br/>"
                            " 1. Check your back-end engine or<br/>"
                            " 2. Run elementary_test with engine option or<br/>"
                            "    ex) $ <b>ELM_ACCEL=gl</b> elementary_test<br/>"
                            " 3. Change your back-end engine from elementary_config.<br/></align>");
        evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(bx1, lb);
        evas_object_show(lb);
        free(gld);
     }

   // Add Close Button
   bt_ok = elm_button_add(win);
   elm_object_text_set(bt_ok, "Close");
   evas_object_size_hint_align_set(bt_ok, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt_ok, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx1, bt_ok);
   evas_object_show(bt_ok);
   evas_object_smart_callback_add(bt_ok, "clicked", _on_done, win);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
