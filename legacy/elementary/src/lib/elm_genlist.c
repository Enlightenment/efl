#include <assert.h>
#include <fnmatch.h>
#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "els_scroller.h"
#include "elm_gen_common.h"

// internally allocated
#define CLASS_ALLOCATED 0x3a70f11f

#define MAX_ITEMS_PER_BLOCK 32
#define REORDER_EFFECT_TIME 0.5

#define ELM_GEN_SETUP(wd) \
   (wd)->calc_cb = (Ecore_Cb)_calc_job; \
   (wd)->clear_cb = (Ecore_Cb)_clear_cb; \
   (wd)->sizing_cb = (Ecore_Cb)_sizing_eval

#define ELM_GEN_ITEM_SETUP(it) \
   (it)->del_cb = (Ecore_Cb)_item_del; \
   (it)->highlight_cb = (Ecore_Cb)_item_highlight; \
   (it)->unsel_cb = (Ecore_Cb)_item_unselect; \
   (it)->unhighlight_cb = (Ecore_Cb)_item_unhighlight; \
   (it)->unrealize_cb = (Ecore_Cb)_item_unrealize_cb

#define ELM_GENLIST_CHECK_ITC_VER(itc) \
   do \
     { \
        if (!itc) \
          { \
             ERR("Genlist_Item_Class(itc) is NULL"); \
             return; \
          } \
        if ((itc->version != ELM_GENLIST_ITEM_CLASS_VERSION) && \
            (itc->version != CLASS_ALLOCATED)) \
          { \
             ERR("Genlist_Item_Class version mismatched! current = (%d), required = (%d) or (%d)", itc->version, ELM_GENLIST_ITEM_CLASS_VERSION, CLASS_ALLOCATED); \
             return; \
          } \
     } \
   while(0)

typedef struct _Item_Block  Item_Block;
typedef struct _Item_Cache  Item_Cache;

struct Elm_Gen_Item_Type
{
   Elm_Gen_Item                 *it;
   Item_Block                   *block;
   Eina_List                    *items;
   Evas_Coord                    w, h, minw, minh;
   Elm_Gen_Item                 *group_item;
   Elm_Genlist_Item_Type        type;
   Eina_List                    *mode_texts, *mode_contents, *mode_states, *mode_content_objs;
   Eina_List                    *edit_texts, *edit_contents, *edit_states, *edit_content_objs;
   Ecore_Timer                  *swipe_timer;
   Evas_Coord                    scrl_x, scrl_y, old_scrl_y;

   Elm_Gen_Item                 *rel;
   Evas_Object                  *mode_view;
   int                           expanded_depth;
   int                           order_num_in;

   Eina_Bool                     before : 1;

   Eina_Bool                     want_realize : 1;
   Eina_Bool                     expanded : 1;
   Eina_Bool                     mincalcd : 1;
   Eina_Bool                     queued : 1;
   Eina_Bool                     showme : 1;
   Eina_Bool                     updateme : 1;
   Eina_Bool                     nocache : 1; /* do not use cache for this item */
   Eina_Bool                     nocache_once : 1; /* do not use cache for this item only once */
   Eina_Bool                     stacking_even : 1;
   Eina_Bool                     nostacking : 1;
   Eina_Bool                     move_effect_enabled : 1;
   Eina_Bool                     decorate_mode_item_realized : 1;
   Eina_Bool                     tree_effect_finished : 1; /* tree effect */
   Eina_Bool                     tree_effect_hideme : 1; /* item hide for tree effect */
};

struct _Item_Block
{
   EINA_INLIST;
   int          count;
   int          num;
   int          reorder_offset;
   Widget_Data *wd;
   Eina_List   *items;
   Evas_Coord   x, y, w, h, minw, minh;
   int          position;
   int          item_position_stamp;

   Eina_Bool    position_update : 1;
   Eina_Bool    want_unrealize : 1;
   Eina_Bool    realized : 1;
   Eina_Bool    changed : 1;
   Eina_Bool    updateme : 1;
   Eina_Bool    showme : 1;
   Eina_Bool    must_recalc : 1;
};

struct _Item_Cache
{
   EINA_INLIST;

   Evas_Object *base_view, *spacer;

   const char  *item_style; // it->itc->item_style
   Eina_Bool    tree : 1; // it->group

   Eina_Bool    selected : 1; // it->selected
   Eina_Bool    disabled : 1; // it->disabled
   Eina_Bool    expanded : 1; // it->item->expanded
};

static const char *widtype = NULL;
static void      _item_cache_zero(Widget_Data *wd);
static void      _del_hook(Evas_Object *obj);
static void      _mirrored_set(Evas_Object *obj,
                               Eina_Bool    rtl);
static void      _theme_hook(Evas_Object *obj);
static void      _show_region_hook(void        *data,
                                   Evas_Object *obj);
static void      _sizing_eval(Evas_Object *obj);
static void      _item_realize(Elm_Gen_Item *it,
                               int           in,
                               Eina_Bool     calc);
static void      _item_unrealize_cb(Elm_Gen_Item *it);
static void      _item_block_unrealize(Item_Block *itb);
static void      _calc_job(void *data);
static void      _on_focus_hook(void        *data,
                                Evas_Object *obj);
static Eina_Bool _item_multi_select_up(Widget_Data *wd);
static Eina_Bool _item_multi_select_down(Widget_Data *wd);
static Eina_Bool _item_single_select_up(Widget_Data *wd);
static Eina_Bool _item_single_select_down(Widget_Data *wd);
static Eina_Bool _event_hook(Evas_Object       *obj,
                             Evas_Object       *src,
                             Evas_Callback_Type type,
                             void              *event_info);
static void      _signal_emit_hook(Evas_Object *obj,
                                   const char  *emission,
                                   const char  *source);
static Eina_Bool _deselect_all_items(Widget_Data *wd);
static void      _pan_calculate(Evas_Object *obj);
static void      _pan_max_get(Evas_Object *obj,
                              Evas_Coord  *x,
                              Evas_Coord  *y);
static void      _item_position(Elm_Gen_Item *it,
                                Evas_Object  *obj,
                                Evas_Coord    it_x,
                                Evas_Coord    it_y);
static void      _mode_item_realize(Elm_Gen_Item *it);
static void      _mode_item_unrealize(Elm_Gen_Item *it);
static void      _item_mode_set(Elm_Gen_Item *it);
static void      _item_mode_unset(Widget_Data *wd);
static void      _decorate_mode_item_position(Elm_Gen_Item *it, int itx, int ity);
static void      _decorate_mode_item_realize(Elm_Gen_Item *it, Eina_Bool effect_on);
static void      _decorate_mode_item_unrealize(Elm_Gen_Item *it);
static void      _group_items_recalc(void *data);
static void      _item_move_after(Elm_Gen_Item *it,
                                  Elm_Gen_Item *after);
static void      _item_move_before(Elm_Gen_Item *it,
                                   Elm_Gen_Item *before);
static void      _item_auto_scroll(Widget_Data *wd);
static void      _elm_genlist_clear(Evas_Object *obj,
                                    Eina_Bool    standby);
static void      _pan_child_size_get(Evas_Object *obj,
                                     Evas_Coord  *w,
                                     Evas_Coord  *h);
static Evas_Object* _create_tray_alpha_bg(const Evas_Object *obj);
static void         _item_contract_emit(Elm_Gen_Item *it);
static int          _item_tree_effect_before(Elm_Gen_Item *it);
static void         _item_tree_effect(Widget_Data *wd, int y);
static void         _item_tree_effect_finish(Widget_Data *wd);
static Eina_Bool    _tree_effect_animator_cb(void *data);

static Evas_Smart_Class _pan_sc = EVAS_SMART_CLASS_INIT_VERSION;

static const char SIG_ACTIVATED[] = "activated";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_SELECTED[] = "selected";
static const char SIG_UNSELECTED[] = "unselected";
static const char SIG_EXPANDED[] = "expanded";
static const char SIG_CONTRACTED[] = "contracted";
static const char SIG_EXPAND_REQUEST[] = "expand,request";
static const char SIG_CONTRACT_REQUEST[] = "contract,request";
static const char SIG_REALIZED[] = "realized";
static const char SIG_UNREALIZED[] = "unrealized";
static const char SIG_DRAG_START_UP[] = "drag,start,up";
static const char SIG_DRAG_START_DOWN[] = "drag,start,down";
static const char SIG_DRAG_START_LEFT[] = "drag,start,left";
static const char SIG_DRAG_START_RIGHT[] = "drag,start,right";
static const char SIG_DRAG_STOP[] = "drag,stop";
static const char SIG_DRAG[] = "drag";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_MULTI_SWIPE_LEFT[] = "multi,swipe,left";
static const char SIG_MULTI_SWIPE_RIGHT[] = "multi,swipe,right";
static const char SIG_MULTI_SWIPE_UP[] = "multi,swipe,up";
static const char SIG_MULTI_SWIPE_DOWN[] = "multi,swipe,down";
static const char SIG_MULTI_PINCH_OUT[] = "multi,pinch,out";
static const char SIG_MULTI_PINCH_IN[] = "multi,pinch,in";
static const char SIG_SWIPE[] = "swipe";
static const char SIG_MOVED[] = "moved";
static const char SIG_MOVED_AFTER[] = "moved,after";
static const char SIG_MOVED_BEFORE[] = "moved,before";
static const char SIG_INDEX_UPDATE[] = "index,update";
static const char SIG_TREE_EFFECT_FINISHED [] = "tree,effect,finished";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_ACTIVATED, ""},
   {SIG_SELECTED, ""},
   {SIG_UNSELECTED, ""},
   {SIG_EXPANDED, ""},
   {SIG_CONTRACTED, ""},
   {SIG_EXPAND_REQUEST, ""},
   {SIG_CONTRACT_REQUEST, ""},
   {SIG_REALIZED, ""},
   {SIG_UNREALIZED, ""},
   {SIG_DRAG_START_UP, ""},
   {SIG_DRAG_START_DOWN, ""},
   {SIG_DRAG_START_LEFT, ""},
   {SIG_DRAG_START_RIGHT, ""},
   {SIG_DRAG_STOP, ""},
   {SIG_DRAG, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {SIG_EDGE_TOP, ""},
   {SIG_EDGE_BOTTOM, ""},
   {SIG_EDGE_LEFT, ""},
   {SIG_EDGE_RIGHT, ""},
   {SIG_MULTI_SWIPE_LEFT, ""},
   {SIG_MULTI_SWIPE_RIGHT, ""},
   {SIG_MULTI_SWIPE_UP, ""},
   {SIG_MULTI_SWIPE_DOWN, ""},
   {SIG_MULTI_PINCH_OUT, ""},
   {SIG_MULTI_PINCH_IN, ""},
   {SIG_SWIPE, ""},
   {SIG_MOVED, ""},
   {SIG_MOVED_AFTER, ""},
   {SIG_MOVED_BEFORE, ""},
   {SIG_TREE_EFFECT_FINISHED, ""},
   {NULL, NULL}
};

/* TEMPORARY */
#undef ELM_CHECK_WIDTYPE
#define ELM_CHECK_WIDTYPE(obj, widtype) \
   if ((!obj) || (!elm_genlist_type_check((obj), __func__))) return
#undef ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN
#define ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, ...)                \
   ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
   ELM_CHECK_WIDTYPE(WIDGET((it)), widtype) __VA_ARGS__;

static const char *_gengrid = NULL;
static const char *_genlist = NULL;

/* THIS FUNCTION IS HACKY AND TEMPORARY!!! */
Eina_Bool
elm_genlist_type_check(const Evas_Object *obj,
                       const char        *func)
{
   const char *provided, *expected = "(unknown)";
   static int abort_on_warn = -1;
   provided = elm_widget_type_get(obj);
   if (!_genlist) _genlist = eina_stringshare_add("genlist");
   if (!_gengrid) _gengrid = eina_stringshare_add("gengrid");
   if (EINA_LIKELY(provided == _genlist) || EINA_LIKELY(provided == _gengrid))
     return EINA_TRUE;
   if ((!provided) || (!provided[0]))
     {
        provided = evas_object_type_get(obj);
        if ((!provided) || (!provided[0]))
          provided = "(unknown)";
     }
   ERR("Passing Object: %p in function: %s, of type: '%s' when expecting type: '%s'", obj, func, provided, expected);
   if (abort_on_warn == -1)
     {
        if (getenv("ELM_ERROR_ABORT")) abort_on_warn = 1;
        else abort_on_warn = 0;
     }
   if (abort_on_warn == 1) abort();
   return EINA_FALSE;
}

