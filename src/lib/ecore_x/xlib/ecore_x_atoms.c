#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <string.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"
#include "ecore_x_atoms_decl.h"

void
_ecore_x_atoms_init(void)
{
   Atom *atoms;
   char **names;
   int i, num;

   num = sizeof(atom_items) / sizeof(Atom_Item);
   atoms = alloca(num * sizeof(Atom));
   names = alloca(num * sizeof(char *));
   for (i = 0; i < num; i++)
     names[i] = (char *) atom_items[i].name;
   XInternAtoms(_ecore_x_disp, names, num, False, atoms);
   for (i = 0; i < num; i++)
     *(atom_items[i].atom) = atoms[i];
}

/**
 * Retrieves the atom value associated with the given name.
 * @param  name The given name.
 * @return Associated atom value.
 */
EAPI Ecore_X_Atom
ecore_x_atom_get(const char *name)
{
   Ecore_X_Atom atom;
   if (!_ecore_x_disp)
     return 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   atom = XInternAtom(_ecore_x_disp, name, False);
   if (_ecore_xlib_sync) ecore_x_sync();
   return atom;
}

EAPI void
ecore_x_atoms_get(const char **names,
                  int num,
                  Ecore_X_Atom *atoms)
{
   Atom *atoms_int;
   int i;

   if (!_ecore_x_disp)
     return;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   atoms_int = alloca(num * sizeof(Atom));
   XInternAtoms(_ecore_x_disp, (char **)names, num, False, atoms_int);
   for (i = 0; i < num; i++)
     atoms[i] = atoms_int[i];
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI char *
ecore_x_atom_name_get(Ecore_X_Atom atom)
{
   char *name;
   char *xname;

   if (!_ecore_x_disp)
     return NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xname = XGetAtomName(_ecore_x_disp, atom);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!xname)
     return NULL;

   name = strdup(xname);
   XFree(xname);

   return name;
}

