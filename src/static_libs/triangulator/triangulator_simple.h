#ifndef TRIANGULATOR_SIMPLE_H_
#define TRIANGULATOR_SIMPLE_H_

#include <Eina.h>
#include "efl/interfaces/efl_gfx_types.eot.h"
typedef Efl_Gfx_Path_Command_Type Efl_Gfx_Path_Command;

typedef struct _Triangulator_Simple Triangulator_Simple;
struct _Triangulator_Simple
{
   Eina_Inarray *vertices;
   Eina_Inarray *stops;  //list of contours need to be drawn as separate triangle fan.
   float minx;
   float miny;
   float maxx;
   float maxy;
};

/**
 * Creates a new simple triangulator.
 *
 */
Triangulator_Simple * triangulator_simple_new(void);

/**
 * Frees the given triangulator and any associated resource.
 *
 * st The given triangulator.
 */
void triangulator_simple_free(Triangulator_Simple *st);

/**
 * Process the command list to generate triangle fans.
 * The alogrithm handles multiple contours by providing the list of stops.
 *
 * cmds :        commnad list
 * pts  :        point list.
 * convex :      shape is convex or not.
 *
 * output: If the shape is convex then, the triangle fan will exactly fill the shape. but if its not convex, it will overflow
 *         to outside shape, in that case user has to use stencil method (2 pass drawing) to fill the shape.
 */
void triangulator_simple_process(Triangulator_Simple *st, const Efl_Gfx_Path_Command *cmds, const double *pts, Eina_Bool convex);

#endif // #endif // TRIANGULATOR_SIMPLE_H_
