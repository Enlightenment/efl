/* EFL - EFL library
 * Copyright (C) 2013 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "efl_ui_suite.h"

static const int child_number = 3;
static const int base_ints[] = { 41, 42, 43 };
static const Eina_Bool base_selections[] = { EINA_FALSE, EINA_FALSE, EINA_TRUE };

static Eina_Value
_selection_children_slice_get_then(void *data EINA_UNUSED,
                                   const Eina_Value v,
                                   const Eina_Future *dead_future EINA_UNUSED)
{
   unsigned int i, len;
   Efl_Model *child = NULL;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *p_int = NULL;
        Eina_Value *p_bool = NULL;
        Eina_Value *p_index = NULL;
        int v_int = 0;
        unsigned int index = 0;
        Eina_Bool v_bool = EINA_FALSE;

        p_bool = efl_model_property_get(child, "selected");
        p_int = efl_model_property_get(child, "test_p_int");
        p_index = efl_model_property_get(child, "child.index");

        eina_value_get(p_bool, &v_bool);
        eina_value_get(p_int, &v_int);
        fail_if(!eina_value_uint_convert(p_index, &index));

        fail_if(v_bool != base_selections[i]);
        fail_if(v_int != base_ints[i]);
        ck_assert_int_eq(i, index);

        eina_value_free(p_bool);
        eina_value_free(p_int);
        eina_value_free(p_index);
     }

   ecore_main_loop_quit();

   return v;
}

static Eina_Value
_wait_propagate(void *data EINA_UNUSED,
                const Eina_Value v,
                const Eina_Future *dead_future EINA_UNUSED)
{
   ecore_main_loop_quit();
   return v;
}

EFL_START_TEST(efl_test_select_model)
{
   Efl_Generic_Model *base_model, *child;
   int i;
   Eina_Value v = { 0 };
   Efl_Ui_Select_Model *model;
   Eina_Future *future;
   Eina_Iterator *it;
   unsigned int *index;

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);

   for (i = 0; i < child_number; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, base_ints[i]));
        efl_model_property_set(child, "test_p_int", &v);
     }

   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                   efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   future = efl_model_property_ready_get(model, "child.selected");
   eina_future_then(future, _wait_propagate, NULL, NULL);

   efl_model_property_set(model, "child.selected", eina_value_int_new(2));
   ecore_main_loop_begin();

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   eina_future_then(future, _selection_children_slice_get_then, NULL, NULL);

   ecore_main_loop_begin();

   it = efl_ui_multi_selectable_selected_ndx_iterator_new(model);
   EINA_ITERATOR_FOREACH(it, index)
     fail_if(*index != 2);
   eina_iterator_free(it);

   it = efl_ui_multi_selectable_unselected_ndx_iterator_new(model);
   EINA_ITERATOR_FOREACH(it, index)
     fail_if(*index == 2);
   eina_iterator_free(it);

   efl_model_property_set(model, "child.selected", eina_value_int_new(1));

   // both were made with efl_add_ref: the refs are the test's to give back
   efl_unref(model);
   efl_unref(base_model);
   eina_value_flush(&v);
}
EFL_END_TEST

/* The first child, selected and then asked about.
 *
 * Selection is a bit per child in @Efl.Boolean_Model and reading it back is
 * that model's iterator, which walked from one rather than from zero: a
 * selection that included the first child came back one short and every
 * other answer was right, so nothing noticed. Which child is picked first is
 * not an unusual thing for a person to do.
 *
 * All three are selected here rather than only the first, so that a count
 * one short is a failure and not an empty answer that could be anything. */
