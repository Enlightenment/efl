#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "eo_internal.h"

EFL_CLASS_SIMPLE_CLASS(efl_ui_spotlight_manager, "Efl.Ui.Spotlight.Manager", EFL_UI_SPOTLIGHT_MANAGER_CLASS);
EFL_CLASS_SIMPLE_CLASS(efl_ui_spotlight_indicator, "Efl.Ui.Spotlight.Indicator", EFL_UI_SPOTLIGHT_INDICATOR_CLASS);

static Efl_Ui_Win *win;
static Efl_Ui_Spotlight_Container *container;

typedef struct {
  struct {
    int called;
    double position;
  } position_update;
  struct {
    int called;
    Efl_Gfx_Entity *subobj;
    int index;
    Efl_Ui_Widget *current_page_at_call;
  } content_del;
  struct {
    int called;
    Efl_Gfx_Entity *subobj;
    int index;
    Efl_Ui_Widget *current_page_at_call;
  } content_add;
  struct {
    int called;
    Efl_Ui_Spotlight_Container *spotlight;
  } spotlight;
} Indicator_Calls;

Indicator_Calls indicator_calls = { 0 };

static void
_indicator_content_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   indicator_calls.content_del.called ++;
   indicator_calls.content_del.subobj = subobj;
   indicator_calls.content_del.index = index;
   indicator_calls.content_del.current_page_at_call = efl_ui_spotlight_active_element_get(container);
}

static void
_indicator_content_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   indicator_calls.content_add.called ++;
   indicator_calls.content_add.subobj = subobj;
   indicator_calls.content_add.index = index;
   indicator_calls.content_add.current_page_at_call = efl_ui_spotlight_active_element_get(container);
}

static void
_indicator_position_update(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, double position)
{
   indicator_calls.position_update.called ++;
   indicator_calls.position_update.position = position;
}

static void
_indicator_bind(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Ui_Spotlight_Container *spotlight)
{
   indicator_calls.spotlight.called++;
   indicator_calls.spotlight.spotlight = spotlight;
}

EFL_OPS_DEFINE(indicator_tracker,
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_indicator_content_add, _indicator_content_add),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_indicator_content_del, _indicator_content_del),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_indicator_position_update, _indicator_position_update),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_indicator_bind, _indicator_bind),
);

static Efl_Ui_Spotlight_Manager*
_create_indicator(void)
{
   Eo *obj;

   obj = efl_new(efl_ui_spotlight_indicator_realized_class_get());
   efl_object_override(obj, &indicator_tracker);

   return obj;
}

typedef struct {
  struct {
    int called;
    Efl_Gfx_Entity *subobj;
    int index;
    Efl_Ui_Widget *current_page_at_call;
  } content_del;
  struct {
    int called;
    Efl_Gfx_Entity *subobj;
    int index;
    Efl_Ui_Widget *current_page_at_call;
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
    Efl_Ui_Spotlight_Container *spotlight;
    Efl_Canvas_Group *group;
  } spotlight;
  struct {
    int called;
    Eina_Bool value;
  } animation;
  double last_position;
} Transition_Calls;

Transition_Calls transition_calls = { 0 };

static void
_emit_pos(Eo *obj, double d)
{
   printf("EMITTING %f %f\n", d, transition_calls.last_position);
   if (EINA_DBL_EQ(d, transition_calls.last_position)) return;

   efl_event_callback_call(obj, EFL_UI_SPOTLIGHT_MANAGER_EVENT_POS_UPDATE, &d);
   transition_calls.last_position = d;
}

static void
_transition_content_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   transition_calls.content_add.called ++;
   transition_calls.content_add.subobj = subobj;
   transition_calls.content_add.index = index;
   transition_calls.content_add.current_page_at_call = efl_ui_spotlight_active_element_get(container);

   int i = efl_pack_index_get(container, efl_ui_spotlight_active_element_get(container));
   if (i != -1)
     _emit_pos(obj, i);
}

