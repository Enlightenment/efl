#ifndef ECTOR_CAIRO_PRIVATE_H_
# define ECTOR_CAIRO_PRIVATE_H_

typedef void cairo_pattern_t;

typedef struct _Ector_Cairo_Surface_Data Ector_Cairo_Surface_Data;
struct _Ector_Cairo_Surface_Data
{
   cairo_t *cairo;
};

#define USE(Obj, Sym, Error)                     \
  if (!Sym) _ector_cairo_symbol_get(Obj, #Sym);  \
  if (!Sym) return Error;

#define CHECK_CAIRO(Parent) (!(Parent && Parent->cairo))

static inline void *
_ector_cairo_symbol_get(Eo *obj, const char *name)
{
   Eo *parent;
   void *sym;

   eo_do(obj, parent = eo_parent_get());
   if (!parent) return NULL;

   eo_do(parent, sym = ector_cairo_surface_symbol_get(name));
   return sym;
}

#endif
