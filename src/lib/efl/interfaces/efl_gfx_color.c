#include "config.h"
#include "Efl.h"
#include "efl_gfx_color_internal.h"

EOLIAN static void
_efl_gfx_color_color_code_set(Eo *obj, void *_pd EINA_UNUSED, const char *colorcode)
{
    int len;
    unsigned char r, g, b, a;

    len = _gfx_color_format_clean_param(colorcode);

    _gfx_color_format_color_parse(colorcode, len, &r, &g, &b, &a);
    efl_gfx_color_set(obj, r, g, b, a);
}

EOLIAN static const char *
_efl_gfx_color_color_code_get(Eo *obj, void *_pd EINA_UNUSED)
{
    int r, g, b, a;

    efl_gfx_color_get(obj, &r, &g, &b, &a);
    return eina_slstr_printf("#%02X%02X%02X%02X", r, g, b, a);
}

#include "interfaces/efl_gfx_color.eo.c"