static void
_transition_content_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Gfx_Entity *subobj, int index EINA_UNUSED)
{
   transition_calls.content_del.called ++;
   transition_calls.content_del.subobj = subobj;
   transition_calls.content_del.index = index;
   transition_calls.content_del.current_page_at_call = efl_ui_spotlight_active_element_get(container);

   int i = efl_pack_index_get(container, efl_ui_spotlight_active_element_get(container));
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
_transition_bind(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Ui_Spotlight_Container *spotlight, Efl_Canvas_Group *group)
{
   transition_calls.spotlight.called++;
   transition_calls.spotlight.spotlight = spotlight;
   transition_calls.spotlight.group = group;
}

static void
_transition_animation_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Eina_Bool animation)
{
   transition_calls.animation.called++;
   transition_calls.animation.value = animation;
}

static Eina_Bool
_transition_animation_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EFL_OPS_DEFINE(transition_tracker,
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_manager_animated_transition_set, _transition_animation_set),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_manager_animated_transition_get, _transition_animation_get),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_manager_content_add, _transition_content_add),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_manager_content_del, _transition_content_del),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_manager_switch_to, _transition_request_switch),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_manager_size_set, _transition_page_size_set),
  EFL_OBJECT_OP_FUNC(efl_ui_spotlight_manager_bind, _transition_bind),
);

static Efl_Ui_Spotlight_Manager*
_create_transition(void)
{
   Eo *obj;

   obj = efl_new(efl_ui_spotlight_manager_realized_class_get());
   efl_object_override(obj, &transition_tracker);

   return obj;
}

EFL_START_TEST (efl_ui_spotlight_init)
{
   ck_assert_ptr_ne(container, NULL);
   ck_assert_ptr_ne(win, NULL);
   ck_assert_ptr_eq(efl_ui_spotlight_indicator_get(container), NULL);
   ck_assert_ptr_eq(efl_ui_spotlight_manager_get(container), NULL);
   Eina_Size2D s = efl_ui_spotlight_size_get(container);
   ck_assert_int_eq(s.w, 0); //FIXME
   ck_assert_int_eq(s.h, 0); //FIXME
   ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), NULL);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_spotlight_active_index)
{
   Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
   efl_pack(container, w);

   ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), w);
   EXPECT_ERROR_START;
   efl_ui_spotlight_active_element_set(container, 0x0);
   EXPECT_ERROR_END;
   ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), w);
   EXPECT_ERROR_START;
   efl_ui_spotlight_active_element_set(container, (void*)0xAFFE);
   EXPECT_ERROR_END;
   ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), w);
   EXPECT_ERROR_START;
   efl_ui_spotlight_active_element_set(container, efl_main_loop_get());
   EXPECT_ERROR_END;
   ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), w);

   efl_del(w);
   ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), NULL);
}
EFL_END_TEST

