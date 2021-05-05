#ifndef _EFL_UI_H
#define _EFL_UI_H

#include <Efl_Config.h>
#include <Elementary_Options.h>

/* Standard headers for standard system calls etc. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <math.h>
#ifndef _WIN32
# include <fnmatch.h>
#endif
#include <limits.h>
#include <ctype.h>
#include <time.h>
#ifdef ELM_DIRENT_H
#include <dirent.h>
#endif
#ifndef _WIN32
# include <pwd.h>
#endif
#include <errno.h>

#ifdef ELM_UNIX
#include <locale.h>
#ifdef ELM_LIBINTL_H
#include <libintl.h>
#endif
#include <signal.h>
#include <grp.h>
#include <glob.h>
#endif

#ifdef ELM_ALLOCA_H
#include <alloca.h>
#endif

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# undef WIN32_LEAN_AND_MEAN
#elif defined (__FreeBSD__) || defined (__OpenBSD__)
# include <sys/select.h>
# include <signal.h>
#elif defined (__ANDROID__)
# include <sys/select.h>
#elif defined (__sun)
# include <sys/siginfo.h>
#else
# include <sys/time.h>
# include <signal.h>
#endif

#include <sys/types.h>

#define __EFL_UI_IS_REQUIRED

#include <Efl_Core.h>
#include <Efl_Net.h>

#include <Efl_Canvas.h>
#include <Efl_Layout.h>

#ifdef EAPI
# undef EAPI
#endif
#ifdef EWAPI
# undef EWAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
# define EAPI_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#   define EAPI_WEAK __attribute__ ((weak))
#  else
#   define EAPI
#   define EAPI_WEAK
# endif
# else
#  define EAPI
#  define EAPI_WEAK
# endif
#endif

#define EWAPI EAPI EAPI_WEAK

#ifdef __cplusplus
extern "C" {
#endif

EAPI extern double _efl_startup_time;

/** Successfully applied the requested style from the current theme. */
EAPI extern Eina_Error EFL_UI_THEME_APPLY_ERROR_NONE;

// EO types. Defined for legacy-only builds as legacy uses typedef of EO types.
#include "efl_ui.eot.h"

//define focus manager earlier since focus object and manager is circular
typedef Eo Efl_Ui_Focus_Manager;
#define _EFL_UI_FOCUS_MANAGER_EO_CLASS_TYPE

# include <efl_ui_focus_object.eo.h>
# include <efl_ui_focus_manager.eo.h>
# ifdef EFL_BETA_API_SUPPORT
EAPI void efl_ui_focus_relation_free(Efl_Ui_Focus_Relations *rel);
# endif
# include <efl_ui_focus_manager_window_root.eo.h>
# include <efl_ui_focus_manager_calc.eo.h>
# include <efl_ui_focus_manager_sub.eo.h>
# include <efl_ui_focus_manager_root_focus.eo.h>
# include <efl_ui_focus_util.eo.h>
# include <efl_ui_textpath.eo.h>
# include <efl_ui_l10n.eo.h>
# include <efl_ui_focus_composition.eo.h>
# include <efl_ui_focus_layer.eo.h>
# include <efl_access_object.h>
# include <efl_access_text.h>
# include <efl_access_window.h>

# include <efl_access_action.eo.h>
# include <efl_access_component.eo.h>
# include <efl_access_editable_text.eo.h>
# include <efl_access_selection.eo.h>

