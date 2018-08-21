/***********************************************************

Copyright 1987, 1988, 1989, 1998  The Open Group

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
 

Copyright 1987, 1988, 1989 by 
Digital Equipment Corporation, Maynard, Massachusetts. 

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

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "region.h"
#include "Eina.h"
#include "evas_macros.h"
typedef struct _Region_Data  Region_Data;

typedef Eina_Bool (*Overlap_Func) (Region *region, Box *r1, Box *r1end, Box *r2, Box *r2end, int y1, int y2, Eina_Bool *overlap_ret);

struct _Region_Data
{
   int size;
   int num;
};

struct _Region
{
   int w, h;
   struct {
      int          x, y;
      unsigned int w, h;
   } last_add, last_del;
   Box          bound;
   Region_Data *data;
};

#define PIXREGION_NIL(reg)       ((reg)->data && !(reg)->data->num)
#define PIXREGION_NAR(reg)       ((reg)->data == &_region_brokendata)
#define PIXREGION_NUM_RECTS(reg) ((reg)->data ? (reg)->data->num : 1)
#define PIXREGION_SIZE(reg)      ((reg)->data ? (reg)->data->size : 0)
#define PIXREGION_RECTS(reg)     ((reg)->data ? (Box *)((reg)->data + 1) : &(reg)->bound)
#define PIXREGION_BOXPTR(reg)    ((Box *)((reg)->data + 1))
#define PIXREGION_BOX(reg, i)    (&PIXREGION_BOXPTR(reg)[i])
#define PIXREGION_TOP(reg)       PIXREGION_BOX(reg, (reg)->data->num)
#define PIXREGION_END(reg)       PIXREGION_BOX(reg, (reg)->data->num - 1)
#define PIXREGION_SZOF(n)        (sizeof(Region_Data) + ((n) * sizeof(Box)))

// r1 and r2 overlap
#define OVERLAP(r1, r2) \
   (!(((r1)->x2 <= (r2)->x1) || ((r1)->x1 >= (r2)->x2)  || \
     ((r1)->y2 <= (r2)->y1) || ((r1)->y1 >= (r2)->y2)))
// x,y inside r
#define INBOX(r, x, y) \
   (((r)->x2 > x) && ((r)->x1 <= x) && ((r)->y2 > y) && ((r)->y1 <= y))
// r1 contains r2
#define CONTAINS(r1, r2) \
   (((r1)->x1 <= (r2)->x1) && ((r1)->x2 >= (r2)->x2) && \
    ((r1)->y1 <= (r2)->y1) && ((r1)->y2 >= (r2)->y2))

#define ALLOC(n) malloc(PIXREGION_SZOF(n))
#define FREE_DATA(reg) if ((reg)->data && ((reg)->data != &_region_brokendata) && (reg)->data->size) free((reg)->data)
#define RECTALLOC_BAIL(pReg ,n, bail) \
   if (!(pReg)->data || (((pReg)->data->num + (n)) > (pReg)->data->size)) \
     { \
        if (!_region_rect_alloc(pReg, n)) goto bail; \
     }
#define RECTALLOC(region, n) \
   if (!(region)->data || \
       (((region)->data->num + (n)) > (region)->data->size)) { \
      if (!_region_rect_alloc(region, n)) return EINA_FALSE; \
   }
#define ADDRECT(r, nx1, ny1, nx2, ny2) \
   { r->x1 = nx1; r->y1 = ny1; r->x2 = nx2; r->y2 = ny2; r++; }
#define NEWRECT(region, next_bx, nx1, ny1, nx2, ny2) { \
   if (!(region)->data || ((region)->data->num == (region)->data->size)) { \
      if (!_region_rect_alloc(region, 1)) return EINA_FALSE; \
        next_bx = PIXREGION_TOP(region); \
    } \
    ADDRECT(next_bx, nx1, ny1, nx2, ny2); \
    region->data->num++; \
}
#define DOWNSIZE(reg, num) \
if (((num) < ((reg)->data->size >> 1)) && ((reg)->data->size > 50)) { \
    Region_Data *new_data; \
    new_data = realloc((reg)->data, PIXREGION_SZOF(num)); \
    if (new_data) { \
       new_data->size = (num); \
       (reg)->data = new_data; \
    } \
}
// Quicky macro to avoid trivial reject procedure calls to _region_coalesce
#define COALESCE(new_reg, prev_band, cur_band) \
   if ((cur_band - prev_band) == (new_reg->data->num - cur_band)) { \
      prev_band = _region_coalesce(new_reg, prev_band, cur_band); \
   } else prev_band = cur_band;
#define FIND_BAND(r, r_band_end, r_end, ry1) { \
   ry1 = r->y1; \
   r_band_end = r + 1; \
   while ((r_band_end != r_end) && (r_band_end->y1 == ry1)) r_band_end++; \
}
#define APPEND_REGIONS(new_reg, r, r_end) { \
   int __new_rects; \
   if ((__new_rects = r_end - r)) { \
      RECTALLOC(new_reg, __new_rects); \
      memmove(PIXREGION_TOP(new_reg), r, __new_rects * sizeof(Box)); \
      new_reg->data->num += __new_rects; \
    } \
}
#define MERGERECT(r) { \
   if (r->x1 <= x2) { \
      /* Merge with current rectangle */ \
      if (r->x1 < x2) *overlap_ret = EINA_TRUE; \
      if (x2 < r->x2) x2 = r->x2; \
   } else { \
      /* Add current rectangle, start new one */ \
      NEWRECT(region, next_bx, x1, y1, x2, y2); \
      x1 = r->x1; \
      x2 = r->x2; \
   } \
   r++; \
}
#define EXCHANGE_RECTS(a, b) { \
   Box __t; \
   __t = rects[a]; \
   rects[a] = rects[b]; \
   rects[b] = __t; \
}

