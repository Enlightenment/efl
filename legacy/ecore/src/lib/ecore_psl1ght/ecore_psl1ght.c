#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sysutil/video.h>
#include <sysutil/sysutil.h>
#include <sysmodule/sysmodule.h>
#include <io/pad.h>
#include <io/mouse.h>
#include <io/kb.h>
#include <io/camera.h>
#include <io/move.h>
#include <sys/process.h>

#include "moveutil.h"

#include "Eina.h"
#include "Ecore_Psl1ght.h"
#include "Ecore_Input.h"
#include "Ecore.h"
#include "ecore_psl1ght_private.h"
#include "ecore_private.h"
#include "Ecore_Psl1ght_Keys.h"

/* Allocate 1MB stack to avoid overflows */
SYS_PROCESS_PARAM(1001, 0x100000);

int _ecore_psl1ght_log_dom = -1;

EAPI int ECORE_PSL1GHT_EVENT_KEY_MODIFIERS = 0;
EAPI int ECORE_PSL1GHT_EVENT_GOT_FOCUS = 0;
EAPI int ECORE_PSL1GHT_EVENT_LOST_FOCUS = 0;
EAPI int ECORE_PSL1GHT_EVENT_EXPOSE = 0;

static int _ecore_psl1ght_init_count = 0;
static int window_width = 0;
static int window_height = 0;
/* Mouse support */
static int mouse_connected = FALSE;
static u8 mouse_buttons = 0;
static int mouse_x = 0;
static int mouse_y = 0;
/* Keyboard support */
static int keyboard_connected = FALSE;
static KbLed keyboard_leds = {{0}};
static KbMkey keyboard_mods = {{0}};
static u16 keyboard_old_key = 0;
/* Pad support */
static padData pad_data;
static int pad_old_x = 0;
static int pad_old_o = 0;
/* Move support */
static int move_connected = FALSE;
static moveContext *move_context = NULL;
u16 move_buttons = 0;

static void xmb_event_handler(u64 status, u64 param, void *user_data);

/**
 * @defgroup Ecore_Psl1ght_Library_Group PSL1GHT Library Functions
 *
 * Functions used to set up and shut down the Ecore_Psl1ght functions.
 */

/**
 * Sets up the Ecore_Psl1ght library.
 * @param   name device target name
 * @return  @c 0 on failure.  Otherwise, the number of times the library has
 *          been initialised without being shut down.
 * @ingroup Ecore_PSL1GHT_Library_Group
 */
