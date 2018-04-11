#include "edje_private.h"

static Edje_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Edje_Version * edje_version = &_version;

static int _edje_init_count = 0;
static Eina_Bool _need_imf = EINA_FALSE;

int _edje_default_log_dom = -1;
Eina_Mempool *_edje_real_part_mp = NULL;
Eina_Mempool *_edje_real_part_state_mp = NULL;

Eina_Cow *_edje_calc_params_map_cow = NULL;
Eina_Cow *_edje_calc_params_physics_cow = NULL;

Edje_Global *_edje_global_obj = NULL;

static const Edje_Calc_Params_Map default_calc_map = {
   { 0, 0, 0 }, { 0.0, 0.0, 0.0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0.0, 0.0 }, NULL, 0
};

static const Edje_Calc_Params_Physics default_calc_physics = {
   0.0, 0.0, 0.0, 0.0, 0.0, { 0.0, 0.0 }, { 0.0, 0.0 }, 0, 0, { { 0, 0, 0 }, { 0, 0, 0 } }, 0, 0, 0, 0
};

#ifdef HAVE_EPHYSICS
static void _edje_ephysics_clear(void);
#endif


/*============================================================================*
*                                   API                                      *
*============================================================================*/
EAPI int
edje_init(void)
{
   Eina_Strbuf *str;

   if (++_edje_init_count != 1)
     return _edje_init_count;

   srand(time(NULL));

   if (!eina_init())
     return --_edje_init_count;

   _edje_default_log_dom = eina_log_domain_register
       ("edje", EDJE_DEFAULT_LOG_COLOR);
   if (_edje_default_log_dom < 0)
     {
        EINA_LOG_ERR("Edje Can not create a general log domain.");
        goto shutdown_eina;
     }

   if (!ecore_init())
     {
        ERR("Ecore init failed");
        goto unregister_log_domain;
     }

   if (!embryo_init())
     {
        ERR("Embryo init failed");
        goto shutdown_ecore;
     }

   if (!eet_init())
     {
        ERR("Eet init failed");
        goto shutdown_embryo;
     }

   if (!evas_init())
     {
        ERR("Evas init failed");
        goto shutdown_eet;
     }

   if (!efreet_init())
     {
        ERR("Efreet init failed");
        goto shutdown_evas;
     }

   _edje_scale = FROM_DOUBLE(1.0);
   _edje_global_obj = efl_add(EDJE_GLOBAL_CLASS, efl_main_loop_get());
   if (!_edje_global_obj ||
       !efl_loop_register(efl_main_loop_get(), EFL_GFX_COLOR_CLASS_INTERFACE, _edje_global_obj) ||
       !efl_loop_register(efl_main_loop_get(), EFL_GFX_TEXT_CLASS_INTERFACE, _edje_global_obj) ||
       !efl_loop_register(efl_main_loop_get(), EFL_GFX_SIZE_CLASS_INTERFACE, _edje_global_obj))
     goto shutdown_efreet;

   _edje_edd_init();
   _edje_text_init();
   _edje_box_init();
   _edje_external_init();
   _edje_module_init();
   _edje_message_init();
   _edje_multisense_init();
   edje_signal_init();
   _edje_class_init();

   _edje_real_part_mp = eina_mempool_add("chained_mempool",
                                         "Edje_Real_Part", NULL,
                                         sizeof (Edje_Real_Part), 256);
   if (!_edje_real_part_mp)
     {
        ERR("Mempool for Edje_Real_Part cannot be allocated.");
        goto shutdown_all;
     }

   _edje_real_part_state_mp = eina_mempool_add("chained_mempool",
                                               "Edje_Real_Part_State", NULL,
                                               sizeof (Edje_Real_Part_State), 64);
   if (!_edje_real_part_state_mp)
     {
        ERR("Mempool for Edje_Real_Part_State cannot be allocated.");
        goto shutdown_all;
     }

   _edje_calc_params_map_cow = eina_cow_add("Edje Calc Params Map", sizeof (Edje_Calc_Params_Map), 8, &default_calc_map, EINA_TRUE);
   _edje_calc_params_physics_cow = eina_cow_add("Edje Calc Params Physics", sizeof (Edje_Calc_Params_Physics), 8, &default_calc_physics, EINA_TRUE);

   _edje_language = eina_stringshare_add(getenv("LANGUAGE"));

   str = eina_strbuf_new();
   eina_strbuf_append_printf(str, "%s/edje", efreet_cache_home_get());
   _edje_cache_path = eina_stringshare_add(eina_strbuf_string_get(str));
   eina_strbuf_free(str);

   eina_log_timing(_edje_default_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   return _edje_init_count;

shutdown_all:
   eina_mempool_del(_edje_real_part_state_mp);
   eina_mempool_del(_edje_real_part_mp);
   _edje_real_part_state_mp = NULL;
   _edje_real_part_mp = NULL;
   _edje_class_shutdown();
   _edje_message_shutdown();
   _edje_module_shutdown();
   _edje_external_shutdown();
   _edje_box_shutdown();
   _edje_internal_proxy_shutdown();
   _edje_text_class_hash_free();
   _edje_size_class_hash_free();
   _edje_edd_shutdown();
   if (_edje_global_obj)
     {
        efl_loop_unregister(efl_main_loop_get(), EFL_GFX_COLOR_CLASS_INTERFACE, _edje_global_obj);
        efl_loop_unregister(efl_main_loop_get(), EFL_GFX_TEXT_CLASS_INTERFACE, _edje_global_obj);
        efl_loop_unregister(efl_main_loop_get(), EFL_GFX_SIZE_CLASS_INTERFACE, _edje_global_obj);
        efl_unref(_edje_global_obj);
        _edje_global_obj = NULL;
     }
shutdown_efreet:
   efreet_shutdown();
shutdown_evas:
   evas_shutdown();
shutdown_eet:
   eet_shutdown();
shutdown_embryo:
   embryo_shutdown();
shutdown_ecore:
   ecore_shutdown();
unregister_log_domain:
   eina_log_domain_unregister(_edje_default_log_dom);
   _edje_default_log_dom = -1;
shutdown_eina:
   eina_shutdown();
   return --_edje_init_count;
}

static int _edje_users = 0;

static void
_edje_shutdown_core(void)
{
   if (_edje_users > 0) return;

   eina_log_timing(_edje_default_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   _edje_file_cache_shutdown();
   _edje_color_class_hash_free();

   eina_stringshare_del(_edje_cache_path);
   eina_stringshare_del(_edje_language);
   _edje_cache_path = NULL;
   _edje_language = NULL;

   eina_mempool_del(_edje_real_part_state_mp);
   eina_mempool_del(_edje_real_part_mp);
   _edje_real_part_state_mp = NULL;
   _edje_real_part_mp = NULL;

   _edje_class_shutdown();
   edje_signal_shutdown();
   _edje_multisense_shutdown();
   _edje_message_shutdown();
   _edje_module_shutdown();
   _edje_external_shutdown();
   _edje_internal_proxy_shutdown();
   _edje_box_shutdown();
   _edje_text_class_hash_free();
   _edje_size_class_hash_free();
   _edje_edd_shutdown();
   efl_loop_unregister(efl_main_loop_get(), EFL_GFX_COLOR_CLASS_INTERFACE, _edje_global_obj);
   efl_loop_unregister(efl_main_loop_get(), EFL_GFX_TEXT_CLASS_INTERFACE, _edje_global_obj);
   efl_loop_unregister(efl_main_loop_get(), EFL_GFX_SIZE_CLASS_INTERFACE, _edje_global_obj);
   efl_unref(_edje_global_obj);
   _edje_global_obj = NULL;

   eina_cow_del(_edje_calc_params_map_cow);
   eina_cow_del(_edje_calc_params_physics_cow);
   _edje_calc_params_map_cow = NULL;
   _edje_calc_params_physics_cow = NULL;

#ifdef HAVE_ECORE_IMF
   if (_need_imf)
     ecore_imf_shutdown();
#endif

#ifdef HAVE_EPHYSICS
   _edje_ephysics_clear();
#endif

   efreet_shutdown();
   ecore_shutdown();
   evas_shutdown();
   eet_shutdown();
   embryo_shutdown();
   eina_log_domain_unregister(_edje_default_log_dom);
   _edje_default_log_dom = -1;
   eina_shutdown();
}

void
_edje_lib_ref(void)
{
   _edje_users++;
}

void
_edje_lib_unref(void)
{
   _edje_users--;
   if (_edje_users != 0) return;
   if (_edje_init_count == 0) _edje_shutdown_core();
}

EAPI int
edje_shutdown(void)
{
   if (_edje_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   if (--_edje_init_count != 0)
     return _edje_init_count;

   _edje_shutdown_core();

   return _edje_init_count;
}

/* Private Routines */
void
_edje_class_init(void)
{
   if (!_edje_color_class_member)
     _edje_color_class_member = efl_add(EFL_OBSERVABLE_CLASS, efl_main_loop_get());
   if (!_edje_text_class_member)
     _edje_text_class_member = efl_add(EFL_OBSERVABLE_CLASS, efl_main_loop_get());
   if (!_edje_size_class_member)
     _edje_size_class_member = efl_add(EFL_OBSERVABLE_CLASS, efl_main_loop_get());
}

void
_edje_class_shutdown(void)
{
   if (_edje_color_class_member)
     {
        efl_unref(_edje_color_class_member);
        _edje_color_class_member = NULL;
     }
   if (_edje_text_class_member)
     {
        efl_unref(_edje_text_class_member);
        _edje_text_class_member = NULL;
     }
   if (_edje_size_class_member)
     {
        efl_unref(_edje_size_class_member);
        _edje_size_class_member = NULL;
     }
}

void
_edje_del(Edje *ed)
{
   Edje_Text_Insert_Filter_Callback *cb;

   if (ed->processing_messages)
     {
        ed->delete_me = EINA_TRUE;
        return;
     }
   _edje_message_del(ed);
   _edje_signal_callback_free(ed->callbacks);
   ed->callbacks = NULL;
   _edje_file_del(ed);
   if (ed->path) eina_stringshare_del(ed->path);
   if (ed->group) eina_stringshare_del(ed->group);
   if (ed->parent) eina_stringshare_del(ed->parent);
   ed->path = NULL;
   ed->group = NULL;
   ed->parent = NULL;
   eina_hash_free(ed->color_classes);
   eina_hash_free(ed->text_classes);
   eina_hash_free(ed->size_classes);
   ed->color_classes = NULL;
   ed->text_classes = NULL;
   ed->size_classes = NULL;
   EINA_LIST_FREE(ed->text_insert_filter_callbacks, cb)
     {
        eina_stringshare_del(cb->part);
        free(cb);
     }
   EINA_LIST_FREE(ed->markup_filter_callbacks, cb)
     {
        eina_stringshare_del(cb->part);
        free(cb);
     }

   efl_observable_observer_clean(_edje_color_class_member, ed->obj);
   efl_observable_observer_clean(_edje_text_class_member, ed->obj);
   efl_observable_observer_clean(_edje_size_class_member, ed->obj);
}

void
_edje_ref(Edje *ed)
{
   if (ed->references <= 0) return;
   ed->references++;
}

void
_edje_unref(Edje *ed)
{
   ed->references--;
   if (ed->references == 0) _edje_del(ed);
}

void
_edje_need_imf(void)
{
   if (_need_imf) return;
#ifdef HAVE_ECORE_IMF
   _need_imf = EINA_TRUE;
   ecore_imf_init();
#endif
}

#ifdef HAVE_EPHYSICS
Edje_Ephysics *_edje_ephysics = NULL;

Eina_Bool
_edje_ephysics_load(void)
{
   if (_edje_ephysics)
     {
        if (!_edje_ephysics->mod)
          {
             ERR("Cannot find libephysics at runtime!");
             return EINA_FALSE;
          }
        return EINA_TRUE;
     }
   _edje_ephysics = calloc(1, sizeof(Edje_Ephysics));
   if (!_edje_ephysics) return EINA_FALSE;
# define LOAD(x)                                        \
   if (!_edje_ephysics->mod) {                          \
      if ((_edje_ephysics->mod = eina_module_new(x))) { \
         if (!eina_module_load(_edje_ephysics->mod)) {  \
            eina_module_free(_edje_ephysics->mod);      \
            _edje_ephysics->mod = NULL;                 \
         }                                              \
      }                                                 \
   }
# if defined(_WIN32) || defined(__CYGWIN__)
   LOAD("libephysics-1.dll");
   LOAD("libephysics1.dll");
   LOAD("libephysics.dll");
   if (!_edje_ephysics->mod)
     ERR("Could not find libephysics-1.dll, libephysics1.dll, libephysics.dll");
# elif defined(__APPLE__) && defined(__MACH__)
   LOAD("libephysics.1.dylib");
   LOAD("libephysics.1.so");
   LOAD("libephysics.so.1");
   if (!_edje_ephysics->mod)
     ERR("Could not find libephysics.1.dylib, libephysics.1.so, libephysics.so.1");
# else
   LOAD("libephysics.so.1");
   if (!_edje_ephysics->mod)
     ERR("Could not find libephysics.so.1");
# endif
# undef LOAD
   if (!_edje_ephysics->mod) return EINA_FALSE;
# define SYM(x) \
   if (!(_edje_ephysics->x = eina_module_symbol_get(_edje_ephysics->mod, #x))) { \
      ERR("Cannot find symbol '%s' in'%s", #x, eina_module_file_get(_edje_ephysics->mod)); \
      goto err; \
   }
   SYM(ephysics_init);
   SYM(ephysics_shutdown);
   SYM(ephysics_world_new);
   SYM(ephysics_world_del);
   SYM(ephysics_world_event_callback_add)
   SYM(ephysics_world_rate_set)
   SYM(ephysics_world_gravity_set)
   SYM(ephysics_world_render_geometry_set);
   SYM(ephysics_world_render_geometry_get);
   SYM(ephysics_quaternion_set)
   SYM(ephysics_quaternion_get)
   SYM(ephysics_quaternion_normalize)
   SYM(ephysics_body_box_add)
   SYM(ephysics_body_sphere_add)
   SYM(ephysics_body_cylinder_add)
   SYM(ephysics_body_soft_box_add)
   SYM(ephysics_body_soft_sphere_add)
   SYM(ephysics_body_soft_cylinder_add)
   SYM(ephysics_body_cloth_add)
   SYM(ephysics_body_top_boundary_add)
   SYM(ephysics_body_bottom_boundary_add)
   SYM(ephysics_body_right_boundary_add)
   SYM(ephysics_body_left_boundary_add)
   SYM(ephysics_body_front_boundary_add)
   SYM(ephysics_body_back_boundary_add)
   SYM(ephysics_body_central_impulse_apply)
   SYM(ephysics_body_torque_impulse_apply)
   SYM(ephysics_body_central_force_apply)
   SYM(ephysics_body_torque_apply)
   SYM(ephysics_body_forces_clear)
   SYM(ephysics_body_linear_velocity_set)
   SYM(ephysics_body_angular_velocity_set)
   SYM(ephysics_body_stop)
   SYM(ephysics_body_rotation_set)
   SYM(ephysics_body_forces_get)
   SYM(ephysics_body_torques_get)
   SYM(ephysics_body_linear_velocity_get)
   SYM(ephysics_body_angular_velocity_get)
   SYM(ephysics_body_linear_movement_enable_set)
   SYM(ephysics_body_angular_movement_enable_set)
   SYM(ephysics_body_move)
   SYM(ephysics_body_geometry_get)
   SYM(ephysics_body_resize)
   SYM(ephysics_body_material_set)
   SYM(ephysics_body_density_set)
   SYM(ephysics_body_mass_set)
   SYM(ephysics_body_soft_body_hardness_set)
   SYM(ephysics_body_restitution_set)
   SYM(ephysics_body_friction_set)
   SYM(ephysics_body_damping_set)
   SYM(ephysics_body_sleeping_threshold_set)
   SYM(ephysics_body_light_set)
   SYM(ephysics_body_back_face_culling_set)
   SYM(ephysics_body_evas_object_update)
   SYM(ephysics_body_face_evas_object_set)
   SYM(ephysics_body_evas_object_set)
   SYM(ephysics_body_event_callback_add)
   SYM(ephysics_body_data_set)
   SYM(ephysics_body_data_get)
   SYM(ephysics_body_rotation_get)
#undef SYM
   return EINA_TRUE;
err:
   if (_edje_ephysics->mod)
     {
        eina_module_free(_edje_ephysics->mod);
        _edje_ephysics->mod = NULL;
     }
   return EINA_FALSE;
}

static void
_edje_ephysics_clear(void)
{
   if (_edje_ephysics)
     {
        if (_edje_ephysics->mod)
          eina_module_free(_edje_ephysics->mod);
        free(_edje_ephysics);
        _edje_ephysics = NULL;
     }
}
#endif