static const Box _region_emptybox = {0, 0, 0, 0};
static const Region_Data _region_emptydata = {0, 0};
static const Region_Data _region_brokendata = {0, 0};

static Eina_Bool _region_break(Region *region);

static Eina_Bool
_region_break(Region *region)
{
   FREE_DATA(region);
   region->bound = _region_emptybox;
   region->data = (Region_Data *)&_region_brokendata;
   return EINA_FALSE;
}

static Eina_Bool
_region_rect_alloc(Region *region, int n)
{
   Region_Data *data;

   if (!region->data)
     {
        n++;
        region->data = ALLOC(n);
        if (!region->data) return _region_break(region);
        region->data->num = 1;
        *PIXREGION_BOXPTR(region) = region->bound;
     }
   else if (!region->data->size)
     {
        region->data = ALLOC(n);
        if (!region->data) return _region_break(region);
        region->data->num = 0;
     }
   else
     {
        if (n == 1)
          {
             n = region->data->num;
             if (n > 500) n = 250;
          }
        n += region->data->num;
        data = realloc(region->data, PIXREGION_SZOF(n));
        if (!data) return _region_break(region);
        region->data = data;
     }
   region->data->size = n;
   return EINA_TRUE;
}

static int
_region_coalesce(Region *region, int prev_start, int cur_start)
{
   Box *prev_bx; // Current box in previous band
   Box *cur_bx; // Current box in current band
   int num; // Number rectangles in both bands
   int y2; // Bottom of current band

   // Figure out how many rectangles are in the band.
   num = cur_start - prev_start;

   if (!num) return cur_start;

   // The bands may only be _region_coalesced if the bottom of the previous
   // matches the top scanline of the current.
   prev_bx = PIXREGION_BOX(region, prev_start);
   cur_bx = PIXREGION_BOX(region, cur_start);
   if (prev_bx->y2 != cur_bx->y1) return cur_start;

   // Make sure the bands have boxes in the same places. This
   // assumes that boxes have been added in such a way that they
   // cover the most area possible. I.e. two boxes in a band must
   // have some horizontal space between them.
   y2 = cur_bx->y2;

   do
     {
        if ((prev_bx->x1 != cur_bx->x1) || (prev_bx->x2 != cur_bx->x2))
          return (cur_start);
        prev_bx++;
        cur_bx++;
        num--;
     }
   while (num);

   // The bands may be merged, so set the bottom y of each box
   // in the previous band to the bottom y of the current band.
   num = cur_start - prev_start;
   region->data->num -= num;
   do
     {
        prev_bx--;
        prev_bx->y2 = y2;
        num--;
     }
   while (num);

   return prev_start;
}

static Eina_Bool
_region_append_non(Region *region, Box *r, Box *r_end, int y1, int y2)
{
   Box *next_bx;
   int new_rects = r_end - r;

   // Make sure we have enough space for all rectangles to be added
   RECTALLOC(region, new_rects);
   next_bx = PIXREGION_TOP(region);
   region->data->num += new_rects;
   do
     {
        ADDRECT(next_bx, r->x1, y1, r->x2, y2);
        r++;
     }
   while (r != r_end);
   return EINA_TRUE;
}