EFL_START_TEST(efl_test_select_model_selects_the_first)
{
   Efl_Model *base_model, *model, *child;
   Eina_Value v = EINA_VALUE_EMPTY;
   Eina_Iterator *it;
   unsigned int *index, seen = 0, i;
   Eina_Bool got[3] = { EINA_FALSE, EINA_FALSE, EINA_FALSE };

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);

   for (i = 0; i < 3; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, (int) i));
        efl_model_property_set(child, "test_p_int", &v);
     }

   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);

   efl_ui_multi_selectable_ndx_range_select(model, 0, 2);
   /* One future per child, so more than one turn of the loop. */
   for (i = 0; i < 8; i++) ecore_main_loop_iterate();

   it = efl_ui_multi_selectable_selected_ndx_iterator_new(model);
   ck_assert_ptr_ne(it, NULL);
   EINA_ITERATOR_FOREACH(it, index)
     {
        ck_assert_int_lt(*index, 3);
        got[*index] = EINA_TRUE;
        seen++;
     }
   eina_iterator_free(it);

   ck_assert_int_eq(seen, 3);
   for (i = 0; i < 3; i++) ck_assert_int_eq(got[i], EINA_TRUE);

   /* And the other way round: with everything selected, nothing is not. */
   it = efl_ui_multi_selectable_unselected_ndx_iterator_new(model);
   ck_assert_ptr_ne(it, NULL);
   seen = 0;
   EINA_ITERATOR_FOREACH(it, index) seen++;
   eina_iterator_free(it);
   ck_assert_int_eq(seen, 0);

   eina_value_flush(&v);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

/* A range given high end first is the same range, and unselecting
 * everything in an empty model is nothing to do. */
EFL_START_TEST(efl_test_select_model_reversed_range)
{
   Efl_Model *base_model, *model, *empty_base, *empty;
   Eina_Iterator *it;
   unsigned int *index, seen = 0, i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 3; ++i)
     ck_assert(!!efl_model_child_add(base_model));

   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);

   efl_ui_multi_selectable_ndx_range_select(model, 2, 0);
   for (i = 0; i < 8; i++) ecore_main_loop_iterate();

   it = efl_ui_multi_selectable_selected_ndx_iterator_new(model);
   EINA_ITERATOR_FOREACH(it, index) seen++;
   eina_iterator_free(it);
   ck_assert_int_eq(seen, 3);

   empty_base = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   empty = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, empty_base));
   efl_ui_multi_selectable_all_unselect(empty);
   for (i = 0; i < 8; i++) ecore_main_loop_iterate();

   efl_unref(empty);
   efl_unref(empty_base);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

/* In single mode, allow_manual_deselection decides whether the model may end
 * up with nothing selected. It defaults to allowing it, the same as
 * Efl.Ui.Collection does; turning it off makes the last selection stick.
 *
 * This is checked on the model directly rather than through a view: the
 * property is documented in terms of clicking, but a model has no clicks, and
 * a model driven from code has to behave the same way. */
EFL_START_TEST(efl_test_select_model_manual_deselection)
{
   Efl_Model *base_model, *model, *child;
   Eina_Value v = EINA_VALUE_EMPTY;
   Eina_Value *readback;
   Eina_Bool selected = EINA_FALSE;
   int i;

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);

   for (i = 0; i < 3; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, i));
        efl_model_property_set(child, "test_p_int", &v);
     }

   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);

   /* Allowed by default, as Efl.Ui.Collection has it. */
   ck_assert_int_eq(efl_ui_selectable_allow_manual_deselection_get(model), EINA_TRUE);

   efl_ui_selectable_allow_manual_deselection_set(model, EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_allow_manual_deselection_get(model), EINA_FALSE);

   /* Select one, then try to give it back. */
   v = eina_value_int_init(1);
   efl_model_property_set(model, "child.selected", &v);
   ecore_main_loop_iterate();

   {
      Eina_Value keep = eina_value_bool_init(EINA_FALSE);
      Efl_Model *sel = efl_ui_selectable_last_selected_get(model);

      ck_assert_ptr_ne(sel, NULL);
      efl_model_property_set(sel, "self.selected", &keep);
      eina_value_flush(&keep);
      ecore_main_loop_iterate();
      ecore_main_loop_iterate();

      readback = efl_model_property_get(sel, "selected");
      ck_assert_ptr_ne(readback, NULL);
      eina_value_bool_get(readback, &selected);
      eina_value_free(readback);

      /* Refused: something is still selected. */
      ck_assert_int_eq(selected, EINA_TRUE);
      ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), sel);
   }

   eina_value_flush(&v);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static Eina_Bool
_state_get(Eo *child, const char *name)
{
   Eina_Value *r = efl_model_property_get(child, name);
   Eina_Bool b = EINA_FALSE;

   ck_assert(!!r);
   ck_assert(eina_value_type_get(r) == EINA_VALUE_TYPE_BOOL);
   eina_value_bool_get(r, &b);
   eina_value_free(r);
   return b;
}

