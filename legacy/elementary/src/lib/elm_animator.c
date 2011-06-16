#include <Elementary.h>
#include "elm_priv.h"

#define ELM_ANIMATOR_CHECK_OR_RETURN(animator, ...) \
   do { \
      if (!animator) { \
         CRITICAL("Elm_Animator " # animator " is NULL!"); \
         return __VA_ARGS__; \
      } \
      if (!EINA_MAGIC_CHECK(animator, ELM_ANIMATOR_MAGIC)) { \
         EINA_MAGIC_FAIL(animator, ELM_ANIMATOR_MAGIC); \
         return __VA_ARGS__; \
      } \
   } while (0)


struct _Elm_Animator
{
#define ELM_ANIMATOR_MAGIC 0x40777770
   EINA_MAGIC;

   Evas_Object *parent;
   Ecore_Animator *animator;
   double begin_time;
   double cur_time;
   double duration;
   unsigned int repeat_cnt;
   unsigned int cur_repeat_cnt;
   void (*animator_op) (void *data, Elm_Animator *animator, double frame);
   void *animator_arg;
   void (*completion_op) (void *data);
   void *completion_arg;
   Elm_Animator_Curve_Style curve_style;
   Eina_Bool auto_reverse:1;
   Eina_Bool on_animating:1;
};

static double _animator_curve_linear(double frame);
static double _animator_curve_in_out(double frame);
static double _animator_curve_in(double frame);
static double _animator_curve_out(double frame);
static unsigned int _animator_compute_reverse_repeat_count(unsigned int cnt);
static unsigned int _animator_compute_no_reverse_repeat_count(unsigned int cnt);
static Eina_Bool _animator_animate_cb(void *data);
static void _delete_animator(Elm_Animator *animator);
static void _animator_parent_del(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event __UNUSED__);

static unsigned int
_animator_compute_reverse_repeat_count(unsigned int cnt)
{
   return ((cnt + 1) * 2) - 1;
}

static unsigned int
_animator_compute_no_reverse_repeat_count(unsigned int cnt)
{
   return cnt / 2;
}

static double
_animator_curve_linear(double frame)
{
   return frame;
}

static double
_animator_curve_in_out(double frame)
{
   if (frame < 0.5) return _animator_curve_in(frame * 2) * 0.5;
   else return (_animator_curve_out(frame * 2 - 1) * 0.5) + 0.5;
}

static double
_animator_curve_in(double frame)
{
   return 1 - sqrt(1 - pow(frame, 2));
}

static double
_animator_curve_out(double frame)
{
   return sqrt(1 - pow(frame - 1, 2));
}

static void
_delete_animator(Elm_Animator *animator)
{
   if (!animator->animator) return;
   ecore_animator_del(animator->animator);
   animator->animator = NULL;
}

static Eina_Bool
_animator_animate_cb(void *data)
{
   double elapsed_time, frame;
   Elm_Animator *animator = (Elm_Animator *) data;

   animator->cur_time = ecore_loop_time_get();
   elapsed_time = animator->cur_time - animator->begin_time;
   if (elapsed_time > animator->duration) elapsed_time = animator->duration;

   //Compute current frame
   switch (animator->curve_style)
     {
       case ELM_ANIMATOR_CURVE_IN_OUT:
          frame = _animator_curve_in_out(elapsed_time / animator->duration);
          break;
       case ELM_ANIMATOR_CURVE_IN:
          frame = _animator_curve_in(elapsed_time / animator->duration);
          break;
       case ELM_ANIMATOR_CURVE_OUT:
          frame = _animator_curve_out(elapsed_time / animator->duration);
          break;
       default:
          frame = _animator_curve_linear(elapsed_time / animator->duration);
          break;
     }

   //Reverse?
   if (animator->auto_reverse)
     {
        if (!(animator->cur_repeat_cnt % 2)) frame = 1 - frame;
     }

   if (animator->duration > 0)
      animator->animator_op(animator->animator_arg, animator, frame);
   //Not end. Keep going.
   if (elapsed_time < animator->duration) return ECORE_CALLBACK_RENEW;

   //Repeat and reverse and time done!
   if (!animator->cur_repeat_cnt)
     {
        animator->on_animating = EINA_FALSE;
        _delete_animator(animator);
        if (animator->completion_op)
          animator->completion_op(animator->completion_arg);
        return ECORE_CALLBACK_CANCEL;
     }

   //Repeat Case
   animator->cur_repeat_cnt--;
   animator->begin_time = ecore_loop_time_get();

   return ECORE_CALLBACK_RENEW;
}

static void
_animator_parent_del(void *data, Evas *evas __UNUSED__,
                     Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   elm_animator_del(data);
}

EAPI Eina_Bool
elm_animator_auto_reverse_get(const Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator, EINA_FALSE);
   return animator->auto_reverse;
}