static Eina_Bool
_event_hook(Evas_Object       *obj,
            Evas_Object       *src __UNUSED__,
            Evas_Callback_Type type,
            void              *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord pan_max_x = 0, pan_max_y = 0;
   if (!wd) return EINA_FALSE;
   if (!wd->items) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   Elm_Object_Item *it = NULL;
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   elm_smart_scroller_child_pos_get(wd->scr, &x, &y);
   elm_smart_scroller_step_size_get(wd->scr, &step_x, &step_y);
   elm_smart_scroller_page_size_get(wd->scr, &page_x, &page_y);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &v_w, &v_h);

   if ((!strcmp(ev->keyname, "Left")) || (!strcmp(ev->keyname, "KP_Left")))
     {
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) || (!strcmp(ev->keyname, "KP_Right")))
     {
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) || (!strcmp(ev->keyname, "KP_Up")))
     {
        if (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
             (_item_multi_select_up(wd)))
            || (_item_single_select_up(wd)))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) || (!strcmp(ev->keyname, "KP_Down")))
     {
        if (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
             (_item_multi_select_down(wd)))
            || (_item_single_select_down(wd)))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Home")) || (!strcmp(ev->keyname, "KP_Home")))
     {
        it = elm_genlist_first_item_get(obj);
        elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
        elm_genlist_item_selected_set(it, EINA_TRUE);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "End")) || (!strcmp(ev->keyname, "KP_End")))
     {
        it = elm_genlist_last_item_get(obj);
        elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
        elm_genlist_item_selected_set(it, EINA_TRUE);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Prior")) || (!strcmp(ev->keyname, "KP_Prior")))
     {
        if (page_y < 0)
          y -= -(page_y * v_h) / 100;
        else
          y -= page_y;
     }
   else if ((!strcmp(ev->keyname, "Next")) || (!strcmp(ev->keyname, "KP_Next")))
     {
        if (page_y < 0)
          y += -(page_y * v_h) / 100;
        else
          y += page_y;
     }
   else if (!strcmp(ev->keyname, "Escape"))
     {
        if (!_deselect_all_items(wd)) return EINA_FALSE;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if (((!strcmp(ev->keyname, "Return")) ||
             (!strcmp(ev->keyname, "KP_Enter")) ||
             (!strcmp(ev->keyname, "space")))
            && (!wd->multi) && (wd->selected))
     {
        it = elm_genlist_selected_item_get(obj);
        elm_genlist_item_expanded_set(it,
                                      !elm_genlist_item_expanded_get(it));
        evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   _pan_max_get(wd->pan_smart, &pan_max_x, &pan_max_y);
   if (x < 0) x = 0;
   if (x > pan_max_x) x = pan_max_x;
   if (y < 0) y = 0;
   if (y > pan_max_y) y = pan_max_y;
   elm_smart_scroller_child_pos_set(wd->scr, x, y);
   return EINA_TRUE;
}

static Eina_Bool
_deselect_all_items(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;
   while (wd->selected)
     elm_genlist_item_selected_set(wd->selected->data, EINA_FALSE);

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_up(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;
   if (!wd->multi) return EINA_FALSE;

   Elm_Object_Item *prev = elm_genlist_item_prev_get(wd->last_selected_item);
   if (!prev) return EINA_TRUE;

   if (elm_genlist_item_selected_get(prev))
     {
        elm_genlist_item_selected_set(wd->last_selected_item, EINA_FALSE);
        wd->last_selected_item = prev;
        elm_genlist_item_show(wd->last_selected_item, ELM_GENLIST_ITEM_SCROLLTO_IN);
     }
   else
     {
        elm_genlist_item_selected_set(prev, EINA_TRUE);
        elm_genlist_item_show(prev, ELM_GENLIST_ITEM_SCROLLTO_IN);
     }
   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_down(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;
   if (!wd->multi) return EINA_FALSE;

   Elm_Object_Item *next;
   next = elm_genlist_item_next_get(wd->last_selected_item);
   if (!next) return EINA_TRUE;

   if (elm_genlist_item_selected_get(next))
     {
        elm_genlist_item_selected_set(wd->last_selected_item, EINA_FALSE);
        wd->last_selected_item = next;
        elm_genlist_item_show(wd->last_selected_item, ELM_GENLIST_ITEM_SCROLLTO_IN);
     }
   else
     {
        elm_genlist_item_selected_set(next, EINA_TRUE);
        elm_genlist_item_show(next, ELM_GENLIST_ITEM_SCROLLTO_IN);
     }
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_up(Widget_Data *wd)
{
   Elm_Gen_Item *prev;
   if (!wd->selected)
     {
        prev = ELM_GEN_ITEM_FROM_INLIST(wd->items->last);
        while ((prev) && (prev->generation < wd->generation))
          prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
     }
   else prev = (Elm_Gen_Item *) elm_genlist_item_prev_get(wd->last_selected_item);

   if (!prev) return EINA_FALSE;

   _deselect_all_items(wd);

   elm_genlist_item_selected_set((Elm_Object_Item *) prev, EINA_TRUE);
   elm_genlist_item_show((Elm_Object_Item *) prev, ELM_GENLIST_ITEM_SCROLLTO_IN);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_down(Widget_Data *wd)
{
   Elm_Gen_Item *next;
   if (!wd->selected)
     {
        next = ELM_GEN_ITEM_FROM_INLIST(wd->items);
        while ((next) && (next->generation < wd->generation))
          next = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
     }
   else next = (Elm_Gen_Item *) elm_genlist_item_next_get(wd->last_selected_item);

   if (!next) return EINA_FALSE;

   _deselect_all_items(wd);

   elm_genlist_item_selected_set((Elm_Object_Item *) next, EINA_TRUE);
   elm_genlist_item_show((Elm_Object_Item *) next, ELM_GENLIST_ITEM_SCROLLTO_IN);
   return EINA_TRUE;
}

static void
_on_focus_hook(void        *data __UNUSED__,
               Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        elm_object_signal_emit(wd->obj, "elm,action,focus", "elm");
        evas_object_focus_set(wd->obj, EINA_TRUE);
        if ((wd->selected) && (!wd->last_selected_item))
          wd->last_selected_item = eina_list_data_get(wd->selected);
     }
   else
     {
        elm_object_signal_emit(wd->obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->obj, EINA_FALSE);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _item_cache_zero(wd);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   if (wd->update_job) ecore_job_del(wd->update_job);
   if (wd->queue_idle_enterer) ecore_idle_enterer_del(wd->queue_idle_enterer);
   if (wd->must_recalc_idler) ecore_idler_del(wd->must_recalc_idler);
   if (wd->multi_timer) ecore_timer_del(wd->multi_timer);
   if (wd->decorate_type) eina_stringshare_del(wd->decorate_type);
   if (wd->scr_hold_timer) ecore_timer_del(wd->scr_hold_timer);
   free(wd);
}

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->decorate_mode) elm_genlist_decorate_mode_set(wd->obj, EINA_FALSE);
   elm_genlist_clear(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool    rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _item_cache_zero(wd);
   elm_smart_scroller_mirrored_set(wd->scr, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item_Block *itb;
   if (!wd) return;
   evas_event_freeze(evas_object_evas_get(wd->obj));
   _item_cache_zero(wd);
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   elm_smart_scroller_object_theme_set(obj, wd->scr, "genlist", "base",
                                       elm_widget_style_get(obj));
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   wd->item_width = wd->item_height = 0;
   wd->group_item_width = wd->group_item_height = 0;
   wd->minw = wd->minh = wd->realminw = 0;
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Eina_List *l;
        Elm_Gen_Item *it;

        if (itb->realized) _item_block_unrealize(itb);
        EINA_LIST_FOREACH(itb->items, l, it)
          it->item->mincalcd = EINA_FALSE;

        itb->changed = EINA_TRUE;
     }
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
   _sizing_eval(obj);
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_show_region_hook(void        *data,
                  Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   if (!wd) return;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   //x & y are screen coordinates, Add with pan coordinates
   x += wd->pan_x;
   y += wd->pan_y;
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_translate_hook(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "language,changed", NULL);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord vmw = 0, vmh = 0;
   if (!wd) return;

   evas_object_size_hint_min_get(wd->scr, &minw, NULL);
   evas_object_size_hint_max_get(wd->scr, &maxw, &maxh);

   edje_object_size_min_calc
      (elm_smart_scroller_edje_object_get(wd->scr), &vmw, &vmh);

   if (wd->mode == ELM_LIST_COMPRESS)
     {
        Evas_Coord vw, vh;

        elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
        if ((vw != 0) && (vw != wd->prev_viewport_w))
          {
             Item_Block *itb;

             wd->prev_viewport_w = vw;
             EINA_INLIST_FOREACH(wd->blocks, itb)
               {
                  itb->must_recalc = EINA_TRUE;
               }
             if (wd->calc_job) ecore_job_del(wd->calc_job);
             wd->calc_job = ecore_job_add(_calc_job, wd);
          }
        minw = vmw;
        minh = vmh;
     }
   else if (wd->mode == ELM_LIST_LIMIT)
     {
        maxw = -1;
        minw = vmw + minw;
     }
   else
     {
        minw = vmw;
        minh = vmh;
     }

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_signal_emit_hook(Evas_Object *obj,
                  const char  *emission,
                  const char  *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
                           emission, source);
}

static void
_item_highlight(Elm_Gen_Item *it)
{
   const char *selectraise;
   if ((it->wd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (!it->wd->highlight) ||
       (it->generation < it->wd->generation) ||
       (it->highlighted) || elm_widget_item_disabled_get(it) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_NONE) || (it->item->mode_view) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     return;
   edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
   if (it->edit_obj) edje_object_signal_emit(it->edit_obj, "elm,state,selected", "elm");
   selectraise = edje_object_data_get(VIEW(it), "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     {
        if (it->edit_obj) evas_object_raise(it->edit_obj);
        else evas_object_raise(VIEW(it));
        if ((it->item->group_item) && (it->item->group_item->realized))
          evas_object_raise(it->item->VIEW(group_item));
     }
   it->highlighted = EINA_TRUE;
}

static void
_item_unhighlight(Elm_Gen_Item *it)
{
   if ((it->generation < it->wd->generation) || (!it->highlighted)) return;
   edje_object_signal_emit(VIEW(it), "elm,state,unselected", "elm");
   if (it->edit_obj) edje_object_signal_emit(it->edit_obj, "elm,state,unselected", "elm");
   if (!it->item->nostacking)
     {
       if ((it->item->order_num_in & 0x1) ^ it->item->stacking_even)
         {
             if (it->edit_obj) evas_object_lower(it->edit_obj);
             else evas_object_lower(VIEW(it));
         }
       else
         {
             if (it->edit_obj) evas_object_raise(it->edit_obj);
             else evas_object_raise(VIEW(it));
         }
     }
   it->highlighted = EINA_FALSE;
}

static void
_item_block_position_update(Eina_Inlist *list, int idx)
{
   Item_Block *tmp;

   EINA_INLIST_FOREACH(list, tmp)
     {
        tmp->position = idx++;
        tmp->position_update = EINA_TRUE;
     }
}

static void
_item_position_update(Eina_List *list, int idx)
{
   Elm_Gen_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(list, l, it)
     {
        it->position = idx++;
        it->position_update = EINA_TRUE;
     }
}

static void
_item_block_del(Elm_Gen_Item *it)
{
   Eina_Inlist *il;
   Item_Block *itb = it->item->block;

   itb->items = eina_list_remove(itb->items, it);
   itb->count--;
   itb->changed = EINA_TRUE;
   if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
   it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
   if (itb->count < 1)
     {
        il = EINA_INLIST_GET(itb);
        Item_Block *itbn = (Item_Block *)(il->next);
        if (it->parent)
          it->parent->item->items = eina_list_remove(it->parent->item->items, it);
        else
          {
             _item_block_position_update(il->next, itb->position);
             it->wd->blocks = eina_inlist_remove(it->wd->blocks, il);
          }
        free(itb);
        if (itbn) itbn->changed = EINA_TRUE;
     }
   else
     {
        if (itb->count < itb->wd->max_items_per_block/2)
          {
             il = EINA_INLIST_GET(itb);
             Item_Block *itbp = (Item_Block *)(il->prev);
             Item_Block *itbn = (Item_Block *)(il->next);
             if ((itbp) && ((itbp->count + itb->count) < itb->wd->max_items_per_block + itb->wd->max_items_per_block/2))
               {
                  Elm_Gen_Item *it2;

                  EINA_LIST_FREE(itb->items, it2)
                    {
                       it2->item->block = itbp;
                       itbp->items = eina_list_append(itbp->items, it2);
                       itbp->count++;
                       itbp->changed = EINA_TRUE;
                    }
                  _item_block_position_update(EINA_INLIST_GET(itb)->next,
                                              itb->position);
                  it->wd->blocks = eina_inlist_remove(it->wd->blocks,
                                                      EINA_INLIST_GET(itb));
                  free(itb);
               }
             else if ((itbn) && ((itbn->count + itb->count) < itb->wd->max_items_per_block + itb->wd->max_items_per_block/2))
               {
                  while (itb->items)
                    {
                       Eina_List *last = eina_list_last(itb->items);
                       Elm_Gen_Item *it2 = last->data;

                       it2->item->block = itbn;
                       itb->items = eina_list_remove_list(itb->items, last);
                       itbn->items = eina_list_prepend(itbn->items, it2);
                       itbn->count++;
                       itbn->changed = EINA_TRUE;
                    }
                  _item_block_position_update(EINA_INLIST_GET(itb)->next,
                                              itb->position);
                  it->wd->blocks =
                    eina_inlist_remove(it->wd->blocks, EINA_INLIST_GET(itb));
                  free(itb);
               }
          }
     }
}

static void
_item_subitems_clear(Elm_Gen_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Eina_List *tl = NULL, *l;
   Elm_Object_Item *it2;

   EINA_LIST_FOREACH(it->item->items, l, it2)
     tl = eina_list_append(tl, it2);
   EINA_LIST_FREE(tl, it2)
     elm_object_item_del(it2);
}

static void
_item_del(Elm_Gen_Item *it)
{
   Evas_Object *obj = WIDGET(it);

   evas_event_freeze(evas_object_evas_get(obj));
   elm_genlist_item_subitems_clear((Elm_Object_Item *)it);
   if (it->wd->show_item == it) it->wd->show_item = NULL;
   if (it->realized) _elm_genlist_item_unrealize(it, EINA_FALSE);
   if (it->item->decorate_mode_item_realized) _decorate_mode_item_unrealize(it);
   if (it->item->block) _item_block_del(it);
   if (it->item->queued)
     it->wd->queue = eina_list_remove(it->wd->queue, it);
   if (it->wd->anchor_item == it)
     {
        it->wd->anchor_item = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        if (!it->wd->anchor_item)
          it->wd->anchor_item = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
     }
   if (it->wd->expanded_item == it) it->wd->expanded_item = NULL;
   if (it->wd->expanded_next_item == it) it->wd->expanded_next_item = NULL;
   if (it->parent)
     it->parent->item->items = eina_list_remove(it->parent->item->items, it);
   if (it->item->swipe_timer) ecore_timer_del(it->item->swipe_timer);
   _elm_genlist_item_del_serious(it);
   elm_genlist_item_class_unref((Elm_Genlist_Item_Class *)it->itc);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_clear_cb(Widget_Data *wd)
{
   wd->anchor_item = NULL;
   while (wd->blocks)
     {
        Item_Block *itb = (Item_Block *)(wd->blocks);

        wd->blocks = eina_inlist_remove(wd->blocks, wd->blocks);
        if (itb->items) eina_list_free(itb->items);
        free(itb);
     }
   if (wd->queue_idle_enterer)
     {
        ecore_idle_enterer_del(wd->queue_idle_enterer);
        wd->queue_idle_enterer = NULL;
     }
   if (wd->must_recalc_idler)
     {
        ecore_idler_del(wd->must_recalc_idler);
        wd->must_recalc_idler = NULL;
     }
   if (wd->queue) wd->queue = eina_list_free(wd->queue);
   if (wd->reorder_move_animator)
     {
        ecore_animator_del(wd->reorder_move_animator);
        wd->reorder_move_animator = NULL;
     }
   wd->show_item = NULL;
   wd->reorder_old_pan_y = 0;
}

static void
_item_unselect(Elm_Gen_Item *it)
{
   if ((it->generation < it->wd->generation) || (!it->selected)) return;
   it->selected = EINA_FALSE;
   it->wd->selected = eina_list_remove(it->wd->selected, it);
   evas_object_smart_callback_call(WIDGET(it), SIG_UNSELECTED, it);
}

static void
_mouse_move(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *obj,
            void        *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;
   Evas_Coord ox, oy, ow, oh, it_scrl_y, y_pos;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        if (!it->wd->on_hold)
          {
             it->wd->on_hold = EINA_TRUE;
             if ((!it->wd->wasselected) && (!it->flipped))
               {
                  _item_unhighlight(it);
                  _item_unselect(it);
               }
          }
     }
   if (it->wd->multitouched)
     {
        it->wd->cur_x = ev->cur.canvas.x;
        it->wd->cur_y = ev->cur.canvas.y;
        return;
     }
   if ((it->dragging) && (it->down))
     {
        if (it->wd->movements == SWIPE_MOVES) it->wd->swipe = EINA_TRUE;
        else
          {
             it->wd->history[it->wd->movements].x = ev->cur.canvas.x;
             it->wd->history[it->wd->movements].y = ev->cur.canvas.y;
             if (abs((it->wd->history[it->wd->movements].x -
                      it->wd->history[0].x)) > 40)
               it->wd->swipe = EINA_TRUE;
             else
               it->wd->movements++;
          }
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG, it);
        return;
     }
   if ((!it->down) /* || (it->wd->on_hold)*/ || (it->wd->longpressed))
     {
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if ((it->wd->reorder_mode) && (it->wd->reorder_it))
          {
             evas_object_geometry_get(it->wd->pan_smart, &ox, &oy, &ow, &oh);
             it_scrl_y = ev->cur.canvas.y - it->wd->reorder_it->dy;

             if (!it->wd->reorder_start_y)
               it->wd->reorder_start_y = it->item->block->y + it->y;

             if (it_scrl_y < oy)
               y_pos = oy;
             else if (it_scrl_y + it->wd->reorder_it->item->h > oy + oh)
               y_pos = oy + oh - it->wd->reorder_it->item->h;
             else
               y_pos = it_scrl_y;

             if (it->edit_obj)
               _item_position(it, it->edit_obj, it->item->scrl_x, y_pos);
             else
             _item_position(it, VIEW(it), it->item->scrl_x, y_pos);

             if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
             it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
          }
        return;
     }
   if (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
     elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - it->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   dy = y - it->dy;
   ady = dy;
   if (ady < 0) ady = -dy;
   minw /= 2;
   minh /= 2;
   if ((adx > minw) || (ady > minh))
     {
        it->dragging = EINA_TRUE;
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if (!it->wd->wasselected)
          {
             _item_unhighlight(it);
             _item_unselect(it);
          }
        if (dy < 0)
          {
             if (ady > adx)
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_DRAG_START_UP, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(WIDGET(it),
                                                    SIG_DRAG_START_LEFT, it);
                  else
                    evas_object_smart_callback_call(WIDGET(it),
                                                    SIG_DRAG_START_RIGHT, it);
               }
          }
        else
          {
             if (ady > adx)
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_DRAG_START_DOWN, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(WIDGET(it),
                                                    SIG_DRAG_START_LEFT, it);
                  else
                    evas_object_smart_callback_call(WIDGET(it),
                                                    SIG_DRAG_START_RIGHT, it);
               }
          }
     }
}

static Eina_Bool
_long_press(void *data)
{
   Elm_Gen_Item *it = data, *it_tmp;
   Eina_List *list, *l;

   it->long_timer = NULL;
   if (elm_widget_item_disabled_get(it) || (it->dragging) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     return ECORE_CALLBACK_CANCEL;
   it->wd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(WIDGET(it), SIG_LONGPRESSED, it);
   if ((it->wd->reorder_mode) && (!it->group))
     {
        it->wd->reorder_it = it;
        it->wd->reorder_start_y = 0;
        if (it->edit_obj)
          evas_object_raise(it->edit_obj);
        else
          evas_object_raise(VIEW(it));

        elm_smart_scroller_hold_set(it->wd->scr, EINA_TRUE);
        elm_smart_scroller_bounce_allow_set(it->wd->scr, EINA_FALSE, EINA_FALSE);

        list = elm_genlist_realized_items_get(it->wd->obj);
        EINA_LIST_FOREACH(list, l, it_tmp)
          {
             if (it != it_tmp) _item_unselect(it_tmp);
          }
        if (elm_genlist_item_expanded_get((Elm_Object_Item *)it))
          {
             elm_genlist_item_expanded_set((Elm_Object_Item *)it, EINA_FALSE);
             return ECORE_CALLBACK_RENEW;
          }

        if (!it->wd->decorate_mode)
          edje_object_signal_emit(VIEW(it), "elm,state,reorder,enabled", "elm");
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_swipe(Elm_Gen_Item *it)
{
   int i, sum = 0;

   if (!it) return;
   if ((it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       elm_widget_item_disabled_get(it)) return;
   it->wd->swipe = EINA_FALSE;
   for (i = 0; i < it->wd->movements; i++)
     {
        sum += it->wd->history[i].x;
        if (abs(it->wd->history[0].y - it->wd->history[i].y) > 10) return;
     }

   sum /= it->wd->movements;
   if (abs(sum - it->wd->history[0].x) <= 10) return;
   evas_object_smart_callback_call(WIDGET(it), SIG_SWIPE, it);
}

static Eina_Bool
_swipe_cancel(void *data)
{
   Elm_Gen_Item *it = data;

   if (!it) return ECORE_CALLBACK_CANCEL;
   it->wd->swipe = EINA_FALSE;
   it->wd->movements = 0;
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_multi_cancel(void *data)
{
   Widget_Data *wd = data;

   if (!wd) return ECORE_CALLBACK_CANCEL;
   wd->multi_timeout = EINA_TRUE;
   return ECORE_CALLBACK_RENEW;
}

static void
_multi_touch_gesture_eval(void *data)
{
   Elm_Gen_Item *it = data;

   it->wd->multitouched = EINA_FALSE;
   if (it->wd->multi_timer)
     {
        ecore_timer_del(it->wd->multi_timer);
        it->wd->multi_timer = NULL;
     }
   if (it->wd->multi_timeout)
     {
        it->wd->multi_timeout = EINA_FALSE;
        return;
     }

   Evas_Coord minw = 0, minh = 0;
   Evas_Coord off_x, off_y, off_mx, off_my;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   off_x = abs(it->wd->cur_x - it->wd->prev_x);
   off_y = abs(it->wd->cur_y - it->wd->prev_y);
   off_mx = abs(it->wd->cur_mx - it->wd->prev_mx);
   off_my = abs(it->wd->cur_my - it->wd->prev_my);

   if (((off_x > minw) || (off_y > minh)) && ((off_mx > minw) || (off_my > minh)))
     {
        if ((off_x + off_mx) > (off_y + off_my))
          {
             if ((it->wd->cur_x > it->wd->prev_x) && (it->wd->cur_mx > it->wd->prev_mx))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_SWIPE_RIGHT, it);
             else if ((it->wd->cur_x < it->wd->prev_x) && (it->wd->cur_mx < it->wd->prev_mx))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_SWIPE_LEFT, it);
             else if (abs(it->wd->cur_x - it->wd->cur_mx) > abs(it->wd->prev_x - it->wd->prev_mx))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_PINCH_OUT, it);
             else
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_PINCH_IN, it);
          }
        else
          {
             if ((it->wd->cur_y > it->wd->prev_y) && (it->wd->cur_my > it->wd->prev_my))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_SWIPE_DOWN, it);
             else if ((it->wd->cur_y < it->wd->prev_y) && (it->wd->cur_my < it->wd->prev_my))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_SWIPE_UP, it);
             else if (abs(it->wd->cur_y - it->wd->cur_my) > abs(it->wd->prev_y - it->wd->prev_my))
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_PINCH_OUT, it);
             else
               evas_object_smart_callback_call(WIDGET(it),
                                               SIG_MULTI_PINCH_IN, it);
          }
     }
   it->wd->multi_timeout = EINA_FALSE;
}

static void
_multi_down(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *obj __UNUSED__,
            void        *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Multi_Down *ev = event_info;

   if ((it->wd->multi_device != 0) || (it->wd->multitouched) || (it->wd->multi_timeout)) return;
   it->wd->multi_device = ev->device;
   it->wd->multi_down = EINA_TRUE;
   it->wd->multitouched = EINA_TRUE;
   it->wd->prev_mx = ev->canvas.x;
   it->wd->prev_my = ev->canvas.y;
   if (!it->wd->wasselected)
     {
        _item_unhighlight(it);
        _item_unselect(it);
     }
   it->wd->wasselected = EINA_FALSE;
   it->wd->longpressed = EINA_FALSE;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->dragging)
     {
        it->dragging = EINA_FALSE;
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG_STOP, it);
     }
   if (it->item->swipe_timer)
     {
        ecore_timer_del(it->item->swipe_timer);
        it->item->swipe_timer = NULL;
     }
   if (it->wd->on_hold)
     {
        it->wd->swipe = EINA_FALSE;
        it->wd->movements = 0;
        it->wd->on_hold = EINA_FALSE;
     }
}

static void
_multi_up(void        *data,
          Evas        *evas __UNUSED__,
          Evas_Object *obj __UNUSED__,
          void        *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Multi_Up *ev = event_info;

   if (it->wd->multi_device != ev->device) return;
   it->wd->multi_device = 0;
   it->wd->multi_down = EINA_FALSE;
   if (it->wd->mouse_down) return;
   _multi_touch_gesture_eval(data);
}

static void
_multi_move(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *obj __UNUSED__,
            void        *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Multi_Move *ev = event_info;

   if (it->wd->multi_device != ev->device) return;
   it->wd->cur_mx = ev->cur.canvas.x;
   it->wd->cur_my = ev->cur.canvas.y;
}

static void
_mouse_down(void        *data,
            Evas        *evas __UNUSED__,
            Evas_Object *obj,
            void        *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        it->wd->on_hold = EINA_TRUE;
     }

   it->down = EINA_TRUE;
   it->dragging = EINA_FALSE;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   it->dx = ev->canvas.x - x;
   it->dy = ev->canvas.y - y;
   it->wd->mouse_down = EINA_TRUE;
   if (!it->wd->multitouched)
     {
        it->wd->prev_x = ev->canvas.x;
        it->wd->prev_y = ev->canvas.y;
        it->wd->multi_timeout = EINA_FALSE;
        if (it->wd->multi_timer) ecore_timer_del(it->wd->multi_timer);
        it->wd->multi_timer = ecore_timer_add(1, _multi_cancel, it->wd);
     }
   it->wd->longpressed = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = EINA_TRUE;
   else it->wd->on_hold = EINA_FALSE;
   if (it->wd->on_hold) return;
   it->wd->wasselected = it->selected;
   _item_highlight(it);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     if ((!elm_widget_item_disabled_get(it)) &&
         (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
       {
          evas_object_smart_callback_call(WIDGET(it), SIG_CLICKED_DOUBLE, it);
          evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
       }
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->item->swipe_timer) ecore_timer_del(it->item->swipe_timer);
   it->item->swipe_timer = ecore_timer_add(0.4, _swipe_cancel, it);
   if (it->realized)
     it->long_timer = ecore_timer_add(it->wd->longpress_timeout, _long_press,
                                      it);
   else
     it->long_timer = NULL;
   it->wd->swipe = EINA_FALSE;
   it->wd->movements = 0;
}

static void
_mouse_up(void        *data,
          Evas        *evas __UNUSED__,
          Evas_Object *obj __UNUSED__,
          void        *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool dragged = EINA_FALSE;

   if (ev->button != 1) return;
   it->down = EINA_FALSE;
   it->wd->mouse_down = EINA_FALSE;
   if (it->wd->multitouched)
     {
        if ((!it->wd->multi) && (!it->selected) && (it->highlighted)) _item_unhighlight(it);
        if (it->wd->multi_down) return;
        _multi_touch_gesture_eval(data);
        return;
     }
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = EINA_TRUE;
   else it->wd->on_hold = EINA_FALSE;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->dragging)
     {
        it->dragging = EINA_FALSE;
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG_STOP, it);
        dragged = 1;
     }
   if (it->item->swipe_timer)
     {
        ecore_timer_del(it->item->swipe_timer);
        it->item->swipe_timer = NULL;
     }
   if (it->wd->multi_timer)
     {
        ecore_timer_del(it->wd->multi_timer);
        it->wd->multi_timer = NULL;
        it->wd->multi_timeout = EINA_FALSE;
     }
   if (it->wd->on_hold)
     {
        if (it->wd->swipe) _swipe(data);
        it->wd->longpressed = EINA_FALSE;
        it->wd->on_hold = EINA_FALSE;
        return;
     }
   if ((it->wd->reorder_mode) && (it->wd->reorder_it))
     {
        Evas_Coord it_scrl_y = ev->canvas.y - it->wd->reorder_it->dy;

        if (it->wd->reorder_rel && (it->wd->reorder_it->parent == it->wd->reorder_rel->parent))
          {
             if (it_scrl_y <= it->wd->reorder_rel->item->scrl_y)
               _item_move_before(it->wd->reorder_it, it->wd->reorder_rel);
             else
               _item_move_after(it->wd->reorder_it, it->wd->reorder_rel);
             evas_object_smart_callback_call(WIDGET(it), SIG_MOVED, it);
          }
        else
          {
             if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
             it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
          }
        edje_object_signal_emit(VIEW(it), "elm,state,reorder,disabled", "elm");
        it->wd->reorder_it = it->wd->reorder_rel = NULL;
        elm_smart_scroller_hold_set(it->wd->scr, EINA_FALSE);
        elm_smart_scroller_bounce_allow_set(it->wd->scr, it->wd->h_bounce, it->wd->v_bounce);
     }
   if (it->wd->longpressed)
     {
        it->wd->longpressed = EINA_FALSE;
        if ((!it->wd->wasselected) && (!it->flipped))
          {
             _item_unhighlight(it);
             _item_unselect(it);
          }
        it->wd->wasselected = EINA_FALSE;
        return;
     }
   if (dragged)
     {
        if (it->want_unrealize)
          {
             _elm_genlist_item_unrealize(it, EINA_FALSE);
             if (it->item->block->want_unrealize)
               _item_block_unrealize(it->item->block);
          }
     }
   if (elm_widget_item_disabled_get(it) || (dragged) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (it->wd->multi)
     {
        if (!it->selected)
          {
             _item_highlight(it);
             it->sel_cb(it);
          }
        else
          {
             _item_unhighlight(it);
             _item_unselect(it);
          }
     }
   else
     {
        if (!it->selected)
          {
             Widget_Data *wd = it->wd;
             if (wd)
               {
                  while (wd->selected)
                    {
                       _item_unhighlight(wd->selected->data);
                       _item_unselect(wd->selected->data);
                    }
               }
          }
        else
          {
             const Eina_List *l, *l_next;
             Elm_Gen_Item *it2;

             EINA_LIST_FOREACH_SAFE(it->wd->selected, l, l_next, it2)
               if (it2 != it)
                  {
                     _item_unhighlight(it2);
                     _item_unselect(it2);
                  }
             //_item_highlight(it);
             //_item_select(it);
          }
        _item_highlight(it);
        it->sel_cb(it);
     }
}