EAPI int
ecore_psl1ght_init(const char *name __UNUSED__)
{
   videoState state;
   videoResolution resolution;
   int ret, camera_loaded, gem_loaded;

   if (++_ecore_psl1ght_init_count != 1)
     return _ecore_psl1ght_init_count;
   _ecore_psl1ght_log_dom = eina_log_domain_register
       ("ecore_psl1ght", ECORE_PSL1GHT_DEFAULT_LOG_COLOR);
   if (_ecore_psl1ght_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for the Ecore PSL1GHT module.");
        return --_ecore_psl1ght_init_count;
     }
   if (!ecore_event_init())
     {
        eina_log_domain_unregister(_ecore_psl1ght_log_dom);
        _ecore_psl1ght_log_dom = -1;
        return --_ecore_psl1ght_init_count;
     }

   if (videoGetState (0, 0, &state) == 0 &&
       videoGetResolution (state.displayMode.resolution, &resolution) == 0)
     {
        ecore_psl1ght_resolution_set (resolution.width, resolution.height);
     }
   else
     {
        ecore_event_shutdown();
        eina_log_domain_unregister(_ecore_psl1ght_log_dom);
        _ecore_psl1ght_log_dom = -1;
        return --_ecore_psl1ght_init_count;
     }

   /* Pad support */
   ioPadInit (7);
   /* Mouse support */
   ioMouseInit(2);
   mouse_buttons = 0;
   mouse_x = 0;
   mouse_y = 0;

   /* Keyboard support */
   ioKbInit(2);
   keyboard_leds._KbLedU.leds = 0;
   keyboard_mods._KbMkeyU.mkeys = 0;

   /* Initialize Move */
   move_context = NULL;
   move_buttons = 0;

   camera_loaded = !sysModuleIsLoaded (SYSMODULE_CAMERA);
   if (!camera_loaded)
     ret = sysModuleLoad (SYSMODULE_CAMERA);
   else
     ret = 0;
   if (ret == 0)
     {
        gem_loaded = !sysModuleIsLoaded (SYSMODULE_GEM);
        if (!gem_loaded)
          ret = sysModuleLoad (SYSMODULE_GEM);
        if (ret == 0)
          {
             move_context = initMove ();
          }
        else {
             if (gem_loaded)
               sysModuleUnload (SYSMODULE_CAMERA);
          }
     }

   sysUtilRegisterCallback (SYSUTIL_EVENT_SLOT0, xmb_event_handler, NULL);

   ECORE_PSL1GHT_EVENT_GOT_FOCUS = ecore_event_type_new();
   ECORE_PSL1GHT_EVENT_LOST_FOCUS = ecore_event_type_new();
   ECORE_PSL1GHT_EVENT_EXPOSE = ecore_event_type_new();
   ECORE_PSL1GHT_EVENT_KEY_MODIFIERS = ecore_event_type_new();

   mouse_x = 0;
   mouse_y = 0;

   return _ecore_psl1ght_init_count;
}

/**
 * Shuts down the Ecore_Psl1ght library.
 * @return  @c The number of times the system has been initialised without
 *             being shut down.
 * @ingroup Ecore_PSL1GHT_Library_Group
 */
EAPI int
ecore_psl1ght_shutdown(void)
{
   if (--_ecore_psl1ght_init_count != 0)
     return _ecore_psl1ght_init_count;

   ecore_event_shutdown();
   eina_log_domain_unregister(_ecore_psl1ght_log_dom);
   _ecore_psl1ght_log_dom = -1;

   ECORE_PSL1GHT_EVENT_GOT_FOCUS = 0;
   ECORE_PSL1GHT_EVENT_LOST_FOCUS = 0;
   ECORE_PSL1GHT_EVENT_EXPOSE = 0;
   ECORE_PSL1GHT_EVENT_KEY_MODIFIERS = 0;

   ioPadEnd();
   ioMouseEnd();
   ioKbEnd();

   if (move_context)
     {
        endMove (move_context);
        move_context = NULL;
        sysModuleUnload (SYSMODULE_CAMERA);
        sysModuleUnload (SYSMODULE_GEM);
     }

   sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);

   return _ecore_psl1ght_init_count;
}

static unsigned int
_ecore_psl1ght_get_time(void)
{
   return (unsigned int)((unsigned long long)
                         (ecore_time_get() * 1000.0) & 0xffffffff);
}

static unsigned int
_ecore_psl1ght_get_modifiers(void)
{
   unsigned int modifiers = 0;

   if (keyboard_mods._KbMkeyU._KbMkeyS.r_shift ||
       keyboard_mods._KbMkeyU._KbMkeyS.l_shift)
     modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (keyboard_mods._KbMkeyU._KbMkeyS.r_ctrl ||
       keyboard_mods._KbMkeyU._KbMkeyS.l_ctrl)
     modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (keyboard_mods._KbMkeyU._KbMkeyS.r_alt ||
       keyboard_mods._KbMkeyU._KbMkeyS.l_alt)
     modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if (keyboard_mods._KbMkeyU._KbMkeyS.r_win ||
       keyboard_mods._KbMkeyU._KbMkeyS.l_win)
     modifiers |= ECORE_EVENT_MODIFIER_WIN;

   if (keyboard_leds._KbLedU._KbLedS.num_lock)
     modifiers |= ECORE_EVENT_LOCK_NUM;
   if (keyboard_leds._KbLedU._KbLedS.caps_lock)
     modifiers |= ECORE_EVENT_LOCK_CAPS;
   if (keyboard_leds._KbLedU._KbLedS.scroll_lock)
     modifiers |= ECORE_EVENT_LOCK_SCROLL;

   return modifiers;
}

