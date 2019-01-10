/* This is the prmary Elementary header file that includes all other useful
 * headers.
 *
 * Elementary is distributed under the LGPLv2 license. Please see the COPYING
 * file that was distributed with this source.
 */
#ifndef ELEMENTARY_H
#define ELEMENTARY_H

/* Options which can be enabled or disabled by the buildtool */
#include "Elementary_Options.h"

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
#include <fnmatch.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#ifdef ELM_DIRENT_H
#include <dirent.h>
#endif
#include <pwd.h>
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

#ifdef ELM_WIN32
#include <malloc.h>
#ifndef alloca
#define alloca _alloca
#endif
#endif

/* EFL headers */
#include "Efl_Config.h"

#include <Eina.h>
#include <Eet.h>
#include <Evas.h>
#include <Evas_GL.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Ecore_Input.h>
#include <Ecore_IMF.h>
#include <Ecore_Con.h>
#include <Edje.h>
#include <Eldbus.h>
#include <Efreet.h>
#include <Efreet_Mime.h>
#include <Efreet_Trash.h>
#include <Ethumb_Client.h>
#include <Efl.h>

#ifdef ELM_ELOCATION
#include <Elocation.h>
#endif

#ifdef ELM_EMAP
#include <EMap.h>
#endif

#ifdef EAPI
# undef EAPI
#endif
#ifdef EWAPI
# undef EWAPI
#endif

#ifdef _WIN32
# ifdef ELEMENTARY_BUILD
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


