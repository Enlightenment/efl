#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <string.h>
#include <Elementary.h>
#include "test.h"
#include "test_explode.h"

int _log_domain = -1;

/* all tests prototypes */
void test_access(void *data, Evas_Object *obj, void *event_info);
void test_access2(void *data, Evas_Object *obj, void *event_info);
void test_access3(void *data, Evas_Object *obj, void *event_info);
void test_bg_plain(void *data, Evas_Object *obj, void *event_info);
void test_bg_image(void *data, Evas_Object *obj, void *event_info);
void test_bg_options(void *data, Evas_Object *obj, void *event_info);
void test_bg_window(void *data, Evas_Object *obj, void *event_info);
void test_icon(void *data, Evas_Object *obj, void *event_info);
void test_icon_transparent(void *data, Evas_Object *obj, void *event_info);
void test_icon_standard(void *data, Evas_Object *obj, void *event_info);
void test_box_vert(void *data, Evas_Object *obj, void *event_info);
void test_box_vert2(void *data, Evas_Object *obj, void *event_info);
void test_box_pack(void *data, Evas_Object *obj, void *event_info);
void test_box_horiz(void *data, Evas_Object *obj, void *event_info);
void test_box_homo(void *data, Evas_Object *obj, void *event_info);
void test_box_transition(void *data, Evas_Object *obj, void *event_info);
void test_box_align(void *data, Evas_Object *obj, void *event_info);
void test_box_stack(void *data, Evas_Object *obj, void *event_info);
void test_ui_box(void *data, Evas_Object *obj, void *event_info);
void test_button(void *data, Evas_Object *obj, void *event_info);
void test_cnp(void *data, Evas_Object *obj, void *event_info);
void test_events(void *data, Evas_Object *obj, void *event_info);
void test_transit(void *data, Evas_Object *obj, void *event_info);
void test_transit_blend(void *data, Evas_Object *obj, void *event_info);
void test_transit_chain(void *data, Evas_Object *obj, void *event_info);
void test_transit_custom(void *data, Evas_Object *obj, void *event_info);
void test_transit_fade(void *data, Evas_Object *obj, void *event_info);
void test_transit_flip(void *data, Evas_Object *obj, void *event_info);
void test_transit_resizable_flip(void *data, Evas_Object *obj, void *event_info);
void test_transit_resizing(void *data, Evas_Object *obj, void *event_info);
void test_transit_zoom(void *data, Evas_Object *obj, void *event_info);
void test_transit_tween(void *data, Evas_Object *obj, void *event_info);
void test_transit_bezier(void *data, Evas_Object *obj, void *event_info);
void test_fileselector_button(void *data, Evas_Object *obj, void *event_info);
void test_fileselector_entry(void *data, Evas_Object *obj, void *event_info);
void test_clock(void *data, Evas_Object *obj, void *event_info);
void test_clock_edit(void *data, Evas_Object *obj, void *event_info);
void test_clock_edit2(void *data, Evas_Object *obj, void *event_info);
void test_clock_pause(void *data, Evas_Object *obj, void *event_info);
void test_combobox(void *data, Evas_Object *obj, void *event_info);
void test_check(void *data, Evas_Object *obj, void *event_info);
void test_check_toggle(void *data, Evas_Object *obj, void *event_info);
void test_nstate(void *data, Evas_Object *obj, void *event_info);
void test_radio(void *data, Evas_Object *obj, void *event_info);
void test_layout(void *data, Evas_Object *obj, void *event_info);
void test_layout2(void *data, Evas_Object *obj, void *event_info);
void test_hover(void *data, Evas_Object *obj, void *event_info);
void test_hover2(void *data, Evas_Object *obj, void *event_info);
void test_hover3(void *data, Evas_Object *obj, void *event_info);
void test_entry(void *data, Evas_Object *obj, void *event_info);
void test_entry_style_user(void *data, Evas_Object *obj, void *event_info);
void test_entry_style_default(void *data, Evas_Object *obj, void *event_info);
void test_entry_scrolled(void *data, Evas_Object *obj, void *event_info);
void test_entry_on_page_scroll(void *data, Evas_Object *obj, void *event_info);
void test_entry3(void *data, Evas_Object *obj, void *event_info);
void test_entry4(void *data, Evas_Object *obj, void *event_info);
void test_entry5(void *data, Evas_Object *obj, void *event_info);
void test_entry6(void *data, Evas_Object *obj, void *event_info);
void test_entry7(void *data, Evas_Object *obj, void *event_info);
void test_entry8(void *data, Evas_Object *obj, void *event_info);
void test_entry_regex(void *data, Evas_Object *obj, void *event_info);
void test_entry_notepad(void *data, Evas_Object *obj, void *event_info);
void test_multibuttonentry(void *data, Evas_Object *obj, void *event_info);
void test_multibuttonentry2(void *data, Evas_Object *obj, void *event_info);
void test_entry_anchor2(void *data, Evas_Object *obj, void *event_info);
void test_entry_anchor(void *data, Evas_Object *obj, void *event_info);
void test_entry_emoticon(void *data, Evas_Object *obj, void *event_info);
void test_entry_password(void *data, Evas_Object *obj, void *event_info);
void test_toolbar(void *data, Evas_Object *obj, void *event_info);
void test_toolbar2(void *data, Evas_Object *obj, void *event_info);
void test_toolbar3(void *data, Evas_Object *obj, void *event_info);
void test_toolbar4(void *data, Evas_Object *obj, void *event_info);
void test_toolbar5(void *data, Evas_Object *obj, void *event_info);
void test_toolbar6(void *data, Evas_Object *obj, void *event_info);
void test_toolbar7(void *data, Evas_Object *obj, void *event_info);
void test_toolbar8(void *data, Evas_Object *obj, void *event_info);
void test_toolbar_vertical(void *data, Evas_Object *obj, void *event_info);
void test_toolbar_focus(void *data, Evas_Object *obj, void *event_info);
void test_hoversel(void *data, Evas_Object *obj, void *event_info);
void test_hoversel_focus(void *data, Evas_Object *obj, void *event_info);
void test_list(void *data, Evas_Object *obj, void *event_info);
void test_list_horizontal(void *data, Evas_Object *obj, void *event_info);
void test_list2(void *data, Evas_Object *obj, void *event_info);
void test_list3(void *data, Evas_Object *obj, void *event_info);
void test_list4(void *data, Evas_Object *obj, void *event_info);
void test_list5(void *data, Evas_Object *obj, void *event_info);
void test_list6(void *data, Evas_Object *obj, void *event_info);
void test_list7(void *data, Evas_Object *obj, void *event_info);
void test_list_focus(void *data, Evas_Object *obj, void *event_info);
void test_list_horiz_focus(void *data, Evas_Object *obj, void *event_info);
void test_list_separator(void *data, Evas_Object *obj, void *event_info);
void test_list_multi_select(void *data, Evas_Object *obj, void *event_info);
void test_inwin(void *data, Evas_Object *obj, void *event_info);
void test_inwin2(void *data, Evas_Object *obj, void *event_info);
void test_inwin3(void *data, Evas_Object *obj, void *event_info);
void test_scaling(void *data, Evas_Object *obj, void *event_info);
void test_scaling2(void *data, Evas_Object *obj, void *event_info);
void test_slider(void *data, Evas_Object *obj, void *event_info);
void test_actionslider(void *data, Evas_Object *obj, void *event_info);
void test_genlist(void *data, Evas_Object *obj, void *event_info);
void test_genlist2(void *data, Evas_Object *obj, void *event_info);
void test_genlist3(void *data, Evas_Object *obj, void *event_info);
void test_genlist4(void *data, Evas_Object *obj, void *event_info);
void test_genlist5(void *data, Evas_Object *obj, void *event_info);
void test_genlist6(void *data, Evas_Object *obj, void *event_info);
void test_genlist7(void *data, Evas_Object *obj, void *event_info);
void test_genlist8(void *data, Evas_Object *obj, void *event_info);
void test_genlist9(void *data, Evas_Object *obj, void *event_info);
void test_genlist10(void *data, Evas_Object *obj, void *event_info);
void test_genlist_reorder(void *data, Evas_Object *obj, void *event_info);
void test_genlist12(void *data, Evas_Object *obj, void *event_info);
void test_genlist13(void *data, Evas_Object *obj, void *event_info);
void test_genlist14(void *data, Evas_Object *obj, void *event_info);
void test_genlist15(void *data, Evas_Object *obj, void *event_info);
void test_genlist16(void *data, Evas_Object *obj, void *event_info);
void test_genlist17(void *data, Evas_Object *obj, void *event_info);
void test_genlist18(void *data, Evas_Object *obj, void *event_info);
void test_genlist19(void *data, Evas_Object *obj, void *event_info);
void test_genlist20(void *data, Evas_Object *obj, void *event_info);
void test_genlist_focus(void *data, Evas_Object *obj, void *event_info);
void test_genlist_item_styles(void *data, Evas_Object *obj, void *event_info);
void test_genlist_multi_select(void *data, Evas_Object *obj, void *event_info);
void test_genlist_del(void *data, Evas_Object *obj, void *event_info);
void test_genlist_filter(void *data, Evas_Object *obj, void *event_info);
void test_genlist_show_bring(void *data, Evas_Object *obj, void *event_info);
void test_genlist_cache(void *data, Evas_Object *obj, void *event_info);
void test_genlist_reusable(void *data, Evas_Object *obj, void *event_info);
void test_gesture_layer(void *data, Evas_Object *obj, void *event_info);
void test_gesture_layer2(void *data, Evas_Object *obj, void *event_info);
void test_gesture_layer3(void *data, Evas_Object *obj, void *event_info);
void test_table(void *data, Evas_Object *obj, void *event_info);
void test_table2(void *data, Evas_Object *obj, void *event_info);
void test_table3(void *data, Evas_Object *obj, void *event_info);
void test_table4(void *data, Evas_Object *obj, void *event_info);
void test_table5(void *data, Evas_Object *obj, void *event_info);
void test_table6(void *data, Evas_Object *obj, void *event_info);
void test_table7(void *data, Evas_Object *obj, void *event_info);
void test_table8(void *data, Evas_Object *obj, void *event_info);
void test_ui_grid(void *data, Evas_Object *obj, void *event_info);
void test_ui_grid_linear(void *data, Evas_Object *obj, void *event_info);
void test_gengrid(void *data, Evas_Object *obj, void *event_info);
void test_gengrid2(void *data, Evas_Object *obj, void *event_info);
void test_gengrid3(void *data, Evas_Object *obj, void *event_info);
void test_gengrid4(void *data, Evas_Object *obj, void *event_info);
void test_gengrid5(void *data, Evas_Object *obj, void *event_info);
void test_gengrid_item_styles(void *data, Evas_Object *obj, void *event_info);
void test_gengrid_speed(void *data, Evas_Object *obj, void *event_info);
void test_gengrid_focus(void *data, Evas_Object *obj, void *event_info);
void test_gengrid_update(void *data, Evas_Object *obj, void *event_info);
void test_gengrid_disabled_item_focus(void *data, Evas_Object *obj, void *event_info);
void test_gengrid_item_custom_size(void *data, Evas_Object *obj, void *event_info);
void test_win_state(void *data, Evas_Object *obj, void *event_info);
void test_win_state2(void *data, Evas_Object *obj, void *event_info);
void test_win_stack(void *data, Evas_Object *obj, void *event_info);
void test_progressbar(void *data, Evas_Object *obj, void *event_info);
void test_progressbar2(void *data, Evas_Object *obj, void *event_info);
void test_fileselector(void *data, Evas_Object *obj, void *event_info);
void test_separator(void *data, Evas_Object *obj, void *event_info);
void test_scroller(void *data, Evas_Object *obj, void *event_info);
void test_scroller2(void *data, Evas_Object *obj, void *event_info);
void test_scroller3(void *data, Evas_Object *obj, void *event_info);
void test_scroller4(void *data, Evas_Object *obj, void *event_info);
void test_scroller5(void *data, Evas_Object *obj, void *event_info);
void test_spinner(void *data, Evas_Object *obj, void *event_info);
void test_index(void *data, Evas_Object *obj, void *event_info);
void test_index2(void *data, Evas_Object *obj, void *event_info);
void test_index3(void *data, Evas_Object *obj, void *event_info);
void test_index_horizontal(void *data, Evas_Object *obj, void *event_info);
void test_photocam(void *data, Evas_Object *obj, void *event_info);
void test_photocam_remote(void *data, Evas_Object *obj, void *event_info);
void test_photocam_icon(void *data, Evas_Object *obj, void *event_info);
void test_image_zoomable_animated(void *data, Evas_Object *obj, void *event_info);
void test_photo(void *data, Evas_Object *obj, void *event_info);
void test_prefs(void *data, Evas_Object *obj, void *event_info);
void test_thumb(void *data, Evas_Object *obj, void *event_info);
void test_icon_desktops(void *data, Evas_Object *obj, void *event_info);
void test_icon_animated(void *data, Evas_Object *obj, void *event_info);
void test_notify(void *data, Evas_Object *obj, void *event_info);
void test_slideshow(void *data, Evas_Object *obj, void *event_info);
void test_menu(void *data, Evas_Object *obj, void *event_info);
void test_menu2(void *data, Evas_Object *obj, void *event_info);
void test_panel(void *data, Evas_Object *obj, void *event_info);
void test_panel2(void *data, Evas_Object *obj, void *event_info);
void test_panes(void *data, Evas_Object *obj, void *event_info);
void test_panes_minsize(void *data, Evas_Object *obj, void *event_info);
void test_calendar(void *data, Evas_Object *obj, void *event_info);
void test_calendar2(void *data, Evas_Object *obj, void *event_info);
void test_calendar3(void *data, Evas_Object *obj, void *event_info);
void test_map(void *data, Evas_Object *obj, void *event_info);
void test_weather(void *data, Evas_Object *obj, void *event_info);
void test_flip(void *data, Evas_Object *obj, void *event_info);
void test_flip2(void *data, Evas_Object *obj, void *event_info);
void test_flip3(void *data, Evas_Object *obj, void *event_info);
void test_flip_interactive(void *data, Evas_Object *obj, void *event_info);
void test_flip_to(void *data, Evas_Object *obj, void *event_info);
void test_flip_page(void *data, Evas_Object *obj, void *event_info);
void test_flip_page_eo(void *data, Evas_Object *obj, void *event_info);
void test_label(void *data, Evas_Object *obj, void *event_info);
void test_label_slide(void *data, Evas_Object *obj, void *event_info);
void test_label_wrap(void *data, Evas_Object *obj, void *event_info);
void test_label_ellipsis(void *data, Evas_Object *obj, void *event_info);
void test_label_emoji(void *data, Evas_Object *obj, void *event_info);
void test_conformant(void *data, Evas_Object *obj, void *event_info);
void test_conformant2(void *data, Evas_Object *obj, void *event_info);
void test_conformant_indicator(void *data, Evas_Object *obj, void *event_info);
void test_main_menu(void *data, Evas_Object *obj, void *event_info);
void test_multi(void *data, Evas_Object *obj, void *event_info);
void test_floating(void *data, Evas_Object *obj, void *event_info);
void test_launcher(void *data, Evas_Object *obj, void *event_info);
void test_launcher2(void *data, Evas_Object *obj, void *event_info);
void test_launcher3(void *data, Evas_Object *obj, void *event_info);
void test_anim(void *data, Evas_Object *obj, void *event_info);
void test_tooltip(void *data, Evas_Object *obj, void *event_info);
void test_tooltip2(void *data, Evas_Object *obj, void *event_info);
void test_tooltip3(void *data, Evas_Object *obj, void *event_info);
void test_tooltip4(void *data, Evas_Object *obj, void *event_info);
void test_cursor(void *data, Evas_Object *obj, void *event_info);
void test_cursor2(void *data, Evas_Object *obj, void *event_info);
void test_cursor3(void *data, Evas_Object *obj, void *event_info);
void test_cursor4(void *data, Evas_Object *obj, void *event_info);
void test_focus(void *data, Evas_Object *obj, void *event_info);
void test_focus2(void *data, Evas_Object *obj, void *event_info);
void test_focus_hide_del(void *data, Evas_Object *obj, void *event_info);
void test_focus_custom_chain(void *data, Evas_Object *obj, void *event_info);
void test_focus_style(void *data, Evas_Object *obj, void *event_info);
void test_focus_part(void *data, Evas_Object *obj, void *event_info);
void test_focus3(void *data, Evas_Object *obj, void *event_info);
void test_focus_object_style(void *data, Evas_Object *obj, void *event_info);
void test_focus_object_policy(void *data, Evas_Object *obj, void *event_info);
void test_focus4(void *data, Evas_Object *obj, void *event_info);
void test_flipselector(void *data, Evas_Object *obj, void *event_info);
void test_diskselector(void *data, Evas_Object *obj, void *event_info);
void test_colorselector(void *data, Evas_Object *obj, void *event_info);
void test_ctxpopup(void *data, Evas_Object *obj, void *event_info);
void test_bubble(void *data, Evas_Object *obj, void *event_info);
void test_segment_control(void *data, Evas_Object *obj, void *event_info);
void test_store(void *data, Evas_Object *obj, void *event_info);
void test_sys_notify(void *data, Evas_Object *obj, void *event_info);
void test_systray(void *data, Evas_Object *obj, void *event_info);
void test_win_inline(void *data, Evas_Object *obj, void *event_info);
void test_win_keygrab(void *data, Evas_Object *obj, void *event_info);
void test_win_socket(void *data, Evas_Object *obj, void *event_info);
void test_win_plug(void *data, Evas_Object *obj, void *event_info);
void test_win_wm_rotation(void *data, Evas_Object *obj, void *event_info);
void test_grid(void *data, Evas_Object *obj, void *event_info);
void test_grid_static(void *data, Evas_Object *obj, void *event_info);
void test_glview_simple(void *data, Evas_Object *obj, void *event_info);
void test_glview(void *data, Evas_Object *obj, void *event_info);
void test_glview_manygears(void *data, Evas_Object *obj, void *event_info);
void test_glview_gles3(void *data, Evas_Object *obj, void *event_info);
void test_3d(void *data, Evas_Object *obj, void *event_info);
void test_naviframe(void *data, Evas_Object *obj, void *event_info);
void test_naviframe2(void *data, Evas_Object *obj, void *event_info);
void test_naviframe3(void *data, Evas_Object *obj, void *event_info);
void test_naviframe_complex(void *data, Evas_Object *obj, void *event_info);
//void test_factory(void *data, Evas_Object *obj, void *event_info);
void test_datetime(void *data, Evas_Object *obj, void *event_info);
void test_ui_clock(void *data, Evas_Object *obj, void *event_info);
void test_popup(void *data, Evas_Object *obj, void *event_info);
void test_dayselector(void *data, Evas_Object *obj, void *event_info);
void test_image(void *data, Evas_Object *obj, void *event_info);
void test_image_swallow_align(void *data, Evas_Object *obj, void *event_info);
void test_remote_image(void *data, Evas_Object *obj, void *event_info);
void test_click_image(void *data, Evas_Object *obj, void *event_info);
void test_load_image(void *data, Evas_Object *obj, void *event_info);
void test_external_button(void *data, Evas_Object *obj, void *event_info);
void test_external_slider(void *data, Evas_Object *obj, void *event_info);
void test_external_scroller(void *data, Evas_Object *obj, void *event_info);
void test_external_pbar(void *data, Evas_Object *obj, void *event_info);
void test_external_video(void *data, Evas_Object *obj, void *event_info);
void test_external_icon(void *data, Evas_Object *obj, void *event_info);
void test_external_combobox(void *data, Evas_Object *obj, void *event_info);
void test_config(void *data, Evas_Object *obj, void *event_info);
void test_config_font_overlay(void *data, Evas_Object *obj, void *event_info);
void test_video(void *data, Evas_Object *obj, void *event_info);
void test_eio(void *data, Evas_Object *obj, void *event_info);
void test_web(void *data, Evas_Object *obj, void *event_info);
void test_web_ui(void *data, Evas_Object *obj, void *event_info);
void test_dnd_genlist_default_anim(void *data, Evas_Object *obj, void *event_info);
void test_dnd_genlist_user_anim(void *data, Evas_Object *obj, void *event_info);
void test_dnd_genlist_gengrid(void *data, Evas_Object *obj, void *event_info);
void test_dnd_multi_features(void *data, Evas_Object *obj, void *event_info);
void test_dnd_types(void *data, Evas_Object *obj, void *event_info);
void test_task_switcher(void *data, Evas_Object *obj, void *event_info);
void test_win_dialog(void *data, Evas_Object *obj, void *event_info);
void test_win_modal(void *data, Evas_Object *obj, void *event_info);
void test_box_disable(void *data, Evas_Object *obj, void *event_info);
void test_layout_disable(void *data, Evas_Object *obj, void *event_info);

