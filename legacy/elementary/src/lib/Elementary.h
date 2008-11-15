/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef ELEMENTARY_H
#define ELEMENTARY_H

/* What is this?
 * 
 * This is a VERY SIMPLE toolkit - VERY SIMPLE. It is not meant for writing
 * extensive applications. Small simple ones with simple needs. It is meant
 * to make the programmers work almost brainless.
 * 
 * I'm toying with backing this with etk or ewl - or not. right now I am
 * unsure as some of the widgets will be faily complex edje creations. Do
 * not consider any choices permanent - but the API should stay unbroken.
 * 
 */

#include "elementary_config.h"

/* Standard headers for standard system calls etc. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <dlfcn.h>
#include <math.h>
#include <fnmatch.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <glob.h>
#include <locale.h>
#include <libintl.h>
#include <errno.h>
#include <signal.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

/* EFL headers */
#include <Eina.h>
#include <Eet.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>
#include <Ecore_Job.h>
#include <Ecore_Txt.h>
#include <Ecore_File.h>
#include <Edje.h>

/* allow usage from c++ */
#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/   
   /* Objects */
   typedef enum _Elm_Win_Type
     {
	ELM_WIN_BASIC,
	  ELM_WIN_DIALOG_BASIC
     } Elm_Win_Type;
   
