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

#ifdef ELM_EMAP
#include <EMap.h>
#endif

#ifdef EAPI
# undef EAPI
#endif
#ifdef EWAPI
# undef EWAPI
#endif
#ifdef EAPI_WEAK
# undef EAPI_WEAK
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

/**
 * Elementary Version Information
 *
 * @ingroup Elm_General
 */
EAPI extern Elm_Version *elm_version;


/* include these first for general used definitions */
#include <elm_gen.h>
#include <elm_general.h>
#include <elm_config.h>
#include <elm_focus.h>

#include <Efl.h>

#ifdef EFL_BETA_API_SUPPORT
# include <elm_interface_scrollable.h>
# include <elm_interface_scrollable.eo.h>
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
/* this is elm_animation_view */
#include <efl_ui_vg_animation.h>
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
/* this is actually elm_video */
#include <efl_ui_video.h>
#include <elm_view_list.h>
#include <elm_view_form.h>
#include <elm_web.h>
#include <elm_win.h>

/* include deprecated calls last of all */
#include <elm_deprecated.h>

#include <elm_helper.h>

#ifdef __cplusplus
}
#endif

#ifndef EFL_BUILD
# undef EAPI
# define EAPI
#endif

#endif