void test_colorclass(void *data, Evas_Object *obj, void *event_info);

void test_code_welcome(void *data, Evas_Object *obj, void *event_info);
void test_code_editor(void *data, Evas_Object *obj, void *event_info);
void test_code_syntax(void *data, Evas_Object *obj, void *event_info);
void test_code_mirror(void *data, Evas_Object *obj, void *event_info);
void test_code_log(void *data, Evas_Object *obj, void *event_info);
void test_code_diff(void *data, Evas_Object *obj, void *event_info);
void test_code_diff_inline(void *data, Evas_Object *obj, void *event_info);

void test_efl_ui_text(void *data, Evas_Object *obj, void *event_info);
void test_efl_ui_text_label(void *data, Evas_Object *obj, void *event_info);
void test_evas_mask(void *data, Edje_Object *obj, void *event_info);
void test_gfx_filters(void *data, Evas_Object *obj, void *event_info);
void test_evas_snapshot(void *data, Evas_Object *obj, void *event_info);
void test_evas_map(void *data, Edje_Object *obj, void *event_info);
void test_efl_gfx_map(void *data, Edje_Object *obj, void *event_info);

Evas_Object *win, *tbx; // TODO: refactoring
void *tt;
Eina_List *tests;

struct elm_test
{
   const char *icon;
   const char *category;
   const char *name;
   Evas_Object *frame;
   Evas_Object *box;
   Evas_Object *btn;

