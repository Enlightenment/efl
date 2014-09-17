
extern "C"
{

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#define ELM_INTERFACE_ATSPI_ACTION_PROTECTED
#define ELM_INTERFACE_ATSPI_VALUE_PROTECTED
#define ELM_INTERFACE_ATSPI_EDITABLE_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_SELECTION_PROTECTED
#define ELM_INTERFACE_ATSPI_IMAGE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>

#include <elm_widget.h>
#include <elm_interface_scrollable.h>
#include <elm_interface_atspi_text.h>
#include <elm_interface_atspi_accessible.h>
#include <elm_interface_atspi_widget_action.h>
}

#include <elm_access.eo.hh>
#include <elm_actionslider.eo.hh>
#include <elm_app_client.eo.hh>
#include <elm_app_client_view.eo.hh>
#include <elm_app_server.eo.hh>
#include <elm_app_server_view.eo.hh>
#include <elm_atspi_app_object.eo.hh>
#include <elm_bg.eo.hh>
#include <elm_box.eo.hh>
#include <elm_bubble.eo.hh>
#include <elm_button.eo.hh>
#include <elm_calendar.eo.hh>
#include <elm_check.eo.hh>
#include <elm_clock.eo.hh>
#include <elm_colorselector.eo.hh>
#include <elm_conformant.eo.hh>
#include <elm_container.eo.hh>
#include <elm_ctxpopup.eo.hh>
#include <elm_datetime.eo.hh>
#include <elm_dayselector.eo.hh>
#include <elm_diskselector.eo.hh>
#include <elm_entry.eo.hh>
#include <elm_fileselector_button.eo.hh>
#include <elm_fileselector_entry.eo.hh>
#include <elm_fileselector.eo.hh>
#include <elm_flip.eo.hh>
#include <elm_flipselector.eo.hh>
#include <elm_frame.eo.hh>
#include <elm_gengrid.eo.hh>
#include <elm_gengrid_pan.eo.hh>
#include <elm_genlist.eo.hh>
#include <elm_genlist_pan.eo.hh>
#include <elm_gesture_layer.eo.hh>
#include <elm_glview.eo.hh>
#include <elm_grid.eo.hh>
#include <elm_hover.eo.hh>
#include <elm_hoversel.eo.hh>
#include <elm_icon.eo.hh>
#include <elm_image.eo.hh>
#include <elm_index.eo.hh>
#include <elm_interface_atspi_accessible.eo.hh>
#include <elm_interface_atspi_action.eo.hh>
#include <elm_interface_atspi_component.eo.hh>
#include <elm_interface_atspi_editable_text.eo.hh>
#include <elm_interface_atspi_image.eo.hh>
#include <elm_interface_atspi_selection.eo.hh>
#include <elm_interface_atspi_text.eo.hh>
#include <elm_interface_atspi_value.eo.hh>
#include <elm_interface_atspi_widget_action.eo.hh>
#include <elm_interface_atspi_widget.eo.hh>
#include <elm_interface_atspi_window.eo.hh>
#include <elm_interface_fileselector.eo.hh>
#include <elm_interface_scrollable.eo.hh>
#include <elm_inwin.eo.hh>
#include <elm_label.eo.hh>
#include <elm_layout.eo.hh>
#include <elm_list.eo.hh>
#include <elm_mapbuf.eo.hh>
#include <elm_map.eo.hh>
#include <elm_map_pan.eo.hh>
#include <elm_menu.eo.hh>
#include <elm_multibuttonentry.eo.hh>
#include <elm_naviframe.eo.hh>
#include <elm_notify.eo.hh>
#include <elm_panel.eo.hh>
#include <elm_pan.eo.hh>
#include <elm_panes.eo.hh>
#include <elm_photocam.eo.hh>
#include <elm_photocam_pan.eo.hh>
#include <elm_photo.eo.hh>
#include <elm_player.eo.hh>
#include <elm_plug.eo.hh>
#include <elm_popup.eo.hh>
#include <elm_prefs.eo.hh>
#include <elm_progressbar.eo.hh>
#include <elm_radio.eo.hh>
#include <elm_route.eo.hh>
#include <elm_scroller.eo.hh>
#include <elm_segment_control.eo.hh>
#include <elm_separator.eo.hh>
#include <elm_slider.eo.hh>
#include <elm_slideshow.eo.hh>
#include <elm_spinner.eo.hh>
#include <elm_systray.eo.hh>
#include <elm_table.eo.hh>
#include <elm_thumb.eo.hh>
#include <elm_toolbar.eo.hh>
#include <elm_video.eo.hh>
#include <elm_web.eo.hh>
#include <elm_widget.eo.hh>
#include <elm_win.eo.hh>

int main()
{
}