static Eina_Bool
_region_op(Region *dest, // Place to store result
           Region *reg1, // First region in operation
           Region *reg2, // 2d region in operation
           Overlap_Func overlap_func, // Function to call for overlapping bands
           Eina_Bool append_non1, // Append non-overlapping bands in region 1 ?
           Eina_Bool append_non2, // Append non-overlapping bands in region 2 ?
           Eina_Bool *overlap_ret)
{
   Box *r1; // Pointer into first region
   Box *r2; // Pointer into 2d region
   Box *r1_end; // End of 1st region
   Box *r2_end;  // End of 2d region
   int ybot; // Bottom of intersection
   int ytop; // Top of intersection
   Region_Data *old_data; // Old data for dest
   int prev_band; // Index of start of previous band in dest
   int cur_band; // Index of start of curren band in dest
   Box *r1_band_end; // End of current band in r1
   Box *r2_band_end; // End of current band in r2
   int top; // Top of non-overlapping band
   int bot; // Bottom of non-overlapping band
   int r1y1, r2y1; // Temps for r1->y1 and r2->y1
   int new_size, num;

   // Break any region computed from a broken region
   if (PIXREGION_NAR(reg1) || PIXREGION_NAR(reg2))
   return _region_break(dest);

   // Initialization:
   // set r1, r2, r1_end and r2_end appropriately, save the rectangles
   // of the destination region until the end in case it's one of
   // the two source regions, then mark the "new" region empty, allocating
   // another array of rectangles for it to use.

   r1 = PIXREGION_RECTS(reg1);
   new_size = PIXREGION_NUM_RECTS(reg1);
   r1_end = r1 + new_size;
   num = PIXREGION_NUM_RECTS(reg2);
   r2 = PIXREGION_RECTS(reg2);
   r2_end = r2 + num;

   old_data = NULL;
   if (((dest == reg1) && (new_size > 1)) || ((dest == reg2) && (num > 1)))
     {
        old_data = dest->data;
        dest->data = (Region_Data *)&_region_emptydata;
     }
   // guess at new size
   if (num > new_size) new_size = num;
   new_size <<= 1;
   if (!dest->data) dest->data = (Region_Data *)&_region_emptydata;
   else if (dest->data->size)
   dest->data->num = 0;
   if (new_size > dest->data->size)
     {
        if (!_region_rect_alloc(dest, new_size)) return EINA_FALSE;
     }

   // Initialize ybot.
   // In the upcoming loop, ybot and ytop serve different functions depending
   // on whether the band being handled is an overlapping or non-overlapping
   // band.
   // In the case of a non-overlapping band (only one of the regions
   // has points in the band), ybot is the bottom of the most recent
   // intersection and thus clips the top of the rectangles in that band.
   // ytop is the top of the next intersection between the two regions and
   // serves to clip the bottom of the rectangles in the current band.
   // For an overlapping band (where the two regions intersect), ytop clips
   //  the top of the rectangles of both regions and ybot clips the bottoms.
   ybot = MIN(r1->y1, r2->y1);

   // prev_band serves to mark the start of the previous band so rectangles
   // can be _region_coalesced into larger rectangles. qv. _region_coalesce, above.
   // In the beginning, there is no previous band, so prev_band == cur_band
   // (cur_band is set later on, of course, but the first band will always
   // start at index 0). prev_band and cur_band must be indices because of
   // the possible expansion, and resultant moving, of the new region's
   // array of rectangles.
   prev_band = 0;

   do
     {
        // This algorithm proceeds one source-band (as opposed to a
        // destination band, which is determined by where the two regions
        // intersect) at a time. r1_band_end and r2_band_end serve to mark the
        // rectangle after the last one in the current band for their
        // respective regions.
        FIND_BAND(r1, r1_band_end, r1_end, r1y1);
        FIND_BAND(r2, r2_band_end, r2_end, r2y1);

        // First handle the band that doesn't intersect, if any.
        // Note that attention is restricted to one band in the
        // non-intersecting region at once, so if a region has n
        // bands between the current position and the next place it overlaps
        // the other, this entire loop will be passed through n times.
        if (r1y1 < r2y1)
          {
             if (append_non1)
               {
                  top = MAX(r1y1, ybot);
                  bot = MIN(r1->y2, r2y1);
                  if (top != bot)
                    {
                       cur_band = dest->data->num;
                       _region_append_non(dest, r1, r1_band_end, top, bot);
                       COALESCE(dest, prev_band, cur_band);
                    }
               }
             ytop = r2y1;
          }
        else if (r2y1 < r1y1)
          {
             if (append_non2)
               {
                  top = MAX(r2y1, ybot);
                  bot = MIN(r2->y2, r1y1);
                  if (top != bot)
                    {
                       cur_band = dest->data->num;
                       _region_append_non(dest, r2, r2_band_end, top, bot);
                       COALESCE(dest, prev_band, cur_band);
                    }
               }
             ytop = r1y1;
          }
        else ytop = r1y1;

        // Now see if we've hit an intersecting band. The two bands only
        // intersect if ybot > ytop
        ybot = MIN(r1->y2, r2->y2);
        if (ybot > ytop)
          {
             cur_band = dest->data->num;
             overlap_func(dest, r1, r1_band_end, r2, r2_band_end, ytop, ybot,
                          overlap_ret);
             COALESCE(dest, prev_band, cur_band);
          }

        // If we've finished with a band (y2 == ybot) we skip forward
        // in the region to the next band.
        if (r1->y2 == ybot) r1 = r1_band_end;
        if (r2->y2 == ybot) r2 = r2_band_end;
     }
   while ((r1 != r1_end) && (r2 != r2_end));

   // Deal with whichever region (if any) still has rectangles left.
   // We only need to worry about banding and coalescing for the very first
   // band left.  After that, we can just group all remaining boxes,
   // regardless of how many bands, into one final append to the list.

   if ((r1 != r1_end) && append_non1)
     {
        // Do first nonOverlap1Func call, which may be able to _region_coalesce
        FIND_BAND(r1, r1_band_end, r1_end, r1y1);
        cur_band = dest->data->num;
        _region_append_non(dest, r1, r1_band_end, MAX(r1y1, ybot), r1->y2);
        COALESCE(dest, prev_band, cur_band);
        // Just append the rest of the boxes
        APPEND_REGIONS(dest, r1_band_end, r1_end);
     }
   else if ((r2 != r2_end) && append_non2)
     {
        // Do first nonOverlap2Func call, which may be able to _region_coalesce
        FIND_BAND(r2, r2_band_end, r2_end, r2y1);
        cur_band = dest->data->num;
        _region_append_non(dest, r2, r2_band_end, MAX(r2y1, ybot), r2->y2);
        COALESCE(dest, prev_band, cur_band);
        /// Append rest of boxes
        APPEND_REGIONS(dest, r2_band_end, r2_end);
     }

   if (old_data) free(old_data);

   if (!(num = dest->data->num))
     {
        FREE_DATA(dest);
        dest->data = (Region_Data *)&_region_emptydata;
     }
   else if (num == 1)
    {
       dest->bound = *PIXREGION_BOXPTR(dest);
       FREE_DATA(dest);
       dest->data = NULL;
    }
   else DOWNSIZE(dest, num);

   return EINA_TRUE;
}

