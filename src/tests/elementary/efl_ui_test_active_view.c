#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "eo_internal.h"

EFL_CLASS_SIMPLE_CLASS(efl_ui_active_view_view_manager, "efl_ui_active_view_view_manager", EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_CLASS);
EFL_CLASS_SIMPLE_CLASS(efl_ui_active_view_indicator, "efl_ui_active_view_indicator", EFL_UI_ACTIVE_VIEW_INDICATOR_CLASS);

static Efl_Ui_Win *win;
static Efl_Ui_Active_View_Container *container;
static int tree_abort;
static int tree_abort_level;

static void
_shutdown(void)
{
   eina_log_abort_on_critical_set(tree_abort);
   eina_log_abort_on_critical_level_set(tree_abort_level);
}

static void
_setup(void)
{
   tree_abort = eina_log_abort_on_critical_get();
   tree_abort_level = eina_log_abort_on_critical_level_get();
   eina_log_abort_on_critical_level_set(2);
   eina_log_abort_on_critical_set(1);
}
static void
active_view_setup()
{
   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC));

   container = efl_add(EFL_UI_ACTIVE_VIEW_CONTAINER_CLASS, win,
                       efl_content_set(win, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 200));
}

static void
active_view_teardown()
{
   if (win)
     {
        efl_del(win);
        win = NULL;
     }
}

typedef struct {
  struct {
    int called;
    double position;
  } position_update;
  struct {
    int called;
    Efl_Gfx_Entity *subobj;
    int index;
    int current_page_at_call;
  } content_del;
  struct {
    int called;
    Efl_Gfx_Entity *subobj;
    int index;
    int current_page_at_call;
  } content_add;
  struct {
    int called;
    Efl_Ui_Active_View_Container *active_view;
  } active_view;
} Indicator_Calls;

Indicator_Calls indicator_calls = { 0 };

static void
_indicator_content_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   indicator_calls.content_del.called ++;
   indicator_calls.content_del.subobj = subobj;
   indicator_calls.content_del.index = index;
   indicator_calls.content_del.current_page_at_call = efl_ui_active_view_active_index_get(container);
}

static void
_indicator_content_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   indicator_calls.content_add.called ++;
   indicator_calls.content_add.subobj = subobj;
   indicator_calls.content_add.index = index;
   indicator_calls.content_add.current_page_at_call = efl_ui_active_view_active_index_get(container);
}

static void
_indicator_position_update(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, double position)
{
   indicator_calls.position_update.called ++;
   indicator_calls.position_update.position = position;
}

static void
_indicator_bind(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Ui_Active_View_Container *active_view)
{
   indicator_calls.active_view.called++;
   indicator_calls.active_view.active_view = active_view;
}

EFL_OPS_DEFINE(indicator_tracker,
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_indicator_content_add, _indicator_content_add),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_indicator_content_del, _indicator_content_del),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_indicator_position_update, _indicator_position_update),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_indicator_bind, _indicator_bind),
);

static Efl_Ui_Active_View_View_Manager*
_create_indicator(void)
{
   Eo *obj;

   obj = efl_add(efl_ui_active_view_indicator_realized_class_get(), win);
   efl_object_override(obj, &indicator_tracker);

   return obj;
}

typedef struct {
  struct {
    int called;
    Efl_Gfx_Entity *subobj;
    int index;
    int current_page_at_call;
  } content_del;
  struct {
    int called;
    Efl_Gfx_Entity *subobj;
    int index;
    int current_page_at_call;
  } content_add;
  struct {
    int called;
    int from;
    int to;
  } request_switch;
  struct {
    int called;
    Eina_Size2D size;
  } page_size;
  struct {
    int called;
    Efl_Ui_Active_View_Container *active_view;
    Efl_Canvas_Group *group;
  } active_view;
  struct {
    int called;
  } animation;
  double last_position;
} Transition_Calls;

Transition_Calls transition_calls = { 0 };

static void
_emit_pos(Eo *obj, double d)
{
   if (d == transition_calls.last_position) return;
   efl_event_callback_call(obj, EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_EVENT_POS_UPDATE, &d);
   transition_calls.last_position = d;
}

static void
_transition_content_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   transition_calls.content_add.called ++;
   transition_calls.content_add.subobj = subobj;
   transition_calls.content_add.index = index;
   transition_calls.content_add.current_page_at_call = efl_ui_active_view_active_index_get(container);

   int i = efl_ui_active_view_active_index_get(container);
   if (i != -1)
     _emit_pos(obj, i);
}

static void
_transition_content_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   transition_calls.content_del.called ++;
   transition_calls.content_del.subobj = subobj;
   transition_calls.content_del.index = index;
   transition_calls.content_del.current_page_at_call = efl_ui_active_view_active_index_get(container);

   int i = efl_ui_active_view_active_index_get(container);
   if (i != -1)
     _emit_pos(obj, i);
}

