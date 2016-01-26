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

static const Edje_Calc_Params_Map default_calc_map = {
   { 0, 0, 0 }, { 0.0, 0.0, 0.0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0.0, 0.0 }, NULL, 0
};

static const Edje_Calc_Params_Physics default_calc_physics = {
   0.0, 0.0, 0.0, 0.0, 0.0, { 0.0, 0.0 }, { 0.0, 0.0 }, 0, 0, { { 0, 0, 0 }, { 0, 0, 0 } }, 0, 0, 0, 0
};

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

   _edje_edd_init();
   _edje_text_init();
   _edje_box_init();
   _edje_external_init();
   _edje_module_init();
   _edje_message_init();
   _edje_multisense_init();
   edje_signal_init();

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
   _edje_message_shutdown();
   _edje_module_shutdown();
   _edje_external_shutdown();
   _edje_box_shutdown();
   _edje_text_class_members_free();
   _edje_text_class_hash_free();
   _edje_size_class_members_free();
   _edje_size_class_hash_free();
   _edje_edd_shutdown();
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
   _edje_color_class_members_free();
   _edje_color_class_hash_free();

   eina_stringshare_del(_edje_cache_path);
   eina_stringshare_del(_edje_language);
   _edje_cache_path = NULL;
   _edje_language = NULL;

   eina_mempool_del(_edje_real_part_state_mp);
   eina_mempool_del(_edje_real_part_mp);
   _edje_real_part_state_mp = NULL;
   _edje_real_part_mp = NULL;

   edje_signal_shutdown();
   _edje_multisense_shutdown();
   _edje_message_shutdown();
   _edje_module_shutdown();
   _edje_external_shutdown();
   _edje_box_shutdown();
   _edje_text_class_members_free();
   _edje_text_class_hash_free();
   _edje_size_class_members_free();
   _edje_size_class_hash_free();
   _edje_edd_shutdown();

   eina_cow_del(_edje_calc_params_map_cow);
   eina_cow_del(_edje_calc_params_physics_cow);
   _edje_calc_params_map_cow = NULL;
   _edje_calc_params_physics_cow = NULL;

#ifdef HAVE_ECORE_IMF
   if (_need_imf)
     ecore_imf_shutdown();
#endif

   efreet_shutdown();
   evas_shutdown();
   eet_shutdown();
   embryo_shutdown();
   ecore_shutdown();
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
   _edje_file_del(ed);
   if (ed->path) eina_stringshare_del(ed->path);
   if (ed->group) eina_stringshare_del(ed->group);
   if (ed->parent) eina_stringshare_del(ed->parent);
   ed->path = NULL;
   ed->group = NULL;
   eina_hash_free(ed->color_classes);
   eina_hash_free(ed->text_classes);
   eina_hash_free(ed->size_classes);
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

   _edje_color_class_member_clean(ed);
   _edje_text_class_members_clean(ed);
   _edje_size_class_members_clean(ed);
}

void
_edje_clean_objects(Edje *ed)
{
   evas_object_del(ed->base->clipper);
   ed->base->evas = NULL;
   ed->obj = NULL;
   ed->base->clipper = NULL;
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