static void
_region_set_bound(Region *region)
{
   Box *bx, *bx_end;

   if (!region->data) return;
   if (!region->data->size)
     {
        region->bound.x2 = region->bound.x1;
        region->bound.y2 = region->bound.y1;
        return;
     }

   bx = PIXREGION_BOXPTR(region);
   bx_end = PIXREGION_END(region);

   // Since bx is the first rectangle in the region, it must have the
   // smallest y1 and since bx_end is the last rectangle in the region,
   // it must have the largest y2, because of banding. Initialize x1 and
   // x2 from  bx and bx_end, resp., as good things to initialize them
   // to...
   region->bound.x1 = bx->x1;
   region->bound.y1 = bx->y1;
   region->bound.x2 = bx_end->x2;
   region->bound.y2 = bx_end->y2;

   while (bx <= bx_end)
     {
        if (bx->x1 < region->bound.x1) region->bound.x1 = bx->x1;
        if (bx->x2 > region->bound.x2) region->bound.x2 = bx->x2;
        bx++;
    };
}

static Eina_Bool
_region_intersect(Region *region, Box *r1, Box *r1_end, Box *r2, Box *r2_end,
                  int y1, int y2, Eina_Bool *overlap_ret EINA_UNUSED)
{
   int x1, x2;
   Box *next_bx = PIXREGION_TOP(region);

   do
     {
        x1 = MAX(r1->x1, r2->x1);
        x2 = MIN(r1->x2, r2->x2);

        // If there's any overlap between the two rectangles, add that
        // overlap to the new region.
        if (x1 < x2) NEWRECT(region, next_bx, x1, y1, x2, y2);

        // Advance the pointer(s) with the leftmost right side, since the next
        // rectangle on that list may still overlap the other region's
        // current rectangle.
        if (r1->x2 == x2) r1++;
        if (r2->x2 == x2) r2++;
     }
   while ((r1 != r1_end) && (r2 != r2_end));

   return EINA_TRUE;
}

static Eina_Bool
_region_add(Region *region, Box *r1, Box *r1_end, Box *r2, Box *r2_end,
            int y1, int y2, Eina_Bool *overlap_ret)
{
   Box *next_bx;
   int  x1, x2; // left and right side of current union

   next_bx = PIXREGION_TOP(region);
   // Start off current rectangle
   if (r1->x1 < r2->x1)
     {
        x1 = r1->x1;
        x2 = r1->x2;
        r1++;
     }
   else
     {
        x1 = r2->x1;
        x2 = r2->x2;
        r2++;
     }
   while ((r1 != r1_end) && (r2 != r2_end))
     {
        if (r1->x1 < r2->x1) MERGERECT(r1)
        else MERGERECT(r2);
     }

   // Finish off whoever (if any) is left
   if (r1 != r1_end)
     {
        do
          {
             MERGERECT(r1);
          }
        while (r1 != r1_end);
     }
   else if (r2 != r2_end)
     {
        do
          {
             MERGERECT(r2);
          }
        while (r2 != r2_end);
     }

   // Add current rectangle
   NEWRECT(region, next_bx, x1, y1, x2, y2);
   return EINA_TRUE;
}

static void
_region_rects_sort(Box *rects, int num)
{
   int x1, y1, i, j;
   Box *r;

   // Always called with num > 1
   do
     {
        if (num == 2)
          {
             if ((rects[0].y1 > rects[1].y1) ||
                 ((rects[0].y1 == rects[1].y1) && (rects[0].x1 > rects[1].x1)))
               EXCHANGE_RECTS(0, 1);
             return;
          }

        // Choose partition element, stick in location 0
        EXCHANGE_RECTS(0, num >> 1);
        y1 = rects[0].y1;
        x1 = rects[0].x1;
        // Partition array
        i = 0;
        j = num;
        do
          {
             r = &(rects[i]);
             do
               {
                  r++;
                  i++;
               }
             while ((i != num) &&
                    ((r->y1 < y1) || ((r->y1 == y1) && (r->x1 < x1))));
             r = &(rects[j]);
             do
               {
                  r--;
                  j--;
               }
             while (y1 < r->y1 || ((y1 == r->y1) && (x1 < r->x1)));
             if (i < j) EXCHANGE_RECTS(i, j);
          }
        while (i < j);

        // Move partition element back to middle
        EXCHANGE_RECTS(0, j);
        // Recurse
        if ((num - j - 1) > 1)
          _region_rects_sort(&rects[j + 1], num - j - 1);
        num = j;
     }
   while (num > 1);
}

static Eina_Bool
_region_del(Region *region, Box *r1, Box *r1end, Box *r2, Box *r2end,
            int y1, int y2, Eina_Bool *overlap_ret EINA_UNUSED)
{
   Box *next_bx;
   int  x1;

   x1 = r1->x1;
   next_bx = PIXREGION_TOP(region);

   do
     {
        // Subtrahend entirely to left of minuend: go to next subtrahend.
        if (r2->x2 <= x1) r2++;
        else if (r2->x1 <= x1)
          {
             // Subtrahend preceeds minuend: nuke left edge of minuend.
             x1 = r2->x2;
             if (x1 >= r1->x2)
               {
                  // Minuend completely covered: advance to next minuend and
                  // reset left fence to edge of new minuend.
                  r1++;
                  if (r1 != r1end) x1 = r1->x1;
               }
             // Subtrahend now used up since it doesn't extend beyond minuend
             else r2++;
          }
        else if (r2->x1 < r1->x2)
          {
             // Left part of subtrahend covers part of minuend: add uncovered
             // part of minuend to region and skip to next subtrahend.
             NEWRECT(region, next_bx, x1, y1, r2->x1, y2);
             x1 = r2->x2;
             if (x1 >= r1->x2)
               {
                  // Minuend used up: advance to new...
                  r1++;
                  if (r1 != r1end) x1 = r1->x1;
               }
             // Subtrahend used up
             else r2++;
          }
        else
          {
             // Minuend used up: add any remaining piece before advancing.
             if (r1->x2 > x1) NEWRECT(region, next_bx, x1, y1, r1->x2, y2);
             r1++;
             if (r1 != r1end) x1 = r1->x1;
          }
     }
   while ((r1 != r1end) && (r2 != r2end));

   // Add remaining minuend rectangles to region.
   while (r1 != r1end)
     {
        NEWRECT(region, next_bx, x1, y1, r1->x2, y2);
        r1++;
        if (r1 != r1end) x1 = r1->x1;
     }
   return EINA_TRUE;
}

