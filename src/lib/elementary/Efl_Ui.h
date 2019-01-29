#ifndef _EFL_UI_H
#define _EFL_UI_H

#include <Efl_Config.h>
#include <Elementary_Options.h>

#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif

#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

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

#include <Eina.h>
#include <Eo.h>
#include <Efl.h>

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
# if !defined (EXOTIC_NO_SIGNAL)
#  include <signal.h>
# endif
#endif

#include <sys/types.h>

#define __EFL_UI_IS_REQUIRED

#include <Efl_Core.h>
#include <Efl_Net.h>

#include <Evas.h>
#ifndef EFL_NOLEGACY_API_SUPPORT
# include <Evas_GL.h>
#endif
#include <Eio.h>
#include <Edje.h>
#include <Eldbus.h>
#include <Efreet.h>
#include <Efreet_Mime.h>
#include <Efreet_Trash.h>
#include <Ethumb_Client.h>

#ifdef ELM_ELOCATION
#include <Elocation.h>
#endif

#ifdef ELM_EMAP
#include <EMap.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern EAPI double _efl_startup_time;

// EO types. Defined for legacy-only builds as legacy uses typedef of EO types.
#include "efl_ui.eot.h"
#include "efl_ui_selection_types.eot.h"

//define focus manager earlier since focus object and manager is circular
typedef Eo Efl_Ui_Focus_Manager;
#define _EFL_UI_FOCUS_MANAGER_EO_CLASS_TYPE

# include <efl_ui_focus_object.eo.h>
# include <efl_ui_focus_manager.eo.h>
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
# include <efl_access_value.eo.h>

# include <efl_ui_theme.eo.h>
# include <efl_config_global.eo.h>
# include <efl_ui_widget_part.eo.h>
# include <efl_ui_layout.eo.h>
# include <efl_ui_layout_part.eo.h>
# include <efl_ui_layout_part_box.eo.h>
# include <efl_ui_layout_part_content.eo.h>
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

/* FIXME: Efl.Ui.Text must not use elm_general.h */
# warning Efl.Ui.Text is not available yet without Elementary.h
# if 0
# include <efl_ui_text_interactive.eo.h>
# include <efl_ui_text.eo.h>
# include <efl_ui_text_editable.eo.h>
# include <efl_ui_text_async.eo.h>
# endif

# include <efl_ui_clock.h>
# include <efl_ui_image_factory.eo.h>
# include <efl_ui_video.h>
# include <efl_ui_nstate.h>
# include <efl_ui_calendar.h>
# include <efl_ui_button_eo.h>

# include "efl_ui_caching_factory.eo.h"

/* FIXME: Multibuttonentry must not use elm_widget_item */
# warning Efl.Ui.Multibutton is not available yet without Elementary.h
# if 0
# include <efl_ui_multibuttonentry.h>
# endif

# include <efl_ui_flip_eo.h>
# include <efl_ui_frame_eo.h>
# include <efl_ui_check_eo.h>
# include <efl_ui_image_zoomable_eo.h>
# include <efl_ui_progressbar_eo.h>
# include <efl_ui_radio_eo.h>
# include <efl_ui_panes_eo.h>

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
