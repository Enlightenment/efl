/***************************************************************************/
/*                                                                         */
/*  ftgrays.c                                                              */
/*                                                                         */
/*    A new `perfect' anti-aliasing renderer (body).                       */
/*                                                                         */
/*  Copyright 2000-2017 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file can be compiled without the rest of the FreeType engine, by */
  /* defining the STANDALONE_ macro when compiling it.  You also need to   */
  /* put the files `ftgrays.h' and `ftimage.h' into the current            */
  /* compilation directory.  Typically, you could do something like        */
  /*                                                                       */
  /* - copy `src/smooth/ftgrays.c' (this file) to your current directory   */
  /*                                                                       */
  /* - copy `include/freetype/ftimage.h' and `src/smooth/ftgrays.h' to the */
  /*   same directory                                                      */
  /*                                                                       */
  /* - compile `ftgrays' with the STANDALONE_ macro defined, as in         */
  /*                                                                       */
  /*     cc -c -DSTANDALONE_ ftgrays.c                                     */
  /*                                                                       */
  /* The renderer can be initialized with a call to                        */
  /* `ft_gray_raster.raster_new'; an anti-aliased bitmap can be generated  */
  /* with a call to `ft_gray_raster.raster_render'.                        */
  /*                                                                       */
  /* See the comments and documentation in the file `ftimage.h' for more   */
  /* details on how the raster works.                                      */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This is a new anti-aliasing scan-converter for FreeType 2.  The       */
  /* algorithm used here is _very_ different from the one in the standard  */
  /* `ftraster' module.  Actually, `ftgrays' computes the _exact_          */
  /* coverage of the outline on each pixel cell.                           */
  /*                                                                       */
  /* It is based on ideas that I initially found in Raph Levien's          */
  /* excellent LibArt graphics library (see http://www.levien.com/libart   */
  /* for more information, though the web pages do not tell anything       */
  /* about the renderer; you'll have to dive into the source code to       */
  /* understand how it works).                                             */
  /*                                                                       */
  /* Note, however, that this is a _very_ different implementation         */
  /* compared to Raph's.  Coverage information is stored in a very         */
  /* different way, and I don't use sorted vector paths.  Also, it doesn't */
  /* use floating point values.                                            */
  /*                                                                       */
  /* This renderer has the following advantages:                           */
  /*                                                                       */
  /* - It doesn't need an intermediate bitmap.  Instead, one can supply a  */
  /*   callback function that will be called by the renderer to draw gray  */
  /*   spans on any target surface.  You can thus do direct composition on */
  /*   any kind of bitmap, provided that you give the renderer the right   */
  /*   callback.                                                           */
  /*                                                                       */
  /* - A perfect anti-aliaser, i.e., it computes the _exact_ coverage on   */
  /*   each pixel cell.                                                    */
  /*                                                                       */
  /* - It performs a single pass on the outline (the `standard' FT2        */
  /*   renderer makes two passes).                                         */
  /*                                                                       */
  /* - It can easily be modified to render to _any_ number of gray levels  */
  /*   cheaply.                                                            */
  /*                                                                       */
  /* - For small (< 20) pixel sizes, it is faster than the standard        */
  /*   renderer.                                                           */
  /*                                                                       */
  /*************************************************************************/

#include "sw_ft_raster.h"
#include "sw_ft_math.h"

  /* Auxiliary macros for token concatenation. */
#define SW_FT_ERR_XCAT( x, y )  x ## y
#define SW_FT_ERR_CAT( x, y )   SW_FT_ERR_XCAT( x, y )

#define SW_FT_BEGIN_STMNT  do {
#define SW_FT_END_STMNT    } while ( 0 )


#include <stddef.h>
#include <string.h>
#include <setjmp.h>
#include <limits.h>
#define SW_FT_UINT_MAX   UINT_MAX
#define SW_FT_INT_MAX    INT_MAX
#define SW_FT_ULONG_MAX  ULONG_MAX
#define SW_FT_CHAR_BIT   CHAR_BIT

#define ADD_LONG( a, b )                                    \
          (long)( (unsigned long)(a) + (unsigned long)(b) )
#define SUB_LONG( a, b )                                    \
          (long)( (unsigned long)(a) - (unsigned long)(b) )
#define MUL_LONG( a, b )                                    \
          (long)( (unsigned long)(a) * (unsigned long)(b) )
#define NEG_LONG( a )                                       \
          (long)( -(unsigned long)(a) )



/* empty function useful for setting a breakpoint to catch errors */

#define SW_FT_TRACE5( x )  do { } while ( 0 )     /* nothing */
#define SW_FT_TRACE7( x )  do { } while ( 0 )     /* nothing */
#define FT_ERROR( x )   do { } while ( 0 )     /* nothing */


#define ft_memset   memset

#define ft_setjmp   setjmp
#define ft_longjmp  longjmp
#define ft_jmp_buf  jmp_buf

typedef ptrdiff_t  SW_FT_PtrDist;


#define ErrRaster_Invalid_Mode      -2
#define ErrRaster_Invalid_Outline   -1
#define ErrRaster_Invalid_Argument  -3
#define ErrRaster_Memory_Overflow   -4

#define SW_FT_BEGIN_HEADER
#define SW_FT_END_HEADER


  /* This macro is used to indicate that a function parameter is unused. */
  /* Its purpose is simply to reduce compiler warnings.  Note also that  */
  /* simply defining it as `(void)x' doesn't avoid warnings with certain */
  /* ANSI compilers (e.g. LCC).                                          */
#define SW_FT_UNUSED( x )  (x) = (x)


#define SW_FT_THROW( e )   SW_FT_ERR_CAT( ErrRaster_, e )

  /* The size in bytes of the render pool used by the scan-line converter  */
  /* to do all of its work.                                                */
#define SW_FT_RENDER_POOL_SIZE  16384L

typedef int
(*SW_FT_Outline_MoveToFunc)( const SW_FT_Vector*  to,
                          void*             user );

#define SW_FT_Outline_MoveTo_Func  SW_FT_Outline_MoveToFunc

typedef int
(*SW_FT_Outline_LineToFunc)( const SW_FT_Vector*  to,
                          void*             user );

#define SW_FT_Outline_LineTo_Func  SW_FT_Outline_LineToFunc


typedef int
(*SW_FT_Outline_ConicToFunc)( const SW_FT_Vector*  control,
                           const SW_FT_Vector*  to,
                           void*             user );

#define SW_FT_Outline_ConicTo_Func  SW_FT_Outline_ConicToFunc

