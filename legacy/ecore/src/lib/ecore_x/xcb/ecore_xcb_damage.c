#include "ecore_xcb_private.h"
# ifdef ECORE_XCB_DAMAGE
#  include <xcb/damage.h>
# endif

/* local variables */
static Eina_Bool _damage_avail = EINA_FALSE;

/* external variables */
int _ecore_xcb_event_damage = -1;

void 
_ecore_xcb_damage_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DAMAGE
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_damage_id);
#endif
}

void 
_ecore_xcb_damage_finalize(void) 
{
#ifdef ECORE_XCB_DAMAGE
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DAMAGE
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_damage_id);
   if ((ext_reply) && (ext_reply->present)) 
     {
        xcb_damage_query_version_cookie_t cookie;
        xcb_damage_query_version_reply_t *reply;

        cookie = 
          xcb_damage_query_version_unchecked(_ecore_xcb_conn, 
                                             XCB_DAMAGE_MAJOR_VERSION, 
                                             XCB_DAMAGE_MINOR_VERSION);
        reply = xcb_damage_query_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply)
          {
             _damage_avail = EINA_TRUE;
             free(reply);
          }

        if (_damage_avail)
          _ecore_xcb_event_damage = ext_reply->first_event;
     }
#endif
}

/**
 * @defgroup Ecore_X_Damage_Group X Damage Extension Functions
 * 
 * Functions related to the X Damage Extension.
 */

EAPI Eina_Bool 
ecore_x_damage_query(void) 
{
   return _damage_avail;
}

/**
 * Create a damage object
 * 
 * @param drawable The drawable to monitor
 * @param level The level of the damage report
 * @return The damage object
 * 
 * Creates a damage object to monitor changes to @p drawable, 
 * with the level @p level.
 * 
 * @ingroup Ecore_X_Damage_Group
 */
EAPI Ecore_X_Damage 
ecore_x_damage_new(Ecore_X_Drawable drawable, Ecore_X_Damage_Report_Level level) 
{
   Ecore_X_Damage damage = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DAMAGE
   damage = xcb_generate_id(_ecore_xcb_conn);
   xcb_damage_create(_ecore_xcb_conn, damage, drawable, level);
   ecore_x_flush();
#endif

   return damage;
}

/**
 * Destroy a damage object
 * 
 * @param The damage object to destroy
 * 
 * Destroys the damage object @p damage
 * 
 * @ingroup Ecore_X_Damage_Group
 */
EAPI void 
ecore_x_damage_free(Ecore_X_Damage damage) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DAMAGE
   xcb_damage_destroy(_ecore_xcb_conn, damage);
   ecore_x_flush();
#endif
}

/**
 * Synchronously modifies the region
 * 
 * @param damage The damage object to destroy
 * @param repair The repair region
 * @param parts The parts region
 *
 * Synchronously modifies the regions in the following manner:
 * If @p repair is @c XCB_NONE:
 *   1) parts = damage
 *   2) damage = <empty>
 * Otherwise:
 *   1) parts = damage INTERSECT repair
 *   2) damage = damage - parts
 *   3) Generate DamageNotify for remaining damage areas
 * 
 * @ingroup Ecore_X_Damage_Group
 */
EAPI void 
ecore_x_damage_subtract(Ecore_X_Damage damage, Ecore_X_Region repair, Ecore_X_Region parts) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DAMAGE
   xcb_damage_subtract(_ecore_xcb_conn, damage, repair, parts);
   ecore_x_flush();
#endif
}