static void
_transition_request_switch(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int from, int to)
{
   transition_calls.request_switch.called++;
   transition_calls.request_switch.from = from;
   transition_calls.request_switch.to = to;

   _emit_pos(obj, to);
}

static void
_transition_page_size_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Eina_Size2D size)
{
   transition_calls.page_size.called++;
   transition_calls.page_size.size = size;
}

static void
_transition_bind(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Ui_Active_View_Container *active_view, Efl_Canvas_Group *group)
{
   transition_calls.active_view.called++;
   transition_calls.active_view.active_view = active_view;
   transition_calls.active_view.group = group;
}

static void
_transition_animation_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Eina_Bool animation)
{
   transition_calls.animation.called++;
   ck_assert_int_eq(animation, EINA_TRUE);
}

static Eina_Bool
_transition_animation_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EFL_OPS_DEFINE(transition_tracker,
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_view_manager_animation_enabled_set, _transition_animation_set),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_view_manager_animation_enabled_get, _transition_animation_get),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_view_manager_content_add, _transition_content_add),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_view_manager_content_del, _transition_content_del),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_view_manager_switch_to, _transition_request_switch),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_view_manager_view_size_set, _transition_page_size_set),
  EFL_OBJECT_OP_FUNC(efl_ui_active_view_view_manager_bind, _transition_bind),
);

static Efl_Ui_Active_View_View_Manager*
_create_transition(void)
{
   Eo *obj;

   obj = efl_add(efl_ui_active_view_view_manager_realized_class_get(), win);
   efl_object_override(obj, &transition_tracker);

   return obj;
}

EFL_START_TEST (efl_ui_active_view_init)
{
   ck_assert_ptr_ne(container, NULL);
   ck_assert_ptr_ne(win, NULL);
   ck_assert_ptr_eq(efl_ui_active_view_indicator_get(container), NULL);
   ck_assert_ptr_eq(efl_ui_active_view_manager_get(container), NULL);
   Eina_Size2D s = efl_ui_active_view_size_get(container);
   ck_assert_int_eq(s.w, 0); //FIXME
   ck_assert_int_eq(s.h, 0); //FIXME
   ck_assert_int_eq(efl_ui_active_view_active_index_get(container), -1);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_active_view_active_index)
{
   Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
   efl_pack(container, w);
   ck_assert_int_eq(efl_ui_active_view_active_index_get(container), 0);

   for (int i = -20; i < 20; ++i)
     {
        if (i == 0) continue;
        EXPECT_ERROR_START;
        efl_ui_active_view_active_index_set(container, i);
        EXPECT_ERROR_END;
     }
   efl_del(w);
   ck_assert_int_eq(efl_ui_active_view_active_index_get(container), -1);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_smart_transition_calls)
{
   Efl_Ui_Widget *w, *w1, *w2;
   Efl_Ui_Active_View_View_Manager*t = _create_transition();

   w = efl_add(WIDGET_CLASS, win);
   w1 = efl_add(WIDGET_CLASS, win);
   w2 = efl_add(WIDGET_CLASS, win);

   efl_ui_active_view_manager_set(container, t);
   transition_calls.last_position = -2.0;
   ck_assert_int_eq(transition_calls.animation.called, 1);
   ck_assert_int_eq(transition_calls.active_view.called, 1);
   ck_assert_ptr_eq(transition_calls.active_view.active_view, container);
   //We cannot verify group
   ck_assert_int_eq(transition_calls.page_size.called, 1);
   ck_assert_int_eq(transition_calls.page_size.size.w, 0); //FIXME
   ck_assert_int_eq(transition_calls.page_size.size.h, 0); //FIXME
   ck_assert_int_eq(transition_calls.request_switch.called, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 0);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.active_view.called = 0;
   transition_calls.page_size.called = 0;

   //must update content, and request a switch from -1 to 1
   efl_pack(container, w);
   ck_assert_int_eq(transition_calls.active_view.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 1);
   ck_assert_int_eq(transition_calls.request_switch.from, -1);
   ck_assert_int_eq(transition_calls.request_switch.to, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 1);
   ck_assert_int_eq(transition_calls.content_add.index, 0);
   ck_assert_ptr_eq(transition_calls.content_add.subobj, w);
   ck_assert_int_eq(transition_calls.content_add.current_page_at_call, -1);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.content_add.called = 0;
   transition_calls.request_switch.called = 0;

   //this must update content and a updated current page, but no other call
   efl_pack_begin(container, w1);
   ck_assert_int_eq(transition_calls.active_view.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 1);
   ck_assert_int_eq(transition_calls.content_add.index, 0);
   ck_assert_ptr_eq(transition_calls.content_add.subobj, w1);
   ck_assert_int_eq(transition_calls.content_add.current_page_at_call, 1);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.content_add.called = 0;
   transition_calls.request_switch.called = 0;
   ck_assert_int_eq(efl_ui_active_view_active_index_get(container), 1);

   //new object, must update the content and a not update current page
   efl_pack_end(container, w2);
   ck_assert_int_eq(transition_calls.active_view.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 1);
   ck_assert_int_eq(transition_calls.content_add.index, 2);
   ck_assert_ptr_eq(transition_calls.content_add.subobj, w2);
   ck_assert_int_eq(transition_calls.content_add.current_page_at_call, 1);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.content_add.called = 0;
   ck_assert_int_eq(efl_ui_active_view_active_index_get(container), 1);

   //page change must result in a call to request a switch
   efl_ui_active_view_active_index_set(container, 2);
   ck_assert_int_eq(transition_calls.active_view.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 1);
   ck_assert_int_eq(transition_calls.request_switch.from, 1);
   ck_assert_int_eq(transition_calls.request_switch.to, 2);
   ck_assert_int_eq(transition_calls.content_add.called, 0);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.request_switch.called = 0;

   //deletion of object must result in a content update
   efl_del(w);
   ck_assert_int_eq(transition_calls.active_view.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 0);
   ck_assert_int_eq(transition_calls.content_del.called, 1);
   ck_assert_int_eq(transition_calls.content_del.index, 1);
   ck_assert_ptr_eq(transition_calls.content_del.subobj, w);
   ck_assert_int_eq(transition_calls.content_del.current_page_at_call, 1);
   transition_calls.content_del.called = 0;
}
EFL_END_TEST