   void (*cb)(void *, Evas_Object *, void *);
};

static int
_elm_test_sort(const void *pa, const void *pb)
{
   const struct elm_test *a = pa, *b = pb;
   int res = strcasecmp(a->category, b->category);
   if (res) return res;
   return strcasecmp(a->name, b->name);
}

static void
_elm_test_add(Eina_List **p_list, const char *icon, const char *category, const char *name, void (*cb)(void *, Evas_Object *, void *))
{
   struct elm_test *t = calloc(1, sizeof(struct elm_test));
   t->icon = icon;
   t->category = category;
   t->name = name;
   t->cb = cb;
   *p_list = eina_list_sorted_insert(*p_list, _elm_test_sort, t);
}

void
my_win_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

static void
_ui_tg_changed(void *data, Evas_Object *obj, void *event_info)
{
   (void) data; (void) event_info;
   elm_config_mirrored_set(elm_check_state_get(obj));
}

static void
_frame_clicked(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   printf("frame %p is: %s\n", obj, elm_frame_collapse_get(obj) ? "collapsed" : "expanded");
}

static void
_clear_menu()
{
   Eina_List *child, *children;
   Evas_Object *box, *obj, *obj1;

   child = elm_box_children_get(tbx);
   EINA_LIST_FREE(child, obj)
     {
        box = elm_object_content_get(obj);
        children = elm_box_children_get(box);
        EINA_LIST_FREE(children, obj1)
          {
             evas_object_hide(obj1);
          }
        elm_box_unpack_all(box);
        evas_object_hide(obj);
     }
   elm_box_unpack_all(tbx);
}

