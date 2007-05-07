/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_Atom_Group XCB Atom Functions
 *
 * Functions that operate on atoms.
 */

/*********/
/* Atoms */
/*********/

/* generic atoms */
EAPI Ecore_X_Atom ECORE_X_ATOM_ATOM                     = 0;
EAPI Ecore_X_Atom ECORE_X_ATOM_CARDINAL                 = 0;
EAPI Ecore_X_Atom ECORE_X_ATOM_COMPOUND_TEXT            = 0;
EAPI Ecore_X_Atom ECORE_X_ATOM_FILE_NAME                = 0;
EAPI Ecore_X_Atom ECORE_X_ATOM_STRING                   = 0;
EAPI Ecore_X_Atom ECORE_X_ATOM_TEXT                     = 0;
EAPI Ecore_X_Atom ECORE_X_ATOM_UTF8_STRING              = 0;
EAPI Ecore_X_Atom ECORE_X_ATOM_WINDOW                   = 0;

/* dnd atoms */
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_PROP_XDND     = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_XDND          = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_AWARE              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_ENTER              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_TYPE_LIST          = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_POSITION           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_ACTION_COPY        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_ACTION_MOVE        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_ACTION_PRIVATE     = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_ACTION_ASK         = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_ACTION_LIST        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_ACTION_LINK        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_ACTION_DESCRIPTION = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_PROXY              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_STATUS             = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_LEAVE              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_DROP               = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_XDND_FINISHED           = 0;

/* dnd atoms that need to be exposed to the application interface */
EAPI Ecore_X_Atom  ECORE_X_DND_ACTION_COPY              = 0;
EAPI Ecore_X_Atom  ECORE_X_DND_ACTION_MOVE              = 0;
EAPI Ecore_X_Atom  ECORE_X_DND_ACTION_LINK              = 0;
EAPI Ecore_X_Atom  ECORE_X_DND_ACTION_ASK               = 0;
EAPI Ecore_X_Atom  ECORE_X_DND_ACTION_PRIVATE           = 0;

/* old E atom */
EAPI Ecore_X_Atom  ECORE_X_ATOM_E_FRAME_SIZE            = 0;

/* old Gnome atom */
EAPI Ecore_X_Atom  ECORE_X_ATOM_WIN_LAYER               = 0;

/* ICCCM atoms */

/* ICCCM: client properties */
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_NAME                 = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_ICON_NAME            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_NORMAL_HINTS         = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_SIZE_HINTS           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_HINTS                = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_CLASS                = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_TRANSIENT_FOR        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_PROTOCOLS            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_COLORMAP_WINDOWS     = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_COMMAND              = 0; /* obsolete */
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_CLIENT_MACHINE       = 0; /* obsolete */

/* ICCCM: window manager properties */
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_STATE                = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_ICON_SIZE            = 0;

/* ICCCM: WM_STATE  property */
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_CHANGE_STATE         = 0;

/* ICCCM: WM_PROTOCOLS properties */
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_TAKE_FOCUS           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_SAVE_YOURSELF        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_DELETE_WINDOW        = 0;

/* ICCCM: WM_COLORMAP properties */
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_COLORMAP_NOTIFY      = 0;

/* ICCCM: session management properties */
EAPI Ecore_X_Atom  ECORE_X_ATOM_SM_CLIENT_ID            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_CLIENT_LEADER        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_WM_WINDOW_ROLE          = 0;

/* Motif WM atom */
EAPI Ecore_X_Atom  ECORE_X_ATOM_MOTIF_WM_HINTS          = 0;

/* NetWM 1.3 atoms (http://standards.freedesktop.org/wm-spec/wm-spec-1.3.html) */

/*
 * NetWM: Root Window Properties and related messages (complete)
 */

EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_SUPPORTED                  = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_CLIENT_LIST                = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_CLIENT_LIST_STACKING       = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS         = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_DESKTOP_GEOMETRY           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_DESKTOP_VIEWPORT           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_CURRENT_DESKTOP            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_DESKTOP_NAMES              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_ACTIVE_WINDOW              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WORKAREA                   = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_VIRTUAL_ROOTS              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_DESKTOP_LAYOUT             = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_SHOWING_DESKTOP            = 0;

/*
 * NetWM: Other Root Window Messages (complete)
 */

/* pager */
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_CLOSE_WINDOW               = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_MOVERESIZE_WINDOW          = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_MOVERESIZE              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_RESTACK_WINDOW             = 0;

EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS      = 0;

/*
 * NetWM: Application Window Properties (complete)
 */

EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_NAME                    = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_VISIBLE_NAME            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ICON_NAME               = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME       = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_DESKTOP                 = 0;

/* window type */
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE             = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP     = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR     = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY     = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH      = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG      = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL      = 0;

/* state */
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE                   = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_MODAL             = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_STICKY            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT    = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ    = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_SHADED            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR      = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_HIDDEN            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN        = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_ABOVE             = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_BELOW             = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION = 0;

/* allowed actions */
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS         = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_MOVE             = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_RESIZE           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE         = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_SHADE            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_STICK            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ    = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT    = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN       = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP   = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ACTION_CLOSE            = 0;

EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STRUT                   = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_STRUT_PARTIAL           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ICON_GEOMETRY           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_ICON                    = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_PID                     = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_HANDLED_ICONS           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_USER_TIME               = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_STARTUP_ID                 = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_FRAME_EXTENTS              = 0;

/*
 * NetWM: Window Manager Protocols (complete)
 */

EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_PING                    = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_SYNC_REQUEST            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER    = 0;

/*
 * NetWM: Not in the spec
 */

EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_OPACITY          = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_SHADOW           = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_WM_WINDOW_SHADE            = 0;

/*
 * Startup Notification (http://standards.freedesktop.org/startup-notification-spec/startup-notification-0.1.txt)
 */

EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_STARTUP_INFO_BEGIN         = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_NET_STARTUP_INFO               = 0;

/* selection atoms */
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_TARGETS              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_PRIMARY              = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_SECONDARY            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_CLIPBOARD            = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_PROP_PRIMARY         = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_PROP_SECONDARY       = 0;
EAPI Ecore_X_Atom  ECORE_X_ATOM_SELECTION_PROP_CLIPBOARD       = 0;

/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_atom_init and
   _ecore_xcb_atom_init_finalize. The first one gets the cookies and
   the second one gets the replies and set the atoms. */

#define FETCH_ATOM(s) \
   atom_cookies[i] = xcb_intern_atom(_ecore_xcb_conn, 0, sizeof(s), s); \
   i++