static void
_verify_transition_calls(int number_of_animation_calls, Eina_Bool animation_value)
{
   Efl_Ui_Widget *w, *w1, *w2;

   w = efl_add(WIDGET_CLASS, win);
   w1 = efl_add(WIDGET_CLASS, win);
   w2 = efl_add(WIDGET_CLASS, win);

   transition_calls.last_position = -2.0;
   ck_assert_int_eq(transition_calls.animation.called, number_of_animation_calls);
   ck_assert_int_eq(transition_calls.animation.value, animation_value);
   ck_assert_int_eq(transition_calls.spotlight.called, 1);
   ck_assert_ptr_eq(transition_calls.spotlight.spotlight, container);
   //We cannot verify group
   ck_assert_int_eq(transition_calls.page_size.called, 1);
   ck_assert_int_eq(transition_calls.page_size.size.w, 0); //FIXME
   ck_assert_int_eq(transition_calls.page_size.size.h, 0); //FIXME
   ck_assert_int_eq(transition_calls.request_switch.called, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 0);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.spotlight.called = 0;
   transition_calls.page_size.called = 0;

   //must update content, and request a switch from -1 to 1
   efl_pack(container, w);
   ck_assert_int_eq(transition_calls.spotlight.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 1);
   ck_assert_int_eq(transition_calls.request_switch.from, -1);
   ck_assert_int_eq(transition_calls.request_switch.to, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 1);
   ck_assert_int_eq(transition_calls.content_add.index, 0);
   ck_assert_ptr_eq(transition_calls.content_add.subobj, w);
   ck_assert_ptr_eq(transition_calls.content_add.current_page_at_call, NULL);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.content_add.called = 0;
   transition_calls.request_switch.called = 0;

   //this must update content and a updated current page, but no other call
   efl_pack_begin(container, w1);
   ck_assert_int_eq(transition_calls.spotlight.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 1);
   ck_assert_int_eq(transition_calls.content_add.index, 0);
   ck_assert_ptr_eq(transition_calls.content_add.subobj, w1);
   ck_assert_ptr_eq(transition_calls.content_add.current_page_at_call, w);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.content_add.called = 0;
   transition_calls.request_switch.called = 0;
   ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), w);

   //new object, must update the content and a not update current page
   efl_pack_end(container, w2);
   ck_assert_int_eq(transition_calls.spotlight.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 1);
   ck_assert_int_eq(transition_calls.content_add.index, 2);
   ck_assert_ptr_eq(transition_calls.content_add.subobj, w2);
   ck_assert_ptr_eq(transition_calls.content_add.current_page_at_call, w);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.content_add.called = 0;
   ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), w);

   //page change must result in a call to request a switch
   efl_ui_spotlight_active_element_set(container, w2);
   ck_assert_int_eq(transition_calls.spotlight.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 1);
   ck_assert_int_eq(transition_calls.request_switch.from, 1);
   ck_assert_int_eq(transition_calls.request_switch.to, 2);
   ck_assert_int_eq(transition_calls.content_add.called, 0);
   ck_assert_int_eq(transition_calls.content_del.called, 0);
   transition_calls.request_switch.called = 0;

   //deletion of object must result in a content update
   efl_del(w);
   ck_assert_int_eq(transition_calls.spotlight.called, 0);
   ck_assert_int_eq(transition_calls.page_size.called, 0);
   ck_assert_int_eq(transition_calls.request_switch.called, 0);
   ck_assert_int_eq(transition_calls.content_add.called, 0);
   ck_assert_int_eq(transition_calls.content_del.called, 1);
   ck_assert_int_eq(transition_calls.content_del.index, 1);
   ck_assert_ptr_eq(transition_calls.content_del.subobj, w);
   ck_assert_ptr_eq(transition_calls.content_del.current_page_at_call, w2);
   transition_calls.content_del.called = 0;
}

