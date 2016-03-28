/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

/*
 * Spring model implemented by Kristian Hogsberg.
 */

#include "evas_engine.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Eina.h>
#include <Evas.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "www.h"

# define GRID_WIDTH        4
# define GRID_HEIGHT       4

# define MODEL_MAX_SPRINGS (GRID_WIDTH * GRID_HEIGHT * 2)

# define MASS              15.0f

# define MAXSHORT 32767
# define MINSHORT -MAXSHORT

# define WOBBLY_GRID_RESOLUTION 32
# define WOBBLY_MIN_GRID_SIZE 64

typedef struct _xy_pair
{
   float x, y;
} Point, Vector;

typedef struct _Object
{
   Vector       force;
   Point        position;
   Vector       velocity;
   float        theta;
   Eina_Bool    immobile;
} Object;

typedef struct _Spring
{
   Object *a;
   Object *b;
   Vector  offset;
} Spring;

typedef struct _Model
{
   Object      *objects;
   int          numObjects;
   Spring       springs[MODEL_MAX_SPRINGS];
   int          numSprings;
   Object      *anchorObject;
   float        steps;
   Point        topLeft;
   Point        bottomRight;

   int indexSize;
   GLushort *indices;
   int vertexStride;
   GLfloat *vertices;
   int vertexSize;
   int texCoordSize;

   int wobbleGrace;
   int vCount;
   int indexCount;
   struct timeval tv;

   GLuint fs;
   GLuint vs;
   GLuint prog;
} Model;

static const GLchar *prog_vs =
   "attribute vec3 pos;\n"
   "attribute vec2 tc;\n"
   "varying vec2 f_tc;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = vec4(pos, 1.0);\n"
   "   f_tc = tc;\n"
   "}\n";

static const GLchar *prog_fs =
   "precision mediump float;\n"
   "varying vec2 f_tc;\n"
   "uniform sampler2D ts;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(ts, f_tc);\n"
   "}\n";