void
_ecore_x_atom_init(xcb_intern_atom_cookie_t *atom_cookies)
{
   int i = 0;

   /* generic atoms */
   FETCH_ATOM("COMPOUND_TEXT");
   FETCH_ATOM("FILE_NAME");
   FETCH_ATOM("TEXT");
   FETCH_ATOM("UTF8_STRING");

   /* dnd atoms */
   FETCH_ATOM("JXSelectionWindowProperty");
   FETCH_ATOM("XdndSelection");
   FETCH_ATOM("XdndAware");
   FETCH_ATOM("XdndEnter");
   FETCH_ATOM("XdndTypeList");
   FETCH_ATOM("XdndPosition");
   FETCH_ATOM("XdndActionCopy");
   FETCH_ATOM("XdndActionMove");
   FETCH_ATOM("XdndActionPrivate");
   FETCH_ATOM("XdndActionAsk");
   FETCH_ATOM("XdndActionList");
   FETCH_ATOM("XdndActionLink");
   FETCH_ATOM("XdndActionDescription");
   FETCH_ATOM("XdndProxy");
   FETCH_ATOM("XdndStatus");
   FETCH_ATOM("XdndLeave");
   FETCH_ATOM("XdndDrop");
   FETCH_ATOM("XdndFinished");

   /* old E atom */
   FETCH_ATOM("_E_FRAME_SIZE");

   /* old Gnome atom */
   FETCH_ATOM("_WIN_LAYER");

   /* ICCCM */
   FETCH_ATOM("WM_PROTOCOLS");
   FETCH_ATOM("WM_COLORMAP_WINDOWS");

   FETCH_ATOM("WM_STATE");

   FETCH_ATOM("WM_CHANGE_STATE");

   FETCH_ATOM("WM_TAKE_FOCUS");
   FETCH_ATOM("WM_SAVE_YOURSELF");
   FETCH_ATOM("WM_DELETE_WINDOW");

   FETCH_ATOM("WM_COLORMAP_NOTIFY");

   FETCH_ATOM("SM_CLIENT_ID");
   FETCH_ATOM("WM_CLIENT_LEADER");
   FETCH_ATOM("WM_WINDOW_ROLE");

   /* Motif WM atom */
   FETCH_ATOM("_MOTIF_WM_HINTS");

   /* NetWM atoms */
   FETCH_ATOM("_NET_SUPPORTED");
   FETCH_ATOM("_NET_CLIENT_LIST");
   FETCH_ATOM("_NET_CLIENT_LIST_STACKING");
   FETCH_ATOM("_NET_NUMBER_OF_DESKTOPS");
   FETCH_ATOM("_NET_DESKTOP_GEOMETRY");
   FETCH_ATOM("_NET_DESKTOP_VIEWPORT");
   FETCH_ATOM("_NET_CURRENT_DESKTOP");
   FETCH_ATOM("_NET_DESKTOP_NAMES");
   FETCH_ATOM("_NET_ACTIVE_WINDOW");
   FETCH_ATOM("_NET_WORKAREA");
   FETCH_ATOM("_NET_SUPPORTING_WM_CHECK");
   FETCH_ATOM("_NET_VIRTUAL_ROOTS");
   FETCH_ATOM("_NET_DESKTOP_LAYOUT");
   FETCH_ATOM("_NET_SHOWING_DESKTOP");

   FETCH_ATOM("_NET_CLOSE_WINDOW");
   FETCH_ATOM("_NET_MOVERESIZE_WINDOW");
   FETCH_ATOM("_NET_WM_MOVERESIZE");
   FETCH_ATOM("_NET_RESTACK_WINDOW");
   FETCH_ATOM("_NET_REQUEST_FRAME_EXTENTS");

   FETCH_ATOM("_NET_WM_NAME");
   FETCH_ATOM("_NET_WM_VISIBLE_NAME");
   FETCH_ATOM("_NET_WM_ICON_NAME");
   FETCH_ATOM("_NET_WM_VISIBLE_ICON_NAME");
   FETCH_ATOM("_NET_WM_DESKTOP");

   FETCH_ATOM("_NET_WM_WINDOW_TYPE");
   FETCH_ATOM("_NET_WM_WINDOW_TYPE_DESKTOP");
   FETCH_ATOM("_NET_WM_WINDOW_TYPE_DOCK");
   FETCH_ATOM("_NET_WM_WINDOW_TYPE_TOOLBAR");
   FETCH_ATOM("_NET_WM_WINDOW_TYPE_MENU");
   FETCH_ATOM("_NET_WM_WINDOW_TYPE_UTILITY");
   FETCH_ATOM("_NET_WM_WINDOW_TYPE_SPLASH");
   FETCH_ATOM("_NET_WM_WINDOW_TYPE_DIALOG");
   FETCH_ATOM("_NET_WM_WINDOW_TYPE_NORMAL");

   FETCH_ATOM("_NET_WM_STATE");
   FETCH_ATOM("_NET_WM_STATE_MODAL");
   FETCH_ATOM("_NET_WM_STATE_STICKY");
   FETCH_ATOM("_NET_WM_STATE_MAXIMIZED_VERT");
   FETCH_ATOM("_NET_WM_STATE_MAXIMIZED_HORZ");
   FETCH_ATOM("_NET_WM_STATE_SHADED");
   FETCH_ATOM("_NET_WM_STATE_SKIP_TASKBAR");
   FETCH_ATOM("_NET_WM_STATE_SKIP_PAGER");
   FETCH_ATOM("_NET_WM_STATE_HIDDEN");
   FETCH_ATOM("_NET_WM_STATE_FULLSCREEN");
   FETCH_ATOM("_NET_WM_STATE_ABOVE");
   FETCH_ATOM("_NET_WM_STATE_BELOW");
   FETCH_ATOM("_NET_WM_STATE_DEMANDS_ATTENTION");

   FETCH_ATOM("_NET_WM_ALLOWED_ACTIONS");
   FETCH_ATOM("_NET_WM_ACTION_MOVE");
   FETCH_ATOM("_NET_WM_ACTION_RESIZE");
   FETCH_ATOM("_NET_WM_ACTION_MINIMIZE");
   FETCH_ATOM("_NET_WM_ACTION_SHADE");
   FETCH_ATOM("_NET_WM_ACTION_STICK");
   FETCH_ATOM("_NET_WM_ACTION_MAXIMIZE_HORZ");
   FETCH_ATOM("_NET_WM_ACTION_MAXIMIZE_VERT");
   FETCH_ATOM("_NET_WM_ACTION_FULLSCREEN");
   FETCH_ATOM("_NET_WM_ACTION_CHANGE_DESKTOP");
   FETCH_ATOM("_NET_WM_ACTION_CLOSE");

   FETCH_ATOM("_NET_WM_STRUT");
   FETCH_ATOM("_NET_WM_STRUT_PARTIAL");
   FETCH_ATOM("_NET_WM_ICON_GEOMETRY");
   FETCH_ATOM("_NET_WM_ICON");
   FETCH_ATOM("_NET_WM_PID");
   FETCH_ATOM("_NET_WM_HANDLED_ICONS");
   FETCH_ATOM("_NET_WM_USER_TIME");
   FETCH_ATOM("_NET_STARTUP_ID");
   FETCH_ATOM("_NET_FRAME_EXTENTS");

   FETCH_ATOM("_NET_WM_PING");
   FETCH_ATOM("_NET_WM_SYNC_REQUEST");
   FETCH_ATOM("_NET_WM_SYNC_REQUEST_COUNTER");

   FETCH_ATOM("_NET_WM_WINDOW_OPACITY");
   FETCH_ATOM("_NET_WM_WINDOW_SHADOW");
   FETCH_ATOM("_NET_WM_WINDOW_SHADE");

   FETCH_ATOM("_NET_STARTUP_INFO_BEGIN");
   FETCH_ATOM("_NET_STARTUP_INFO");

   /* selection atoms */
   FETCH_ATOM("TARGETS");
   FETCH_ATOM("CLIPBOARD");
   FETCH_ATOM("_ECORE_SELECTION_PRIMARY");
   FETCH_ATOM("_ECORE_SELECTION_SECONDARY");
   FETCH_ATOM("_ECORE_SELECTION_CLIPBOARD");

   /* These atoms are already internally defined */
   ECORE_X_ATOM_SELECTION_PRIMARY   = 1;
   ECORE_X_ATOM_SELECTION_SECONDARY = 2;
   ECORE_X_ATOM_ATOM                = 4;
   ECORE_X_ATOM_CARDINAL            = 6;
   ECORE_X_ATOM_STRING              = 31;
   ECORE_X_ATOM_WINDOW              = 33;
   ECORE_X_ATOM_WM_NAME             = 39;
   ECORE_X_ATOM_WM_ICON_NAME        = 37;
   ECORE_X_ATOM_WM_NORMAL_HINTS     = 40;
   ECORE_X_ATOM_WM_SIZE_HINTS       = 41;
   ECORE_X_ATOM_WM_HINTS            = 35;
   ECORE_X_ATOM_WM_CLASS            = 67;
   ECORE_X_ATOM_WM_TRANSIENT_FOR    = 68;
   ECORE_X_ATOM_WM_COMMAND          = 34;
   ECORE_X_ATOM_WM_CLIENT_MACHINE   = 36;
   ECORE_X_ATOM_WM_ICON_SIZE        = 38;

   /* Initialize the globally defined xdnd atoms */
   ECORE_X_DND_ACTION_COPY          = ECORE_X_ATOM_XDND_ACTION_COPY;
   ECORE_X_DND_ACTION_MOVE          = ECORE_X_ATOM_XDND_ACTION_MOVE;
   ECORE_X_DND_ACTION_LINK          = ECORE_X_ATOM_XDND_ACTION_LINK;
   ECORE_X_DND_ACTION_ASK           = ECORE_X_ATOM_XDND_ACTION_ASK;
   ECORE_X_DND_ACTION_PRIVATE       = ECORE_X_ATOM_XDND_ACTION_PRIVATE;
}