EFL_START_TEST (efl_ui_smart_transition_calls)
{
   Efl_Ui_Spotlight_Manager*t = _create_transition();
   efl_ui_spotlight_manager_set(container, t);

   _verify_transition_calls(2, EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_smart_transition_calls_no_animation)
{
   Efl_Ui_Spotlight_Manager*t = _create_transition();
   efl_ui_spotlight_manager_set(container, t);
   efl_ui_spotlight_animated_transition_set(container, EINA_FALSE);

   _verify_transition_calls(3, EINA_FALSE);
}
EFL_END_TEST


EFL_START_TEST (efl_ui_smart_transition_lifetime)
{
   Efl_Ui_Spotlight_Manager*t, *t1;

   t = _create_transition();
   efl_wref_add(t, &t);
   t1 = _create_transition();

   efl_ui_spotlight_manager_set(container, t);
   efl_ui_spotlight_manager_set(container, t1);
   ck_assert_ptr_eq(t, NULL);
   ck_assert_ptr_ne(t1, NULL);
}
EFL_END_TEST

static void
_verify_indicator_calls(void)
{
   Efl_Ui_Widget *w, *w1, *w2;

   w = efl_add(WIDGET_CLASS, win);
   w1 = efl_add(WIDGET_CLASS, win);
   w2 = efl_add(WIDGET_CLASS, win);

   ck_assert_int_eq(indicator_calls.spotlight.called, 1);
   ck_assert_ptr_eq(indicator_calls.spotlight.spotlight, container);
   ck_assert_int_eq(indicator_calls.content_add.called, 0);
   ck_assert_int_eq(indicator_calls.content_del.called, 0);
   ck_assert_int_eq(indicator_calls.position_update.called, 0);
   indicator_calls.spotlight.called = 0;

   efl_pack(container, w);
   ck_assert_int_eq(indicator_calls.spotlight.called, 0);
   ck_assert_int_eq(indicator_calls.content_add.called, 1);
   ck_assert_int_eq(indicator_calls.content_add.index, 0);
   ck_assert_ptr_eq(indicator_calls.content_add.subobj, w);
   ck_assert_int_eq(indicator_calls.content_del.called, 0);
   ck_assert_int_eq(indicator_calls.position_update.called, 1);
   ck_assert(EINA_DBL_EQ(indicator_calls.position_update.position, 0.0));
   indicator_calls.content_add.called = 0;
   indicator_calls.position_update.called = 0;

   efl_pack_begin(container, w1);
   ck_assert_int_eq(indicator_calls.spotlight.called, 0);
   ck_assert_int_eq(indicator_calls.content_add.called, 1);
   ck_assert_int_eq(indicator_calls.content_add.index, 0);
   ck_assert_ptr_eq(indicator_calls.content_add.subobj, w1);
   ck_assert_int_eq(indicator_calls.content_del.called, 0);
   ck_assert_int_eq(indicator_calls.position_update.called, 1);
   ck_assert(EINA_DBL_EQ(indicator_calls.position_update.position, 1.0));
   indicator_calls.content_add.called = 0;
   indicator_calls.position_update.called = 0;

   efl_pack_end(container, w2);
   ck_assert_int_eq(indicator_calls.spotlight.called, 0);
   ck_assert_int_eq(indicator_calls.content_add.called, 1);
   ck_assert_int_eq(indicator_calls.content_add.index, 2);
   ck_assert_ptr_eq(indicator_calls.content_add.subobj, w2);
   ck_assert_int_eq(indicator_calls.content_del.called, 0);
   ck_assert_int_eq(indicator_calls.position_update.called, 0);
   indicator_calls.content_add.called = 0;
   indicator_calls.position_update.called = 0;

   efl_del(w1);
   ck_assert_int_eq(indicator_calls.spotlight.called, 0);
   ck_assert_int_eq(indicator_calls.content_add.called, 0);
   ck_assert_int_eq(indicator_calls.content_del.called, 1);
   ck_assert_int_eq(indicator_calls.content_del.index, 0);
   ck_assert_ptr_eq(indicator_calls.content_del.subobj, w1);
   ck_assert_int_eq(indicator_calls.position_update.called, 1);
   ck_assert(EINA_DBL_EQ(indicator_calls.position_update.position, 0.0));
   indicator_calls.content_del.called = 0;
   indicator_calls.position_update.called = 0;
}

EFL_START_TEST (efl_ui_smart_indicator_calls)
{
   Efl_Ui_Spotlight_Manager*i = _create_indicator();
   efl_ui_spotlight_indicator_set(container, i);
   _verify_indicator_calls();
   ck_assert_ptr_ne(i, NULL);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_smart_indicator_transition_calls)
{
   Efl_Ui_Spotlight_Manager *i = _create_indicator();
   Efl_Ui_Spotlight_Manager *t = _create_transition();

   transition_calls.last_position = -2.0;
   efl_ui_spotlight_indicator_set(container, i);
   efl_ui_spotlight_manager_set(container, t);
   _verify_indicator_calls();
   ck_assert_ptr_ne(i, NULL);
   ck_assert_ptr_ne(t, NULL);
}
EFL_END_TEST

Efl_Ui_Spotlight_Transition_Event start;
Efl_Ui_Spotlight_Transition_Event end;

static void
_start(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Spotlight_Transition_Event *e = ev->info;

   memcpy(&start, e, sizeof(Efl_Ui_Spotlight_Transition_Event));
}

static void
_end(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Spotlight_Transition_Event *e = ev->info;

   memcpy(&end, e, sizeof(Efl_Ui_Spotlight_Transition_Event));
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

   efl_event_callback_add(container, EFL_UI_SPOTLIGHT_EVENT_TRANSITION_START, _start, NULL);
   efl_event_callback_add(container, EFL_UI_SPOTLIGHT_EVENT_TRANSITION_END, _end, NULL);

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
   efl_ui_spotlight_active_element_set(container, w2);
   ck_assert_int_eq(start.to, 2);
   ck_assert_int_eq(end.to, 2);
   ck_assert_int_eq(start.from, 1);
   ck_assert_int_eq(end.from, 1);
}

EFL_START_TEST (efl_ui_spotlight_manager_start_end)
{
   transition_calls.last_position = -2.0;

   _verify_transition_start_end_events();
}
EFL_END_TEST

EFL_START_TEST (efl_ui_spotlight_test_push1)
{
   for (int i = 0; i < 5; ++i)
     {
        Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
        efl_pack_end(container, w);
     }
    Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
    efl_ui_spotlight_push(container, w);
    ck_assert_int_eq(efl_pack_index_get(container, w), 1);
    ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), efl_pack_content_get(container, 1));
}
EFL_END_TEST

