static void _ecore_fb_event_free_key_down(void *data, void *ev);
static void _ecore_fb_event_free_key_up(void *data, void *ev);

static const char *_ecore_fb_kbd_syms[128 * 6] =
{
#include "ecore_fb_keytable.h"
};

static const char *_ecore_fb_btn_syms[128] =
{
   "0x00",
     "Escape", 
     "F1", 
     "F2", 
     "F3", 
     "F4", 
     "Up", 
     "Right",
     "Left", 
     "Down", 
     "Return", 
     "0x1b", 
     "0x1c", 
     "0x1d", 
     "0x1e",
     "0x1f",
     "0x20", 
     "0x21", 
     "0x22", 
     "0x23", 
     "0x24", 
     "0x25", 
     "0x26", 
     "0x27",
     "0x28", 
     "0x29", 
     "0x2a",
     "0x2b",
     "0x2c",
     "0x2d",
     "0x2e", 
     "0x2f", 
     "0x30", 
     "0x31", 
     "0x32", 
     "0x33", 
     "0x34", 
     "0x35", 
     "0x36", 
     "0x37",
     "0x38", 
     "0x39", 
     "0x3a",
     "0x3b",
     "0x3c",
     "0x3d",
     "0x3e", 
     "0x3f", 
     "0x40", 
     "0x41", 
     "0x42", 
     "0x43", 
     "0x44", 
     "0x45", 
     "0x46", 
     "0x47",
     "0x48", 
     "0x49", 
     "0x4a",
     "0x4b",
     "0x4c",
     "0x4d",
     "0x4e", 
     "0x4f", 
     "0x50", 
     "0x51", 
     "0x52", 
     "0x53", 
     "0x54", 
     "0x55", 
     "0x56", 
     "0x57",
     "0x58", 
     "0x59", 
     "0x5a",
     "0x5b",
     "0x5c",
     "0x5d",
     "0x5e", 
     "0x5f", 
     "0x60", 
     "0x61", 
     "0x62", 
     "0x63", 
     "0x64", 
     "0x65", 
     "0x66", 
     "0x67",
     "0x68", 
     "0x69", 
     "0x6a",
     "0x6b",
     "0x6c",
     "0x6d",
     "0x6e", 
     "0x6f", 
     "0x70", 
     "0x71", 
     "0x72", 
     "0x73", 
     "0x74", 
     "0x75", 
     "0x76", 
     "0x77",
     "0x78", 
     "0x79", 
     "0x7a",
     "0x7b",
     "0x7c",
     "0x7d",
     "0x7e", 
     "0x7f"
};
static int _ecore_fb_kbd_fd = 0;
static int _ecore_fb_ctrl = 0;
static int _ecore_fb_alt = 0;
static int _ecore_fb_shift = 0;
static int _ecore_fb_lock = 0;

static Ecore_Fd_Handler *_ecore_fb_kbd_fd_handler_handle = NULL;
static int _ecore_fb_kbd_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);

