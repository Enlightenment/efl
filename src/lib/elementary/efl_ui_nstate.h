/**
 * @defgroup Elm_Nstate Nstate
 * @ingroup Elementary
 *
 * @image html nstate_inheritance_tree.png
 * @image latex nstate_inheritance_tree.eps
 *
 * @image html img/widget/nstate/preview-00.png
 * @image latex img/widget/nstate/preview-00.eps
 *
 * A Nstate is a widget which displays one of the state among states defined by user.
 *
 * This widget inherits from the @ref Button, so that all the functions acting on @ref Button also work for nstate objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Button:
 * - @c "state,changed" - whenever state of nstate is changed
 *
 * Default content parts of the nstate widget that you can use are the
 * the same that you use with the @ref Button
 * @{
 */

#include "efl_ui_nstate.eo.h"