static Eina_Value
_state_ignore(void *data EINA_UNUSED, const Eina_Value v,
              const Eina_Future *dead_future EINA_UNUSED)
{
   return v;
}

// Leaving multiple selection for single keeps the row that was selected last, as
// Efl.Ui.Collection does (it keeps the last of its selected items), and unselects the
// others. Going to none unselects everything, in both.
static unsigned int
_selected_count(Efl_Ui_Select_Model *model, unsigned int *first)
{
   Eina_Iterator *it = efl_ui_multi_selectable_selected_ndx_iterator_new(model);
   unsigned int *index, n = 0;

   if (first) *first = (unsigned int) -1;
   EINA_ITERATOR_FOREACH(it, index)
     {
        if (!n && first) *first = *index;
        n++;
     }
   eina_iterator_free(it);
   return n;
}

EFL_START_TEST(efl_test_select_model_multi_to_single_keeps_last)
{
   Efl_Model *base_model, *model, *child;
   Eina_Value v = EINA_VALUE_EMPTY;
   unsigned int i, first = 99;

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);
   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);
   for (i = 0; i < 6; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, (int) i));
        efl_model_property_set(child, "test_p_int", &v);
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);

   // (one at a time, the last one is the one selected last)
   efl_ui_multi_selectable_ndx_range_select(model, 1, 1);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_ndx_range_select(model, 4, 4);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_ndx_range_select(model, 2, 2);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 3);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 2);

   // none: everything goes
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 0);

   eina_value_flush(&v);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// In single mode, selecting by index replaces the selection, as selecting the row does.
EFL_START_TEST(efl_test_select_model_single_index_select_replaces)
{
   Efl_Model *base_model, *model;
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);

   efl_ui_multi_selectable_ndx_range_select(model, 2, 2);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 2);

   efl_ui_multi_selectable_ndx_range_select(model, 4, 4);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 4);
   ck_assert_int_eq(efl_composite_model_index_get(efl_ui_selectable_last_selected_get(model)), 4);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// The row selected last was unselected before leaving multiple selection: the lowest
// still selected is kept, and it is the selected one from then on, so a single selection
// that follows replaces it rather than joining it.
EFL_START_TEST(efl_test_select_model_multi_to_single_keeps_one_without_last)
{
   Efl_Model *base_model, *model;
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);

   efl_ui_multi_selectable_ndx_range_select(model, 1, 1);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_ndx_range_select(model, 4, 4);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_ndx_range_unselect(model, 4, 4);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), NULL);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 1);
   ck_assert_ptr_ne(efl_ui_selectable_last_selected_get(model), NULL);
   ck_assert_int_eq(efl_composite_model_index_get(efl_ui_selectable_last_selected_get(model)), 1);

   efl_ui_multi_selectable_ndx_range_select(model, 3, 3);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 3);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Leaving multiple selection, then selecting another row before the kept one is known:
// the later selection is the one that stays.
EFL_START_TEST(efl_test_select_model_multi_to_single_select_at_once)
{
   Efl_Model *base_model, *model;
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);

   efl_ui_multi_selectable_ndx_range_select(model, 1, 1);
   efl_ui_multi_selectable_ndx_range_select(model, 4, 4);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_ndx_range_unselect(model, 4, 4);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   efl_ui_multi_selectable_ndx_range_select(model, 3, 3);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 3);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Single mode holds one row even when several selections are asked for in one turn,
// before any of them committed: the last one asked for is the one.
EFL_START_TEST(efl_test_select_model_single_selects_in_one_turn)
{
   Efl_Model *base_model, *model;
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 8; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);

   efl_ui_multi_selectable_ndx_range_select(model, 1, 3);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 3);

   efl_ui_multi_selectable_ndx_range_select(model, 5, 5);
   efl_ui_multi_selectable_ndx_range_select(model, 0, 0);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 0);
   ck_assert_int_eq(efl_composite_model_index_get(efl_ui_selectable_last_selected_get(model)), 0);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Going to none unselects every row, whatever mode it comes from: a row selected but not