static void
_item_mouse_callbacks_add(Elm_Gen_Item *it, Evas_Object *view)
{
   evas_object_event_callback_add(view, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, it);
   evas_object_event_callback_add(view, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, it);
   evas_object_event_callback_add(view, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move, it);
   evas_object_event_callback_add(view, EVAS_CALLBACK_MULTI_DOWN,
                                  _multi_down, it);
   evas_object_event_callback_add(view, EVAS_CALLBACK_MULTI_UP,
                                  _multi_up, it);
   evas_object_event_callback_add(view, EVAS_CALLBACK_MULTI_MOVE,
                                  _multi_move, it);
}

static void
_item_mouse_callbacks_del(Elm_Gen_Item *it, Evas_Object *view)
{
   evas_object_event_callback_del_full(view, EVAS_CALLBACK_MOUSE_DOWN,
                                       _mouse_down, it);
   evas_object_event_callback_del_full(view, EVAS_CALLBACK_MOUSE_UP,
                                       _mouse_up, it);
   evas_object_event_callback_del_full(view, EVAS_CALLBACK_MOUSE_MOVE,
                                       _mouse_move, it);
   evas_object_event_callback_del_full(view, EVAS_CALLBACK_MULTI_DOWN,
                                       _multi_down, it);
   evas_object_event_callback_del_full(view, EVAS_CALLBACK_MULTI_UP,
                                       _multi_up, it);
   evas_object_event_callback_del_full(view, EVAS_CALLBACK_MULTI_MOVE,
                                       _multi_move, it);
}

static void
_signal_expand_toggle(void        *data,
                      Evas_Object *obj __UNUSED__,
                      const char  *emission __UNUSED__,
                      const char  *source __UNUSED__)
{
   Elm_Gen_Item *it = data;

   if (it->item->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_CONTRACT_REQUEST, it);
   else
     evas_object_smart_callback_call(WIDGET(it), SIG_EXPAND_REQUEST, it);
}

static void
_signal_expand(void        *data,
               Evas_Object *obj __UNUSED__,
               const char  *emission __UNUSED__,
               const char  *source __UNUSED__)
{
   Elm_Gen_Item *it = data;

   if (!it->item->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_EXPAND_REQUEST, it);
}

static void
_signal_contract(void        *data,
                 Evas_Object *obj __UNUSED__,
                 const char  *emission __UNUSED__,
                 const char  *source __UNUSED__)
{
   Elm_Gen_Item *it = data;

   if (it->item->expanded)
     evas_object_smart_callback_call(WIDGET(it), SIG_CONTRACT_REQUEST, it);
}

static Eina_Bool
_scr_hold_timer_cb(void *data)
{
   if (!data) return ECORE_CALLBACK_CANCEL;
   Widget_Data *wd = data;
   elm_smart_scroller_hold_set(wd->scr, EINA_FALSE);
   wd->scr_hold_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_mode_finished_signal_cb(void        *data,
                         Evas_Object *obj,
                         const char  *emission __UNUSED__,
                         const char  *source __UNUSED__)
{
   if (!data) return;
   if (!obj) return;
   Elm_Gen_Item *it = data;
   if ((it->generation < it->wd->generation) || (!it->realized) || (!it->item->mode_view)) return;
   char buf[1024];
   Evas *te = evas_object_evas_get(obj);

   evas_event_freeze(te);
   it->item->nocache_once = EINA_FALSE;
   _mode_item_unrealize(it);
   if (it->item->group_item)
     evas_object_raise(it->item->VIEW(group_item));
   snprintf(buf, sizeof(buf), "elm,state,%s,passive,finished", it->wd->decorate_type);
   edje_object_signal_callback_del_full(obj, buf, "elm", _mode_finished_signal_cb, it);
   evas_event_thaw(te);
   evas_event_thaw_eval(te);
}

static void
_item_cache_clean(Widget_Data *wd)
{
   evas_event_freeze(evas_object_evas_get(wd->obj));
   while ((wd->item_cache) && (wd->item_cache_count > wd->item_cache_max))
     {
        Item_Cache *itc;

        itc = EINA_INLIST_CONTAINER_GET(wd->item_cache->last, Item_Cache);
        wd->item_cache = eina_inlist_remove(wd->item_cache,
                                            wd->item_cache->last);
        wd->item_cache_count--;
        if (itc->spacer) evas_object_del(itc->spacer);
        if (itc->base_view) evas_object_del(itc->base_view);
        eina_stringshare_del(itc->item_style);
        free(itc);
     }
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_item_cache_zero(Widget_Data *wd)
{
   int pmax = wd->item_cache_max;
   wd->item_cache_max = 0;
   _item_cache_clean(wd);
   wd->item_cache_max = pmax;
}

static void
_item_cache_add(Elm_Gen_Item *it)
{
   Item_Cache *itc;

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   if (it->wd->item_cache_max <= 0)
     {
        evas_object_del(VIEW(it));
        VIEW(it) = NULL;
        if (it->spacer)
          {
             evas_object_del(it->spacer);
             it->spacer = NULL;
          }
        evas_event_thaw(evas_object_evas_get(it->wd->obj));
        evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
        return;
     }

   it->wd->item_cache_count++;
   itc = calloc(1, sizeof(Item_Cache));
   if (!itc)
     {
        evas_event_thaw(evas_object_evas_get(it->wd->obj));
        evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
        return;
     }
   it->wd->item_cache = eina_inlist_prepend(it->wd->item_cache,
                                            EINA_INLIST_GET(itc));
   itc->spacer = it->spacer;
   it->spacer = NULL;
   itc->base_view = VIEW(it);
   VIEW(it) = NULL;
   edje_object_signal_emit(itc->base_view, "elm,state,unselected", "elm");
   evas_object_hide(itc->base_view);
   evas_object_move(itc->base_view, -9999, -9999);
   itc->item_style = eina_stringshare_add(it->itc->item_style);
   if (it->item->type & ELM_GENLIST_ITEM_TREE) itc->tree = 1;
   itc->selected = it->selected;
   itc->disabled = elm_widget_item_disabled_get(it);
   itc->expanded = it->item->expanded;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->item->swipe_timer)
     {
        ecore_timer_del(it->item->swipe_timer);
        it->item->swipe_timer = NULL;
     }
   // FIXME: other callbacks?
   edje_object_signal_callback_del_full(itc->base_view,
                                        "elm,action,expand,toggle",
                                        "elm", _signal_expand_toggle, it);
   edje_object_signal_callback_del_full(itc->base_view, "elm,action,expand",
                                        "elm",
                                        _signal_expand, it);
   edje_object_signal_callback_del_full(itc->base_view, "elm,action,contract",
                                        "elm", _signal_contract, it);
   _item_mouse_callbacks_del(it, itc->base_view);
   _item_cache_clean(it->wd);
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
}

static Item_Cache *
_item_cache_find(Elm_Gen_Item *it)
{
   Item_Cache *itc;
   Eina_Bool tree = 0;

   if (it->item->type & ELM_GENLIST_ITEM_TREE) tree = 1;
   EINA_INLIST_FOREACH(it->wd->item_cache, itc)
     {
        if ((itc->selected) || (itc->disabled) || (itc->expanded))
          continue;
        if ((itc->tree == tree) &&
            (((!it->itc->item_style) && (!itc->item_style)) ||
             (it->itc->item_style && itc->item_style &&
            (!strcmp(it->itc->item_style, itc->item_style)))))
          {
             it->wd->item_cache = eina_inlist_remove(it->wd->item_cache,
                                                     EINA_INLIST_GET(itc));
             it->wd->item_cache_count--;
             return itc;
          }
     }
   return NULL;
}

static void
_elm_genlist_item_index_update(Elm_Gen_Item *it)
{
   if (it->position_update || it->item->block->position_update)
     {
        evas_object_smart_callback_call(WIDGET(it), SIG_INDEX_UPDATE, it);
        it->position_update = EINA_FALSE;
     }
}

static void
_elm_genlist_item_odd_even_update(Elm_Gen_Item *it)
{
   if (!it->item->nostacking)
     {
        if ((it->item->order_num_in & 0x1) ^ it->item->stacking_even)
          {
             if (it->edit_obj) evas_object_lower(it->edit_obj);
             else evas_object_lower(VIEW(it));
          }
        else
          {
             if (it->edit_obj) evas_object_raise(it->edit_obj);
             else evas_object_raise(VIEW(it));
          }
     }

   if (it->item->order_num_in & 0x1)
     {
        edje_object_signal_emit(VIEW(it), "elm,state,odd", "elm");
        if (it->edit_obj) edje_object_signal_emit(it->edit_obj, "elm,state,odd", "elm");
     }
   else
     {
        edje_object_signal_emit(VIEW(it), "elm,state,even", "elm");
        if (it->edit_obj) edje_object_signal_emit(it->edit_obj, "elm,state,even", "elm");
     }
}

static void
_elm_genlist_item_state_update(Elm_Gen_Item *it, Item_Cache *itc)
{
   if (itc)
     {
        if (it->selected != itc->selected)
          {
             if (it->selected)
               {
                  edje_object_signal_emit(VIEW(it),
                                          "elm,state,selected", "elm");

                  if (it->edit_obj)
                    edje_object_signal_emit(it->edit_obj,
                                            "elm,state,selected", "elm");
               }
          }
        if (elm_widget_item_disabled_get(it) != itc->disabled)
          {
             if (elm_widget_item_disabled_get(it))
               edje_object_signal_emit(VIEW(it),
                                       "elm,state,disabled", "elm");
             if (it->edit_obj)
               edje_object_signal_emit(it->edit_obj,
                                       "elm,state,disabled", "elm");
          }
        if (it->item->expanded != itc->expanded)
          {
             if (it->item->expanded)
               edje_object_signal_emit(VIEW(it),
                                       "elm,state,expanded", "elm");
             if (it->edit_obj)
               edje_object_signal_emit(it->edit_obj,
                                       "elm,state,expanded", "elm");
          }
     }
   else
     {
        if (it->selected)
          {
             edje_object_signal_emit(VIEW(it),
                                     "elm,state,selected", "elm");
             if (it->edit_obj)
               edje_object_signal_emit(it->edit_obj,
                                       "elm,state,selected", "elm");
          }

        if (elm_widget_item_disabled_get(it))
          {
             edje_object_signal_emit(VIEW(it),
                                     "elm,state,disabled", "elm");
             if (it->edit_obj)
               edje_object_signal_emit(it->edit_obj,
                                       "elm,state,disabled", "elm");
          }
        if (it->item->expanded)
          {
             edje_object_signal_emit(VIEW(it),
                                     "elm,state,expanded", "elm");
             if (it->edit_obj)
               edje_object_signal_emit(it->edit_obj,
                                       "elm,state,expanded", "elm");
          }
     }
}

static void
_item_cache_free(Item_Cache *itc)
{
   if (itc->spacer) evas_object_del(itc->spacer);
   if (itc->base_view) evas_object_del(itc->base_view);
   eina_stringshare_del(itc->item_style);
   free(itc);
}

static void
_item_text_realize(Elm_Gen_Item *it,
                   Evas_Object *target,
                   Eina_List **source,
                   const char *parts)
{
   if (it->itc->func.text_get)
     {
        const Eina_List *l;
        const char *key;

        *source = elm_widget_stringlist_get(edje_object_data_get(target, "texts"));
        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             char *s = it->itc->func.text_get
                ((void *)it->base.data, WIDGET(it), key);

             if (s)
               {
                  edje_object_part_text_set(target, key, s);
                  free(s);
               }
             else
               {
                  edje_object_part_text_set(target, key, "");
               }
          }
     }
}

static Eina_List *
_item_content_unrealize(Elm_Gen_Item *it,
                        Evas_Object *target,
                        Eina_List **source,
                        const char *parts)
{
   Eina_List *res = it->content_objs;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;
        Evas_Object *ic = NULL;

        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             ic = edje_object_part_swallow_get(target, key);
             if (ic)
               {
                  res = eina_list_remove(res, ic);
                  edje_object_part_unswallow(target, ic);
                  evas_object_del(ic);
               }
          }
     }

   return res;
}

static Eina_List *
_item_content_realize(Elm_Gen_Item *it,
                      Evas_Object *target,
                      Eina_List **source,
                      const char *parts)
{
   Eina_List *res = NULL;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;
        Evas_Object *ic = NULL;
        Eina_List *cons = NULL;

        cons = elm_widget_stringlist_get(edje_object_data_get(target, "contents"));

        if (parts && (eina_list_count(*source) != eina_list_count(it->content_objs)))
          res = it->content_objs;

        EINA_LIST_FOREACH(cons, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             if (it->itc->func.content_get)
               ic = it->itc->func.content_get
                  ((void *)it->base.data, WIDGET(it), key);
             if (ic)
               {
                  res = eina_list_append(res, ic);
                  edje_object_part_swallow(target, key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(WIDGET(it), ic);
                  if (elm_widget_item_disabled_get(it))
                    elm_widget_disabled_set(ic, EINA_TRUE);
               }
          }
        *source = eina_list_merge(*source, cons);
     }

   return res;
}

static void
_item_state_realize(Elm_Gen_Item *it,
                    Evas_Object *target,
                    Eina_List **source,
                    const char *parts)
{
   if (it->itc->func.state_get)
     {
        const Eina_List *l;
        const char *key;
        char buf[4096];

        *source = elm_widget_stringlist_get(edje_object_data_get(target, "states"));
        EINA_LIST_FOREACH(*source, l, key)
          {
             if (parts && fnmatch(parts, key, FNM_PERIOD))
               continue;

             Eina_Bool on = it->itc->func.state_get
                ((void *)it->base.data, WIDGET(it), key);

             if (on)
               {
                  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
                  edje_object_signal_emit(target, buf, "elm");
               }
             else
               {
                  snprintf(buf, sizeof(buf), "elm,state,%s,passive", key);
                  edje_object_signal_emit(target, buf, "elm");
               }
          }
     }
}

static Eina_List *
_item_flips_realize(Elm_Gen_Item *it,
                    Evas_Object *target,
                    Eina_List **source)
{
   Eina_List *res = NULL;

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;
        Evas_Object *ic = NULL;
        Eina_List *cons = NULL;

        cons = elm_widget_stringlist_get(edje_object_data_get(target, "flips"));

        EINA_LIST_FOREACH(cons, l, key)
          {
             if (it->itc->func.content_get)
               ic = it->itc->func.content_get
                  ((void *)it->base.data, WIDGET(it), key);
             if (ic)
               {
                  res = eina_list_append(res, ic);
                  edje_object_part_swallow(target, key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(WIDGET(it), ic);
                  if (elm_widget_item_disabled_get(it))
                    elm_widget_disabled_set(ic, EINA_TRUE);
               }
          }
        *source = eina_list_merge(*source, cons);
     }

   return res;
}

