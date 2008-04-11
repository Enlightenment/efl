/*  Float arithmetic for the Small AMX engine
 *
 *  Copyright (c) Artran, Inc. 1999
 *  Written by Greg Garner (gmg@artran.com)
 *  Portions Copyright (c) Carsten Haitzler, 2004 <raster@rasterman.com>
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 */

/* CHANGES -
 * 2002-08-27: Basic conversion of source from C++ to C by Adam D. Moss
 *             <adam@gimp.org> <aspirin@icculus.org>
 * 2003-08-29: Removal of the dynamic memory allocation and replacing two
 *             type conversion functions by macros, by Thiadmer Riemersma
 * 2003-09-22: Moved the type conversion macros to AMX.H, and simplifications
 *             of some routines, by Thiadmer Riemersma
 * 2003-11-24: A few more native functions (geometry), plus minor modifications,
 *             mostly to be compatible with dynamically loadable extension
 *             modules, by Thiadmer Riemersma
 * 2004-03-20: Cleaned up and reduced size for Embryo, Modified to conform to
 *             E coding style. Added extra parameter checks.
 *             Carsten Haitzler, <raster@rasterman.com>
 */

/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <math.h>
#include "embryo_private.h"

#define PI  3.1415926535897932384626433832795

/* internally useful calls */

static float
_embryo_fp_degrees_to_radians(float angle, int radix)
{
   switch (radix)
     {
      case 1: /* degrees, sexagesimal system (technically: degrees/minutes/seconds) */
	return (float)(angle * PI / 180.0);
      case 2: /* grades, centesimal system */
	return (float)(angle * PI / 200.0);
      default: /* assume already radian */
	break;
     }
   return angle;
}

/* exported float api */

static Embryo_Cell
_embryo_fp(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = long value to convert to a float */
   float f;

   if (params[0] != (1 * sizeof(Embryo_Cell))) return 0;
   f = (float)params[1];
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_str(Embryo_Program *ep, Embryo_Cell *params)
{
   /* params[1] = virtual string address to convert to a float */
   char buf[64];
   Embryo_Cell *str;
   float f;
   int len;

   if (params[0] != (1 * sizeof(Embryo_Cell))) return 0;
   str = embryo_data_address_get(ep, params[1]);
   len = embryo_data_string_length_get(ep, str);
   if ((len == 0) || (len >= sizeof(buf))) return 0;
   embryo_data_string_get(ep, str, buf);
   f = (float)atof(buf);
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_mul(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand 1 */
   /* params[2] = float operand 2 */
   float f;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]) * EMBRYO_CELL_TO_FLOAT(params[2]);
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_div(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float dividend (top) */
   /* params[2] = float divisor (bottom) */
   float f;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]) / EMBRYO_CELL_TO_FLOAT(params[2]);
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_add(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand 1 */
   /* params[2] = float operand 2 */
   float f;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]) + EMBRYO_CELL_TO_FLOAT(params[2]);
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_sub(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand 1 */
   /* params[2] = float operand 2 */
   float f;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]) - EMBRYO_CELL_TO_FLOAT(params[2]);
   return EMBRYO_FLOAT_TO_CELL(f);
}