///////////////////////////////////////////////////////////////////////////

Region *
region_new(int w, int h)
{
   Region *region = calloc(1, sizeof(Region));
   if (!region) return NULL;
   region->bound = _region_emptybox;
   region->data = (Region_Data *)&_region_emptydata;
   region->w = w;
   region->h = h;
   return region;
}

void
region_free(Region *region)
{
   if (!region) return;
   FREE_DATA(region);
   free(region);
}

void
region_size_get(Region *region, int *w, int *h)
{
   *w = region->w;
   *h = region->h;
}

int
region_rects_num(Region *region)
{
   return PIXREGION_NUM_RECTS(region);
}

Box *
region_rects(Region *region)
{
   return PIXREGION_RECTS(region);
}

Eina_Bool
region_copy(Region *dest, Region *src)
{
   dest->last_del.w = 0;
   dest->last_add.w = 0;

   dest->w = src->w;
   dest->h = src->h;

   dest->bound = src->bound;
   if ((!src->data) || (!src->data->size))
     {
        FREE_DATA(dest);
        dest->data = src->data;
        return EINA_TRUE;
     }
   if ((!dest->data) || (dest->data->size < src->data->num))
     {
        FREE_DATA(dest);
        dest->data = ALLOC(src->data->num);
        if (!dest->data) return _region_break(dest);
        dest->data->size = src->data->num;
     }
   dest->data->num = src->data->num;
   memmove(PIXREGION_BOXPTR(dest), PIXREGION_BOXPTR(src),
           dest->data->num * sizeof(Box));
   return EINA_TRUE;
}

Eina_Bool
region_intersect(Region *dest, Region *source)
{
   dest->last_del.w = 0;
   dest->last_add.w = 0;

   // check for trivial reject
   if (PIXREGION_NIL(dest) || PIXREGION_NIL(source) ||
       !OVERLAP(&dest->bound, &source->bound))
     {
        // Covers about 20% of all cases
        dest->bound.x2 = dest->bound.x1;
        dest->bound.y2 = dest->bound.y1;
        if (PIXREGION_NAR(dest) || PIXREGION_NAR(source))
          {
             FREE_DATA(dest);
             dest->data = (Region_Data *)&_region_brokendata;
             return EINA_FALSE;
          }
        else
          {
             FREE_DATA(dest);
             dest->data = (Region_Data *)&_region_emptydata;
          }
     }
   else if (!dest->data && !source->data)
     {
        // Covers about 80% of cases that aren't trivially rejected
        dest->bound.x1 = MAX(dest->bound.x1, source->bound.x1);
        dest->bound.y1 = MAX(dest->bound.y1, source->bound.y1);
        dest->bound.x2 = MIN(dest->bound.x2, source->bound.x2);
        dest->bound.y2 = MIN(dest->bound.y2, source->bound.y2);
        FREE_DATA(dest);
        dest->data = NULL;
     }
   else if (!source->data && CONTAINS(&source->bound, &dest->bound))
     return region_copy(dest, dest);
   else if (!dest->data && CONTAINS(&dest->bound, &source->bound))
     return region_copy(dest, source);
   else if (dest == source)
     return region_copy(dest, dest);
   else
     {
        // General purpose intersection
        Eina_Bool overlap;

        if (!_region_op(dest, dest, source, _region_intersect,
                        EINA_FALSE, EINA_FALSE, &overlap))
          return EINA_FALSE;
        _region_set_bound(dest);
     }
   return EINA_TRUE;
}

Eina_Bool
region_add(Region *dest, Region *source)
{
   Eina_Bool overlap;

   // Return EINA_TRUE if some overlap between dest, source
   // checks all the simple cases

   dest->last_del.w = 0;
   dest->last_add.w = 0;

   // Region 1 is empty
   if (PIXREGION_NIL(dest))
     {
        if (PIXREGION_NAR(dest)) return _region_break(dest);
        if (dest != source) return region_copy(dest, source);
        return EINA_TRUE;
     }

   // Region 2 is empty
   if (PIXREGION_NIL(source)) return EINA_TRUE;

   // Region 1 completely subsumes region 2
   if (!dest->data && CONTAINS(&dest->bound, &source->bound))
        return EINA_TRUE;

   // Region 2 completely subsumes region 1
   if (!source->data && CONTAINS(&source->bound, &dest->bound))
     {
        if (dest != source) return region_copy(dest, source);
        return EINA_TRUE;
     }

   if (!_region_op(dest, dest, source, _region_add,
                   EINA_TRUE, EINA_TRUE, &overlap))
     return EINA_FALSE;

   dest->bound.x1 = MIN(dest->bound.x1, source->bound.x1);
   dest->bound.y1 = MIN(dest->bound.y1, source->bound.y1);
   dest->bound.x2 = MAX(dest->bound.x2, source->bound.x2);
   dest->bound.y2 = MAX(dest->bound.y2, source->bound.y2);
   return EINA_TRUE;
}