EFL_START_TEST (efl_ui_spotlight_test_push2)
{
   for (int i = 0; i < 5; ++i)
     {
        Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
        efl_pack_end(container, w);
        if (i == 3)
          efl_ui_spotlight_active_element_set(container, w);
     }
    Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
    efl_ui_spotlight_push(container, w);
    ck_assert_int_eq(efl_pack_index_get(container, w), 4);
    ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), w);
}
EFL_END_TEST

static Eina_Value
_then_cb(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Eo **value = data;

   *value = eina_value_object_get(&v);

   return EINA_VALUE_EMPTY;
}

EFL_START_TEST (efl_ui_spotlight_test_pop1)
{
   Eo *called;
   for (int i = 0; i < 5; ++i)
     {
        Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
        efl_pack_end(container, w);
     }
    Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
    efl_ui_spotlight_push(container, w);
    Eina_Future *f = efl_ui_spotlight_pop(container, EINA_FALSE);
    eina_future_then(f, _then_cb, &called);
    for (int i = 0; i < 10; ++i)
      {
         efl_loop_iterate(efl_provider_find(container, EFL_LOOP_CLASS));
      }
    ck_assert_ptr_eq(efl_ui_widget_parent_get(w), win);
    ck_assert_int_eq(efl_content_count(container), 5);
    ck_assert_ptr_eq(called, w);
    ck_assert_ptr_ne(f, NULL);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_spotlight_test_pop2)
{
   for (int i = 0; i < 5; ++i)
     {
        Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
        efl_pack_end(container, w);
     }
    Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
    efl_ui_spotlight_push(container, w);
    Eina_Future *f = efl_ui_spotlight_pop(container, EINA_TRUE);
    for (int i = 0; i < 10; ++i)
      {
         efl_loop_iterate(efl_provider_find(container, EFL_LOOP_CLASS));
      }
    ck_assert_int_eq(efl_ref_count(w), 0);
    ck_assert_int_eq(efl_content_count(container), 5);
    ck_assert_ptr_ne(f, NULL);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_spotlight_test_pop3)
{
   for (int i = 0; i < 5; ++i)
     {
        Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
        efl_pack_end(container, w);
        if (i == 3)
          efl_ui_spotlight_active_element_set(container, w);
     }
    Efl_Ui_Widget *w = efl_add(WIDGET_CLASS, win);
    efl_ui_spotlight_push(container, w);
    Eina_Future *f = efl_ui_spotlight_pop(container, EINA_TRUE);
    for (int i = 0; i < 10; ++i)
      {
         efl_loop_iterate(efl_provider_find(container, EFL_LOOP_CLASS));
      }
    ck_assert_ptr_eq(efl_ui_spotlight_active_element_get(container), efl_pack_content_get(container, 3));
    ck_assert_int_eq(efl_ref_count(w), 0);
    ck_assert_int_eq(efl_content_count(container), 5);
    ck_assert_ptr_ne(f, NULL);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_spotlight_animated_transition)
{
   //this checks animation handing with the plain manager, there is no animation that will be played, but the flag should be preserved
   ck_assert_int_eq(efl_ui_spotlight_animated_transition_get(container), EINA_TRUE);
   efl_ui_spotlight_animated_transition_set(container, EINA_FALSE);
   ck_assert_int_eq(efl_ui_spotlight_manager_animated_transition_get(efl_ui_spotlight_manager_get(container)), EINA_FALSE);

   //now check with a real spotlight manager
   efl_ui_spotlight_manager_set(container, efl_new(EFL_UI_SPOTLIGHT_FADE_MANAGER_CLASS));
   efl_ui_spotlight_animated_transition_set(container, EINA_TRUE);
   ck_assert_int_eq(efl_ui_spotlight_animated_transition_get(container), EINA_TRUE);
   ck_assert_int_eq(efl_ui_spotlight_manager_animated_transition_get(efl_ui_spotlight_manager_get(container)), EINA_TRUE);

   efl_ui_spotlight_animated_transition_set(container, EINA_FALSE);
   ck_assert_int_eq(efl_ui_spotlight_animated_transition_get(container), EINA_FALSE);
   ck_assert_int_eq(efl_ui_spotlight_manager_animated_transition_get(efl_ui_spotlight_manager_get(container)), EINA_FALSE);

   Eo *manager2 = efl_new(EFL_UI_SPOTLIGHT_FADE_MANAGER_CLASS);
   Eina_Bool animated_transition_manager;
   efl_add(EFL_UI_SPOTLIGHT_CONTAINER_CLASS, win,
    efl_ui_spotlight_manager_set(efl_added, manager2),
    efl_ui_spotlight_animated_transition_set(efl_added, EINA_TRUE),
    animated_transition_manager = efl_ui_spotlight_manager_animated_transition_get(manager2));
   ck_assert_int_eq(animated_transition_manager, EINA_FALSE);
   ck_assert_int_eq(efl_ui_spotlight_manager_animated_transition_get(manager2), EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST (efl_ui_spotlight_min_max_sizing)
{
   Efl_Ui_Button *btn0, *btn1;
   Eina_Size2D min, size;

   btn0 = efl_add(WIDGET_CLASS, container);
   efl_gfx_hint_size_min_set(btn0, EINA_SIZE2D(20, 200));

   btn1 = efl_add(WIDGET_CLASS, container);
   efl_gfx_hint_size_min_set(btn1, EINA_SIZE2D(200, 20));

   efl_pack_end(container, btn0);
   efl_pack_end(container, btn1);
   min = efl_gfx_hint_size_restricted_min_get(container);
   ck_assert_int_eq(min.w, 200);
   ck_assert_int_eq(min.h, 200);

   efl_gfx_hint_size_min_set(btn0, EINA_SIZE2D(20, 300));
   efl_canvas_group_calculate(container);

   min = efl_gfx_hint_size_restricted_min_get(container);
   ck_assert_int_eq(min.w, 200);
   ck_assert_int_eq(min.h, 300);

   efl_gfx_hint_size_min_set(btn0, EINA_SIZE2D(20, 20));
   efl_canvas_group_calculate(container);

   min = efl_gfx_hint_size_restricted_min_get(container);
   ck_assert_int_eq(min.w, 200);
   ck_assert_int_eq(min.h, 20);

   efl_ui_spotlight_size_set(container, EINA_SIZE2D(2000, 2000));

   efl_gfx_entity_size_set(container, EINA_SIZE2D(200, 200));
   size = efl_gfx_entity_size_get(btn0);
   ck_assert_int_eq(size.w, 200);
   ck_assert_int_eq(size.h, 200);

}
EFL_END_TEST

static void
spotlight_setup()
{
   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC));

   container = efl_add(EFL_UI_SPOTLIGHT_CONTAINER_CLASS, win,
                       efl_content_set(win, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 200));
}

