#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>


#ifdef LOGRT
#include <dlfcn.h>
#endif /* ifdef LOGRT */

#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"


//////////////////////////////////////////////////////////////////////////////
// This api and structure only for the key router and window client side
// Application do not use this

//this mask is defined by key router.
//after discussing with keyrouter module, this mask can be changed
#define GRAB_MASK 0xffff00
#define OVERRIDE_EXCLUSIVE_GRAB 0xf00000
#define EXCLUSIVE_GRAB 0x0f0000
#define TOPMOST_GRAB 0x00f000
#define SHARED_GRAB 0x000f00

//if _ecore_keyrouter = 0, not yet check keyrouter
//if _ecore_keyrouter = -1, keyrouter not exist
//if _ecore_keyrouter = 1, keyrouter exist
int _ecore_keyrouter = 0;

struct _Ecore_X_Window_Key_Table
{
   Ecore_X_Window       win;       //windo ID
   int                 *key_list;  //list of key
   unsigned long        key_cnt;   // the number of key
};

typedef struct _Ecore_X_Window_Key_Table             Ecore_X_Window_Key_Table;

static int       _ecore_x_window_keytable_key_search(Ecore_X_Window_Key_Table *keytable, int key);
static Eina_Bool _ecore_x_window_keytable_key_del(Ecore_X_Window_Key_Table *key_table, int key, Ecore_X_Atom keytable_atom);

static Eina_Bool _ecore_x_window_keytable_key_add(Ecore_X_Window_Key_Table *keytable,
                                 int keycode,
                                 Ecore_X_Win_Keygrab_Mode grab_mode);

static Eina_Bool _ecore_x_window_keygrab_set_internal(Ecore_X_Window win, const char *key, Ecore_X_Win_Keygrab_Mode grab_mode);
static Eina_Bool _ecore_x_window_keygrab_unset_internal(Ecore_X_Window win, const char *key);
static Eina_Bool _ecore_x_window_keytable_get(Ecore_X_Window win, Ecore_X_Window_Key_Table *keytable);


//(Below Atom and exclusiveness_get/set functions) should be changed after keyrouter finds the solution to avoid race condition
//solution 1. window manages two key table. keytable and keytable result
//solution 2. using client messabe between the window client and the key router.

static Atom _atom_grab_excl_win = None;
#define STR_ATOM_GRAB_EXCL_WIN "_GRAB_EXCL_WIN_KEYCODE"

static void
_keytable_free(Ecore_X_Window_Key_Table *keytable)
{
   if (keytable->key_list)
     free(keytable->key_list);
   keytable->key_list = NULL;
   keytable->win = 0;
   keytable->key_cnt = 0;
}

static int
_keytable_property_list_get(Ecore_X_Window win,
                            Ecore_X_Atom atom,
                            unsigned int **plst)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   unsigned int i, *val;
   int num;

   *plst = NULL;
   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          XA_CARDINAL, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) != Success)
     {
        WRN("XGetWindowProperty failed");
        return -1;
     }
   else if ((num_ret == 0) || (!prop_ret))
     num = 0;
   else
     {
        val = malloc(num_ret * sizeof(unsigned int));
        if (!val)
          {
             if (prop_ret) XFree(prop_ret);
             WRN("Memory alloc failed");
             return -1;
          }
        for (i = 0; i < num_ret; i++)
          val[i] = ((unsigned long *)prop_ret)[i];
        num = num_ret;
        *plst = val;
     }

   if (_ecore_xlib_sync) ecore_x_sync();
   if (prop_ret)
     XFree(prop_ret);
   return num;
}

static Eina_Bool
_ecore_x_window_keytable_possible_global_exclusiveness_get(int keycode)
{
   int ret = 0;

   Ecore_X_Window_Key_Table keytable;

   keytable.win = ecore_x_window_root_first_get();
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   if(_atom_grab_excl_win == None )
     _atom_grab_excl_win = XInternAtom(_ecore_x_disp, STR_ATOM_GRAB_EXCL_WIN, False);

   ret = _keytable_property_list_get(keytable.win, _atom_grab_excl_win,
                                     (unsigned int **)&(keytable.key_list));

   if (ret < 0)
     {
	    return EINA_FALSE;
     }

   keytable.key_cnt = ret;

   if (keytable.key_cnt == 0)
     {
        WRN("There is no keygrab entry in the table");
        return EINA_TRUE;
     }

   //check keycode exists in the global exclusiveness keytable

   ret = _ecore_x_window_keytable_key_search(&keytable, keycode);
   if (ret != -1)
     {
        WRN("Can't search keygrab entry in the table");
        _keytable_free(&keytable);
        return EINA_FALSE;
     }
   _keytable_free(&keytable);
   return EINA_TRUE;
}