Eina_Bool
region_rect_add(Region *dest, int x, int y, unsigned int w, unsigned int h)
{
   Region region;
   Eina_Bool ret;

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, dest->w, dest->h);

   if (!w || !h) return EINA_FALSE;

   if (dest->last_add.w > 0)
     {
        if ((dest->last_add.x == x) && (dest->last_add.y == y) &&
            (dest->last_add.w == w) && (dest->last_add.h == h))
          return EINA_TRUE;
     }

   region.w = dest->w;
   region.h = dest->h;
   region.data = NULL;
   region.bound.x1 = x;
   region.bound.y1 = y;
   region.bound.x2 = x + w;
   region.bound.y2 = y + h;
   ret = region_add(dest, &region);
   dest->last_add.x = x;
   dest->last_add.y = y;
   dest->last_add.w = w;
   dest->last_add.h = h;
   dest->last_del.w = 0;
   return ret;
}

Eina_Bool
region_append(Region *dest, Region *region)
{
   int num, dnum, size, prepend;
   Box *bx, *old;

   dest->last_del.w = 0;
   dest->last_add.w = 0;

   if (PIXREGION_NAR(region)) return _region_break(dest);

   if (!region->data && (dest->data == &_region_emptydata))
     {
        dest->bound = region->bound;
        dest->data = NULL;
        return EINA_TRUE;
     }

   num = PIXREGION_NUM_RECTS(region);
   if (!num) return EINA_TRUE;
   prepend = EINA_FALSE;
   size = num;
   dnum = PIXREGION_NUM_RECTS(dest);
   if (!dnum && (size < 200)) size = 200;
   RECTALLOC(dest, size);
   old = PIXREGION_RECTS(region);
   if (!dnum) dest->bound = region->bound;
   else if (dest->bound.x2 > dest->bound.x1)
     {
        Box *first, *last;

        first = old;
        last = PIXREGION_BOXPTR(dest) + (dnum - 1);
        if ((first->y1 > last->y2) ||
            ((first->y1 == last->y1) && (first->y2 == last->y2) &&
             (first->x1 > last->x2)))
          {
             if (region->bound.x1 < dest->bound.x1)
               dest->bound.x1 = region->bound.x1;
             if (region->bound.x2 > dest->bound.x2)
               dest->bound.x2 = region->bound.x2;
             dest->bound.y2 = region->bound.y2;
          }
        else
          {
             first = PIXREGION_BOXPTR(dest);
             last = old + (num - 1);
             if ((first->y1 > last->y2) ||
                 ((first->y1 == last->y1) && (first->y2 == last->y2) &&
                  (first->x1 > last->x2)))
               {
                  prepend = EINA_TRUE;
                  if (region->bound.x1 < dest->bound.x1)
                    dest->bound.x1 = region->bound.x1;
                  if (region->bound.x2 > dest->bound.x2)
                    dest->bound.x2 = region->bound.x2;
                  dest->bound.y1 = region->bound.y1;
               }
             else dest->bound.x2 = dest->bound.x1;
          }
     }
   if (prepend)
     {
        bx = PIXREGION_BOX(dest, num);
        if (dnum == 1) *bx = *PIXREGION_BOXPTR(dest);
        else memmove(bx, PIXREGION_BOXPTR(dest), dnum * sizeof(Box));
        bx = PIXREGION_BOXPTR(dest);
     }
   else bx = PIXREGION_BOXPTR(dest) + dnum;
   if (num == 1) *bx = *old;
   else memmove(bx, old, num * sizeof(Box));
   dest->data->num += num;
   return EINA_TRUE;
}

