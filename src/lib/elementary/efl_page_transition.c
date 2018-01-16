#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_page_transition.h"
#include "efl_ui_widget_pager.h"

#define MY_CLASS EFL_PAGE_TRANSITION_CLASS


EOLIAN static void
_efl_page_transition_page_size_set(Eo *obj EINA_UNUSED,
                                   Efl_Page_Transition_Data *pd,
                                   Eina_Size2D sz)
{
   pd->page_spec.sz = sz;
}

EOLIAN static void
_efl_page_transition_padding_size_set(Eo *obj EINA_UNUSED,
                                      Efl_Page_Transition_Data *pd,
                                      int padding)
{
   pd->page_spec.padding = padding;
}

EOLIAN static void
_efl_page_transition_bind(Eo *obj EINA_UNUSED,
                          Efl_Page_Transition_Data *pd,
                          Eo *pager,
                          Efl_Canvas_Group *group)
{
   EFL_UI_PAGER_DATA_GET(pager, ppd);

   pd->pager.obj = pager;
   pd->pager.group = group;

   pd->pager.x = ppd->x;
   pd->pager.y = ppd->y;
   pd->pager.w = ppd->w;
   pd->pager.h = ppd->h;

   pd->page_spec.sz = ppd->page_spec.sz;
   pd->page_spec.padding = ppd->page_spec.padding;

   pd->loop = ppd->loop;
}

EOLIAN static void
_efl_page_transition_update(Eo *obj EINA_UNUSED,
                            Efl_Page_Transition_Data *pd EINA_UNUSED,
                            double move EINA_UNUSED)
{

}

EOLIAN static void
_efl_page_transition_curr_page_change(Eo *obj EINA_UNUSED,
                                      Efl_Page_Transition_Data *pd EINA_UNUSED,
                                      int diff EINA_UNUSED)
{

}

EOLIAN static void
_efl_page_transition_pack_end(Eo *obj EINA_UNUSED,
                              Efl_Page_Transition_Data *pd EINA_UNUSED,
                              Efl_Gfx *subobj EINA_UNUSED)
{

}

EOLIAN static void
_efl_page_transition_loop_set(Eo *obj EINA_UNUSED,
                              Efl_Page_Transition_Data *pd,
                              Efl_Ui_Pager_Loop loop)
{
   pd->loop = loop;
}


EOAPI EFL_VOID_FUNC_BODYV(efl_page_transition_update,
                          EFL_FUNC_CALL(move), double move)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_transition_curr_page_change,
                          EFL_FUNC_CALL(move), double move)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_transition_page_size_set,
                          EFL_FUNC_CALL(sz), Eina_Size2D sz)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_transition_padding_size_set,
                          EFL_FUNC_CALL(padding), int padding)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_transition_pack_end,
                          EFL_FUNC_CALL(subobj), Efl_Gfx *subobj)
EOAPI EFL_VOID_FUNC_BODYV(efl_page_transition_loop_set,
                          EFL_FUNC_CALL(loop), Efl_Ui_Pager_Loop loop)


#define EFL_PAGE_TRANSITION_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_page_transition_update, \
                      _efl_page_transition_update), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_curr_page_change, \
                      _efl_page_transition_curr_page_change), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_page_size_set, \
                      _efl_page_transition_page_size_set), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_padding_size_set, \
                      _efl_page_transition_padding_size_set), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_pack_end, \
                      _efl_page_transition_pack_end), \
   EFL_OBJECT_OP_FUNC(efl_page_transition_loop_set, \
                      _efl_page_transition_loop_set)

#include "efl_page_transition.eo.c"