static void
_menu_create(const char *option_str)
{
   struct elm_test *t = NULL;
   const char *pcat = NULL;
   Evas_Object *cfr = NULL, *tbx2 = NULL, *bt = NULL, *ic = NULL;
   char buf[PATH_MAX];
   Eina_List *l;

   _clear_menu();
   EINA_LIST_FOREACH(tests, l, t)
     {
        if (option_str &&
            !(strcasestr(t->name, option_str) || strcasestr(t->category, option_str)))
          continue;
        if ((!pcat) || (strcmp(pcat, t->category)))
          {
             if (t->frame)
               {
                  elm_box_pack_end(tbx, t->frame);
                  evas_object_show(t->frame);
               }
             else
               {
                  cfr = elm_frame_add(win);
                  // FIXME: add new style of frame for this
                  evas_object_smart_callback_add(cfr, "clicked", _frame_clicked, NULL);
                  elm_frame_autocollapse_set(cfr, EINA_TRUE);
                  elm_object_text_set(cfr, t->category);
                  evas_object_size_hint_weight_set(cfr, EVAS_HINT_EXPAND, 0.0);
                  evas_object_size_hint_fill_set(cfr, EVAS_HINT_FILL, 0.0);
                  elm_box_pack_end(tbx, cfr);
                  evas_object_show(cfr);

                  tbx2 = elm_box_add(win);
                  elm_box_layout_set(tbx2, evas_object_box_layout_flow_horizontal, NULL, NULL);
                  evas_object_size_hint_weight_set(tbx2, EVAS_HINT_EXPAND, 0.0);
                  evas_object_size_hint_align_set(tbx2, EVAS_HINT_FILL, 0.0);
                  elm_box_align_set(tbx2, 0.0, 0.5);
                  elm_object_content_set(cfr, tbx2);
                  evas_object_show(tbx2);
               }
          }

        if (t->btn)
          {
             elm_box_pack_end(t->box, t->btn);
             evas_object_show(t->btn);
          }
        else
          {
             bt = elm_button_add(win);
             // FIXME: add new style of button for this like efm in e17
             elm_object_text_set(bt, t->name);
             if (t->icon)
               {
                  ic = elm_icon_add(win);
                  snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), t->icon);
                  elm_image_file_set(ic, buf, NULL);
                  elm_object_part_content_set(bt, "icon", ic);
                  evas_object_show(ic);
               }
             elm_box_pack_end(tbx2, bt);
             evas_object_show(bt);
             evas_object_smart_callback_add(bt, "clicked", t->cb, win);
             t->frame = cfr;
             t->box = tbx2;
             t->btn = bt;
          }
        pcat = t->category;
        if (t == tt) tt = cfr;
     }
}

static void
_entry_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   const char *str = elm_entry_entry_get(obj);
   if (!str) return;
   _menu_create(str);
}

static void
_entry_activated_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   const char *str = elm_entry_entry_get(obj);
   struct elm_test *t, *found = NULL, *exact = NULL;
   int count = 0;
   Eina_List *l;

   if (!str) return;
   EINA_LIST_FOREACH(tests, l, t)
     {
        if (!exact && !strcasecmp(t->name, str)) exact = t;
        if (!strcasestr(t->name, str)) continue;
        found = t;
        count++;
        if ((count > 1) && exact) break;
     }
   if (exact)
     evas_object_smart_callback_call(exact->btn, "clicked", NULL);
   else if (found && (count == 1))
     evas_object_smart_callback_call(found->btn, "clicked", NULL);
}

static char *
_space_removed_string_get(const char *name)
{
   int i = 0, j = 0, len;
   char *ret;

   len = strlen(name);
   ret = calloc(len + 1, sizeof(char));

   while (name[i])
     {
        if (name[i] != ' ')
          {
             ret[j] = name[i];
             j++;
          }
        i++;
     }

   return ret;
}

