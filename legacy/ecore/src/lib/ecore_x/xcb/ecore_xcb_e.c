/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * OLD E hints
 */

#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"


EAPI void
ecore_x_e_frame_size_set(Ecore_X_Window window,
                         int            fl,
                         int            fr,
                         int            ft,
                         int            fb)
{
   uint32_t frames[4];

   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_E_FRAME_SIZE, ECORE_X_ATOM_CARDINAL, 32,
                       4, (const void *)frames);
}