static Eina_Bool
_ecore_x_window_keytable_possible_global_exclusiveness_set(int keycode)
{
   int ret = 0;

   Ecore_X_Window_Key_Table keytable;

   keytable.win = ecore_x_window_root_first_get();
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   if(_atom_grab_excl_win == None )
     _atom_grab_excl_win = XInternAtom(_ecore_x_disp, STR_ATOM_GRAB_EXCL_WIN, False);

   ret = _keytable_property_list_get(keytable.win, _atom_grab_excl_win,
                                     (unsigned int **)&(keytable.key_list));
   if (ret < 0) return EINA_FALSE;

   keytable.key_cnt = ret;

   if (keytable.key_cnt == 0)
     {
        XChangeProperty(_ecore_x_disp, keytable.win, _atom_grab_excl_win, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char *)&keycode, 1);
        XSync(_ecore_x_disp, False);
        _keytable_free(&keytable);
        return EINA_TRUE;
     }

   //check keycode exists in the global exclusiveness keytable
   ret = _ecore_x_window_keytable_key_search(&keytable, keycode);
   if (ret != -1)
     {
        XChangeProperty(_ecore_x_disp, keytable.win, _atom_grab_excl_win, XA_CARDINAL, 32,
                        PropModeAppend, (unsigned char *)&keycode, 1);
        XSync(_ecore_x_disp, False);
        _keytable_free(&keytable);
        return EINA_TRUE;
     }
   WRN("Already key is grabbed");
   _keytable_free(&keytable);
   return EINA_FALSE;
}

static Eina_Bool
_ecore_x_window_keytable_possible_global_exclusiveness_unset(int keycode)
{
   int ret = 0;

   Ecore_X_Window_Key_Table keytable;

   keytable.win = ecore_x_window_root_first_get();
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   if(_atom_grab_excl_win == None )
     _atom_grab_excl_win = XInternAtom(_ecore_x_disp, STR_ATOM_GRAB_EXCL_WIN, False);

   ret = _keytable_property_list_get(keytable.win, _atom_grab_excl_win,
                                     (unsigned int **)&(keytable.key_list));
   if (ret <= 0) return EINA_FALSE;

   keytable.key_cnt = ret;

   //check keycode exists in the global exclusiveness keytable
   ret = _ecore_x_window_keytable_key_search(&keytable, keycode);
   if (ret == -1)
     {
        WRN("Already key exists");
        _keytable_free(&keytable);
        return EINA_FALSE;
     }
   else
     _ecore_x_window_keytable_key_del(&keytable, keycode, _atom_grab_excl_win);

   _keytable_free(&keytable);
   return EINA_FALSE;
}

static Eina_Bool
_ecore_x_window_keytable_keycode_decode(int keycode_encoded,
                                        int *keycode,
                                        Ecore_X_Win_Keygrab_Mode *grab_mode)
{
   int key_mask = 0;

   *keycode = keycode_encoded & (~GRAB_MASK);
   key_mask = keycode_encoded & GRAB_MASK;

   if (key_mask == SHARED_GRAB)
     {
        *grab_mode = ECORE_X_WIN_KEYGRAB_SHARED;
        return EINA_TRUE;
     }
   else if (key_mask == TOPMOST_GRAB)
     {
        *grab_mode = ECORE_X_WIN_KEYGRAB_TOPMOST;
        return EINA_TRUE;
     }
   else if (key_mask == EXCLUSIVE_GRAB)
     {
        *grab_mode = ECORE_X_WIN_KEYGRAB_EXCLUSIVE;
        return EINA_TRUE;
     }
   else if (key_mask == OVERRIDE_EXCLUSIVE_GRAB)
     {
        *grab_mode = ECORE_X_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE;
        return EINA_TRUE;
     }
   else
     {
        *grab_mode = ECORE_X_WIN_KEYGRAB_UNKNOWN;
        WRN("Keycode decoding failed. Unknown Keygrab mode");
        return EINA_FALSE;
     }
}

static Eina_Bool
_ecore_x_window_keytable_keycode_encode(int keycode,
                                        Ecore_X_Win_Keygrab_Mode grab_mode,
                                        int *keycode_encoded)
{
   if ((grab_mode <= ECORE_X_WIN_KEYGRAB_UNKNOWN) || (grab_mode > ECORE_X_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE))
     {
        *keycode_encoded = 0;
        WRN("Keycode encoding failed. Unknown Keygrab mode");
        return EINA_FALSE;
     }
   if (grab_mode == ECORE_X_WIN_KEYGRAB_SHARED)
     *keycode_encoded = keycode | SHARED_GRAB;
   else if (grab_mode == ECORE_X_WIN_KEYGRAB_TOPMOST)
     *keycode_encoded = keycode | TOPMOST_GRAB;
   else if (grab_mode == ECORE_X_WIN_KEYGRAB_EXCLUSIVE)
     *keycode_encoded = keycode | EXCLUSIVE_GRAB;
   else if (grab_mode == ECORE_X_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE)
     *keycode_encoded = keycode | OVERRIDE_EXCLUSIVE_GRAB;
   return EINA_TRUE;
}

