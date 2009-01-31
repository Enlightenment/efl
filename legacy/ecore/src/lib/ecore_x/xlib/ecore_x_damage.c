/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "ecore_x_private.h"
#include "Ecore_X.h"

static int _damage_available;
#ifdef ECORE_XDAMAGE
static int _damage_major, _damage_minor;
#endif

void
_ecore_x_damage_init(void)
{
#ifdef ECORE_XDAMAGE
   _damage_major = 1;
   _damage_minor = 0;

   if (XDamageQueryVersion(_ecore_x_disp, &_damage_major, &_damage_minor))
     _damage_available = 1;
   else
     _damage_available = 0;
#else
   _damage_available = 0;
#endif
}

EAPI int
ecore_x_damage_query(void)
{
   return _damage_available;
}

EAPI Ecore_X_Damage
ecore_x_damage_new(Ecore_X_Drawable d, Ecore_X_Damage_Report_Level level)
{
#ifdef ECORE_XDAMAGE
   Ecore_X_Damage damage;

   damage = XDamageCreate(_ecore_x_disp, d, level);
   return damage;
#else
   return 0;
#endif
}

EAPI void
ecore_x_damage_del(Ecore_X_Damage damage)
{
#ifdef ECORE_XDAMAGE
   XDamageDestroy(_ecore_x_disp, damage);
#endif
}

EAPI void
ecore_x_damage_subtract(Ecore_X_Damage damage, Ecore_X_Region repair, Ecore_X_Region parts)
{
#ifdef ECORE_XDAMAGE
   XDamageSubtract(_ecore_x_disp, damage, repair, parts);
#endif
}

