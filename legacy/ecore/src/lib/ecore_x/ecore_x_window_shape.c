#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * @defgroup Ecore_X_Window_Shape X Window Shape Functions
 *
 * These functions use the shape extension of the X server to change
 * shape of given windows.
 */

/**
 * Sets the shape of the given window to that given by the pixmap @p mask.
 * @param   win  The given window.
 * @param   mask A 2-bit depth pixmap that provides the new shape of the
 *               window.
 * @ingroup Ecore_X_Window_Shape
 */
void
ecore_x_window_shape_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask)
{
   XShapeCombineMask(_ecore_x_disp, win, ShapeBounding, 0, 0, mask, ShapeSet);
}