// known as the last one (kept on leaving multiple selection, before it is fetched) does not
// come back on the next switch to multiple selection.
EFL_START_TEST(efl_test_select_model_single_to_none_unselects_all)
{
   Efl_Model *base_model, *model;
   unsigned int i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_ndx_range_select(model, 1, 1);
   efl_ui_multi_selectable_ndx_range_select(model, 4, 4);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_ndx_range_unselect(model, 4, 4);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 1);

   // single keeps row 1, then none at once, before row 1 is known as the last one
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 0);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// A row that left while the slice holding it was pending has no index: selecting it
// selects nothing and does not make it the selected one.
static Eina_Value
_gone_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Eo **kids = data;
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        if (i < 2) kids[i] = efl_ref(c);
     }
   return v;
}

EFL_START_TEST(efl_test_select_model_gone_row_is_not_selected)
{
   Efl_Model *base_model, *model, *rows[3];
   Eo *kids[2] = { NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 3; ++i)
     {
        rows[i] = efl_model_child_add(base_model);
        ck_assert(!!rows[i]);
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);

   eina_future_then(efl_model_children_slice_get(model, 0, 2), _gone_then, kids, NULL);
   efl_model_child_del(base_model, rows[0]);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_ptr_ne(kids[0], NULL);
   ck_assert_int_eq(efl_composite_model_index_get(kids[0]), 0xFFFFFFFF);

   eina_future_cancel(efl_model_property_set(kids[0], "self.selected", &select));
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), NULL);
   ck_assert_int_eq(_selected_count(model, NULL), 0);

   // and the row that is there now is still selectable
   eina_future_cancel(efl_model_property_set(kids[1], "self.selected", &select));
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), kids[1]);

   efl_unref(kids[0]);
   efl_unref(kids[1]);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Single mode: a row selected directly (a click) after a selection by index was asked for,
// before that one fetched its row, is the later one and wins.
EFL_START_TEST(efl_test_select_model_single_later_direct_select_wins)
{
   Efl_Model *base_model, *model;
   Eo *kids[2] = { NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 10; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   efl_ui_multi_selectable_ndx_range_select(model, 2, 2);
   eina_future_then(efl_model_children_slice_get(model, 8, 1), _gone_then, kids, NULL);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!kids[0]);
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 2);

   efl_ui_multi_selectable_ndx_range_select(model, 5, 5);
   eina_future_then(efl_model_property_set(kids[0], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 8);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), kids[0]);

   efl_unref(kids[0]);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Single mode: selecting a row that is the selected one already changes nothing, and