static Eina_Bool
setup_shaders(Model *m)
{
   GLint err;
   char errbuf[2048];

   m->vs = glsym_glCreateShader(GL_VERTEX_SHADER);
   glsym_glShaderSource(m->vs, 1, &prog_vs, NULL);
   glsym_glCompileShader(m->vs);
   glsym_glGetShaderiv(m->vs, GL_COMPILE_STATUS, &err);
   if (!err)
     {
        glsym_glGetShaderInfoLog(m->vs, sizeof errbuf, NULL, errbuf);
        fprintf(stderr, "vs sigh: %s\n", errbuf);
        return EINA_FALSE;
     }

   m->fs = glsym_glCreateShader(GL_FRAGMENT_SHADER);
   glsym_glShaderSource(m->fs, 1, &prog_fs, NULL);
   glsym_glCompileShader(m->fs);
   glsym_glGetShaderiv(m->fs, GL_COMPILE_STATUS, &err);
   if (!err)
     {
        glsym_glGetShaderInfoLog(m->fs, sizeof errbuf, NULL, errbuf);
        fprintf(stderr, "fs sigh: %s\n", errbuf);
        return EINA_FALSE;
     }

   m->prog = glsym_glCreateProgram();
   glsym_glAttachShader(m->prog, m->vs);
   glsym_glAttachShader(m->prog, m->fs);
   glsym_glBindAttribLocation(m->prog, 0, "tc");
   glsym_glBindAttribLocation(m->prog, 1, "pos");
   glsym_glLinkProgram(m->prog);
   glsym_glGetProgramiv(m->prog, GL_LINK_STATUS, &err);
   if (!err)
     {
        glsym_glGetProgramInfoLog(m->prog, sizeof errbuf, NULL, errbuf);
        fprintf(stderr, "link : %s\n", errbuf);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}


static void
objectInit(Object *object,
           float positionX,
           float positionY,
           float velocityX,
           float velocityY)
{
   object->force.x = 0;
   object->force.y = 0;

   object->position.x = positionX;
   object->position.y = positionY;

   object->velocity.x = velocityX;
   object->velocity.y = velocityY;

   object->theta = 0;
   object->immobile = EINA_FALSE;
}

static void
springInit(Spring *spring,
           Object *a,
           Object *b,
           float offsetX,
           float offsetY)
{
   spring->a = a;
   spring->b = b;
   spring->offset.x = offsetX;
   spring->offset.y = offsetY;
}

static void
modelCalcBounds(Model *model)
{
   int i;

   model->topLeft.x = MAXSHORT;
   model->topLeft.y = MAXSHORT;
   model->bottomRight.x = MINSHORT;
   model->bottomRight.y = MINSHORT;

   for (i = 0; i < model->numObjects; i++)
     {
        if (model->objects[i].position.x < model->topLeft.x)
          model->topLeft.x = model->objects[i].position.x;
        else if (model->objects[i].position.x > model->bottomRight.x)
          model->bottomRight.x = model->objects[i].position.x;

        if (model->objects[i].position.y < model->topLeft.y)
          model->topLeft.y = model->objects[i].position.y;
        else if (model->objects[i].position.y > model->bottomRight.y)
          model->bottomRight.y = model->objects[i].position.y;
     }
}

static void
modelAddSpring(Model *model,
               Object *a,
               Object *b,
               float offsetX,
               float offsetY)
{
   Spring *spring;

   spring = &model->springs[model->numSprings];
   model->numSprings++;

   springInit(spring, a, b, offsetX, offsetY);
}

static void
modelSetMiddleAnchor(Model *model,
                     int x,
                     int y,
                     int width,
                     int height)
{
   float gx, gy;

   gx = ((GRID_WIDTH - 1) / 2 * width) / (float)(GRID_WIDTH - 1);
   gy = ((GRID_HEIGHT - 1) / 2 * height) / (float)(GRID_HEIGHT - 1);

   if (model->anchorObject)
     model->anchorObject->immobile = EINA_FALSE;

   model->anchorObject = &model->objects[GRID_WIDTH *
                                         ((GRID_HEIGHT - 1) / 2) +
                                         (GRID_WIDTH - 1) / 2];
   model->anchorObject->position.x = x + gx;
   model->anchorObject->position.y = y + gy;

   model->anchorObject->immobile = EINA_TRUE;
}

static void
modelAdjustObjectPosition(Model *model,
                          Object *object,
                          int x,
                          int y,
                          int width,
                          int height)
{
   Object *o;
   int gridX, gridY, i = 0;

   for (gridY = 0; gridY < GRID_HEIGHT; gridY++)
     {
        for (gridX = 0; gridX < GRID_WIDTH; gridX++)
          {
             o = &model->objects[i];
             if (o == object)
               {
                  o->position.x = x + (gridX * width) / (GRID_WIDTH - 1);
                  o->position.y = y + (gridY * height) / (GRID_HEIGHT - 1);

                  return;
               }

             i++;
          }
     }
}

static void
modelInitObjects(Model *model,
                 int x,
                 int y,
                 int width,
                 int height)
{
   int gridX, gridY, i = 0;
   float gw, gh;

   gw = GRID_WIDTH - 1;
   gh = GRID_HEIGHT - 1;

   for (gridY = 0; gridY < GRID_HEIGHT; gridY++)
     {
        for (gridX = 0; gridX < GRID_WIDTH; gridX++)
          {
             objectInit(&model->objects[i],
                        x + (gridX * width) / gw,
                        y + (gridY * height) / gh,
                        0, 0);
             i++;
          }
     }

   modelSetMiddleAnchor(model, x, y, width, height);
}

static void
modelAdjustObjectsForShiver(Model *model,
                            int x,
                            int y,
                            int width,
                            int height)
{
   int gridX, gridY, i = 0;
   float vX, vY;
   float w, h;
   float scale;

   w = width;
   h = height;

   for (gridY = 0; gridY < GRID_HEIGHT; gridY++)
     {
        for (gridX = 0; gridX < GRID_WIDTH; gridX++)
          {
             if (!model->objects[i].immobile)
               {
                  vX = model->objects[i].position.x - (x + w / 2);
                  vY = model->objects[i].position.y - (y + h / 2);

                  vX /= w;
                  vY /= h;

                  scale = ((float)rand() * 7.5f) / RAND_MAX;

                  model->objects[i].velocity.x += vX * scale;
                  model->objects[i].velocity.y += vY * scale;
               }

             i++;
          }
     }
}

static void
modelInitSprings(Model *model,
                 int x EINA_UNUSED,
                 int y EINA_UNUSED,
                 int width,
                 int height)
{
   int gridX, gridY, i = 0;
   float hpad, vpad;

   model->numSprings = 0;

   hpad = ((float)width) / (GRID_WIDTH - 1);
   vpad = ((float)height) / (GRID_HEIGHT - 1);

   for (gridY = 0; gridY < GRID_HEIGHT; gridY++)
     {
        for (gridX = 0; gridX < GRID_WIDTH; gridX++)
          {
             if (gridX > 0)
               modelAddSpring(model,
                              &model->objects[i - 1],
                              &model->objects[i],
                              hpad, 0);

             if (gridY > 0)
               modelAddSpring(model,
                              &model->objects[i - GRID_WIDTH],
                              &model->objects[i],
                              0, vpad);

             i++;
          }
     }
}

Model *
wobbly_create(int x,
              int y,
              int width,
              int height)
{
   Model *model;

   model = calloc(sizeof (Model), 1);
   if (!model)
     return 0;

   model->numObjects = GRID_WIDTH * GRID_HEIGHT;
   model->objects = malloc(sizeof (Object) * model->numObjects);
   if (!model->objects)
     {
        free(model);
        return 0;
     }

   model->anchorObject = 0;
   model->numSprings = 0;

   model->steps = 0;

   modelInitObjects(model, x, y, width, height);
   modelInitSprings(model, x, y, width, height);

   modelCalcBounds(model);

   setup_shaders(model);

   return model;
}

static void
objectApplyForce(Object *object,
                 float fx,
                 float fy)
{
   object->force.x += fx;
   object->force.y += fy;
}

static void
springExertForces(Spring *spring,
                  float k)
{
   Vector da, db;
   Vector a, b;

   a = spring->a->position;
   b = spring->b->position;

   da.x = 0.5f * (b.x - a.x - spring->offset.x);
   da.y = 0.5f * (b.y - a.y - spring->offset.y);

   db.x = 0.5f * (a.x - b.x + spring->offset.x);
   db.y = 0.5f * (a.y - b.y + spring->offset.y);

   objectApplyForce(spring->a, k * da.x, k * da.y);
   objectApplyForce(spring->b, k * db.x, k * db.y);
}

static float
modelStepObject(Object *object, float *force)
{
   float friction = 0.8f;

   object->theta += 0.05f;

   if (object->immobile)
     {
        object->velocity.x = 0.0f;
        object->velocity.y = 0.0f;

        object->force.x = 0.0f;
        object->force.y = 0.0f;

        *force = 0.0f;

        return 0.0f;
     }
   else
     {
        object->force.x -= friction * object->velocity.x;
        object->force.y -= friction * object->velocity.y;

        object->velocity.x += object->force.x / MASS;
        object->velocity.y += object->force.y / MASS;

        object->position.x += object->velocity.x;
        object->position.y += object->velocity.y;

        *force = fabs(object->force.x) + fabs(object->force.y);

        object->force.x = 0.0f;
        object->force.y = 0.0f;

        return fabs(object->velocity.x) + fabs(object->velocity.y);
     }
}

int
modelStep(Model *model, float time)
{
   float k = 0.9;
   int i, j, steps;
   float velocitySum = 0.0f;
   float force, forceSum = 0.0f;
   int wobbly = 0;

   if (model->wobbleGrace)
     {
        model->wobbleGrace--;
        wobbly |= 4;
     }

   model->steps += time / 15.0f;
   steps = floor(model->steps);
   model->steps -= steps;

   if (!steps)
     return 1;

   for (j = 0; j < steps; j++)
     {
        for (i = 0; i < model->numSprings; i++)
          springExertForces(&model->springs[i], k);

        for (i = 0; i < model->numObjects; i++)
          {
             velocitySum += modelStepObject(&model->objects[i], &force);
             forceSum += force;
          }
     }

   modelCalcBounds(model);

   if (velocitySum > 0.5f)
     wobbly |= 1; //WobblyVelocity;

   if (forceSum > 20.0f)
     wobbly |= 2; //WobblyForce;

   return wobbly;
}

static void
bezierPatchEvaluate(Model *model,
                    float u,
                    float v,
                    float *patchX,
                    float *patchY)
{
   float coeffsU[4], coeffsV[4];
   float x, y;
   int i, j;

   coeffsU[0] = (1 - u) * (1 - u) * (1 - u);
   coeffsU[1] = 3 * u * (1 - u) * (1 - u);
   coeffsU[2] = 3 * u * u * (1 - u);
   coeffsU[3] = u * u * u;

   coeffsV[0] = (1 - v) * (1 - v) * (1 - v);
   coeffsV[1] = 3 * v * (1 - v) * (1 - v);
   coeffsV[2] = 3 * v * v * (1 - v);
   coeffsV[3] = v * v * v;

   x = y = 0.0f;

   for (i = 0; i < 4; i++)
     {
        for (j = 0; j < 4; j++)
          {
             x += coeffsU[i] * coeffsV[j] *
               model->objects[j * GRID_WIDTH + i].position.x;
             y += coeffsU[i] * coeffsV[j] *
               model->objects[j * GRID_WIDTH + i].position.y;
          }
     }

   *patchX = x;
   *patchY = y;
}

static float
objectDistance(Object *object,
               float x,
               float y)
{
   float dx, dy;

   dx = object->position.x - x;
   dy = object->position.y - y;

   return sqrt(dx * dx + dy * dy);
}

static Object *
modelFindNearestObject(Model *model,
                       float x,
                       float y)
{
   Object *object = &model->objects[0];
   float distance, minDistance = 0.0;
   int i;

   for (i = 0; i < model->numObjects; i++)
     {
        distance = objectDistance(&model->objects[i], x, y);
        if (i == 0 || distance < minDistance)
          {
             minDistance = distance;
             object = &model->objects[i];
          }
     }

   return object;
}

static Eina_Bool
moreWindowVertices(Model *m, int newSize)
{
   if (newSize > m->vertexSize)
     {
        GLfloat *vertices;

        vertices = realloc(m->vertices, sizeof(GLfloat) * newSize);
        if (!vertices) return EINA_FALSE;

        m->vertices = vertices;
        m->vertexSize = newSize;
     }

   return EINA_TRUE;
}

static Eina_Bool
moreWindowIndices(Model *m, int newSize)
{
   if (newSize > m->indexSize)
     {
        GLushort *indices;

        indices = realloc(m->indices, sizeof(GLushort) * newSize);
        if (!indices) return EINA_FALSE;

        m->indices = indices;
        m->indexSize = newSize;
     }

   return EINA_TRUE;
}

void
wobblyGeometryDo(Model *m, int in_w, int in_h)
{
   int nVertices, nIndices;
   GLushort *i;
   GLfloat *v;
   int x1, y1, x2, y2;
   float width, height;
   float deformedX, deformedY;
   int x, y, iw, ih, wx, wy;
   int vSize;
   int gridW, gridH;
   int bound_w = m->bottomRight.x - m->topLeft.x;
   int bound_h = m->bottomRight.y - m->topLeft.y;

   wx = 0;
   wy = 0;
   width = in_w;
   height = in_h;

   gridW = width / WOBBLY_GRID_RESOLUTION;
   if (gridW < WOBBLY_MIN_GRID_SIZE)
     gridW = WOBBLY_MIN_GRID_SIZE;

   gridH = height / WOBBLY_GRID_RESOLUTION;
   if (gridH < WOBBLY_MIN_GRID_SIZE)
     gridH = WOBBLY_MIN_GRID_SIZE;

   vSize = 3 + 2; /* 3 vert, 2 texcoord */

   nVertices = m->vCount = 0;
   nIndices = m->indexCount = 0;

   v = m->vertices + (nVertices * vSize);
   i = m->indices + nIndices;

   x1 = 0;
   y1 = 0;
   x2 = width;
   y2 = height;

   iw = ((x2 - x1 - 1) / gridW) + 1;
   ih = ((y2 - y1 - 1) / gridH) + 1;

   if (nIndices + (iw * ih * 6) > m->indexSize)
     {
        if (!moreWindowIndices(m, nIndices + (iw * ih * 6)))
          return;

        i = m->indices + nIndices;
     }

   iw++;
   ih++;

   for (y = 0; y < ih - 1; y++)
     {
        for (x = 0; x < iw - 1; x++)
          {
             *i++ = nVertices + iw * (y + 1) + x;
             *i++ = nVertices + iw * (y + 1) + x + 1;
             *i++ = nVertices + iw * y + x + 1;

             *i++ = nVertices + iw * y + x + 1;
             *i++ = nVertices + iw * y + x;
             *i++ = nVertices + iw * (y + 1) + x;

             nIndices += 6;
          }
     }

   if (((nVertices + iw * ih) * vSize) > m->vertexSize)
     {
        if (!moreWindowVertices(m, (nVertices + iw * ih) * vSize))
          return;

        v = m->vertices + (nVertices * vSize);
     }

   for (y = y1;; y += gridH)
     {
        if (y > y2)
          y = y2;

        for (x = x1;; x += gridW)
          {
             if (x > x2)
               x = x2;

             bezierPatchEvaluate(m,
                                 (x - wx) / width,
                                 (y - wy) / height,
                                 &deformedX,
                                 &deformedY);

             //Tex co-ord
             *v++ = x / width;
             *v++ = y / height;

             // vertex
             *v++ = -1 + ((deformedX - m->topLeft.x) / (bound_w / 2.0));
             *v++ = -1 + ((deformedY - m->topLeft.y) / (bound_h / 2.0));
             *v++ = 0.0;

             nVertices++;

             if (x == x2)
               break;
          }

        if (y == y2)
          break;
     }

   m->vCount = nVertices;
   m->vertexStride = vSize;
   m->texCoordSize = 2;
   m->indexCount = nIndices;
}

void
wobblyMap(Model *m, int w, int h)
{
   modelInitObjects(m, 0, 0, w, h);
   modelInitSprings(m, 0, 0, w, h);
   modelSetMiddleAnchor(m, 0, 0, w, h);
   modelAdjustObjectsForShiver(m, 0, 0, w, h);
   m->wobbleGrace = 10;
}

void
wobblyMove(Model *m, int dx, int dy, int w, int h)
{
   m->wobbleGrace = 10;
   modelAdjustObjectPosition(m, m->anchorObject, 0, 0, w, h);
   m->anchorObject->position.x += dx;
   m->anchorObject->position.y += dy;
}

void
wobblyAnchorRelease(Model *m)
{
   if (m->anchorObject)
     m->anchorObject->immobile = EINA_FALSE;
}

void
wobblyAnchor(Model *m, int cx, int cy, int w, int h)
{
   wobblyAnchorRelease(m);
   m->anchorObject = modelFindNearestObject(m, cx, h - cy);
   m->anchorObject->immobile = EINA_TRUE;
   modelAdjustObjectPosition(m, m->anchorObject, 0, 0, w, h);
   m->wobbleGrace = 10;
}

void
wobbly_resize(Model *m, int dwidth, int dheight)
{
   m->wobbleGrace = 10;
   modelInitObjects(m, 0, 0, dwidth, dheight);
   modelInitSprings(m, 0, 0, dwidth, dheight);
   modelCalcBounds(m);
}

void
wobbly_draw(Evas_Engine_GL_Context *gl_context, Model *m)
{
   GLuint tex;

   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);
   glUseProgram(m->prog);
   tex = glsym_evas_gl_common_context_redirect_texture_get(gl_context);
   glBindTexture(GL_TEXTURE_2D, tex);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), m->vertices);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), m->vertices + 2);
   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glDrawElements(GL_TRIANGLES, m->indexCount, GL_UNSIGNED_SHORT, m->indices);
   glDisableVertexAttribArray(1);
   glDisableVertexAttribArray(0);
}