static void
_ecore_psl1ght_key_modifiers(KbMkey *mods, KbLed *leds)
{
   Ecore_Psl1ght_Event_Key_Modifiers *ev;
   Eina_Bool emit = EINA_FALSE;

   ev = malloc(sizeof(Ecore_Psl1ght_Event_Key_Modifiers));
   if (!ev) return;

   if (mods->_KbMkeyU._KbMkeyS.l_shift !=
       keyboard_mods._KbMkeyU._KbMkeyS.l_shift ||
       mods->_KbMkeyU._KbMkeyS.r_shift !=
       keyboard_mods._KbMkeyU._KbMkeyS.r_shift)
     {
        emit = EINA_TRUE;
        ev->shift_changed = EINA_TRUE;
        ev->shift = mods->_KbMkeyU._KbMkeyS.r_shift |
          mods->_KbMkeyU._KbMkeyS.l_shift;
     }
   if (mods->_KbMkeyU._KbMkeyS.l_ctrl !=
       keyboard_mods._KbMkeyU._KbMkeyS.l_ctrl ||
       mods->_KbMkeyU._KbMkeyS.r_ctrl !=
       keyboard_mods._KbMkeyU._KbMkeyS.r_ctrl)
     {
        emit = EINA_TRUE;
        ev->ctrl_changed = EINA_TRUE;
        ev->ctrl = mods->_KbMkeyU._KbMkeyS.r_ctrl |
          mods->_KbMkeyU._KbMkeyS.l_ctrl;
     }
   if (mods->_KbMkeyU._KbMkeyS.l_alt !=
       keyboard_mods._KbMkeyU._KbMkeyS.l_alt ||
       mods->_KbMkeyU._KbMkeyS.r_alt !=
       keyboard_mods._KbMkeyU._KbMkeyS.r_alt)
     {
        emit = EINA_TRUE;
        ev->alt_changed = EINA_TRUE;
        ev->alt = mods->_KbMkeyU._KbMkeyS.r_alt |
          mods->_KbMkeyU._KbMkeyS.l_alt;
     }
   if (mods->_KbMkeyU._KbMkeyS.l_win !=
       keyboard_mods._KbMkeyU._KbMkeyS.l_win ||
       mods->_KbMkeyU._KbMkeyS.r_win !=
       keyboard_mods._KbMkeyU._KbMkeyS.r_win)
     {
        emit = EINA_TRUE;
        ev->win_changed = EINA_TRUE;
        ev->win = mods->_KbMkeyU._KbMkeyS.r_win |
          mods->_KbMkeyU._KbMkeyS.l_win;
     }
   keyboard_mods = *mods;

   if (leds->_KbLedU._KbLedS.num_lock !=
       keyboard_leds._KbLedU._KbLedS.num_lock)
     {
        emit = EINA_TRUE;
        ev->num_lock_changed = EINA_TRUE;
        ev->num_lock = leds->_KbLedU._KbLedS.num_lock;
     }
   if (leds->_KbLedU._KbLedS.caps_lock !=
       keyboard_leds._KbLedU._KbLedS.caps_lock)
     {
        emit = EINA_TRUE;
        ev->caps_lock_changed = EINA_TRUE;
        ev->caps_lock = leds->_KbLedU._KbLedS.caps_lock;
     }
   if (leds->_KbLedU._KbLedS.scroll_lock !=
       keyboard_leds._KbLedU._KbLedS.scroll_lock)
     {
        emit = EINA_TRUE;
        ev->scroll_lock_changed = EINA_TRUE;
        ev->scroll_lock = leds->_KbLedU._KbLedS.scroll_lock;
     }
   keyboard_leds = *leds;

   if (emit)
     {
        ev->timestamp = _ecore_psl1ght_get_time ();
        ev->modifiers = _ecore_psl1ght_get_modifiers();
        ecore_event_add(ECORE_PSL1GHT_EVENT_KEY_MODIFIERS, ev, NULL, NULL);
     }
   else
     {
        free(ev);
     }
}