void
_ecore_x_atom_init_finalize(xcb_intern_atom_cookie_t *atom_cookies)
{
   xcb_intern_atom_reply_t *replies[ECORE_X_ATOMS_COUNT];
   int                      i;

   for (i = 0; i < ECORE_X_ATOMS_COUNT; i++) {
      xcb_generic_error_t *error = NULL;

      replies[i] = xcb_intern_atom_reply(_ecore_xcb_conn,
                                         atom_cookies[i],
                                         &error);
      if (!replies[i]) {
        printf ("pas de reply %d\n", i);
        if (error)
          printf ("Error !\n");
      }
   }

   ECORE_X_ATOM_COMPOUND_TEXT                  = replies[0]->atom;
   ECORE_X_ATOM_FILE_NAME                      = replies[1]->atom;
   ECORE_X_ATOM_TEXT                           = replies[2]->atom;
   ECORE_X_ATOM_UTF8_STRING                    = replies[3]->atom;
   ECORE_X_ATOM_SELECTION_PROP_XDND            = replies[4]->atom;
   ECORE_X_ATOM_SELECTION_XDND                 = replies[5]->atom;
   ECORE_X_ATOM_XDND_AWARE                     = replies[6]->atom;
   ECORE_X_ATOM_XDND_ENTER                     = replies[7]->atom;
   ECORE_X_ATOM_XDND_TYPE_LIST                 = replies[8]->atom;
   ECORE_X_ATOM_XDND_POSITION                  = replies[9]->atom;
   ECORE_X_ATOM_XDND_ACTION_COPY               = replies[10]->atom;
   ECORE_X_ATOM_XDND_ACTION_MOVE               = replies[11]->atom;
   ECORE_X_ATOM_XDND_ACTION_PRIVATE            = replies[12]->atom;
   ECORE_X_ATOM_XDND_ACTION_ASK                = replies[13]->atom;
   ECORE_X_ATOM_XDND_ACTION_LIST               = replies[14]->atom;
   ECORE_X_ATOM_XDND_ACTION_LINK               = replies[15]->atom;
   ECORE_X_ATOM_XDND_ACTION_DESCRIPTION        = replies[16]->atom;
   ECORE_X_ATOM_XDND_PROXY                     = replies[17]->atom;
   ECORE_X_ATOM_XDND_STATUS                    = replies[18]->atom;
   ECORE_X_ATOM_XDND_LEAVE                     = replies[19]->atom;
   ECORE_X_ATOM_XDND_DROP                      = replies[20]->atom;
   ECORE_X_ATOM_XDND_FINISHED                  = replies[21]->atom;
   ECORE_X_ATOM_E_FRAME_SIZE                   = replies[22]->atom;
   ECORE_X_ATOM_WIN_LAYER                      = replies[23]->atom;
   ECORE_X_ATOM_WM_PROTOCOLS                   = replies[24]->atom;
   ECORE_X_ATOM_WM_COLORMAP_WINDOWS            = replies[25]->atom;
   ECORE_X_ATOM_WM_STATE                       = replies[26]->atom;
   ECORE_X_ATOM_WM_CHANGE_STATE                = replies[27]->atom;
   ECORE_X_ATOM_WM_TAKE_FOCUS                  = replies[28]->atom;
   ECORE_X_ATOM_WM_SAVE_YOURSELF               = replies[29]->atom;
   ECORE_X_ATOM_WM_DELETE_WINDOW               = replies[30]->atom;
   ECORE_X_ATOM_WM_COLORMAP_NOTIFY             = replies[31]->atom;
   ECORE_X_ATOM_SM_CLIENT_ID                   = replies[32]->atom;
   ECORE_X_ATOM_WM_CLIENT_LEADER               = replies[33]->atom;
   ECORE_X_ATOM_WM_WINDOW_ROLE                 = replies[34]->atom;
   ECORE_X_ATOM_MOTIF_WM_HINTS                 = replies[35]->atom;
   ECORE_X_ATOM_NET_SUPPORTED                  = replies[36]->atom;
   ECORE_X_ATOM_NET_CLIENT_LIST                = replies[37]->atom;
   ECORE_X_ATOM_NET_CLIENT_LIST_STACKING       = replies[38]->atom;
   ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS         = replies[39]->atom;
   ECORE_X_ATOM_NET_DESKTOP_GEOMETRY           = replies[40]->atom;
   ECORE_X_ATOM_NET_DESKTOP_VIEWPORT           = replies[41]->atom;
   ECORE_X_ATOM_NET_CURRENT_DESKTOP            = replies[42]->atom;
   ECORE_X_ATOM_NET_DESKTOP_NAMES              = replies[43]->atom;
   ECORE_X_ATOM_NET_ACTIVE_WINDOW              = replies[44]->atom;
   ECORE_X_ATOM_NET_WORKAREA                   = replies[45]->atom;
   ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK        = replies[46]->atom;
   ECORE_X_ATOM_NET_VIRTUAL_ROOTS              = replies[47]->atom;
   ECORE_X_ATOM_NET_DESKTOP_LAYOUT             = replies[48]->atom;
   ECORE_X_ATOM_NET_SHOWING_DESKTOP            = replies[49]->atom;
   ECORE_X_ATOM_NET_CLOSE_WINDOW               = replies[50]->atom;
   ECORE_X_ATOM_NET_MOVERESIZE_WINDOW          = replies[51]->atom;
   ECORE_X_ATOM_NET_WM_MOVERESIZE              = replies[52]->atom;
   ECORE_X_ATOM_NET_RESTACK_WINDOW             = replies[53]->atom;
   ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS      = replies[54]->atom;
   ECORE_X_ATOM_NET_WM_NAME                    = replies[55]->atom;
   ECORE_X_ATOM_NET_WM_VISIBLE_NAME            = replies[56]->atom;
   ECORE_X_ATOM_NET_WM_ICON_NAME               = replies[57]->atom;
   ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME       = replies[58]->atom;
   ECORE_X_ATOM_NET_WM_DESKTOP                 = replies[59]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE             = replies[60]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP     = replies[61]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK        = replies[62]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR     = replies[63]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU        = replies[64]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY     = replies[65]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH      = replies[66]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG      = replies[67]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL      = replies[68]->atom;
   ECORE_X_ATOM_NET_WM_STATE                   = replies[69]->atom;
   ECORE_X_ATOM_NET_WM_STATE_MODAL             = replies[70]->atom;
   ECORE_X_ATOM_NET_WM_STATE_STICKY            = replies[71]->atom;
   ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT    = replies[72]->atom;
   ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ    = replies[73]->atom;
   ECORE_X_ATOM_NET_WM_STATE_SHADED            = replies[74]->atom;
   ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR      = replies[75]->atom;
   ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER        = replies[76]->atom;
   ECORE_X_ATOM_NET_WM_STATE_HIDDEN            = replies[77]->atom;
   ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN        = replies[78]->atom;
   ECORE_X_ATOM_NET_WM_STATE_ABOVE             = replies[79]->atom;
   ECORE_X_ATOM_NET_WM_STATE_BELOW             = replies[80]->atom;
   ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION = replies[81]->atom;
   ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS         = replies[82]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_MOVE             = replies[83]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_RESIZE           = replies[84]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE         = replies[85]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_SHADE            = replies[86]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_STICK            = replies[87]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ    = replies[88]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT    = replies[89]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN       = replies[90]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP   = replies[91]->atom;
   ECORE_X_ATOM_NET_WM_ACTION_CLOSE            = replies[92]->atom;
   ECORE_X_ATOM_NET_WM_STRUT                   = replies[93]->atom;
   ECORE_X_ATOM_NET_WM_STRUT_PARTIAL           = replies[94]->atom;
   ECORE_X_ATOM_NET_WM_ICON_GEOMETRY           = replies[95]->atom;
   ECORE_X_ATOM_NET_WM_ICON                    = replies[96]->atom;
   ECORE_X_ATOM_NET_WM_PID                     = replies[97]->atom;
   ECORE_X_ATOM_NET_WM_HANDLED_ICONS           = replies[98]->atom;
   ECORE_X_ATOM_NET_WM_USER_TIME               = replies[99]->atom;
   ECORE_X_ATOM_NET_STARTUP_ID                 = replies[100]->atom;
   ECORE_X_ATOM_NET_FRAME_EXTENTS              = replies[101]->atom;
   ECORE_X_ATOM_NET_WM_PING                    = replies[102]->atom;
   ECORE_X_ATOM_NET_WM_SYNC_REQUEST            = replies[103]->atom;
   ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER    = replies[104]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_OPACITY          = replies[105]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_SHADOW           = replies[106]->atom;
   ECORE_X_ATOM_NET_WM_WINDOW_SHADE            = replies[107]->atom;
   ECORE_X_ATOM_NET_STARTUP_INFO_BEGIN         = replies[108]->atom;
   ECORE_X_ATOM_NET_STARTUP_INFO               = replies[109]->atom;
   ECORE_X_ATOM_SELECTION_TARGETS              = replies[110]->atom;
   ECORE_X_ATOM_SELECTION_CLIPBOARD            = replies[111]->atom;
   ECORE_X_ATOM_SELECTION_PROP_PRIMARY         = replies[112]->atom;
   ECORE_X_ATOM_SELECTION_PROP_SECONDARY       = replies[113]->atom;
   ECORE_X_ATOM_SELECTION_PROP_CLIPBOARD       = replies[114]->atom;

   for (i = 0; i < ECORE_X_ATOMS_COUNT; i++)
      free(replies[i]);
}