typedef int
(*SW_FT_Outline_CubicToFunc)( const SW_FT_Vector*  control1,
                           const SW_FT_Vector*  control2,
                           const SW_FT_Vector*  to,
                           void*             user );

#define SW_FT_Outline_CubicTo_Func  SW_FT_Outline_CubicToFunc

typedef struct  SW_FT_Outline_Funcs_
{
  SW_FT_Outline_MoveToFunc   move_to;
  SW_FT_Outline_LineToFunc   line_to;
  SW_FT_Outline_ConicToFunc  conic_to;
  SW_FT_Outline_CubicToFunc  cubic_to;

  int                        shift;
  SW_FT_Pos                  delta;

} SW_FT_Outline_Funcs;



#define SW_FT_DEFINE_OUTLINE_FUNCS( class_,               \
                                 move_to_, line_to_,   \
                                 conic_to_, cubic_to_, \
                                 shift_, delta_ )      \
          static const SW_FT_Outline_Funcs class_ =       \
          {                                            \
            move_to_,                                  \
            line_to_,                                  \
            conic_to_,                                 \
            cubic_to_,                                 \
            shift_,                                    \
            delta_                                     \
         };

#define SW_FT_DEFINE_RASTER_FUNCS( class_,                        \
                                raster_new_, raster_reset_,       \
                                raster_render_,                   \
                                raster_done_ )                    \
          const SW_FT_Raster_Funcs class_ =                       \
          {                                                       \
            raster_new_,                                          \
            raster_reset_,                                        \
            raster_render_,                                       \
            raster_done_                                          \
         };

#ifndef SW_FT_MEM_SET
#define SW_FT_MEM_SET( d, s, c )  ft_memset( d, s, c )
#endif

#ifndef SW_FT_MEM_ZERO
#define SW_FT_MEM_ZERO( dest, count )  SW_FT_MEM_SET( dest, 0, count )
#endif

#ifndef SW_FT_ZERO
#define SW_FT_ZERO( p )  SW_FT_MEM_ZERO( p, sizeof ( *(p) ) )
#endif

  /* as usual, for the speed hungry :-) */

#undef RAS_ARG
#undef RAS_ARG_
#undef RAS_VAR
#undef RAS_VAR_

#ifndef SW_FT_STATIC_RASTER

#define RAS_ARG   gray_PWorker  worker
#define RAS_ARG_  gray_PWorker  worker,

#define RAS_VAR   worker
#define RAS_VAR_  worker,

#else /* SW_FT_STATIC_RASTER */

#define RAS_ARG   void
#define RAS_ARG_  /* empty */
#define RAS_VAR   /* empty */
#define RAS_VAR_  /* empty */

#endif /* SW_FT_STATIC_RASTER */


  /* must be at least 6 bits! */
#define PIXEL_BITS  8

#undef FLOOR
#undef CEILING
#undef TRUNC
#undef SCALED

#define ONE_PIXEL       ( 1 << PIXEL_BITS )
#define TRUNC( x )      ( (TCoord)( (x) >> PIXEL_BITS ) )
#define SUBPIXELS( x )  ( (TPos)(x) * ONE_PIXEL )
#define FLOOR( x )      ( (x) & -ONE_PIXEL )
#define CEILING( x )    ( ( (x) + ONE_PIXEL - 1 ) & -ONE_PIXEL )
#define ROUND( x )      ( ( (x) + ONE_PIXEL / 2 ) & -ONE_PIXEL )

#if PIXEL_BITS >= 6
#define UPSCALE( x )    ( (x) * ( ONE_PIXEL >> 6 ) )
#define DOWNSCALE( x )  ( (x) >> ( PIXEL_BITS - 6 ) )
#else
#define UPSCALE( x )    ( (x) >> ( 6 - PIXEL_BITS ) )
#define DOWNSCALE( x )  ( (x) * ( 64 >> PIXEL_BITS ) )
#endif


  /* Compute `dividend / divisor' and return both its quotient and     */
  /* remainder, cast to a specific type.  This macro also ensures that */
  /* the remainder is always positive.  We use the remainder to keep   */
  /* track of accumulating errors and compensate for them.             */
#define SW_FT_DIV_MOD( type, dividend, divisor, quotient, remainder ) \
  SW_FT_BEGIN_STMNT                                                   \
    (quotient)  = (type)( (dividend) / (divisor) );                \
    (remainder) = (type)( (dividend) % (divisor) );                \
    if ( (remainder) < 0 )                                         \
    {                                                              \
      (quotient)--;                                                \
      (remainder) += (type)(divisor);                              \
    }                                                              \
  SW_FT_END_STMNT

#ifdef  __arm__
  /* Work around a bug specific to GCC which make the compiler fail to */
  /* optimize a division and modulo operation on the same parameters   */
  /* into a single call to `__aeabi_idivmod'.  See                     */
  /*                                                                   */
  /*  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=43721                */
#undef SW_FT_DIV_MOD
#define SW_FT_DIV_MOD( type, dividend, divisor, quotient, remainder ) \
  SW_FT_BEGIN_STMNT                                                   \
    (quotient)  = (type)( (dividend) / (divisor) );                \
    (remainder) = (type)( (dividend) - (quotient) * (divisor) );   \
    if ( (remainder) < 0 )                                         \
    {                                                              \
      (quotient)--;                                                \
      (remainder) += (type)(divisor);                              \
    }                                                              \
  SW_FT_END_STMNT
#endif /* __arm__ */


  /* These macros speed up repetitive divisions by replacing them */
  /* with multiplications and right shifts.                       */
#define SW_FT_UDIVPREP( c, b )                                        \
  long  b ## _r = c ? (long)( SW_FT_ULONG_MAX >> PIXEL_BITS ) / ( b ) \
                    : 0