// so does not cancel a selection by index that was asked for and is on its way.
EFL_START_TEST(efl_test_select_model_noop_select_keeps_index_select)
{
   Efl_Model *base_model, *model;
   Eo *kids[2] = { NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 10; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   eina_future_then(efl_model_children_slice_get(model, 2, 1), _gone_then, kids, NULL);
   for (i = 0; (i < 12) && !kids[0]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!kids[0]);
   eina_future_then(efl_model_property_set(kids[0], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 2);

   efl_ui_multi_selectable_ndx_range_select(model, 5, 5);
   eina_future_then(efl_model_property_set(kids[0], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 5);

   efl_unref(kids[0]);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Leaving multiple selection, then picking one of the rows that were selected, in the same
// turn: the unselects of the others, which arrive later, must not take the pick away.
EFL_START_TEST(efl_test_select_model_multi_to_single_pick_a_selected_row)
{
   Efl_Model *base_model, *model;
   Eo *kids[2] = { NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   eina_future_then(efl_model_children_slice_get(model, 0, 2), _gone_then, kids, NULL);
   for (i = 0; (i < 12) && !kids[1]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!kids[1]);

   efl_ui_multi_selectable_ndx_range_select(model, 1, 1);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_ndx_range_select(model, 4, 4);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   efl_ui_multi_selectable_ndx_range_select(model, 2, 2);
   for (i = 0; i < 8; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 3);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   eina_future_then(efl_model_property_set(kids[1], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 1);
   ck_assert_ptr_ne(efl_ui_selectable_last_selected_get(model), NULL);
   ck_assert_int_eq(efl_composite_model_index_get(efl_ui_selectable_last_selected_get(model)), 1);

   efl_unref(kids[0]);
   efl_unref(kids[1]);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static Eina_Value
_four_rows_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Eo **rows = data;
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        rows[i] = efl_ref(c);
     }
   return v;
}

// The fallback takes over when the selected row is unselected directly.
EFL_START_TEST(efl_test_select_model_fallback_after_direct_unselect)
{
   Efl_Model *base_model, *model;
   Eo *rows[4] = { NULL, NULL, NULL, NULL };
   Eina_Value unselect = eina_value_bool_init(EINA_FALSE);
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _four_rows_then, rows, NULL);
   for (i = 0; (i < 12) && !rows[3]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!rows[3]);

   efl_ui_selectable_fallback_selection_set(model, rows[0]);
   efl_ui_multi_selectable_ndx_range_select(model, 3, 3);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 3);

   eina_future_then(efl_model_property_set(rows[3], "self.selected", &unselect),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 0);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), rows[0]);

   for (i = 0; i < 4; i++)
     {
        efl_unref(rows[i]);
     }
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Going to none unselects every row, whatever allow_manual_deselection says: that is not a
// person giving up the selection but the mode taking it away. Back in single mode nothing
// is selected, whether the mode came back at once or after the unselects ran.
EFL_START_TEST(efl_test_select_model_none_clears_without_manual_deselection)
{
   Efl_Model *base_model, *model;
   unsigned int i, round, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   efl_ui_selectable_allow_manual_deselection_set(model, EINA_FALSE);

   for (round = 0; round < 2; round++)
     {
        efl_ui_multi_selectable_ndx_range_select(model, 1, 1);
        for (i = 0; i < 12; i++)
          {
             ecore_main_loop_iterate();
          }
        ck_assert_int_eq(_selected_count(model, &first), 1);
        ck_assert_int_eq(first, 1);

        efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
        // (round 0: back to single in the same turn, round 1: once the unselects ran)
        if (round) for (i = 0; i < 12; i++) ecore_main_loop_iterate();
        efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
        for (i = 0; i < 12; i++)
          {
             ecore_main_loop_iterate();
          }
        ck_assert_int_eq(_selected_count(model, NULL), 0);
        ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), NULL);
     }

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// A row selected before a clear, picked again once the mode is back, in the same turn: the
// pick is the later one, and the queued clear must leave it.
static void
_repick_after_clear(Efl_Ui_Select_Mode mode)
{
   Efl_Model *base_model, *model;
   Eo *kids[2] = { NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, mode);
   eina_future_then(efl_model_children_slice_get(model, 0, 2), _gone_then, kids, NULL);
   for (i = 0; (i < 12) && !kids[1]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!kids[1]);

   eina_future_then(efl_model_property_set(kids[1], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 1);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, mode);
   eina_future_then(efl_model_property_set(kids[1], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 1);

   efl_unref(kids[0]);
   efl_unref(kids[1]);
   efl_unref(model);
   efl_unref(base_model);
}

EFL_START_TEST(efl_test_select_model_multi_repick_after_clear)
{
   _repick_after_clear(EFL_UI_SELECT_MODE_MULTI);
}
EFL_END_TEST

EFL_START_TEST(efl_test_select_model_single_repick_after_clear)
{
   _repick_after_clear(EFL_UI_SELECT_MODE_SINGLE);
}
EFL_END_TEST

// The same with nothing holding the row once it is picked: the object of a row that nobody
// refs goes away, and the clear, whose slice resolves on a later pass, builds a new one for
// the same index, which must still be known as picked after the clear.
EFL_START_TEST(efl_test_select_model_repick_after_clear_rows_not_held)
{
   Efl_Model *base_model, *model;
   Eo *kids[2] = { NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   eina_future_then(efl_model_children_slice_get(model, 0, 2), _gone_then, kids, NULL);
   for (i = 0; (i < 12) && !kids[1]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!kids[1]);

   eina_future_then(efl_model_property_set(kids[1], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   eina_future_then(efl_model_property_set(kids[1], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   // nothing holds the rows any more when the slice of the clear resolves
   efl_unref(kids[0]);
   efl_unref(kids[1]);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 1);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static Eina_Value
_three_rows_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Eo **rows = data;
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        if (i < 3) rows[i] = efl_ref(c);
     }
   return v;
}

// The pick of a row follows the row when one before it leaves: it is the pick of the row at
// its new index, not of whatever comes to the old one.
EFL_START_TEST(efl_test_select_model_pick_follows_removed_row)
{
   Efl_Model *base_model, *model, *rows[6];
   Eo *kids[3] = { NULL, NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i, first = 99;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        rows[i] = efl_model_child_add(base_model);
        ck_assert(!!rows[i]);
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   eina_future_then(efl_model_children_slice_get(model, 0, 3), _three_rows_then, kids, NULL);
   for (i = 0; (i < 12) && !kids[2]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!kids[2]);

   eina_future_then(efl_model_property_set(kids[2], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 2);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   eina_future_then(efl_model_property_set(kids[2], "self.selected", &select),
                    _state_ignore, NULL, NULL);
   efl_model_child_del(base_model, rows[0]);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 1);

   for (i = 0; i < 3; i++)
     {
        efl_unref(kids[i]);
     }
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// A select that is cancelled while it waits for the unselect of the row before is rolled back:
// the new row is not selected, so it is not the last selected one either, and the row before
// is the selected one still.
EFL_START_TEST(efl_test_select_model_failed_unselect_rolls_back_last)
{
   Efl_Model *base_model, *model;
   Eo *rows[3] = { NULL, NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 3; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   eina_future_then(efl_model_children_slice_get(model, 0, 3), _four_rows_then, rows, NULL);
   for (i = 0; (i < 12) && !rows[2]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!rows[2]);

   efl_ui_multi_selectable_ndx_range_select(model, 2, 2);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), rows[2]);

   eina_future_cancel(efl_model_property_set(rows[0], "self.selected", &select));
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }

   // The new row was taken back: it is not selected, so it is not the last selected one
   ck_assert_int_eq(_state_get(rows[0], "selected"), EINA_FALSE);
   // and the row whose unselect failed is still the selected one
   ck_assert_int_eq(_state_get(rows[2], "selected"), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), rows[2]);
   ck_assert_int_eq(_selected_count(model, &i), 1);
   ck_assert_int_eq(i, 2);

   for (i = 0; i < 3; i++)
     {
        efl_unref(rows[i]);
     }
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static Eina_Future *
_refuse_unselect(Eo *obj, void *pd EINA_UNUSED, const char *property, Eina_Value *value)
{
   Eina_Bool flag = EINA_TRUE;

   if (eina_streq(property, "selected") &&
       (eina_value_type_get(value) == EINA_VALUE_TYPE_BOOL) &&
       eina_value_bool_get(value, &flag) && !flag)
     return efl_loop_future_rejected(obj, EINVAL);
   return efl_model_property_set(efl_super(obj, EFL_OBJECT_OVERRIDE_CLASS), property, value);
}

// Single mode: a select whose unselect of the row selected before fails is rolled back, as
// one cancelled while it waits for that unselect is. The unselect runs next to the commit of
// the new row, and its failure used to be dropped with its result: both rows stayed selected.
EFL_START_TEST(efl_test_select_model_refused_unselect_rolls_back)
{
   EFL_OPS_DEFINE(refuse, EFL_OBJECT_OP_FUNC(efl_model_property_set, _refuse_unselect));
   Efl_Model *base_model, *model;
   Eo *rows[3] = { NULL, NULL, NULL };
   Eina_Value select = eina_value_bool_init(EINA_TRUE);
   unsigned int i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 3; ++i)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   eina_future_then(efl_model_children_slice_get(model, 0, 3), _four_rows_then, rows, NULL);
   for (i = 0; (i < 12) && !rows[2]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!rows[2]);

   efl_ui_multi_selectable_ndx_range_select(model, 2, 2);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), rows[2]);

   ck_assert(efl_object_override(rows[2], &refuse));
   efl_model_property_set(rows[0], "self.selected", &select);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }

   ck_assert_int_eq(_state_get(rows[0], "selected"), EINA_FALSE);
   ck_assert_int_eq(_state_get(rows[2], "selected"), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), rows[2]);
   ck_assert_int_eq(_selected_count(model, &i), 1);
   ck_assert_int_eq(i, 2);

   ck_assert(efl_object_override(rows[2], NULL));
   for (i = 0; i < 3; i++)
     {
        efl_unref(rows[i]);
     }
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static Eina_Value
_rows_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Eo **rows = data;
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        if (i < 4) rows[i] = efl_ref(c);
     }
   return v;
}