# include <efl_ui_theme.eo.h>
# include <efl_config_global.eo.h>
# include <efl_ui_widget.eo.h>
# include <efl_ui_widget_scrollable_content.eo.h>
# include <efl_ui_widget_common.h>
# include <efl_ui_widget_part.eo.h>
# include <efl_ui_widget_part_bg.eo.h>
# include <efl_ui_widget_part_shadow.eo.h>
# include <efl_ui_layout_base.eo.h>
# include <efl_ui_layout.eo.h>
# include <efl_ui_layout_part.eo.h>
# include <efl_ui_layout_part_bg.eo.h>
# include <efl_ui_layout_part_box.eo.h>
# include <efl_ui_layout_part_content.eo.h>
# include <efl_ui_layout_part_text.eo.h>
# include <efl_ui_layout_part_table.eo.h>
# include <efl_ui_item.eo.h>
# include <efl_ui_default_item.eo.h>
# include <efl_ui_group_item.eo.h>
# include <efl_ui_win_part.eo.h>
# include <efl_ui_bg.eo.h>
# include <efl_ui_box.eo.h>
# include <efl_ui_box_flow.eo.h>
# include <efl_ui_box_stack.eo.h>
# include <efl_ui_table.eo.h>
# include <efl_ui_table_static.eo.h>
# include <efl_ui_image.eo.h>
# include <efl_ui_image_zoomable.eo.h>
# include <efl_ui_win.eo.h>
/* FIXME: what the actual fuck. */
#ifdef EFL_BETA_API_SUPPORT
/**
 * @brief Set the window's autodel state.
 *
 * When closing the window in any way outside of the program control, like
 * pressing the X button in the titlebar or using a command from the Window
 * Manager, a "delete,request" signal is emitted to indicate that this event
 * occurred and the developer can take any action, which may include, or not,
 * destroying the window object.
 *
 * When the @c autodel parameter is set, the window will be automatically
 * destroyed when this event occurs, after the signal is emitted. If @c autodel
 * is @c false, then the window will not be destroyed and is up to the program
 * to do so when it's required.
 *
 * @param[in] obj The object.
 * @param[in] autodel If @c true, the window will automatically delete itself
 * when closed.
 *
 * Note: This function is only available in C.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI void efl_ui_win_autodel_set(Efl_Ui_Win *obj, Eina_Bool autodel);

/**
 * @brief Get the window's autodel state.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window will automatically delete itself when closed.
 *
 * Note: This function is only available in C.
 *
 * @ingroup Efl_Ui_Win
 */
EAPI Eina_Bool efl_ui_win_autodel_get(const Efl_Ui_Win *obj);
#endif
# include <efl_ui_win_inlined.eo.h>
# include <efl_ui_win_socket.eo.h>
# include <efl_ui_relative_container.eo.h>

// FIXME - Text object must stop using Context_Item_Clicked_Cb!
typedef void (*Context_Item_Clicked_Cb)(void *data, Eo *obj, void *event_info);

# include <efl_text_interactive.eo.h>
# include <efl_ui_textbox.eo.h>
# include <efl_ui_textbox_async.eo.h>

# include <efl_ui_vg_animation.eo.h>
# include <efl_ui_clock.eo.h>
# include <efl_ui_video.eo.h>

# include <efl_ui_widget_factory.eo.h>
# include <efl_ui_image_factory.eo.h>
# include <efl_ui_layout_factory.eo.h>
# include <efl_ui_caching_factory.eo.h>
# include <efl_ui_text_factory_fallback.eo.h>
# include <efl_ui_text_factory_images.eo.h>
# include <efl_ui_text_factory_emoticons.eo.h>

/* FIXME: Multibuttonentry must not use elm_widget_item */

# include <efl_ui_tags.eo.h>
# include <efl_ui_button.eo.h>
# include <efl_ui_check.eo.h>
# include <efl_ui_radio.eo.h>
# include <efl_ui_radio_group.eo.h>
# include <efl_ui_radio_group_impl.eo.h>
# include <efl_ui_radio_box.eo.h>
# include <efl_ui_progressbar.eo.h>

# include <efl_ui_flip.eo.h>
# include <efl_ui_frame.eo.h>
# include <efl_ui_panel.eo.h>
# include <efl_ui_panes.eo.h>
# include <efl_ui_panes_part.eo.h>

#define _EFL_UI_SPOTLIGHT_MANAGEREO_EO_CLASS_TYPE
typedef Eo Efl_Ui_Spotlight_Manager;
#define _EFL_UI_SPOTLIGHT_INDICATOR_EO_CLASS_TYPE
typedef Eo Efl_Ui_Spotlight_Indicator;

# include <efl_ui_spotlight_container.eo.h>
# include <efl_ui_spotlight_manager.eo.h>
# include <efl_ui_spotlight_indicator.eo.h>
# include <efl_ui_spotlight_icon_indicator.eo.h>
# include <efl_ui_spotlight_scroll_manager.eo.h>
# include <efl_ui_spotlight_fade_manager.eo.h>
# include <efl_ui_spotlight_animation_manager.eo.h>
# include <efl_ui_spotlight_util.eo.h>
# include <efl_ui_stack.eo.h>
# include <efl_ui_pager.eo.h>