static void
my_win_main(const char *autorun, Eina_Bool test_win_only)
{
   Evas_Object *bg = NULL, *bx0 = NULL, *bx1 = NULL, *lb = NULL;
   Evas_Object *fr = NULL, *tg = NULL, *sc = NULL, *en = NULL;
   Eina_List *l = NULL;
   struct elm_test *t = NULL;

   if (test_win_only) goto add_tests;
   /* Create an elm window - It returns an evas object. This is a little
    * special as the object lives in the canvas that is inside the window
    * so what is returned is really inside the window, but as you manipulate
    * the evas object returned - the window will respond. elm_win makes sure
    * of that so you can blindly treat it like any other evas object
    * pretty much, just as long as you know it has special significance.
    * The first parameter is a "parent" window - eg for a dialog you want to
    * have a main window it is related to, here it is NULL meaning there
    * is no parent. "main" is the name of the window - used by the window
    * manager for identifying the window uniquely amongst all the windows
    * within this application (and all instances of the application). The
    * type is a basic window (the final parameter).
    * You can call elm_win_util_standard_add() instead. This is a convenient API
    * for window and bg creation. You don't need to create bg object manually.
    * You can also set the title of the window at the same time.
    *   ex) win = elm_win_util_standard_add("main", "Elementary Tests"); */
   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   if (!win) exit(1);

   explode_win_enable(win);
   /* Set the title of the window - This is in the titlebar. */
   elm_win_title_set(win, "Elementary Tests");

   /* Set a callback on the window when "delete,request" is emitted as
    * a callback. When this happens my_win_del() is called and the
    * data pointer (first param) is passed the final param here (in this
    * case it is NULL). This is how you can pass specific things to a
    * callback like objects or data layered on top. */
   evas_object_smart_callback_add(win, "delete,request", my_win_del, NULL);

   /* Add a background to our window. This just uses the standard theme set
    * background. Without a background, you could make a window seem
    * transparent with elm_win_alpha_set(win, EINA_TRUE); For example. if you
    * have a compositor running this will make the window able to be
    * semi-transparent and any space not filled by object/widget pixels will
    * be transparent or translucent based on alpha. If you do not have a
    * compositor running this should fall back to using shaped windows
    * (which have a mask). Both these features will be slow and rely on
    * a lot more resources, so only use it if you need it. */
   bg = elm_bg_add(win);
   /* Set weight to 1.0 x 1.0 == expand in both x and y direction. */
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   /* Tell the window that this object is to be resized along with the window.
    * Also as a result this object will be one of several objects that
    * controls the minimum/maximum size of the window. */
   elm_win_resize_object_add(win, bg);
   /* And show the background. */
   evas_object_show(bg);

   /* Add a box layout widget to the window. */
   bx0 = elm_box_add(win);
   /* Allow base box (bx0) to expand in x and y. */
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   /* Tell the window that the box affects window size and also will be
    * resized when the window is resized. */
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   fr = elm_frame_add(win);
   evas_object_smart_callback_add(fr, "clicked", _frame_clicked, NULL);
   elm_frame_autocollapse_set(fr, EINA_TRUE);
   elm_object_text_set(fr, "Information");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "Please select a test from the list below by clicking<br/>"
                       "the test button to show the test window.");
   elm_object_content_set(fr, lb);
   evas_object_show(lb);

   /* This label will not be read out */
   elm_access_object_unregister(lb);

   tg = elm_check_add(win);
   elm_object_style_set(tg, "toggle");
   elm_object_text_set(tg, "UI-Mirroring:");
   elm_check_state_set(tg, elm_config_mirrored_get());
   evas_object_smart_callback_add(tg, "changed", _ui_tg_changed, NULL);
   elm_box_pack_end(bx0, tg);
   evas_object_show(tg);

   bx1 = elm_box_add(win);
   elm_box_horizontal_set(bx1, EINA_TRUE);
   evas_object_size_hint_weight_set(bx1, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx1, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bx0, bx1);
   evas_object_show(bx1);

   lb = elm_label_add(win);
   elm_object_text_set(lb, " Search Menu :");
   evas_object_size_hint_weight_set(lb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(lb, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(bx1, lb);
   evas_object_show(lb);

   en = elm_entry_add(win);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_object_part_text_set(en, "guide", "Type widget name here to search.");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(en, "changed,user", _entry_changed_cb, NULL);
   evas_object_smart_callback_add(en, "activated", _entry_activated_cb, NULL);
   elm_box_pack_end(bx1, en);
   evas_object_show(en);
   elm_object_focus_set(en, EINA_TRUE);

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx0, sc);
   evas_object_show(sc);

   tbx = elm_box_add(win);
   evas_object_size_hint_weight_set(tbx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tbx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(sc, tbx);
   evas_object_show(tbx);

add_tests:
   tests = NULL;
#define ADD_TEST(icon_, cat_, name_, cb_) _elm_test_add(&tests, icon_, cat_, name_, cb_)

   //------------------------------//
   ADD_TEST(NULL, "Window / Background", "Bg Plain", test_bg_plain);
   ADD_TEST(NULL, "Window / Background", "Bg Image", test_bg_image);
   ADD_TEST(NULL, "Window / Background", "Bg Options", test_bg_options);
   ADD_TEST(NULL, "Window / Background", "Bg EOAPI (Efl.Ui.Win)", test_bg_window);
   ADD_TEST(NULL, "Window / Background", "Window States", test_win_state);
   ADD_TEST(NULL, "Window / Background", "Window States 2", test_win_state2);
   ADD_TEST(NULL, "Window / Background", "Inwin", test_inwin);
   ADD_TEST(NULL, "Window / Background", "Inwin 2", test_inwin2);
   ADD_TEST(NULL, "Window / Background", "Inwin 3", test_inwin3);
   ADD_TEST(NULL, "Window / Background", "Window Inline", test_win_inline);
   ADD_TEST(NULL, "Window / Background", "Window Socket", test_win_socket);
   ADD_TEST(NULL, "Window / Background", "Window Plug", test_win_plug);
   ADD_TEST(NULL, "Window / Background", "Window WM Rotation", test_win_wm_rotation);
   ADD_TEST(NULL, "Window / Background", "Window Standard/Dialog", test_win_dialog);
   ADD_TEST(NULL, "Window / Background", "Window Keygrab Set", test_win_keygrab);
   ADD_TEST(NULL, "Window / Background", "Window Modal", test_win_modal);
   ADD_TEST(NULL, "Window / Background", "Window Stack", test_win_stack);

   //------------------------------//
   ADD_TEST(NULL, "Images", "Icon", test_icon);
   ADD_TEST(NULL, "Images", "Icon Transparent", test_icon_transparent);
   ADD_TEST(NULL, "Images", "Icon Animation", test_icon_animated);
   ADD_TEST(NULL, "Images", "Icon Standard", test_icon_standard);
   ADD_TEST(NULL, "Images", "Photocam", test_photocam);
   ADD_TEST(NULL, "Images", "Photocam Remote", test_photocam_remote);
   ADD_TEST(NULL, "Images", "Photocam Icon", test_photocam_icon);
   ADD_TEST(NULL, "Images", "Image Zoomable animation", test_image_zoomable_animated);
   ADD_TEST(NULL, "Images", "Photo", test_photo);
   ADD_TEST(NULL, "Images", "Thumb", test_thumb);
   ADD_TEST(NULL, "Images", "Image", test_image);
   ADD_TEST(NULL, "Images", "Image Align", test_image_swallow_align);
   ADD_TEST(NULL, "Images", "Image Remote", test_remote_image);
   ADD_TEST(NULL, "Images", "Image Click", test_click_image);
   ADD_TEST(NULL, "Images", "Image Async Load", test_load_image);
   ADD_TEST(NULL, "Images", "Slideshow", test_slideshow);
   ADD_TEST(NULL, "Images", "Video", test_video);

   //------------------------------//
   ADD_TEST(NULL, "Containers", "Box Vert", test_box_vert);
   ADD_TEST(NULL, "Containers", "Box Vert 2", test_box_vert2);
   ADD_TEST(NULL, "Containers", "Box Pack", test_box_pack);
   ADD_TEST(NULL, "Containers", "Box Horiz", test_box_horiz);
   ADD_TEST(NULL, "Containers", "Box Homogeneous", test_box_homo);
   ADD_TEST(NULL, "Containers", "Box Transition", test_box_transition);
   ADD_TEST(NULL, "Containers", "Box Align", test_box_align);
   ADD_TEST(NULL, "Containers", "Box Stack", test_box_stack);
   ADD_TEST(NULL, "Containers", "Table", test_table);
   ADD_TEST(NULL, "Containers", "Table Homogeneous", test_table2);
   ADD_TEST(NULL, "Containers", "Table 3", test_table3);
   ADD_TEST(NULL, "Containers", "Table 4", test_table4);
   ADD_TEST(NULL, "Containers", "Table 5", test_table5);
   ADD_TEST(NULL, "Containers", "Table 6", test_table6);
   ADD_TEST(NULL, "Containers", "Table 7", test_table7);
   ADD_TEST(NULL, "Containers", "Table Padding", test_table8);
   ADD_TEST(NULL, "Containers", "Layout", test_layout);
   ADD_TEST(NULL, "Containers", "Layout 2", test_layout2);
   ADD_TEST(NULL, "Containers", "Grid", test_grid);

   //------------------------------//
   ADD_TEST(NULL, "New Containers", "Ui.Box", test_ui_box);
   ADD_TEST(NULL, "New Containers", "Ui.Grid", test_ui_grid);
   ADD_TEST(NULL, "New Containers", "Ui.Grid Linear", test_ui_grid_linear);
   ADD_TEST(NULL, "New Containers", "Ui.Grid.Static", test_grid_static);

   //------------------------------//
   ADD_TEST(NULL, "New Events", "Event Refeed", test_events);

   //------------------------------//
   ADD_TEST(NULL, "Entries", "Entry", test_entry);
   ADD_TEST(NULL, "Entries", "Entry Style Default", test_entry_style_default);
   ADD_TEST(NULL, "Entries", "Entry Style User", test_entry_style_user);
   ADD_TEST(NULL, "Entries", "Entry Scrolled", test_entry_scrolled);
   ADD_TEST(NULL, "Entries", "Entry on Page Scroll", test_entry_on_page_scroll);
   ADD_TEST(NULL, "Entries", "Entry 3", test_entry3);
   ADD_TEST(NULL, "Entries", "Entry 4", test_entry4);
   ADD_TEST(NULL, "Entries", "Entry 5", test_entry5);
   ADD_TEST(NULL, "Entries", "Entry 6", test_entry6);
   ADD_TEST(NULL, "Entries", "Entry 7", test_entry7);
   ADD_TEST(NULL, "Entries", "Entry 8", test_entry8);
   ADD_TEST(NULL, "Entries", "Entry Regex", test_entry_regex);
   ADD_TEST(NULL, "Entries", "Entry Notepad", test_entry_notepad);
   ADD_TEST(NULL, "Entries", "Multibuttonentry", test_multibuttonentry);
   ADD_TEST(NULL, "Entries", "Multibuttonentry Shrink", test_multibuttonentry2);
   ADD_TEST(NULL, "Entries", "Entry Anchor", test_entry_anchor);
   ADD_TEST(NULL, "Entries", "Entry Anchor2", test_entry_anchor2);
   ADD_TEST(NULL, "Entries", "Entry Emoticon", test_entry_emoticon);
   ADD_TEST(NULL, "Entries", "Entry Password", test_entry_password);
   ADD_TEST(NULL, "Entries", "Efl UI Text", test_efl_ui_text);
   ADD_TEST(NULL, "Entries", "Efl UI Text Label", test_efl_ui_text_label);

   //------------------------------//
   ADD_TEST(NULL, "Advanced Entries", "Code Entry Markup", test_code_welcome);
   ADD_TEST(NULL, "Advanced Entries", "Code Editor", test_code_editor);
   ADD_TEST(NULL, "Advanced Entries", "Code Syntax", test_code_syntax);
   ADD_TEST(NULL, "Advanced Entries", "Mirrored Editor", test_code_mirror);
   ADD_TEST(NULL, "Advanced Entries", "Logger", test_code_log);
   ADD_TEST(NULL, "Advanced Entries", "Diff Comparison", test_code_diff);
   ADD_TEST(NULL, "Advanced Entries", "Diff Inline", test_code_diff_inline);

   //------------------------------//
   ADD_TEST(NULL, "Buttons", "Button", test_button);

   //------------------------------//
   ADD_TEST(NULL, "Prefs", "Prefs", test_prefs);

   //------------------------------//
   ADD_TEST(NULL, "Effects", "Transit", test_transit);
   ADD_TEST(NULL, "Effects", "Transit Resizing", test_transit_resizing);
   ADD_TEST(NULL, "Effects", "Transit Flip", test_transit_flip);
   ADD_TEST(NULL, "Effects", "Transit Zoom", test_transit_zoom);
   ADD_TEST(NULL, "Effects", "Transit Blend", test_transit_blend);
   ADD_TEST(NULL, "Effects", "Transit Fade", test_transit_fade);
   ADD_TEST(NULL, "Effects", "Transit Resizable Flip",
            test_transit_resizable_flip);
   ADD_TEST(NULL, "Effects", "Transit Custom", test_transit_custom);
   ADD_TEST(NULL, "Effects", "Transit Chain", test_transit_chain);
   ADD_TEST(NULL, "Effects", "Transit Tween", test_transit_tween);
   ADD_TEST(NULL, "Effects", "Transit Bezier", test_transit_bezier);
   ADD_TEST(NULL, "Effects", "Flip", test_flip);
   ADD_TEST(NULL, "Effects", "Flip 2", test_flip2);
   ADD_TEST(NULL, "Effects", "Flip 3", test_flip3);
   ADD_TEST(NULL, "Effects", "Flip Interactive", test_flip_interactive);
   ADD_TEST(NULL, "Effects", "Flip To", test_flip_to);
   ADD_TEST(NULL, "Effects", "Flip Page", test_flip_page);
   ADD_TEST(NULL, "Effects", "Flip Page (EO API)", test_flip_page_eo);
   ADD_TEST(NULL, "Effects", "Animation", test_anim);

   //------------------------------//
   ADD_TEST(NULL, "Edje External", "ExtButton", test_external_button);
   ADD_TEST(NULL, "Edje External", "ExtProgressBar", test_external_pbar);
   ADD_TEST(NULL, "Edje External", "ExtScroller", test_external_scroller);
   ADD_TEST(NULL, "Edje External", "ExtSlider", test_external_slider);
   ADD_TEST(NULL, "Edje External", "ExtVideo", test_external_video);
   ADD_TEST(NULL, "Edje External", "ExtIcon", test_external_icon);
   ADD_TEST(NULL, "Edje External", "ExtCombobox", test_external_combobox);

   //------------------------------//
   ADD_TEST(NULL, "Toolbars", "Toolbar", test_toolbar);
   ADD_TEST(NULL, "Toolbars", "Toolbar 2", test_toolbar2);
   ADD_TEST(NULL, "Toolbars", "Toolbar 3", test_toolbar3);
   ADD_TEST(NULL, "Toolbars", "Toolbar 4", test_toolbar4);
   ADD_TEST(NULL, "Toolbars", "Toolbar 5", test_toolbar5);
   ADD_TEST(NULL, "Toolbars", "Toolbar 6", test_toolbar6);
   ADD_TEST(NULL, "Toolbars", "Toolbar 7", test_toolbar7);
   ADD_TEST(NULL, "Toolbars", "Toolbar 8", test_toolbar8);
   ADD_TEST(NULL, "Toolbars", "Toolbar Vertical", test_toolbar_vertical);
   ADD_TEST(NULL, "Toolbars", "Toolbar Focus", test_toolbar_focus);

   //------------------------------//
   ADD_TEST(NULL, "Lists - List", "List", test_list);
   ADD_TEST(NULL, "Lists - List", "List - Horizontal", test_list_horizontal);
   ADD_TEST(NULL, "Lists - List", "List 2", test_list2);
   ADD_TEST(NULL, "Lists - List", "List 3", test_list3);
   ADD_TEST(NULL, "Lists - List", "List 4", test_list4);
   ADD_TEST(NULL, "Lists - List", "List 5", test_list5);
   ADD_TEST(NULL, "Lists - List", "List 6", test_list6);
   ADD_TEST(NULL, "Lists - List", "List 7", test_list7);
   ADD_TEST(NULL, "Lists - List", "List Focus", test_list_focus);
   ADD_TEST(NULL, "Lists - List", "List Focus Horizontal", test_list_horiz_focus);
   ADD_TEST(NULL, "Lists - List", "List Separator", test_list_separator);
   ADD_TEST(NULL, "Lists - List", "List Multi Select", test_list_multi_select);

   //------------------------------//

   ADD_TEST(NULL, "Lists - Genlist", "Genlist", test_genlist);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist 2", test_genlist2);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist 3", test_genlist3);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist 4", test_genlist4);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist 5", test_genlist5);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist 7", test_genlist7);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Tree", test_genlist6);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Group", test_genlist8);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Group Tree", test_genlist9);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Decorate Item Mode", test_genlist10);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Decorate All Mode", test_genlist15);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Reorder Mode", test_genlist_reorder);