/* Return fractional part of float */
static Embryo_Cell
_embryo_fp_fract(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand */
   float f;

   if (params[0] != (1 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   f -= (float)(floor((double)f));
   return EMBRYO_FLOAT_TO_CELL(f);
}

/* Return integer part of float, rounded */
static Embryo_Cell
_embryo_fp_round(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand */
   /* params[2] = Type of rounding (cell) */
   float f;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   switch (params[2])
     {
      case 1: /* round downwards (truncate) */
	f = (float)(floor((double)f));
	break;
      case 2: /* round upwards */
	f = (float)(ceil((double)f));
	break;
      case 3: /* round towards zero */
	if (f >= 0.0) f = (float)(floor((double)f));
	else          f = (float)(ceil((double)f));
	break;
      default: /* standard, round to nearest */
	f = (float)(floor((double)f + 0.5));
	break;
     }
    return (Embryo_Cell)f;
}

static Embryo_Cell
_embryo_fp_cmp(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand 1 */
   /* params[2] = float operand 2 */
   float f, ff;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   ff = EMBRYO_CELL_TO_FLOAT(params[2]);
   if (f == ff) return 0;
   else if (f > ff) return 1;
   return -1;
}

static Embryo_Cell
_embryo_fp_sqroot(Embryo_Program *ep, Embryo_Cell *params)
{
   /* params[1] = float operand */
   float f;

   if (params[0] != (1 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   f = (float)sqrt(f);
   if (f < 0)
     {
	embryo_program_error_set(ep, EMBRYO_ERROR_DOMAIN);
	return 0;
     }
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_power(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand 1 */
   /* params[2] = float operand 2 */
   float f, ff;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   ff = EMBRYO_CELL_TO_FLOAT(params[2]);
   f = (float)pow(f, ff);
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_log(Embryo_Program *ep, Embryo_Cell *params)
{
   /* params[1] = float operand 1 (value) */
   /* params[2] = float operand 2 (base) */
   float f, ff;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   ff = EMBRYO_CELL_TO_FLOAT(params[2]);
   if ((f <= 0.0) || (ff <= 0.0))
     {
	embryo_program_error_set(ep, EMBRYO_ERROR_DOMAIN);
	return 0;
     }
    if (ff == 10.0) f = (float)log10(f);
    else f = (float)(log(f) / log(ff));
    return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_sin(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand 1 (angle) */
   /* params[2] = float operand 2 (radix) */
   float f;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   f = _embryo_fp_degrees_to_radians(f, params[2]);
   f = sin(f);
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_cos(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand 1 (angle) */
   /* params[2] = float operand 2 (radix) */
   float f;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   f = _embryo_fp_degrees_to_radians(f, params[2]);
   f = cos(f);
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_tan(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand 1 (angle) */
   /* params[2] = float operand 2 (radix) */
   float f;

   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   f = _embryo_fp_degrees_to_radians(f, params[2]);
   f = tan(f);
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_fp_abs(Embryo_Program *ep __UNUSED__, Embryo_Cell *params)
{
   /* params[1] = float operand */
   float f;

   if (params[0] != (1 * sizeof(Embryo_Cell))) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   f = (f >= 0) ? f : -f;
   return EMBRYO_FLOAT_TO_CELL(f);
}

/* functions used by the rest of embryo */

void
_embryo_fp_init(Embryo_Program *ep)
{
   embryo_program_native_call_add(ep, "float",     _embryo_fp);
   embryo_program_native_call_add(ep, "atof",      _embryo_fp_str);
   embryo_program_native_call_add(ep, "float_mul", _embryo_fp_mul);
   embryo_program_native_call_add(ep, "float_div", _embryo_fp_div);
   embryo_program_native_call_add(ep, "float_add", _embryo_fp_add);
   embryo_program_native_call_add(ep, "float_sub", _embryo_fp_sub);
   embryo_program_native_call_add(ep, "fract",     _embryo_fp_fract);
   embryo_program_native_call_add(ep, "round",     _embryo_fp_round);
   embryo_program_native_call_add(ep, "float_cmp", _embryo_fp_cmp);
   embryo_program_native_call_add(ep, "sqrt",      _embryo_fp_sqroot);
   embryo_program_native_call_add(ep, "pow",       _embryo_fp_power);
   embryo_program_native_call_add(ep, "log",       _embryo_fp_log);
   embryo_program_native_call_add(ep, "sin",       _embryo_fp_sin);
   embryo_program_native_call_add(ep, "cos",       _embryo_fp_cos);
   embryo_program_native_call_add(ep, "tan",       _embryo_fp_tan);
   embryo_program_native_call_add(ep, "abs",       _embryo_fp_abs);
}
