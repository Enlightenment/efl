/**
 * Defines stubs for unimplemented features/functions/structs in UNPOSIX namespace.
 */
#ifndef UNPOSIX_UNIMPLEMENTED_H
#define UNPOSIX_UNIMPLEMENTED_H

#define UNIMPLEMENTED // For future checks. Use when some type is just a stub to implement later.

UNIMPLEMENTED struct unimplemented {
    int attr;
};

UNIMPLEMENTED typedef struct unimplemented unimplemented_struct_t;

#define UNIMPLEMENTED_STRUCT_T(type) typedef unimplemented_struct_t  type ## _t;
#define UNIMPLEMENTED_STRUCT(type) struct type { int x; };

#endif
