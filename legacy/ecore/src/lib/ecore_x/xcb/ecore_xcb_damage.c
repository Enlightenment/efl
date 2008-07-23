/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_Damage_Group X Damage Extension Functions
 *
 * Functions related to the X Damage extension.
 */


#ifdef ECORE_XCB_DAMAGE
static uint8_t _damage_available = 0;
static xcb_damage_query_version_cookie_t _ecore_xcb_damage_init_cookie;
#endif /* ECORE_XCB_DAMAGE */


/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_damage_init and
   _ecore_xcb_damage_init_finalize. The first one gets the cookies and
   the second one gets the replies. */

void
_ecore_x_damage_init(const xcb_query_extension_reply_t *reply)
{
#ifdef ECORE_XCB_DAMAGE
   if (reply && (reply->present))
      _ecore_xcb_damage_init_cookie = xcb_damage_query_version_unchecked(_ecore_xcb_conn, 1, 1);
#endif /* ECORE_XCB_DAMAGE */
}

void
_ecore_x_damage_init_finalize(void)
{
#ifdef ECORE_XCB_DAMAGE
   xcb_damage_query_version_reply_t *reply;

   reply = xcb_damage_query_version_reply(_ecore_xcb_conn,
                                          _ecore_xcb_damage_init_cookie,
                                          NULL);
   if (reply)
     {
        if (reply->major_version >= 1)
          _damage_available = 1;
        free(reply);
     }
#endif /* ECORE_XCB_DAMAGE */
}


/**
 * Return whether the Damage Extension is available.
 * @return 1 if the Damage Extension is available, 0 if not.
 *
 * Return 1 if the X server supports the Damage Extension version 1.0,
 * 0 otherwise.
 * @ingroup Ecore_X_Damage_Group
 */
EAPI int
ecore_x_damage_query(void)
{
#ifdef ECORE_XCB_DAMAGE
   return _damage_available;
#else
   return 0;
#endif /* ECORE_XCB_DAMAGE */
}


/**
 * Creates a damage object.
 * @param drawable The drawable to monotor.
 * @param level    The level of the damage report.
 * @return         The damage object.
 *
 * Creates a damage object to monitor changes to @p drawable, with the
 * level @p level.
 * @ingroup Ecore_X_Damage_Group
 */
EAPI Ecore_X_Damage
ecore_x_damage_new(Ecore_X_Drawable            drawable,
                   Ecore_X_Damage_Report_Level level)
{
   Ecore_X_Damage damage = 0;

#ifdef ECORE_XCB_DAMAGE
   damage = xcb_generate_id(_ecore_xcb_conn);
   xcb_damage_create(_ecore_xcb_conn, damage, drawable, level);
#endif /* ECORE_XCB_DAMAGE */

   return damage;
}


/**
 * Destroys a damage object.
 * @param damage The damage object to destroy.
 *
 * Destroys the damage object @p damage.
 * @ingroup Ecore_X_Damage_Group
 */
EAPI void
ecore_x_damage_del(Ecore_X_Damage damage)
{
#ifdef ECORE_XCB_DAMAGE
   xcb_damage_destroy(_ecore_xcb_conn, damage);
#endif /* ECORE_XCB_DAMAGE */
}


/**
 * Synchronously modifies the region.
 * @param damage The damage object to destroy.
 * @param repair The repair region.
 * @param parts  The parts region.
 *
 * Synchronously modifies the regions in the following manner:
 * If @p repair is @c XCB_NONE:
 *   1) parts = damage
 *   2) damage = <empty>
 * Otherwise:
 *   1) parts = damage INTERSECT repair
 *   2) damage = damage - parts
 *   3) Generate DamageNotify for remaining damage areas
 * @ingroup Ecore_X_Damage_Group
 */
EAPI void
ecore_x_damage_subtract(Ecore_X_Damage damage,
                        Ecore_X_Region repair,
                        Ecore_X_Region parts)
{
#ifdef ECORE_XCB_DAMAGE
   xcb_damage_subtract(_ecore_xcb_conn, damage, repair, parts);
#endif /* ECORE_XCB_DAMAGE */
}