/* allow usage from c++ */
#ifdef __cplusplus
extern "C"
{
#endif

/* docs */
#include <elm_intro.h>
#include <elm_getting_started.h>
#include <elm_authors.h>

#define ELM_VERSION_MAJOR EFL_VERSION_MAJOR
#define ELM_VERSION_MINOR EFL_VERSION_MINOR

typedef struct _Elm_Version
{
   int major;
   int minor;
   int micro;
   int revision;
} Elm_Version;

EAPI extern Elm_Version *elm_version;


/* include these first for general used definitions */
#include <elm_gen.h>
#include <elm_general.h>
#include <elm_config.h>
#include <elm_focus.h>

#if defined (EFL_EO_API_SUPPORT) && defined (EFL_BETA_API_SUPPORT)

//define focus manager earlier since focus object and manager is circular
typedef Eo Efl_Ui_Focus_Manager;
#define _EFL_UI_FOCUS_MANAGER_EO_CLASS_TYPE

# include <efl_ui_focus_object.eo.h>
# include <efl_ui_focus_manager.eo.h>
# include <efl_ui_focus_manager_window_root.eo.h>
# include <efl_ui_focus_manager_calc.eo.h>
# include <efl_ui_focus_manager_sub.eo.h>
# include <efl_ui_focus_manager_root_focus.eo.h>
# include <efl_ui_focus_util.eo.h>
# include <efl_ui_l10n.eo.h>
# include <efl_ui_focus_composition.eo.h>
# include <efl_ui_focus_layer.eo.h>
# include <elm_interface_scrollable.h>
# include <elm_interface_scrollable.eo.h>
# include <elm_spinner.eo.h>
#endif

#include <elm_tooltip.h>
#include <elm_object_item.h>
#include <elm_focus_item.h>

#include <elm_prefs_data.h>
#include <elm_prefs.h>
#include <elm_systray.h>
#include <elm_sys_notify.h>

/* special widgets - types used elsewhere */
#include <elm_access.h>
#include <elm_icon.h>
#include <elm_scroller.h>
#include <elm_entry.h>
#include <elm_list.h>

/* Interfaces */
#include <elm_interfaces.h>

/* other includes */
#include <elc_ctxpopup.h>
#include <elc_combobox.h>
#include <elm_dayselector.h>
#include <elc_fileselector_button.h>
#include <elc_fileselector_entry.h>
#include <elc_fileselector.h>
#include <elc_hoversel.h>
#include <elc_multibuttonentry.h>
#include <elc_naviframe.h>
#include <elc_popup.h>
#include <elm_actionslider.h>
#include <elm_app.h>
#include <elm_atspi_app_object.h>
#include <elm_atspi_bridge.h>
#include <elm_bg.h>
#include <elm_box.h>

#include <elm_bubble.h>
#include <elm_button.h>
#include <elm_cache.h>
#include <elm_calendar.h>
#include <efl_ui_calendar.h>
#include <elm_check.h>
#include <elm_clock.h>
#include <elm_cnp.h>
#include <elm_code.h>
#include <elm_colorselector.h>
#include <elm_color_class.h>
#include <elm_conform.h>
#include <elm_cursor.h>
#include <elm_datetime.h>
#include <elm_debug.h>
#include <elm_diskselector.h>

//#include <elm_factory.h>
#include <elm_animation_view.h>
#include <elm_finger.h>
#include <elm_flip.h>
#include <elm_flipselector.h>
#include <elm_font.h>
#include <elm_frame.h>
#include <elm_gengrid.h>
#include <elm_genlist.h>
#include <elm_gesture_layer.h>
#include <elm_glview.h>
#include <elm_grid.h>

#include <elm_hover.h>
#include <elm_image.h>
#include <elm_index.h>
#include <elm_inwin.h>
#include <elm_label.h>
#include <elm_layout.h>
#include <elm_macros.h>
#include <elm_map.h>
#include <elm_mapbuf.h>
#include <elm_menu.h>
#include <elm_mirroring.h>
#include <elm_need.h>
#include <elm_notify.h>
#include <efl_ui_nstate.h>
#include <elm_object.h>

#include <elm_panel.h>
#include <elm_panes.h>
#include <elm_photocam.h>
#include <elm_photo.h>
#include <elm_plug.h>
#include <elm_progressbar.h>
#include <elm_radio.h>
#include <elm_route.h>
#include <elm_scale.h>
#include <elm_scroll.h>
#include <elm_segment_control.h>

#include <elm_separator.h>
#include <elm_slider.h>
#include <elm_slideshow.h>
#include <elm_spinner.h>
#include <elm_store.h>
#include <elm_table.h>
#include <elm_textpath.h>
#include <elm_theme.h>
#include <elm_thumb.h>
#include <elm_toolbar.h>
#include <elm_transit.h>
#include <efl_ui_video.h>
#include <elm_view_list.h>
#include <elm_view_form.h>
#include <elm_web.h>
#include <elm_win.h>

#ifdef EFL_EO_API_SUPPORT
# include <efl_ui_selection_types.eot.h>
# include <efl_ui_theme.eo.h>
# include <efl_config_global.eo.h>
# include <efl_ui_widget.eo.h>
# include <efl_ui_widget_part.eo.h>
# include <efl_ui_widget_part_bg.eo.h>
# include <efl_ui_widget_part_shadow.eo.h>
# include <efl_ui_layout_part.eo.h>
# include <efl_ui_layout_part_box.eo.h>
# include <efl_ui_layout_part_content.eo.h>
# include <efl_ui_layout_part_bg.eo.h>
# include <efl_ui_layout_part_text.eo.h>
# include <efl_ui_layout_part_table.eo.h>
# include <efl_ui_win_part.eo.h>
# include <efl_ui_bg.eo.h>
# include <efl_ui_box.eo.h>
# include <efl_ui_box_flow.eo.h>
# include <efl_ui_box_stack.eo.h>
# include <efl_ui_table.eo.h>
# include <efl_ui_table_static.eo.h>
# include <efl_ui_image.eo.h>
# include <efl_ui_win.eo.h>
# include <efl_ui_win_inlined.eo.h>
# include <efl_ui_win_socket.eo.h>
# include <efl_text_interactive.eo.h>
# include <efl_ui_text.eo.h>
# include <efl_ui_popup.eo.h>
# include <efl_ui_alert_popup.eo.h>
# include <efl_ui_scroll_alert_popup.eo.h>
# include <efl_ui_text_alert_popup.eo.h>
# include <efl_ui_anchor_popup.eo.h>
# include <efl_ui_popup_part_backwall.eo.h>
# include <efl_ui_text_factory_images.eo.h>
# include <efl_ui_text_factory_emoticons.eo.h>
# include <efl_ui_text_factory_fallback.eo.h>
# include <efl_ui_text_editable.eo.h>
# include <efl_ui_text_async.eo.h>
# include <efl_ui_clock.eo.h>
# include <efl_ui_spin.eo.h>
# include <efl_ui_spin_button.eo.h>
# include <efl_ui_datepicker.eo.h>
# include <efl_ui_timepicker.eo.h>
# include <efl_ui_tags.eo.h>
# include <efl_ui_panel.eo.h>
# include <efl_ui_image_factory.eo.h>
# include <efl_ui_slider.eo.h>
# include <efl_ui_slider_interval.eo.h>
# include <efl_ui_layout_factory.eo.h>
# include <efl_ui_item.eo.h>
# include <efl_ui_list_item.eo.h>
# include <efl_ui_list_default_item_part_icon.eo.h>
# include <efl_ui_list_default_item_part_end.eo.h>
# include <efl_ui_list_default_item.eo.h>
# include <efl_ui_list_empty_item.eo.h>
# include <efl_ui_list.eo.h>
# include <efl_ui_grid_item.eo.h>
# include <efl_ui_grid_default_item_part_icon.eo.h>
# include <efl_ui_grid_default_item_part_end.eo.h>
# include <efl_ui_grid_default_item.eo.h>
# include <efl_ui_grid.eo.h>
# include <efl_ui_list_view_types.eot.h>
# include <efl_ui_list_view_seg_array.h>
# include <efl_ui_list_view_seg_array.eo.h>
# include <efl_ui_list_view_model.eo.h>
# include <efl_ui_list_view_relayout.eo.h>
# include <efl_ui_list_view.eo.h>
# include <efl_ui_list_view_pan.eo.h>
# include <efl_ui_pan.eo.h>
# include <efl_ui_scroll_manager.eo.h>
# include <efl_ui_scroller.eo.h>
# include <efl_ui_dnd_types.eot.h>
# include <efl_ui_selection.eo.h>
# include <efl_ui_dnd.eo.h>
# include <efl_ui_dnd_container.eo.h>
# include <efl_ui_navigation_bar.eo.h>
# include <efl_ui_navigation_bar_part.eo.h>
# include <efl_ui_navigation_bar_part_back_button.eo.h>
# include <efl_ui_navigation_layout.eo.h>
# include <efl_ui_stack.eo.h>

# ifndef _EFL_UI_PAGER_EO_CLASS_TYPE
# define _EFL_UI_PAGER_EO_CLASS_TYPE
typedef Eo Efl_Ui_Pager;
# endif

# include <efl_page_transition.eo.h>
# include <efl_page_transition_scroll.eo.h>
# include <efl_page_indicator.eo.h>
# include <efl_page_indicator_icon.eo.h>
# include <efl_ui_pager.eo.h>

# include <efl_ui_tab_bar.eo.h>
# include <efl_ui_tab_page.eo.h>
# include <efl_ui_tab_page_part_tab.eo.h>
# include <efl_ui_tab_pager.eo.h>
#endif

/* include deprecated calls last of all */
#include <elm_deprecated.h>

#include <elm_helper.h>

#ifdef __cplusplus
}
#endif

#ifndef ELEMENTARY_BUILD
# undef EAPI
# define EAPI
#endif

#endif