static Efl_Model *
_rows_model_new(Efl_Model **base, Eo **rows, Efl_Ui_Select_Mode mode)
{
   Efl_Model *model;
   unsigned int i;

   *base = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; ++i)
     {
        ck_assert(!!efl_model_child_add(*base));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, *base));
   efl_ui_multi_selectable_select_mode_set(model, mode);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _rows_then, rows, NULL);
   for (i = 0; (i < 12) && !rows[3]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!rows[3]);
   return model;
}

static void
_row_select(Eo *row)
{
   Eina_Value select = eina_value_bool_init(EINA_TRUE);

   eina_future_then(efl_model_property_set(row, "self.selected", &select),
                    _state_ignore, NULL, NULL);
   eina_value_flush(&select);
}

static void
_rows_free(Efl_Model *model, Efl_Model *base, Eo **rows)
{
   unsigned int i;

   for (i = 0; i < 4; i++)
     {
        efl_unref(rows[i]);
     }
   efl_unref(model);
   efl_unref(base);
}

// Single mode, A selected: picking B and then A again in the same turn leaves the last
// pick, A, selected, and only it. The unselect B asks for must not take A back.
EFL_START_TEST(efl_test_select_model_single_pick_a_b_a_same_turn)
{
   Efl_Model *base_model, *model;
   Eo *rows[4] = { NULL, NULL, NULL, NULL };
   unsigned int i, first = 99;

   model = _rows_model_new(&base_model, rows, EFL_UI_SELECT_MODE_SINGLE);
   _row_select(rows[1]);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);

   _row_select(rows[2]);
   _row_select(rows[1]);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), rows[1]);

   _rows_free(model, base_model, rows);
}
EFL_END_TEST