static void
unicodeToUtf8(u16 w, char *utf8buf)
{
   unsigned char *utf8s = (unsigned char *)utf8buf;

   if ( w < 0x0080 )
     {
        utf8s[0] = ( unsigned char )w;
        utf8s[1] = 0;
     }
   else if ( w < 0x0800 )
     {
        utf8s[0] = 0xc0 | ((w) >> 6);
        utf8s[1] = 0x80 | ((w) & 0x3f);
        utf8s[2] = 0;
     }
   else {
        utf8s[0] = 0xe0 | ((w) >> 12);
        utf8s[1] = 0x80 | (((w) >> 6) & 0x3f);
        utf8s[2] = 0x80 | ((w) & 0x3f);
        utf8s[3] = 0;
     }
}

static Ecore_Event_Key *
_ecore_psl1ght_event_key(u16 key)
{
   Ecore_Event_Key *ev;
   char utf8[4];
   u16 utf16;
   unsigned int i;

   ev = malloc(sizeof(Ecore_Event_Key));
   if (!ev) return NULL;

   ev->timestamp = _ecore_psl1ght_get_time ();
   ev->window = 0;
   ev->event_window = 0;
   ev->modifiers = _ecore_psl1ght_get_modifiers();

   printf ("Key is %X\n", key);
   key &= ~KB_KEYPAD;
   for (i = 0; i < sizeof(keystable) / sizeof(struct _ecore_psl1ght_keys_s); ++i)
     if (keystable[i].code == key)
       {
          ev->keyname = keystable[i].name;
          ev->key = keystable[i].name;
          ev->string = keystable[i].compose;
          ev->compose = keystable[i].compose;

          printf ("Found key '%s' in the table\n", ev->keyname);
          return ev;
       }

   utf16 = ioKbCnvRawCode (KB_MAPPING_101, keyboard_mods, keyboard_leds, key);
   unicodeToUtf8(utf16, utf8);
   printf ("Converting to utf16 : %X - utf8 : %s\n", utf16, utf8);
   ev->keyname = ev->key = ev->string = ev->compose = strdup (utf8);

   return ev;
}