Eina_Bool
region_validate(Region *region, Eina_Bool *overlap_ret)
{
   // Descriptor for regions under construction  in Step 2.
   typedef struct
     {
        Region reg;
        int    prev_band;
        int    cur_band;
     } Region_Info;

   int num; // Original num for region
   Region_Info *ri; // Array of current regions
   int num_ri; // Number of entries used in ri
   int size_ri; // Number of entries available in ri
   int i, j; // Index into rects, and ri
   Region_Info *rit; // &ri[j]
   Region *reg; // ri[j].reg
   Box *box; // Current box in rects
   Box *ri_box; // Last box in ri[j].reg
   Region *hreg; // ri[j_half].reg
   Eina_Bool ret = EINA_TRUE;

   *overlap_ret = EINA_FALSE;
   if (!region->data) return EINA_TRUE;

   region->last_del.w = 0;
   region->last_add.w = 0;

   num = region->data->num;
   if (!num)
     {
        if (PIXREGION_NAR(region)) return EINA_FALSE;
        return EINA_TRUE;
     }
   if (region->bound.x1 < region->bound.x2)
     {
        if (num == 1)
          {
             FREE_DATA(region);
             region->data = NULL;
          }
        else DOWNSIZE(region, num);
        return EINA_TRUE;
     }

   // Step 1: Sort the rects array into ascending (y1, x1) order
   _region_rects_sort(PIXREGION_BOXPTR(region), num);

   // Step 2: Scatter the sorted array into the minimum number of regions

   // Set up the first region to be the first rectangle in region
   // Note that step 2 code will never overflow the ri[0].reg rects array
   ri = calloc(1, 4 * sizeof(Region_Info));
   if (!ri) return _region_break(region);
   size_ri = 4;
   num_ri = 1;
   ri[0].reg = *region;
   box = PIXREGION_BOXPTR(&ri[0].reg);
   ri[0].reg.bound = *box;
   ri[0].reg.data->num = 1;
   ri[0].reg.w = region->w;
   ri[0].reg.h = region->h;

   // Now scatter rectangles into the minimum set of valid regions.  If the
   // next rectangle to be added to a region would force an existing rectangle
   // in the region to be split up in order to maintain y-x banding, just
   // forget it.  Try the next region.  If it doesn't fit cleanly into any
   // region, make a new one.

   for (i = num; --i > 0;)
     {
        box++;
        // Look for a region to append box to
        for (j = num_ri, rit = ri; --j >= 0; rit++)
          {
             reg = &rit->reg;
             ri_box = PIXREGION_END(reg);

             if ((box->y1 == ri_box->y1) && (box->y2 == ri_box->y2))
               {
                  // box is in same band as ri_box.  Merge or append it
                  if (box->x1 <= ri_box->x2)
                    {
                       // Merge it with ri_box
                       if (box->x1 < ri_box->x2) *overlap_ret = EINA_TRUE;
                       if (box->x2 > ri_box->x2) ri_box->x2 = box->x2;
                    }
                  else
                    {
                       RECTALLOC_BAIL(reg, 1, bail);
                       *PIXREGION_TOP(reg) = *box;
                       reg->data->num++;
                    }
                  goto next_rect;
               }
             else if (box->y1 >= ri_box->y2)
               {
                  // Put box into new band
                  if (reg->bound.x2 < ri_box->x2) reg->bound.x2 = ri_box->x2;
                  if (reg->bound.x1 > box->x1)   reg->bound.x1 = box->x1;
                  COALESCE(reg, rit->prev_band, rit->cur_band);
                  rit->cur_band = reg->data->num;
                  RECTALLOC_BAIL(reg, 1, bail);
                  *PIXREGION_TOP(reg) = *box;
                  reg->data->num++;
                  goto next_rect;
               }
             // Well, this region was inappropriate.  Try the next one.
          }

        // Uh-oh.  No regions were appropriate.  Create a new one.
        if (size_ri == num_ri)
          {
             // Oops, allocate space for new region information
             size_ri <<= 1;
             rit = realloc(ri, size_ri * sizeof(Region_Info));
             if (!rit) goto bail;
             ri = rit;
             rit = &ri[num_ri];
          }
        num_ri++;
        rit->prev_band = 0;
        rit->cur_band = 0;
        rit->reg.bound = *box;
        rit->reg.data = NULL;
        rit->reg.w = region->w;
        rit->reg.h = region->h;
        // MUST force allocation
        if (!_region_rect_alloc(&rit->reg, (i + num_ri) / num_ri)) goto bail;
        next_rect: ;
     }

   // Make a final pass over each region in order to COALESCE and set
   // bound.x2 and bound.y2

   for (j = num_ri, rit = ri; --j >= 0; rit++)
     {
        reg = &rit->reg;
        ri_box = PIXREGION_END(reg);
        reg->bound.y2 = ri_box->y2;
        if (reg->bound.x2 < ri_box->x2) reg->bound.x2 = ri_box->x2;
        COALESCE(reg, rit->prev_band, rit->cur_band);
        if (reg->data->num == 1) // keep unions happy below
          {
             FREE_DATA(reg);
             reg->data = NULL;
          }
     }

   // Step 3: Union all regions into a single region
   while (num_ri > 1)
     {
        int half = num_ri / 2;
        for (j = num_ri & 1; j < (half + (num_ri & 1)); j++)
          {
             reg = &ri[j].reg;
             hreg = &ri[j + half].reg;
             if (!_region_op(reg, reg, hreg, _region_add,
                             EINA_TRUE, EINA_TRUE, overlap_ret))
               ret = EINA_FALSE;
             if (hreg->bound.x1 < reg->bound.x1)
               reg->bound.x1 = hreg->bound.x1;
             if (hreg->bound.y1 < reg->bound.y1)
               reg->bound.y1 = hreg->bound.y1;
             if (hreg->bound.x2 > reg->bound.x2)
               reg->bound.x2 = hreg->bound.x2;
             if (hreg->bound.y2 > reg->bound.y2)
               reg->bound.y2 = hreg->bound.y2;
             FREE_DATA(hreg);
          }
        num_ri -= half;
     }
   *region = ri[0].reg;
   free(ri);
   return ret;
   bail:
   for (i = 0; i < num_ri; i++) FREE_DATA(&ri[i].reg);
   free (ri);
   return _region_break(region);
}

Eina_Bool
region_del(Region *dest, Region *source)
{
   Eina_Bool overlap;

   dest->last_del.w = 0;
   dest->last_add.w = 0;

   // check for trivial rejects
   if (PIXREGION_NIL(dest) || PIXREGION_NIL(source) ||
       (!OVERLAP(&dest->bound, &source->bound)))
     {
        if (PIXREGION_NAR(source)) return _region_break(dest);
        return region_copy(dest, dest);
     }
   else if (dest == source)
     {
        FREE_DATA(dest);
        dest->bound.x2 = dest->bound.x1;
        dest->bound.y2 = dest->bound.y1;
        dest->data = (Region_Data *)&_region_emptydata;
        return EINA_TRUE;
     }

   // Add those rectangles in region 1 that aren't in region 2,
   // do yucky subtraction for overlaps, and
   // just throw away rectangles in region 2 that aren't in region 1
   if (!_region_op(dest, dest, source, _region_del,
                   EINA_TRUE, EINA_FALSE, &overlap))
     return EINA_FALSE;

   // Can't alter RegD's bound before we call op because
   // it might be one of the source regions and op depends
   // on the bound of those regions being unaltered. Besides, this
   // way there's no checking against rectangles that will be nuked
   // due to coalescing, so we have to examine fewer rectangles.
   _region_set_bound(dest);
   return EINA_TRUE;
}

