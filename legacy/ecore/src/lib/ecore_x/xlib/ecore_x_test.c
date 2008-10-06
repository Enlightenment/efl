/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_x_private.h"
#include "Ecore_X.h"


EAPI int
ecore_x_test_fake_key_down(const char *key)
{
#ifdef ECORE_XTEST
   KeyCode             keycode = 0;
   KeySym              keysym;
   
   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
	keysym = XStringToKeysym(key);
	if (keysym == NoSymbol) return 0;
	keycode  = XKeysymToKeycode(_ecore_x_disp, keysym);
     }
   if (keycode == 0) return 0;
   return XTestFakeKeyEvent(_ecore_x_disp, keycode, 1, 0);
#else
   return 0;
#endif
}

EAPI int
ecore_x_test_fake_key_up(const char *key)
{
#ifdef ECORE_XTEST
   KeyCode             keycode = 0;
   KeySym              keysym;
   
   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
	keysym = XStringToKeysym(key);
	if (keysym == NoSymbol) return 0;
	keycode  = XKeysymToKeycode(_ecore_x_disp, keysym);
     }
   if (keycode == 0) return 0;
   return XTestFakeKeyEvent(_ecore_x_disp, keycode, 0, 0);
#else
   return 0;
#endif
}

EAPI int
ecore_x_test_fake_key_press(const char *key)
{
#ifdef ECORE_XTEST
   KeyCode             keycode = 0;
   KeySym              keysym = 0;
   int                 shift = 0;
   
   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
	keysym = XStringToKeysym(key);
	if (keysym == NoSymbol) return 0;
	keycode = XKeysymToKeycode(_ecore_x_disp, keysym);
	if (XKeycodeToKeysym(_ecore_x_disp, keycode, 0) != keysym)
	  {  
	     if (XKeycodeToKeysym(_ecore_x_disp, keycode, 1) == keysym)
	       shift = 1;
	     else
	       keycode = 0;
	  }
	else
	  shift = 0;
     }
   if (keycode == 0)
     {
	static int mod = 0;
	KeySym *keysyms;
	int keycode_min, keycode_max, keycode_num;
	int i;	

	XDisplayKeycodes(_ecore_x_disp, &keycode_min, &keycode_max);
	keysyms = XGetKeyboardMapping(_ecore_x_disp, keycode_min,
				      keycode_max - keycode_min + 1,
				      &keycode_num);
	mod = (mod + 1) & 0x7;
	i = (keycode_max - keycode_min - mod - 1) * keycode_num;
	
	keysyms[i] = keysym;
	XChangeKeyboardMapping(_ecore_x_disp, keycode_min, keycode_num,
			       keysyms, (keycode_max - keycode_min));
	XFree(keysyms);
	XSync(_ecore_x_disp, False);
	keycode = keycode_max - mod - 1;
     }
   if (shift)
     XTestFakeKeyEvent(_ecore_x_disp, 
		       XKeysymToKeycode(_ecore_x_disp, XK_Shift_L), 1, 0);
   XTestFakeKeyEvent(_ecore_x_disp, keycode, 1, 0);
   XTestFakeKeyEvent(_ecore_x_disp, keycode, 0, 0);
   if (shift)
     XTestFakeKeyEvent(_ecore_x_disp, 
		       XKeysymToKeycode(_ecore_x_disp, XK_Shift_L), 0, 0);
   return 1;
#else
   return 0;
#endif
}

EAPI const char *
ecore_x_keysym_string_get(int keysym)
{
   return XKeysymToString(keysym);
}