// Going to none and then back to single, in one turn, and picking a row: the unselects that
// going to none asked for arrive after the pick, and must not take it.
EFL_START_TEST(efl_test_select_model_none_single_pick_same_turn)
{
   Efl_Model *base_model, *model;
   Eo *rows[4] = { NULL, NULL, NULL, NULL };
   unsigned int i, first = 99;

   model = _rows_model_new(&base_model, rows, EFL_UI_SELECT_MODE_SINGLE);
   _row_select(rows[1]);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_SINGLE);
   _row_select(rows[3]);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 3);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(model), rows[3]);

   _rows_free(model, base_model, rows);
}
EFL_END_TEST

// The same going back to multiple: the picks stay, what was selected before goes.
EFL_START_TEST(efl_test_select_model_none_multi_pick_same_turn)
{
   Efl_Model *base_model, *model;
   Eo *rows[4] = { NULL, NULL, NULL, NULL };
   unsigned int i, first = 99;

   model = _rows_model_new(&base_model, rows, EFL_UI_SELECT_MODE_MULTI);
   _row_select(rows[1]);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   _row_select(rows[2]);
   _row_select(rows[3]);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 2);
   ck_assert_int_eq(first, 2);

   _rows_free(model, base_model, rows);
}
EFL_END_TEST

// A row added before a picked row moves its pick with it: a clear asked for before the pick
// still leaves the row, whatever came to its old index.
EFL_START_TEST(efl_test_select_model_pick_follows_added_row)
{
   Efl_Model *base_model, *model;
   Efl_Model_Children_Event ev = { 0 };
   Eo *rows[4] = { NULL, NULL, NULL, NULL };
   unsigned int i, first = 99;

   model = _rows_model_new(&base_model, rows, EFL_UI_SELECT_MODE_MULTI);
   _row_select(rows[2]);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   _row_select(rows[2]);
   // a row comes in at the front, before the picked one (the generic model appends, and
   // says so; then the row is said to be at the front)
   ev.child = efl_model_child_add(base_model);
   ev.index = 0;
   efl_event_callback_call(base_model, EFL_MODEL_EVENT_CHILD_ADDED, &ev);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 3);

   _rows_free(model, base_model, rows);
}
EFL_END_TEST

// The picked row itself leaves: its pick goes with it, and is not the pick of the row that
// comes to its index. That row was selected before the clear and is not protected.
EFL_START_TEST(efl_test_select_model_pick_of_removed_row)
{
   Efl_Model *base_model, *model;
   Eo *rows[4] = { NULL, NULL, NULL, NULL };
   Eo *base_rows[4] = { NULL, NULL, NULL, NULL };
   unsigned int i;

   model = _rows_model_new(&base_model, rows, EFL_UI_SELECT_MODE_MULTI);
   _row_select(rows[1]);
   _row_select(rows[2]);
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 2);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   _row_select(rows[1]);
   eina_future_then(efl_model_children_slice_get(base_model, 0, 4), _rows_then, base_rows, NULL);
   for (i = 0; (i < 12) && !base_rows[3]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!base_rows[3]);
   efl_model_child_del(base_model, base_rows[1]);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 0);

   for (i = 0; i < 4; i++)
     {
        efl_unref(base_rows[i]);
     }
   _rows_free(model, base_model, rows);
}
EFL_END_TEST

