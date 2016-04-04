#include "ecore_xcb_private.h"

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

typedef struct _Ecore_X_Window_Key_Table
{
   Ecore_X_Window       win;       //windo ID
   int                 *key_list;  //list of key
   unsigned long        key_cnt;   // the number of key
} Ecore_X_Window_Key_Table;

static Ecore_X_Atom _atom_grab_excl_win = XCB_NONE;
#define STR_ATOM_GRAB_EXCL_WIN "_GRAB_EXCL_WIN_KEYCODE"

static void
_keytable_free(Ecore_X_Window_Key_Table *keytable)
{
   if (keytable->key_list) free(keytable->key_list);
   keytable->key_list = NULL;
   keytable->win = 0;
   keytable->key_cnt = 0;
}

static int
_keytable_property_list_get(Ecore_X_Window win, Ecore_X_Atom atom, unsigned int **plst)
{
   return ecore_x_window_prop_card32_list_get(win, atom, plst);
}

static Eina_Bool
_keytable_get(Ecore_X_Window win, Ecore_X_Window_Key_Table *keytable)
{
   int ret = 0;

   ret = _keytable_property_list_get(win, ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE,
                                     (unsigned int **)&(keytable->key_list));
   if (ret < 0) return EINA_FALSE;

   keytable->key_cnt = ret;
   return EINA_TRUE;
}

static Eina_Bool
_keytable_keycode_decode(int encoded, int *keycode, Ecore_X_Win_Keygrab_Mode *grab_mode)
{
   int mask = 0;

   *keycode = encoded & (~GRAB_MASK);
   mask = encoded & GRAB_MASK;

   if (mask == SHARED_GRAB)
     *grab_mode = ECORE_X_WIN_KEYGRAB_SHARED;
   else if (mask == TOPMOST_GRAB)
     *grab_mode = ECORE_X_WIN_KEYGRAB_TOPMOST;
   else if (mask == EXCLUSIVE_GRAB)
     *grab_mode = ECORE_X_WIN_KEYGRAB_EXCLUSIVE;
   else if (mask == OVERRIDE_EXCLUSIVE_GRAB)
     *grab_mode = ECORE_X_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE;
   else
     {
        *grab_mode = ECORE_X_WIN_KEYGRAB_UNKNOWN;
        WRN("Keycode decoding failed. Unknown Keygrab mode");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_keytable_keycode_encode(int keycode, Ecore_X_Win_Keygrab_Mode grab_mode, int *encoded)
{
   if ((grab_mode <= ECORE_X_WIN_KEYGRAB_UNKNOWN) ||
       (grab_mode > ECORE_X_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE))
     {
        *encoded = 0;
        WRN("Keycode encoding failed. Unknown Keygrab mode");
        return EINA_FALSE;
     }

   if (grab_mode == ECORE_X_WIN_KEYGRAB_SHARED)
     *encoded = keycode | SHARED_GRAB;
   else if (grab_mode == ECORE_X_WIN_KEYGRAB_TOPMOST)
     *encoded = keycode | TOPMOST_GRAB;
   else if (grab_mode == ECORE_X_WIN_KEYGRAB_EXCLUSIVE)
     *encoded = keycode | EXCLUSIVE_GRAB;
   else if (grab_mode == ECORE_X_WIN_KEYGRAB_OVERRIDE_EXCLUSIVE)
     *encoded = keycode | OVERRIDE_EXCLUSIVE_GRAB;

   return EINA_TRUE;
}

static int
_keytable_key_search(Ecore_X_Window_Key_Table *keytable, int key)
{
   int i, code = 0, *list = NULL;
   unsigned long count;

   code = key & (~GRAB_MASK);
   count = keytable->key_cnt;
   list = keytable->key_list;

   for (i = count - 1; i >= 0; i--)
     if ((list[i] & (~GRAB_MASK)) == code) break;

   return i;
}

static Eina_Bool
_keytable_key_add(Ecore_X_Window_Key_Table *keytable, int keycode, Ecore_X_Win_Keygrab_Mode grab_mode)
{
   Ecore_X_Window win;
   unsigned long count;
   int i = 0, masked = 0;

   win = keytable->win;
   count = keytable->key_cnt;

   if (!_keytable_keycode_encode(keycode, grab_mode, &masked))
     return EINA_FALSE;

   if (count == 0)
     {
        xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                            ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE,
                            ECORE_X_ATOM_CARDINAL, 32, 1,
                            (unsigned char *)&masked);
        return EINA_TRUE;
     }
   else
     {
        i = _keytable_key_search(keytable, masked);
        if (i != -1)
          {
             WRN("Key already exists");
             return EINA_FALSE;
          }
        xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_APPEND, win,
                            ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE,
                            ECORE_X_ATOM_CARDINAL, 32, 1,
                            (unsigned char *)&masked);
        return EINA_TRUE;
     }
}

