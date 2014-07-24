#ifndef _TYPES_OUTPUT_C_
#define _TYPES_OUTPUT_C_

#ifndef _STRUCT_EO_TYPES
#define _STRUCT_EO_TYPES

typedef struct _Foo {
  int field;
  float another;
} Foo;

typedef struct {
  Foo a;
  struct _Foo b;
} Bar;

struct Named {
  const char *something;
  int field;
};

struct Another {
  struct Named field;
};

struct _Foo {
  int field;
  float another;
};


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