/**
 * Sends the InternAtom request.
 * @param name Name of the requested atom.
 * @ingroup Ecore_X_Atom_Group
 */
EAPI void
ecore_x_atom_get_prefetch(const char *name)
{
   xcb_intern_atom_cookie_t cookie;

   cookie = xcb_intern_atom_unchecked(_ecore_xcb_conn, 0, strlen(name), name);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the InternAtom request sent by ecore_x_atom_get_prefetch().
 * @ingroup Ecore_X_Atom_Group
 */
EAPI void
ecore_x_atom_get_fetch(void)
{
   xcb_intern_atom_cookie_t cookie;
   xcb_intern_atom_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Retrieves the atom value associated to a name.
 * @param  name Unused.
 * @return      Associated atom value.
 *
 * Retrieves the atom value associated to a name. The reply is the
 * returned value of the function ecore_xcb_intern_atom_reply(). If
 * @p reply is @c NULL, the NULL atom is returned. Otherwise, the atom
 * associated to the name is returned.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_atom_get_prefetch(), which sends the InternAtom request,
 * then ecore_x_atom_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_Atom_Group
 */
EAPI Ecore_X_Atom
ecore_x_atom_get(const char *name __UNUSED__)
{
   xcb_intern_atom_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return XCB_NONE;

   return reply->atom;
}
