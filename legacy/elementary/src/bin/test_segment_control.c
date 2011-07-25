#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

void
test_segment_control(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *ic, *ic1, *ic2, *ic3, *ic4, *ic5;
   Elm_Segment_Item *it1, *it2, *it3, *it4, *it5;

   Evas_Object * in_layout;
   Evas_Object *segment1, *segment2, *segment3, *segment4;
   char buf[PATH_MAX];
   char buf1[PATH_MAX];
   char buf2[PATH_MAX];
   char buf3[PATH_MAX];
   char buf4[PATH_MAX];
   char buf5[PATH_MAX];
   char buf6[PATH_MAX];

   win = elm_win_add(NULL, "segmentcontrol", ELM_WIN_BASIC);
   elm_win_title_set(win, "Segment Control");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   in_layout = elm_layout_add( win );
   elm_win_resize_object_add(win, in_layout);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(in_layout, buf, "segment_test");
   evas_object_size_hint_weight_set(in_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   ic = elm_icon_add(in_layout);
   snprintf(buf1, sizeof(buf1), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf1, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   ic1 = elm_icon_add(in_layout);
   snprintf(buf2, sizeof(buf2), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic1, buf2, NULL);
   evas_object_size_hint_aspect_set(ic1, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   segment1 = elm_segment_control_add(win);
   it1 = elm_segment_control_item_add(segment1, NULL, "Only Text");
   it2 = elm_segment_control_item_add(segment1, ic, NULL);
   elm_segment_control_item_selected_set(it2, EINA_TRUE);
   it3 = elm_segment_control_item_add(segment1, ic1, "Text_Icon_test");
   it4 = elm_segment_control_item_add(segment1, NULL, "Seg4");
   it5 = elm_segment_control_item_add(segment1, NULL, "Seg5");

   segment2 = elm_segment_control_add(win);
   it1 = elm_segment_control_item_add(segment2, NULL, "SegmentItem");
   it2 = elm_segment_control_item_add(segment2, NULL, "SegmentItem");
   elm_segment_control_item_selected_set(it2, EINA_TRUE);
   it3 = elm_segment_control_item_add(segment2, NULL, "SegmentControlItem");
   it4 = elm_segment_control_item_add(segment2, NULL, "SegmentItem");

   ic2 = elm_icon_add(in_layout);
   snprintf(buf3, sizeof(buf3), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic2, buf3, NULL);
   evas_object_size_hint_aspect_set(ic2, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   ic3 = elm_icon_add(in_layout);
   snprintf(buf4, sizeof(buf4), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic3, buf4, NULL);
   evas_object_size_hint_aspect_set(ic3, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   ic4 = elm_icon_add(in_layout);
   snprintf(buf5, sizeof(buf5), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic4, buf5, NULL);
   evas_object_size_hint_aspect_set(ic4, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   segment3 = elm_segment_control_add(win);
   it1 = elm_segment_control_item_add(segment3, ic2, NULL);
   it2 = elm_segment_control_item_add(segment3, ic3, NULL);
   elm_segment_control_item_selected_set(it2, EINA_TRUE);
   it3 = elm_segment_control_item_add(segment3, ic4, NULL);

   ic5 = elm_icon_add(in_layout);
   snprintf(buf6, sizeof(buf6), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic5, buf6, NULL);
   evas_object_size_hint_aspect_set(ic5, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   segment4 = elm_segment_control_add(win);
   it1 = elm_segment_control_item_add(segment4, NULL, "Disabled");
   it2 = elm_segment_control_item_add(segment4, ic5, "Disabled");
   elm_segment_control_item_selected_set(it2, EINA_TRUE);
   it3 = elm_segment_control_item_add(segment4, NULL, "Disabled");
   elm_object_disabled_set(segment4, EINA_TRUE);

   elm_layout_content_set(in_layout, "segment1", segment1);
   elm_layout_content_set(in_layout, "segment2", segment2);
   elm_layout_content_set(in_layout, "segment3", segment3);
   elm_layout_content_set(in_layout, "segment4", segment4);

   evas_object_show(in_layout);

   evas_object_show(win);
}
#endif
