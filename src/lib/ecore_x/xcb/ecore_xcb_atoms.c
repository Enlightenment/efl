#include "ecore_xcb_private.h"
#include "ecore_x_atoms_decl.h"

/* NB: Increment if you add new atoms */
#define ECORE_X_ATOMS_COUNT 199

/* local function prototypes */

/* local variables */
static xcb_intern_atom_cookie_t cookies[ECORE_X_ATOMS_COUNT];

#define MYMIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void
_ecore_xcb_atoms_init(void)
{
   int i = 0, num = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   num = (sizeof(atom_items) / sizeof(Atom_Item));
   num = MYMIN(num, ECORE_X_ATOMS_COUNT);
   for (i = 0; i < num; i++)
     {
        cookies[i] =
          xcb_intern_atom_unchecked(_ecore_xcb_conn, 0,
                                    strlen(atom_items[i].name), atom_items[i].name);
     }
}

void
_ecore_xcb_atoms_finalize(void)
{
   int i = 0, num = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   num = (sizeof(atom_items) / sizeof(Atom_Item));
   num = MYMIN(num, ECORE_X_ATOMS_COUNT);
   for (i = 0; i < num; i++)
     {
        xcb_intern_atom_reply_t *reply = NULL;

        if (!(reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookies[i], 0)))
          continue;
        *(atom_items[i].atom) = reply->atom;
        free(reply);
     }
}

/**
 * @defgroup Ecore_X_Atom_Group X Atom Functions
 * @ingroup Ecore_X_Group
 *
 * Functions that operate on atoms
 */

/**
 * Retrieves the atom value associated to a name.
 *
 * @param  name Unused.
 * @return      Associated atom value.
 *
 * Retrieves the atom value associated to a name. The reply is the
 * returned value of the function ecore_xcb_intern_atom_reply(). If
 * @p reply is @c NULL, the NULL atom is returned. Otherwise, the atom
 * associated to the name is returned.
 *
 * @ingroup Ecore_X_Atom_Group
 */
EAPI Ecore_X_Atom
ecore_x_atom_get(const char *name)
{
   xcb_intern_atom_cookie_t cookie;
   xcb_intern_atom_reply_t *reply;
   Ecore_X_Atom a;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   cookie = xcb_intern_atom_unchecked(_ecore_xcb_conn, 0, strlen(name), name);
   reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return XCB_ATOM_NONE;
   a = reply->atom;
   free(reply);
   return a;
}

/**
 * Retrieves the name of the given atom.
 *
 * @param  atom
 * @return      The name of the atom.
 *
 * @ingroup Ecore_X_Atom_Group
 */
EAPI char *
ecore_x_atom_name_get(Ecore_X_Atom atom)
{
   xcb_get_atom_name_cookie_t cookie;
   xcb_get_atom_name_reply_t *reply;
   char *name;
   int len = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   cookie = xcb_get_atom_name_unchecked(_ecore_xcb_conn, atom);
   reply = xcb_get_atom_name_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return NULL;
   len = xcb_get_atom_name_name_length(reply);
   name = (char *)malloc(sizeof(char) * (len + 1));
   if (!name)
     {
        free(reply);
        return NULL;
     }
   memcpy(name, xcb_get_atom_name_name(reply), len);
   name[len] = '\0';

   free(reply);
   return name;
}

EAPI void
ecore_x_atoms_get(const char  **names,
                  int           num,
                  Ecore_X_Atom *atoms)
{
   xcb_intern_atom_cookie_t cookies[num];
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   num = MYMIN(num, ECORE_X_ATOMS_COUNT);
   for (i = 0; i < num; i++)
     {
        cookies[i] =
          xcb_intern_atom_unchecked(_ecore_xcb_conn, 0,
                                    strlen(names[i]), names[i]);
     }
   for (i = 0; i < num; i++)
     {
        xcb_intern_atom_reply_t *reply = NULL;

        if (!(reply = xcb_intern_atom_reply(_ecore_xcb_conn, cookies[i], 0)))
          continue;
        atoms[i] = reply->atom;
        free(reply);
     }
}