static Eina_Bool
_ecore_x_window_keytable_get(Ecore_X_Window win,
                             Ecore_X_Window_Key_Table *keytable)
{
   int ret = 0;

   ret = _keytable_property_list_get(win, ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE,
                                     (unsigned int **)&(keytable->key_list));
   if (ret < 0) return EINA_FALSE;

   keytable->key_cnt = ret;

   return EINA_TRUE;
}

static int
_ecore_x_window_keytable_key_search(Ecore_X_Window_Key_Table *keytable,
                                    int key)
{
   int  i;
   int keycode = 0;
   unsigned long key_cnt;
   int *key_list = NULL;

   keycode = key & (~GRAB_MASK);
   key_cnt = keytable->key_cnt;
   key_list = keytable->key_list;

   for (i = key_cnt - 1; i >= 0; i--)
     {
        if ((key_list[i] & (~GRAB_MASK)) == keycode) break;
     }
   return i;
}


static Eina_Bool
_ecore_x_window_keytable_key_add(Ecore_X_Window_Key_Table *keytable,
                                 int keycode,
                                 Ecore_X_Win_Keygrab_Mode grab_mode)
{
   int i = 0;
   int keycode_masked = 0;

   Ecore_Window   win;
   unsigned long  key_cnt;

   win = keytable->win;
   key_cnt = keytable->key_cnt;

   if (!_ecore_x_window_keytable_keycode_encode(keycode, grab_mode, &keycode_masked))
     return EINA_FALSE;

   if (key_cnt == 0)
     {
        XChangeProperty(_ecore_x_disp, win, ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char *)&keycode_masked, 1);
        XSync(_ecore_x_disp, False);
        return EINA_TRUE;
     }
   else
     {
        i = _ecore_x_window_keytable_key_search(keytable, keycode_masked);
        if ( i != -1 )
          {
             //already exist key in key table
             WRN("Already key exists");
             return EINA_FALSE;
          }
        XChangeProperty(_ecore_x_disp, win, ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE, XA_CARDINAL, 32,
                        PropModeAppend, (unsigned char *)&keycode_masked, 1);
        XSync(_ecore_x_disp, False);
        return EINA_TRUE;
     }
}

static Eina_Bool
_ecore_x_window_keytable_key_del(Ecore_X_Window_Key_Table *key_table,
                                 int key,
                                 Ecore_X_Atom keytable_atom)
{
   int i;
   int *new_key_list = NULL;
   unsigned long key_cnt = 0;

   // Only one element is exists in the list of grabbed key
   i = _ecore_x_window_keytable_key_search(key_table, key);

   if (i == -1)
     {
        WRN("Key doesn't exist in the key table.");
        return EINA_FALSE;
     }

   (key_table->key_cnt)--;
   key_cnt = key_table->key_cnt;

   if (key_cnt == 0)
     {
        XDeleteProperty(_ecore_x_disp, key_table->win, keytable_atom);
        XSync(_ecore_x_disp, False);
        return EINA_TRUE;
     }

   // Shrink the buffer
   new_key_list = malloc((key_cnt) * sizeof(int));

   if (new_key_list == NULL)
     return EINA_FALSE;

   // copy head
   if (i > 0)
     memcpy(new_key_list, key_table->key_list, sizeof(int) * i);

   // copy tail
   if ((key_cnt) - i > 0)
     {
        memcpy(new_key_list + i,
               key_table->key_list + i + 1,
               sizeof(int) * (key_cnt - i));
     }

   XChangeProperty(_ecore_x_disp, key_table->win, keytable_atom, XA_CARDINAL, 32,
                   PropModeReplace, (unsigned char *)new_key_list, key_cnt);
   XSync(_ecore_x_disp, False);

   free(new_key_list);
   return EINA_TRUE;
}

