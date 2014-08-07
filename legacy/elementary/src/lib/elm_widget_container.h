#ifndef ELM_WIDGET_CONTAINER_H
#define ELM_WIDGET_CONTAINER_H

#include <elm_widget.h>

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-container-class The Elementary Container Class
 *
 * @image html container_inheritance_tree.png
 * @image latex container_inheritance_tree.eps
 *
 * This class defines a common interface for objects acting like
 * containers, i.e. objects parenting others and displaying their
 * childs "inside" of them somehow.
 *
 * The container must define "parts" (or spots) into which child
 * objects will be placed, inside of it. This is a way of handling
 * more the one content object, by naming content locations
 * properly. This is the role of the @c name argument of the virtual
 * functions in the class.
 *
 * The following object functions are meant to be used with all
 * container objects and derived ones:
 *
 * - elm_object_part_content_set()
 * - elm_object_part_content_get()
 * - elm_object_part_content_unset()
 */

#include "elm_container.eo.h"

#endif