// Going to none lets go of the picks, and what is picked after it still survives the clear
// asked for by it, though the rows were all picked before.
EFL_START_TEST(efl_test_select_model_none_drops_the_picks)
{
   Efl_Model *base_model, *model;
   Eo *rows[4] = { NULL, NULL, NULL, NULL };
   unsigned int i, first = 99;

   model = _rows_model_new(&base_model, rows, EFL_UI_SELECT_MODE_MULTI);
   for (i = 0; i < 4; i++)
     {
        _row_select(rows[i]);
     }
   for (i = 0; i < 12; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 4);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 0);

   // None, back to multiple and a pick in one turn: the pick is newer than the clear
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   _row_select(rows[3]);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   _row_select(rows[2]);
   for (i = 0; i < 16; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, &first), 1);
   ck_assert_int_eq(first, 2);

   _rows_free(model, base_model, rows);
}
EFL_END_TEST

// The time it takes to put rows at the front of a list of the same count, one after
// the other.
// A clear that is pending is what makes the picks worth keeping, and the selection after
// it is only what was picked after it.
EFL_START_TEST(efl_test_select_model_clear_after_all_select)
{
   Efl_Model *base_model, *model;
   unsigned int i, rows = 200;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < rows; i++)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }
   model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_all_select(model);
   for (i = 0; i < 100; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), rows);

   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_NONE);
   efl_ui_multi_selectable_select_mode_set(model, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_ndx_range_select(model, 3, 5);
   for (i = 0; i < 100; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_selected_count(model, NULL), 3);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

void
efl_ui_test_select_model(TCase *tc)
{
   tcase_add_test(tc, efl_test_select_model_clear_after_all_select);
   tcase_add_test(tc, efl_test_select_model);
   tcase_add_test(tc, efl_test_select_model_selects_the_first);
   tcase_add_test(tc, efl_test_select_model_reversed_range);
   tcase_add_test(tc, efl_test_select_model_manual_deselection);
   tcase_add_test(tc, efl_test_select_model_multi_to_single_keeps_last);
   tcase_add_test(tc, efl_test_select_model_single_index_select_replaces);
   tcase_add_test(tc, efl_test_select_model_multi_to_single_keeps_one_without_last);
   tcase_add_test(tc, efl_test_select_model_multi_to_single_select_at_once);
   tcase_add_test(tc, efl_test_select_model_multi_to_single_pick_a_selected_row);
   tcase_add_test(tc, efl_test_select_model_single_pick_a_b_a_same_turn);
   tcase_add_test(tc, efl_test_select_model_none_single_pick_same_turn);
   tcase_add_test(tc, efl_test_select_model_none_multi_pick_same_turn);
   tcase_add_test(tc, efl_test_select_model_fallback_after_direct_unselect);
   tcase_add_test(tc, efl_test_select_model_none_clears_without_manual_deselection);
   tcase_add_test(tc, efl_test_select_model_multi_repick_after_clear);
   tcase_add_test(tc, efl_test_select_model_single_repick_after_clear);
   tcase_add_test(tc, efl_test_select_model_repick_after_clear_rows_not_held);
   tcase_add_test(tc, efl_test_select_model_pick_follows_removed_row);
   tcase_add_test(tc, efl_test_select_model_failed_unselect_rolls_back_last);
   tcase_add_test(tc, efl_test_select_model_refused_unselect_rolls_back);
   tcase_add_test(tc, efl_test_select_model_gone_row_is_not_selected);
   tcase_add_test(tc, efl_test_select_model_single_selects_in_one_turn);
   tcase_add_test(tc, efl_test_select_model_single_to_none_unselects_all);
   tcase_add_test(tc, efl_test_select_model_single_later_direct_select_wins);
   tcase_add_test(tc, efl_test_select_model_noop_select_keeps_index_select);
   tcase_add_test(tc, efl_test_select_model_pick_follows_added_row);
   tcase_add_test(tc, efl_test_select_model_pick_of_removed_row);
   tcase_add_test(tc, efl_test_select_model_none_drops_the_picks);
}