static Eina_Bool
_keytable_key_del(Ecore_X_Window_Key_Table *keytable, int key, Ecore_X_Atom atom)
{
   int i, *new_key_list = NULL;
   unsigned long count = 0;

   i = _keytable_key_search(keytable, key);
   if (i == -1)
     {
        WRN("Key does not exist in the key table");
        return EINA_FALSE;
     }

   keytable->key_cnt--;
   count = keytable->key_cnt;
   if (count == 0)
     {
        ecore_x_window_prop_property_del(keytable->win, atom);
        return EINA_TRUE;
     }

   new_key_list = malloc(count * sizeof(int));
   if (!new_key_list) return EINA_FALSE;

   if (i > 0)
     memcpy(new_key_list, keytable->key_list, sizeof(int) * i);

   if (count - i > 0)
     memcpy(new_key_list + i, keytable->key_list + i + 1,
            sizeof(int) * (count - i));

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, keytable->win,
                       atom, ECORE_X_ATOM_CARDINAL, 32, count,
                       (unsigned char *)new_key_list);

   free(new_key_list);
   return EINA_TRUE;
}

static Eina_Bool
_keytable_possible_global_exclusiveness_get(int keycode)
{
   Ecore_X_Window_Key_Table keytable;
   int ret = 0;

   keytable.win = ecore_x_window_root_first_get();
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   if (_atom_grab_excl_win == XCB_NONE)
     _atom_grab_excl_win = ecore_x_atom_get(STR_ATOM_GRAB_EXCL_WIN);

   ret = _keytable_property_list_get(keytable.win, _atom_grab_excl_win,
                                     (unsigned int **)&(keytable.key_list));
   if (ret < 0) return EINA_FALSE;

   keytable.key_cnt = ret;
   if (keytable.key_cnt == 0)
     {
        WRN("There is no keygrab entry in the table");
        return EINA_TRUE;
     }

   ret = _keytable_key_search(&keytable, keycode);
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
_keytable_possible_global_exclusiveness_set(int keycode)
{
   Ecore_X_Window_Key_Table keytable;
   int ret = 0;

   keytable.win = ecore_x_window_root_first_get();
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   if (_atom_grab_excl_win == XCB_NONE)
     _atom_grab_excl_win = ecore_x_atom_get(STR_ATOM_GRAB_EXCL_WIN);

   ret = _keytable_property_list_get(keytable.win, _atom_grab_excl_win,
                                     (unsigned int **)&(keytable.key_list));
   if (ret < 0) return EINA_FALSE;

   keytable.key_cnt = ret;
   if (keytable.key_cnt == 0)
     {
        xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE,
                            keytable.win, _atom_grab_excl_win,
                            ECORE_X_ATOM_CARDINAL, 32, 1,
                            (unsigned char *)&keycode);
        _keytable_free(&keytable);
        return EINA_TRUE;
     }

   ret = _keytable_key_search(&keytable, keycode);
   if (ret != -1)
     {
        xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_APPEND,
                            keytable.win, _atom_grab_excl_win,
                            ECORE_X_ATOM_CARDINAL, 32, 1,
                            (unsigned char *)&keycode);
        _keytable_free(&keytable);
        return EINA_TRUE;
     }

   WRN("Key is already grabbed");
   _keytable_free(&keytable);
   return EINA_FALSE;
}

static Eina_Bool
_keytable_possible_global_exclusiveness_unset(int keycode)
{
   Ecore_X_Window_Key_Table keytable;
   int ret = 0;

   keytable.win = ecore_x_window_root_first_get();
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   if (_atom_grab_excl_win == XCB_NONE)
     _atom_grab_excl_win = ecore_x_atom_get(STR_ATOM_GRAB_EXCL_WIN);

   ret = _keytable_property_list_get(keytable.win, _atom_grab_excl_win,
                                     (unsigned int **)&(keytable.key_list));
   if (ret < 0) return EINA_FALSE;

   keytable.key_cnt = ret;

   ret = _keytable_key_search(&keytable, keycode);
   if (ret == -1)
     {
        WRN("Keygrab already exists");
        _keytable_free(&keytable);
        return EINA_FALSE;
     }
   else
     ret = _keytable_key_del(&keytable, keycode, _atom_grab_excl_win);

   _keytable_free(&keytable);
   return EINA_FALSE;
}

