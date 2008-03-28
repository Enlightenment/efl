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
	keycode  = XKeysymToKeycode(_ecore_x_disp, XStringToKeysym(key));
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
	keycode  = XKeysymToKeycode(_ecore_x_disp, XStringToKeysym(key));
     }
   if (keycode == 0) return 0;
   return XTestFakeKeyEvent(_ecore_x_disp, keycode, 0, 0);
#else
   return 0;
#endif
}