Eina_Bool
region_rect_del(Region *dest, int x, int y, unsigned int w, unsigned int h)
{
   Region region;
   Eina_Bool ret;

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, dest->w, dest->h);

   if (!w || !h) return EINA_FALSE;

   if (dest->last_del.w > 0)
     {
        if ((dest->last_del.x == x) && (dest->last_del.y == y) &&
            (dest->last_del.w == w) && (dest->last_del.h == h))
          return EINA_TRUE;
     }

   region.w = dest->w;
   region.h = dest->h;
   region.data = NULL;
   region.bound.x1 = x;
   region.bound.y1 = y;
   region.bound.x2 = x + w;
   region.bound.y2 = y + h;
   ret = region_del(dest, &region);
   dest->last_del.x = x;
   dest->last_del.y = y;
   dest->last_del.w = w;
   dest->last_del.h = h;
   dest->last_add.w = 0;
   return ret;
}

Region_State
region_rect_inside(Region *region, Box *bx)
{
   int x, y, num;
   Region_State part_in = REGION_STATE_OUT, part_out = REGION_STATE_OUT;
   Box *pbx, *bxend;

   num = PIXREGION_NUM_RECTS(region);
   if (!num || (!OVERLAP(&region->bound, bx))) return REGION_STATE_OUT;

   if (num == 1)
     {
        // We know that it must be REGION_STATE_IN or REGION_STATE_PARTIAL
        if (CONTAINS(&region->bound, bx)) return REGION_STATE_IN;
        return REGION_STATE_PARTIAL;
     }

   // (x,y) starts at upper left of rect, moving to the right and down 
   x = bx->x1;
   y = bx->y1;

   // can stop when both part_out and part_in are EINA_TRUE, or we reach bx->y2
   for (pbx = PIXREGION_BOXPTR(region), bxend = pbx + num; pbx != bxend; pbx++)
     {
        // getting up to speed or skipping remainder of band
        if (pbx->y2 <= y) continue;
        if (pbx->y1 > y)
          {
             part_out = EINA_TRUE; // missed part of rectangle above
             if (part_in || (pbx->y1 >= bx->y2)) break;
             y = pbx->y1; // x guaranteed to be == bx->x1
          }
        if (pbx->x2 <= x) continue; // not far enough over yet

        if (pbx->x1 > x)
          {
             part_out = EINA_TRUE; // missed part of rectangle to left
             if (part_in) break;
          }

        if (pbx->x1 < bx->x2)
          {
             part_in = EINA_TRUE; // definitely overlap
             if (part_out) break;
          }

        if (pbx->x2 >= bx->x2)
          {
             y = pbx->y2; // finished with this band
             if (y >= bx->y2) break;
             x = bx->x1; // reset x out to left again
          }
        else
          {
             // Because boxes in a band are maximal width, if the first box
             // to overlap the rectangle doesn't completely cover it in that
             // band, the rectangle must be partially out, since some of it
             // will be uncovered in that band. part_in will have been set true
             // by now...
             part_out = EINA_TRUE;
             break;
          }
     }
   if (part_in)
     {
        if (y < bx->y2) return REGION_STATE_PARTIAL;
        return REGION_STATE_IN;
     }
   return REGION_STATE_OUT;
}

void
region_move(Region *region, int x, int y)
{
   int num;
   Box *bx;

   if ((x == 0) && (y == 0)) return;
   region->last_del.w = 0;
   region->last_add.w = 0;
   region->bound.x1 += x;
   region->bound.y1 += y;
   region->bound.x2 += x;
   region->bound.y2 += y;
   if (region->data && (num = region->data->num))
     {
        for (bx = PIXREGION_BOXPTR(region); num--; bx++)
          {
             bx->x1 += x;
             bx->y1 += y;
             bx->x2 += x;
             bx->y2 += y;
          }
     }
}

void
region_reset(Region *region, Box *box)
{
   region->bound = *box;
   FREE_DATA(region);
   region->data = NULL;
   region->last_del.w = 0;
   region->last_add.w = 0;
}

Eina_Bool
region_point_inside(Region *region, int x, int y, Box *box)
{
   Box *bx, *bxend;
   int num;

   num = PIXREGION_NUM_RECTS(region);
   if ((!num) || (!INBOX(&region->bound, x, y))) return EINA_FALSE;
   if (num == 1)
     {
        if (box) *box = region->bound;
        return EINA_TRUE;
     }
   for (bx = PIXREGION_BOXPTR(region), bxend = bx + num; bx != bxend; bx++)
     {
        if (y >= bx->y2) continue; // not there yet
        if ((y < bx->y1) || (x < bx->x1)) break; // missed it
        if (x >= bx->x2) continue; // not there yet
        if (box) *box = *bx;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

Eina_Bool
region_exists(Region *region)
{
   return !PIXREGION_NIL(region);
}

void
region_empty(Region *region)
{
   FREE_DATA(region);
   region->bound.x2 = region->bound.x1;
   region->bound.y2 = region->bound.y1;
   region->data = (Region_Data *)&_region_emptydata;
   region->last_del.w = 0;
   region->last_add.w = 0;
}

Box *
region_bounds(Region *region)
{
   return &region->bound;
}
