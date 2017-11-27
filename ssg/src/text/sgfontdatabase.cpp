#include"sgfontdatabase.h"
#include <pango/pangoft2.h>
#include"sgdebug.h"

class SGFontDatabasePrivate
{
public:
    SGFontDatabasePrivate();
public:
    PangoFontMap             *m_fontmap;
    SGFontDatabase::HintMode  m_hinting;
};

static void
substitute_func (FcPattern *pattern,
                 gpointer   data )
{
  SGFontDatabasePrivate *p = (SGFontDatabasePrivate *) data;
  if (p->m_hinting != SGFontDatabase::HintMode::Default)
    {
      FcPatternDel (pattern, FC_HINTING);
      FcPatternAddBool (pattern, FC_HINTING, p->m_hinting != SGFontDatabase::HintMode::None);

      FcPatternDel (pattern, FC_AUTOHINT);
      FcPatternAddBool (pattern, FC_AUTOHINT, p->m_hinting == SGFontDatabase::HintMode::Auto);
    }
}

SGFontDatabasePrivate::SGFontDatabasePrivate():m_hinting(SGFontDatabase::HintMode::Auto)
{
    m_fontmap = pango_ft2_font_map_new();
    pango_ft2_font_map_set_resolution (PANGO_FT2_FONT_MAP (m_fontmap), 1, 1);
    pango_ft2_font_map_set_default_substitute (PANGO_FT2_FONT_MAP (m_fontmap), substitute_func, this, NULL);
}

void SGFontDatabase::print() const
{
    PangoFontFamily** families = NULL;
    int nb;

    pango_font_map_list_families(d->m_fontmap, &families, &nb);

}

SGFontDatabase::~SGFontDatabase()
{
}


SGFontDatabase::SGFontDatabase()
{
    d = new SGFontDatabasePrivate;
}