#define SW_FT_UDIV( a, b )                                        \
  ( ( (unsigned long)( a ) * (unsigned long)( b ## _r ) ) >>   \
    ( sizeof( long ) * SW_FT_CHAR_BIT - PIXEL_BITS ) )


  /*************************************************************************/
  /*                                                                       */
  /*   TYPE DEFINITIONS                                                    */
  /*                                                                       */

  /* don't change the following types to SW_FT_Int or SW_FT_Pos, since we might */
  /* need to define them to "float" or "double" when experimenting with   */
  /* new algorithms                                                       */

  typedef long  TPos;     /* sub-pixel coordinate              */
  typedef int   TCoord;   /* integer scanline/pixel coordinate */
  typedef int   TArea;    /* cell areas, coordinate products   */


/* maximum number of gray spans in a call to the span callback */
#define SW_FT_MAX_GRAY_SPANS  256

  typedef struct TCell_*  PCell;

  typedef struct  TCell_
  {
    TCoord  x;     /* same with gray_TWorker.ex    */
    TCoord  cover; /* same with gray_TWorker.cover */
    TArea   area;
    PCell   next;

  } TCell;

  typedef struct TPixmap_
  {
    unsigned char*  origin;  /* pixmap origin at the bottom-left */
    int             pitch;   /* pitch to go down one row */

  } TPixmap;

  /* maximum number of gray cells in the buffer */
#if SW_FT_RENDER_POOL_SIZE > 2048
#define SW_FT_MAX_GRAY_POOL  ( SW_FT_RENDER_POOL_SIZE / sizeof ( TCell ) )
#else
#define SW_FT_MAX_GRAY_POOL  ( 2048 / sizeof ( TCell ) )
#endif


#if defined( _MSC_VER )      /* Visual C++ (and Intel C++) */
  /* We disable the warning `structure was padded due to   */
  /* __declspec(align())' in order to compile cleanly with */
  /* the maximum level of warnings.                        */
#pragma warning( push )
#pragma warning( disable : 4324 )
#endif /* _MSC_VER */

  typedef struct  gray_TWorker_
  {
    ft_jmp_buf  jump_buffer;

    TCoord  ex, ey;
    TCoord  min_ex, max_ex;
    TCoord  min_ey, max_ey;

    TArea   area;
    TCoord  cover;
    int     invalid;

    PCell*      ycells;
    PCell       cells;
    SW_FT_PtrDist  max_cells;
    SW_FT_PtrDist  num_cells;

    TPos    x,  y;

    SW_FT_Outline  outline;
    TPixmap     target;

    SW_FT_Span     gray_spans[SW_FT_MAX_GRAY_SPANS];
    int            num_gray_spans;

    SW_FT_Raster_Span_Func  render_span;
    void*                render_span_data;

  } gray_TWorker, *gray_PWorker;

#if defined( _MSC_VER )
#pragma warning( pop )
#endif


#ifndef SW_FT_STATIC_RASTER
#define ras  (*worker)
#else
  static gray_TWorker  ras;
#endif


  typedef struct gray_TRaster_
  {
    void*         memory;

  } gray_TRaster, *gray_PRaster;


  /*************************************************************************/
  /*                                                                       */
  /* Record the current cell in the table.                                 */
  /*                                                                       */
  static void
  gray_record_cell( RAS_ARG )
  {
    PCell  *pcell, cell;
    TCoord  x = ras.ex;


    pcell = &ras.ycells[ras.ey - ras.min_ey];
    for (;;)
    {
      cell = *pcell;
      if ( !cell || cell->x > x )
        break;

      if ( cell->x == x )
        goto Found;

      pcell = &cell->next;
    }

    if ( ras.num_cells >= ras.max_cells )
      ft_longjmp( ras.jump_buffer, 1 );

    /* insert new cell */
    cell        = ras.cells + ras.num_cells++;
    cell->x     = x;
    cell->area  = ras.area;
    cell->cover = ras.cover;

    cell->next  = *pcell;
    *pcell      = cell;

    return;

  Found:
    /* update old cell */
    cell->area  += ras.area;
    cell->cover += ras.cover;
  }


  /*************************************************************************/
  /*                                                                       */
  /* Set the current cell to a new position.                               */
  /*                                                                       */
  static void
  gray_set_cell( RAS_ARG_ TCoord  ex,
                          TCoord  ey )
  {
    /* Move the cell pointer to a new position.  We set the `invalid'      */
    /* flag to indicate that the cell isn't part of those we're interested */
    /* in during the render phase.  This means that:                       */
    /*                                                                     */
    /* . the new vertical position must be within min_ey..max_ey-1.        */
    /* . the new horizontal position must be strictly less than max_ex     */
    /*                                                                     */
    /* Note that if a cell is to the left of the clipping region, it is    */
    /* actually set to the (min_ex-1) horizontal position.                 */

    if ( ex < ras.min_ex )
      ex = ras.min_ex - 1;

    /* record the current one if it is valid and substantial */
    if ( !ras.invalid && ( ras.area || ras.cover ) )
      gray_record_cell( RAS_VAR );

    ras.area  = 0;
    ras.cover = 0;
    ras.ex    = ex;
    ras.ey    = ey;

    ras.invalid = ( ey >= ras.max_ey || ey < ras.min_ey ||
                    ex >= ras.max_ex );
  }


  /*************************************************************************/
  /*                                                                       */
  /* Render a scanline as one or more cells.                               */
  /*                                                                       */
  static void
  gray_render_scanline( RAS_ARG_ TCoord  ey,
                                 TPos    x1,
                                 TCoord  y1,
                                 TPos    x2,
                                 TCoord  y2 )
  {
    TCoord  ex1, ex2, fx1, fx2, first, dy, delta, mod;
    TPos    p, dx;
    int     incr;


    ex1 = TRUNC( x1 );
    ex2 = TRUNC( x2 );

    /* trivial case.  Happens often */
    if ( y1 == y2 )
    {
      gray_set_cell( RAS_VAR_ ex2, ey );
      return;
    }

    fx1   = (TCoord)( x1 - SUBPIXELS( ex1 ) );
    fx2   = (TCoord)( x2 - SUBPIXELS( ex2 ) );

    /* everything is located in a single cell.  That is easy! */
    /*                                                        */
    if ( ex1 == ex2 )
      goto End;

    /* ok, we'll have to render a run of adjacent cells on the same */
    /* scanline...                                                  */
    /*                                                              */
    dx = x2 - x1;
    dy = y2 - y1;

    if ( dx > 0 )
    {
      p     = ( ONE_PIXEL - fx1 ) * dy;
      first = ONE_PIXEL;
      incr  = 1;
    }
    else
    {
      p     = fx1 * dy;
      first = 0;
      incr  = -1;
      dx    = -dx;
    }

    SW_FT_DIV_MOD( TCoord, p, dx, delta, mod );

    ras.area  += (TArea)( ( fx1 + first ) * delta );
    ras.cover += delta;
    y1        += delta;
    ex1       += incr;
    gray_set_cell( RAS_VAR_ ex1, ey );

    if ( ex1 != ex2 )
    {
      TCoord  lift, rem;


      p = ONE_PIXEL * dy;
      SW_FT_DIV_MOD( TCoord, p, dx, lift, rem );

      do
      {
        delta = lift;
        mod  += rem;
        if ( mod >= (TCoord)dx )
        {
          mod -= (TCoord)dx;
          delta++;
        }

        ras.area  += (TArea)( ONE_PIXEL * delta );
        ras.cover += delta;
        y1        += delta;
        ex1       += incr;
        gray_set_cell( RAS_VAR_ ex1, ey );
      } while ( ex1 != ex2 );
    }

    fx1 = ONE_PIXEL - first;

  End:
    dy = y2 - y1;

    ras.area  += (TArea)( ( fx1 + fx2 ) * dy );
    ras.cover += dy;
  }


  /*************************************************************************/
  /*                                                                       */
  /* Render a given line as a series of scanlines.                         */
  /*                                                                       */
  static void
  gray_render_line( RAS_ARG_ TPos  to_x,
                             TPos  to_y )
  {
    TCoord  ey1, ey2, fy1, fy2, first, delta, mod;
    TPos    p, dx, dy, x, x2;
    int     incr;


    ey1 = TRUNC( ras.y );
    ey2 = TRUNC( to_y );     /* if (ey2 >= ras.max_ey) ey2 = ras.max_ey-1; */

    /* perform vertical clipping */
    if ( ( ey1 >= ras.max_ey && ey2 >= ras.max_ey ) ||
         ( ey1 <  ras.min_ey && ey2 <  ras.min_ey ) )
      goto End;

    fy1 = (TCoord)( ras.y - SUBPIXELS( ey1 ) );
    fy2 = (TCoord)( to_y - SUBPIXELS( ey2 ) );

    /* everything is on a single scanline */
    if ( ey1 == ey2 )
    {
      gray_render_scanline( RAS_VAR_ ey1, ras.x, fy1, to_x, fy2 );
      goto End;
    }

    dx = to_x - ras.x;
    dy = to_y - ras.y;

    /* vertical line - avoid calling gray_render_scanline */
    if ( dx == 0 )
    {
      TCoord  ex     = TRUNC( ras.x );
      TCoord  two_fx = (TCoord)( ( ras.x - SUBPIXELS( ex ) ) << 1 );
      TArea   area;


      if ( dy > 0)
      {
        first = ONE_PIXEL;
        incr  = 1;
      }
      else
      {
        first = 0;
        incr  = -1;
      }

      delta      = first - fy1;
      ras.area  += (TArea)two_fx * delta;
      ras.cover += delta;
      ey1       += incr;

      gray_set_cell( RAS_VAR_ ex, ey1 );

      delta = first + first - ONE_PIXEL;
      area  = (TArea)two_fx * delta;
      while ( ey1 != ey2 )
      {
        ras.area  += area;
        ras.cover += delta;
        ey1       += incr;

        gray_set_cell( RAS_VAR_ ex, ey1 );
      }

      delta      = fy2 - ONE_PIXEL + first;
      ras.area  += (TArea)two_fx * delta;
      ras.cover += delta;

      goto End;
    }

    /* ok, we have to render several scanlines */
    if ( dy > 0)
    {
      p     = ( ONE_PIXEL - fy1 ) * dx;
      first = ONE_PIXEL;
      incr  = 1;
    }
    else
    {
      p     = fy1 * dx;
      first = 0;
      incr  = -1;
      dy    = -dy;
    }

    SW_FT_DIV_MOD( TCoord, p, dy, delta, mod );

    x = ras.x + delta;
    gray_render_scanline( RAS_VAR_ ey1, ras.x, fy1, x, first );

    ey1 += incr;
    gray_set_cell( RAS_VAR_ TRUNC( x ), ey1 );

    if ( ey1 != ey2 )
    {
      TCoord  lift, rem;


      p    = ONE_PIXEL * dx;
      SW_FT_DIV_MOD( TCoord, p, dy, lift, rem );

      do
      {
        delta = lift;
        mod  += rem;
        if ( mod >= (TCoord)dy )
        {
          mod -= (TCoord)dy;
          delta++;
        }

        x2 = x + delta;
        gray_render_scanline( RAS_VAR_ ey1,
                                       x, ONE_PIXEL - first,
                                       x2, first );
        x = x2;

        ey1 += incr;
        gray_set_cell( RAS_VAR_ TRUNC( x ), ey1 );
      } while ( ey1 != ey2 );
    }

    gray_render_scanline( RAS_VAR_ ey1,
                                   x, ONE_PIXEL - first,
                                   to_x, fy2 );

  End:
    ras.x       = to_x;
    ras.y       = to_y;
  }

  static void
  gray_split_conic( SW_FT_Vector*  base )
  {
    TPos  a, b;


    base[4].x = base[2].x;
    b = base[1].x;
    a = base[3].x = ( base[2].x + b ) / 2;
    b = base[1].x = ( base[0].x + b ) / 2;
    base[2].x = ( a + b ) / 2;

    base[4].y = base[2].y;
    b = base[1].y;
    a = base[3].y = ( base[2].y + b ) / 2;
    b = base[1].y = ( base[0].y + b ) / 2;
    base[2].y = ( a + b ) / 2;
  }


  static void
  gray_render_conic( RAS_ARG_ const SW_FT_Vector*  control,
                              const SW_FT_Vector*  to )
  {
    SW_FT_Vector   bez_stack[16 * 2 + 1];  /* enough to accommodate bisections */
    SW_FT_Vector*  arc = bez_stack;
    TPos        dx, dy;
    int         draw, split;


    arc[0].x = UPSCALE( to->x );
    arc[0].y = UPSCALE( to->y );
    arc[1].x = UPSCALE( control->x );
    arc[1].y = UPSCALE( control->y );
    arc[2].x = ras.x;
    arc[2].y = ras.y;

    /* short-cut the arc that crosses the current band */
    if ( ( TRUNC( arc[0].y ) >= ras.max_ey &&
           TRUNC( arc[1].y ) >= ras.max_ey &&
           TRUNC( arc[2].y ) >= ras.max_ey ) ||
         ( TRUNC( arc[0].y ) <  ras.min_ey &&
           TRUNC( arc[1].y ) <  ras.min_ey &&
           TRUNC( arc[2].y ) <  ras.min_ey ) )
    {
      ras.x = arc[0].x;
      ras.y = arc[0].y;
      return;
    }

    dx = SW_FT_ABS( arc[2].x + arc[0].x - 2 * arc[1].x );
    dy = SW_FT_ABS( arc[2].y + arc[0].y - 2 * arc[1].y );
    if ( dx < dy )
      dx = dy;

    /* We can calculate the number of necessary bisections because  */
    /* each bisection predictably reduces deviation exactly 4-fold. */
    /* Even 32-bit deviation would vanish after 16 bisections.      */
    draw = 1;
    while ( dx > ONE_PIXEL / 4 )
    {
      dx   >>= 2;
      draw <<= 1;
    }

    /* We use decrement counter to count the total number of segments */
    /* to draw starting from 2^level. Before each draw we split as    */
    /* many times as there are trailing zeros in the counter.         */
    do
    {
      split = 1;
      while ( ( draw & split ) == 0 )
      {
        gray_split_conic( arc );
        arc += 2;
        split <<= 1;
      }

      gray_render_line( RAS_VAR_ arc[0].x, arc[0].y );
      arc -= 2;

    } while ( --draw );
  }


  static void
  gray_split_cubic( SW_FT_Vector*  base )
  {
    TPos  a, b, c, d;


    base[6].x = base[3].x;
    c = base[1].x;
    d = base[2].x;
    base[1].x = a = ( base[0].x + c ) / 2;
    base[5].x = b = ( base[3].x + d ) / 2;
    c = ( c + d ) / 2;
    base[2].x = a = ( a + c ) / 2;
    base[4].x = b = ( b + c ) / 2;
    base[3].x = ( a + b ) / 2;

    base[6].y = base[3].y;
    c = base[1].y;
    d = base[2].y;
    base[1].y = a = ( base[0].y + c ) / 2;
    base[5].y = b = ( base[3].y + d ) / 2;
    c = ( c + d ) / 2;
    base[2].y = a = ( a + c ) / 2;
    base[4].y = b = ( b + c ) / 2;
    base[3].y = ( a + b ) / 2;
  }


  static void
  gray_render_cubic( RAS_ARG_ const SW_FT_Vector*  control1,
                              const SW_FT_Vector*  control2,
                              const SW_FT_Vector*  to )
  {
    SW_FT_Vector   bez_stack[16 * 3 + 1];  /* enough to accommodate bisections */
    SW_FT_Vector*  arc = bez_stack;
    TPos        dx, dy, dx_, dy_;
    TPos        dx1, dy1, dx2, dy2;
    TPos        L, s, s_limit;


    arc[0].x = UPSCALE( to->x );
    arc[0].y = UPSCALE( to->y );
    arc[1].x = UPSCALE( control2->x );
    arc[1].y = UPSCALE( control2->y );
    arc[2].x = UPSCALE( control1->x );
    arc[2].y = UPSCALE( control1->y );
    arc[3].x = ras.x;
    arc[3].y = ras.y;

    /* short-cut the arc that crosses the current band */
    if ( ( TRUNC( arc[0].y ) >= ras.max_ey &&
           TRUNC( arc[1].y ) >= ras.max_ey &&
           TRUNC( arc[2].y ) >= ras.max_ey &&
           TRUNC( arc[3].y ) >= ras.max_ey ) ||
         ( TRUNC( arc[0].y ) <  ras.min_ey &&
           TRUNC( arc[1].y ) <  ras.min_ey &&
           TRUNC( arc[2].y ) <  ras.min_ey &&
           TRUNC( arc[3].y ) <  ras.min_ey ) )
    {
      ras.x = arc[0].x;
      ras.y = arc[0].y;
      return;
    }

    for (;;)
    {
      /* Decide whether to split or draw. See `Rapid Termination          */
      /* Evaluation for Recursive Subdivision of Bezier Curves' by Thomas */
      /* F. Hain, at                                                      */
      /* http://www.cis.southalabama.edu/~hain/general/Publications/Bezier/Camera-ready%20CISST02%202.pdf */

      /* dx and dy are x and y components of the P0-P3 chord vector. */
      dx = dx_ = arc[3].x - arc[0].x;
      dy = dy_ = arc[3].y - arc[0].y;

      L = SW_FT_HYPOT( dx_, dy_ );

      /* Avoid possible arithmetic overflow below by splitting. */
      if ( L > 32767 )
        goto Split;

      /* Max deviation may be as much as (s/L) * 3/4 (if Hain's v = 1). */
      s_limit = L * (TPos)( ONE_PIXEL / 6 );

      /* s is L * the perpendicular distance from P1 to the line P0-P3. */
      dx1 = arc[1].x - arc[0].x;
      dy1 = arc[1].y - arc[0].y;
      s = SW_FT_ABS( SUB_LONG( MUL_LONG( dy, dx1 ), MUL_LONG( dx, dy1 ) ) );

      if ( s > s_limit )
        goto Split;

      /* s is L * the perpendicular distance from P2 to the line P0-P3. */
      dx2 = arc[2].x - arc[0].x;
      dy2 = arc[2].y - arc[0].y;
      s = SW_FT_ABS( SUB_LONG( MUL_LONG( dy, dx2 ), MUL_LONG( dx, dy2 ) ) );

      if ( s > s_limit )
        goto Split;

      /* Split super curvy segments where the off points are so far
         from the chord that the angles P0-P1-P3 or P0-P2-P3 become
         acute as detected by appropriate dot products. */
      if ( dx1 * ( dx1 - dx ) + dy1 * ( dy1 - dy ) > 0 ||
           dx2 * ( dx2 - dx ) + dy2 * ( dy2 - dy ) > 0 )
        goto Split;

      gray_render_line( RAS_VAR_ arc[0].x, arc[0].y );

      if ( arc == bez_stack )
        return;

      arc -= 3;
      continue;

    Split:
      gray_split_cubic( arc );
      arc += 3;
    }
  }


  static int
  gray_move_to( const SW_FT_Vector*  to,
                gray_PWorker      worker )
  {
    TPos  x, y;


    /* start to a new position */
    x = UPSCALE( to->x );
    y = UPSCALE( to->y );

    gray_set_cell( RAS_VAR_ TRUNC( x ), TRUNC( y ) );

    ras.x = x;
    ras.y = y;
    return 0;
  }


  static int
  gray_line_to( const SW_FT_Vector*  to,
                gray_PWorker      worker )
  {
    gray_render_line( RAS_VAR_ UPSCALE( to->x ), UPSCALE( to->y ) );
    return 0;
  }


  static int
  gray_conic_to( const SW_FT_Vector*  control,
                 const SW_FT_Vector*  to,
                 gray_PWorker      worker )
  {
    gray_render_conic( RAS_VAR_ control, to );
    return 0;
  }


  static int
  gray_cubic_to( const SW_FT_Vector*  control1,
                 const SW_FT_Vector*  control2,
                 const SW_FT_Vector*  to,
                 gray_PWorker      worker )
  {
    gray_render_cubic( RAS_VAR_ control1, control2, to );
    return 0;
  }


  static void
  gray_hline( RAS_ARG_ TCoord  x,
                       TCoord  y,
                       TArea   coverage,
                       TCoord  acount )
  {
    /* scale the coverage from 0..(ONE_PIXEL*ONE_PIXEL*2) to 0..256  */
    coverage >>= PIXEL_BITS * 2 + 1 - 8;
    if ( coverage < 0 )
      coverage = -coverage - 1;

    /* compute the line's coverage depending on the outline fill rule */
    if ( ras.outline.flags & SW_FT_OUTLINE_EVEN_ODD_FILL )
    {
      coverage &= 511;

      if ( coverage >= 256 )
        coverage = 511 - coverage;
    }
    else
    {
      /* normal non-zero winding rule */
      if ( coverage >= 256 )
        coverage = 255;
    }

    if ( coverage )
    {
      SW_FT_Span*  span;
      int       count;


      /* see whether we can add this span to the current list */
      count = ras.num_gray_spans;
      span  = ras.gray_spans + count - 1;
      if ( count > 0                          &&
           span->y == y                       &&
           (int)span->x + span->len == (int)x &&
           span->coverage == coverage         )
      {
        span->len = (unsigned short)( span->len + acount );
        return;
      }

      if ( count >= SW_FT_MAX_GRAY_SPANS )
      {
        if ( ras.render_span && count > 0 )
          ras.render_span(count, ras.gray_spans,
                           ras.render_span_data );

        ras.num_gray_spans = 0;

        span  = ras.gray_spans;
      }
      else
        span++;

      /* add a gray span to the current list */
      span->x        = (short)x;
      span->y        = (short)y;
      span->len      = (unsigned short)acount;
      span->coverage = (unsigned char)coverage;

      ras.num_gray_spans++;
    }
  }


  static void
  gray_sweep( RAS_ARG )
  {
    int  y;

    ras.num_gray_spans = 0;

    for ( y = ras.min_ey; y < ras.max_ey; y++ )
    {
      PCell   cell  = ras.ycells[y - ras.min_ey];
      TCoord  x     = ras.min_ex;
      TArea   cover = 0;
      TArea   area;


      for ( ; cell != NULL; cell = cell->next )
      {
        if ( cover != 0 && cell->x > x )
          gray_hline( RAS_VAR_ x, y, cover, cell->x - x );

        cover += (TArea)cell->cover * ( ONE_PIXEL * 2 );
        area   = cover - cell->area;

        if ( area != 0 && cell->x >= ras.min_ex )
          gray_hline( RAS_VAR_ cell->x, y, area, 1 );

        x = cell->x + 1;
      }

      if ( cover != 0 )
        gray_hline( RAS_VAR_ x, y, cover, ras.max_ex - x );
    }

    if ( ras.render_span && ras.num_gray_spans > 0 )
      ras.render_span(ras.num_gray_spans,
                       ras.gray_spans, ras.render_span_data );
  }


//#ifdef STANDALONE_

  /*************************************************************************/
  /*                                                                       */
  /*  The following functions should only compile in stand-alone mode,     */
  /*  i.e., when building this component without the rest of FreeType.     */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    SW_FT_Outline_Decompose                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Walk over an outline's structure to decompose it into individual   */
  /*    segments and Bézier arcs.  This function is also able to emit      */
  /*    `move to' and `close to' operations to indicate the start and end  */
  /*    of new contours in the outline.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline        :: A pointer to the source target.                  */
  /*                                                                       */
  /*    func_interface :: A table of `emitters', i.e., function pointers   */
  /*                      called during decomposition to indicate path     */
  /*                      operations.                                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    user           :: A typeless pointer which is passed to each       */
  /*                      emitter during the decomposition.  It can be     */
  /*                      used to store the state during the               */
  /*                      decomposition.                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  static int
  SW_FT_Outline_Decompose( const SW_FT_Outline*        outline,
                        const SW_FT_Outline_Funcs*  func_interface,
                        void*                    user )
  {
#undef SCALED
#define SCALED( x )  ( ( (x) << shift ) - delta )

    SW_FT_Vector   v_last;
    SW_FT_Vector   v_control;
    SW_FT_Vector   v_start;

    SW_FT_Vector*  point;
    SW_FT_Vector*  limit;
    char*       tags;

    int         error;

    int   n;         /* index of contour in outline     */
    int   first;     /* index of first point in contour */
    char  tag;       /* current point's state           */

    int   shift;
    TPos  delta;


    if ( !outline )
      return SW_FT_THROW( Invalid_Outline );

    if ( !func_interface )
      return SW_FT_THROW( Invalid_Argument );

    shift = func_interface->shift;
    delta = func_interface->delta;
    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      int  last;  /* index of last point in contour */


      SW_FT_TRACE5(( "SW_FT_Outline_Decompose: Outline %d\n", n ));

      last  = outline->contours[n];
      if ( last < 0 )
        goto Invalid_Outline;
      limit = outline->points + last;

      v_start   = outline->points[first];
      v_start.x = SCALED( v_start.x );
      v_start.y = SCALED( v_start.y );

      v_last   = outline->points[last];
      v_last.x = SCALED( v_last.x );
      v_last.y = SCALED( v_last.y );

      v_control = v_start;

      point = outline->points + first;
      tags  = outline->tags   + first;
      tag   = SW_FT_CURVE_TAG( tags[0] );

      /* A contour cannot start with a cubic control point! */
      if ( tag == SW_FT_CURVE_TAG_CUBIC )
        goto Invalid_Outline;

      /* check first point to determine origin */
      if ( tag == SW_FT_CURVE_TAG_CONIC )
      {
        /* first point is conic control.  Yes, this happens. */
        if ( SW_FT_CURVE_TAG( outline->tags[last] ) == SW_FT_CURVE_TAG_ON )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
          limit--;
        }
        else
        {
          /* if both first and last points are conic,         */
          /* start at their middle and record its position    */
          /* for closure                                      */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;

          v_last = v_start;
        }
        point--;
        tags--;
      }

      SW_FT_TRACE5(( "  move to (%.2f, %.2f)\n",
                  v_start.x / 64.0, v_start.y / 64.0 ));
      error = func_interface->move_to( &v_start, user );
      if ( error )
        goto Exit;

      while ( point < limit )
      {
        point++;
        tags++;

        tag = SW_FT_CURVE_TAG( tags[0] );
        switch ( tag )
        {
        case SW_FT_CURVE_TAG_ON:  /* emit a single line_to */
          {
            SW_FT_Vector  vec;


            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            SW_FT_TRACE5(( "  line to (%.2f, %.2f)\n",
                        vec.x / 64.0, vec.y / 64.0 ));
            error = func_interface->line_to( &vec, user );
            if ( error )
              goto Exit;
            continue;
          }

        case SW_FT_CURVE_TAG_CONIC:  /* consume conic arcs */
          v_control.x = SCALED( point->x );
          v_control.y = SCALED( point->y );

        Do_Conic:
          if ( point < limit )
          {
            SW_FT_Vector  vec;
            SW_FT_Vector  v_middle;


            point++;
            tags++;
            tag = SW_FT_CURVE_TAG( tags[0] );

            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            if ( tag == SW_FT_CURVE_TAG_ON )
            {
              SW_FT_TRACE5(( "  conic to (%.2f, %.2f)"
                          " with control (%.2f, %.2f)\n",
                          vec.x / 64.0, vec.y / 64.0,
                          v_control.x / 64.0, v_control.y / 64.0 ));
              error = func_interface->conic_to( &v_control, &vec, user );
              if ( error )
                goto Exit;
              continue;
            }

            if ( tag != SW_FT_CURVE_TAG_CONIC )
              goto Invalid_Outline;

            v_middle.x = ( v_control.x + vec.x ) / 2;
            v_middle.y = ( v_control.y + vec.y ) / 2;

            SW_FT_TRACE5(( "  conic to (%.2f, %.2f)"
                        " with control (%.2f, %.2f)\n",
                        v_middle.x / 64.0, v_middle.y / 64.0,
                        v_control.x / 64.0, v_control.y / 64.0 ));
            error = func_interface->conic_to( &v_control, &v_middle, user );
            if ( error )
              goto Exit;

            v_control = vec;
            goto Do_Conic;
          }

          SW_FT_TRACE5(( "  conic to (%.2f, %.2f)"
                      " with control (%.2f, %.2f)\n",
                      v_start.x / 64.0, v_start.y / 64.0,
                      v_control.x / 64.0, v_control.y / 64.0 ));
          error = func_interface->conic_to( &v_control, &v_start, user );
          goto Close;

        default:  /* SW_FT_CURVE_TAG_CUBIC */
          {
            SW_FT_Vector  vec1, vec2;


            if ( point + 1 > limit                             ||
                 SW_FT_CURVE_TAG( tags[1] ) != SW_FT_CURVE_TAG_CUBIC )
              goto Invalid_Outline;

            point += 2;
            tags  += 2;

            vec1.x = SCALED( point[-2].x );
            vec1.y = SCALED( point[-2].y );

            vec2.x = SCALED( point[-1].x );
            vec2.y = SCALED( point[-1].y );

            if ( point <= limit )
            {
              SW_FT_Vector  vec;


              vec.x = SCALED( point->x );
              vec.y = SCALED( point->y );

              SW_FT_TRACE5(( "  cubic to (%.2f, %.2f)"
                          " with controls (%.2f, %.2f) and (%.2f, %.2f)\n",
                          vec.x / 64.0, vec.y / 64.0,
                          vec1.x / 64.0, vec1.y / 64.0,
                          vec2.x / 64.0, vec2.y / 64.0 ));
              error = func_interface->cubic_to( &vec1, &vec2, &vec, user );
              if ( error )
                goto Exit;
              continue;
            }

            SW_FT_TRACE5(( "  cubic to (%.2f, %.2f)"
                        " with controls (%.2f, %.2f) and (%.2f, %.2f)\n",
                        v_start.x / 64.0, v_start.y / 64.0,
                        vec1.x / 64.0, vec1.y / 64.0,
                        vec2.x / 64.0, vec2.y / 64.0 ));
            error = func_interface->cubic_to( &vec1, &vec2, &v_start, user );
            goto Close;
          }
        }
      }

      /* close the contour with a line segment */
      SW_FT_TRACE5(( "  line to (%.2f, %.2f)\n",
                  v_start.x / 64.0, v_start.y / 64.0 ));
      error = func_interface->line_to( &v_start, user );

   Close:
      if ( error )
        goto Exit;

      first = last + 1;
    }

    SW_FT_TRACE5(( "SW_FT_Outline_Decompose: Done\n", n ));
    return 0;

  Exit:
    SW_FT_TRACE5(( "SW_FT_Outline_Decompose: Error 0x%x\n", error ));
    return error;

  Invalid_Outline:
    return SW_FT_THROW( Invalid_Outline );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    SW_FT_Outline_Get_CBox                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return an outline's `control box'.  The control box encloses all   */
  /*    the outline's points, including Bézier control points.  Though it  */
  /*    coincides with the exact bounding box for most glyphs, it can be   */
  /*    slightly larger in some situations (like when rotating an outline  */
  /*    that contains Bézier outside arcs).                                */
  /*                                                                       */
  /*    Computing the control box is very fast, while getting the bounding */
  /*    box can take much more time as it needs to walk over all segments  */
  /*    and arcs in the outline.  To get the latter, you can use the       */
  /*    `ftbbox' component, which is dedicated to this single task.        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    acbox   :: The outline's control box.                              */
  /*                                                                       */
  /* <Note>                                                                */
  /*    See @SW_FT_Glyph_Get_CBox for a discussion of tricky fonts.           */
  /*                                                                       */