# include <efl_ui_navigation_bar.eo.h>
# include <efl_ui_navigation_bar_part.eo.h>
# include <efl_ui_navigation_bar_part_back_button.eo.h>
# include <efl_ui_navigation_layout.eo.h>
# include <efl_ui_action_connector.eo.h>
# include <efl_ui_format.eo.h>
# include <efl_ui_spin.eo.h>
# include <efl_ui_spin_button.eo.h>
# include <efl_ui_slider.eo.h>
# include <efl_ui_slider_interval.eo.h>
# include <efl_ui_item.eo.h>
# include <efl_ui_position_manager_entity.eo.h>
# include <efl_ui_position_manager_data_access_v1.eo.h>
# include <canvas/efl_input_clickable.eo.h>
# include <efl_ui_item_clickable.eo.h>
# include <efl_ui_collection.eo.h>
# include <efl_ui_position_manager_list.eo.h>
# include <efl_ui_position_manager_grid.eo.h>
# include <efl_ui_list_default_item.eo.h>
# include <efl_ui_grid_default_item.eo.h>
# include <efl_ui_list_placeholder_item.eo.h>
# include <efl_ui_selectable.eo.h>
# include <efl_ui_single_selectable.eo.h>
# include <efl_ui_multi_selectable.eo.h>
# include <efl_ui_multi_selectable_object_range.eo.h>
# include <efl_ui_multi_selectable_index_range.eo.h>
# include <efl_ui_popup.eo.h>
# include <efl_ui_alert_popup.eo.h>
# include <efl_ui_popup_part_backwall.eo.h>
# include <efl_ui_grid.eo.h>
# include <efl_ui_list.eo.h>
# include <efl_ui_tab_bar_default_item.eo.h>
# include <efl_ui_tab_bar.eo.h>
# include <efl_ui_tab_page.eo.h>
# include <efl_ui_tab_pager.eo.h>
# include <efl_ui_select_model.eo.h>

# include <efl_ui_list_view.eo.h>
# include <efl_ui_view_model.eo.h>
# include <efl_ui_collection_view.eo.h>
# include <efl_ui_list_view.eo.h>
# include <efl_ui_grid_view.eo.h>

# include <efl_ui_scroller.eo.h>
# include <efl_ui_pan.eo.h>
# include <efl_ui_scroll_manager.eo.h>

# include <efl_ui_widget_focus_manager.eo.h>
# include <efl_ui_selection.eo.h>
# include <efl_ui_dnd.eo.h>

# include <efl_ui_timepicker.eo.h>
# include <efl_ui_datepicker.eo.h>
# include <efl_ui_calendar.eo.h>
# include <efl_ui_separator.eo.h>

/**
 * Initialize Elementary
 *
 * @param[in] argc System's argument count value
 * @param[in] argv System's pointer to array of argument strings
 * @return The init counter value.
 *
 * This function initializes Elementary and increments a counter of
 * the number of calls to it. It returns the new counter's value.
 *
 * @warning This call is exported only for use by the @c ELM_MAIN()
 * macro. There is no need to use this if you use this macro (which
 * is highly advisable). An elm_main() should contain the entry
 * point code for your application, having the same prototype as
 * elm_init(), and @b not being static (putting the @c EAPI_MAIN symbol
 * in front of its type declaration is advisable). The @c
 * ELM_MAIN() call should be placed just after it.
 *
 * Example:
 * @dontinclude bg_example_01.c
 * @skip static void
 * @until ELM_MAIN
 *
 * See the full @ref bg_example_01_c "example".
 *
 * @see elm_shutdown().
 * @ingroup Elm_General
 */
EAPI int       elm_init(int argc, char **argv);

/**
 * Shut down Elementary
 *
 * @return The init counter value.
 *
 * This should be called at the end of your application, just
 * before it ceases to do any more processing. This will clean up
 * any permanent resources your application may have allocated via
 * Elementary that would otherwise persist.
 *
 * @see elm_init() for an example
 *
 * @note elm_shutdown() will iterate main loop until all ecore_evas are freed.
 * There is a possibility to call your ecore callbacks(timer, animator, event,
 * job, and etc.) in elm_shutdown()
 *
 * @ingroup Elm_General
 */
EAPI int       elm_shutdown(void);

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif

// We are including efl_general.h again, just in case Efl_Core.h was already included before this
// and the __EFL_*_IS_REQUIRED changed since then.
#include <efl_general.h>
