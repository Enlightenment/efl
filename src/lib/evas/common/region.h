#ifndef _REGION_H_
#define _REGION_H_

/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Id: pixman.h,v 1.21 2005/06/25 01:21:16 jrmuizel Exp $ */

/* pixregion.h */

////////////////////////////////////////////////////////////////////////////
#include "Eina.h"
typedef struct _Region Region;
typedef struct _Box    Box;

typedef enum _Region_State
{
   REGION_STATE_OUT,
   REGION_STATE_IN,
   REGION_STATE_PARTIAL
} Region_State;

struct _Box
{
   int x1, y1, x2, y2;
};

/* creation/destruction */

Region       *region_new          (int w, int h);
void          region_free         (Region *region);
void          region_size_get     (Region *region, int *w, int *h);

void          region_move         (Region *region, int x, int y);
Eina_Bool     region_copy         (Region *dest, Region *source);
Eina_Bool     region_intersect    (Region *dest, Region *source);
Eina_Bool     region_add          (Region *dest, Region *source);
Eina_Bool     region_rect_add     (Region *dest, int x, int y, unsigned int w, unsigned int h);
Eina_Bool     region_del          (Region *dest, Region *source);
Eina_Bool     region_rect_del     (Region *dest, int x, int y, unsigned int w, unsigned int h);

int           region_rects_num    (Region *region);
Box          *region_rects        (Region *region);

Eina_Bool     region_point_inside (Region *region, int x, int y, Box *bx);
Region_State  region_rect_inside  (Region *region, Box *bx);
Eina_Bool     region_exists       (Region *region);
Box          *region_bounds       (Region *region);

Eina_Bool     region_append       (Region *dest, Region *region);
Eina_Bool     region_validate     (Region *region, Eina_Bool *overlap_ret);
void          region_reset        (Region *region, Box *bx);
void          region_empty        (Region *region);

#endif