static void
spotlight_teardown()
{
   memset(&transition_calls, 0, sizeof(transition_calls));
   memset(&indicator_calls, 0, sizeof(indicator_calls));
}

void efl_ui_test_spotlight(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, spotlight_setup, spotlight_teardown);
   tcase_add_test(tc, efl_ui_spotlight_init);
   tcase_add_test(tc, efl_ui_spotlight_active_index);
   tcase_add_test(tc, efl_ui_smart_transition_calls);
   tcase_add_test(tc, efl_ui_smart_transition_calls_no_animation);
   tcase_add_test(tc, efl_ui_smart_transition_lifetime);
   tcase_add_test(tc, efl_ui_smart_indicator_calls);
   tcase_add_test(tc, efl_ui_smart_indicator_transition_calls);
   tcase_add_test(tc, efl_ui_spotlight_manager_start_end);
   tcase_add_test(tc, efl_ui_spotlight_test_push1);
   tcase_add_test(tc, efl_ui_spotlight_test_push2);
   tcase_add_test(tc, efl_ui_spotlight_test_pop1);
   tcase_add_test(tc, efl_ui_spotlight_test_pop2);
   tcase_add_test(tc, efl_ui_spotlight_test_pop3);
   tcase_add_test(tc, efl_ui_spotlight_animated_transition);
   tcase_add_test(tc, efl_ui_spotlight_min_max_sizing);
}