static void 
_ecore_fb_event_free_key_down(void *data __UNUSED__, void *ev)
{
   Ecore_Fb_Event_Key_Up *e;
   e = ev;
   free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static void
_ecore_fb_event_free_key_up(void *data __UNUSED__, void *ev)
{
   Ecore_Fb_Event_Key_Up *e;
   
   e = ev;
   free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static int
_ecore_fb_kbd_fd_handler(void *data __UNUSED__, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   int v = 0;
   
   do
     {
	unsigned char buf;
	
	v = read(_ecore_fb_kbd_fd, &buf, 1);
	if (v < 0) return 1;
	if (v < 1) return 1;
	if (!(buf & 0x80))
	  {
	     /* DOWN */
	     int vt_switch = -1;
	     Ecore_Fb_Event_Key_Down *e;
	     
	     e = calloc(1, sizeof(Ecore_Fb_Event_Key_Down));
	     if (!e) goto retry;
	     if (_ecore_fb_kbd_fd == _ecore_fb_tty_fd)
	       {
		  int add = 0;
		  
		  if (_ecore_fb_shift) add = 1;
		  else if (_ecore_fb_lock) add = 2;
		  e->keyname = strdup(_ecore_fb_kbd_syms[(buf & 0x7f) * 6]);
		  e->keysymbol = strdup(_ecore_fb_kbd_syms[((buf & 0x7f) * 6) + add]);
		  e->key_compose = strdup(_ecore_fb_kbd_syms[((buf & 0x7f) * 6) + 3 + add]);
	       }
	     else
	       e->keyname = strdup(_ecore_fb_btn_syms[buf & 0x7f]);
	     if (!e->keyname)
	       {
		  free(e);
		  goto retry;
	       }
	     ecore_event_add(ECORE_FB_EVENT_KEY_DOWN, e, _ecore_fb_event_free_key_down, NULL);
	     if (!strcmp(e->keyname, "Control_L"))
	       _ecore_fb_ctrl++;
	     else if (!strcmp(e->keyname, "Control_R"))
	       _ecore_fb_ctrl++;
	     else if (!strcmp(e->keyname, "Alt_L"))
	       _ecore_fb_alt++;
	     else if (!strcmp(e->keyname, "Alt_R"))
	       _ecore_fb_alt++;
	     else if (!strcmp(e->keyname, "Shift_L"))
	       _ecore_fb_shift++;
	     else if (!strcmp(e->keyname, "Shift_R"))
	       _ecore_fb_shift++;
	     else if (!strcmp(e->keyname, "Caps_Lock"))
	       _ecore_fb_lock++;
	     else if (!strcmp(e->keyname, "F1")) vt_switch = 0;
	     else if (!strcmp(e->keyname, "F2")) vt_switch = 1;
	     else if (!strcmp(e->keyname, "F3")) vt_switch = 2;
	     else if (!strcmp(e->keyname, "F4")) vt_switch = 3;
	     else if (!strcmp(e->keyname, "F5")) vt_switch = 4;
	     else if (!strcmp(e->keyname, "F6")) vt_switch = 5;
	     else if (!strcmp(e->keyname, "F7")) vt_switch = 6;
	     else if (!strcmp(e->keyname, "F8")) vt_switch = 7;
	     else if (!strcmp(e->keyname, "F9")) vt_switch = 8;
	     else if (!strcmp(e->keyname, "F10")) vt_switch = 9;
	     else if (!strcmp(e->keyname, "F11")) vt_switch = 10;
	     else if (!strcmp(e->keyname, "F12")) vt_switch = 11;
	     if (_ecore_fb_ctrl > 2) _ecore_fb_ctrl = 2;
	     if (_ecore_fb_alt > 2) _ecore_fb_alt = 2;
	     if ((vt_switch >= 0) &&
		 (_ecore_fb_ctrl) &&
		 (_ecore_fb_alt))
	       _ecore_fb_vt_switch(vt_switch);
	  }
	else
	  {
	     /* UP */
	     Ecore_Fb_Event_Key_Up *e;
	     
	     e = calloc(1, sizeof(Ecore_Fb_Event_Key_Up));
	     if (!e) goto retry;
	     if (_ecore_fb_kbd_fd == _ecore_fb_tty_fd)
	       {
		  int add = 0;
		  
		  if (_ecore_fb_shift) add = 1;
		  else if (_ecore_fb_lock) add = 2;
		  e->keyname = strdup(_ecore_fb_kbd_syms[(buf & 0x7f) * 6]);
		  e->keysymbol = strdup(_ecore_fb_kbd_syms[((buf & 0x7f) * 6) + add]);
		  e->key_compose = strdup(_ecore_fb_kbd_syms[((buf & 0x7f) * 6) + 3 + add]);
	       }
	     else
	       e->keyname = strdup(_ecore_fb_btn_syms[buf & 0x7f]);
	     if (!e->keyname)
	       {
		  free(e);
		  goto retry;
	       }
	     ecore_event_add(ECORE_FB_EVENT_KEY_UP, e, _ecore_fb_event_free_key_up, NULL);
	     if (!strcmp(e->keyname, "Control_L"))
	       _ecore_fb_ctrl--;
	     else if (!strcmp(e->keyname, "Control_R"))
	       _ecore_fb_ctrl--;
	     else if (!strcmp(e->keyname, "Alt_L"))
	       _ecore_fb_alt--;
	     else if (!strcmp(e->keyname, "Alt_R"))
	       _ecore_fb_alt--;
	     else if (!strcmp(e->keyname, "Shift_L"))
	       _ecore_fb_shift--;
	     else if (!strcmp(e->keyname, "Shift_R"))
	       _ecore_fb_shift--;
	     else if (!strcmp(e->keyname, "Caps_Lock"))
	       _ecore_fb_lock--;
	     if (_ecore_fb_ctrl < 0) _ecore_fb_ctrl = 0;
	     if (_ecore_fb_alt < 0) _ecore_fb_alt = 0;
	     if (_ecore_fb_shift < 0) _ecore_fb_shift = 0;
	     if (_ecore_fb_lock < 0) _ecore_fb_lock = 0;
	  }
	retry:
	;
     }
   while (v > 0);
   return 1;
}

int
ecore_fb_kbd_init(void)
{
   int prev_flags;
   
   prev_flags = fcntl(_ecore_fb_kbd_fd, F_GETFL);
   fcntl(_ecore_fb_kbd_fd, F_SETFL, prev_flags | O_NONBLOCK);
   _ecore_fb_kbd_fd_handler_handle = ecore_main_fd_handler_add(_ecore_fb_kbd_fd,
								    ECORE_FD_READ,
								    _ecore_fb_kbd_fd_handler, NULL,
								    NULL, NULL);
   if(!_ecore_fb_kbd_fd_handler_handle) return 0;
   return 1;
}

void
ecore_fb_kbd_shutdown(void)
{
   if (_ecore_fb_kbd_fd >= 0) close(_ecore_fb_kbd_fd);
   if (_ecore_fb_kbd_fd_handler_handle) 
     ecore_main_fd_handler_del(_ecore_fb_kbd_fd_handler_handle);
   _ecore_fb_kbd_fd = 0;
   _ecore_fb_kbd_fd_handler_handle = NULL;
   _ecore_fb_ctrl = 0;
   _ecore_fb_lock = 0;
   _ecore_fb_shift = 0;
   _ecore_fb_alt = 0;
}
