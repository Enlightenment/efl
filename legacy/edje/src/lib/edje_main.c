#include "edje_private.h"

static Edje_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Edje_Version *edje_version = &_version;

static int _edje_init_count = 0;
int _edje_default_log_dom = -1;
Eina_Mempool *_edje_real_part_mp = NULL;
Eina_Mempool *_edje_real_part_state_mp = NULL;

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


EAPI int
edje_init(void)
{
   if (++_edje_init_count != 1)
     return _edje_init_count;

   srand(time(NULL));

   if (!eina_init())
     {
	fprintf(stderr, "Edje: Eina init failed");
	return --_edje_init_count;
     }

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

   _edje_scale = FROM_DOUBLE(1.0);

   _edje_edd_init();
   _edje_text_init();
   _edje_box_init();
   _edje_external_init();
   _edje_module_init();
   _edje_message_init();

   _edje_real_part_mp = eina_mempool_add("chained_mempool",
					 "Edje_Real_Part", NULL,
					 sizeof (Edje_Real_Part), 128);
   if (!_edje_real_part_mp)
     {
	ERR("Mempool for Edje_Real_Part cannot be allocated.");
	goto shutdown_eet;
     }

   _edje_real_part_state_mp = eina_mempool_add("chained_mempool",
					       "Edje_Real_Part_State", NULL,
					       sizeof (Edje_Real_Part_State), 256);
   if (!_edje_real_part_state_mp)
     {
	ERR("Mempool for Edje_Real_Part_State cannot be allocated.");
	goto shutdown_eet;
     }

   return _edje_init_count;

 shutdown_eet:
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
   _edje_edd_shutdown();
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


EAPI int
edje_shutdown(void)
{
   if (--_edje_init_count != 0)
     return _edje_init_count;

   if (_edje_timer)
     ecore_animator_del(_edje_timer);
   _edje_timer = NULL;

   _edje_file_cache_shutdown();
   _edje_color_class_members_free();
   _edje_color_class_hash_free();

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
   _edje_edd_shutdown();

   eet_shutdown();
   embryo_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_edje_default_log_dom);
   _edje_default_log_dom = -1;
   eina_shutdown();

   return _edje_init_count;
}

/* Private Routines */
static Eina_Bool
_text_class_member_free(const Eina_Hash *hash __UNUSED__,
			const void *key,
			void *data,
			void *fdata __UNUSED__)
{
   _edje_text_class_member_direct_del(key, data);
   return EINA_TRUE;
}

void
_edje_del(Edje *ed)
{
   Edje_Running_Program *runp;
   Edje_Pending_Program *pp;
   Edje_Signal_Callback *escb;
   Edje_Color_Class *cc;
   Edje_Text_Class *tc;
   Edje_Text_Insert_Filter_Callback *cb;

   if (ed->processing_messages)
     {
	ed->delete_me = 1;
	return;
     }
   _edje_message_del(ed);
   _edje_callbacks_patterns_clean(ed);
   _edje_file_del(ed);
   if (ed->path) eina_stringshare_del(ed->path);
   if (ed->group) eina_stringshare_del(ed->group);
   if (ed->parent) eina_stringshare_del(ed->parent);
   ed->path = NULL;
   ed->group = NULL;
   if ((ed->actions) || (ed->pending_actions))
     {
	_edje_animators = eina_list_remove(_edje_animators, ed);
     }
   EINA_LIST_FREE(ed->actions, runp)
     free(runp);
   EINA_LIST_FREE(ed->pending_actions, pp)
     free(pp);
   EINA_LIST_FREE(ed->callbacks, escb)
     {
	if (escb->signal) eina_stringshare_del(escb->signal);
	if (escb->source) eina_stringshare_del(escb->source);
	free(escb);
     }
   EINA_LIST_FREE(ed->color_classes, cc)
     {
	if (cc->name) eina_stringshare_del(cc->name);
	free(cc);
     }
   EINA_LIST_FREE(ed->text_classes, tc)
     {
	if (tc->name) eina_stringshare_del(tc->name);
	if (tc->font) eina_stringshare_del(tc->font);
	free(tc);
     }
   EINA_LIST_FREE(ed->text_insert_filter_callbacks, cb)
     {
        eina_stringshare_del(cb->part);
        free(cb);
     }

   if (ed->members)
     {
        eina_hash_foreach(ed->members, _text_class_member_free, NULL);
        eina_hash_free(ed->members);
     }
   free(ed);
}

void
_edje_clean_objects(Edje *ed)
{
   evas_object_del(ed->base.clipper);
   ed->base.evas = NULL;
   ed->obj = NULL;
   ed->base.clipper = NULL;
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
