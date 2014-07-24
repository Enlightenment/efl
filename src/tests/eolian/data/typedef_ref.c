#ifndef _TYPES_OUTPUT_C_
#define _TYPES_OUTPUT_C_

#ifndef _TYPEDEF_EO_TYPES
#define _TYPEDEF_EO_TYPES

typedef int Evas_Coord;

typedef Eina_List *List_Objects;


#endif
#define DUMMY_CLASS dummy_class_get()

const Eo_Class *dummy_class_get(void) EINA_CONST;

/**
 *
 * No description supplied.
 *
 * @param[in] idx No description supplied.
 *
 */
EOAPI char * dummy_foo(int idx);


#endif
