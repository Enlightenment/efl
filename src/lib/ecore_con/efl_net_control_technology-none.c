#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct
{

} Efl_Net_Control_Technology_Data;

EOLIAN static void
_efl_net_control_technology_efl_object_destructor(Eo *obj, Efl_Net_Control_Technology_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, EFL_NET_CONTROL_TECHNOLOGY_CLASS));
}

EOLIAN static void
_efl_net_control_technology_powered_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Technology_Data *pd EINA_UNUSED, Eina_Bool powered EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_efl_net_control_technology_powered_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Technology_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_efl_net_control_technology_tethering_set(Eo *obj EINA_UNUSED, Efl_Net_Control_Technology_Data *pd EINA_UNUSED, Eina_Bool enabled EINA_UNUSED, const char *identifier EINA_UNUSED, const char *passphrase EINA_UNUSED)
{
}

EOLIAN static void
_efl_net_control_technology_tethering_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Technology_Data *pd EINA_UNUSED, Eina_Bool *enabled EINA_UNUSED, const char **identifier EINA_UNUSED, const char **passphrase EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_efl_net_control_technology_connected_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Technology_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static const char *
_efl_net_control_technology_efl_object_name_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Technology_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Efl_Net_Control_Technology_Type
_efl_net_control_technology_type_get(const Eo *obj EINA_UNUSED, Efl_Net_Control_Technology_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN static Eina_Future *
_efl_net_control_technology_scan(Eo *obj, Efl_Net_Control_Technology_Data *pd EINA_UNUSED)
{
   return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                               EINA_ERROR_NOT_IMPLEMENTED);
}

#include "efl_net_control_technology.eo.c"