static Eina_Bool
_ecore_x_window_keygrab_set_internal(Ecore_X_Window win,
                                     const char *key,
                                     Ecore_X_Win_Keygrab_Mode grab_mode)
{
   KeyCode keycode = 0;
   KeySym keysym;

   Eina_Bool ret = EINA_FALSE;
   Ecore_X_Window_Key_Table keytable;

   keytable.win = win;
   keytable.key_list = NULL;
   keytable.key_cnt = 0;


   //check the key string
   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
        keysym = XStringToKeysym(key);
        if (keysym == NoSymbol)
          {
             WRN("Keysym of key(\"%s\") doesn't exist", key);
             return ret;
          }
        keycode = XKeysymToKeycode(_ecore_x_disp, keysym);
     }

   if (keycode == 0)
     {
        WRN("Keycode of key(\"%s\") doesn't exist", key);
        return ret;
     }

   if(grab_mode == ECORE_X_WIN_KEYGRAB_EXCLUSIVE)
     {
        //Only one window can grab this key;
        //keyrouter should avoid race condition
        if (!_ecore_x_window_keytable_possible_global_exclusiveness_get(keycode))
          return EINA_FALSE;
     }

   if (!_ecore_x_window_keytable_get(win, &keytable))
     return EINA_FALSE;

   ret = _ecore_x_window_keytable_key_add(&keytable, keycode, grab_mode);


   if (!ret)
     {
        WRN("Key(\"%s\") add failed", key);
        goto error;
     }

   if(grab_mode == ECORE_X_WIN_KEYGRAB_EXCLUSIVE)
     {
        //Only one window can grab this key;
        if(!_ecore_x_window_keytable_possible_global_exclusiveness_set(keycode))
          {
             _ecore_x_window_keytable_key_del(&keytable, keycode, ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE);
             WRN("Key(\"%s\") already is grabbed", key);
             goto error;
          }
     }

   _keytable_free(&keytable);
   return EINA_TRUE;
error:
   _keytable_free(&keytable);
   return EINA_FALSE;
}

static Eina_Bool
_ecore_x_window_keygrab_unset_internal(Ecore_X_Window win,
                                       const char *key)
{
   KeyCode keycode = 0;
   KeySym keysym;

   int i;
   int key_masked = 0;
   int key_decoded = 0;

   Eina_Bool ret = EINA_FALSE;

   Ecore_X_Window_Key_Table keytable;
   Ecore_X_Win_Keygrab_Mode grab_mode = ECORE_X_WIN_KEYGRAB_UNKNOWN;

   keytable.win = win;
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
        keysym = XStringToKeysym(key);
        if (keysym == NoSymbol)
          {
             WRN("Keysym of key(\"%s\") doesn't exist", key);
             return EINA_FALSE;
          }
        keycode = XKeysymToKeycode(_ecore_x_disp, keysym);
     }

   if (keycode == 0)
     {
        WRN("Keycode of key(\"%s\") doesn't exist", key);
        return EINA_FALSE;
     }

   //construct the keytable structure using Xproperty
   if (!_ecore_x_window_keytable_get(win, &keytable))
      return EINA_FALSE;

   if (keytable.key_cnt == 0)
     return EINA_FALSE;

   i = _ecore_x_window_keytable_key_search(&keytable, keycode);

   if (i == -1) //cannot find key in keytable
     {
        WRN("Key(\"%s\") doesn't exist", key);
        goto error;
     }

   //find key in keytable
   key_masked = keytable.key_list[i];

   ret = _ecore_x_window_keytable_keycode_decode(key_masked, &key_decoded, &grab_mode);

   if (!ret)
     goto error;

   ret = _ecore_x_window_keytable_key_del(&keytable, key_masked, ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE);
   if (!ret)
     goto error;

   if (grab_mode == ECORE_X_WIN_KEYGRAB_EXCLUSIVE)
     {
        ret = _ecore_x_window_keytable_possible_global_exclusiveness_unset(keycode);
     }

   _keytable_free(&keytable);
   return EINA_TRUE;
error:
   _keytable_free(&keytable);
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_window_keygrab_set(Ecore_X_Window win,
                           const char *key,
                           int mod EINA_UNUSED,
                           int not_mod EINA_UNUSED,
                           int priority EINA_UNUSED,
                           Ecore_X_Win_Keygrab_Mode grab_mode)
{
   if (_ecore_keyrouter == 0)
     {
        if(ecore_x_e_keyrouter_get(win))
          _ecore_keyrouter = 1;
        else
          {
             WRN("Keyrouter is not supported");
             _ecore_keyrouter = -1;
          }
     }
   if (_ecore_keyrouter < 0)
     return EINA_FALSE;

   return _ecore_x_window_keygrab_set_internal(win, key, grab_mode);
}

EAPI Eina_Bool
ecore_x_window_keygrab_unset(Ecore_X_Window win,
                             const char *key,
                             int mod EINA_UNUSED,
                             int any_mod EINA_UNUSED)
{
   if (_ecore_keyrouter != 1)
     {
        WRN("Keyrouter is not supported");
        return EINA_FALSE;
     }

   return _ecore_x_window_keygrab_unset_internal(win, key);
}