#if 0
  static void
  SW_FT_Outline_Get_CBox( const SW_FT_Outline*  outline,
                       SW_FT_BBox           *acbox )
  {
    TPos  xMin, yMin, xMax, yMax;


    if ( outline && acbox )
    {
      if ( outline->n_points == 0 )
      {
        xMin = 0;
        yMin = 0;
        xMax = 0;
        yMax = 0;
      }
      else
      {
        SW_FT_Vector*  vec   = outline->points;
        SW_FT_Vector*  limit = vec + outline->n_points;


        xMin = xMax = vec->x;
        yMin = yMax = vec->y;
        vec++;

        for ( ; vec < limit; vec++ )
        {
          TPos  x, y;


          x = vec->x;
          if ( x < xMin ) xMin = x;
          if ( x > xMax ) xMax = x;

          y = vec->y;
          if ( y < yMin ) yMin = y;
          if ( y > yMax ) yMax = y;
        }
      }
      acbox->xMin = xMin;
      acbox->xMax = xMax;
      acbox->yMin = yMin;
      acbox->yMax = yMax;
    }
  }
#endif
//#endif /* STANDALONE_ */


  SW_FT_DEFINE_OUTLINE_FUNCS(
    func_interface,

    (SW_FT_Outline_MoveTo_Func) gray_move_to,   /* move_to  */
    (SW_FT_Outline_LineTo_Func) gray_line_to,   /* line_to  */
    (SW_FT_Outline_ConicTo_Func)gray_conic_to,  /* conic_to */
    (SW_FT_Outline_CubicTo_Func)gray_cubic_to,  /* cubic_to */

    0,                                       /* shift    */
    0                                        /* delta    */
  )


  static int
  gray_convert_glyph_inner( RAS_ARG )
  {

    volatile int  error = 0;

#ifdef SW_FT_CONFIG_OPTION_PIC
      SW_FT_Outline_Funcs func_interface;
      Init_Class_func_interface(&func_interface);
#endif

    if ( ft_setjmp( ras.jump_buffer ) == 0 )
    {
      error = SW_FT_Outline_Decompose( &ras.outline, &func_interface, &ras );
      if ( !ras.invalid )
        gray_record_cell( RAS_VAR );

      SW_FT_TRACE7(( "band [%d..%d]: %d cells\n",
                  ras.min_ey, ras.max_ey, ras.num_cells ));
    }
    else
    {
      error = SW_FT_THROW( Memory_Overflow );

      SW_FT_TRACE7(( "band [%d..%d]: to be bisected\n",
                  ras.min_ey, ras.max_ey ));
    }

    return error;
  }


  static int
  gray_convert_glyph( RAS_ARG )
  {
    const TCoord  yMin = ras.min_ey;
    const TCoord  yMax = ras.max_ey;
    const TCoord  xMin = ras.min_ex;
    const TCoord  xMax = ras.max_ex;

    TCell    buffer[SW_FT_MAX_GRAY_POOL];
    size_t   height = (size_t)( yMax - yMin );
    size_t   n = SW_FT_MAX_GRAY_POOL / 8;
    TCoord   y;
    TCoord   bands[32];  /* enough to accommodate bisections */
    TCoord*  band;


    /* set up vertical bands */
    if ( height > n )
    {
      /* two divisions rounded up */
      n       = ( height + n - 1 ) / n;
      height  = ( height + n - 1 ) / n;
    }

    /* memory management */
    n = ( height * sizeof ( PCell ) + sizeof ( TCell ) - 1 ) / sizeof ( TCell );

    ras.cells     = buffer + n;
    ras.max_cells = (SW_FT_PtrDist)( SW_FT_MAX_GRAY_POOL - n );
    ras.ycells    = (PCell*)buffer;

    for ( y = yMin; y < yMax; )
    {
      ras.min_ey = y;
      y         += height;
      ras.max_ey = SW_FT_MIN( y, yMax );

      band    = bands;
      band[1] = xMin;
      band[0] = xMax;

      do
      {
        TCoord  width = band[0] - band[1];
        int     error;


        SW_FT_MEM_ZERO( ras.ycells, height * sizeof ( PCell ) );

        ras.num_cells = 0;
        ras.invalid   = 1;
        ras.min_ex    = band[1];
        ras.max_ex    = band[0];

        error = gray_convert_glyph_inner( RAS_VAR );

        if ( !error )
        {
          gray_sweep( RAS_VAR );
          band--;
          continue;
        }
        else if ( error != ErrRaster_Memory_Overflow )
          return 1;

        /* render pool overflow; we will reduce the render band by half */
        width >>= 1;

        /* this should never happen even with tiny rendering pool */
        if ( width == 0 )
        {
          SW_FT_TRACE7(( "gray_convert_glyph: rotten glyph\n" ));
          return 1;
        }

        band++;
        band[1]  = band[0];
        band[0] += width;
      } while ( band >= bands );
    }

    return 0;
  }


  static int
  gray_raster_render( SW_FT_Raster                raster,
                      const SW_FT_Raster_Params*  params )
  {
    const SW_FT_Outline*  outline    = (const SW_FT_Outline*)params->source;
    SW_FT_BBox            clip;

#ifndef SW_FT_STATIC_RASTER
    gray_TWorker  worker[1];
#endif


    if ( !raster )
      return SW_FT_THROW( Invalid_Argument );

    /* this version does not support monochrome rendering */
    if ( !( params->flags & SW_FT_RASTER_FLAG_AA ) )
      return SW_FT_THROW( Invalid_Mode );

    if ( !outline )
      return SW_FT_THROW( Invalid_Outline );

    /* return immediately if the outline is empty */
    if ( outline->n_points == 0 || outline->n_contours <= 0 )
      return 0;

    if ( !outline->contours || !outline->points )
      return SW_FT_THROW( Invalid_Outline );

    if ( outline->n_points !=
           outline->contours[outline->n_contours - 1] + 1 )
      return SW_FT_THROW( Invalid_Outline );

    ras.outline = *outline;

    if ( params->flags & SW_FT_RASTER_FLAG_DIRECT )
    {
      if ( !params->gray_spans )
        return 0;

      ras.render_span      = (SW_FT_Raster_Span_Func)params->gray_spans;
      ras.render_span_data = params->user;
      ras.num_gray_spans = 0;
    }
    else
    {
      /* if direct mode is not set, we must have a target bitmap */
      return SW_FT_THROW( Invalid_Argument );
    }

    if ( params->flags & SW_FT_RASTER_FLAG_CLIP )
      clip = params->clip_box;
    else
    {
      clip.xMin = -32768L;
      clip.yMin = -32768L;
      clip.xMax =  32767L;
      clip.yMax =  32767L;
    }

    /* clip to target bitmap, exit if nothing to do */
    ras.min_ex = clip.xMin;
    ras.min_ey = clip.yMin;
    ras.max_ex = clip.xMax;
    ras.max_ey = clip.yMax;

    if ( ras.max_ex <= ras.min_ex || ras.max_ey <= ras.min_ey )
      return 0;

    return gray_convert_glyph( RAS_VAR );
  }


  /**** RASTER OBJECT CREATION: In stand-alone mode, we simply use *****/
  /****                         a static object.                   *****/


  static int
  gray_raster_new( SW_FT_Raster*  araster )
  {
    static gray_TRaster  the_raster;

    *araster = (SW_FT_Raster)&the_raster;
    SW_FT_MEM_ZERO( &the_raster, sizeof ( the_raster ) );
    //SW_FT_ZERO( &the_raster );

    return 0;
  }


  static void
  gray_raster_done( SW_FT_Raster  raster )
  {
    /* nothing */
    SW_FT_UNUSED( raster );
  }

  static void
  gray_raster_reset( SW_FT_Raster       raster,
                     unsigned char*  pool_base,
                     unsigned long   pool_size )
  {
    SW_FT_UNUSED( raster );
    SW_FT_UNUSED( pool_base );
    SW_FT_UNUSED( pool_size );
  }


  SW_FT_DEFINE_RASTER_FUNCS(sw_ft_grays_raster,

    (SW_FT_Raster_New_Func)     gray_raster_new,
    (SW_FT_Raster_Reset_Func)   gray_raster_reset,
    (SW_FT_Raster_Render_Func)  gray_raster_render,
    (SW_FT_Raster_Done_Func)    gray_raster_done
  )