static void
_item_realize(Elm_Gen_Item *it,
              int               in,
              Eina_Bool         calc)
{
   const char *treesize;
   char buf[1024];
   int tsize = 20;
   Item_Cache *itc = NULL;

   if (it->generation < it->wd->generation) return;
   //evas_event_freeze(evas_object_evas_get(it->wd->obj));
   if (it->realized)
     {
        if (it->item->order_num_in != in)
          {
             it->item->order_num_in = in;
             _elm_genlist_item_odd_even_update(it);
             _elm_genlist_item_state_update(it, NULL);
             _elm_genlist_item_index_update(it);
          }
        //evas_event_thaw(evas_object_evas_get(it->wd->obj));
        //evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
        return;
     }
   it->item->order_num_in = in;

   if (it->item->nocache_once)
     it->item->nocache_once = EINA_FALSE;
   else if (!it->item->nocache)
     itc = _item_cache_find(it);
   if (itc && (!it->wd->tree_effect_enabled))
     {
        VIEW(it) = itc->base_view;
        itc->base_view = NULL;
        it->spacer = itc->spacer;
        itc->spacer = NULL;
     }
   else
     {
        const char *stacking_even;
        const char *stacking;

        VIEW(it) = edje_object_add(evas_object_evas_get(WIDGET(it)));
        edje_object_scale_set(VIEW(it),
                              elm_widget_scale_get(WIDGET(it)) *
                              _elm_config->scale);
        evas_object_smart_member_add(VIEW(it), it->wd->pan_smart);
        elm_widget_sub_object_add(WIDGET(it), VIEW(it));

        if (it->item->type & ELM_GENLIST_ITEM_TREE)
          snprintf(buf, sizeof(buf), "tree%s/%s", it->wd->mode == ELM_LIST_COMPRESS ? "_compress" : "", it->itc->item_style ?: "default");
        else
          snprintf(buf, sizeof(buf), "item%s/%s", it->wd->mode == ELM_LIST_COMPRESS ? "_compress" : "", it->itc->item_style ?: "default");

        _elm_theme_object_set(WIDGET(it), VIEW(it), "genlist", buf,
                              elm_widget_style_get(WIDGET(it)));

        stacking_even = edje_object_data_get(VIEW(it), "stacking_even");
        if (!stacking_even) stacking_even = "above";
        it->item->stacking_even = !!strcmp("above", stacking_even);

        stacking = edje_object_data_get(VIEW(it), "stacking");
        if (!stacking) stacking = "yes";
        it->item->nostacking = !!strcmp("yes", stacking);

        edje_object_mirrored_set(VIEW(it),
                                 elm_widget_mirrored_get(WIDGET(it)));
     }

   _elm_genlist_item_odd_even_update(it);

   treesize = edje_object_data_get(VIEW(it), "treesize");
   if (treesize) tsize = atoi(treesize);
   if (!it->spacer && treesize)
     {
        it->spacer =
          evas_object_rectangle_add(evas_object_evas_get(WIDGET(it)));
        evas_object_color_set(it->spacer, 0, 0, 0, 0);
        elm_widget_sub_object_add(WIDGET(it), it->spacer);
     }
   if (it->spacer)
     {
        evas_object_size_hint_min_set(it->spacer,
                                      (it->item->expanded_depth * tsize) * _elm_config->scale, 1);
        edje_object_part_swallow(VIEW(it), "elm.swallow.pad", it->spacer);
     }
   if (!calc)
     {
        edje_object_signal_callback_add(VIEW(it),
                                        "elm,action,expand,toggle",
                                        "elm", _signal_expand_toggle, it);
        edje_object_signal_callback_add(VIEW(it), "elm,action,expand",
                                        "elm", _signal_expand, it);
        edje_object_signal_callback_add(VIEW(it), "elm,action,contract",
                                        "elm", _signal_contract, it);
        _item_mouse_callbacks_add(it, VIEW(it));

        if ((it->wd->decorate_mode) && (!it->edit_obj) &&
            (it->item->type != ELM_GENLIST_ITEM_GROUP) && (it->itc->decorate_all_item_style))
          _decorate_mode_item_realize(it, EINA_FALSE);

        _elm_genlist_item_state_update(it, itc);
        _elm_genlist_item_index_update(it);
     }

   if ((calc) && (it->wd->homogeneous) &&
       ((it->wd->item_width) ||
        ((it->wd->item_width) && (it->wd->group_item_width))))
     {
        /* homogenous genlist shortcut */
        if (!it->item->mincalcd)
          {
             if (it->group)
               {
                  it->item->w = it->item->minw = it->wd->group_item_width;
                  it->item->h = it->item->minh = it->wd->group_item_height;
               }
             else
               {
                  it->item->w = it->item->minw = it->wd->item_width;
                  it->item->h = it->item->minh = it->wd->item_height;
               }
             it->item->mincalcd = EINA_TRUE;
          }
     }
   else
     {
        /* FIXME: If you see that assert, please notify us and we
           will clean our mess */
        assert(eina_list_count(it->content_objs) == 0);

        _item_text_realize(it, VIEW(it), &it->texts, NULL);
        it->content_objs = _item_content_realize(it, VIEW(it), &it->contents, NULL);
        _item_state_realize(it, VIEW(it), &it->states, NULL);
        if (it->flipped)
          {
             edje_object_signal_emit(VIEW(it), "elm,state,flip,enabled", "elm");
             it->content_objs = _item_flips_realize(it, VIEW(it), &it->contents);
          }

        if (!it->item->mincalcd)
          {
             Evas_Coord mw = -1, mh = -1;

             if (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
               elm_coords_finger_size_adjust(1, &mw, 1, &mh);
             if (it->wd->mode == ELM_LIST_COMPRESS) mw = it->wd->prev_viewport_w;
             edje_object_size_min_restricted_calc(VIEW(it), &mw, &mh, mw,
                                                  mh);
             if (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
               elm_coords_finger_size_adjust(1, &mw, 1, &mh);
             it->item->w = it->item->minw = mw;
             it->item->h = it->item->minh = mh;
             it->item->mincalcd = EINA_TRUE;

             if ((!it->wd->group_item_width) && (it->group))
               {
                  it->wd->group_item_width = mw;
                  it->wd->group_item_height = mh;
               }
             else if ((!it->wd->item_width) && (it->item->type == ELM_GENLIST_ITEM_NONE))
               {
                  it->wd->item_width = mw;
                  it->wd->item_height = mh;
               }
          }
        if (!calc) evas_object_show(VIEW(it));
     }

   if (it->tooltip.content_cb)
     {
        elm_widget_item_tooltip_content_cb_set(it,
                                               it->tooltip.content_cb,
                                               it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, it->tooltip.style);
        elm_widget_item_tooltip_window_mode_set(it, it->tooltip.free_size);
     }

   if (it->mouse_cursor)
     elm_widget_item_cursor_set(it, it->mouse_cursor);

   it->realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;

   if (itc) _item_cache_free(itc);
   //evas_event_thaw(evas_object_evas_get(it->wd->obj));
   //evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
   if (!calc)
     {
        if (it->item->tree_effect_hideme)
          {
             if (it->wd->move_effect_mode != ELM_GENLIST_TREE_EFFECT_NONE)
               edje_object_signal_emit(VIEW(it), "elm,state,hide", "");
             it->item->tree_effect_hideme = EINA_FALSE;
          }
        evas_object_smart_callback_call(WIDGET(it), SIG_REALIZED, it);
     }

   if ((!calc) && (it->wd->decorate_mode) && (it->item->type != ELM_GENLIST_ITEM_GROUP))
     {
        if (it->itc->decorate_all_item_style)
          {
             if (!it->edit_obj) _decorate_mode_item_realize(it, EINA_FALSE);
             edje_object_message_signal_process(it->edit_obj);
          }
     }
   edje_object_message_signal_process(VIEW(it));
}

static void
_item_unrealize_cb(Elm_Gen_Item *it)
{
   if (it->item->nocache_once || it->item->nocache)
     {
        evas_object_del(VIEW(it));
        VIEW(it) = NULL;
        if (it->spacer)
          {
             evas_object_del(it->spacer);
             it->spacer = NULL;
          }
     }
   else
     {
        edje_object_mirrored_set(VIEW(it),
                                 elm_widget_mirrored_get(WIDGET(it)));
        edje_object_scale_set(VIEW(it),
                              elm_widget_scale_get(WIDGET(it))
                              * _elm_config->scale);
        _item_cache_add(it);
     }

   _mode_item_unrealize(it);
   it->states = NULL;
   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;
   if (it->wd->decorate_mode) _decorate_mode_item_unrealize(it);
}

static Eina_Bool
_item_block_recalc(Item_Block *itb,
                   int         in,
                   Eina_Bool   qadd)
{
   const Eina_List *l;
   Elm_Gen_Item *it;
   Evas_Coord minw = 0, minh = 0;
   Eina_Bool showme = EINA_FALSE, changed = EINA_FALSE;
   Evas_Coord y = 0;

   //evas_event_freeze(evas_object_evas_get(itb->wd->obj));
   itb->num = in;
   EINA_LIST_FOREACH(itb->items, l, it)
     {
        if (it->generation < it->wd->generation) continue;
        showme |= it->item->showme;
        if (!itb->realized)
          {
             if (qadd)
               {
                  if (!it->item->mincalcd) changed = EINA_TRUE;
                  if (changed)
                    {
                       _item_realize(it, in, EINA_TRUE);
                       _elm_genlist_item_unrealize(it, EINA_TRUE);
                    }
               }
             else
               {
                  _item_realize(it, in, EINA_TRUE);
                  _elm_genlist_item_unrealize(it, EINA_TRUE);
               }
          }
        else
          {
             if (!it->item->mincalcd) changed = EINA_TRUE;
             _item_realize(it, in, EINA_FALSE);
          }
        minh += it->item->minh;
        if (minw < it->item->minw) minw = it->item->minw;
        in++;
        it->x = 0;
        it->y = y;
        y += it->item->h;
     }
   if (changed) itb->wd->pan_changed = changed;
   itb->minw = minw;
   itb->minh = minh;
   itb->changed = EINA_FALSE;
   itb->position_update = EINA_FALSE;
   //evas_event_thaw(evas_object_evas_get(itb->wd->obj));
   //evas_event_thaw_eval(evas_object_evas_get(itb->wd->obj));
   return showme;
}

static void
_item_block_realize(Item_Block *itb)
{
   if (itb->realized) return;
   itb->realized = EINA_TRUE;
   itb->want_unrealize = EINA_FALSE;
}

static void
_item_block_unrealize(Item_Block *itb)
{
   const Eina_List *l;
   Elm_Gen_Item *it;
   Eina_Bool dragging = EINA_FALSE;

   if (!itb->realized) return;
   evas_event_freeze(evas_object_evas_get(itb->wd->obj));
   EINA_LIST_FOREACH(itb->items, l, it)
     {
        if (itb->must_recalc || !it->group)
          {
             if (it->dragging)
               {
                  dragging = EINA_TRUE;
                  it->want_unrealize = EINA_TRUE;
               }
             else
               _elm_genlist_item_unrealize(it, EINA_FALSE);
          }
     }
   if (!dragging)
     {
        itb->realized = EINA_FALSE;
        itb->want_unrealize = EINA_TRUE;
     }
   else
     itb->want_unrealize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(itb->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(itb->wd->obj));
}

static int
_get_space_for_reorder_item(Elm_Gen_Item *it)
{
   Evas_Coord rox, roy, row, roh, oy, oh;
   Eina_Bool top = EINA_FALSE;
   Elm_Gen_Item *reorder_it = it->wd->reorder_it;
   if (!reorder_it) return 0;

   evas_object_geometry_get(it->wd->pan_smart, NULL, &oy, NULL, &oh);
   evas_object_geometry_get(it->wd->VIEW(reorder_it), &rox, &roy, &row, &roh);

   if ((it->wd->reorder_start_y < it->item->block->y) &&
       (roy - oy + (roh / 2) >= it->item->block->y - it->wd->pan_y))
     {
        it->item->block->reorder_offset = it->wd->reorder_it->item->h * -1;
        if (it->item->block->count == 1)
          it->wd->reorder_rel = it;
     }
   else if ((it->wd->reorder_start_y >= it->item->block->y) &&
            (roy - oy + (roh / 2) <= it->item->block->y - it->wd->pan_y))
     {
        it->item->block->reorder_offset = it->wd->reorder_it->item->h;
     }
   else
     it->item->block->reorder_offset = 0;

   it->item->scrl_y += it->item->block->reorder_offset;

   top = (ELM_RECTS_INTERSECT(it->item->scrl_x, it->item->scrl_y, it->item->w, it->item->h,
                              rox, roy + (roh / 2), row, 1));
   if (top)
     {
        it->wd->reorder_rel = it;
        it->item->scrl_y += it->wd->reorder_it->item->h;
        return it->wd->reorder_it->item->h;
     }
   else
     return 0;
}

static Eina_Bool
_reorder_move_animator_cb(void *data)
{
   Elm_Gen_Item *it = data;
   Eina_Bool down = EINA_FALSE;
   double t;
   int y, dy = it->item->h / 10 * _elm_config->scale, diff;

   t = ((0.0 > (t = ecore_loop_time_get()-it->wd->start_time)) ? 0.0 : t);

   if (t <= REORDER_EFFECT_TIME) y = (1 * sin((t / REORDER_EFFECT_TIME) * (M_PI / 2)) * dy);
   else y = dy;

   diff = abs(it->item->old_scrl_y - it->item->scrl_y);
   if (diff > it->item->h) y = diff / 2;

   if (it->item->old_scrl_y < it->item->scrl_y)
     {
        it->item->old_scrl_y += y;
        down = EINA_TRUE;
     }
   else if (it->item->old_scrl_y > it->item->scrl_y)
     {
        it->item->old_scrl_y -= y;
        down = EINA_FALSE;
     }

   if (it->edit_obj)
     _item_position(it, it->edit_obj, it->item->scrl_x, it->item->old_scrl_y);
   else
     _item_position(it, VIEW(it), it->item->scrl_x, it->item->old_scrl_y);
   _group_items_recalc(it->wd);

   if ((it->wd->reorder_pan_move) ||
       (down && it->item->old_scrl_y >= it->item->scrl_y) ||
       (!down && it->item->old_scrl_y <= it->item->scrl_y))
     {
        it->item->old_scrl_y = it->item->scrl_y;
        it->item->move_effect_enabled = EINA_FALSE;
        it->wd->reorder_move_animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_item_position(Elm_Gen_Item *it,
               Evas_Object      *view,
               Evas_Coord        it_x,
               Evas_Coord        it_y)
{
   if (!it) return;
   if (!view) return;

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   evas_object_resize(view, it->item->w, it->item->h);
   evas_object_move(view, it_x, it_y);
   evas_object_show(view);
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
}

static void
_item_block_position(Item_Block *itb,
                     int         in)
{
   const Eina_List *l;
   Elm_Gen_Item *it;
   Elm_Gen_Item *git;
   Evas_Coord y = 0, ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   Eina_Bool vis = EINA_FALSE;

   evas_event_freeze(evas_object_evas_get(itb->wd->obj));
   evas_object_geometry_get(itb->wd->pan_smart, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(itb->wd->obj), &cvx, &cvy,
                            &cvw, &cvh);
   EINA_LIST_FOREACH(itb->items, l, it)
     {
        if (it->generation < it->wd->generation) continue;
        else if (it->wd->reorder_it == it) continue;
        it->x = 0;
        it->y = y;
        it->item->w = itb->w;
        it->item->scrl_x = itb->x + it->x - it->wd->pan_x + ox;
        it->item->scrl_y = itb->y + it->y - it->wd->pan_y + oy;

        vis = (ELM_RECTS_INTERSECT(it->item->scrl_x, it->item->scrl_y, it->item->w, it->item->h,
                                   cvx, cvy, cvw, cvh));
        if (!it->group)
          {
             if ((itb->realized) && (!it->realized))
               {
                  if (vis) _item_realize(it, in, EINA_FALSE);
               }
             if (it->realized)
               {
                  if (vis || it->dragging)
                    {
                       if (it->wd->reorder_mode)
                         y += _get_space_for_reorder_item(it);
                       git = it->item->group_item;
                       if (git)
                         {
                            if (git->item->scrl_y < oy)
                              git->item->scrl_y = oy;
                            if ((git->item->scrl_y + git->item->h) > (it->item->scrl_y + it->item->h))
                              git->item->scrl_y = (it->item->scrl_y + it->item->h) - git->item->h;
                            git->item->want_realize = EINA_TRUE;
                         }
                       if ((it->wd->reorder_it) && (it->item->old_scrl_y != it->item->scrl_y))
                         {
                            if (!it->item->move_effect_enabled)
                              {
                                 it->item->move_effect_enabled = EINA_TRUE;
                                 it->wd->reorder_move_animator =
                                    ecore_animator_add(
                                       _reorder_move_animator_cb, it);
                              }
                         }
                       if (!it->item->move_effect_enabled)
                         {
                            if ((it->wd->decorate_mode) && (it->itc->decorate_all_item_style))
                              _decorate_mode_item_position(it, it->item->scrl_x,
                                                       it->item->scrl_y);
                            else
                              {
                                 if (!it->wd->tree_effect_enabled ||
                                     (it->wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_NONE) ||
                                     ((it->wd->move_effect_mode != ELM_GENLIST_TREE_EFFECT_NONE) &&
                                     (it->item->old_scrl_y == it->item->scrl_y)))
                                   {
                                      if (it->item->mode_view)
                                        _item_position(it, it->item->mode_view,
                                                       it->item->scrl_x,
                                                       it->item->scrl_y);
                                      else
                                        _item_position(it, VIEW(it), it->item->scrl_x,
                                                       it->item->scrl_y);
                                   }
                              }
                            it->item->old_scrl_y = it->item->scrl_y;
                         }
                    }
                  else
                    {
                       if (!it->wd->tree_effect_animator)
                         _elm_genlist_item_unrealize(it, EINA_FALSE);
                    }
               }
             in++;
          }
        else
          {
             if (vis) it->item->want_realize = EINA_TRUE;
          }
        y += it->item->h;
     }
   evas_event_thaw(evas_object_evas_get(itb->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(itb->wd->obj));
}

static void
_group_items_recalc(void *data)
{
   Widget_Data *wd = data;
   Eina_List *l;
   Elm_Gen_Item *git;

   evas_event_freeze(evas_object_evas_get(wd->obj));
   EINA_LIST_FOREACH(wd->group_items, l, git)
     {
        if (git->item->want_realize)
          {
             if (!git->realized)
               _item_realize(git, 0, EINA_FALSE);
             evas_object_resize(VIEW(git), wd->minw, git->item->h);
             evas_object_move(VIEW(git), git->item->scrl_x, git->item->scrl_y);
             evas_object_show(VIEW(git));
             evas_object_raise(VIEW(git));
          }
        else if (!git->item->want_realize && git->realized)
          {
             if (!git->dragging)
               _elm_genlist_item_unrealize(git, EINA_FALSE);
          }
     }
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static Eina_Bool
_must_recalc_idler(void *data)
{
   Widget_Data *wd = data;
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
   wd->must_recalc_idler = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_scroll_item(Widget_Data *wd)
{
   Elm_Gen_Item *it = NULL;
   Evas_Coord gith = 0;
   Evas_Coord ow, oh, dx = 0, dy = 0, dw = 0, dh = 0;
   if (!wd->show_item) return;

   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &ow, &oh);
   it = wd->show_item;
   dx = it->x + it->item->block->x;
   dy = it->y + it->item->block->y;
   dw = it->item->block->w;
   dh = oh;
   switch (wd->scrollto_type)
     {
      case ELM_GENLIST_ITEM_SCROLLTO_TOP:
         if (it->item->group_item) gith = it->item->group_item->item->h;
         dy -= gith;
         break;
      case ELM_GENLIST_ITEM_SCROLLTO_MIDDLE:
         dy += ((it->item->h / 2) - (oh / 2));
         break;
      case ELM_GENLIST_ITEM_SCROLLTO_IN:
      default:
         if ((wd->expanded_item) &&
             ((wd->show_item->y + wd->show_item->item->block->y + wd->show_item->item->h)
              - (wd->expanded_item->y + wd->expanded_item->item->block->y) > oh))
           {
              it = wd->expanded_item;
              if (it->item->group_item) gith = it->item->group_item->item->h;
              dx = it->x + it->item->block->x;
              dy = it->y + it->item->block->y - gith;
              dw = it->item->block->w;
           }
         else
           {
              if ((it->item->group_item) && (wd->pan_y > (it->y + it->item->block->y)))
                gith = it->item->group_item->item->h;
              dy -= gith;
              dh = it->item->h;
           }
         break;
     }
   if (wd->bring_in)
     elm_smart_scroller_region_bring_in(wd->scr, dx, dy, dw, dh);
   else
     elm_smart_scroller_child_region_show(wd->scr, dx, dy, dw, dh);

   it->item->showme = EINA_FALSE;
   wd->show_item = NULL;
   wd->auto_scroll_enabled = EINA_FALSE;
   wd->check_scroll = EINA_FALSE;
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Item_Block *itb, *chb = NULL;
   Evas_Coord minw = -1, minh = 0, y = 0, ow, dy = 0, vw;
   Evas_Coord pan_w = 0, pan_h = 0;
   int in = 0;
   Eina_Bool minw_change = EINA_FALSE;
   Eina_Bool did_must_recalc = EINA_FALSE;
   if (!wd) return;

   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &ow, &wd->h);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, NULL);

   if (wd->w != ow)
     wd->w = ow;

   evas_event_freeze(evas_object_evas_get(wd->obj));
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Eina_Bool showme = EINA_FALSE;

        itb->num = in;
        showme = itb->showme;
        itb->showme = EINA_FALSE;
        if (chb)
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
        if ((itb->changed) || ((itb->must_recalc) && (!did_must_recalc)))
          {
             if (itb->must_recalc)
               {
                  Eina_List *l;
                  Elm_Gen_Item *it;
                  EINA_LIST_FOREACH(itb->items, l, it)
                    if (it->item->mincalcd) it->item->mincalcd = EINA_FALSE;
                  itb->changed = EINA_TRUE;
                  if (itb->must_recalc) did_must_recalc = EINA_TRUE;
                  if (itb->realized) _item_block_unrealize(itb);
                  itb->must_recalc = EINA_FALSE;
               }
             showme = _item_block_recalc(itb, in, EINA_FALSE);
             chb = itb;
          }
        itb->y = y;
        itb->x = 0;
        minh += itb->minh;
        if (minw == -1) minw = itb->minw;
        else if ((!itb->must_recalc) && (minw < itb->minw))
          {
             minw = itb->minw;
             minw_change = EINA_TRUE;
          }
        if (minw > vw)
          {
             minw = vw;
             minw_change = EINA_TRUE;
          }
        itb->w = minw;
        itb->h = itb->minh;
        y += itb->h;
        in += itb->count;
        if ((showme) && (wd->show_item) && (!wd->show_item->item->queued))
          wd->check_scroll = EINA_TRUE;
     }
   if (minw_change)
     {
        EINA_INLIST_FOREACH(wd->blocks, itb)
          {
             itb->minw = minw;
             itb->w = itb->minw;
          }
     }
   if ((chb) && (EINA_INLIST_GET(chb)->next))
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(chb)->next, itb)
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
     }
   wd->realminw = minw;
   if (minw < wd->w) minw = wd->w;
   if ((minw != wd->minw) || (minh != wd->minh))
     {
        wd->minw = minw;
        wd->minh = minh;
        evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
        _sizing_eval(wd->obj);
        if ((wd->anchor_item) && (wd->anchor_item->item->block) && (!wd->auto_scroll_enabled))
          {
             Elm_Gen_Item *it;
             Evas_Coord it_y;

             it = wd->anchor_item;
             it_y = wd->anchor_y;
             elm_smart_scroller_child_pos_set(wd->scr, wd->pan_x,
                                              it->item->block->y + it->y + it_y);
             wd->anchor_item = it;
             wd->anchor_y = it_y;
          }
     }
   if (did_must_recalc)
     {
        if (!wd->must_recalc_idler)
          wd->must_recalc_idler = ecore_idler_add(_must_recalc_idler, wd);
     }
   if (wd->check_scroll)
     {
        _pan_child_size_get(wd->pan_smart, &pan_w, &pan_h);
        if (EINA_INLIST_GET(wd->show_item) == wd->items->last)
          wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_IN;
       switch (wd->scrollto_type)
          {
           case ELM_GENLIST_ITEM_SCROLLTO_TOP:
              dy = wd->h;
              break;
           case ELM_GENLIST_ITEM_SCROLLTO_MIDDLE:
              dy = wd->h / 2;
              break;
           case ELM_GENLIST_ITEM_SCROLLTO_IN:
           default:
              dy = 0;
              break;
          }
        if (wd->show_item)
          {
             if ((pan_w > (wd->show_item->x + wd->show_item->item->block->x)) &&
                 (pan_h > (wd->show_item->y + wd->show_item->item->block->y + dy)))
               {
                  _scroll_item(wd);
               }
          }
     }
   wd->calc_job = NULL;
   evas_object_smart_changed(wd->pan_smart);
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_update_job(void *data)
{
   Widget_Data *wd = data;
   Eina_List *l2;
   Item_Block *itb;
   int num, num0;
   Eina_Bool position = EINA_FALSE, recalc = EINA_FALSE;
   if (!wd) return;
   wd->update_job = NULL;
   num = 0;

   evas_event_freeze(evas_object_evas_get(wd->obj));
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Evas_Coord itminw, itminh;
        Elm_Gen_Item *it;

        if (!itb->updateme)
          {
             num += itb->count;
             if (position)
               _item_block_position(itb, num);
             continue;
          }
        num0 = num;
        recalc = EINA_FALSE;
        EINA_LIST_FOREACH(itb->items, l2, it)
          {
             if (it->item->updateme)
               {
                  itminw = it->item->minw;
                  itminh = it->item->minh;

                  it->item->updateme = EINA_FALSE;
                  if (it->realized)
                    {
                       _elm_genlist_item_unrealize(it, EINA_FALSE);
                       _item_realize(it, num, EINA_FALSE);
                       position = EINA_TRUE;
                    }
                  else
                    {
                       _item_realize(it, num, EINA_TRUE);
                       _elm_genlist_item_unrealize(it, EINA_TRUE);
                    }
                  if ((it->item->minw != itminw) || (it->item->minh != itminh))
                    recalc = EINA_TRUE;
               }
             num++;
          }
        itb->updateme = EINA_FALSE;
        if (recalc)
          {
             position = EINA_TRUE;
             itb->changed = EINA_TRUE;
             _item_block_recalc(itb, num0, EINA_FALSE);
             _item_block_position(itb, num0);
          }
     }
   if (position)
     {
        if (wd->calc_job) ecore_job_del(wd->calc_job);
        wd->calc_job = ecore_job_add(_calc_job, wd);
     }
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

static void
_pan_set(Evas_Object *obj,
         Evas_Coord   x,
         Evas_Coord   y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Item_Block *itb;

   if (!sd) return;
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;

   EINA_INLIST_FOREACH(sd->wd->blocks, itb)
     {
        if ((itb->y + itb->h) > y)
          {
             Elm_Gen_Item *it;
             Eina_List *l2;

             EINA_LIST_FOREACH(itb->items, l2, it)
               {
                  if ((itb->y + it->y) >= y)
                    {
                       sd->wd->anchor_item = it;
                       sd->wd->anchor_y = -(itb->y + it->y - y);
                       goto done;
                    }
               }
          }
     }
done:
   if (!sd->wd->reorder_move_animator) evas_object_smart_changed(obj);
}

static void
_pan_get(Evas_Object *obj,
         Evas_Coord  *x,
         Evas_Coord  *y)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_max_get(Evas_Object *obj,
             Evas_Coord  *x,
             Evas_Coord  *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   if (!sd) return;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = sd->wd->minw - ow;
   if (ow < 0) ow = 0;
   oh = sd->wd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

static void
_pan_min_get(Evas_Object *obj __UNUSED__,
             Evas_Coord  *x,
             Evas_Coord  *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

static void
_pan_child_size_get(Evas_Object *obj,
                    Evas_Coord  *w,
                    Evas_Coord  *h)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = ELM_NEW(Pan);
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}

static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   if (sd->resize_job)
     {
        ecore_job_del(sd->resize_job);
        sd->resize_job = NULL;
     }
   _pan_sc.del(obj);
}

static void
_pan_resize_job(void *data)
{
   Pan *sd = data;
   if (!sd) return;
   _sizing_eval(sd->wd->obj);
   sd->resize_job = NULL;
}

static void
_pan_resize(Evas_Object *obj,
            Evas_Coord   w,
            Evas_Coord   h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   if (!sd) return;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if ((sd->wd->mode == ELM_LIST_COMPRESS) && (ow != w))
     {
        /* fix me later */
        if (sd->resize_job) ecore_job_del(sd->resize_job);
        sd->resize_job = ecore_job_add(_pan_resize_job, sd);
     }
   sd->wd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(obj);
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = NULL;
/* OLD
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
 */
}

static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Item_Block *itb;
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   int in = 0;
   Elm_Gen_Item *git;
   Eina_List *l;

   if (!sd) return;
   evas_event_freeze(evas_object_evas_get(obj));

   if (sd->wd->pan_changed)
     {
        _calc_job(sd->wd);
        sd->wd->pan_changed = EINA_FALSE;
     }

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(obj), &cvx, &cvy, &cvw, &cvh);
   EINA_LIST_FOREACH(sd->wd->group_items, l, git)
     {
        git->item->want_realize = EINA_FALSE;
     }

   if (sd->wd->tree_effect_enabled && (sd->wd->move_effect_mode != ELM_GENLIST_TREE_EFFECT_NONE))
     {
        if (!sd->wd->tree_effect_animator)
          {
             _item_tree_effect_before(sd->wd->expanded_item);
             evas_object_raise(sd->wd->alpha_bg);
             evas_object_show(sd->wd->alpha_bg);
             sd->wd->start_time = ecore_time_get();
             sd->wd->tree_effect_animator = ecore_animator_add(_tree_effect_animator_cb, sd->wd);
          }
     }

   EINA_INLIST_FOREACH(sd->wd->blocks, itb)
     {
        itb->w = sd->wd->minw;
        if (ELM_RECTS_INTERSECT(itb->x - sd->wd->pan_x + ox,
                                itb->y - sd->wd->pan_y + oy,
                                itb->w, itb->h,
                                cvx, cvy, cvw, cvh))
          {
             if ((!itb->realized) || (itb->changed))
               _item_block_realize(itb);
             _item_block_position(itb, in);
          }
        else
          {
             if (itb->realized) _item_block_unrealize(itb);
          }
        in += itb->count;
     }
   if ((!sd->wd->reorder_it) || (sd->wd->reorder_pan_move))
      _group_items_recalc(sd->wd);
   if ((sd->wd->reorder_mode) && (sd->wd->reorder_it))
     {
        if (sd->wd->pan_y != sd->wd->reorder_old_pan_y)
           sd->wd->reorder_pan_move = EINA_TRUE;
        else sd->wd->reorder_pan_move = EINA_FALSE;
        evas_object_raise(sd->wd->VIEW(reorder_it));
        sd->wd->reorder_old_pan_y = sd->wd->pan_y;
        sd->wd->start_time = ecore_loop_time_get();
     }

   if (!sd->wd->tree_effect_enabled || (sd->wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_NONE))
     _item_auto_scroll(sd->wd);

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_pan_move(Evas_Object *obj,
          Evas_Coord   x __UNUSED__,
          Evas_Coord   y __UNUSED__)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;

   sd->wd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(obj);
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = NULL;
}