/**************************************************************************/   
   /* General calls */
   EAPI void         elm_init(int argc, char **argv);
   EAPI void         elm_shutdown(void);
   EAPI void         elm_run(void);
   EAPI void         elm_exit(void);
   
   EAPI Evas_Object *elm_win_add(Evas_Object *parent, const char *name, Elm_Win_Type type);
   EAPI void         elm_win_resize_object_add(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_win_resize_object_del(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_win_title_set(Evas_Object *obj, const char *title);
   EAPI void         elm_win_autodel_set(Evas_Object *obj, Evas_Bool autodel);
   EAPI void         elm_win_activate(Evas_Object *obj);
   EAPI void         elm_win_borderless_set(Evas_Object *obj, Evas_Bool borderless);
   EAPI void         elm_win_shaped_set(Evas_Object *obj, Evas_Bool shaped);
   EAPI void         elm_win_alpha_set(Evas_Object *obj, Evas_Bool alpha);
   EAPI void         elm_win_override_set(Evas_Object *obj, Evas_Bool override);
// FIXME: implement more of the above calls
   /* smart callbacks called:
    * "delete-request" - the user requested to delete the window
    */

   EAPI Evas_Object *elm_bg_add(Evas_Object *parent);
   EAPI void         elm_bg_file_set(Evas_Object *obj, const char *file, const char *group);
   /* smart callbacks called:
    */

   EAPI Evas_Object *elm_icon_add(Evas_Object *parent);
   EAPI void         elm_icon_file_set(Evas_Object *obj, const char *file, const char *group);
   EAPI void         elm_icon_standard_set(Evas_Object *obj, const char *name);
   EAPI void         elm_icon_smooth_set(Evas_Object *obj, Evas_Bool smooth);
   EAPI void         elm_icon_no_scale_set(Evas_Object *obj, Evas_Bool no_scale);
   EAPI void         elm_icon_scale_set(Evas_Object *obj, Evas_Bool scale_up, Evas_Bool scale_down);
   EAPI void         elm_icon_fill_outside_set(Evas_Object *obj, Evas_Bool fill_outside);
   /* smart callbacks called:
    * "clicked" - the user clicked the icon
    */

   EAPI Evas_Object *elm_box_add(Evas_Object *parent);
   EAPI void         elm_box_horizontal_set(Evas_Object *obj, Evas_Bool horizontal);
   EAPI void         elm_box_homogenous_set(Evas_Object *obj, Evas_Bool homogenous);
   EAPI void         elm_box_pack_start(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_box_pack_end(Evas_Object *obj, Evas_Object *subobj);
   EAPI void         elm_box_pack_before(Evas_Object *obj, Evas_Object *subobj, Evas_Object *before);
   EAPI void         elm_box_pack_after(Evas_Object *obj, Evas_Object *subobj, Evas_Object *after);
   /* smart callbacks called:
    */
   
   EAPI Evas_Object *elm_button_add(Evas_Object *parent);
   EAPI void         elm_button_label_set(Evas_Object *obj, const char *label);
   EAPI void         elm_button_icon_set(Evas_Object *obj, Evas_Object *icon);
   /* smart callbacks called:
    * "clicked" - the user clicked the button
    */
   
   EAPI Evas_Object *elm_scroller_add(Evas_Object *parent);
   EAPI void         elm_scroller_content_set(Evas_Object *obj, Evas_Object *child);
   EAPI void         elm_scroller_content_min_limit(Evas_Object *obj, Evas_Bool w, Evas_Bool h);
   /* smart callbacks called:
    */

   EAPI Evas_Object *elm_label_add(Evas_Object *parent);
   EAPI void         elm_label_label_set(Evas_Object *obj, const char *label);
   /* smart callbacks called:
    */
       
   EAPI Evas_Object *elm_toggle_add(Evas_Object *parent);
   EAPI void         elm_toggle_label_set(Evas_Object *obj, const char *label);
   EAPI void         elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon);
   EAPI void         elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel);
   EAPI void         elm_toggle_state_set(Evas_Object *obj, Evas_Bool state);
   EAPI void         elm_toggle_state_pointer_set(Evas_Object *obj, Evas_Bool *statep);
   /* smart callbacks called:
    * "changed" - the user toggled the state
    */
   
   EAPI Evas_Object *elm_frame_add(Evas_Object *parent);
   EAPI void         elm_frame_label_set(Evas_Object *obj, const char *label);
   EAPI void         elm_frame_content_set(Evas_Object *obj, Evas_Object *content);
   /* smart callbacks called:
    */
       
   EAPI Evas_Object *elm_table_add(Evas_Object *parent);
   EAPI void         elm_table_homogenous_set(Evas_Object *obj, Evas_Bool homogenous);
   EAPI void         elm_table_pack(Evas_Object *obj, Evas_Object *subobj, int x, int y, int w, int h);
   /* smart callbacks called:
    */
   
   EAPI Evas_Object *elm_clock_add(Evas_Object *parent);
   EAPI void         elm_clock_time_set(Evas_Object *obj, int hrs, int min, int sec);
   EAPI void         elm_clock_time_get(Evas_Object *obj, int *hrs, int *min, int *sec);
   EAPI void         elm_clock_edit_set(Evas_Object *obj, Evas_Bool edit);
   EAPI void         elm_clock_show_am_pm_set(Evas_Object *obj, Evas_Bool am_pm);
   EAPI void         elm_clock_show_seconds_set(Evas_Object *obj, Evas_Bool seconds);
   /* smart callbacks called:
    * "changed" - the user changed the time
    */

   EAPI Evas_Object *elm_layout_add(Evas_Object *parent);
   EAPI void         elm_layout_file_set(Evas_Object *obj, const char *file, const char *group);
   EAPI void         elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);
   EAPI Evas_Object *elm_layout_edje_get(Evas_Object *obj);
   /* smart callbacks called:
    */
   
   EAPI Evas_Object *elm_hover_add(Evas_Object *parent);
   EAPI void         elm_hover_target_set(Evas_Object *obj, Evas_Object *target);
   EAPI void         elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent);
   EAPI void         elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);
   EAPI void         elm_hover_style_set(Evas_Object *obj, const char *style);
   /* smart callbacks called:
    * "clicked" - the user clicked the empty space in the hover to dismiss
    */

   typedef struct _Elm_Entry_Anchor_Info Elm_Entry_Anchor_Info;
   struct _Elm_Entry_Anchor_Info
     {
	const char *name;
	int button;
	Evas_Coord x, y, w, h;
     };
   EAPI Evas_Object *elm_entry_add(Evas_Object *parent);
   EAPI void         elm_entry_single_line_set(Evas_Object *obj, Evas_Bool single_line);
   EAPI void         elm_entry_password_set(Evas_Object *obj, Evas_Bool password);
   EAPI void         elm_entry_entry_set(Evas_Object *obj, const char *entry);
   EAPI const char  *elm_entry_entry_get(Evas_Object *obj);
   EAPI const char  *elm_entry_selection_get(Evas_Object *obj);
   EAPI void         elm_entry_entry_insert(Evas_Object *obj, const char *entry);
   EAPI void         elm_entry_line_wrap_set(Evas_Object *obj, Evas_Bool wrap);
   EAPI void         elm_entry_editable_set(Evas_Object *obj, Evas_Bool editable);
   /* smart callbacks called:
    * "changed" - the text content changed
    * "selection,start" - the user started selecting text
    * "selection,changed" - the user modified the selection size/location
    * "selection,cleared" - the user cleared the selection
    * "selection,paste" - the user rrequested a paste of text
    * "selection,copy" - the user copied the text
    * "selection,cut" - the user cut the text
    * "cursor,changed" - the cursor changed position
    * "anchor,clicked" - achor called was clicked | event_info = Elm_Entry_Anchor_Info
    * "activated" - when the enter key is pressed (useful for single line)
    */
   
   /* composite widgets - these basically put together basic widgets above
    * in convenient packages that do more than basic stuff */

   typedef enum _Elm_Text_Format
     {
	ELM_TEXT_FORMAT_PLAIN_UTF8,
	ELM_TEXT_FORMAT_MARKUP_UTF8
     } Elm_Text_Format;
   EAPI Evas_Object *elm_notepad_add(Evas_Object *parent);
   EAPI void         elm_notepad_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format);
   /* smart callbacks called:
    */

   typedef struct _Elm_Entry_Anchorview_Info Elm_Entry_Anchorview_Info;
   struct _Elm_Entry_Anchorview_Info
     {
	const char *name;
	int button;
	Evas_Object *hover;
	struct {
	   Evas_Coord x, y, w, h;
	} anchor, hover_parent;
	Evas_Bool hover_left : 1;
	Evas_Bool hover_right : 1;
	Evas_Bool hover_top : 1;
	Evas_Bool hover_bottom : 1;
     };
   EAPI Evas_Object *elm_anchorview_add(Evas_Object *parent);
   EAPI void         elm_anchorview_text_set(Evas_Object *obj, const char *text);
   EAPI void         elm_anchorview_hover_style_set(Evas_Object *obj, const char *style);
   EAPI void         elm_anchorview_hover_end(Evas_Object *obj);
   /* smart callbacks called:
    * "anchor,clicked" - achor called was clicked | event_info = Elm_Entry_Anchorview_Info
    */
       
   typedef struct _Elm_Entry_Anchorblock_Info Elm_Entry_Anchorblock_Info;
   struct _Elm_Entry_Anchorblock_Info
     {
	const char *name;
	int button;
	Evas_Object *hover;
	struct {
	   Evas_Coord x, y, w, h;
	} anchor, hover_parent;
	Evas_Bool hover_left : 1;
	Evas_Bool hover_right : 1;
	Evas_Bool hover_top : 1;
	Evas_Bool hover_bottom : 1;
     };
   EAPI Evas_Object *elm_anchorblock_add(Evas_Object *parent);
   EAPI void         elm_anchorblock_text_set(Evas_Object *obj, const char *text);
   EAPI void         elm_anchorblock_hover_style_set(Evas_Object *obj, const char *style);
   EAPI void         elm_anchorblock_hover_end(Evas_Object *obj);
   /* smart callbacks called:
    * "anchor,clicked" - achor called was clicked | event_info = Elm_Entry_Anchorblock_Info
    */
       
   EAPI Evas_Object *elm_bubble_add(Evas_Object *parent);
   EAPI void         elm_bubble_label_set(Evas_Object *obj, const char *label);
   EAPI void         elm_bubble_info_set(Evas_Object *obj, const char *label);
   EAPI void         elm_bubble_content_set(Evas_Object *obj, Evas_Object *content);
   EAPI void         elm_bubble_icon_set(Evas_Object *obj, Evas_Object *icon);
   /* smart callbacks called:
    */
       
