#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"
#include "Ecore_X.h"

static Eina_Bool _damage_available = EINA_FALSE;
#ifdef ECORE_XDAMAGE
static int _damage_major, _damage_minor;
#endif /* ifdef ECORE_XDAMAGE */

void
_ecore_x_damage_init(void)
{
#ifdef ECORE_XDAMAGE
   _damage_major = 1;
   _damage_minor = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (XDamageQueryVersion(_ecore_x_disp, &_damage_major, &_damage_minor))
     _damage_available = EINA_TRUE;
   else
     _damage_available = EINA_FALSE;

#else /* ifdef ECORE_XDAMAGE */
   _damage_available = EINA_FALSE;
#endif /* ifdef ECORE_XDAMAGE */
} /* _ecore_x_damage_init */

EAPI Eina_Bool
ecore_x_damage_query(void)
{
   return _damage_available;
} /* ecore_x_damage_query */

EAPI Ecore_X_Damage
ecore_x_damage_new(Ecore_X_Drawable            d,
                   Ecore_X_Damage_Report_Level level)
{
#ifdef ECORE_XDAMAGE
   Ecore_X_Damage damage;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   damage = XDamageCreate(_ecore_x_disp, d, level);
   return damage;
#else /* ifdef ECORE_XDAMAGE */
   return 0;
#endif /* ifdef ECORE_XDAMAGE */
} /* ecore_x_damage_new */

EAPI void
ecore_x_damage_free(Ecore_X_Damage damage)
{
#ifdef ECORE_XDAMAGE
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XDamageDestroy(_ecore_x_disp, damage);
#endif /* ifdef ECORE_XDAMAGE */
} /* ecore_x_damage_free */

EAPI void
ecore_x_damage_subtract(Ecore_X_Damage damage,
                        Ecore_X_Region repair,
                        Ecore_X_Region parts)
{
#ifdef ECORE_XDAMAGE
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XDamageSubtract(_ecore_x_disp, damage, repair, parts);
#endif /* ifdef ECORE_XDAMAGE */
} /* ecore_x_damage_subtract */

