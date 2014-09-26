
#ifndef EFL_CXX_DETAIL_EO_OPS_HH
#define EFL_CXX_DETAIL_EO_OPS_HH

extern "C"
{
#include <Eo.h>
}

namespace efl { namespace eo { namespace detail {

inline void
init()
{
   ::eo_init();
}

inline void
shutdown()
{
   ::eo_shutdown();
}

inline Eo*
ref(const Eo *obj)
{
   return ::eo_ref(obj);
}

inline void
unref(const Eo *obj)
{
   ::eo_unref(obj);
}

inline int
ref_get(const Eo *obj)
{
   return ::eo_ref_get(obj);
}

inline void
del(const Eo *obj)
{
   ::eo_del(obj);
}

inline Eina_Bool
isa(const Eo *obj, const Eo_Class *klass)
{
   return eo_isa(obj, klass);
}

inline Eo*
add(const Eo_Class *klass, Eo *parent = NULL)
{
   Eo *eo = eo_add_ref(klass, parent);
   return eo;
}

inline void
dbg_info_get(const Eo *obj, Eo_Dbg_Info *info)
{
   eo_do(obj, eo_dbg_info_get(info));
}

inline void
base_data_set(const Eo *obj, const char *key, const void *data,
              eo_key_data_free_func free_func = NULL)
{
   eo_do(obj, eo_key_data_set(key, data, free_func));
}

inline void*
base_data_get(const Eo *obj, const char *key)
{
   void *data;
   eo_do(obj, data = eo_key_data_get(key));
   return data;
}

inline void
base_data_del(const Eo *obj, const char *key)
{
   eo_do(obj, eo_key_data_del(key));
}

inline void
parent_set(const Eo *obj, Eo *parent)
{
   eo_do(obj, eo_parent_set(parent));
}

inline Eo*
parent_get(const Eo *obj)
{
   Eo *parent;
   eo_do(obj, parent = eo_parent_get());
   return parent;
}

inline void
event_freeze(const Eo *obj)
{
   eo_do(obj, eo_event_freeze());
}

inline void
event_thaw(const Eo *obj)
{
   eo_do(obj, eo_event_thaw());
}

inline int
event_freeze_get(const Eo *obj)
{
   int count = -1;
   eo_do(obj, count = eo_event_freeze_count_get());
   return count;
}

inline void
wref_add(const Eo *obj, Eo **wref)
{
   eo_do(obj, eo_wref_add(wref));
}

inline void
wref_del(const Eo *obj, Eo **wref)
{
   eo_do(obj, eo_wref_del(wref));
}

} } }


#endif // EFL_CXX_DETAIL_EO_OPS_HH