static void
_ecore_psl1ght_mouse_move(s32 x_axis, s32 y_axis)
{
   Ecore_Event_Mouse_Move *ev;

   ev = malloc(sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   mouse_x += x_axis;
   mouse_y += y_axis;
   if (mouse_x < 0) mouse_x = 0;
   if (mouse_y < 0) mouse_y = 0;
   if (mouse_x > window_width) mouse_x = window_width;
   if (mouse_y > window_height) mouse_y = window_height;

   ev->window = 0;
   ev->root_window = 0;
   ev->event_window = 0;
   ev->same_screen = 0;
   ev->timestamp = _ecore_psl1ght_get_time ();
   ev->modifiers = _ecore_psl1ght_get_modifiers ();
   ev->x = ev->root.x = mouse_x;
   ev->y = ev->root.x = mouse_y;

   ev->multi.device = 0;
   ev->multi.radius = ev->multi.radius_x = ev->multi.radius_y = 0;
   ev->multi.pressure = ev->multi.angle = 0;
   ev->multi.x = ev->multi.y = ev->multi.root.x = ev->multi.root.y = 0;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void
_ecore_psl1ght_mouse_button(int button, int pressed)
{
   Ecore_Event_Mouse_Button *ev;
   static unsigned int previous_timestamp = 0;

   ev = malloc(sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   ev->window = 0;
   ev->root_window = 0;
   ev->event_window = 0;
   ev->same_screen = 0;
   ev->timestamp = _ecore_psl1ght_get_time ();
   ev->modifiers = _ecore_psl1ght_get_modifiers ();
   ev->buttons = button;
   if (ev->timestamp - previous_timestamp <= 500)
     ev->double_click = 1;
   ev->triple_click = 0;
   previous_timestamp = ev->timestamp;

   ev->x = ev->root.x = mouse_x;
   ev->y = ev->root.y = mouse_y;
   ev->multi.device = 0;
   ev->multi.radius = ev->multi.radius_x = ev->multi.radius_y = 0;
   ev->multi.pressure = ev->multi.angle = 0;
   ev->multi.x = ev->multi.y = ev->multi.root.x = ev->multi.root.y = 0;

   if (pressed)
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static void
_ecore_psl1ght_mouse_wheel(s8 wheel, s8 tilt)
{
   Ecore_Event_Mouse_Wheel *ev;

   ev = malloc(sizeof(Ecore_Event_Mouse_Wheel));
   if (!ev) return;

   ev->timestamp = _ecore_psl1ght_get_time ();
   ev->window = 0;
   ev->event_window = 0;
   ev->modifiers = _ecore_psl1ght_get_modifiers ();
   ev->direction = 0;
   ev->z = wheel;

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
}

#define PAD_STICK_DEADZONE 0x20

static void
_ecore_psl1ght_poll_joypad(void)
{
   padInfo padinfo;
   int i;

   /**/
   /* Check mouse events */
   ioPadGetInfo (&padinfo);

   for (i = 0; i < 4; i++) /* Ignore the move */
     {
        if (padinfo.status[i])
          {
             int analog_h, analog_v;

             if (ioPadGetData (i, &pad_data) != 0)
               continue;
             analog_h = pad_data.ANA_L_H - 0x80;
             analog_v = pad_data.ANA_L_V - 0x80;

             if (analog_h > PAD_STICK_DEADZONE)
               analog_h -= PAD_STICK_DEADZONE;
             else if (analog_h < -PAD_STICK_DEADZONE)
               analog_h += PAD_STICK_DEADZONE;
             else
               analog_h = 0;
             analog_h /= 10;

             if (analog_v > PAD_STICK_DEADZONE)
               analog_v -= PAD_STICK_DEADZONE;
             else if (analog_v < -PAD_STICK_DEADZONE)
               analog_v += PAD_STICK_DEADZONE;
             else
               analog_v = 0;
             analog_v /= 10;

             if (analog_h != 0 || analog_v != 0)
               _ecore_psl1ght_mouse_move (analog_h, analog_v);

             if (pad_old_x != pad_data.BTN_CROSS)
               _ecore_psl1ght_mouse_button (1, pad_data.BTN_CROSS);
             if (pad_old_o != pad_data.BTN_CIRCLE)
               _ecore_psl1ght_mouse_button (3, pad_data.BTN_CIRCLE);

             pad_old_x = pad_data.BTN_CROSS;
             pad_old_o = pad_data.BTN_CIRCLE;

             //pad_buttons = paddata.buttons;
          }
     }
}

static void
_ecore_psl1ght_poll_mouse(void)
{
   mouseInfo mouseinfo;
   u32 i;

   /**/
   /* Check mouse events */
   ioMouseGetInfo(&mouseinfo);

   if (mouseinfo.status[0] == 1 && !mouse_connected) // Connected
     {
        mouse_connected = TRUE;
        mouse_buttons = 0;

        // Old events in the queue are discarded
        ioMouseClearBuf(0);
     }
   else if (mouseinfo.status[0] != 1 && mouse_connected) // Disconnected
     {
        mouse_connected = FALSE;
        mouse_buttons = 0;
     }

   if (mouse_connected)
     {
        mouseDataList datalist;

        ioMouseGetDataList(0, &datalist);

        for (i = 0; i < datalist.count; i++)
          {
             u8 old_left = mouse_buttons & 1;
             u8 new_left = datalist.list[i].buttons & 1;
             u8 old_right = mouse_buttons & 2;
             u8 new_right = datalist.list[i].buttons & 2;
             u8 old_middle = mouse_buttons & 4;
             u8 new_middle = datalist.list[i].buttons & 4;

             if (datalist.list[i].x_axis != 0 ||
                 datalist.list[i].y_axis != 0)
               _ecore_psl1ght_mouse_move (datalist.list[i].x_axis,
                                          datalist.list[i].y_axis);

             if (old_left != new_left)
               _ecore_psl1ght_mouse_button (1, new_left);
             if (old_middle != new_middle)
               _ecore_psl1ght_mouse_button (2, new_middle);
             if (old_right != new_right)
               _ecore_psl1ght_mouse_button (3, new_right);

             if (datalist.list[i].wheel != 0)
               _ecore_psl1ght_mouse_wheel (datalist.list[i].wheel,
                                           datalist.list[i].tilt);

             mouse_buttons = datalist.list[i].buttons;
          }
     }
}

static void
_ecore_psl1ght_poll_move(void)
{
   int i;
   u16 new_buttons;
   static int t_pressed = 0;
   static int calibrated = 0;
   static float prev_x = 0;
   static float prev_y = 0;
   static int gyro = 0;
   float x, y, z;

   /* Check move events */
   processMove (move_context);
   new_buttons = move_context->state.paddata.buttons & (~move_buttons);
   move_buttons = move_context->state.paddata.buttons;

   moveGet3DPosition (move_context, &x, &y, &z);
   //printf ("Move 3D position is : %f, %f, %f\n", x,y,z);

   switch (new_buttons) {
      case 1:
        gyro = !gyro;
        break;

      case 4:
        // Move button
        printf ("Calibrating\n");
        gemCalibrate (0);
        calibrated = 1;
        break;

      case 8:
        // start button
        _ecore_psl1ght_mouse_move ((window_width / 2) - mouse_x, (window_height / 2) - mouse_y);
        break;
     }

   if (calibrated)
     {
        float x_axis, y_axis;

        if (gyro)
          {
             gemInertialState gem_inert;

             gemGetInertialState (0, 0, 0, &gem_inert);
             x_axis = -vec_array (gem_inert.gyro, 1) * 25;
             y_axis = -vec_array (gem_inert.gyro, 0) * 25;
             if (abs (x_axis) > 2 || abs (y_axis) > 2)
               _ecore_psl1ght_mouse_move (x_axis, y_axis);
          }
        else {
             x_axis = (x - prev_x) * 2.5;
             y_axis = -(y - prev_y) * 2.5;
             prev_x = x;
             prev_y = y;
             _ecore_psl1ght_mouse_move (x_axis, y_axis);
          }

        if (!t_pressed && (move_buttons & 0x2))
          _ecore_psl1ght_mouse_button (1, 1);
        else if (t_pressed && (move_buttons & 0x2) == 0)
          _ecore_psl1ght_mouse_button (1, 0);
        t_pressed = move_buttons & 0x2;
     }
}

static void
_ecore_psl1ght_poll_keyboard(void)
{
   KbInfo kbInfo;
   int i;

   /* Check keyboard events */
   ioKbGetInfo(&kbInfo);

   if (kbInfo.status[0] == 1 && !keyboard_connected)
     {
        /* Connected */
         keyboard_connected = true;

         // Old events in the queue are discarded
         ioKbClearBuf(0);
         keyboard_leds._KbLedU.leds = 0;
         keyboard_mods._KbMkeyU.mkeys = 0;
         keyboard_old_key = 0;

         // Set raw keyboard code types to get scan codes
         ioKbSetCodeType(0, KB_CODETYPE_RAW);
         ioKbSetReadMode(0, KB_RMODE_INPUTCHAR);
     }
   else if (kbInfo.status[0] != 1 && keyboard_connected)
     {
        /* Disconnected keyboard */
         keyboard_connected = FALSE;
     }

   if (keyboard_connected)
     {
        KbData Keys;

        // Read data from the keyboard buffer
        if (ioKbRead(0, &Keys) == 0 && Keys.nb_keycode > 0)
          {
             Ecore_Event_Key *ev = NULL;

             _ecore_psl1ght_key_modifiers (&Keys.mkey, &Keys.led);

             if (Keys.nb_keycode == 0 && keyboard_old_key != 0)
               {
                  ev = _ecore_psl1ght_event_key (keyboard_old_key);
                  if (ev) ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
               }
             for (i = 0; i < Keys.nb_keycode; i++)
               {
                  if (Keys.keycode[i] != keyboard_old_key)
                    {
                       if (Keys.keycode[i] != 0)
                         {
                            ev = _ecore_psl1ght_event_key (Keys.keycode[i]);
                            if (ev)
                              ecore_event_add(ECORE_EVENT_KEY_DOWN, ev,
                                              NULL, NULL);
                         }
                       else
                         {
                            ev = _ecore_psl1ght_event_key (keyboard_old_key);
                            if (ev)
                              ecore_event_add(ECORE_EVENT_KEY_UP, ev,
                                              NULL, NULL);
                         }
                       keyboard_old_key = Keys.keycode[0];
                    }
               }
          }
     }
}

static void
xmb_event_handler(u64 status, u64 param, void *user_data)
{
   printf ("Received event %lX\n", status);
   if (status == SYSUTIL_EXIT_GAME)
     {
        ecore_main_loop_quit();
     }
   else if (status == SYSUTIL_MENU_OPEN)
     {
     }
   else if (status == SYSUTIL_MENU_CLOSE)
     {
     }
   else if (status == SYSUTIL_DRAW_BEGIN)
     {
     }
   else if (status == SYSUTIL_DRAW_END)
     {
     }
}

EAPI void
ecore_psl1ght_poll_events(void)
{
   _ecore_psl1ght_poll_joypad ();
   _ecore_psl1ght_poll_mouse ();
   if (move_context)
     _ecore_psl1ght_poll_move ();
   _ecore_psl1ght_poll_keyboard ();

   sysUtilCheckCallback ();
}

EAPI void
ecore_psl1ght_resolution_set(int width, int height)
{
   window_width = width;
   window_height = height;
   if (mouse_x > window_width) mouse_x = window_width;
   if (mouse_y > window_height) mouse_y = window_height;
}

EAPI void
ecore_psl1ght_screen_resolution_get(int *w, int *h)
{
   videoState state;
   videoResolution resolution;

   /* Get the state of the display */
   if (videoGetState (0, 0, &state) == 0 &&
       videoGetResolution (state.displayMode.resolution, &resolution) == 0)
     {
        if (w) *w = resolution.width;
        if (h) *h = resolution.height;
     }
   else {
        if (w) *w = 0;
        if (h) *h = 0;
     }
}

EAPI void
ecore_psl1ght_optimal_screen_resolution_get(int *w, int *h)
{
   videoDeviceInfo info;
   videoResolution res;
   int area = 720 * 480;
   int mode_area;
   int i;

   if (w) *w = 720;
   if (h) *h = 480;

   videoGetDeviceInfo(0, 0, &info);

   for (i = 0; i < info.availableModeCount; i++) {
        videoGetResolution (info.availableModes[i].resolution, &res);
        mode_area = res.width * res.height;
        if (mode_area > area)
          {
             area = mode_area;
             if (w) *w = res.width;
             if (h) *h = res.height;
          }
     }
}