static Eina_Bool
_ecore_xcb_window_keygrab_set_internal(Ecore_X_Window win, const char *key, Ecore_X_Win_Keygrab_Mode grab_mode)
{
   Ecore_X_Window_Key_Table keytable;
   xcb_keycode_t keycode = 0;
   Eina_Bool ret = EINA_FALSE;

   keytable.win = win;
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   keycode = _ecore_xcb_keymap_string_to_keycode(key);
   if (keycode == XCB_NO_SYMBOL)
     {
        WRN("Keycode of key(\"%s\") does not exist", key);
        return EINA_FALSE;
     }

   if (grab_mode == ECORE_X_WIN_KEYGRAB_EXCLUSIVE)
     {
        if (!_keytable_possible_global_exclusiveness_get(keycode))
          return EINA_FALSE;
     }

   if (!_keytable_get(win, &keytable)) return EINA_FALSE;

   ret = _keytable_key_add(&keytable, keycode, grab_mode);
   if (!ret)
     {
        WRN("Key(\"%s\") add failed", key);
        goto err;
     }

   if (grab_mode == ECORE_X_WIN_KEYGRAB_EXCLUSIVE)
     {
        if (!_keytable_possible_global_exclusiveness_set(keycode))
          {
             _keytable_key_del(&keytable, keycode, ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE);
             WRN("Key(\"%s\") already is grabbed", key);
             goto err;
          }
     }

   _keytable_free(&keytable);
   return EINA_TRUE;

err:
   _keytable_free(&keytable);
   return EINA_FALSE;
}

static Eina_Bool
_ecore_xcb_window_keygrab_unset_internal(Ecore_X_Window win, const char *key)
{
   Ecore_X_Window_Key_Table keytable;
   Ecore_X_Win_Keygrab_Mode grab_mode = ECORE_X_WIN_KEYGRAB_UNKNOWN;
   xcb_keycode_t keycode = 0;
   int i, masked = 0, decoded = 0;
   Eina_Bool ret = EINA_FALSE;

   keytable.win = win;
   keytable.key_list = NULL;
   keytable.key_cnt = 0;

   keycode = _ecore_xcb_keymap_string_to_keycode(key);
   if (keycode == XCB_NO_SYMBOL)
     {
        WRN("Keycode of key(\"%s\") does not exist", key);
        return EINA_FALSE;
     }

   if (!_keytable_get(win, &keytable)) return EINA_FALSE;

   if (keytable.key_cnt <= 0) return EINA_FALSE;

   i = _keytable_key_search(&keytable, keycode);
   if (i == -1)
     {
        WRN("Key(\"%s\") does not exist", key);
        goto err;
     }

   masked = keytable.key_list[i];

   ret = _keytable_keycode_decode(masked, &decoded, &grab_mode);
   if (!ret) goto err;

   ret = _keytable_key_del(&keytable, masked, ECORE_X_ATOM_E_KEYROUTER_WINDOW_KEYTABLE);
   if (!ret) goto err;

   if (grab_mode == ECORE_X_WIN_KEYGRAB_EXCLUSIVE)
     ret = _keytable_possible_global_exclusiveness_unset(keycode);

   _keytable_free(&keytable);
   return EINA_TRUE;

err:
   _keytable_free(&keytable);
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_window_keygrab_set(Ecore_X_Window win, const char *key, int mod EINA_UNUSED, int not_mod EINA_UNUSED, int priority EINA_UNUSED, Ecore_X_Win_Keygrab_Mode grab_mode)
{
   if (_ecore_keyrouter == 0)
     {
        if (ecore_x_e_keyrouter_get(win))
          _ecore_keyrouter = 1;
        else
          {
             WRN("Keyrouter is not supported");
             _ecore_keyrouter = -1;
          }
     }

   if (_ecore_keyrouter < 0) return EINA_FALSE;

   return _ecore_xcb_window_keygrab_set_internal(win, key, grab_mode);
}

EAPI Eina_Bool
ecore_x_window_keygrab_unset(Ecore_X_Window win, const char *key, int mod EINA_UNUSED, int any_mod EINA_UNUSED)
{
   if (_ecore_keyrouter != 1)
     {
        WRN("Keyrouter is not supported");
        return EINA_FALSE;
     }

   return _ecore_xcb_window_keygrab_unset_internal(win, key);
}