static void
_hold_on(void        *data __UNUSED__,
         Evas_Object *obj,
         void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void        *data __UNUSED__,
          Evas_Object *obj,
          void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void        *data __UNUSED__,
           Evas_Object *obj,
           void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void        *data __UNUSED__,
            Evas_Object *obj,
            void        *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_scr_anim_start(void        *data,
                Evas_Object *obj __UNUSED__,
                void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scr_anim_stop(void        *data,
               Evas_Object *obj __UNUSED__,
               void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scr_drag_start(void            *data,
                Evas_Object     *obj __UNUSED__,
                void            *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scr_drag_stop(void            *data,
               Evas_Object     *obj __UNUSED__,
               void            *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_edge_left(void        *data,
           Evas_Object *scr __UNUSED__,
           void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_EDGE_LEFT, NULL);
}

static void
_edge_right(void        *data,
            Evas_Object *scr __UNUSED__,
            void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_EDGE_RIGHT, NULL);
}

static void
_edge_top(void        *data,
          Evas_Object *scr __UNUSED__,
          void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_EDGE_TOP, NULL);
}

static void
_edge_bottom(void        *data,
             Evas_Object *scr __UNUSED__,
             void        *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   evas_object_smart_callback_call(obj, SIG_EDGE_BOTTOM, NULL);
}

static void
_mode_item_realize(Elm_Gen_Item *it)
{
   char buf[1024];

   if ((it->item->mode_view) || (it->generation < it->wd->generation)) return;

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   it->item->mode_view = edje_object_add(evas_object_evas_get(WIDGET(it)));
   edje_object_scale_set(it->item->mode_view,
                         elm_widget_scale_get(WIDGET(it)) *
                         _elm_config->scale);
   evas_object_smart_member_add(it->item->mode_view, it->wd->pan_smart);
   elm_widget_sub_object_add(WIDGET(it), it->item->mode_view);

   strncpy(buf, "item", sizeof(buf));
   if (it->wd->mode == ELM_LIST_COMPRESS)
     strncat(buf, "_compress", sizeof(buf) - strlen(buf));

   if (it->item->order_num_in & 0x1) strncat(buf, "_odd", sizeof(buf) - strlen(buf));
   strncat(buf, "/", sizeof(buf) - strlen(buf));
   strncat(buf, it->itc->decorate_item_style, sizeof(buf) - strlen(buf));

   _elm_theme_object_set(WIDGET(it), it->item->mode_view, "genlist", buf,
                         elm_widget_style_get(WIDGET(it)));
   edje_object_mirrored_set(it->item->mode_view,
                            elm_widget_mirrored_get(WIDGET(it)));

   /* signal callback add */
   evas_object_event_callback_add(it->item->mode_view, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, it);
   evas_object_event_callback_add(it->item->mode_view, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, it);
   evas_object_event_callback_add(it->item->mode_view, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move, it);

   /* text_get, content_get, state_get */
   /* FIXME: If you see that assert, please notify us and we
      will clean our mess */
   assert(eina_list_count(it->item->mode_content_objs) == 0);

   _item_text_realize(it, it->item->mode_view, &it->item->mode_texts, NULL);
   it->item->mode_content_objs =
     _item_content_realize(it, it->item->mode_view,
                           &it->item->mode_contents, NULL);
   _item_state_realize(it, it->item->mode_view, &it->item->mode_states, NULL);

   edje_object_part_swallow(it->item->mode_view,
                            edje_object_data_get(it->item->mode_view, "mode_part"),
                            VIEW(it));

   it->want_unrealize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
}

static void
_mode_item_unrealize(Elm_Gen_Item *it)
{
   Widget_Data *wd = it->wd;
   Evas_Object *content;
   if (!it->item->mode_view) return;

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   elm_widget_stringlist_free(it->item->mode_texts);
   it->item->mode_texts = NULL;
   elm_widget_stringlist_free(it->item->mode_contents);
   it->item->mode_contents = NULL;
   elm_widget_stringlist_free(it->item->mode_states);

   EINA_LIST_FREE(it->item->mode_content_objs, content)
     evas_object_del(content);

   edje_object_part_unswallow(it->item->mode_view, VIEW(it));
   evas_object_smart_member_add(VIEW(it), wd->pan_smart);
   evas_object_del(it->item->mode_view);
   it->item->mode_view = NULL;

   if (wd->mode_item == it)
     wd->mode_item = NULL;
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));
}

static void
_item_mode_set(Elm_Gen_Item *it)
{
   if (!it) return;
   Widget_Data *wd = it->wd;
   if (!wd) return;
   char buf[1024];

   wd->mode_item = it;
   it->item->nocache_once = EINA_TRUE;

   if (wd->scr_hold_timer)
     {
        ecore_timer_del(wd->scr_hold_timer);
        wd->scr_hold_timer = NULL;
     }
   elm_smart_scroller_hold_set(wd->scr, EINA_TRUE);
   wd->scr_hold_timer = ecore_timer_add(0.1, _scr_hold_timer_cb, wd);

   evas_event_freeze(evas_object_evas_get(it->wd->obj));
   _mode_item_realize(it);
   if (it->item->group_item)
     evas_object_raise(it->item->VIEW(group_item));
   _item_position(it, it->item->mode_view, it->item->scrl_x, it->item->scrl_y);
   evas_event_thaw(evas_object_evas_get(it->wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(it->wd->obj));

   snprintf(buf, sizeof(buf), "elm,state,%s,active", wd->decorate_type);
   edje_object_signal_emit(it->item->mode_view, buf, "elm");
}

static void
_item_mode_unset(Widget_Data *wd)
{
   if (!wd) return;
   if (!wd->mode_item) return;
   char buf[1024], buf2[1024];
   Elm_Gen_Item *it;

   it = wd->mode_item;
   it->item->nocache_once = EINA_TRUE;

   snprintf(buf, sizeof(buf), "elm,state,%s,passive", wd->decorate_type);
   snprintf(buf2, sizeof(buf2), "elm,state,%s,passive,finished", wd->decorate_type);

   edje_object_signal_emit(it->item->mode_view, buf, "elm");
   edje_object_signal_callback_add(it->item->mode_view, buf2, "elm", _mode_finished_signal_cb, it);

   wd->mode_item = NULL;
}

static void
_decorate_mode_item_position(Elm_Gen_Item *it, int itx, int ity)
{
   if ((!it) || (!it->wd->decorate_mode)) return;
   evas_object_resize(it->edit_obj, it->item->w, it->item->h);
   evas_object_move(it->edit_obj, itx, ity);
}

static void
_decorate_mode_item_realize(Elm_Gen_Item *it, Eina_Bool effect_on)
{
   char buf[1024];
   const char *stacking_even;
   const char *stacking;

   if ((!it) || (it->item->decorate_mode_item_realized) ||
       (it->generation < it->wd->generation))
     return;

   it->edit_obj = edje_object_add(evas_object_evas_get(WIDGET(it)));
   edje_object_scale_set(it->edit_obj, elm_widget_scale_get(WIDGET(it)) *
                         _elm_config->scale);
   evas_object_smart_member_add(it->edit_obj, it->wd->pan_smart);
   elm_widget_sub_object_add(WIDGET(it), it->edit_obj);

   if (it->item->type & ELM_GENLIST_ITEM_TREE)
      strncpy(buf, "tree", sizeof(buf));
   else strncpy(buf, "item", sizeof(buf));
   if (it->wd->mode == ELM_LIST_COMPRESS)
      strncat(buf, "_compress", sizeof(buf) - strlen(buf));

   strncat(buf, "/", sizeof(buf) - strlen(buf));
   strncat(buf, it->itc->decorate_all_item_style, sizeof(buf) - strlen(buf));

   _elm_theme_object_set(WIDGET(it),  it->edit_obj, "genlist", buf,
                         elm_widget_style_get(WIDGET(it)));

   stacking_even = edje_object_data_get(VIEW(it), "stacking_even");
   if (!stacking_even) stacking_even = "above";
   it->item->stacking_even = !!strcmp("above", stacking_even);

   stacking = edje_object_data_get(VIEW(it), "stacking");
   if (!stacking) stacking = "yes";
   it->item->nostacking = !!strcmp("yes", stacking);

   edje_object_mirrored_set(it->edit_obj,
                            elm_widget_mirrored_get(WIDGET(it)));

   _elm_genlist_item_odd_even_update(it);
   _elm_genlist_item_state_update(it, NULL);

   if (effect_on) edje_object_signal_emit(it->edit_obj, "elm,state,edit,enabled,effect", "elm");
   else edje_object_signal_emit(it->edit_obj, "elm,state,edit,enabled", "elm");

   _item_mouse_callbacks_del(it, VIEW(it));
   _item_mouse_callbacks_add(it, it->edit_obj);

   _item_text_realize(it, it->edit_obj, &it->item->edit_texts, NULL);
   if (it->flipped)  edje_object_signal_emit(it->edit_obj, "elm,state,flip,enabled", "elm");
   it->item->edit_content_objs =
     _item_content_realize(it, it->edit_obj, &it->item->edit_contents, NULL);
   _item_state_realize(it, it->edit_obj, &it->item->edit_states, NULL);
   edje_object_part_swallow(it->edit_obj, "elm.swallow.edit.content", VIEW(it));

   _decorate_mode_item_position(it, it->item->scrl_x, it->item->scrl_y);
   evas_object_show(it->edit_obj);

   it->item->decorate_mode_item_realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;
}

static void
_decorate_mode_item_unrealize(Elm_Gen_Item *it)
{
   Evas_Object *icon;
   if ((!it) || (!it->item->decorate_mode_item_realized)) return;

   edje_object_part_unswallow(it->edit_obj, VIEW(it));
   evas_object_smart_member_add(VIEW(it), it->wd->pan_smart);
   elm_widget_sub_object_add(WIDGET(it), VIEW(it));
   _elm_genlist_item_odd_even_update(it);
   _elm_genlist_item_state_update(it, NULL);

   evas_object_del(it->edit_obj);
   it->edit_obj = NULL;
   elm_widget_stringlist_free(it->item->edit_texts);
   it->item->edit_texts = NULL;
   elm_widget_stringlist_free(it->item->edit_contents);
   it->item->edit_contents = NULL;
   elm_widget_stringlist_free(it->item->edit_states);
   it->item->edit_states = NULL;
   EINA_LIST_FREE(it->item->edit_content_objs, icon)
     evas_object_del(icon);
   edje_object_message_signal_process(it->edit_obj);
   _item_mouse_callbacks_del(it, it->edit_obj);
   _item_mouse_callbacks_add(it, VIEW(it));

   it->item->decorate_mode_item_realized = EINA_FALSE;
}

static void
_item_auto_scroll(Widget_Data *wd)
{
   if (!wd) return;
   Elm_Gen_Item  *tmp_item = NULL;

   if ((wd->expanded_item) && (wd->auto_scroll_enabled))
     {
        tmp_item = eina_list_data_get(eina_list_last(wd->expanded_item->item->items));
        if (!tmp_item) return;
        wd->show_item = tmp_item;
        wd->bring_in = EINA_TRUE;
        wd->scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_IN;
        if ((wd->show_item->item->queued) || (!wd->show_item->item->mincalcd))
          {
             wd->show_item->item->showme = EINA_TRUE;
             wd->auto_scroll_enabled = EINA_FALSE;
          }
        else
          _scroll_item(wd);
     }
}

EAPI Evas_Object *
elm_genlist_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;
   static Evas_Smart *smart = NULL;

   if (!smart)
     {
        static Evas_Smart_Class sc;

        evas_object_smart_clipped_smart_set(&_pan_sc);
        sc = _pan_sc;
        sc.name = "elm_genlist_pan";
        sc.version = EVAS_SMART_CLASS_VERSION;
        sc.add = _pan_add;
        sc.del = _pan_del;
        sc.resize = _pan_resize;
        sc.move = _pan_move;
        sc.calculate = _pan_calculate;
        if (!(smart = evas_smart_class_new(&sc))) return NULL;
     }

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "genlist");
   ELM_GEN_SETUP(wd);
   elm_widget_type_set(obj, "genlist");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_on_show_region_hook_set(obj, _show_region_hook, obj);
   elm_widget_translate_hook_set(obj, _translate_hook);

   wd->generation = 1;
   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "genlist", "base",
                                       elm_widget_style_get(obj));
   elm_smart_scroller_bounce_allow_set(wd->scr, EINA_FALSE,
                                       _elm_config->thumbscroll_bounce_enable);
   elm_widget_resize_object_set(obj, wd->scr);

   evas_object_smart_callback_add(wd->scr, "animate,start", _scr_anim_start, obj);
   evas_object_smart_callback_add(wd->scr, "animate,stop", _scr_anim_stop, obj);
   evas_object_smart_callback_add(wd->scr, "drag,start", _scr_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scr_drag_stop, obj);
   evas_object_smart_callback_add(wd->scr, "edge,left", _edge_left, obj);
   evas_object_smart_callback_add(wd->scr, "edge,right", _edge_right, obj);
   evas_object_smart_callback_add(wd->scr, "edge,top", _edge_top, obj);
   evas_object_smart_callback_add(wd->scr, "edge,bottom", _edge_bottom, obj);

   wd->obj = obj;
   wd->mode = ELM_LIST_SCROLL;
   wd->max_items_per_block = MAX_ITEMS_PER_BLOCK;
   wd->item_cache_max = wd->max_items_per_block * 2;
   wd->longpress_timeout = _elm_config->longpress_timeout;
   wd->highlight = EINA_TRUE;

   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   wd->pan_smart = evas_object_smart_add(e, smart);
   wd->pan = evas_object_smart_data_get(wd->pan_smart);
   wd->pan->wd = wd;

   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
                                     _pan_set, _pan_get, _pan_max_get,
                                     _pan_min_get, _pan_child_size_get);

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr),
                             &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

void
_item_select(Elm_Gen_Item *it)
{
   if ((it->generation < it->wd->generation) || (it->mode_set) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (it->wd->select_mode == ELM_OBJECT_SELECT_MODE_NONE))
     return;
   if (!it->selected)
     {
        it->selected = EINA_TRUE;
        it->wd->selected = eina_list_append(it->wd->selected, it);
     }
   else if (it->wd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS) return;

   evas_object_ref(WIDGET(it));
   it->walking++;
   it->wd->walking++;
   if (it->func.func) it->func.func((void *)it->func.data, WIDGET(it), it);
   if (it->generation == it->wd->generation)
     evas_object_smart_callback_call(WIDGET(it), SIG_SELECTED, it);
   it->walking--;
   it->wd->walking--;
   if ((it->wd->clear_me) && (!it->wd->walking))
     _elm_genlist_clear(WIDGET(it), EINA_TRUE);
   else
     {
        if ((!it->walking) && (it->generation < it->wd->generation))
          {
             if (!it->relcount)
               {
                  it->del_cb(it);
                  elm_widget_item_free(it);
               }
          }
        else
          it->wd->last_selected_item = (Elm_Object_Item *)it;
     }
   evas_object_unref(WIDGET(it));
}

static Evas_Object *
_item_content_get_hook(Elm_Gen_Item *it, const char *part)
{
   return edje_object_part_swallow_get(VIEW(it), part);
}

static void
_item_content_set_hook(Elm_Gen_Item *it, const char *part, Evas_Object *content)
{
   if (content && part)
     {
        it->content_objs = eina_list_append(it->content_objs, content);
        edje_object_part_swallow(VIEW(it), part, content);
     }
}

static Evas_Object *
_item_content_unset_hook(Elm_Gen_Item *it, const char *part)
{
   Evas_Object *obj;
   obj = edje_object_part_swallow_get(VIEW(it), part);
   if (!obj) return NULL;
   it->content_objs = eina_list_remove(it->content_objs, obj);
   edje_object_part_unswallow(VIEW(it), obj);
   return obj;
}

static const char *
_item_text_hook(Elm_Gen_Item *it, const char *part)
{
   if (!it->itc->func.text_get) return NULL;
   return edje_object_part_text_get(VIEW(it), part);
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   Eina_List *l;
   Evas_Object *obj;
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if (_it->generation < _it->wd->generation) return;

   if (_it->selected)
     elm_genlist_item_selected_set(it, EINA_FALSE);

   if (_it->realized)
     {
        if (elm_widget_item_disabled_get(it))
          {
             edje_object_signal_emit(VIEW(_it), "elm,state,disabled", "elm");
             if (_it->edit_obj)
               edje_object_signal_emit(_it->edit_obj, "elm,state,disabled", "elm");
          }
        else
          {
             edje_object_signal_emit(VIEW(_it), "elm,state,enabled", "elm");
             if (_it->edit_obj)
               edje_object_signal_emit(_it->edit_obj, "elm,state,enabled", "elm");
          }
        EINA_LIST_FOREACH(_it->content_objs, l, obj)
          elm_widget_disabled_set(obj, elm_widget_item_disabled_get(_it));
     }
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if ((_it->relcount > 0) || (_it->walking > 0))
     {
        elm_genlist_item_subitems_clear(it);
        if (_it->wd->show_item == _it) _it->wd->show_item = NULL;
        _elm_genlist_item_del_notserious(_it);
        if (_it->item->block)
          {
             if (_it->realized) _elm_genlist_item_unrealize(_it, EINA_FALSE);
             _it->item->block->changed = EINA_TRUE;
             if (_it->wd->calc_job) ecore_job_del(_it->wd->calc_job);
             _it->wd->calc_job = ecore_job_add(_calc_job, _it->wd);
          }
        if (_it->parent)
          {
             _it->parent->item->items =
                eina_list_remove(_it->parent->item->items, it);
             _it->parent = NULL;
          }
        return EINA_FALSE;
     }
   _item_del(_it);
   return EINA_TRUE;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   edje_object_signal_emit(VIEW(it), emission, source);
}

