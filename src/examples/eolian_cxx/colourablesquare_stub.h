#ifndef EFL_EXAMPLE_EOLIAN_CXX_COLOURABLE_SQUARE_STUB_H
#define EFL_EXAMPLE_EOLIAN_CXX_COLOURABLE_SQUARE_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

struct _ColourableSquare_Data
{
   int size;
};
typedef struct _ColourableSquare_Data  ColourableSquare_Data;

void _colourablesquare_size_constructor(Eo *obj, ColourableSquare_Data *self, int size);
int _colourablesquare_size_get(Eo *obj, ColourableSquare_Data *self);
void _colourablesquare_size_print(Eo *obj, ColourableSquare_Data *self);
void _colourablesquare_size_set(Eo *obj, ColourableSquare_Data *self, int size);

#ifdef __cplusplus
}
#endif

#endif // EFL_EXAMPLE_EOLIAN_CXX_COLOURABLE_SQUARE_STUB_H
