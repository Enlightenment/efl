#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_page_indicator.h"
#include "efl_ui_widget_pager.h"

#define MY_CLASS EFL_PAGE_INDICATOR_CLASS



EOLIAN static void
_efl_page_indicator_update(Eo *obj EINA_UNUSED,
                           Efl_Page_Indicator_Data *pd EINA_UNUSED,
                           double pos EINA_UNUSED)
{

}

EOLIAN static void
_efl_page_indicator_pack_begin(Eo *obj EINA_UNUSED,
                               Efl_Page_Indicator_Data *pd)
{
   pd->cnt++;
}

EOLIAN static void
_efl_page_indicator_pack_end(Eo *obj EINA_UNUSED,
                             Efl_Page_Indicator_Data *pd)
{
   pd->cnt++;
}

EOLIAN static void
_efl_page_indicator_pack_before(Eo *obj EINA_UNUSED,
                                Efl_Page_Indicator_Data *pd,
                                int index EINA_UNUSED)
{
   pd->cnt++;
}

EOLIAN static void
_efl_page_indicator_pack_after(Eo *obj EINA_UNUSED,
                               Efl_Page_Indicator_Data *pd,
                               int index EINA_UNUSED)
{
   pd->cnt++;
}

EOLIAN static Eo *
_efl_page_indicator_efl_object_constructor(Eo *obj,
                                           Efl_Page_Indicator_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->idbox = efl_parent_get(obj);
   pd->pager.obj = efl_parent_get(pd->idbox);

   pd->cnt = efl_content_count(pd->pager.obj);

   return obj;
}



EOAPI EFL_VOID_FUNC_BODYV(efl_page_indicator_update,
                          EFL_FUNC_CALL(pos), double pos)
EOAPI EFL_VOID_FUNC_BODY(efl_page_indicator_pack_begin)
EOAPI EFL_VOID_FUNC_BODY(efl_page_indicator_pack_end)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_indicator_pack_before,
                          EFL_FUNC_CALL(index), int index)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_indicator_pack_after,
                          EFL_FUNC_CALL(index), int index)


#define EFL_PAGE_INDICATOR_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_update, \
                      _efl_page_indicator_update), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack_begin, \
                      _efl_page_indicator_pack_begin), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack_end, \
                      _efl_page_indicator_pack_end), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack_before, \
                      _efl_page_indicator_pack_before), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack_after, \
                      _efl_page_indicator_pack_after)

#include "efl_page_indicator.eo.c"
