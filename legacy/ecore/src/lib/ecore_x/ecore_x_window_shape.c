#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_shape_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask)
{
   XShapeCombineMask(_ecore_x_disp, win, ShapeBounding, 0, 0, mask, ShapeSet);
}
