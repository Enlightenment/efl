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
                           Efl_Page_Indicator_Data *pd,
                           double pos EINA_UNUSED)
{
   pd->curr_idx = efl_ui_pager_current_page_get(pd->pager.obj);
}

EOLIAN static void
_efl_page_indicator_pack(Eo *obj EINA_UNUSED,
                         Efl_Page_Indicator_Data *pd,
                         int index)
{
   pd->cnt++;
   if ((pd->curr_idx == -1) || (pd->curr_idx >= index)) pd->curr_idx++;
}

EOLIAN static void
_efl_page_indicator_unpack(Eo *obj EINA_UNUSED,
                           Efl_Page_Indicator_Data *pd,
                           int index)
{
   pd->cnt--;

   if ((pd->cnt == 0) ||
       ((index == pd->curr_idx) && (index != 0)) ||
       (index < pd->curr_idx))
     pd->curr_idx--;
}

EOLIAN static void
_efl_page_indicator_unpack_all(Eo *obj EINA_UNUSED,
                               Efl_Page_Indicator_Data *pd)
{
   pd->cnt = 0;
   pd->curr_idx = -1;
}

EOLIAN static void
_efl_page_indicator_bind(Eo *obj EINA_UNUSED,
                         Efl_Page_Indicator_Data *pd,
                         Eo *pager,
                         Efl_Canvas_Group *idbox)
{
   if (pd->pager.obj == pager) return;

   pd->pager.obj = pager;
   pd->idbox = idbox;

   if (pager)
     {
        pd->cnt = efl_content_count(pd->pager.obj);
        pd->curr_idx = efl_ui_pager_current_page_get(pd->pager.obj);
     }
   else
     {
        pd->cnt = 0;
        pd->curr_idx = -1;
     }
}


EOAPI EFL_VOID_FUNC_BODYV(efl_page_indicator_update,
                          EFL_FUNC_CALL(pos), double pos)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_indicator_pack,
                          EFL_FUNC_CALL(index), int index)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_indicator_unpack,
                          EFL_FUNC_CALL(index), int index)
EOAPI EFL_VOID_FUNC_BODY(efl_page_indicator_unpack_all)

#define EFL_PAGE_INDICATOR_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_update, \
                      _efl_page_indicator_update), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_pack, \
                      _efl_page_indicator_pack), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_unpack, \
                      _efl_page_indicator_unpack), \
   EFL_OBJECT_OP_FUNC(efl_page_indicator_unpack_all, \
                      _efl_page_indicator_unpack_all)

#include "efl_page_indicator.eo.c"