// FIXME: fixes to do
// * current sizing tree inefficient
// * scroller could do with page up/down/left/right buttons and and idea of
//   a page size
// * tab widget focusing (not useful for touchscreen tho...)
// * entry has no way to show its focused vs other entries
// * entry selection conflicts with finger scroll
// * toolbar widget (edje + box + button + separators)
// * listitem widget (simple label + icon)
// * radio widget + group handling
// * checkbox widget (like toggle)
// * entry needs to support setting selection to all, or clearing
// * entry click below text doesnt put cursor at end
// * bubble maybe needs icon + label at top?
// * need separator widget (h/v)
// * need slid-open "panel" that can hold stuff and optionally scroll
// * need "photoframe" widget (for contact photos?)
// * need calendar widget (select date)
// * need multi-sel (with button + hover + ...)
// * need slider widget
// * need range selector (select range of values from X to Y over an interval)
// * need a way to filter entry data for entry (eg for phone numbers)
// * entry needs to set vkbd properties/hint (as does win)
// * need to get toplevel
// * need "dialogbutton" widget (bigger button for bottom of wins)
// * need dialog window widget
// * need a phone-number widget (hilight country dial prefixes, add flags, 
//   photos of contacts that match etc.)
// * need imageview widget (for large not iconic images)
// * labels with "" are 0x0 size, but with " " are correct vertically. check
// * tiled image + zoom widget (tiled map viewer)
// * dialpad widget - need one with a phone dialpad
// * scale property from e - watch for changes. also allow for env var option
#endif
