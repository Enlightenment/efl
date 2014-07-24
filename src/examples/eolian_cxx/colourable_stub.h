#ifndef EFL_EXAMPLE_EOLIAN_CXX_COLOURABLE_STUB_H
#define EFL_EXAMPLE_EOLIAN_CXX_COLOURABLE_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

struct _Colourable_Data
{
   int r;
   int g;
   int b;
};
typedef struct _Colourable_Data  Colourable_Data;

void _colourable_constructor(Eo *obj, Colourable_Data *self);
void _colourable_rgb_composite_constructor(Eo *obj, Colourable_Data *self, int r, int g, int b);
void _colourable_rgb_24bits_constructor(Eo *obj, Colourable_Data *self, int rgb);
void _colourable_print_colour(Eo *obj, Colourable_Data *self);
void _colourable_print_colour(Eo *obj, Colourable_Data *self);
int _colourable_colour_mask(Eo *obj, Colourable_Data *self, int mask);
void _colourable_composite_colour_get(Eo *obj, Colourable_Data *self, int* r, int* g, int* b);
void _colourable_composite_colour_set(Eo *obj, Colourable_Data *self, int r, int g, int b);
int _colourable_colour_get(Eo *obj, Colourable_Data *self);
void _colourable_colour_set(Eo *obj, Colourable_Data *self, int rgb);

#ifdef __cplusplus
}
#endif

#endif // EFL_EXAMPLE_EOLIAN_CXX_COLOURABLE_STUB_H