EAPI unsigned int
elm_animator_repeat_get(const Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator, 0);
   return animator->repeat_cnt;
}

EAPI Elm_Animator_Curve_Style
elm_animator_curve_style_get(const Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator, ELM_ANIMATOR_CURVE_LINEAR);
   return animator->curve_style;
}

EAPI void
elm_animator_auto_reverse_set(Elm_Animator *animator, Eina_Bool reverse)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   if (animator->auto_reverse == reverse) return;
   animator->auto_reverse = reverse;
   if (reverse)
      animator->repeat_cnt =
        _animator_compute_reverse_repeat_count(animator->repeat_cnt);
   else
      animator->repeat_cnt =
        _animator_compute_no_reverse_repeat_count(animator->repeat_cnt);
}

EAPI void
elm_animator_curve_style_set(Elm_Animator *animator,
                             Elm_Animator_Curve_Style cs)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   animator->curve_style = cs;
}
EAPI void
elm_animator_duration_set(Elm_Animator *animator, double duration)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   if (animator->on_animating) return;
   animator->duration = duration;
}

EAPI void
elm_animator_operation_callback_set(Elm_Animator *animator,
                                    Elm_Animator_Operation_Cb func,
                                    void *data)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   if (animator->on_animating) return;
   animator->animator_op = func;
   animator->animator_arg = data;
}

EAPI Elm_Animator *
elm_animator_add(Evas_Object *parent)
{
   Elm_Animator *animator = ELM_NEW(Elm_Animator);
   if (!animator) return NULL;
   EINA_MAGIC_SET(animator, ELM_ANIMATOR_MAGIC);
   animator->parent = parent;
   elm_animator_auto_reverse_set(animator, EINA_FALSE);
   elm_animator_curve_style_set(animator, ELM_ANIMATOR_CURVE_LINEAR);
   if (parent)
      evas_object_event_callback_add(parent, EVAS_CALLBACK_DEL,
                                     _animator_parent_del, animator);
   return animator;
}

EAPI Eina_Bool
elm_animator_operating_get(const Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator, EINA_FALSE);
   return animator->on_animating;
}

EAPI void
elm_animator_del(Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   _delete_animator(animator);
   if (animator->parent)
      evas_object_event_callback_del(animator->parent, EVAS_CALLBACK_DEL,
                                     _animator_parent_del);

   EINA_MAGIC_SET(animator, EINA_MAGIC_NONE);
   free(animator);
}

EAPI void
elm_animator_completion_callback_set(Elm_Animator *animator,
                                     Elm_Animator_Completion_Cb func,
                                     void *data)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   if (animator->on_animating) return;
   animator->completion_op = func;
   animator->completion_arg = data;
}

EAPI void
elm_animator_pause(Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   if (!animator->on_animating) return;
   ecore_animator_freeze(animator->animator);
}

EAPI void
elm_animator_resume(Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   if (!animator->on_animating) return;
   ecore_animator_thaw(animator->animator);
}

EAPI void
elm_animator_stop(Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   animator->on_animating = EINA_FALSE;
   _delete_animator(animator);
}

EAPI void
elm_animator_repeat_set(Elm_Animator *animator, unsigned int repeat_cnt)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   if (!animator->auto_reverse) animator->repeat_cnt = repeat_cnt;
   else
      animator->repeat_cnt = _animator_compute_reverse_repeat_count(repeat_cnt);
}

EAPI void
elm_animator_animate(Elm_Animator *animator)
{
   ELM_ANIMATOR_CHECK_OR_RETURN(animator);
   if (!animator->animator_op) return;
   animator->begin_time = ecore_loop_time_get();
   animator->cur_repeat_cnt = animator->repeat_cnt;
   if (!animator->animator)
      animator->animator = ecore_animator_add(_animator_animate_cb, animator);
   if (animator->animator) animator->on_animating = EINA_TRUE;
}
