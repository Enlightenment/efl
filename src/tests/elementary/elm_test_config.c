#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

#include <stdbool.h>
typedef unsigned int uint;

EFL_START_TEST (elm_config_eoapi)
{
   Eo *cfg = efl_provider_find(efl_main_loop_get(), EFL_CONFIG_INTERFACE);
   fail_if(!cfg);

#define CONFIG_CHK(opt, typ, val) do { \
   typ old = elm_config_ ## opt ## _get(); \
   fail_if(old != efl_config_ ## typ ## _get(cfg, #opt)); \
   fail_if(!efl_config_ ## typ ## _set(cfg, #opt, val)); \
   fail_if(elm_config_ ## opt ## _get() != val); \
   fail_if(efl_config_ ## typ ## _get(cfg, #opt) != val); \
   } while (0)

#define CONFIG_CHKB(opt, val) CONFIG_CHK(opt, bool, val)
#define CONFIG_CHKI(opt, val) CONFIG_CHK(opt, int, val)
#define CONFIG_CHKD(opt, val) CONFIG_CHK(opt, double, val)

   // note: leaks badly
#define CONFIG_CHKS(opt, val) do { \
   const char *old = elm_config_ ## opt ## _get(); \
   fail_if(!eina_streq(old, efl_config_string_get(cfg, #opt))); \
   fail_if(!efl_config_string_set(cfg, #opt, val)); \
   fail_if(!eina_streq(elm_config_ ## opt ## _get(), val)); \
   fail_if(!eina_streq(efl_config_string_get(cfg, #opt), val)); \
   } while (0)

#define CONFIG_CHKE(opt, ival, sval) do { \
   elm_config_ ## opt ## _set(ival); \
   fail_if(!eina_streq(efl_config_string_get(cfg, #opt), sval)); \
   fail_if(!efl_config_string_set(cfg, #opt, sval)); \
   fail_if(!eina_streq(efl_config_string_get(cfg, #opt), sval)); \
   } while (0)

   CONFIG_CHKB(scroll_bounce_enabled, !old);
   CONFIG_CHKD(scroll_bounce_friction, 0);
   CONFIG_CHKD(scroll_page_scroll_friction, 0);
   CONFIG_CHKB(context_menu_disabled, !old);
   CONFIG_CHKD(scroll_bring_in_scroll_friction, 0);
   CONFIG_CHKD(scroll_zoom_friction, 0);
   CONFIG_CHKB(scroll_thumbscroll_enabled, !old);
   CONFIG_CHKI(scroll_thumbscroll_threshold, 0);
   CONFIG_CHKI(scroll_thumbscroll_hold_threshold, 0);
   CONFIG_CHKD(scroll_thumbscroll_momentum_threshold, 0);
   CONFIG_CHKI(scroll_thumbscroll_flick_distance_tolerance, 0);
   CONFIG_CHKD(scroll_thumbscroll_friction, 0);
   CONFIG_CHKD(scroll_thumbscroll_min_friction, 0);
   CONFIG_CHKD(scroll_thumbscroll_friction_standard, 0);
   CONFIG_CHKD(scroll_thumbscroll_border_friction, 0);
   CONFIG_CHKD(scroll_thumbscroll_sensitivity_friction, 1.0);
   CONFIG_CHKB(scroll_thumbscroll_smooth_start, 0);
   CONFIG_CHKB(scroll_animation_disabled, 0);
   CONFIG_CHKD(scroll_accel_factor, 0);
   CONFIG_CHKD(scroll_thumbscroll_smooth_amount, 0);
   CONFIG_CHKD(scroll_thumbscroll_smooth_time_window, 0);
   CONFIG_CHKD(scroll_thumbscroll_acceleration_threshold, 0);
   CONFIG_CHKD(scroll_thumbscroll_acceleration_time_limit, 0);
   CONFIG_CHKD(scroll_thumbscroll_acceleration_weight, 0);
   CONFIG_CHKE(focus_autoscroll_mode, EFL_UI_FOCUS_AUTOSCROLL_MODE_NONE, "none");
   CONFIG_CHKE(slider_indicator_visible_mode, EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS, "always");
   CONFIG_CHKD(longpress_timeout, 0);
   CONFIG_CHKE(softcursor_mode, EFL_UI_SOFTCURSOR_MODE_ON, "on");
   CONFIG_CHKD(tooltip_delay, 0);
   CONFIG_CHKB(cursor_engine_only, 0);
   CONFIG_CHKD(scale, 0);
   CONFIG_CHKS(icon_theme, ELM_CONFIG_ICON_THEME_ELEMENTARY);
   CONFIG_CHKB(password_show_last, 0);
   CONFIG_CHKD(password_show_last_timeout, 0);
   CONFIG_CHKS(preferred_engine, 0);
   CONFIG_CHKS(accel_preference, 0);
   //font_overlay
   CONFIG_CHKB(access, 0);
   CONFIG_CHKB(selection_unfocused_clear, 0);
   //elm_config_font_overlay_unset
   //CONFIG_CHKI(font_hint_type, 0); // this has no get!
   CONFIG_CHKI(finger_size, 0);
   CONFIG_CHKI(cache_flush_interval, 10);
   CONFIG_CHKB(cache_flush_enabled, !old);
   CONFIG_CHKI(cache_font_cache_size, 0);
   CONFIG_CHKI(cache_image_cache_size, 0);
   CONFIG_CHKI(cache_edje_file_cache_size, 0);
   CONFIG_CHKI(cache_edje_collection_cache_size, 0);
   CONFIG_CHKB(vsync, 0);
   CONFIG_CHKB(accel_preference_override, 0);
   CONFIG_CHKB(focus_highlight_enabled, !old);
   CONFIG_CHKB(focus_highlight_animate, 0);
   CONFIG_CHKB(focus_highlight_clip_disabled, !old);
   CONFIG_CHKE(focus_move_policy, EFL_UI_FOCUS_MOVE_POLICY_IN, "in");
   CONFIG_CHKB(item_select_on_focus_disabled, !old);
   CONFIG_CHKB(first_item_focus_on_first_focusin, 0);
   CONFIG_CHKB(mirrored, 0);
   //see clouseau tests
   //CONFIG_CHKB(clouseau_enabled, !old);
   CONFIG_CHKD(glayer_long_tap_start_timeout, 0);
   CONFIG_CHKD(glayer_double_tap_timeout, 0);
   //color_overlay
   //color_overlay_unset
   CONFIG_CHKB(magnifier_enable, 0);
   CONFIG_CHKD(magnifier_scale, 0);
   CONFIG_CHKB(window_auto_focus_enable, 0);
   CONFIG_CHKB(window_auto_focus_animate, 0);
   CONFIG_CHKB(popup_scrollable, 0);
   CONFIG_CHKB(atspi_mode, 0);
   CONFIG_CHKD(transition_duration_factor, 0);
   CONFIG_CHKS(web_backend, old); // no value change (requires web support)

   static const struct {
      Edje_Channel chan;
      const char  *name;
   } channels[] = {
   { EDJE_CHANNEL_EFFECT, "audio_mute_effect" },
   { EDJE_CHANNEL_BACKGROUND, "audio_mute_background" },
   { EDJE_CHANNEL_MUSIC, "audio_mute_music" },
   { EDJE_CHANNEL_FOREGROUND, "audio_mute_foreground" },
   { EDJE_CHANNEL_INTERFACE, "audio_mute_interface" },
   { EDJE_CHANNEL_INPUT, "audio_mute_input" },
   { EDJE_CHANNEL_ALERT, "audio_mute_alert" },
   { EDJE_CHANNEL_ALL, "audio_mute_all" },
   { EDJE_CHANNEL_ALL, "audio_mute" },
   };

   for (unsigned i = 0; i < (sizeof(channels) / sizeof(channels[0])); i++)
     {
        Eina_Bool b = elm_config_audio_mute_get(channels[i].chan);
        if (b != efl_config_bool_get(cfg, channels[i].name))
          fail(channels[i].name);
        efl_config_bool_set(cfg, channels[i].name, !b);
        if(efl_config_bool_get(cfg, channels[i].name) != !b)
          fail(channels[i].name);
        if(elm_config_audio_mute_get(channels[i].chan) != !b)
          fail(channels[i].name);
     }

}
EFL_END_TEST

EFL_START_TEST (elm_config_win)
{
   Eo *cfg = efl_provider_find(efl_main_loop_get(), EFL_CONFIG_INTERFACE);
   fail_if(!cfg);

   Eo *win = efl_add_ref(EFL_UI_WIN_CLASS, NULL);
   Eo *cfg2 = efl_provider_find(win, EFL_CONFIG_INTERFACE);
   fail_if(cfg != cfg2);

   elm_config_cache_flush_interval_set(42);
   fail_if(efl_config_int_get(win, "cache_flush_interval") != 42);

   efl_unref(win);
}
EFL_END_TEST

static inline Eina_Bool
_eina_list_string_has(const Eina_List *list, const char *str)
{
   const char *s;
   const Eina_List *l;

   EINA_LIST_FOREACH(list, l, s)
     if (eina_streq(str, s))
       return EINA_TRUE;

   return EINA_FALSE;
}

EFL_START_TEST (elm_config_profiles)
{
   // this only tests some of the profile APIs. we're not going to mess with
   // the global config during make check :)

   Eo *cfg = efl_provider_find(efl_main_loop_get(), EFL_CONFIG_INTERFACE);
   fail_if(!cfg);

   for (int hidden = 0; hidden <= 1; hidden++)
     {
        Eina_List *list;
        Eina_Iterator *it;
        const char *prof;
        unsigned int cnt = 0;

        list = (!hidden) ? elm_config_profile_list_get()
                         : elm_config_profile_list_full_get();
        it = efl_config_profile_iterate(cfg, hidden);
        EINA_ITERATOR_FOREACH(it, prof)
          {
             fail_if(!_eina_list_string_has(list, prof));
             cnt++;
          }
        fail_if(cnt != eina_list_count(list));
        elm_config_profile_list_free(list);
        eina_iterator_free(it);
     }

   const char *dir, *profile;
   Eina_Stringshare *str;

   profile = elm_config_profile_get();
   fail_if(!profile);
   dir = elm_config_profile_dir_get(profile, EINA_TRUE);
   str = efl_config_profile_dir_get(cfg, profile, EINA_TRUE);
   fail_if(!eina_streq(dir, str));
   elm_config_profile_dir_free(dir);
   eina_stringshare_del(str);

   dir = elm_config_profile_dir_get(profile, EINA_FALSE);
   str = efl_config_profile_dir_get(cfg, profile, EINA_FALSE);
   fail_if(!eina_streq(dir, str));
   elm_config_profile_dir_free(dir);
   eina_stringshare_del(str);

}
EFL_END_TEST

void elm_test_config(TCase *tc)
{
   tcase_add_test(tc, elm_config_eoapi);
   tcase_add_test(tc, elm_config_win);
   tcase_add_test(tc, elm_config_profiles);
}