Elm_Gen_Item *
_elm_genlist_item_new(Widget_Data              *wd,
                      const Elm_Gen_Item_Class *itc,
                      const void               *data,
                      Elm_Gen_Item             *parent,
                      Evas_Smart_Cb             func,
                      const void               *func_data)
{
   Elm_Gen_Item *it;

   it = elm_widget_item_new(wd->obj, Elm_Gen_Item);
   if (!it) return NULL;
   it->wd = wd;
   it->generation = wd->generation;
   it->itc = itc;
   elm_genlist_item_class_ref((Elm_Genlist_Item_Class *)itc);
   it->base.data = data;
   it->parent = parent;
   it->func.func = func;
   it->func.data = func_data;
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_hook);
   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);

   /* TEMPORARY */
   it->sel_cb = (Ecore_Cb)_item_select;

   return it;
}

static Elm_Gen_Item *
_item_new(Widget_Data                  *wd,
          const Elm_Genlist_Item_Class *itc,
          const void                   *data,
          Elm_Gen_Item                 *parent,
          Elm_Genlist_Item_Type         type,
          Evas_Smart_Cb                 func,
          const void                   *func_data)
{
   Elm_Gen_Item *it, *it2;
   int depth = 0;

   it = _elm_genlist_item_new(wd, itc, data, parent, func, func_data);
   if (!it) return NULL;
   it->item = ELM_NEW(Elm_Gen_Item_Type);
   it->item->type = type;
   if (type & ELM_GENLIST_ITEM_GROUP) it->group++;
   it->item->expanded_depth = 0;
   ELM_GEN_ITEM_SETUP(it);
   if (it->parent)
     {
        if (it->parent->group)
          it->item->group_item = parent;
        else if (it->parent->item->group_item)
          it->item->group_item = it->parent->item->group_item;
     }
   for (it2 = it, depth = 0; it2->parent; it2 = it2->parent)
     {
        if (!it2->parent->group) depth += 1;
     }
   it->item->expanded_depth = depth;
   wd->item_count++;
   return it;
}

static Item_Block *
_item_block_new(Widget_Data *wd, Eina_Bool prepend)
{
   Item_Block *itb;

   itb = calloc(1, sizeof(Item_Block));
   if (!itb) return NULL;
   itb->wd = wd;
   if (prepend)
     {
        wd->blocks = eina_inlist_prepend(wd->blocks, EINA_INLIST_GET(itb));
        _item_block_position_update(wd->blocks, 0);
     }
   else
     {
        wd->blocks = eina_inlist_append(wd->blocks, EINA_INLIST_GET(itb));
        itb->position_update = EINA_TRUE;
        if (wd->blocks != EINA_INLIST_GET(itb))
          {
             itb->position = ((Item_Block *) (EINA_INLIST_GET(itb)->prev))->position + 1;
          }
        else
          {
             itb->position = 0;
          }
     }
   return itb;
}

static Eina_Bool
_item_block_add(Widget_Data *wd,
                Elm_Gen_Item *it)
{
   Item_Block *itb = NULL;

   if (!it->item->rel)
     {
newblock:
        if (it->item->rel)
          {
             itb = calloc(1, sizeof(Item_Block));
             if (!itb) return EINA_FALSE;
             itb->wd = wd;
             if (!it->item->rel->item->block)
               {
                  wd->blocks =
                    eina_inlist_append(wd->blocks, EINA_INLIST_GET(itb));
                  itb->items = eina_list_append(itb->items, it);
                  itb->position_update = EINA_TRUE;
                  it->position = eina_list_count(itb->items);
                  it->position_update = EINA_TRUE;

                  if (wd->blocks != EINA_INLIST_GET(itb))
                    {
                       itb->position = ((Item_Block *) (EINA_INLIST_GET(itb)->prev))->position + 1;
                    }
                  else
                    {
                       itb->position = 0;
                    }
               }
             else
               {
                  Eina_List *tmp;

                  tmp = eina_list_data_find_list(itb->items, it->item->rel);
                  if (it->item->before)
                    {
                       wd->blocks = eina_inlist_prepend_relative
                           (wd->blocks, EINA_INLIST_GET(itb),
                           EINA_INLIST_GET(it->item->rel->item->block));
                       itb->items =
                         eina_list_prepend_relative_list(itb->items, it, tmp);

                       /* Update index from where we prepended */
                       _item_position_update(eina_list_prev(tmp), it->item->rel->position);
                       _item_block_position_update(EINA_INLIST_GET(itb),
                                                   it->item->rel->item->block->position);
                    }
                  else
                    {
                       wd->blocks = eina_inlist_append_relative
                           (wd->blocks, EINA_INLIST_GET(itb),
                           EINA_INLIST_GET(it->item->rel->item->block));
                       itb->items =
                         eina_list_append_relative_list(itb->items, it, tmp);

                       /* Update block index from where we appended */
                       _item_position_update(eina_list_next(tmp), it->item->rel->position + 1);
                       _item_block_position_update(EINA_INLIST_GET(itb),
                                                   it->item->rel->item->block->position + 1);
                    }
               }
          }
        else
          {
             if (it->item->before)
               {
                  if (wd->blocks)
                    {
                       itb = (Item_Block *)(wd->blocks);
                       if (itb->count >= wd->max_items_per_block)
                         {
                            itb = _item_block_new(wd, EINA_TRUE);
                            if (!itb) return EINA_FALSE;
                         }
                    }
                  else
                    {
                       itb = _item_block_new(wd, EINA_TRUE);
                       if (!itb) return EINA_FALSE;
                    }
                  itb->items = eina_list_prepend(itb->items, it);

                  _item_position_update(itb->items, 0);
               }
             else
               {
                  if (wd->blocks)
                    {
                       itb = (Item_Block *)(wd->blocks->last);
                       if (itb->count >= wd->max_items_per_block)
                         {
                            itb = _item_block_new(wd, EINA_FALSE);
                            if (!itb) return EINA_FALSE;
                         }
                    }
                  else
                    {
                       itb = _item_block_new(wd, EINA_FALSE);
                       if (!itb) return EINA_FALSE;
                    }
                  itb->items = eina_list_append(itb->items, it);
                  it->position = eina_list_count(itb->items);
               }
          }
     }
   else
     {
        Eina_List *tmp;

        if (it->item->rel->item->queued)
          {
             /* NOTE: for a strange reason eina_list and eina_inlist don't have the same property
                on sorted insertion order, so the queue is not always ordered like the item list.
                This lead to issue where we depend on an item that is not yet created. As a quick
                work around, we reschedule the calc of the item and stop reordering the list to
                prevent any nasty issue to show up here.
              */
             wd->queue = eina_list_append(wd->queue, it);
             wd->requeued = EINA_TRUE;
             it->item->queued = EINA_TRUE;
             return EINA_FALSE;
          }
        itb = it->item->rel->item->block;
        if (!itb) goto newblock;
        tmp = eina_list_data_find_list(itb->items, it->item->rel);
        if (it->item->before)
          {
             itb->items = eina_list_prepend_relative_list(itb->items, it, tmp);
             _item_position_update(eina_list_prev(tmp), it->item->rel->position);
          }
        else
          {
             itb->items = eina_list_append_relative_list(itb->items, it, tmp);
             _item_position_update(eina_list_next(tmp), it->item->rel->position + 1);
          }
     }
   itb->count++;
   itb->changed = EINA_TRUE;
   it->item->block = itb;
   if (itb->wd->calc_job) ecore_job_del(itb->wd->calc_job);
   itb->wd->calc_job = ecore_job_add(_calc_job, itb->wd);
   if (it->item->rel)
     {
        it->item->rel->relcount--;
        if ((it->item->rel->generation < it->wd->generation) && (!it->item->rel->relcount))
          {
             _item_del(it->item->rel);
             elm_widget_item_free(it->item->rel);
          }
        it->item->rel = NULL;
     }
   if (itb->count > itb->wd->max_items_per_block)
     {
        Item_Block *itb2;
        Elm_Gen_Item *it2;
        int newc;
        Eina_Bool done = EINA_FALSE;

        newc = itb->count / 2;

        if (EINA_INLIST_GET(itb)->prev)
          {
             Item_Block *itbp = (Item_Block *)(EINA_INLIST_GET(itb)->prev);

             if (itbp->count + newc < wd->max_items_per_block / 2)
               {
                  /* moving items to previous block */
                  while ((itb->count > newc) && (itb->items))
                    {
                       it2 = eina_list_data_get(itb->items);
                       itb->items = eina_list_remove_list(itb->items, itb->items);
                       itb->count--;

                       itbp->items = eina_list_append(itbp->items, it2);
                       it2->item->block = itbp;
                       itbp->count++;
                    }

                  done = EINA_TRUE;
               }
          }

        if (!done && EINA_INLIST_GET(itb)->next)
          {
             Item_Block *itbn = (Item_Block *)(EINA_INLIST_GET(itb)->next);

             if (itbn->count + newc < wd->max_items_per_block / 2)
               {
                  /* moving items to next block */
                  while ((itb->count > newc) && (itb->items))
                    {
                       Eina_List *l;

                       l = eina_list_last(itb->items);
                       it2 = eina_list_data_get(l);
                       itb->items = eina_list_remove_list(itb->items, l);
                       itb->count--;

                       itbn->items = eina_list_prepend(itbn->items, it2);
                       it2->item->block = itbn;
                       itbn->count++;
                    }

                  done = EINA_TRUE;
               }
          }

        if (!done)
          {
             /* moving items to new block */
             itb2 = calloc(1, sizeof(Item_Block));
             if (!itb2) return EINA_FALSE;
             itb2->wd = wd;
             wd->blocks =
               eina_inlist_append_relative(wd->blocks, EINA_INLIST_GET(itb2),
                                           EINA_INLIST_GET(itb));
             itb2->changed = EINA_TRUE;
             while ((itb->count > newc) && (itb->items))
               {
                  Eina_List *l;

                  l = eina_list_last(itb->items);
                  it2 = l->data;
                  itb->items = eina_list_remove_list(itb->items, l);
                  itb->count--;

                  itb2->items = eina_list_prepend(itb2->items, it2);
                  it2->item->block = itb2;
                  itb2->count++;
               }
          }
     }

   return EINA_TRUE;
}

static int
_queue_process(Widget_Data *wd)
{
   int n;
   Eina_Bool showme = EINA_FALSE;
   double t0, t;

   t0 = ecore_loop_time_get();
   //evas_event_freeze(evas_object_evas_get(wd->obj));
   for (n = 0; (wd->queue) && (n < 128); n++)
     {
        Elm_Gen_Item *it;

        it = eina_list_data_get(wd->queue);
        wd->queue = eina_list_remove_list(wd->queue, wd->queue);
        it->item->queued = EINA_FALSE;
        if (!_item_block_add(wd, it)) continue;
        if (!wd->blocks)
          _item_block_realize(it->item->block);
        t = ecore_time_get();
        if (it->item->block->changed)
          {
             showme = _item_block_recalc(it->item->block, it->item->block->num, EINA_TRUE);
             it->item->block->changed = 0;
             if (wd->pan_changed)
               {
                  if (wd->calc_job) ecore_job_del(wd->calc_job);
                  wd->calc_job = NULL;
                  _calc_job(wd);
                  wd->pan_changed = EINA_FALSE;
               }
          }
        if (showme) it->item->block->showme = EINA_TRUE;
        /* same as eina_inlist_count > 1 */
        if (wd->blocks && wd->blocks->next)
          {
             if ((t - t0) > (ecore_animator_frametime_get())) break;
          }
     }
   //evas_event_thaw(evas_object_evas_get(wd->obj));
   //evas_event_thaw_eval(evas_object_evas_get(wd->obj));
   return n;
}

static Eina_Bool
_idle_process(void *data, Eina_Bool *wakeup)
{
   Widget_Data *wd = data;

   //xxx
   //static double q_start = 0.0;
   //if (q_start == 0.0) q_start = ecore_time_get();
   //xxx
   if (_queue_process(wd) > 0) *wakeup = EINA_TRUE;
   if (!wd->queue)
     {
        //xxx
        //printf("PROCESS TIME: %3.3f\n", ecore_time_get() - q_start);
        //xxx
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_item_idle_enterer(void *data)
{
   Widget_Data *wd = data;
   Eina_Bool wakeup = EINA_FALSE;
   Eina_Bool ok = _idle_process(data, &wakeup);

   if (wakeup)
     {
        // wake up mainloop
        if (wd->calc_job) ecore_job_del(wd->calc_job);
        wd->calc_job = ecore_job_add(_calc_job, wd);
     }
   if (ok == ECORE_CALLBACK_CANCEL) wd->queue_idle_enterer = NULL;
   return ok;
}

static void
_item_queue(Widget_Data *wd,
            Elm_Gen_Item *it,
            Eina_Compare_Cb cb)
{
   if (it->item->queued) return;
   it->item->queued = EINA_TRUE;
   if (cb && !wd->requeued)
     wd->queue = eina_list_sorted_insert(wd->queue, cb, it);
   else
     wd->queue = eina_list_append(wd->queue, it);
// FIXME: why does a freeze then thaw here cause some genlist
// elm_genlist_item_append() to be much much slower?
//   evas_event_freeze(evas_object_evas_get(wd->obj));
   while ((wd->queue) && ((!wd->blocks) || (!wd->blocks->next)))
     {
        if (wd->queue_idle_enterer)
          {
             ecore_idle_enterer_del(wd->queue_idle_enterer);
             wd->queue_idle_enterer = NULL;
          }
        _queue_process(wd);
     }
//   evas_event_thaw(evas_object_evas_get(wd->obj));
//   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
   if (!wd->queue_idle_enterer)
     wd->queue_idle_enterer = ecore_idle_enterer_add(_item_idle_enterer, wd);
}

static int
_elm_genlist_item_compare(const void *data, const void *data1)
{
   const Elm_Gen_Item *it, *item1;
   it = ELM_GEN_ITEM_FROM_INLIST(data);
   item1 = ELM_GEN_ITEM_FROM_INLIST(data1);
   return it->wd->item_compare_cb(it, item1);
}

static int
_elm_genlist_item_list_compare(const void *data, const void *data1)
{
   const Elm_Gen_Item *it = data;
   const Elm_Gen_Item *item1 = data1;
   return it->wd->item_compare_cb(it, item1);
}

/*If application want to know the relative item, use elm_genlist_item_prev_get(it)*/
static void
_item_move_after(Elm_Gen_Item *it, Elm_Gen_Item *after)
{
   if (!it) return;
   if (!after) return;

   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   _item_block_del(it);

   it->wd->items = eina_inlist_append_relative(it->wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(after));
   it->item->rel = after;
   it->item->rel->relcount++;
   it->item->before = EINA_FALSE;
   if (after->item->group_item) it->item->group_item = after->item->group_item;
   _item_queue(it->wd, it, NULL);

   evas_object_smart_callback_call(WIDGET(it), SIG_MOVED_AFTER, it);
}

/*If application want to know the relative item, use elm_genlist_item_next_get(it)*/
static void
_item_move_before(Elm_Gen_Item *it, Elm_Gen_Item *before)
{
   if (!it) return;
   if (!before) return;

   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   _item_block_del(it);
   it->wd->items = eina_inlist_prepend_relative(it->wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(before));
   it->item->rel = before;
   it->item->rel->relcount++;
   it->item->before = EINA_TRUE;
   if (before->item->group_item) it->item->group_item = before->item->group_item;
   _item_queue(it->wd, it, NULL);

   evas_object_smart_callback_call(WIDGET(it), SIG_MOVED_BEFORE, it);
}

EAPI unsigned int
elm_genlist_items_count(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->item_count;
}

EAPI Elm_Object_Item *
elm_genlist_item_append(Evas_Object                  *obj,
                        const Elm_Genlist_Item_Class *itc,
                        const void                   *data,
                        Elm_Object_Item              *parent,
                        Elm_Genlist_Item_Type         type,
                        Evas_Smart_Cb                 func,
                        const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Elm_Gen_Item *it = _item_new(wd, itc, data, (Elm_Gen_Item *) parent, type,
                                func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     {
        if (it->group)
          wd->group_items = eina_list_append(wd->group_items, it);
        wd->items = eina_inlist_append(wd->items, EINA_INLIST_GET(it));
        it->item->rel = NULL;
     }
   else
     {
        Elm_Gen_Item *it2 = NULL;
        Eina_List *ll = eina_list_last(it->parent->item->items);
        if (ll) it2 = ll->data;
        it->parent->item->items = eina_list_append(it->parent->item->items, it);
        if (!it2) it2 = it->parent;
        wd->items =
          eina_inlist_append_relative(wd->items, EINA_INLIST_GET(it),
                                      EINA_INLIST_GET(it2));
        it->item->rel = it2;
        it->item->rel->relcount++;
     }
   it->item->before = EINA_FALSE;
   _item_queue(wd, it, NULL);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_prepend(Evas_Object                  *obj,
                         const Elm_Genlist_Item_Class *itc,
                         const void                   *data,
                         Elm_Object_Item              *parent,
                         Elm_Genlist_Item_Type         type,
                         Evas_Smart_Cb                 func,
                         const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Elm_Gen_Item *it = _item_new(wd, itc, data, (Elm_Gen_Item *) parent, type,
                                func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     {
        if (it->group)
          wd->group_items = eina_list_prepend(wd->group_items, it);
        wd->items = eina_inlist_prepend(wd->items, EINA_INLIST_GET(it));
        it->item->rel = NULL;
     }
   else
     {
        Elm_Gen_Item *it2 = NULL;
        Eina_List *ll = it->parent->item->items;
        if (ll) it2 = ll->data;
        it->parent->item->items = eina_list_prepend(it->parent->item->items, it);
        if (!it2) it2 = it->parent;
        wd->items =
          eina_inlist_prepend_relative(wd->items, EINA_INLIST_GET(it),
                                       EINA_INLIST_GET(it2));
        it->item->rel = it2;
        it->item->rel->relcount++;
     }
   it->item->before = EINA_TRUE;
   _item_queue(wd, it, NULL);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_insert_after(Evas_Object                  *obj,
                              const Elm_Genlist_Item_Class *itc,
                              const void                   *data,
                              Elm_Object_Item              *parent,
                              Elm_Object_Item              *after,
                              Elm_Genlist_Item_Type         type,
                              Evas_Smart_Cb                 func,
                              const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   ELM_OBJ_ITEM_CHECK_OR_RETURN(after, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Gen_Item *_after = (Elm_Gen_Item *) after;
   if (!wd) return NULL;
   /* It makes no sense to insert after in an empty list with after != NULL, something really bad is happening in your app. */
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd->items, NULL);

   Elm_Gen_Item *it = _item_new(wd, itc, data, (Elm_Gen_Item *) parent, type,
                                func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     {
        if ((it->group) && (_after->group))
          wd->group_items = eina_list_append_relative(wd->group_items, it,
                                                      _after);
     }
   else
     {
        it->parent->item->items =
           eina_list_append_relative(it->parent->item->items, it, _after);
     }
   wd->items = eina_inlist_append_relative(wd->items, EINA_INLIST_GET(it),
                                           EINA_INLIST_GET(_after));
   it->item->rel = _after;
   it->item->rel->relcount++;
   it->item->before = EINA_FALSE;
   _item_queue(wd, it, NULL);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_insert_before(Evas_Object                  *obj,
                               const Elm_Genlist_Item_Class *itc,
                               const void                   *data,
                               Elm_Object_Item              *parent,
                               Elm_Object_Item              *before,
                               Elm_Genlist_Item_Type         type,
                               Evas_Smart_Cb                 func,
                               const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   ELM_OBJ_ITEM_CHECK_OR_RETURN(before, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Gen_Item *_before = (Elm_Gen_Item *) before;
   if (!wd) return NULL;
   /* It makes no sense to insert before in an empty list with before != NULL, something really bad is happening in your app. */
   EINA_SAFETY_ON_NULL_RETURN_VAL(wd->items, NULL);

   Elm_Gen_Item *it = _item_new(wd, itc, data, (Elm_Gen_Item *) parent, type,
                                func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     {
        if (it->group && _before->group)
          wd->group_items = eina_list_prepend_relative(wd->group_items, it,
                                                       _before);
     }
   else
     {
        it->parent->item->items =
           eina_list_prepend_relative(it->parent->item->items, it, _before);
     }
   wd->items = eina_inlist_prepend_relative(wd->items, EINA_INLIST_GET(it),
                                            EINA_INLIST_GET(_before));
   it->item->rel = _before;
   it->item->rel->relcount++;
   it->item->before = EINA_TRUE;
   _item_queue(wd, it, NULL);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_sorted_insert(Evas_Object                  *obj,
                               const Elm_Genlist_Item_Class *itc,
                               const void                   *data,
                               Elm_Object_Item              *parent,
                               Elm_Genlist_Item_Type         type,
                               Eina_Compare_Cb               comp,
                               Evas_Smart_Cb                 func,
                               const void                   *func_data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Elm_Gen_Item *rel = NULL;
   Elm_Gen_Item *it = _item_new(wd, itc, data, (Elm_Gen_Item *) parent, type,
                                func, func_data);
   if (!it) return NULL;

   wd->item_compare_cb = comp;

   if (it->parent)
     {
        Eina_List *l;
        int cmp_result;

        l = eina_list_search_sorted_near_list(it->parent->item->items,
                                              _elm_genlist_item_list_compare,
                                              it,
                                              &cmp_result);
        if (l)
          rel = eina_list_data_get(l);
        else
          rel = it->parent;

        if (cmp_result >= 0)
          {
             it->parent->item->items = eina_list_prepend_relative_list(it->parent->item->items, it, l);
             wd->items = eina_inlist_prepend_relative(wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(rel));
             it->item->before = EINA_FALSE;
          }
        else if (cmp_result < 0)
          {
             it->parent->item->items = eina_list_append_relative_list(it->parent->item->items, it, l);
             wd->items = eina_inlist_append_relative(wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(rel));
             it->item->before = EINA_TRUE;
          }
     }
   else
     {
        if (!wd->state)
          {
             wd->state = eina_inlist_sorted_state_new();
             eina_inlist_sorted_state_init(wd->state, wd->items);
             wd->requeued = EINA_FALSE;
          }

        if (it->group)
          wd->group_items = eina_list_append(wd->group_items, it);

        wd->items = eina_inlist_sorted_state_insert(wd->items, EINA_INLIST_GET(it),
                                                    _elm_genlist_item_compare, wd->state);

        if (EINA_INLIST_GET(it)->next)
          {
             rel = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
             it->item->before = EINA_TRUE;
          }
        else if (EINA_INLIST_GET(it)->prev)
          {
             rel = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
             it->item->before = EINA_FALSE;
          }
     }

   if (rel)
     {
        it->item->rel = rel;
        it->item->rel->relcount++;
     }

   _item_queue(wd, it, _elm_genlist_item_list_compare);

   return (Elm_Object_Item *)it;
}

static void
_elm_genlist_clear(Evas_Object *obj, Eina_Bool standby)
{
   Eina_Inlist *next, *l;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (!standby) wd->generation++;

   if (wd->state)
     {
        eina_inlist_sorted_state_free(wd->state);
        wd->state = NULL;
     }

   if (wd->walking > 0)
     {
        wd->clear_me = EINA_TRUE;
        return;
     }
   evas_event_freeze(evas_object_evas_get(wd->obj));
   for (l = wd->items, next = l ? l->next : NULL;
        l;
        l = next, next = next ? next->next : NULL)
     {
        Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(l);

        if (it->generation < wd->generation)
          {
             Elm_Gen_Item *itn = NULL;

             if (next) itn = ELM_GEN_ITEM_FROM_INLIST(next);
             if (itn) itn->walking++; /* prevent early death of subitem */
             it->del_cb(it);
             elm_widget_item_free(it);
             if (itn) itn->walking--;
          }
     }
   wd->clear_me = EINA_FALSE;
   wd->pan_changed = EINA_TRUE;
   if (wd->calc_job)
     {
        ecore_job_del(wd->calc_job);
        wd->calc_job = NULL;
     }
   if (wd->selected) wd->selected = eina_list_free(wd->selected);
   if (wd->clear_cb) wd->clear_cb(wd);
   wd->pan_x = 0;
   wd->pan_y = 0;
   wd->minw = 0;
   wd->minh = 0;

   if (wd->alpha_bg) evas_object_del(wd->alpha_bg);
   wd->alpha_bg = NULL;

   if (wd->pan_smart)
     {
        evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
        evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
     }
   if (wd->sizing_cb) wd->sizing_cb(wd->obj);
   elm_smart_scroller_child_region_show(wd->scr, 0, 0, 0, 0);
   evas_event_thaw(evas_object_evas_get(wd->obj));
   evas_event_thaw_eval(evas_object_evas_get(wd->obj));
}

EAPI void
elm_genlist_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   _elm_genlist_clear(obj, EINA_FALSE);
}

EAPI void
elm_genlist_multi_select_set(Evas_Object *obj,
                             Eina_Bool    multi)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->multi = !!multi;
}

EAPI Eina_Bool
elm_genlist_multi_select_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->multi;
}

EAPI Elm_Object_Item *
elm_genlist_selected_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (wd->selected) return wd->selected->data;
   return NULL;
}

EAPI const Eina_List *
elm_genlist_selected_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->selected;
}

EAPI Eina_List *
elm_genlist_realized_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *list = NULL;
   Item_Block *itb;
   Eina_Bool done = EINA_FALSE;
   if (!wd) return NULL;
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        if (itb->realized)
          {
             Eina_List *l;
             Elm_Gen_Item *it;

             done = EINA_TRUE;
             EINA_LIST_FOREACH(itb->items, l, it)
               {
                  if (it->realized) list = eina_list_append(list, it);
               }
          }
        else
          {
             if (done) break;
          }
     }
   return list;
}

EAPI Elm_Object_Item *
elm_genlist_at_xy_item_get(const Evas_Object *obj,
                           Evas_Coord         x,
                           Evas_Coord         y,
                           int               *posret)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord ox, oy, ow, oh;
   Item_Block *itb;
   Evas_Coord lasty;
   if (!wd) return NULL;
   evas_object_geometry_get(wd->pan_smart, &ox, &oy, &ow, &oh);
   lasty = oy;
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
        Eina_List *l;
        Elm_Gen_Item *it;

        if (!ELM_RECTS_INTERSECT(ox + itb->x - itb->wd->pan_x,
                                 oy + itb->y - itb->wd->pan_y,
                                 itb->w, itb->h, x, y, 1, 1))
          continue;
        EINA_LIST_FOREACH(itb->items, l, it)
          {
             Evas_Coord itx, ity;

             itx = ox + itb->x + it->x - itb->wd->pan_x;
             ity = oy + itb->y + it->y - itb->wd->pan_y;
             if (ELM_RECTS_INTERSECT(itx, ity, it->item->w, it->item->h, x, y, 1, 1))
               {
                  if (posret)
                    {
                       if (y <= (ity + (it->item->h / 4))) *posret = -1;
                       else if (y >= (ity + it->item->h - (it->item->h / 4)))
                         *posret = 1;
                       else *posret = 0;
                    }
                  return (Elm_Object_Item *)it;
               }
             lasty = ity + it->item->h;
          }
     }
   if (posret)
     {
        if (y > lasty) *posret = 1;
        else *posret = -1;
     }
   return NULL;
}

EAPI Elm_Object_Item *
elm_genlist_first_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(wd->items);
   while ((it) && (it->generation < wd->generation))
     it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_last_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(wd->items->last);
   while ((it) && (it->generation < wd->generation))
     it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_genlist_item_next_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   while (_it)
     {
        _it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(_it)->next);
        if ((_it) && (_it->generation == _it->wd->generation)) break;
     }
   return (Elm_Object_Item *) _it;
}

