#ifndef EFL_UI_SUITE_H
#define EFL_UI_SUITE_H

#include <check.h>

#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Ui.h>
#include "../efl_check.h"

extern Evas_Object *win;
extern Evas_Object *widget;

void efl_pack_behavior_test(TCase *tc);
void efl_pack_linear_behavior_test(TCase *tc);
void efl_content_behavior_test(TCase *tc);
void efl_gfx_arrangement_behavior_test(TCase *tc);
void efl_ui_clickable_behavior_test(TCase *tc);

void efl_test_container_content_equal(Efl_Ui_Widget **wid, unsigned int len);
void efl_test_container_expect_evt_content_added(Efl_Ui_Widget *widget, const Efl_Event_Description *ev, Eina_Bool *flag, void *event_data);

Eo* create_test_widget(void);

Evas_Object * win_add();

const Efl_Class* efl_ui_widget_realized_class_get(void);

#define WIDGET_CLASS efl_ui_widget_realized_class_get()

#endif
