/**
 * @defgroup Web Web
 * @ingroup Elementary
 *
 * @image html web_inheritance_tree.png
 * @image latex web_inheritance_tree.eps
 *
 * @image html img/widget/web/preview-00.png
 * @image latex img/widget/web/preview-00.eps
 *
 * A web widget is used for displaying web pages (HTML/CSS/JS)
 * using WebKit-EFL. You must have compiled Elementary with
 * ewebkit support.
 *
 * Signals that you can add callbacks for are:
 * @li "download,request": A file download has been requested. Event info is
 * a pointer to a Elm_Web_Download
 * @li "editorclient,contents,changed": Editor client's contents changed
 * @li "editorclient,selection,changed": Editor client's selection changed
 * @li "frame,created": A new frame was created. Event info is an
 * Evas_Object which can be handled with WebKit's ewk_frame API
 * @li "icon,received": An icon was received by the main frame
 * @li "inputmethod,changed": Input method changed. Event info is an
 * Eina_Bool indicating whether it's enabled or not
 * @li "js,windowobject,clear": JS window object has been cleared
 * @li "link,hover,in": Mouse cursor is hovering over a link. Event info
 * is a char *link[2], where the first string contains the URL the link
 * points to, and the second one the title of the link
 * @li "link,hover,out": Mouse cursor left the link
 * @li "load,document,finished": Loading of a document finished. Event info
 * is the frame that finished loading
 * @li "load,error": Load failed. Event info is a pointer to
 * Elm_Web_Frame_Load_Error
 * @li "load,finished": Load finished. Event info is NULL on success, on
 * error it's a pointer to Elm_Web_Frame_Load_Error
 * @li "load,newwindow,show": A new window was created and is ready to be
 * shown
 * @li "load,progress": Overall load progress. Event info is a pointer to
 * a double containing a value between 0.0 and 1.0
 * @li "load,provisional": Started provisional load
 * @li "load,started": Loading of a document started
 * @li "menubar,visible,get": Queries if the menubar is visible. Event info
 * is a pointer to Eina_Bool where the callback should set EINA_TRUE if
 * the menubar is visible, or EINA_FALSE in case it's not
 * @li "menubar,visible,set": Informs menubar visibility. Event info is
 * an Eina_Bool indicating the visibility
 * @li "popup,created": A dropdown widget was activated, requesting its
 * popup menu to be created. Event info is a pointer to Elm_Web_Menu
 * @li "popup,willdelete": The web object is ready to destroy the popup
 * object created. Event info is a pointer to Elm_Web_Menu
 * @li "ready": Page is fully loaded
 * @li "scrollbars,visible,get": Queries visibility of scrollbars. Event
 * info is a pointer to Eina_Bool where the visibility state should be set
 * @li "scrollbars,visible,set": Informs scrollbars visibility. Event info
 * is an Eina_Bool with the visibility state set
 * @li "statusbar,text,set": Text of the statusbar changed. Even info is
 * a string with the new text
 * @li "statusbar,visible,get": Queries visibility of the status bar.
 * Event info is a pointer to Eina_Bool where the visibility state should be
 * set.
 * @li "statusbar,visible,set": Informs statusbar visibility. Event info is
 * an Eina_Bool with the visibility value
 * @li "title,changed": Title of the main frame changed. Event info is a
 * string with the new title
 * @li "toolbars,visible,get": Queries visibility of toolbars. Event info
 * is a pointer to Eina_Bool where the visibility state should be set
 * @li "toolbars,visible,set": Informs the visibility of toolbars. Event
 * info is an Eina_Bool with the visibility state
 * @li "tooltip,text,set": Show and set text of a tooltip. Event info is
 * a string with the text to show
 * @li "uri,changed": URI of the main frame changed. Event info is a string (deprecated. use "url,changed" instead)
 * @li "url,changed": URL of the main frame changed. Event info is a string
 * with the new URI
 * @li "view,resized": The web object internal's view changed sized
 * @li "windows,close,request": A JavaScript request to close the current
 * window was requested
 * @li "zoom,animated,end": Animated zoom finished
 * @li "focused" : When the web has received focus. (since 1.8)
 * @li "unfocused" : When the web has lost focus. (since 1.8)
 *
 * available styles:
 * - default
 *
 * An example of use of web:
 *
 * - @ref web_example_01
 * - @ref web_example_02
 */

/**
 * @addtogroup Web
 * @{
 */

#include "elm_web_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_web_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_web_legacy.h"
#endif
/**
 * @}
 */