Eina_Bool
wobbly_process(Model *m, Evas_Engine_Info_Wayland_Egl *info, int w, int h, Eina_Bool redirected)
{
   int ret = EINA_FALSE;

   if (info->just_mapped)
     {
        info->just_mapped = EINA_FALSE;
        wobblyMap(m, w, h);
        return EINA_TRUE;
     }

   if (info->drag_start)
     {
        wobblyAnchor(m, info->x_cursor, info->y_cursor, w, h);
        ret = EINA_TRUE;
     }
   if (!info->resizing && (info->x_rel || info->y_rel))
     {
        wobblyMove(m, info->x_rel, -info->y_rel, w, h);
        ret = EINA_TRUE;
     }

   if (info->drag_stop)
     {
        wobblyAnchorRelease(m);
        ret = EINA_TRUE;
     }

   if (redirected)
     {
        double oldms, ms;

        oldms = m->tv.tv_usec / 1000 + m->tv.tv_sec * 1000;
        gettimeofday(&m->tv, NULL);
        ms = m->tv.tv_usec / 1000 + m->tv.tv_sec * 1000 - oldms;

        if (ms > 100) ms = 0;
        if (modelStep(m, ms)) ret = EINA_TRUE;

        wobblyGeometryDo(m, w, h);
     }
   return ret;
}

void
wobbly_bounds(Model *m, float *tlx, float *tly, float *brx, float *bry)
{
   *tlx = m->topLeft.x;
   *tly = m->topLeft.y;
   *brx = m->bottomRight.x;
   *bry = m->bottomRight.y;
}
