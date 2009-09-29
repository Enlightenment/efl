/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <time.h>

#include "edje_private.h"

static int initted = 0;
Eina_Mempool *_edje_real_part_mp = NULL;
Eina_Mempool *_edje_real_part_state_mp = NULL;


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Edje_main_Group Main
 *
 * @brief These functions provide an abstraction layer between the
 * application code and the interface, while allowing extremely
 * flexible dynamic layouts and animations.
 *
 * @{
 */

/**
 * @brief Initialize the edje library.
 *
 * @return The new init count. The initial value is zero.
 *
 * This function initializes the ejde library, making the propers
 * calls to initialization functions. It makes calls to functions
 * eina_init(), ecore_job_init(), embryo_init() and eet_init() so
 * there is no need to call those functions again in your code. To
 * shutdown edje there is a function edje_shutdown().
 *
 * @see edje_shutdown()
 * @see eina_init()
 * @see ecore_job_init()
 * @see embryo_init()
 * @see eet_init()
 *
 */

EAPI int
edje_init(void)
{
   initted++;
   if (initted == 1)
     {
	eina_init();
        ecore_job_init();
	srand(time(NULL));
	_edje_edd_setup();
	_edje_text_init();
	_edje_box_init();
	_edje_lua_init();
	embryo_init();
	eet_init();

	_edje_real_part_mp = eina_mempool_add("chained_mempool",
					      "Edje_Real_Part", NULL,
					      sizeof (Edje_Real_Part), 128);
	if (!_edje_real_part_mp)
	  {
	     EINA_ERROR_PERR("ERROR: Mempool for Edje_Real_Part cannot be allocated.\n");
	     goto on_error;
	  }

	_edje_real_part_state_mp = eina_mempool_add("chained_mempool",
					      "Edje_Real_Part_State", NULL,
					      sizeof (Edje_Real_Part_State), 256);
	if (!_edje_real_part_state_mp)
	  {
	     EINA_ERROR_PERR("ERROR: Mempool for Edje_Real_Part_State cannot be allocated.\n");
	     goto on_error;
	  }
     }
   _edje_message_init();
   return initted;

 on_error:
   eina_mempool_del(_edje_real_part_state_mp);
   eina_mempool_del(_edje_real_part_mp);
   _edje_real_part_state_mp = NULL;
   _edje_real_part_mp = NULL;
   return 0;
}

/**
 * @brief Shutdown the edje library.
 *
 * @return Zero, always.
 *
 * This function shuts down the edje library. It calls the functions
 * eina_shutdown(), ecore_job_shutdown(), embryo_shutdown() and
 * eet_shutdown(), so there is no need to call these functions again
 * in your code.
 *
 * @see edje_init()
 * @see eina_shutdown()
 * @see ecore_job_shutdown()
 * @see embryo_shutdown()
 * @see eet_shutdown()
 *
 */

EAPI int
edje_shutdown(void)
{
   initted--;
   if (initted > 0) return initted;

   if (_edje_timer)
     ecore_animator_del(_edje_timer);
   _edje_timer = NULL;

   _edje_file_cache_shutdown();
   _edje_message_shutdown();
   _edje_edd_free();
   _edje_color_class_members_free();
   _edje_color_class_hash_free();
   _edje_text_class_members_free();
   _edje_text_class_hash_free();
   _edje_box_shutdown();

   eina_mempool_del(_edje_real_part_state_mp);
   eina_mempool_del(_edje_real_part_mp);
   _edje_real_part_state_mp = NULL;
   _edje_real_part_mp = NULL;

   embryo_shutdown();
   _edje_lua_shutdown();
   ecore_job_shutdown();
   eet_shutdown();
   eina_shutdown();

   return 0;
}

/* Private Routines */

Edje *
_edje_add(Evas_Object *obj)
{
   Edje *ed;

   ed = calloc(1, sizeof(Edje));
   if (!ed) return NULL;
   ed->evas = evas_object_evas_get(obj);
   ed->clipper = evas_object_rectangle_add(ed->evas);
   evas_object_smart_member_add(ed->clipper, obj);
   evas_object_color_set(ed->clipper, 255, 255, 255, 255);
   evas_object_move(ed->clipper, -10000, -10000);
   evas_object_resize(ed->clipper, 20000, 20000);
   evas_object_pass_events_set(ed->clipper, 1);
   ed->have_objects = 1;
   ed->references = 1;
   return ed;
}

void
_edje_del(Edje *ed)
{
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
   while (ed->actions)
     {
	Edje_Running_Program *runp;

	runp = eina_list_data_get(ed->actions);
	ed->actions = eina_list_remove(ed->actions, runp);
	free(runp);
     }
   while (ed->pending_actions)
     {
	Edje_Pending_Program *pp;

	pp = eina_list_data_get(ed->pending_actions);
	ed->pending_actions = eina_list_remove(ed->pending_actions, pp);
	free(pp);
     }
   while (ed->callbacks)
     {
	Edje_Signal_Callback *escb;

	escb = eina_list_data_get(ed->callbacks);
	ed->callbacks = eina_list_remove(ed->callbacks, escb);
	if (escb->signal) eina_stringshare_del(escb->signal);
	if (escb->source) eina_stringshare_del(escb->source);
	free(escb);
     }
   while (ed->color_classes)
     {
	Edje_Color_Class *cc;

	cc = eina_list_data_get(ed->color_classes);
	ed->color_classes = eina_list_remove(ed->color_classes, cc);
	if (cc->name) eina_stringshare_del(cc->name);
	free(cc);
     }
   while (ed->text_classes)
     {
	Edje_Text_Class *tc;

	tc = eina_list_data_get(ed->text_classes);
	ed->text_classes = eina_list_remove(ed->text_classes, tc);
	if (tc->name) eina_stringshare_del(tc->name);
	if (tc->font) eina_stringshare_del(tc->font);
	free(tc);
     }
   free(ed);
}

void
_edje_clean_objects(Edje *ed)
{
   evas_object_del(ed->clipper);
   ed->evas = NULL;
   ed->obj = NULL;
   ed->clipper = NULL;
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

/**
 *
 * @}
 */