EAPI Elm_Object_Item *
elm_genlist_item_prev_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   while (_it)
     {
        _it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(_it)->prev);
        if ((_it) && (_it->generation == _it->wd->generation)) break;
     }
   return (Elm_Object_Item *) _it;
}

EAPI Elm_Object_Item *
elm_genlist_item_parent_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return (Elm_Object_Item *) ((Elm_Gen_Item *)it)->parent;
}

EAPI void
elm_genlist_item_subitems_clear(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Widget_Data *wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return;
   Elm_Gen_Item *_it = (Elm_Gen_Item *) it;

   if (!wd->tree_effect_enabled || !wd->move_effect_mode)
     _item_subitems_clear(_it);
   else
     {
        if (!wd->tree_effect_animator)
          {
             wd->expanded_item = _it;
             _item_tree_effect_before(_it);
             evas_object_raise(wd->alpha_bg);
             evas_object_show(wd->alpha_bg);
             wd->start_time = ecore_time_get();
             wd->tree_effect_animator = ecore_animator_add(_tree_effect_animator_cb, wd);
          }
        else
           _item_subitems_clear(_it);
     }
}

EAPI void
elm_genlist_item_selected_set(Elm_Object_Item *it,
                              Eina_Bool selected)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   Widget_Data *wd = _it->wd;
   if (!wd) return;
   if ((_it->generation < wd->generation) || elm_widget_item_disabled_get(_it))
     return;
   selected = !!selected;
   if (_it->selected == selected) return;

   if (selected)
     {
        if (!wd->multi)
          {
             while (wd->selected)
               {
                  if (_it->unhighlight_cb) _it->unhighlight_cb(wd->selected->data);
                  _it->unsel_cb(wd->selected->data);
               }
          }
        _it->highlight_cb(_it);
        _item_select(_it);
        return;
     }
   if (_it->unhighlight_cb) _it->unhighlight_cb(_it);
   _it->unsel_cb(_it);
}

EAPI Eina_Bool
elm_genlist_item_selected_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   return ((Elm_Gen_Item *)it)->selected;
}

Elm_Gen_Item *
_elm_genlist_expanded_next_item_get(Elm_Gen_Item *it)
{
   Elm_Gen_Item *it2;
   if (it->item->expanded)
     {
        it2 = (Elm_Gen_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it);
     }
   else
     {
        it2 = (Elm_Gen_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it);
        while (it2)
          {
             if (it->item->expanded_depth >= it2->item->expanded_depth) break;
             it2 = (Elm_Gen_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it2);
          }
     }
   return it2;
}

static void
_elm_genlist_move_items_set(Elm_Gen_Item *it)
{
   Eina_List *l;
   Elm_Gen_Item *it2 = NULL;
   Evas_Coord ox, oy, ow, oh, dh = 0;

   it->wd->expanded_next_item = _elm_genlist_expanded_next_item_get(it);

   if (it->item->expanded)
     {
        it->wd->move_items = elm_genlist_realized_items_get(it->wd->obj);
        EINA_LIST_FOREACH(it->wd->move_items, l, it2)
          {
             if (it2 == it->wd->expanded_next_item) break;
             it->wd->move_items = eina_list_remove(it->wd->move_items, it2);
          }
     }
   else
     {
        evas_object_geometry_get(it->wd->pan_smart, &ox, &oy, &ow, &oh);
        it2 = it->wd->expanded_next_item;
        while (it2 && (dh < oy + oh))
          {
             dh += it2->item->h;
             it->wd->move_items = eina_list_append(it->wd->move_items, it2);
             it2 = (Elm_Gen_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it2);
          }
     }
}

EAPI void
elm_genlist_item_expanded_set(Elm_Object_Item  *it,
                              Eina_Bool         expanded)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   expanded = !!expanded;
   if (_it->item->expanded == expanded) return;
   _it->item->expanded = expanded;
   _it->wd->expanded_item = _it;
   _elm_genlist_move_items_set(_it);

   if (_it->wd->tree_effect_enabled && !_it->wd->alpha_bg)
      _it->wd->alpha_bg = _create_tray_alpha_bg(WIDGET(_it));

   if (_it->item->expanded)
     {
        _it->wd->move_effect_mode = ELM_GENLIST_TREE_EFFECT_EXPAND;
        if (_it->realized)
          edje_object_signal_emit(VIEW(_it), "elm,state,expanded", "elm");
        evas_object_smart_callback_call(WIDGET(_it), SIG_EXPANDED, _it);
        _it->wd->auto_scroll_enabled = EINA_TRUE;
     }
   else
     {
        _it->wd->move_effect_mode = ELM_GENLIST_TREE_EFFECT_CONTRACT;
        if (_it->realized)
          edje_object_signal_emit(VIEW(_it), "elm,state,contracted", "elm");
        evas_object_smart_callback_call(WIDGET(_it), SIG_CONTRACTED, _it);
        _it->wd->auto_scroll_enabled = EINA_FALSE;
     }
}

EAPI Eina_Bool
elm_genlist_item_expanded_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   return ((Elm_Gen_Item *)it)->item->expanded;
}

EAPI int
elm_genlist_item_expanded_depth_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, 0);
   return ((Elm_Gen_Item *)it)->item->expanded_depth;
}

static Eina_Bool
_elm_genlist_item_compute_coordinates(Elm_Object_Item *it,
                                      Elm_Genlist_Item_Scrollto_Type type,
                                      Eina_Bool bring_in,
                                      Evas_Coord *x,
                                      Evas_Coord *y,
                                      Evas_Coord *w,
                                      Evas_Coord *h)
{
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   Evas_Coord gith = 0;
   if (_it->generation < _it->wd->generation) return EINA_FALSE;
   if ((_it->item->queued) || (!_it->item->mincalcd))
     {
        _it->wd->show_item = _it;
        _it->wd->bring_in = bring_in;
        _it->wd->scrollto_type = type;
        _it->item->showme = EINA_TRUE;
        return EINA_FALSE;
     }
   if (_it->wd->show_item)
     {
        _it->wd->show_item->item->showme = EINA_FALSE;
        _it->wd->show_item = NULL;
     }

   evas_object_geometry_get(_it->wd->pan_smart, NULL, NULL, w, h);
   switch (type)
     {
      case ELM_GENLIST_ITEM_SCROLLTO_IN:
         if ((_it->item->group_item) &&
             (_it->wd->pan_y > (_it->y + _it->item->block->y)))
          gith = _it->item->group_item->item->h;
         *h = _it->item->h;
         *y = _it->y + _it->item->block->y - gith;
         break;
      case ELM_GENLIST_ITEM_SCROLLTO_TOP:
         if (_it->item->group_item) gith = _it->item->group_item->item->h;
         *y = _it->y + _it->item->block->y - gith;
         break;
      case ELM_GENLIST_ITEM_SCROLLTO_MIDDLE:
         *y = _it->y + _it->item->block->y - (*h / 2) + (_it->item->h / 2);
         break;
      default:
         return EINA_FALSE;
     }

   *x = _it->x + _it->item->block->x;
   *w = _it->item->block->w;
   return EINA_TRUE;
}

EAPI void
elm_genlist_item_promote(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   if (_it->generation < _it->wd->generation) return;
   _item_move_before(_it,
                     (Elm_Gen_Item *) elm_genlist_first_item_get(WIDGET(_it)));
}

EAPI void
elm_genlist_item_demote(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   if (_it->generation < _it->wd->generation) return;
   _item_move_after(_it,
                    (Elm_Gen_Item *) elm_genlist_last_item_get(WIDGET(_it)));
}

EAPI void
elm_genlist_item_show(Elm_Object_Item *it, Elm_Genlist_Item_Scrollto_Type type)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Evas_Coord x, y, w, h;
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if (_elm_genlist_item_compute_coordinates(it, type, EINA_FALSE, &x, &y, &w, &h))
     elm_smart_scroller_child_region_show(_it->wd->scr, x, y, w, h);
}

EAPI void
elm_genlist_item_bring_in(Elm_Object_Item *it, Elm_Genlist_Item_Scrollto_Type type)
{

   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Evas_Coord x, y, w, h;
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if (_elm_genlist_item_compute_coordinates(it, type, EINA_TRUE, &x, &y, &w, &h))
     elm_smart_scroller_region_bring_in(_it->wd->scr, x, y, w, h);
}

EAPI void
elm_genlist_item_all_contents_unset(Elm_Object_Item *it, Eina_List **l)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);

   Evas_Object *content;
   EINA_LIST_FREE(((Elm_Gen_Item *)it)->content_objs, content)
     {
        elm_widget_sub_object_del(WIDGET(it), content);
        evas_object_smart_member_del(content);
        evas_object_hide(content);
        if (l) *l = eina_list_append(*l, content);
     }
}

EAPI void
elm_genlist_item_update(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if (!_it->item->block) return;
   if (_it->generation < _it->wd->generation) return;
   _it->item->mincalcd = EINA_FALSE;
   _it->item->updateme = EINA_TRUE;
   _it->item->block->updateme = EINA_TRUE;
   if (_it->wd->update_job) ecore_job_del(_it->wd->update_job);
   _it->wd->update_job = ecore_job_add(_update_job, _it->wd);
}

EAPI void
elm_genlist_item_fields_update(Elm_Object_Item *it,
                               const char *parts,
                               Elm_Genlist_Item_Field_Type itf)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if (!_it->item->block) return;
   if (_it->generation < _it->wd->generation) return;

   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_TEXT))
     _item_text_realize(_it, VIEW(_it), &_it->texts, parts);
   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_CONTENT))
     {
        _it->content_objs = _item_content_unrealize(_it, VIEW(_it),
                                                   &_it->contents, parts);
        _it->content_objs = _item_content_realize(_it, VIEW(_it),
                                                 &_it->contents, parts);
     }
   if ((!itf) || (itf & ELM_GENLIST_ITEM_FIELD_STATE))
     _item_state_realize(_it, VIEW(_it), &_it->states, parts);
}

EAPI void
elm_genlist_item_item_class_update(Elm_Object_Item *it,
                                   const Elm_Genlist_Item_Class *itc)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if (!_it->item->block) return;
   EINA_SAFETY_ON_NULL_RETURN(itc);
   if (_it->generation < _it->wd->generation) return;
   _it->itc = itc;
   _it->item->nocache_once = EINA_TRUE;
   elm_genlist_item_update(it);
}

EAPI const Elm_Genlist_Item_Class *
elm_genlist_item_item_class_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   if (_it->generation < _it->wd->generation) return NULL;
   return _it->itc;
}

static Evas_Object *
_elm_genlist_item_label_create(void        *data,
                               Evas_Object *obj __UNUSED__,
                               Evas_Object *tooltip,
                               void        *it __UNUSED__)
{
   Evas_Object *label = elm_label_add(tooltip);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);
   return label;
}

static void
_elm_genlist_item_label_del_cb(void        *data,
                               Evas_Object *obj __UNUSED__,
                               void        *event_info __UNUSED__)
{
   eina_stringshare_del(data);
}

EAPI void
elm_genlist_item_tooltip_text_set(Elm_Object_Item *it,
                                  const char      *text)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   text = eina_stringshare_add(text);
   elm_genlist_item_tooltip_content_cb_set(it, _elm_genlist_item_label_create,
                                           text,
                                           _elm_genlist_item_label_del_cb);
}

EAPI void
elm_genlist_item_tooltip_content_cb_set(Elm_Object_Item           *it,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void                 *data,
                                        Evas_Smart_Cb               del_cb)
{
   ELM_OBJ_ITEM_CHECK_OR_GOTO(it, error);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if ((_it->tooltip.content_cb == func) && (_it->tooltip.data == data))
     return;

   if (_it->tooltip.del_cb)
     _it->tooltip.del_cb((void *) _it->tooltip.data, WIDGET(it), it);

   _it->tooltip.content_cb = func;
   _it->tooltip.data = data;
   _it->tooltip.del_cb = del_cb;

   if (VIEW(_it))
     {
        elm_widget_item_tooltip_content_cb_set(_it,
                                               _it->tooltip.content_cb,
                                               _it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(_it, _it->tooltip.style);
        elm_widget_item_tooltip_window_mode_set(_it, _it->tooltip.free_size);
     }

   return;

error:
   if (del_cb) del_cb((void *)data, NULL, NULL);
}

EAPI void
elm_genlist_item_tooltip_unset(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if ((VIEW(_it)) && (_it->tooltip.content_cb))
     elm_widget_item_tooltip_unset(_it);

   if (_it->tooltip.del_cb)
     _it->tooltip.del_cb((void *) _it->tooltip.data, WIDGET(_it), _it);
   _it->tooltip.del_cb = NULL;
   _it->tooltip.content_cb = NULL;
   _it->tooltip.data = NULL;
   _it->tooltip.free_size = EINA_FALSE;
   if (_it->tooltip.style)
     elm_genlist_item_tooltip_style_set(it, NULL);
}

EAPI void
elm_genlist_item_tooltip_style_set(Elm_Object_Item  *it,
                                   const char       *style)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   eina_stringshare_replace(&_it->tooltip.style, style);
   if (VIEW(_it)) elm_widget_item_tooltip_style_set(_it, style);
}

EAPI const char *
elm_genlist_item_tooltip_style_get(const Elm_Object_Item *it)
{
   return elm_object_item_tooltip_style_get(it);
}

EAPI Eina_Bool
elm_genlist_item_tooltip_window_mode_set(Elm_Object_Item *it,
                                         Eina_Bool disable)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   _it->tooltip.free_size = disable;
   if (VIEW(_it)) return elm_widget_item_tooltip_window_mode_set(_it, disable);
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_genlist_item_tooltip_window_mode_get(const Elm_Object_Item *it)
{
   return elm_object_tooltip_window_mode_get(VIEW(it));
}

EAPI void
elm_genlist_item_cursor_set(Elm_Object_Item  *it,
                            const char       *cursor)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   eina_stringshare_replace(&_it->mouse_cursor, cursor);
   if (VIEW(_it)) elm_widget_item_cursor_set(_it, cursor);
}

EAPI const char *
elm_genlist_item_cursor_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_get(it);
}

EAPI void
elm_genlist_item_cursor_unset(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if (!_it->mouse_cursor) return;

   if (VIEW(_it)) elm_widget_item_cursor_unset(_it);

   eina_stringshare_del(_it->mouse_cursor);
   _it->mouse_cursor = NULL;
}

EAPI void
elm_genlist_item_cursor_style_set(Elm_Object_Item  *it,
                                  const char       *style)
{
   elm_widget_item_cursor_style_set(it, style);
}

EAPI const char *
elm_genlist_item_cursor_style_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_style_get(it);
}

EAPI void
elm_genlist_item_cursor_engine_only_set(Elm_Object_Item *it,
                                        Eina_Bool        engine_only)
{
   elm_widget_item_cursor_engine_only_set(it, engine_only);
}

EAPI Eina_Bool
elm_genlist_item_cursor_engine_only_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_engine_only_get(it);
}

EAPI int
elm_genlist_item_index_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, -1);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   if (_it->item->block)
     return _it->position + (_it->item->block->position * _it->wd->max_items_per_block);
   return -1;
}

EAPI void
elm_genlist_mode_set(Evas_Object  *obj,
                     Elm_List_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->mode == mode) return;
   wd->mode = mode;
   if (wd->mode == ELM_LIST_COMPRESS)
     elm_genlist_homogeneous_set(obj, EINA_FALSE);
   _sizing_eval(obj);
}


EAPI Elm_List_Mode
elm_genlist_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_LIST_LAST;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_LIST_LAST;
   return wd->mode;
}

EAPI void
elm_genlist_bounce_set(Evas_Object *obj,
                       Eina_Bool    h_bounce,
                       Eina_Bool    v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->h_bounce = !!h_bounce;
   wd->v_bounce = !!v_bounce;
   elm_smart_scroller_bounce_allow_set(wd->scr, wd->h_bounce, wd->v_bounce);
}

EAPI void
elm_genlist_bounce_get(const Evas_Object *obj,
                       Eina_Bool         *h_bounce,
                       Eina_Bool         *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (h_bounce) *h_bounce = wd->h_bounce;
   if (v_bounce) *v_bounce = wd->v_bounce;
}