#ifdef HAVE_EIO
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Eio", test_eio);
#endif
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Textblock", test_genlist12);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Tree, Insert Sorted", test_genlist13);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Tree, Insert Relative", test_genlist14);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Flip Mode", test_genlist16);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Decorate Modes", test_genlist17);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Tree and Decorate All Mode", test_genlist18);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Full Widget", test_genlist19);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Item Search By Text", test_genlist20);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Focus", test_genlist_focus);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Item Styles", test_genlist_item_styles);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Multi Select", test_genlist_multi_select);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Del", test_genlist_del);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Filter", test_genlist_filter);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Show/Bring", test_genlist_show_bring);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Cache", test_genlist_cache);
   ADD_TEST(NULL, "Lists - Genlist", "Genlist Reusable Contents", test_genlist_reusable);

   //------------------------------//

   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid", test_gengrid);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid 2", test_gengrid2);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid Group", test_gengrid3);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid Show/Bring_in", test_gengrid4);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid Item Search by Text", test_gengrid5);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid Item Styles", test_gengrid_item_styles);
   ADD_TEST(NULL, "Lists - Gengrid", "Gengrid Update Speed", test_gengrid_speed);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid Focus", test_gengrid_focus);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid Update", test_gengrid_update);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid Disabled Item Focus", test_gengrid_disabled_item_focus);
   ADD_TEST(NULL, "Lists - Gengrid", "GenGrid Item Custom Size", test_gengrid_item_custom_size);

   //------------------------------//
   ADD_TEST(NULL, "General", "Scaling", test_scaling);
   ADD_TEST(NULL, "General", "Scaling 2", test_scaling2);

   //------------------------------//
   ADD_TEST(NULL, "3D", "Evas Map 3D", test_3d);
   ADD_TEST(NULL, "3D", "GLViewSimple", test_glview_simple);
   ADD_TEST(NULL, "3D", "GLView Gears", test_glview);
   ADD_TEST(NULL, "3D", "GLView Many Gears", test_glview_manygears);
   ADD_TEST(NULL, "3D", "GLView GL ES 3.x", test_glview_gles3);

   //------------------------------//
   ADD_TEST(NULL, "Web", "Web", test_web);
   ADD_TEST(NULL, "Web", "Web UI", test_web_ui);

   //------------------------------//
   ADD_TEST(NULL, "Input", "Gesture Layer", test_gesture_layer);
   ADD_TEST(NULL, "Input", "Gesture Layer 2", test_gesture_layer2);
   ADD_TEST(NULL, "Input", "Gesture Layer 3", test_gesture_layer3);
   ADD_TEST(NULL, "Input", "Multi Touch", test_multi);

   //------------------------------//
   ADD_TEST(NULL, "Selectors", "Index", test_index);
   ADD_TEST(NULL, "Selectors", "Index 2", test_index2);
   ADD_TEST(NULL, "Selectors", "Index 3", test_index3);
   ADD_TEST(NULL, "Selectors", "Index Horizontal", test_index_horizontal);
   ADD_TEST(NULL, "Selectors", "FileSelector", test_fileselector);
   ADD_TEST(NULL, "Selectors", "FileSelector Entry", test_fileselector_entry);
   ADD_TEST(NULL, "Selectors", "FileSelector Button", test_fileselector_button);
   ADD_TEST(NULL, "Selectors", "Actionslider", test_actionslider);
   ADD_TEST(NULL, "Selectors", "Menu", test_menu);
   ADD_TEST(NULL, "Selectors", "Menu 2", test_menu2);
   ADD_TEST(NULL, "Selectors", "DiskSelector", test_diskselector);
   ADD_TEST(NULL, "Selectors", "ColorSelector", test_colorselector);
   ADD_TEST(NULL, "Selectors", "Color Classes", test_colorclass);
   ADD_TEST(NULL, "Selectors", "SegmentControl", test_segment_control);
   ADD_TEST(NULL, "Selectors", "Hoversel", test_hoversel);
   ADD_TEST(NULL, "Selectors", "Hoversel Focus", test_hoversel_focus);
   ADD_TEST(NULL, "Selectors", "Radio", test_radio);
   ADD_TEST(NULL, "Selectors", "FlipSelector", test_flipselector);
   ADD_TEST(NULL, "Selectors", "DaySelector", test_dayselector);
   ADD_TEST(NULL, "Selectors", "Main menu", test_main_menu);
   ADD_TEST(NULL, "Selectors", "Combobox", test_combobox);

   //------------------------------//
   ADD_TEST(NULL, "Cursors", "Cursor", test_cursor);
   ADD_TEST(NULL, "Cursors", "Cursor 2", test_cursor2);
   ADD_TEST(NULL, "Cursors", "Cursor 3", test_cursor3);
   ADD_TEST(NULL, "Cursors", "Cursor Layout", test_cursor4);

   //------------------------------//
   ADD_TEST(NULL, "Scroller", "Scroller", test_scroller);
   ADD_TEST(NULL, "Scroller", "Scroller 2", test_scroller2);
   ADD_TEST(NULL, "Scroller", "Scroller 3", test_scroller3);
   ADD_TEST(NULL, "Scroller", "Page Scroller", test_scroller4);
   ADD_TEST(NULL, "Scroller", "Scroller on Popup", test_scroller5);

   //------------------------------//
   // FIXME: add frame test
   ADD_TEST(NULL, "Boundaries", "Bubble", test_bubble);
   ADD_TEST(NULL, "Boundaries", "Separator", test_separator);

   //------------------------------//
   ADD_TEST(NULL, "Range Values", "Spinner", test_spinner);
   ADD_TEST(NULL, "Range Values", "Slider", test_slider);
   ADD_TEST(NULL, "Range Values", "Progressbar", test_progressbar);
   ADD_TEST(NULL, "Range Values", "Progressbar 2", test_progressbar2);

   //------------------------------//
   ADD_TEST(NULL, "Booleans", "Check", test_check);
   ADD_TEST(NULL, "Booleans", "Check Toggle", test_check_toggle);

   //------------------------------//
   ADD_TEST(NULL, "Range Values", "Nstate", test_nstate);

   //------------------------------//
   ADD_TEST(NULL, "Popups", "Ctxpopup", test_ctxpopup);
   ADD_TEST(NULL, "Popups", "Hover", test_hover);
   ADD_TEST(NULL, "Popups", "Hover 2", test_hover2);
   ADD_TEST(NULL, "Popups", "Hover 3", test_hover3);
   ADD_TEST(NULL, "Popups", "Notify", test_notify);
   ADD_TEST(NULL, "Popups", "Tooltip", test_tooltip);
   ADD_TEST(NULL, "Popups", "Tooltip 2", test_tooltip2);
   ADD_TEST(NULL, "Popups", "Tooltip 3", test_tooltip3);
   ADD_TEST(NULL, "Popups", "Tooltip 4", test_tooltip4);
   ADD_TEST(NULL, "Popups", "Popup", test_popup);

   //------------------------------//
   ADD_TEST(NULL, "Times & Dates", "Calendar", test_calendar);
   ADD_TEST(NULL, "Times & Dates", "Calendar 2", test_calendar2);
   ADD_TEST(NULL, "Times & Dates", "Calendar 3", test_calendar3);
   ADD_TEST(NULL, "Times & Dates", "Clock", test_clock);
   ADD_TEST(NULL, "Times & Dates", "Clock Edit", test_clock_edit);
   ADD_TEST(NULL, "Times & Dates", "Clock Edit 2", test_clock_edit2);
   ADD_TEST(NULL, "Times & Dates", "Clock Pause", test_clock_pause);
   ADD_TEST(NULL, "Times & Dates", "Datetime", test_datetime);
   ADD_TEST(NULL, "Times & Dates", "Ui.Clock", test_ui_clock);

   //------------------------------//
   ADD_TEST(NULL, "Text", "Label", test_label);
   ADD_TEST(NULL, "Text", "Label Slide", test_label_slide);
   ADD_TEST(NULL, "Text", "Label Wrap", test_label_wrap);
   ADD_TEST(NULL, "Text", "Label Ellipsis", test_label_ellipsis);
   ADD_TEST(NULL, "Text", "Label Emoji", test_label_emoji);

   //------------------------------//
   ADD_TEST(NULL, "Stored Surface Buffer", "Launcher", test_launcher);
   ADD_TEST(NULL, "Stored Surface Buffer", "Launcher 2", test_launcher2);
   ADD_TEST(NULL, "Stored Surface Buffer", "Launcher 3", test_launcher3);

   //------------------------------//
   ADD_TEST(NULL, "Focus", "Focus", test_focus);
   ADD_TEST(NULL, "Focus", "Focus 2", test_focus2);
   ADD_TEST(NULL, "Focus", "Focus Hide/Del", test_focus_hide_del);
   ADD_TEST(NULL, "Focus", "Focus Custom Chain", test_focus_custom_chain);
   ADD_TEST(NULL, "Focus", "Focus Style", test_focus_style);
   ADD_TEST(NULL, "Focus", "Focus On Part", test_focus_part);
   ADD_TEST(NULL, "Focus", "Focus 3", test_focus3);
   ADD_TEST(NULL, "Focus", "Focus Object Style", test_focus_object_style);
   ADD_TEST(NULL, "Focus", "Focus Object Policy", test_focus_object_policy);
   ADD_TEST(NULL, "Focus", "Focus 4", test_focus4);

   //------------------------------//
   ADD_TEST(NULL, "Naviframe", "Naviframe", test_naviframe);
   ADD_TEST(NULL, "Naviframe", "Naviframe 2", test_naviframe2);
   ADD_TEST(NULL, "Naviframe", "Naviframe 3", test_naviframe3);
   ADD_TEST(NULL, "Naviframe", "Naviframe: Complex", test_naviframe_complex);

   //------------------------------//
   ADD_TEST(NULL, "Geographic", "Map", test_map);

   //------------------------------//
   ADD_TEST(NULL, "Dividers", "Panel", test_panel);
   ADD_TEST(NULL, "Dividers", "Panel Scrollable", test_panel2);
   ADD_TEST(NULL, "Dividers", "Panes", test_panes);
   ADD_TEST(NULL, "Dividers", "Panes minsize", test_panes_minsize);
   //------------------------------//
   ADD_TEST(NULL, "Standardization", "Conformant", test_conformant);
   ADD_TEST(NULL, "Standardization", "Conformant 2", test_conformant2);
   ADD_TEST(NULL, "Standardization", "Conformant indicator", test_conformant_indicator);

   //------------------------------//
   ADD_TEST(NULL, "Helpers", "Store", test_store);