EFL_START_TEST (efl_ui_smart_transition_lifetime)
{
   Efl_Ui_Active_View_View_Manager*t, *t1;

   t = _create_transition();
   efl_wref_add(t, &t);
   t1 = _create_transition();
   efl_wref_add(t1, &t1);

   efl_ui_active_view_manager_set(container, t);
   efl_ui_active_view_manager_set(container, t1);
   ck_assert_ptr_eq(t, NULL);
}
EFL_END_TEST

static void
_verify_indicator_calls(void)
{
   Efl_Ui_Widget *w, *w1, *w2;

   w = efl_add(WIDGET_CLASS, win);
   w1 = efl_add(WIDGET_CLASS, win);
   w2 = efl_add(WIDGET_CLASS, win);

   ck_assert_int_eq(indicator_calls.active_view.called, 1);
   ck_assert_ptr_eq(indicator_calls.active_view.active_view, container);
   ck_assert_int_eq(indicator_calls.content_add.called, 0);
   ck_assert_int_eq(indicator_calls.content_del.called, 0);
   ck_assert_int_eq(indicator_calls.position_update.called, 0);
   indicator_calls.active_view.called = 0;

   efl_pack(container, w);
   ck_assert_int_eq(indicator_calls.active_view.called, 0);
   ck_assert_int_eq(indicator_calls.content_add.called, 1);
   ck_assert_int_eq(indicator_calls.content_add.index, 0);
   ck_assert_ptr_eq(indicator_calls.content_add.subobj, w);
   ck_assert_int_eq(indicator_calls.content_del.called, 0);
   ck_assert_int_eq(indicator_calls.position_update.called, 1);
   ck_assert(indicator_calls.position_update.position == 0.0);
   indicator_calls.content_add.called = 0;
   indicator_calls.position_update.called = 0;

   efl_pack_begin(container, w1);
   ck_assert_int_eq(indicator_calls.active_view.called, 0);
   ck_assert_int_eq(indicator_calls.content_add.called, 1);
   ck_assert_int_eq(indicator_calls.content_add.index, 0);
   ck_assert_ptr_eq(indicator_calls.content_add.subobj, w1);
   ck_assert_int_eq(indicator_calls.content_del.called, 0);
   ck_assert_int_eq(indicator_calls.position_update.called, 1);
   ck_assert(indicator_calls.position_update.position == 1.0);
   indicator_calls.content_add.called = 0;
   indicator_calls.position_update.called = 0;

   efl_pack_end(container, w2);
   ck_assert_int_eq(indicator_calls.active_view.called, 0);
   ck_assert_int_eq(indicator_calls.content_add.called, 1);
   ck_assert_int_eq(indicator_calls.content_add.index, 2);
   ck_assert_ptr_eq(indicator_calls.content_add.subobj, w2);
   ck_assert_int_eq(indicator_calls.content_del.called, 0);
   ck_assert_int_eq(indicator_calls.position_update.called, 0);
   indicator_calls.content_add.called = 0;
   indicator_calls.position_update.called = 0;

   efl_del(w1);
   ck_assert_int_eq(indicator_calls.active_view.called, 0);
   ck_assert_int_eq(indicator_calls.content_add.called, 0);
   ck_assert_int_eq(indicator_calls.content_del.called, 1);
   ck_assert_int_eq(indicator_calls.content_del.index, 0);
   ck_assert_ptr_eq(indicator_calls.content_del.subobj, w1);
   ck_assert_int_eq(indicator_calls.position_update.called, 1);
   ck_assert(indicator_calls.position_update.position == 0.0);
   indicator_calls.content_del.called = 0;
   indicator_calls.position_update.called = 0;
}