EAPI void
elm_genlist_homogeneous_set(Evas_Object *obj,
                            Eina_Bool    homogeneous)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->homogeneous = !!homogeneous;
}

EAPI Eina_Bool
elm_genlist_homogeneous_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->homogeneous;
}

EAPI void
elm_genlist_block_count_set(Evas_Object *obj,
                            int          count)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->max_items_per_block = count;
   wd->item_cache_max = wd->max_items_per_block * 2;
   _item_cache_clean(wd);
}

EAPI int
elm_genlist_block_count_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->max_items_per_block;
}

EAPI void
elm_genlist_longpress_timeout_set(Evas_Object *obj,
                                  double       timeout)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->longpress_timeout = timeout;
}

EAPI double
elm_genlist_longpress_timeout_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->longpress_timeout;
}

EAPI void
elm_genlist_scroller_policy_set(Evas_Object        *obj,
                                Elm_Scroller_Policy policy_h,
                                Elm_Scroller_Policy policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->scr)) return;
   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;
   elm_smart_scroller_policy_set(wd->scr, policy_h, policy_v);
}

EAPI void
elm_genlist_scroller_policy_get(const Evas_Object   *obj,
                                Elm_Scroller_Policy *policy_h,
                                Elm_Scroller_Policy *policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Smart_Scroller_Policy s_policy_h, s_policy_v;
   if ((!wd) || (!wd->scr)) return;
   elm_smart_scroller_policy_get(wd->scr, &s_policy_h, &s_policy_v);
   if (policy_h) *policy_h = (Elm_Scroller_Policy)s_policy_h;
   if (policy_v) *policy_v = (Elm_Scroller_Policy)s_policy_v;
}

EAPI void
elm_genlist_realized_items_update(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Eina_List *list, *l;
   Elm_Object_Item *it;

   list = elm_genlist_realized_items_get(obj);
   EINA_LIST_FOREACH(list, l, it)
     elm_genlist_item_update(it);
}

EAPI void
elm_genlist_item_decorate_mode_set(Elm_Object_Item  *it,
                                   const char       *decorate_type,
                                   Eina_Bool         mode_set)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   Widget_Data *wd = _it->wd;
   Eina_List *l;
   Elm_Object_Item *it2;

   if (!wd) return;
   if (!decorate_type) return;
   if ((_it->generation < _it->wd->generation) ||
       elm_widget_item_disabled_get(_it)) return;
   if (wd->decorate_mode) return;

   if ((wd->mode_item == _it) &&
       (!strcmp(decorate_type, wd->decorate_type)) &&
       (mode_set))
      return;
   if (!_it->itc->decorate_item_style) return;
   _it->mode_set = mode_set;

   if (wd->multi)
     {
        EINA_LIST_FOREACH(wd->selected, l, it2)
          if (((Elm_Gen_Item *)it2)->realized)
            elm_genlist_item_selected_set(it2, EINA_FALSE);
     }
   else
     {
        it2 = elm_genlist_selected_item_get(wd->obj);
        if ((it2) && (((Elm_Gen_Item *)it2)->realized))
          elm_genlist_item_selected_set(it2, EINA_FALSE);
     }

   if (((wd->decorate_type) && (strcmp(decorate_type, wd->decorate_type))) ||
       (mode_set) ||
       ((_it == wd->mode_item) && (!mode_set)))
     _item_mode_unset(wd);

   eina_stringshare_replace(&wd->decorate_type, decorate_type);
   if (mode_set) _item_mode_set(_it);
}

EAPI const char *
elm_genlist_item_decorate_mode_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   return _it->wd->decorate_type;
}

EAPI const Elm_Object_Item *
elm_genlist_decorated_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return (Elm_Object_Item *) wd->mode_item;
}

EAPI Eina_Bool
elm_genlist_decorate_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   return wd->decorate_mode;
}

EAPI void
elm_genlist_decorate_mode_set(Evas_Object *obj, Eina_Bool decorated)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_List *list, *l;
   Elm_Gen_Item *it;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   decorated = !!decorated;
   if (wd->decorate_mode == decorated) return;
   wd->decorate_mode = decorated;

   list = elm_genlist_realized_items_get(obj);
   if (!wd->decorate_mode)
     {
        EINA_LIST_FOREACH(list, l, it)
          {
             if (it->item->type != ELM_GENLIST_ITEM_GROUP)
               _decorate_mode_item_unrealize(it);
          }
        _item_cache_zero(wd);
     }
   else
     {
        EINA_LIST_FOREACH(list, l, it)
          {
             if (it->item->type != ELM_GENLIST_ITEM_GROUP)
               {
                  if (it->selected) _item_unselect(it);
                  if (it->itc->decorate_all_item_style)
                     _decorate_mode_item_realize(it, EINA_TRUE);
               }
          }
     }
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

EAPI void
elm_genlist_reorder_mode_set(Evas_Object *obj,
                             Eina_Bool    reorder_mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->reorder_mode = !!reorder_mode;
}

EAPI Eina_Bool
elm_genlist_reorder_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->reorder_mode;
}

EAPI Elm_Genlist_Item_Type
elm_genlist_item_type_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, ELM_GENLIST_ITEM_MAX);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   return _it->item->type;
}

EAPI Elm_Genlist_Item_Class *
elm_genlist_item_class_new(void)
{
   Elm_Genlist_Item_Class *itc;

   itc = calloc(1, sizeof(Elm_Genlist_Item_Class));
   if (!itc)
     return NULL;
   itc->version = CLASS_ALLOCATED;
   itc->refcount = 1;
   itc->delete_me = EINA_FALSE;

   return itc;
}

EAPI void
elm_genlist_item_class_free(Elm_Genlist_Item_Class *itc)
{
   if (itc && (itc->version == CLASS_ALLOCATED))
     {
        if (!itc->delete_me) itc->delete_me = EINA_TRUE;
        if (itc->refcount > 0) elm_genlist_item_class_unref(itc);
        else
          {
             itc->version = 0;
             free(itc);
          }
     }
}

EAPI void
elm_genlist_item_class_ref(Elm_Genlist_Item_Class *itc)
{
   if (itc && (itc->version == CLASS_ALLOCATED))
     {
        itc->refcount++;
        if (itc->refcount == 0) itc->refcount--;
     }
}

EAPI void
elm_genlist_item_class_unref(Elm_Genlist_Item_Class *itc)
{
   if (itc && (itc->version == CLASS_ALLOCATED))
     {
        if (itc->refcount > 0) itc->refcount--;
        if (itc->delete_me && (!itc->refcount))
          elm_genlist_item_class_free(itc);
     }
}

void _flip_job(void *data)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *) data;
   _elm_genlist_item_unrealize(it, EINA_FALSE);
   if (it->selected) _item_unselect(it);
   it->flipped = EINA_TRUE;
   it->item->nocache = EINA_TRUE;
}

EAPI void
elm_genlist_item_flip_set(Elm_Object_Item *it,
                          Eina_Bool flip)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;

   flip = !!flip;
   if (_it->flipped == flip) return;

   if (flip)
     {
        ecore_job_add(_flip_job, _it);
        if (_it->wd->calc_job) ecore_job_del(_it->wd->calc_job);
        _it->wd->calc_job = ecore_job_add(_calc_job, _it->wd);
     }
   else
     {
        _it->flipped = flip;
        _item_cache_zero(_it->wd);
        elm_genlist_item_update(it);
        _it->item->nocache = EINA_FALSE;
     }
}

EAPI Eina_Bool
elm_genlist_item_flip_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   return _it->flipped;
}

EAPI void
elm_genlist_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;
   if (wd->select_mode != mode)
     wd->select_mode = mode;
}

EAPI Elm_Object_Select_Mode
elm_genlist_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_OBJECT_SELECT_MODE_MAX;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_OBJECT_SELECT_MODE_MAX;
   return wd->select_mode;
}

EAPI void
elm_genlist_highlight_mode_set(Evas_Object *obj,
                               Eina_Bool    highlight)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->highlight = !!highlight;
}

EAPI Eina_Bool
elm_genlist_highlight_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->highlight;
}

EAPI void
elm_genlist_item_select_mode_set(Elm_Object_Item *it,
                                 Elm_Object_Select_Mode mode)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   if (!_it) return;
   if (_it->generation < _it->wd->generation) return;
   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;
   if (_it->select_mode != mode)
     _it->select_mode = mode;

   if (_it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
     {
        _it->item->mincalcd = EINA_FALSE;
        _it->item->updateme = EINA_TRUE;
        if (_it->item->block) _it->item->block->updateme = EINA_TRUE;
        if (_it->wd->update_job) ecore_job_del(_it->wd->update_job);
        _it->wd->update_job = ecore_job_add(_update_job, _it->wd);
     }
}

EAPI Elm_Object_Select_Mode
elm_genlist_item_select_mode_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, ELM_OBJECT_SELECT_MODE_MAX);
   Elm_Gen_Item *_it = (Elm_Gen_Item *)it;
   if (!_it) return ELM_OBJECT_SELECT_MODE_MAX;
   return _it->select_mode;
}

/* for gengrid as of now */
void
_elm_genlist_page_relative_set(Evas_Object *obj,
                               double       h_pagerel,
                               double       v_pagerel)
{
   Evas_Coord pagesize_h;
   Evas_Coord pagesize_v;

   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, NULL, NULL, &pagesize_h, &pagesize_v);
   elm_smart_scroller_paging_set(wd->scr, h_pagerel, v_pagerel, pagesize_h,
                                 pagesize_v);
}

/* for gengrid as of now */
void
_elm_genlist_page_relative_get(const Evas_Object *obj,
                               double            *h_pagerel,
                               double            *v_pagerel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, h_pagerel, v_pagerel, NULL, NULL);
}

/* for gengrid as of now */
void
_elm_genlist_page_size_set(Evas_Object *obj,
                           Evas_Coord   h_pagesize,
                           Evas_Coord   v_pagesize)
{
   double pagerel_h;
   double pagerel_v;

   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_paging_get(wd->scr, &pagerel_h, &pagerel_v, NULL, NULL);
   elm_smart_scroller_paging_set(wd->scr, pagerel_h, pagerel_v, h_pagesize,
                                 v_pagesize);
}

/* for gengrid as of now */
void
_elm_genlist_current_page_get(const Evas_Object *obj,
                              int               *h_pagenumber,
                              int               *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_current_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

/* for gengrid as of now */
void
_elm_genlist_last_page_get(const Evas_Object *obj,
                           int               *h_pagenumber,
                           int               *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_last_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

/* for gengrid as of now */
void
_elm_genlist_page_show(const Evas_Object *obj,
                       int                h_pagenumber,
                       int                v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_page_show(wd->scr, h_pagenumber, v_pagenumber);
}

/* for gengrid as of now */
void
_elm_genlist_page_bring_in(const Evas_Object *obj,
                           int                h_pagenumber,
                           int                v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_page_bring_in(wd->scr, h_pagenumber, v_pagenumber);
}

void
_elm_genlist_item_unrealize(Elm_Gen_Item *it,
                            Eina_Bool     calc)
{
   Evas_Object *content;

   if (!it->realized) return;
   if (it->wd->reorder_it == it) return;

   evas_event_freeze(evas_object_evas_get(WIDGET(it)));
   if (!calc)
     evas_object_smart_callback_call(WIDGET(it), SIG_UNREALIZED, it);
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }

   elm_widget_stringlist_free(it->texts);
   it->texts = NULL;
   elm_widget_stringlist_free(it->contents);
   it->contents = NULL;
   elm_widget_stringlist_free(it->states);
   it->states = NULL;

   EINA_LIST_FREE(it->content_objs, content)
     evas_object_del(content);

   it->unrealize_cb(it);

   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(WIDGET(it)));
   evas_event_thaw_eval(evas_object_evas_get(WIDGET(it)));
}

void
_elm_genlist_item_del_notserious(Elm_Gen_Item *it)
{
   elm_widget_item_pre_notify_del(it);
   it->generation = it->wd->generation - 1; /* This means that the item is deleted */
   if (it->selected) it->wd->selected = eina_list_remove(it->wd->selected, it);

   if (it->itc->func.del)
     it->itc->func.del((void *)it->base.data, WIDGET(it));
}

void
_elm_genlist_item_del_serious(Elm_Gen_Item *it)
{
   _elm_genlist_item_del_notserious(it);
   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->wd->walking -= it->walking;
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->group)
     it->wd->group_items = eina_list_remove(it->wd->group_items, it);

   if (it->wd->state)
     {
        eina_inlist_sorted_state_free(it->wd->state);
        it->wd->state = NULL;
     }
   if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
   it->wd->calc_job = ecore_job_add(it->wd->calc_cb, it->wd);
   free(it->item);

   it->item = NULL;
   if (it->wd->last_selected_item == (Elm_Object_Item *)it)
     it->wd->last_selected_item = NULL;
   it->wd->item_count--;
}

EAPI void
elm_genlist_tree_effect_enabled_set(Evas_Object *obj, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->tree_effect_enabled = !!enabled;
}

EAPI Eina_Bool
elm_genlist_tree_effect_enabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->tree_effect_enabled;
}

static Evas_Object*
_create_tray_alpha_bg(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   Evas_Object *bg = NULL;
   Evas_Coord ox, oy, ow, oh;

   evas_object_geometry_get(wd->pan_smart, &ox, &oy, &ow, &oh);
   bg  =  evas_object_rectangle_add(evas_object_evas_get(wd->obj));
   evas_object_color_set(bg,0,0,0,0);
   evas_object_resize(bg , ow, oh);
   evas_object_move(bg , ox, oy);
   return bg ;
}

static void
_item_contract_emit(Elm_Gen_Item *it)
{
   Elm_Gen_Item *it2;
   Eina_List *l;

   edje_object_signal_emit(VIEW(it), "elm,state,contract_flip", "");
   it->item->tree_effect_finished = EINA_FALSE;

   EINA_LIST_FOREACH(it->item->items, l, it2)
     if (it2) _item_contract_emit(it2);
}

static int
_item_tree_effect_before(Elm_Gen_Item *it)
{
   Elm_Gen_Item *it2;
   Eina_List *l;

   EINA_LIST_FOREACH(it->item->items, l, it2)
     {
        if (it2->parent && (it == it2->parent))
          {
             if (!it2->realized)
               it2->item->tree_effect_hideme = EINA_TRUE;
             if (it->wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_EXPAND)
               edje_object_signal_emit(VIEW(it2), "elm,state,hide", "");
             else if (it->wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_CONTRACT)
               _item_contract_emit(it2);
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_item_tree_effect(Widget_Data *wd, int y)
{
   Elm_Gen_Item *it = NULL, *expanded_next_it;

   expanded_next_it = wd->expanded_next_item;

   if (wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_EXPAND)
     {
        it = (Elm_Gen_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) expanded_next_it);
        while (it)
          {
             if (it->item->expanded_depth <= expanded_next_it->item->expanded_depth) break;
             if (it->item->scrl_y && (it->item->scrl_y <= expanded_next_it->item->old_scrl_y + y) &&
                 (it->item->expanded_depth > expanded_next_it->item->expanded_depth))
               {
                  if (!it->item->tree_effect_finished)
                    {
                       edje_object_signal_emit(VIEW(it), "flip_item", "");
                       _item_position(it, VIEW(it), it->item->scrl_x, it->item->scrl_y);
                       it->item->tree_effect_finished = EINA_TRUE;
                    }
               }
             it = (Elm_Gen_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it);
          }
     }
   else if (wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_CONTRACT)
     {
        it = (Elm_Gen_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) expanded_next_it);
        while (it)
          {
             if ((it->item->scrl_y > expanded_next_it->item->old_scrl_y + y) &&
                 (it->item->expanded_depth > expanded_next_it->item->expanded_depth))
               {
                  if (!it->item->tree_effect_finished)
                    {
                       edje_object_signal_emit(VIEW(it), "elm,state,hide", "");
                       it->item->tree_effect_finished = EINA_TRUE;
                    }
               }
             else
               break;
             it = (Elm_Gen_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it);
          }
     }
}

static void
_item_tree_effect_finish(Widget_Data *wd)
{
   Elm_Gen_Item *it = NULL;
   const Eina_List *l;

   if (wd->tree_effect_animator)
     {
        if (wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_CONTRACT)
           _item_subitems_clear(wd->expanded_item);
        EINA_LIST_FOREACH(wd->expanded_item->item->items, l, it)
          {
             it->item->tree_effect_finished = EINA_TRUE;
             it->item->old_scrl_y = it->item->scrl_y;
             if (it->wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_EXPAND)
               edje_object_signal_emit(VIEW(it), "elm,state,show", "");
          }
     }
   _item_auto_scroll(wd);
   evas_object_lower(wd->alpha_bg);
   evas_object_hide(wd->alpha_bg);
   wd->move_effect_mode = ELM_GENLIST_TREE_EFFECT_NONE;
   if (wd->move_items) wd->move_items = eina_list_free(wd->move_items);

   evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
   evas_object_smart_callback_call(wd->obj, SIG_TREE_EFFECT_FINISHED, NULL);
   evas_object_smart_changed(wd->pan_smart);

   wd->tree_effect_animator = NULL;
}

static Eina_Bool
_tree_effect_animator_cb(void *data)
{
   Widget_Data *wd = data;
   if (!wd) return EINA_FALSE;
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   Elm_Gen_Item *it = NULL, *it2, *expanded_next_it;
   const Eina_List *l;
   double effect_duration = 0.3, t;
   int y = 0, dy = 0, dh = 0;
   Eina_Bool end = EINA_FALSE, vis = EINA_TRUE;
   int in = 0;

   t = ((0.0 > (t = ecore_time_get() - wd->start_time)) ? 0.0 : t);
   evas_object_geometry_get(wd->pan_smart, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(wd->pan_smart), &cvx, &cvy, &cvw, &cvh);
   if (t > effect_duration) end = EINA_TRUE;

   // Below while statement is needed, when the genlist is resized.
   it2 = wd->expanded_item;
   while (it2 && vis)
     {
        evas_object_move(VIEW(it2), it2->item->scrl_x, it2->item->scrl_y);
        vis = (ELM_RECTS_INTERSECT(it2->item->scrl_x, it2->item->scrl_y, it2->item->w, it2->item->h,
                                   cvx, cvy, cvw, cvh));
        it2 = (Elm_Gen_Item *) elm_genlist_item_prev_get((Elm_Object_Item *) it2);
     }

   if (wd->expanded_next_item)
     {
        expanded_next_it = wd->expanded_next_item;

        /* move items */
        EINA_LIST_FOREACH(wd->move_items, l, it)
          {
             if (wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_EXPAND)
               {
                  expanded_next_it->item->old_scrl_y = wd->expanded_item->item->old_scrl_y + wd->expanded_item->item->h;
                  if (expanded_next_it->item->scrl_y <= expanded_next_it->item->old_scrl_y) //did not calculate next item position
                    expanded_next_it->item->scrl_y = cvy + cvh;

                  dy = ((expanded_next_it->item->scrl_y >= (cvy + cvh)) ?
                         cvy + cvh : expanded_next_it->item->scrl_y) -
                         expanded_next_it->item->old_scrl_y;
               }
             else if (wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_CONTRACT)
               {
                  if (expanded_next_it->item->scrl_y >= expanded_next_it->item->old_scrl_y) //did not calculate next item position
                     expanded_next_it->item->old_scrl_y = cvy + cvh;

                  if (expanded_next_it->item->old_scrl_y > (cvy + cvh))
                    {
                       dy = (wd->expanded_item->item->scrl_y + wd->expanded_item->item->h) -
                       cvy + cvh;
                       expanded_next_it->item->old_scrl_y = cvy + cvh;
                    }
                  else
                    {
                       dy = (wd->expanded_item->item->scrl_y + wd->expanded_item->item->h) -
                             expanded_next_it->item->old_scrl_y;
                    }
               }

             if (t <= effect_duration)
               {
                  y = ((1 - (1 - (t / effect_duration)) * (1 - (t /effect_duration))) * dy);
               }
             else
               {
                  end = EINA_TRUE;
                  y = dy;
               }

             if (!it->realized)
               {
                  _item_realize(it, in, 0);
               }
             in++;

             if (it != expanded_next_it)
               {
                  it->item->old_scrl_y = expanded_next_it->item->old_scrl_y + expanded_next_it->item->h + dh;
                  dh += it->item->h;
               }

             if ((it->item->old_scrl_y + y) < (cvy + cvh))
               _item_position(it, VIEW(it),it->item->scrl_x, it->item->old_scrl_y + y);
          }
        /* tree effect */
        _item_tree_effect(wd, y);
     }
   else
     {
        int expanded_item_num = 0;
        int num = 0;

        if (wd->expanded_item)
          it = (Elm_Gen_Item *) elm_genlist_item_next_get((Elm_Object_Item *) wd->expanded_item);

        it2 = it;
        while (it2)
          {
             expanded_item_num++;
             it2 = (Elm_Gen_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it2);
          }

        while (it)
          {
             num++;
             if (wd->expanded_item->item->expanded_depth >= it->item->expanded_depth) break;
             if (wd->move_effect_mode == ELM_GENLIST_TREE_EFFECT_EXPAND)
               {
                  if (!it->item->tree_effect_finished)
                    {
                       if (t >= (((num - 1) * effect_duration) / expanded_item_num))
                         {
                            edje_object_signal_emit(VIEW(it), "flip_item", "");
                            _item_position(it, VIEW(it), it->item->scrl_x, it->item->scrl_y);
                            it->item->tree_effect_finished = EINA_TRUE;
                         }
                    }
               }
             it = (Elm_Gen_Item *) elm_genlist_item_next_get((Elm_Object_Item *) it);
          }
     }

   if (end)
     {
        _item_tree_effect_finish(wd);
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}