//   ADD_TEST(NULL, "Helpers", "Factory", test_factory);

   //------------------------------//
   ADD_TEST(NULL, "System", "Notification", test_sys_notify);
   ADD_TEST(NULL, "System", "Systray Item", test_systray);

   //------------------------------//
   ADD_TEST(NULL, "Drag & Drop", "Genlist DnD Dflt Anim", test_dnd_genlist_default_anim);
   ADD_TEST(NULL, "Drag & Drop", "Genlist DnD User Anim", test_dnd_genlist_user_anim);
   ADD_TEST(NULL, "Drag & Drop", "Genlist-Gengrid DnD", test_dnd_genlist_gengrid);
   ADD_TEST(NULL, "Drag & Drop", "Features DnD", test_dnd_multi_features);
   ADD_TEST(NULL, "Drag & Drop", "Types DnD", test_dnd_types);

   //------------------------------//
   ADD_TEST(NULL, "Miscellaneous", "Copy And Paste", test_cnp);
   ADD_TEST(NULL, "Miscellaneous", "Weather", test_weather);
   ADD_TEST(NULL, "Miscellaneous", "Icon Desktops", test_icon_desktops);
   ADD_TEST(NULL, "Miscellaneous", "Floating Objects", test_floating);
   ADD_TEST(NULL, "Miscellaneous", "Configuration", test_config);
   ADD_TEST(NULL, "Miscellaneous", "Accessibility", test_access);
   ADD_TEST(NULL, "Miscellaneous", "Accessibility2", test_access2);
   ADD_TEST(NULL, "Miscellaneous", "Accessibility3", test_access3);
   ADD_TEST(NULL, "Miscellaneous", "Font overlay", test_config_font_overlay);

   //------------------------------//
   ADD_TEST(NULL, "Evas", "Masking", test_evas_mask);
   ADD_TEST(NULL, "Evas", "Gfx Filters", test_gfx_filters);
   ADD_TEST(NULL, "Evas", "Snapshot", test_evas_snapshot);
   ADD_TEST(NULL, "Evas", "Map", test_evas_map);
   ADD_TEST(NULL, "Evas", "Gfx Map", test_efl_gfx_map);

   //------------------------------//
   ADD_TEST(NULL, "Widgets Disable/Enable", "Box", test_box_disable);
   ADD_TEST(NULL, "Widgets Disable/Enable", "Layout", test_layout_disable);
