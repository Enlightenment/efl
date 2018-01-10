#include "config.h"
#include "Efl.h"
#include "efl_gfx_color_internal.h"

static int
_format_clean_param(Eina_Tmpstr *s)
{
   Eina_Tmpstr *ss;
   char *ds;
   int len = 0;

   ds = (char*) s;
   for (ss = s; *ss; ss++, ds++, len++)
     {
        if ((*ss == '\\') && *(ss + 1)) ss++;
        if (ds != ss) *ds = *ss;
     }
   *ds = 0;

   return len;
}

EOLIAN static void
_efl_gfx_color_color_code_set(Eo *obj, void *_pd EINA_UNUSED, const char *colorcode)
{
    int len;
    unsigned char r, g, b, a;

    len = _format_clean_param(colorcode);

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

EOLIAN static void
_efl_gfx_color_color_class_code_set(Eo *obj, void *_pd EINA_UNUSED, const char *color_class,
                                    Efl_Gfx_Color_Class_Layer layer, const char *colorcode)
{
   int len;
   unsigned char r, g, b, a;

   len = _format_clean_param(colorcode);

   _gfx_color_format_color_parse(colorcode, len, &r, &g, &b, &a);
   efl_gfx_color_class_set(obj, color_class, layer, r, g, b, a);
}

EOLIAN static const char *
_efl_gfx_color_color_class_code_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED,
                                    const char *color_class, Efl_Gfx_Color_Class_Layer layer)
{
   int r, g, b, a;

   efl_gfx_color_class_get(obj, color_class, layer, &r, &g, &b, &a);
   return eina_slstr_printf("#%02X%02X%02X%02X", r, g, b, a);
}

#include "interfaces/efl_gfx_color.eo.c"
