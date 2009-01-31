/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "ecore_private.h"
#include "Ecore.h"

static int _ecore_animator(void *data);

static Ecore_Timer    *timer = NULL;
static int             animators_delete_me = 0;
static Ecore_Animator *animators = NULL;
static double          animators_frametime = 1.0 / 30.0;

/**
 * Add a animator to tick off at every animaton tick during main loop execution.
 * @param func The function to call when it ticks off
 * @param data The data to pass to the function
 * @return A handle to the new animator
 * @ingroup Ecore_Animator_Group
 *
 * This function adds a animator and returns its handle on success and NULL on
 * failure. The function @p func will be called every N seconds where N is the
 * frametime interval set by ecore_animator_frametime_set(). The function will
 * be passed the @p data pointer as its parameter.
 *
 * When the animator @p func is called, it must return a value of either 1 or 0.
 * If it returns 1 (or ECORE_CALLBACK_RENEW), it will be called again at the
 * next tick, or if it returns 0 (or ECORE_CALLBACK_CANCEL) it will be deleted
 * automatically making any references/handles for it invalid.
 */
EAPI Ecore_Animator *
ecore_animator_add(int (*func) (void *data), const void *data)
{
   Ecore_Animator *animator;

   if (!func) return NULL;
   animator = calloc(1, sizeof(Ecore_Animator));
   if (!animator) return NULL;
   ECORE_MAGIC_SET(animator, ECORE_MAGIC_ANIMATOR);
   animator->func = func;
   animator->data = (void *)data;
   animators = _ecore_list2_append(animators, animator);
   if (!timer)
     timer = ecore_timer_add(animators_frametime, _ecore_animator, NULL);
   return animator;
}

/**
 * Delete the specified animator from the animator list.
 * @param animator The animator to delete
 * @return The data pointer set for the animator
 * @ingroup Ecore_Animator_Group
 *
 * Delete the specified @p aqnimator from the set of animators that are executed
 * during main loop execution. This function returns the data parameter that
 * was being passed to the callback on success, or NULL on failure. After this
 * call returns the specified animator object @p animator is invalid and should not
 * be used again. It will not get called again after deletion.
 */
EAPI void *
ecore_animator_del(Ecore_Animator *animator)
{
   if (!ECORE_MAGIC_CHECK(animator, ECORE_MAGIC_ANIMATOR))
     {
	ECORE_MAGIC_FAIL(animator, ECORE_MAGIC_ANIMATOR,
			 "ecore_animator_del");
	return NULL;
     }
   if (animator->delete_me) return animator->data;
   animator->delete_me = 1;
   animators_delete_me++;
   return animator->data;
}

/**
 * Set the animator call interval in seconds.
 * @param frametime The time in seconds in between animator ticks.
 *
 * This function sets the time interval (in seconds) inbetween animator ticks.
 */
EAPI void
ecore_animator_frametime_set(double frametime)
{
   if (frametime < 0.0) frametime = 0.0;
   if (animators_frametime == frametime) return;
   animators_frametime = frametime;
   if (timer)
     {
	ecore_timer_del(timer);
	timer = NULL;
     }
   if (animators)
     timer = ecore_timer_add(animators_frametime, _ecore_animator, NULL);
}

/**
 * Get the animator call interval in seconds.
 * @return The time in second in between animator ticks.
 *
 * this function retrieves the time inbetween animator ticks, in seconds.
 */
EAPI double
ecore_animator_frametime_get(void)
{
   return animators_frametime;
}

void
_ecore_animator_shutdown(void)
{
   if (timer)
     {
	ecore_timer_del(timer);
	timer = NULL;
     }
   while (animators)
     {
	Ecore_Animator *animator;

	animator = animators;
	animators = _ecore_list2_remove(animators, animator);
	ECORE_MAGIC_SET(animator, ECORE_MAGIC_NONE);
	free(animator);
     }
}

static int
_ecore_animator(void *data __UNUSED__)
{
   Ecore_List2 *l;

   for (l = (Ecore_List2 *)animators; l;)
     {
	Ecore_Animator *animator;

	animator = (Ecore_Animator *)l;
	l = l->next;
	if (!animator->delete_me)
	  {
	     if (!animator->func(animator->data))
	       {
		  animator->delete_me = 1;
		  animators_delete_me++;
	       }
	  }
     }
   if (animators_delete_me)
     {
	for (l = (Ecore_List2 *)animators; l;)
	  {
	     Ecore_Animator *animator;

	     animator = (Ecore_Animator *)l;
	     l = l->next;
	     if (animator->delete_me)
	       {
		  animators = _ecore_list2_remove(animators, animator);
		  ECORE_MAGIC_SET(animator, ECORE_MAGIC_NONE);
		  free(animator);
		  animators_delete_me--;
		  if (animators_delete_me == 0) break;
	       }
	  }
     }
   if (!animators)
     {
	timer = NULL;
	return 0;
     }
   return 1;
}