#undef ADD_TEST

   if (autorun)
     {
        char *tmp = _space_removed_string_get(autorun);
        EINA_LIST_FOREACH(tests, l, t)
          {
             char *name;

             if (!t->name || !t->cb) continue;

             name = _space_removed_string_get(t->name);
             if (!strcasecmp(name, tmp))
               {
                  t->cb(NULL, NULL, NULL);
                  free(name);
                  break;
               }
             free(name);
          }
        free(tmp);

        if (!l)
          ERR("'%s' is not valid test case name\n", autorun);
     }
   tt = t;

   if (test_win_only)
     {
        EINA_LIST_FREE(tests, t)
          free(t);

        if (!l)
          elm_exit();

        return;
     }

   if (tests)
     _menu_create(NULL);

   /* bring in autorun frame */
   if (autorun)
     {
        Evas_Coord x, y;
        evas_object_geometry_get(tt, &x, &y, NULL, NULL);
        elm_scroller_region_bring_in(sc, x, y, 0, 0);
     }

   /* set an initial window size */
   evas_object_resize(win, 480 * elm_config_scale_get(), 480 * elm_config_scale_get());
   evas_object_show(win);
}

static char *
colorclass_tl_cb(char *str)
{
   /* FIXME: translate? */
   return str;
}

static Eina_List *
colorclass_list_cb(void)
{
   Eina_List *l, *ret = NULL;
   Eina_Iterator *it;
   Eina_File *f;

   it = edje_file_iterator_new();
   if (!it) return NULL;
   EINA_ITERATOR_FOREACH(it, f)
     {
        l = elm_color_class_util_edje_file_list(f);
        if (l)
          ret = eina_list_merge(ret, l);
     }
   eina_iterator_free(it);
   return ret;
}

static void
_main_loop_death(void *data EINA_UNUSED,
                 const Efl_Event *ev EINA_UNUSED)
{
   struct elm_test *t;

   EINA_LIST_FREE(tests, t)
     free(t);

   eina_log_domain_unregister(_log_domain);
}

/* this is your elementary main function - it MUST be called IMMEDIATELY
 * after elm_init() and MUST be passed argc and argv, and MUST be called
 * efl_main and not be static - must be a visible symbol with EAPI_MAIN infront */
EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   Efl_Loop_Arguments *arge = ev->info;
   Eina_Bool test_win_only = EINA_FALSE;
   char *autorun = NULL;

   if (arge->initialization)
     {
        _log_domain = eina_log_domain_register("elementary_test", NULL);

        efl_event_callback_add(ev->object, EFL_EVENT_DEL, _main_loop_death, NULL);

        elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

        /* tell elm about our app so it can figure out where to get files */
        elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
        elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
        elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
        elm_app_info_set(efl_main, "elementary", "images/logo.png");

        elm_color_class_translate_cb_set(colorclass_tl_cb);
        elm_color_class_list_cb_set(colorclass_list_cb);

        /* put here any init specific to this app like parsing args etc. */
     }

   /* if called with a single argument try to autorun a test with
    * the same name as the given param
    * ex:  elementary_test "Box Vert 2" */
   if (eina_array_count(arge->argv) == 1)
     {
        if (!strcmp(eina_array_data_get(arge->argv, 0), "--help"))
          {
             printf("Usages:\n"
                    "$ elementary_test\n"
                    "$ elementary_test --test-win-only [TEST_NAME]\n"
                    "$ elementary_test -to [TEST_NAME]\n\n"
                    "Examples:\n"
                    "$ elementary_test -to Button\n\n");
             efl_loop_quit(ev->object, 1);
             return ;
          }
        autorun = eina_array_data_get(arge->argv, 0);
     }
   else if (eina_array_count(arge->argv) == 2)
     {
        /* Just a workaround to make the shot module more
         * useful with elementary test. */
        if ((!strcmp(eina_array_data_get(arge->argv, 0), "--test-win-only")) ||
            (!strcmp(eina_array_data_get(arge->argv, 0), "-to")))
          {
             test_win_only = EINA_TRUE;
             autorun = eina_array_data_get(arge->argv, 1);
          }
     }

   my_win_main(autorun, test_win_only); /* create main window */

   /* FIXME: Hum, no exit code anywhere anymore ? */
}
/* all elementary apps should use this. but it should be placed right after
 * efl_main() */
EFL_MAIN()