EFL_START_TEST (efl_ui_smart_indicator_calls)
{
   Efl_Ui_Active_View_View_Manager*i = _create_indicator();
   efl_ui_active_view_indicator_set(container, i);
   _verify_indicator_calls();
}
EFL_END_TEST

EFL_START_TEST (efl_ui_smart_indicator_transition_calls)
{
   Efl_Ui_Active_View_View_Manager *i = _create_indicator();
   Efl_Ui_Active_View_View_Manager *t = _create_transition();

   transition_calls.last_position = -2.0;
   efl_ui_active_view_indicator_set(container, i);
   efl_ui_active_view_manager_set(container, t);
   _verify_indicator_calls();
}
EFL_END_TEST

Efl_Ui_Active_View_Transition_Event start;
Efl_Ui_Active_View_Transition_Event end;

static void
_start(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Active_View_Transition_Event *e = ev->info;

   memcpy(&start, e, sizeof(Efl_Ui_Active_View_Transition_Event));
}

static void
_end(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Active_View_Transition_Event *e = ev->info;

   memcpy(&end, e, sizeof(Efl_Ui_Active_View_Transition_Event));
}

#define EV_RESET \
   start.from = -8; \
   start.to = -8; \
   end.from = -8; \
   end.to = -8;

static void
_verify_transition_start_end_events(void)
{
   Efl_Ui_Widget *w, *w1, *w2;

   w = efl_add(WIDGET_CLASS, win);
   w1 = efl_add(WIDGET_CLASS, win);
   w2 = efl_add(WIDGET_CLASS, win);

   efl_event_callback_add(container, EFL_UI_ACTIVE_VIEW_EVENT_TRANSITION_START, _start, NULL);
   efl_event_callback_add(container, EFL_UI_ACTIVE_VIEW_EVENT_TRANSITION_END, _end, NULL);

   EV_RESET
   efl_pack_end(container, w);
   ck_assert_int_eq(start.to, 0);
   ck_assert_int_eq(end.to, 0);
   ck_assert_int_eq(start.from, -1);
   ck_assert_int_eq(end.from, -1);

   EV_RESET
   efl_pack_begin(container, w1);
   efl_pack_end(container, w2);
   ck_assert_int_eq(start.to, -8);
   ck_assert_int_eq(end.to, -8);
   ck_assert_int_eq(start.from, -8);
   ck_assert_int_eq(end.from, -8);

   EV_RESET
   efl_ui_active_view_active_index_set(container, 2);
   ck_assert_int_eq(start.to, 2);
   ck_assert_int_eq(end.to, 2);
   ck_assert_int_eq(start.from, 1);
   ck_assert_int_eq(end.from, 1);
}

EFL_START_TEST (efl_ui_active_view_view_manager_start_end)
{
   transition_calls.last_position = -2.0;

   _verify_transition_start_end_events();
}
EFL_END_TEST

EFL_START_TEST (efl_ui_active_view_active_index_not_update)
{
   efl_ui_active_view_gravity_set(container, EFL_UI_ACTIVE_VIEW_CONTAINER_GRAVITY_INDEX);

   for (int i = 0; i < 5; ++i)
     {
        Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
        efl_pack(container, w);
        ck_assert_int_eq(efl_ui_active_view_active_index_get(container), 0);
     }

   efl_del(efl_pack_content_get(container, 0));
   ck_assert_int_eq(efl_ui_active_view_active_index_get(container), 0);
}
EFL_END_TEST

void efl_ui_test_active_view(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _shutdown);
   tcase_add_checked_fixture(tc, active_view_setup, active_view_teardown);
   tcase_add_test(tc, efl_ui_active_view_init);
   tcase_add_test(tc, efl_ui_active_view_active_index);
   tcase_add_test(tc, efl_ui_smart_transition_calls);
   tcase_add_test(tc, efl_ui_smart_transition_lifetime);
   tcase_add_test(tc, efl_ui_smart_indicator_calls);
   tcase_add_test(tc, efl_ui_smart_indicator_transition_calls);
   tcase_add_test(tc, efl_ui_active_view_view_manager_start_end);
   tcase_add_test(tc, efl_ui_active_view_active_index_not_update);
}
