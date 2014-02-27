#include "edje_private.h"

typedef struct _Edje_Box_Layout Edje_Box_Layout;
struct _Edje_Box_Layout
{
   EINA_RBTREE;
   Evas_Object_Box_Layout func;
   void *(*layout_data_get)(void *);
   void (*layout_data_free)(void *);
   void *data;
   void (*free_data)(void *);
   char name[];
};

static Eina_Hash *_edje_color_class_hash = NULL;
static Eina_Hash *_edje_color_class_member_hash = NULL;

static Eina_Hash *_edje_text_class_hash = NULL;
static Eina_Hash *_edje_text_class_member_hash = NULL;

static Eina_Rbtree *_edje_box_layout_registry = NULL;

char *_edje_fontset_append = NULL;
FLOAT_T _edje_scale = ZERO;
Eina_Bool _edje_password_show_last = EINA_FALSE;
FLOAT_T _edje_password_show_last_timeout = ZERO;
int _edje_freeze_val = 0;
int _edje_freeze_calc_count = 0;
Eina_List *_edje_freeze_calc_list = NULL;

typedef struct _Edje_List_Foreach_Data Edje_List_Foreach_Data;
struct _Edje_List_Foreach_Data
{
   Eina_List *list;
};

typedef struct _Edje_List_Refcount Edje_List_Refcount;
struct _Edje_List_Refcount
{
   EINA_REFCOUNT;

   Eina_List *lookup;
};

static Eina_Bool _edje_color_class_list_foreach(const Eina_Hash *hash, const void *key, void *data, void *fdata);
static Eina_Bool _edje_text_class_list_foreach(const Eina_Hash *hash, const void *key, void *data, void *fdata);
static void _edje_object_image_preload_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _edje_object_signal_preload_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _edje_user_def_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *child EINA_UNUSED, void *einfo EINA_UNUSED);
static void _edje_child_remove(Edje *ed, Edje_Real_Part *rp, Evas_Object *child);

Edje_Real_Part *_edje_real_part_recursive_get_helper(Edje **ed, char **path);

static Edje_User_Defined *
_edje_user_definition_new(Edje_User_Defined_Type type, const char *part, Edje *ed)
{
   Edje_User_Defined *eud;

   eud = malloc(sizeof (Edje_User_Defined));
   if (!eud) return NULL;

   eud->type = type;
   eud->part = eina_stringshare_add(part);
   eud->ed = ed;
   ed->user_defined = eina_list_append(ed->user_defined, eud);

   return eud;
}

void
_edje_user_definition_remove(Edje_User_Defined *eud, Evas_Object *child)
{
   eud->ed->user_defined = eina_list_remove(eud->ed->user_defined, eud);

   if (child) evas_object_event_callback_del_full(child, EVAS_CALLBACK_DEL, _edje_user_def_del_cb, eud);
   eina_stringshare_del(eud->part);
   free(eud);
}

void
_edje_user_definition_free(Edje_User_Defined *eud)
{
   Evas_Object *child = NULL;
   Edje_Real_Part *rp;

   eud->ed->user_defined = eina_list_remove(eud->ed->user_defined, eud);

   switch (eud->type)
     {
      case EDJE_USER_SWALLOW:
         child = eud->u.swallow.child;
         rp = _edje_real_part_recursive_get(&eud->ed, eud->part);
         if (rp)
           {
	      _edje_real_part_swallow_clear(eud->ed, rp);
              if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
                  (rp->typedata.swallow))
                {
                   rp->typedata.swallow->swallowed_object = NULL;
                   rp->typedata.swallow->swallow_params.min.w = 0;
                   rp->typedata.swallow->swallow_params.min.h = 0;
                   rp->typedata.swallow->swallow_params.max.w = 0;
                   rp->typedata.swallow->swallow_params.max.h = 0;
                }
              eud->ed->dirty = EINA_TRUE;
              eud->ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
              rp->invalidate = EINA_TRUE;
#endif
              _edje_recalc_do(eud->ed);
           }
         break;
      case EDJE_USER_BOX_PACK:
         child = eud->u.box.child;
         rp = _edje_real_part_recursive_get(&eud->ed, eud->part);
         if (rp) _edje_child_remove(eud->ed, rp, child);
         break;
      case EDJE_USER_TABLE_PACK:
         child = eud->u.table.child;
         rp = _edje_real_part_recursive_get(&eud->ed, eud->part);
         if (rp) _edje_child_remove(eud->ed, rp, child);
         break;
      case EDJE_USER_STRING:
      case EDJE_USER_DRAG_STEP:
      case EDJE_USER_DRAG_PAGE:
      case EDJE_USER_DRAG_VALUE:
      case EDJE_USER_DRAG_SIZE:
         break;
     }

   _edje_user_definition_remove(eud, child);
}

static void
_edje_user_def_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *child EINA_UNUSED, void *einfo EINA_UNUSED)
{
   Edje_User_Defined *eud = data;

   _edje_user_definition_free(eud);
}

static void
_edje_class_member_direct_del(const char *class, Edje_List_Refcount *lookup, Eina_Hash *hash)
{
   Eina_List *members;

   if (!lookup) return;
   members = eina_hash_find(hash, class);
   if (members)
     members = eina_list_remove_list(members, lookup->lookup);
   eina_hash_set(hash, class, members);
   free(lookup);
}

static void
_edje_class_member_add(Edje *ed, Eina_Hash **ehash, Eina_Hash **ghash, const char *class)
{
   Edje_List_Refcount *lookup;
   Eina_List *members;

   if ((!ed) || (!ehash) || (!ghash) || (!class)) return;

   lookup = eina_hash_find(*ehash, class);
   if (lookup)
     {
        EINA_REFCOUNT_REF(lookup);
        return;
     }

   lookup = malloc(sizeof (Edje_List_Refcount));
   if (!lookup) return;
   EINA_REFCOUNT_INIT(lookup);

   /* Get members list */
   members = eina_hash_find(*ghash, class);

   /* Update the member list */
   lookup->lookup = members = eina_list_prepend(members, ed);

   /* Don't loose track of members list */
   if (!*ehash)
     *ehash = eina_hash_string_small_new(NULL);
   eina_hash_add(*ehash, class, lookup);

   /* Reset the member list to the right pointer */
   if (!*ghash)
     *ghash = eina_hash_string_superfast_new(NULL);
   eina_hash_set(*ghash, class, members);
}

static void
_edje_class_member_del(Eina_Hash **ehash, Eina_Hash **ghash, const char *class)
{
   Edje_List_Refcount *lookup;
   Eina_List *members;

   if ((!ehash) || (!ghash) || (!class)) return;
   members = eina_hash_find(*ghash, class);
   if (!members) return;

   lookup = eina_hash_find(*ehash, class);
   if (!lookup) return;

   EINA_REFCOUNT_UNREF(lookup)
   {
      members = eina_list_remove_list(members, lookup->lookup);
      eina_hash_set(*ghash, class, members);

      eina_hash_del(*ehash, class, lookup);
      free(lookup);
   }
}

static Eina_Bool
member_list_free(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   eina_list_free(data);
   return EINA_TRUE;
}

static void
_edje_class_members_free(Eina_Hash **ghash)
{
   if (!ghash || !*ghash) return;
   eina_hash_foreach(*ghash, member_list_free, NULL);
   eina_hash_free(*ghash);
   *ghash = NULL;
}

/************************** API Routines **************************/

#define FASTFREEZE 1

EAPI void
edje_freeze(void)
{
#ifdef FASTFREEZE
   _edje_freeze_val++;
#else
// FIXME: could just have a global freeze instead of per object
// above i tried.. but this broke some things. notable e17's menus. why?
   Eina_List *l;
   Evas_Object *data;

   EINA_LIST_FOREACH(_edje_edjes, l, data)
     edje_object_freeze(data);
#endif
}

#ifdef FASTFREEZE
static void
_edje_thaw_edje(Edje *ed)
{
   unsigned int i;

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;

        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          {
             Edje *ed2;

             ed2 = _edje_fetch(rp->typedata.swallow->swallowed_object);
             if (ed2) _edje_thaw_edje(ed2);
          }
     }
   if ((ed->recalc) && (ed->freeze <= 0)) _edje_recalc_do(ed);
}
#endif

EAPI void
edje_thaw(void)
{
#ifdef FASTFREEZE
   if (!_edje_freeze_val) return;
   _edje_freeze_val--;
   if ((_edje_freeze_val == 0) && (_edje_freeze_calc_count > 0))
     {
        Edje *ed;

        _edje_freeze_calc_count = 0;
        EINA_LIST_FREE(_edje_freeze_calc_list, ed)
          {
             _edje_thaw_edje(ed);
             ed->freeze_calc = EINA_FALSE;
          }
     }
#else
  Evas_Object *data;

// FIXME: could just have a global freeze instead of per object
// comment as above.. why?
   Eina_List *l;

   EINA_LIST_FOREACH(_edje_edjes, l, data)
     edje_object_thaw(data);
#endif
}

EAPI void
edje_fontset_append_set(const char *fonts)
{
   if (_edje_fontset_append)
     free(_edje_fontset_append);
   _edje_fontset_append = fonts ? strdup(fonts) : NULL;
}

EAPI const char *
edje_fontset_append_get(void)
{
   return _edje_fontset_append;
}

EAPI void
edje_scale_set(double scale)
{
   Eina_List *l;
   Evas_Object *data;

   if (_edje_scale == FROM_DOUBLE(scale)) return;
   _edje_scale = FROM_DOUBLE(scale);
   EINA_LIST_FOREACH(_edje_edjes, l, data)
     edje_object_calc_force(data);
}

EAPI double
edje_scale_get(void)
{
  return TO_DOUBLE(_edje_scale);
}

EAPI void
edje_password_show_last_set(Eina_Bool password_show_last)
{
   if (_edje_password_show_last == password_show_last) return;
   _edje_password_show_last = password_show_last;
}

EAPI void
edje_password_show_last_timeout_set(double password_show_last_timeout)
{
   if (_edje_password_show_last_timeout == FROM_DOUBLE(password_show_last_timeout)) return;
   _edje_password_show_last_timeout = FROM_DOUBLE(password_show_last_timeout);
}

EAPI Eina_Bool
edje_object_scale_set(Evas_Object *obj, double scale)
{
   if (!eo_isa(obj, EDJE_OBJ_CLASS)) return EINA_FALSE;
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_scale_set(scale, &ret));
   return ret;
}

void
_scale_set(Eo *obj, void *_pd, va_list *list)
{
   double scale = va_arg(*list, double);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_TRUE;
   Edje *ed, *ged;
   Evas_Object *o;
   Eina_List *l;
   unsigned int i;

   ed = _pd;
   if (ed->scale == scale) return;
   ed->scale = FROM_DOUBLE(scale);
   EINA_LIST_FOREACH(ed->groups, l, ged)
      edje_object_scale_set(ged->obj, scale);
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *ep;
        ep = ed->table_parts[i];
        if ((ep->part->type == EDJE_PART_TYPE_BOX) ||
            (ep->part->type == EDJE_PART_TYPE_TABLE))
          {
             EINA_LIST_FOREACH(ep->typedata.container->items, l, o)
               edje_object_scale_set(o, scale);
          }
     }
   edje_object_calc_force(obj);
}

EAPI double
edje_object_scale_get(const Evas_Object *obj)
{
   if (!obj) return 0.0;
   double ret = 0.0;
   eo_do((Eo *)obj, edje_obj_scale_get(&ret));
   return ret;
}

void
_scale_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   const Edje *ed = _pd;
   *ret = TO_DOUBLE(ed->scale);
}

EAPI Eina_Bool
edje_object_mirrored_get(const Evas_Object *obj)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_mirrored_get(&ret));
   return ret;
}

void
_mirrored_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   const Edje *ed = _pd;
   *ret = ed->is_rtl;
}

void
_edje_object_orientation_inform(Evas_Object *obj)
{
   if (!obj) return;
   if (edje_object_mirrored_get(obj))
     edje_object_signal_emit(obj, "edje,state,rtl", "edje");
   else
     edje_object_signal_emit(obj, "edje,state,ltr", "edje");
}

EAPI void
edje_object_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   if (!obj) return;
   eo_do(obj, edje_obj_mirrored_set(rtl));
}

void
_mirrored_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool rtl = va_arg(*list, int);

   Edje *ed = _pd;
   unsigned int i;

   if (ed->is_rtl == rtl) return;

   ed->is_rtl = rtl;

   for (i = 0 ; i < ed->table_parts_size ; i++)
     {
        Edje_Real_Part *ep;
        const char *s;
        double v;

        ep = ed->table_parts[i];
        s = ep->param1.description->state.name,
          v = ep->param1.description->state.value;
        _edje_part_description_apply(ed, ep, s, v , NULL, 0.0);
        ep->chosen_description = ep->param1.description;
     }
   _edje_recalc_do(ed);

   _edje_object_orientation_inform(obj);

   return;
}

EAPI const char *
edje_object_data_get(const Evas_Object *obj, const char *key)
{
   if (!obj) return NULL;
   const char *ret = NULL;
   eo_do((Eo *)obj, edje_obj_data_get(key, &ret));
   return ret;
}

void
_data_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *key = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;

   const Edje *ed = _pd;

   if ((!ed) || (!key))
     return;
   if (!ed->collection) return;
   if (!ed->collection->data) return;
   *ret = edje_string_get(eina_hash_find(ed->collection->data, key));
}

EAPI int
edje_object_freeze(Evas_Object *obj)
{
   if (!obj) return 0;
   int ret = 0;
   eo_do(obj, edje_obj_freeze(&ret));
   return ret;
}

void
_freeze(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);

   Edje *ed = _pd;
   unsigned int i;

   if (ret) *ret = 0;
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;
        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_freeze(rp->typedata.swallow->swallowed_object);
     }
   int int_ret = _edje_freeze(ed);
   if (ret) *ret = int_ret;
}

EAPI int
edje_object_thaw(Evas_Object *obj)
{
   if (!obj) return 0;
   int ret;
   eo_do(obj, edje_obj_thaw(&ret));
   return ret;
}

void
_thaw(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Edje *ed = _pd;
   unsigned int i;

   if (ret) *ret = 0;
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;

        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_thaw(rp->typedata.swallow->swallowed_object);
     }
   int int_ret = _edje_thaw(ed);
   if (ret) *ret = int_ret;
}

EAPI Eina_Bool
edje_color_class_set(const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Eina_List *members;
   Edje_Color_Class *cc;

   if (!color_class) return EINA_FALSE;

   cc = eina_hash_find(_edje_color_class_hash, color_class);
   if (!cc)
     {
        cc = calloc(1, sizeof(Edje_Color_Class));
        if (!cc) return EINA_FALSE;
        cc->name = eina_stringshare_add(color_class);
        if (!cc->name)
          {
             free(cc);
             return EINA_FALSE;
          }
        if (!_edje_color_class_hash)
          _edje_color_class_hash = eina_hash_string_superfast_new(NULL);
        eina_hash_add(_edje_color_class_hash, color_class, cc);
     }

   if (r < 0)        r = 0;
   else if (r > 255) r = 255;
   if (g < 0)        g = 0;
   else if (g > 255) g = 255;
   if (b < 0)        b = 0;
   else if (b > 255) b = 255;
   if (a < 0)        a = 0;
   else if (a > 255) a = 255;
   if ((cc->r == r) && (cc->g == g) &&
       (cc->b == b) && (cc->a == a) &&
       (cc->r2 == r2) && (cc->g2 == g2) &&
       (cc->b2 == b2) && (cc->a2 == a2) &&
       (cc->r3 == r3) && (cc->g3 == g3) &&
       (cc->b3 == b3) && (cc->a3 == a3))
     return EINA_TRUE;
   cc->r = r;
   cc->g = g;
   cc->b = b;
   cc->a = a;
   cc->r2 = r2;
   cc->g2 = g2;
   cc->b2 = b2;
   cc->a2 = a2;
   cc->r3 = r3;
   cc->g3 = g3;
   cc->b3 = b3;
   cc->a3 = a3;

   members = eina_hash_find(_edje_color_class_member_hash, color_class);
   while (members)
     {
        Edje *ed;

        ed = eina_list_data_get(members);
        ed->dirty = EINA_TRUE;
        ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
        ed->all_part_change = EINA_TRUE;
#endif
        _edje_recalc(ed);
        _edje_emit(ed, "color_class,set", color_class);
        members = eina_list_next(members);
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_color_class_get(const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3)
{
   Edje_Color_Class *cc;

   if (!color_class)
     cc = NULL;
   else
     cc = eina_hash_find(_edje_color_class_hash, color_class);

   if (cc)
     {
#define X(C) if (C) *C = cc->C
#define S(_r, _g, _b, _a) X(_r); X(_g); X(_b); X(_a)
        S(r, g, b, a);
        S(r2, g2, b2, a2);
        S(r3, g3, b3, a3);
#undef S
#undef X
        return EINA_TRUE;
     }
   else
     {
#define X(C) if (C) *C = 0
#define S(_r, _g, _b, _a) X(_r); X(_g); X(_b); X(_a)
        S(r, g, b, a);
        S(r2, g2, b2, a2);
        S(r3, g3, b3, a3);
#undef S
#undef X
        return EINA_FALSE;
     }
}

void
edje_color_class_del(const char *color_class)
{
   Edje_Color_Class *cc;
   Eina_List *members;

   if (!color_class) return;

   cc = eina_hash_find(_edje_color_class_hash, color_class);
   if (!cc) return;

   eina_hash_del(_edje_color_class_hash, color_class, cc);
   eina_stringshare_del(cc->name);
   free(cc);

   members = eina_hash_find(_edje_color_class_member_hash, color_class);
   while (members)
     {
        Edje *ed;

        ed = eina_list_data_get(members);
        ed->dirty = EINA_TRUE;
        ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
        ed->all_part_change = EINA_TRUE;
#endif
        _edje_recalc(ed);
        _edje_emit(ed, "color_class,del", color_class);
        members = eina_list_next(members);
     }
}

Eina_List *
edje_color_class_list(void)
{
   Edje_List_Foreach_Data fdata;

   if (!_edje_color_class_hash) return NULL;
   memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
   eina_hash_foreach(_edje_color_class_hash,
                     _edje_color_class_list_foreach, &fdata);

   return fdata.list;
}

static Eina_Bool
_edje_color_class_list_foreach(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data EINA_UNUSED, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = eina_list_append(fd->list, strdup(key));
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_color_class_set(Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_color_class_set(color_class, r, g, b, a, r2, g2, b2, a2, r3, g3, b3, a3, &ret));
   return ret;
}

void
_color_class_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *color_class = va_arg(*list, const char *);
   int r = va_arg(*list, int);
   int g = va_arg(*list, int);
   int b = va_arg(*list, int);
   int a = va_arg(*list, int);
   int r2 = va_arg(*list, int);
   int g2 = va_arg(*list, int);
   int b2 = va_arg(*list, int);
   int a2 = va_arg(*list, int);
   int r3 = va_arg(*list, int);
   int g3 = va_arg(*list, int);
   int b3 = va_arg(*list, int);
   int a3 = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Edje *ed = _pd;
   Edje_Color_Class *cc;
   unsigned int i;

   if ((!ed) || (!color_class)) return;
   if (r < 0)        r = 0;
   else if (r > 255) r = 255;
   if (g < 0)        g = 0;
   else if (g > 255) g = 255;
   if (b < 0)        b = 0;
   else if (b > 255) b = 255;
   if (a < 0)        a = 0;
   else if (a > 255) a = 255;
   cc = eina_hash_find(ed->color_classes, color_class);
   if (cc)
     {
        if ((cc->r == r) && (cc->g == g) &&
            (cc->b == b) && (cc->a == a) &&
            (cc->r2 == r2) && (cc->g2 == g2) &&
            (cc->b2 == b2) && (cc->a2 == a2) &&
            (cc->r3 == r3) && (cc->g3 == g3) &&
            (cc->b3 == b3) && (cc->a3 == a3))
          {
             if (ret) *ret = EINA_TRUE;
             return;
          }
        goto update_color_class;
     }

   color_class = eina_stringshare_add(color_class);
   if (!color_class) return;
   cc = malloc(sizeof(Edje_Color_Class));
   if (!cc)
     {
        eina_stringshare_del(color_class);
        return;
     }
   cc->name = color_class;
   eina_hash_direct_add(ed->color_classes, cc->name, cc);
update_color_class:
   cc->r = r;
   cc->g = g;
   cc->b = b;
   cc->a = a;
   cc->r2 = r2;
   cc->g2 = g2;
   cc->b2 = b2;
   cc->a2 = a2;
   cc->r3 = r3;
   cc->g3 = g3;
   cc->b3 = b3;
   cc->a3 = a3;
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_TRUE;
#endif

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;

        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_color_class_set(rp->typedata.swallow->swallowed_object,
                                      color_class,
                                      r, g, b, a, r2, g2, b2, a2, r3, g3, b3,
                                      a3);
     }

   _edje_recalc(ed);
   _edje_emit(ed, "color_class,set", color_class);
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_color_class_get(const Evas_Object *obj, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_color_class_get(color_class, r, g, b, a, r2, g2, b2, a2, r3, g3, b3, a3, &ret));
   return ret;
}

void
_color_class_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *color_class = va_arg(*list, const char *);
   int *r = va_arg(*list, int *);
   int *g = va_arg(*list, int *);
   int *b = va_arg(*list, int *);
   int *a = va_arg(*list, int *);
   int *r2 = va_arg(*list, int *);
   int *g2 = va_arg(*list, int *);
   int *b2 = va_arg(*list, int *);
   int *a2 = va_arg(*list, int *);
   int *r3 = va_arg(*list, int *);
   int *g3 = va_arg(*list, int *);
   int *b3 = va_arg(*list, int *);
   int *a3 = va_arg(*list, int *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   const Edje *ed = _pd;
   Edje_Color_Class *cc = _edje_color_class_find(ed, color_class);

   if (cc)
     {
#define X(C) if (C) *C = cc->C
#define S(_r, _g, _b, _a) X(_r); X(_g); X(_b); X(_a)
        S(r, g, b, a);
        S(r2, g2, b2, a2);
        S(r3, g3, b3, a3);
#undef S
#undef X
        if (ret) *ret = EINA_TRUE;
        return;
     }
   else
     {
#define X(C) if (C) *C = 0
#define S(_r, _g, _b, _a) X(_r); X(_g); X(_b); X(_a)
        S(r, g, b, a);
        S(r2, g2, b2, a2);
        S(r3, g3, b3, a3);
#undef S
#undef X
        return;
     }
}

void
edje_object_color_class_del(Evas_Object *obj, const char *color_class)
{
   Edje *ed;
   Edje_Color_Class *cc = NULL;
   unsigned int i;

   ed = _edje_fetch(obj);

   if ((!ed) || (!color_class)) return;

   eina_hash_del(ed->color_classes, color_class, cc);

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;

        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_color_class_del(rp->typedata.swallow->swallowed_object, color_class);
     }

   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_TRUE;
#endif
   _edje_recalc(ed);
   _edje_emit(ed, "color_class,del", color_class);
}

EAPI Eina_Bool
edje_text_class_set(const char *text_class, const char *font, Evas_Font_Size size)
{
   Eina_List *members;
   Edje_Text_Class *tc;

   if (!text_class) return EINA_FALSE;
   if (!font) font = "";

   tc = eina_hash_find(_edje_text_class_hash, text_class);
   /* Create new text class */
   if (!tc)
     {
        tc = calloc(1, sizeof(Edje_Text_Class));
        if (!tc) return EINA_FALSE;
        tc->name = eina_stringshare_add(text_class);
        if (!tc->name)
          {
             free(tc);
             return EINA_FALSE;
          }
        if (!_edje_text_class_hash) _edje_text_class_hash = eina_hash_string_superfast_new(NULL);
        eina_hash_add(_edje_text_class_hash, text_class, tc);

        tc->font = eina_stringshare_add(font);
        tc->size = size;
     }
   else
     {
        /* Match and the same, return */
        if (((tc->font && font) && !strcmp(tc->font, font)) &&
            (tc->size == size))
          return EINA_TRUE;

        /* Update the class found */
        eina_stringshare_replace(&tc->font, font);
        tc->size = size;
     }

   /* Tell all members of the text class to recalc */
   members = eina_hash_find(_edje_text_class_member_hash, text_class);
   while (members)
     {
        Edje *ed;

        ed = eina_list_data_get(members);
        ed->dirty = EINA_TRUE;
        ed->recalc_call = EINA_TRUE;
        _edje_textblock_styles_cache_free(ed, text_class);
        _edje_textblock_style_all_update(ed);
#ifdef EDJE_CALC_CACHE
        ed->text_part_change = EINA_TRUE;
#endif
        _edje_recalc(ed);
        members = eina_list_next(members);
     }
   return EINA_TRUE;
}

void
edje_text_class_del(const char *text_class)
{
   Edje_Text_Class *tc;
   Eina_List *members;

   if (!text_class) return;

   tc = eina_hash_find(_edje_text_class_hash, text_class);
   if (!tc) return;

   eina_hash_del(_edje_text_class_hash, text_class, tc);
   eina_stringshare_del(tc->name);
   eina_stringshare_del(tc->font);
   free(tc);

   members = eina_hash_find(_edje_text_class_member_hash, text_class);
   while (members)
     {
        Edje *ed;

        ed = eina_list_data_get(members);
        ed->dirty = EINA_TRUE;
        _edje_textblock_styles_cache_free(ed, text_class);
        _edje_textblock_style_all_update(ed);
#ifdef EDJE_CALC_CACHE
        ed->text_part_change = EINA_TRUE;
#endif
        _edje_recalc(ed);
        members = eina_list_next(members);
     }
}

Eina_List *
edje_text_class_list(void)
{
   Edje_List_Foreach_Data fdata;

   if (!_edje_text_class_hash) return NULL;
   memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
   eina_hash_foreach(_edje_text_class_hash,
                     _edje_text_class_list_foreach, &fdata);
   return fdata.list;
}

static Eina_Bool
_edje_text_class_list_foreach(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data EINA_UNUSED, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = eina_list_append(fd->list, eina_stringshare_add(key));
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_text_class_set(Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_text_class_set(text_class, font, size, &ret));
   return ret;
}

void
_text_class_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *text_class = va_arg(*list, const char *);
   const char *font = va_arg(*list, const char *);
   Evas_Font_Size size = va_arg(*list, Evas_Font_Size);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   Edje *ed = _pd;
   Eina_List *l;
   Edje_Text_Class *tc = NULL;
   unsigned int i;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!text_class)) return;

   /* for each text_class in the edje */
   EINA_LIST_FOREACH(ed->text_classes, l, tc)
     {
        if ((tc->name) && (!strcmp(tc->name, text_class)))
          {
             /* Match and the same, return */
             if ((tc->size == size) &&
                 ((tc->font == font) ||
                  (tc->font && font && !strcmp(tc->font, font))))
               {
                  if (ret) *ret = EINA_TRUE;
                  return;
               }

             /* Update new text class properties */
             eina_stringshare_replace(&tc->font, font);
             tc->size = size;
             break;
          }
     }

   if (!tc)
     {
        /* No matches, create a new text class */
        tc = calloc(1, sizeof(Edje_Text_Class));
        if (!tc) return;
        tc->name = eina_stringshare_add(text_class);
        if (!tc->name)
          {
             free(tc);
             return;
          }
        tc->font = eina_stringshare_add(font);
        tc->size = size;
        /* Add to edje's text class list */
        ed->text_classes = eina_list_append(ed->text_classes, tc);
     }

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;

        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_text_class_set(rp->typedata.swallow->swallowed_object,
                                     text_class, font, size);
     }

   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ed->text_part_change = EINA_TRUE;
#endif
   _edje_textblock_styles_cache_free(ed, text_class);
   _edje_textblock_style_all_update(ed);
   _edje_recalc(ed);
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_exists(const Evas_Object *obj, const char *part)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_exists(part, &ret));
   return ret;
}

void
_part_exists(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;

   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   *ret = EINA_TRUE;
}

EAPI const Evas_Object *
edje_object_part_object_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   const Evas_Object *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_object_get(part, &ret));
   return ret;
}

void
_part_object_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const Evas_Object **ret = va_arg(*list, const Evas_Object **);
   *ret = NULL;
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   *ret = rp->object;
}

EAPI Eina_Bool
edje_object_part_geometry_get(const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h )
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_geometry_get(part, x, y, w, h, &ret));
   return ret;
}

void
_part_geometry_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part))
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
        return;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
        return;
     }
   if (x) *x = rp->x;
   if (y) *y = rp->y;
   if (w) *w = rp->w;
   if (h) *h = rp->h;
   if (ret) *ret = EINA_TRUE;
}

EAPI void
edje_object_item_provider_set(Evas_Object *obj, Edje_Item_Provider_Cb func, void *data)
{
   if (!obj) return;
   eo_do(obj, edje_obj_item_provider_set(func, data));
}

void
_item_provider_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Edje_Item_Provider_Cb func = va_arg(*list, Edje_Item_Provider_Cb);
   void *data = va_arg(*list, void *);
   Edje *ed = _pd;
   ed->item_provider.func = func;
   ed->item_provider.data = data;
}

/* FIXDOC: New Function */
EAPI void
edje_object_text_change_cb_set(Evas_Object *obj, Edje_Text_Change_Cb func, void *data)
{
   if (!obj) return;
   eo_do(obj, edje_obj_text_change_cb_set(func, data));
}

void
_text_change_cb_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Edje_Text_Change_Cb func = va_arg(*list, Edje_Text_Change_Cb);
   void *data = va_arg(*list, void *);
   Edje *ed = _pd;
   unsigned int i;

   ed->text_change.func = func;
   ed->text_change.data = data;

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;

        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_text_change_cb_set(rp->typedata.swallow->swallowed_object, func, data);
     }
}

Eina_Bool
_edje_object_part_text_raw_set(Edje *ed, Evas_Object *obj, Edje_Real_Part *rp, const char *part, const char *text)
{
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_TRUE;
   if ((!rp->typedata.text->text) && (!text))
     return EINA_TRUE; /* nothing to do, no error */
   if ((rp->typedata.text->text) && (text) &&
       (!strcmp(rp->typedata.text->text, text)))
     return EINA_TRUE; /* nothing to do, no error */
   if (rp->typedata.text->text)
     {
        eina_stringshare_del(rp->typedata.text->text);
        rp->typedata.text->text = NULL;
     }
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_text_markup_set(rp, text);
   else
     if (text) rp->typedata.text->text = eina_stringshare_add(text);
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
   if (ed->text_change.func)
     ed->text_change.func(ed->text_change.data, obj, part);
   return EINA_TRUE;
}

Eina_Bool
_edje_object_part_text_raw_append(Edje *ed, Evas_Object *obj, Edje_Real_Part *rp, const char *part, const char *text)
{
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_TRUE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_text_markup_append(rp, text);
   else if (text)
     {
        if (rp->typedata.text->text)
          {
             char *new = NULL;
             int len_added = strlen(text);
             int len_old = strlen(rp->typedata.text->text);
             new = malloc(len_old + len_added + 1);
             memcpy(new, rp->typedata.text->text, len_old);
             memcpy(new + len_old, text, len_added);
             new[len_old + len_added] = '\0';
             eina_stringshare_replace(&rp->typedata.text->text, new);
             free(new);
          }
        else
          {
             eina_stringshare_replace(&rp->typedata.text->text, text);
          }
     }
   ed->dirty = EINA_TRUE;
   ed->recalc_call = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
   if (ed->text_change.func)
     ed->text_change.func(ed->text_change.data, obj, part);
   return EINA_TRUE;
}

EAPI void
edje_object_part_text_style_user_push(Evas_Object *obj, const char *part,
                                const char *style)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_style_user_push(part, style));
}

void
_part_text_style_user_push(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *style = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Evas_Textblock_Style *ts;

   if ((!ed) || (!part) || (!style)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) return;

   ts = evas_textblock_style_new();
   evas_textblock_style_set(ts, style);
   evas_object_textblock_style_user_push(rp->object, ts);
   evas_textblock_style_free(ts);
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
}

EAPI void
edje_object_part_text_style_user_pop(Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_style_user_pop(part));
}

void
_part_text_style_user_pop(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) return;

   evas_object_textblock_style_user_pop(rp->object);
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
}

EAPI const char *
edje_object_part_text_style_user_peek(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   const char *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_style_user_peek(part, &ret));
   return ret;
}

void
_part_text_style_user_peek(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);
   if (ret) *ret = NULL;
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   const Evas_Textblock_Style *ts;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) return;

   ts = evas_object_textblock_style_user_peek(rp->object);
   if (ts)
      if (ret) *ret = evas_textblock_style_get(ts);
}

static void
_edje_user_define_string(Edje *ed, const char *part, const char *raw_text)
{
   /* NOTE: This one is tricky, text is referenced in rp->typedata.text->text for the life of the
    rp. So on edje_object_file_set, we should first ref it, before destroying the old
    layout. */
   Edje_User_Defined *eud;
   Eina_List *l;

   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (eud->type == EDJE_USER_STRING && !strcmp(eud->part, part))
       {
          if (!raw_text)
            {
               _edje_user_definition_free(eud);
               return;
            }
          eud->u.string.text = raw_text;
          return;
       }

   eud = _edje_user_definition_new(EDJE_USER_STRING, part, ed);
   if (!eud) return;
   eud->u.string.text = raw_text;
}

EAPI Eina_Bool
edje_object_part_text_set(Evas_Object *obj, const char *part, const char *text)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_text_set(part, text, &ret));
   return ret;
}

void
_part_text_set(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *text = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Eina_Bool r;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text))
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }
   r = _edje_object_part_text_raw_set(ed, obj, rp, part, text);
   _edje_user_define_string(ed, part, rp->typedata.text->text);
   if (ret) *ret = r;
}

EAPI const char *
edje_object_part_text_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   const char *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_get(part, &ret));
   return ret;
}

void
_part_text_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_text_get(rp);
        return;
     }
   else
     {
        if (rp->part->type == EDJE_PART_TYPE_TEXT)
          {
             *ret = rp->typedata.text->text;
             return;
          }
        if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
          {
             *ret = evas_object_textblock_text_markup_get(rp->object);
             return;
          }
     }
}

EAPI Eina_Bool
edje_object_part_text_escaped_set(Evas_Object *obj, const char *part, const char *text)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_text_escaped_set(part, text, &ret));
   return ret;
}

void
_part_text_escaped_set(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *text = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Eina_Bool int_ret;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   if ((rp->part->type == EDJE_PART_TYPE_TEXT) && (text))
     {
        Eina_Strbuf *sbuf;
        char *esc_start = NULL, *esc_end = NULL;
        char *s, *p;

        sbuf = eina_strbuf_new();
        p = (char *)text;
        s = p;
        for (;;)
          {
             if ((*p == 0) || (esc_end) || (esc_start))
               {
                  if (esc_end)
                    {
                       const char *escape;

                       escape = evas_textblock_escape_string_range_get
                          (esc_start, esc_end + 1);
                       if (escape) eina_strbuf_append(sbuf, escape);
                       esc_start = esc_end = NULL;
                    }
                  else if (*p == 0)
                    {
                       if (!s) s = esc_start; /* This would happen when there is & that isn't escaped */
                       eina_strbuf_append_length(sbuf, s, p - s);
                       s = NULL;
                    }
                  if (*p == 0)
                    break;
               }

             if (*p == '&')
               {
                  if (!s) s = esc_start; /* This would happen when there is & that isn't escaped */
                  esc_start = p;
                  esc_end = NULL;
                  eina_strbuf_append_length(sbuf, s, p - s);
                  s = NULL;
               }
             else if (*p == ';')
               {
                  if (esc_start)
                    {
                       esc_end = p;
                       s = p + 1;
                    }
               }
             p++;
          }
        int_ret = _edje_object_part_text_raw_set(ed, obj, rp, part, eina_strbuf_string_get(sbuf));
        _edje_user_define_string(ed, part, rp->typedata.text->text);
        eina_strbuf_free(sbuf);
        if (ret) *ret = int_ret;
        return;
     }
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) return;
   int_ret = _edje_object_part_text_raw_set(ed, obj, rp, part, text);
   _edje_user_define_string(ed, part, rp->typedata.text->text);
   if (ret) *ret = int_ret;
}


char *
_edje_text_escape(const char *text)
{
   Eina_Strbuf *txt;
   char *ret;
   const char *text_end;
   size_t text_len;

   if (!text) return NULL;

   txt = eina_strbuf_new();
   text_len = strlen(text);

   text_end = text + text_len;
   while (text < text_end)
     {
        int advance;
        const char *escaped = evas_textblock_string_escape_get(text, &advance);
        if (!escaped)
          {
             eina_strbuf_append_char(txt, text[0]);
             advance = 1;
          }
        else
          eina_strbuf_append(txt, escaped);

        text += advance;
     }

   ret = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   return ret;
}

char *
_edje_text_unescape(const char *text)
{
   Eina_Strbuf *txt;
   char *ret;
   const char *text_end, *last, *escape_start;
   size_t text_len;

   if (!text) return NULL;

   txt = eina_strbuf_new();
   text_len = strlen(text);

   text_end = text + text_len;
   last = text;
   escape_start = NULL;
   for (; text < text_end; text++)
     {
        if (*text == '&')
          {
             size_t len;
             const char *str;

             if (last)
               {
                  len = text - last;
                  str = last;
               }
             else
               {
                  len = text - escape_start;
                  str = escape_start;
               }

             if (len > 0)
               eina_strbuf_append_n(txt, str, len);

             escape_start = text;
             last = NULL;
          }
        else if ((*text == ';') && (escape_start))
          {
             size_t len;
             const char *str = evas_textblock_escape_string_range_get(escape_start, text);

             if (str)
               len = strlen(str);
             else
               {
                  str = escape_start;
                  len = text + 1 - escape_start;
               }

             eina_strbuf_append_n(txt, str, len);

             escape_start = NULL;
             last = text + 1;
          }
     }

   if (!last && escape_start)
     last = escape_start;

   if (last && (text > last))
     {
        size_t len = text - last;
        eina_strbuf_append_n(txt, last, len);
     }

   ret = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   return ret;
}

EAPI Eina_Bool
edje_object_part_text_unescaped_set(Evas_Object *obj, const char *part, const char *text_to_escape)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_text_unescaped_set(part, text_to_escape, &ret));
   return ret;
}

void
_part_text_unescaped_set(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *text_to_escape = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Eina_Bool int_ret = EINA_FALSE;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   if (rp->part->type == EDJE_PART_TYPE_TEXT)
     int_ret = _edje_object_part_text_raw_set(ed, obj, rp, part, text_to_escape);
   else if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     {
        char *text = _edje_text_escape(text_to_escape);

        int_ret = _edje_object_part_text_raw_set(ed, obj, rp, part, text);
        free(text);
     }
   _edje_user_define_string(ed, part, rp->typedata.text->text);
   if (ret) *ret = int_ret;
}

EAPI char *
edje_object_part_text_unescaped_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   char *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_unescaped_get(part, &ret));
   return ret;
}

void
_part_text_unescaped_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   char **ret = va_arg(*list, char **);
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        const char *t = _edje_entry_text_get(rp);
        *ret = _edje_text_unescape(t);
        return;
     }
   else
     {
        if (rp->part->type == EDJE_PART_TYPE_TEXT)
          {
             *ret = strdup(rp->typedata.text->text);
             return;
          }
        if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
          {
             const char *t = evas_object_textblock_text_markup_get(rp->object);
             *ret = _edje_text_unescape(t);
             return;
          }
     }
}

EAPI const char *
edje_object_part_text_selection_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   const char* ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_selection_get(part, &ret));
   return ret;
}

void
_part_text_selection_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
      *ret = _edje_entry_selection_get(rp);
}

EAPI void
edje_object_part_text_select_none(const Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_select_none(part));
}

void
_part_text_select_none(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_none(rp);
}

EAPI void
edje_object_part_text_select_all(const Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_select_all(part));
}

void
_part_text_select_all(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_all(rp);
}

EAPI void
edje_object_part_text_insert(Evas_Object *obj, const char *part, const char *text)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_insert(part, text));
}

void
_part_text_insert(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *text = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   if (rp->part->entry_mode <= EDJE_ENTRY_EDIT_MODE_NONE) return;
   _edje_entry_text_markup_insert(rp, text);
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
   if (ed->text_change.func)
     ed->text_change.func(ed->text_change.data, obj, part);
}

EAPI void
edje_object_part_text_append(Evas_Object *obj, const char *part, const char *text)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_append(part, text));
}

void
_part_text_append(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *text = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   _edje_object_part_text_raw_append(ed, obj, rp, part, text);
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
   if (ed->text_change.func)
     ed->text_change.func(ed->text_change.data, obj, part);
}

EAPI const Eina_List *
edje_object_part_text_anchor_list_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   const Eina_List *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_anchor_list_get(part, &ret));
   return ret;
}

void
_part_text_anchor_list_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     *ret = _edje_entry_anchors_list(rp);
}

EAPI const Eina_List *
edje_object_part_text_anchor_geometry_get(const Evas_Object *obj, const char *part, const char *anchor)
{
   if (!obj) return NULL;
   const Eina_List *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_anchor_geometry_get(part, anchor, &ret));
   return ret;
}

void
_part_text_anchor_geometry_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *anchor = va_arg(*list, const char *);
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     *ret = _edje_entry_anchor_geometry_get(rp, anchor);
}

EAPI const Eina_List *
edje_object_part_text_item_list_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   const Eina_List *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_item_list_get(part, &ret));
   return ret;
}

void
_part_text_item_list_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     *ret = _edje_entry_items_list(rp);
}

EAPI Eina_Bool
edje_object_part_text_item_geometry_get(const Evas_Object *obj, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_text_item_geometry_get(part, item, cx, cy, cw, ch, &ret));
   return ret;
}

void
_part_text_item_geometry_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *item = va_arg(*list, const char *);
   Evas_Coord *cx = va_arg(*list, Evas_Coord *);
   Evas_Coord *cy = va_arg(*list, Evas_Coord *);
   Evas_Coord *cw = va_arg(*list, Evas_Coord *);
   Evas_Coord *ch = va_arg(*list, Evas_Coord *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        Eina_Bool int_ret = _edje_entry_item_geometry_get(rp, item, cx, cy, cw, ch);
        if (ret) *ret = int_ret;
     }
}

EAPI void
edje_object_part_text_cursor_geometry_get(const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_cursor_geometry_get(part, x, y, w, h));
}

void
_part_text_cursor_geometry_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_geometry_get(rp, x, y, w, h);
        if (x) *x -= ed->x;
        if (y) *y -= ed->y;
     }
}

EAPI void
edje_object_part_text_user_insert(const Evas_Object *obj, const char *part, const char *text)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_user_insert(part, text));
}

void
_part_text_user_insert(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *text = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_user_insert(rp, text);
}

EAPI void
edje_object_part_text_select_allow_set(const Evas_Object *obj, const char *part, Eina_Bool allow)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_select_allow_set(part, allow));
}

void
_part_text_select_allow_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool allow = va_arg(*list, int);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_allow_set(rp, allow);
}

EAPI void
edje_object_part_text_select_abort(const Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_select_abort(part));
}

void
_part_text_select_abort(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_abort(rp);
}

EAPI void
edje_object_part_text_select_begin(const Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_select_begin(part));
}

void
_part_text_select_begin(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_begin(rp);
}

EAPI void
edje_object_part_text_select_extend(const Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_select_extend(part));
}

void
_part_text_select_extend(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_extend(rp);
}

EAPI void *
edje_object_part_text_imf_context_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   void *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_imf_context_get(part, &ret));
   return ret;
}

void
_part_text_imf_context_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   void **ret = va_arg(*list, void **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, (char *)part);
   if (!rp) return;

   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     *ret = _edje_entry_imf_context_get(rp);
}

EAPI Eina_Bool
edje_object_part_text_cursor_next(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_text_cursor_next(part, cur, &ret));
   return ret;
}

void
_part_text_cursor_next(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        Eina_Bool int_ret = _edje_entry_cursor_next(rp, cur);
        if (ret) *ret = int_ret;
     }
}

EAPI Eina_Bool
edje_object_part_text_cursor_prev(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_text_cursor_prev(part, cur, &ret));
   return ret;
}

void
_part_text_cursor_prev(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        Eina_Bool int_ret = _edje_entry_cursor_prev(rp, cur);
        if (ret) *ret = int_ret;
     }
}

EAPI Eina_Bool
edje_object_part_text_cursor_up(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_text_cursor_up(part, cur, &ret));
   return ret;
}

void
_part_text_cursor_up(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        Eina_Bool int_ret = _edje_entry_cursor_up(rp, cur);
        if (ret) *ret = int_ret;
     }
}

EAPI Eina_Bool
edje_object_part_text_cursor_down(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_text_cursor_down(part, cur, &ret));
   return ret;
}

void
_part_text_cursor_down(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        Eina_Bool int_ret = _edje_entry_cursor_down(rp, cur);
        if (ret) *ret = int_ret;
     }
}

EAPI void
edje_object_part_text_cursor_begin_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_cursor_begin_set(part, cur));
}

void
_part_text_cursor_begin_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_begin(rp, cur);
     }
}

EAPI void
edje_object_part_text_cursor_end_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_cursor_end_set(part, cur));
}

void
_part_text_cursor_end_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_end(rp, cur);
     }
}

EAPI void
edje_object_part_text_cursor_copy(Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_cursor_copy(part, src, dst));
}

void
_part_text_cursor_copy(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor src = va_arg(*list, Edje_Cursor);
   Edje_Cursor dst = va_arg(*list, Edje_Cursor);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_copy(rp, src, dst);
     }
}

EAPI void
edje_object_part_text_cursor_line_begin_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_cursor_line_begin_set(part, cur));
}

void
_part_text_cursor_line_begin_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_line_begin(rp, cur);
     }
}

EAPI void
edje_object_part_text_cursor_line_end_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_cursor_line_end_set(part, cur));
}

void
_part_text_cursor_line_end_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_line_end(rp, cur);
     }
}

EAPI Eina_Bool
edje_object_part_text_cursor_coord_set(Evas_Object *obj, const char *part,
                                       Edje_Cursor cur, Evas_Coord x,
                                       Evas_Coord y)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_text_cursor_coord_set(part, cur, x, y, &ret));
   return ret;
}

void
_part_text_cursor_coord_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if (ret) *ret = EINA_FALSE;
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        Eina_Bool int_ret = _edje_entry_cursor_coord_set(rp, cur, x, y);
        if (ret) *ret = int_ret;
     }
}

EAPI Eina_Bool
edje_object_part_text_cursor_is_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_text_cursor_is_format_get(part, cur, &ret));
   return ret;
}

void
_part_text_cursor_is_format_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_cursor_is_format_get(rp, cur);
     }
}

EAPI Eina_Bool
edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_text_cursor_is_visible_format_get(part, cur, &ret));
   return ret;
}

void
_part_text_cursor_is_visible_format_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_cursor_is_visible_format_get(rp, cur);
     }
}

EAPI char *
edje_object_part_text_cursor_content_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return NULL;
   char *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_text_cursor_content_get(part, cur, &ret));
   return ret;
}

void
_part_text_cursor_content_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   char **ret = va_arg(*list, char **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_cursor_content_get(rp, cur);
     }
}

EAPI void
edje_object_part_text_cursor_pos_set(Evas_Object *obj, const char *part, Edje_Cursor cur, int pos)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_cursor_pos_set(part, cur, pos));
}

void
_part_text_cursor_pos_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   int pos = va_arg(*list, int);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_pos_set(rp, cur, pos);
     }
}

EAPI int
edje_object_part_text_cursor_pos_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   if (!obj) return 0;
   int ret;
   eo_do((Eo *)obj, edje_obj_part_text_cursor_pos_get(part, cur, &ret));
   return ret;
}

void
_part_text_cursor_pos_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Cursor cur = va_arg(*list, Edje_Cursor);
   int *ret = va_arg(*list, int *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = 0;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_cursor_pos_get(rp, cur);
     }
}

EAPI void
edje_object_part_text_imf_context_reset(const Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_imf_context_reset(part));
}

void
_part_text_imf_context_reset(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_imf_context_reset(rp);
     }
}

EAPI void
edje_object_part_text_input_panel_layout_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Layout layout)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_input_panel_layout_set(part, layout));
}

void
_part_text_input_panel_layout_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Input_Panel_Layout layout = va_arg(*list, Edje_Input_Panel_Layout);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_layout_set(rp, layout);
     }
}

EAPI Edje_Input_Panel_Layout
edje_object_part_text_input_panel_layout_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EDJE_INPUT_PANEL_LAYOUT_INVALID;
   Edje_Input_Panel_Layout ret;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_layout_get(part, &ret));
   return ret;
}

void
_part_text_input_panel_layout_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Input_Panel_Layout *ret = va_arg(*list, Edje_Input_Panel_Layout *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EDJE_INPUT_PANEL_LAYOUT_INVALID;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_input_panel_layout_get(rp);
     }
}

EAPI void
edje_object_part_text_input_panel_layout_variation_set(Evas_Object *obj, const char *part, int variation)
{
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_variation_set(part, variation));
}

void
_part_text_input_panel_layout_variation_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   int variation = va_arg(*list, int);

   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_layout_variation_set(rp, variation);
     }
}

EAPI int
edje_object_part_text_input_panel_layout_variation_get(const Evas_Object *obj, const char *part)
{
   int ret = 0;
   if (obj)
     eo_do((Eo *)obj, edje_obj_part_text_input_panel_variation_get(part, &ret));
   return ret;
}

void
_part_text_input_panel_layout_variation_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   int *r = va_arg(*list, int *);

   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if (*r) *r = 0;
   if ((!ed) || (!part)) return ;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return ;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        if (*r) *r = _edje_entry_input_panel_layout_variation_get(rp);
     }
   return ;
}

EAPI void
edje_object_part_text_autocapital_type_set(Evas_Object *obj, const char *part, Edje_Text_Autocapital_Type autocapital_type)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_autocapital_type_set(part, autocapital_type));
}

void
_part_text_autocapital_type_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Text_Autocapital_Type autocapital_type = va_arg(*list, Edje_Text_Autocapital_Type);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_autocapital_type_set(rp, autocapital_type);
     }
}

EAPI Edje_Text_Autocapital_Type
edje_object_part_text_autocapital_type_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EDJE_TEXT_AUTOCAPITAL_TYPE_NONE;
   Edje_Text_Autocapital_Type ret;
   eo_do((Eo *)obj, edje_obj_part_text_autocapital_type_get(part, &ret));
   return ret;
}

void
_part_text_autocapital_type_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Text_Autocapital_Type *ret = va_arg(*list, Edje_Text_Autocapital_Type *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EDJE_TEXT_AUTOCAPITAL_TYPE_NONE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_autocapital_type_get(rp);
     }
}

EAPI void
edje_object_part_text_prediction_allow_set(Evas_Object *obj, const char *part, Eina_Bool prediction)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_prediction_allow_set(part, prediction));
}

void
_part_text_prediction_allow_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool prediction = va_arg(*list, int);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_prediction_allow_set(rp, prediction);
     }
}

EAPI Eina_Bool
edje_object_part_text_prediction_allow_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_text_prediction_allow_get(part, &ret));
   return ret;
}

void
_part_text_prediction_allow_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_prediction_allow_get(rp);
     }
}

EAPI void
edje_object_part_text_input_panel_enabled_set(Evas_Object *obj, const char *part, Eina_Bool enabled)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_input_panel_enabled_set(part, enabled));
}

void
_part_text_input_panel_enabled_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool enabled = va_arg(*list, int);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_enabled_set(rp, enabled);
     }
}

EAPI Eina_Bool
edje_object_part_text_input_panel_enabled_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_enabled_get(part, &ret));
   return ret;
}

void
_part_text_input_panel_enabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_input_panel_enabled_get(rp);
     }
}

EAPI void
edje_object_part_text_input_panel_show(const Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_show(part));
}

void
_part_text_input_panel_show(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_input_panel_show(rp);
}

EAPI void
edje_object_part_text_input_panel_hide(const Evas_Object *obj, const char *part)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_hide(part));
}

void
_part_text_input_panel_hide(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_input_panel_hide(rp);
}

EAPI void
edje_object_part_text_input_panel_language_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Lang lang)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_input_panel_language_set(part, lang));
}

void
_part_text_input_panel_language_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Input_Panel_Lang lang = va_arg(*list, Edje_Input_Panel_Lang);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_language_set(rp, lang);
     }
}

EAPI Edje_Input_Panel_Lang
edje_object_part_text_input_panel_language_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EDJE_INPUT_PANEL_LANG_AUTOMATIC;
   Edje_Input_Panel_Lang ret;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_language_get(part, &ret));
   return ret;
}

void
_part_text_input_panel_language_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Input_Panel_Lang *ret = va_arg(*list, Edje_Input_Panel_Lang *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EDJE_INPUT_PANEL_LANG_AUTOMATIC;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_input_panel_language_get(rp);
     }
}

EAPI void
edje_object_part_text_input_panel_imdata_set(Evas_Object *obj, const char *part, const void *data, int len)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_input_panel_imdata_set(part, data, len));
}

void
_part_text_input_panel_imdata_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const void *data = va_arg(*list, const void *);
   int len = va_arg(*list, int);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_imdata_set(rp, data, len);
     }
}

EAPI void
edje_object_part_text_input_panel_imdata_get(const Evas_Object *obj, const char *part, void *data, int *len)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_imdata_get(part, data, len));
}

void
_part_text_input_panel_imdata_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   void *data = va_arg(*list, void *);
   int *len = va_arg(*list, int *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_imdata_get(rp, data, len);
     }
}

EAPI void
edje_object_part_text_input_panel_return_key_type_set(Evas_Object *obj, const char *part, Edje_Input_Panel_Return_Key_Type return_key_type)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_input_panel_return_key_type_set(part, return_key_type));
}

void
_part_text_input_panel_return_key_type_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Input_Panel_Return_Key_Type return_key_type = va_arg(*list, Edje_Input_Panel_Return_Key_Type);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_return_key_type_set(rp, return_key_type);
     }
}

EAPI Edje_Input_Panel_Return_Key_Type
edje_object_part_text_input_panel_return_key_type_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EDJE_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
   Edje_Input_Panel_Return_Key_Type ret;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_return_key_type_get(part, &ret));
   return ret;
}

void
_part_text_input_panel_return_key_type_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Input_Panel_Return_Key_Type *ret = va_arg(*list, Edje_Input_Panel_Return_Key_Type *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EDJE_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_input_panel_return_key_type_get(rp);
     }
}

EAPI void
edje_object_part_text_input_panel_return_key_disabled_set(Evas_Object *obj, const char *part, Eina_Bool disabled)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_input_panel_return_key_disabled_set(part, disabled));
}

void
_part_text_input_panel_return_key_disabled_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool disabled = va_arg(*list, int);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_return_key_disabled_set(rp, disabled);
     }
}

EAPI Eina_Bool
edje_object_part_text_input_panel_return_key_disabled_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_return_key_disabled_get(part, &ret));
   return ret;
}

void
_part_text_input_panel_return_key_disabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_input_panel_return_key_disabled_get(rp);
     }
}

EAPI void
edje_object_part_text_input_panel_show_on_demand_set(Evas_Object *obj, const char *part, Eina_Bool ondemand)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_text_input_panel_show_on_demand_set(part, ondemand));
}

void _part_text_input_panel_show_on_demand_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool ondemand = va_arg(*list, int);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_input_panel_show_on_demand_set(rp, ondemand);
     }
}

EAPI Eina_Bool
edje_object_part_text_input_panel_show_on_demand_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_text_input_panel_show_on_demand_get(part, &ret));
   return ret;
}

void _part_text_input_panel_show_on_demand_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        *ret = _edje_entry_input_panel_show_on_demand_get(rp);
     }
}

EAPI void
edje_object_text_insert_filter_callback_add(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data)
{
   if (!obj) return;
   eo_do(obj, edje_obj_text_insert_filter_callback_add(part, func, data));
}

void
_text_insert_filter_callback_add(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Text_Filter_Cb func = va_arg(*list, Edje_Text_Filter_Cb);
   void *data = va_arg(*list, void *);
   Edje *ed = _pd;
   Edje_Text_Insert_Filter_Callback *cb;

   if ((!ed) || (!part)) return;
   cb = calloc(1, sizeof(Edje_Text_Insert_Filter_Callback));
   cb->part = eina_stringshare_add(part);
   cb->func = func;
   cb->data = (void *)data;
   ed->text_insert_filter_callbacks =
     eina_list_append(ed->text_insert_filter_callbacks, cb);
}

EAPI void *
edje_object_text_insert_filter_callback_del(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func)
{
   if (!obj) return NULL;
   void *ret = NULL;
   eo_do(obj, edje_obj_text_insert_filter_callback_del(part, func, &ret));
   return ret;
}

void
_text_insert_filter_callback_del(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Text_Filter_Cb func = va_arg(*list, Edje_Text_Filter_Cb);
   void **ret = va_arg(*list, void **);
   Edje *ed = _pd;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;
   if (ret) *ret = NULL;

   if ((!ed) || (!part)) return;
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func))
          {
             void *data = cb->data;
             ed->text_insert_filter_callbacks =
                eina_list_remove_list(ed->text_insert_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             if (ret) *ret = data;
             return;
          }
     }
}

EAPI void *
edje_object_text_insert_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data)
{
   if (!obj) return NULL;
   void *ret = NULL;
   eo_do(obj, edje_obj_text_insert_filter_callback_del_full(part, func, data, &ret));
   return ret;
}

void
_text_insert_filter_callback_del_full(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Text_Filter_Cb func = va_arg(*list, Edje_Text_Filter_Cb);
   void *data = va_arg(*list, void *);
   void **ret = va_arg(*list, void **);
   Edje *ed = _pd;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;
   if (ret) *ret = NULL;

   if ((!ed) || (!part)) return;
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func) &&
            (cb->data == data))
          {
             void *tmp = cb->data;
             ed->text_insert_filter_callbacks =
                eina_list_remove_list(ed->text_insert_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             if (ret) *ret = tmp;
             return;
          }
     }
}

EAPI void
edje_object_text_markup_filter_callback_add(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data)
{
   if (!obj) return;
   eo_do(obj, edje_obj_text_markup_filter_callback_add(part, func, data));
}

void
_text_markup_filter_callback_add(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Markup_Filter_Cb func = va_arg(*list, Edje_Markup_Filter_Cb);
   void *data = va_arg(*list, void *);
   Edje *ed = _pd;
   Edje_Markup_Filter_Callback *cb;

   if ((!ed) || (!part)) return;
   cb = calloc(1, sizeof(Edje_Markup_Filter_Callback));
   cb->part = eina_stringshare_add(part);
   cb->func = func;
   cb->data = (void *)data;
   ed->markup_filter_callbacks =
     eina_list_append(ed->markup_filter_callbacks, cb);
}

EAPI void *
edje_object_text_markup_filter_callback_del(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func)
{
   if (!obj) return NULL;
   void *ret = NULL;
   eo_do(obj, edje_obj_text_markup_filter_callback_del(part, func, &ret));
   return ret;
}

void
_text_markup_filter_callback_del(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Markup_Filter_Cb func = va_arg(*list, Edje_Markup_Filter_Cb);
   void **ret = va_arg(*list, void **);
   Edje *ed = _pd;
   Edje_Markup_Filter_Callback *cb;
   Eina_List *l;
   if (ret) *ret = NULL;

   if ((!ed) || (!part)) return;
   EINA_LIST_FOREACH(ed->markup_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func))
          {
             void *data = cb->data;
             ed->markup_filter_callbacks =
                eina_list_remove_list(ed->markup_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             if (ret) *ret = data;
             return;
          }
     }
}

EAPI void *
edje_object_text_markup_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Markup_Filter_Cb func, void *data)
{
   if (!obj) return NULL;
   void *ret = NULL;
   eo_do(obj, edje_obj_text_markup_filter_callback_del_full(part, func, data, &ret));
   return ret;
}

void
_text_markup_filter_callback_del_full(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Markup_Filter_Cb func = va_arg(*list, Edje_Markup_Filter_Cb);
   void *data = va_arg(*list, void *);
   void **ret = va_arg(*list, void **);
   Edje *ed = _pd;
   Edje_Markup_Filter_Callback *cb;
   Eina_List *l;
   if (ret) *ret = NULL;

   if ((!ed) || (!part)) return;
   EINA_LIST_FOREACH(ed->markup_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func) &&
            (cb->data == data))
          {
             void *tmp = cb->data;
             ed->markup_filter_callbacks =
                eina_list_remove_list(ed->markup_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             if (ret) *ret = tmp;
             return;
          }
     }
}

EAPI Eina_Bool
edje_object_part_swallow(Evas_Object *obj, const char *part, Evas_Object *obj_swallow)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_swallow(part, obj_swallow, &ret));
   return ret;
}

void
_part_swallow(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *obj_swallow = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp, *rpcur;
   Edje_User_Defined *eud = NULL;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   /* Need to recalc before providing the object. */
   // XXX: I guess this is not required, removing for testing purposes
   // XXX: uncomment if you see glitches in e17 or others.
   // XXX: by Gustavo, January 21th 2009.
   // XXX: I got a backtrace with over 30000 calls without this,
   // XXX: only with 32px shelves. The problem is probably somewhere else,
   // XXX: but until it's found, leave this here.
   // XXX: by Sachiel, January 21th 2009, 19:30 UTC
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   rpcur = evas_object_data_get(obj_swallow, "\377 edje.swallowing_part");
   if (rpcur)
     {
        /* the object is already swallowed in the requested part */
        if (rpcur == rp)
          {
             *ret = EINA_TRUE;
             return;
          }
        /* The object is already swallowed somewhere, unswallow it first */
        edje_object_part_unswallow(ed->obj, obj_swallow);
     }

   if (!rp)
     {
        DBG("cannot swallow part %s: part not exist!", part);
        return;
     }
   if (rp->part->type != EDJE_PART_TYPE_SWALLOW)
     {
        ERR("cannot swallow part %s: not swallow type!", rp->part->name);
        return;
     }
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return;
   _edje_real_part_swallow(ed, rp, obj_swallow, EINA_TRUE);

   if (rp->typedata.swallow->swallowed_object)
     {
        if (!eud)
          {
             eud = _edje_user_definition_new(EDJE_USER_SWALLOW, part, ed);
             evas_object_event_callback_add(obj_swallow, EVAS_CALLBACK_DEL, _edje_user_def_del_cb, eud);
          }
        else ed->user_defined = eina_list_append(ed->user_defined, eud);

        if (eud)
          {
             eud->u.swallow.child = obj_swallow;
          }
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_recalc_extern_parent(Evas_Object *obj)
{
   Evas_Object *parent;
   Edje *ed;

   parent = evas_object_smart_parent_get(obj);
   if (!(ed = _edje_fetch(parent))) return;

   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE; // ZZZ: ???
   _edje_recalc(ed);
}

EAPI void
edje_extern_object_min_size_set(Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   if (!obj) return;
   Edje_Real_Part *rp;

   evas_object_size_hint_min_set(obj, minw, minh);
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp)
     {
        if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
            (!rp->typedata.swallow)) return;
        rp->typedata.swallow->swallow_params.min.w = minw;
        rp->typedata.swallow->swallow_params.min.h = minh;

        _recalc_extern_parent(obj);
     }
}

EAPI void
edje_extern_object_max_size_set(Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh)
{
   if (!obj) return;
   Edje_Real_Part *rp;

   evas_object_size_hint_max_set(obj, maxw, maxh);
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp)
     {
        if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
            (!rp->typedata.swallow)) return;
        rp->typedata.swallow->swallow_params.max.w = maxw;
        rp->typedata.swallow->swallow_params.max.h = maxh;

        _recalc_extern_parent(obj);
     }
}

EAPI void
edje_extern_object_aspect_set(Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah)
{
   if (!obj) return;
   Edje_Real_Part *rp;
   Evas_Aspect_Control asp;

   asp = EVAS_ASPECT_CONTROL_NONE;
   switch (aspect)
     {
      case EDJE_ASPECT_CONTROL_NONE: asp = EVAS_ASPECT_CONTROL_NONE; break;
      case EDJE_ASPECT_CONTROL_NEITHER: asp = EVAS_ASPECT_CONTROL_NEITHER; break;
      case EDJE_ASPECT_CONTROL_HORIZONTAL: asp = EVAS_ASPECT_CONTROL_HORIZONTAL; break;
      case EDJE_ASPECT_CONTROL_VERTICAL: asp = EVAS_ASPECT_CONTROL_VERTICAL; break;
      case EDJE_ASPECT_CONTROL_BOTH: asp = EVAS_ASPECT_CONTROL_BOTH; break;
      default: break;
     }
   if (aw < 1) aw = 1;
   if (ah < 1) ah = 1;
   evas_object_size_hint_aspect_set(obj, asp, aw, ah);
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp)
     {
        if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
            (!rp->typedata.swallow)) return;
        rp->typedata.swallow->swallow_params.aspect.mode = aspect;
        rp->typedata.swallow->swallow_params.aspect.w = aw;
        rp->typedata.swallow->swallow_params.aspect.h = ah;
        _recalc_extern_parent(obj);
     }
}

struct edje_box_layout_builtin {
   const char *name;
   Evas_Object_Box_Layout cb;
};

static Evas_Object_Box_Layout
_edje_box_layout_builtin_find(const char *name)
{
   const struct edje_box_layout_builtin _edje_box_layout_builtin[] = {
     {"horizontal", evas_object_box_layout_horizontal},
     {"horizontal_flow", evas_object_box_layout_flow_horizontal},
     {"horizontal_homogeneous", evas_object_box_layout_homogeneous_horizontal},
     {"horizontal_max", evas_object_box_layout_homogeneous_max_size_horizontal},
     {"stack", evas_object_box_layout_stack},
     {"vertical", evas_object_box_layout_vertical},
     {"vertical_flow", evas_object_box_layout_flow_vertical},
     {"vertical_homogeneous", evas_object_box_layout_homogeneous_vertical},
     {"vertical_max", evas_object_box_layout_homogeneous_max_size_vertical},
     {NULL, NULL}
   };
   const struct edje_box_layout_builtin *base;

   switch (name[0])
     {
      case 'h':
         base = _edje_box_layout_builtin + 0;
         break;
      case 's':
         base = _edje_box_layout_builtin + 4;
         break;
      case 'v':
         base = _edje_box_layout_builtin + 5;
         break;
      default:
         return NULL;
     }

   for (; (base->name) && (base->name[0] == name[0]); base++)
     if (strcmp(base->name, name) == 0)
       return base->cb;

   return NULL;
}

static Eina_Rbtree_Direction
_edje_box_layout_external_node_cmp(const Eina_Rbtree *left, const Eina_Rbtree *right, EINA_UNUSED void *data)
{
   Edje_Box_Layout *l = (Edje_Box_Layout *)left;
   Edje_Box_Layout *r = (Edje_Box_Layout *)right;

   if (strcmp(l->name, r->name) < 0)
     return EINA_RBTREE_RIGHT;
   else
     return EINA_RBTREE_LEFT;
}

static int
_edje_box_layout_external_find_cmp(const Eina_Rbtree *node, const void *key, EINA_UNUSED int length, EINA_UNUSED void *data)
{
   Edje_Box_Layout *l = (Edje_Box_Layout *)node;
   return strcmp(key, l->name);
}

static Edje_Box_Layout *
_edje_box_layout_external_find(const char *name)
{
   return (Edje_Box_Layout *)eina_rbtree_inline_lookup
     (_edje_box_layout_registry, name, 0, _edje_box_layout_external_find_cmp,
      NULL);
}

Eina_Bool
_edje_box_layout_find(const char *name, Evas_Object_Box_Layout *cb, void **data, void (**free_data)(void *data))
{
   const Edje_Box_Layout *l;

   if (!name) return EINA_FALSE;

   *cb = _edje_box_layout_builtin_find(name);
   if (*cb)
     {
        *free_data = NULL;
        *data = NULL;
        return EINA_TRUE;
     }

   l = _edje_box_layout_external_find(name);
   if (!l) return EINA_FALSE;

   *cb = l->func;
   *free_data = l->layout_data_free;
   if (l->layout_data_get)
     *data = l->layout_data_get(l->data);
   else
     *data = NULL;

   return EINA_TRUE;
}

static void
_edje_box_layout_external_free(Eina_Rbtree *node, EINA_UNUSED void *data)
{
   Edje_Box_Layout *l = (Edje_Box_Layout *)node;

   if (l->data && l->free_data)
     l->free_data(l->data);
   free(l);
}

static Edje_Box_Layout *
_edje_box_layout_external_new(const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data)
{
   Edje_Box_Layout *l;
   size_t name_len;

   name_len = strlen(name) + 1;
   l = malloc(sizeof(Edje_Box_Layout) + name_len);
   if (!l) return NULL;

   l->func = func;
   l->layout_data_get = layout_data_get;
   l->layout_data_free = layout_data_free;
   l->free_data = free_data;
   l->data = data;

   memcpy(l->name, name, name_len);

   return l;
}

EAPI void
edje_box_layout_register(const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data)
{
   Edje_Box_Layout *l;

   if (!name) return;

   if (_edje_box_layout_builtin_find(name))
     {
        ERR("Cannot register layout '%s': would override builtin!",
            name);

        if (data && free_data) free_data(data);
        return;
     }

   l = _edje_box_layout_external_find(name);
   if (!l)
     {
        if (!func)
          {
             if (data && free_data) free_data(data);
             return;
          }

        l = _edje_box_layout_external_new
           (name, func, layout_data_get, layout_data_free, free_data, data);
        if (!l)
          return;

        _edje_box_layout_registry = eina_rbtree_inline_insert
           (_edje_box_layout_registry, (Eina_Rbtree *)l,
            _edje_box_layout_external_node_cmp, NULL);
     }
   else
     {
        if (func)
          {
             if (l->data && l->free_data) l->free_data(l->data);

             l->func = func;
             l->layout_data_get = layout_data_get;
             l->layout_data_free = layout_data_free;
             l->free_data = free_data;
             l->data = data;
          }
        else
          {
             if (data && free_data) free_data(data);

             _edje_box_layout_registry = eina_rbtree_inline_remove
                (_edje_box_layout_registry, (Eina_Rbtree *)l,
                 _edje_box_layout_external_node_cmp, NULL);
             _edje_box_layout_external_free((Eina_Rbtree *)l, NULL);
          }
     }
}

EAPI void
edje_object_part_unswallow(Evas_Object *obj, Evas_Object *obj_swallow)
{
   if (!obj) return;
   eo_do(obj, edje_obj_part_unswallow(obj_swallow));
}

void
_part_unswallow(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *obj_swallow = va_arg(*list, Evas_Object *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;

   if (!obj_swallow) return;

   rp = (Edje_Real_Part *)evas_object_data_get(obj_swallow, "\377 edje.swallowing_part");
   if (!rp)
     return;
   if (rp->part->type != EDJE_PART_TYPE_SWALLOW)
     {
        ERR("cannot unswallow part %s: not swallow type!", rp->part->name);
        return;
     }

   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return;

   if (rp->typedata.swallow->swallowed_object == obj_swallow)
     {
        Edje_User_Defined *eud;
        Eina_List *l;

        if (obj)
          {
             if (!ed && obj)
               {
                  ERR("edje_object_part_unswallow called on a non Edje object ('%s').",
                      evas_object_type_get(obj));
               }
             else
               {
                  EINA_LIST_FOREACH(ed->user_defined, l, eud)
                    if (eud->type == EDJE_USER_SWALLOW && eud->u.swallow.child == obj_swallow)
                      {
                         _edje_user_definition_free(eud);
                         return;
                      }
               }
          }

        _edje_real_part_swallow_clear(ed, rp);
        rp->typedata.swallow->swallowed_object = NULL;
        rp->typedata.swallow->swallow_params.min.w = 0;
        rp->typedata.swallow->swallow_params.min.h = 0;
        rp->typedata.swallow->swallow_params.max.w = 0;
        rp->typedata.swallow->swallow_params.max.h = 0;
        ed->dirty = EINA_TRUE;
        ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
        rp->invalidate = EINA_TRUE;
#endif
        _edje_recalc_do(ed);
        return;
     }
}

EAPI Evas_Object *
edje_object_part_swallow_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_swallow_get(part, &ret));
   return ret;
}

void
_part_swallow_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return;
   *ret = rp->typedata.swallow->swallowed_object;
}

EAPI void
edje_object_size_min_get(const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_size_min_get(minw, minh));
}

void
_size_min_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *minw = va_arg(*list, Evas_Coord *);
   Evas_Coord *minh = va_arg(*list, Evas_Coord *);
   Edje *ed = _pd;

   if ((!ed) || (!ed->collection))
     {
        if (minw) *minw = 0;
        if (minh) *minh = 0;
        return;
     }
   if (minw) *minw = ed->collection->prop.min.w;
   if (minh) *minh = ed->collection->prop.min.h;
}

EAPI void
edje_object_size_max_get(const Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_size_max_get(maxw, maxh));
}

void
_size_max_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *maxw = va_arg(*list, Evas_Coord *);
   Evas_Coord *maxh = va_arg(*list, Evas_Coord *);
   Edje *ed = (Edje *)_pd;

   if ((!ed) || (!ed->collection))
     {
        if (maxw) *maxw = 0;
        if (maxh) *maxh = 0;
        return;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   if (ed->collection->prop.max.w == 0)
     {
        /* XXX TODO: convert maxw to 0, fix things that break. */
        if (maxw) *maxw = EDJE_INF_MAX_W;
     }
   else
     {
        if (maxw) *maxw = ed->collection->prop.max.w;
     }
   if (ed->collection->prop.max.h == 0)
     {
        /* XXX TODO: convert maxh to 0, fix things that break. */
        if (maxh) *maxh = EDJE_INF_MAX_H;
     }
   else
     {
        if (maxh) *maxh = ed->collection->prop.max.h;
     }
}

EAPI void
edje_object_calc_force(Evas_Object *obj)
{
   if (!obj) return;
   eo_do(obj, edje_obj_calc_force());
}

void
_calc_force(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Edje *ed = _pd;
   int pf, pf2;

   if (!ed) return;
   ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_TRUE;
#endif

   pf2 = _edje_freeze_val;
   pf = ed->freeze;

   _edje_freeze_val = 0;
   ed->freeze = 0;

   _edje_recalc_do(ed);

   ed->freeze = pf;
   _edje_freeze_val = pf2;
}

EAPI void
edje_object_size_min_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   if (!obj) return;
   eo_do(obj, edje_obj_size_min_calc(minw, minh));
}

void
_size_min_calc(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord *minw = va_arg(*list, Evas_Coord *);
   Evas_Coord *minh = va_arg(*list, Evas_Coord *);
   edje_object_size_min_restricted_calc(obj, minw, minh, 0, 0);
}

EAPI Eina_Bool
edje_object_parts_extends_calc(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_parts_extends_calc(x, y, w, h, &ret));
   return ret;
}

void
_parts_extends_calc(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Evas_Coord xx1 = INT_MAX, yy1 = INT_MAX;
   Evas_Coord xx2 = 0, yy2 = 0;
   unsigned int i;
   if (ret) *ret = EINA_FALSE;

   if (!ed)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
        return;
     }

   ed->calc_only = EINA_TRUE;

   /* Need to recalc before providing the object. */
   ed->dirty = EINA_TRUE;
   _edje_recalc_do(ed);

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;
        Evas_Coord rpx1, rpy1;
        Evas_Coord rpx2, rpy2;

        rp = ed->table_parts[i];

        rpx1 = rp->x;
        rpy1 = rp->y;
        rpx2 = rpx1 + rp->w;
        rpy2 = rpy1 + rp->h;

        if (xx1 > rpx1) xx1 = rpx1;
        if (yy1 > rpy1) yy1 = rpy1;
        if (xx2 < rpx2) xx2 = rpx2;
        if (yy2 < rpy2) yy2 = rpy2;
     }

   ed->calc_only = EINA_FALSE;

   if (x) *x = xx1;
   if (y) *y = yy1;
   if (w) *w = xx2 - xx1;
   if (h) *h = yy2 - yy1;

   if (ret) *ret = EINA_TRUE;
}

EAPI void
edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh)
{
   if (!obj) return;
   eo_do(obj, edje_obj_size_min_restricted_calc(minw, minh, restrictedw, restrictedh));
}

void
_size_min_restricted_calc(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *minw = va_arg(*list, Evas_Coord *);
   Evas_Coord *minh = va_arg(*list, Evas_Coord *);
   Evas_Coord restrictedw = va_arg(*list, Evas_Coord);
   Evas_Coord restrictedh = va_arg(*list, Evas_Coord);
   Edje *ed = _pd;
   Evas_Coord pw, ph;
   int maxw, maxh;
   int okw, okh;
   int reset_maxwh;
   Edje_Real_Part *pep = NULL;
   Eina_Bool has_non_fixed_tb = EINA_FALSE;

   if ((!ed) || (!ed->collection))
     {
        if (minw) *minw = restrictedw;
        if (minh) *minh = restrictedh;
        return;
     }
   reset_maxwh = 1;
   ed->calc_only = EINA_TRUE;
   pw = ed->w;
   ph = ed->h;

   again:
   ed->w = restrictedw;
   ed->h = restrictedh;

   maxw = 0;
   maxh = 0;

   do
     {
        unsigned int i;

        okw = okh = 0;
        ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
        ed->all_part_change = EINA_TRUE;
#endif
        _edje_recalc_do(ed);
        if (reset_maxwh)
          {
             maxw = 0;
             maxh = 0;
          }
        pep = NULL;
        has_non_fixed_tb = EINA_FALSE;
        for (i = 0; i < ed->table_parts_size; i++)
          {
             Edje_Real_Part *ep;
             int w, h;
             int didw;

             ep = ed->table_parts[i];
             w = ep->w - ep->req.w;
             h = ep->h - ep->req.h;
             didw = 0;
             if (ep->chosen_description)
               {
                  if (!ep->chosen_description->fixed.w)
                    {
                       if (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK)
                         {
                            Evas_Coord tb_mw;
                            evas_object_textblock_size_formatted_get(ep->object,
                                                                     &tb_mw, NULL);
                            tb_mw -= ep->req.w;
                            if (tb_mw > w)
                              {
                                 w = tb_mw;
                              }
                            has_non_fixed_tb = EINA_TRUE;
                         }
                       if (w > maxw)
                         {
                            maxw = w;
                            okw = 1;
                            pep = ep;
                            didw = 1;
                         }
                    }
                  if (!ep->chosen_description->fixed.h)
                    {
                       if (!((ep->part->type == EDJE_PART_TYPE_TEXTBLOCK) &&
                             (!((Edje_Part_Description_Text *)ep->chosen_description)->text.min_x) &&
                             (didw)))
                         {
                            if (h > maxh)
                              {
                                 maxh = h;
                                 okh = 1;
                                 pep = ep;
                              }
                         }

                       if (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK)
                         {
                            has_non_fixed_tb = EINA_TRUE;
                         }
                    }
               }
          }
        if (okw)
          {
             ed->w += maxw;
             if (ed->w < restrictedw) ed->w = restrictedw;
          }
        if (okh)
          {
             ed->h += maxh;
             if (ed->h < restrictedh) ed->h = restrictedh;
          }
        if ((ed->w > 4000) || (ed->h > 4000))
          {
             /* Only print it if we have a non-fixed textblock.
              * We should possibly avoid all of this if in this case, but in
              * the meanwhile, just doing this. */
             if (!has_non_fixed_tb)
               {
                  if (pep)
                    ERR("file %s, group %s has a non-fixed part '%s'. Adding 'fixed: 1 1;' to source EDC may help. Continuing discarding faulty part.",
                        ed->path, ed->group, pep->part->name);
                  else
                    ERR("file %s, group %s overflowed 4000x4000 with minimum size of %dx%d. Continuing discarding faulty parts.",
                        ed->path, ed->group, ed->w, ed->h);
               }

             if (reset_maxwh)
               {
                  reset_maxwh = 0;
                  goto again;
               }
          }
     }
   while (okw || okh);
   ed->min.w = ed->w;
   ed->min.h = ed->h;

   if (minw) *minw = ed->min.w;
   if (minh) *minh = ed->min.h;

   ed->w = pw;
   ed->h = ph;
   ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_TRUE;
#endif
   _edje_recalc(ed);
   ed->calc_only = EINA_FALSE;
}

/* FIXME: Correctly return other states */
EAPI const char *
edje_object_part_state_get(const Evas_Object *obj, const char *part, double *val_ret)
{
   if (!obj) return NULL;
   const char *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_state_get(part, val_ret, &ret));
   return ret;
}

void
_part_state_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double *val_ret = va_arg(*list, double *);
   const char **ret = va_arg(*list, const char **);
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;
   *ret = "";

   if ((!ed) || (!part))
     {
        if (val_ret) *val_ret = 0;
        return;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp)
     {
        if (val_ret) *val_ret = 0;
        INF("part not found");
        return;
     }
   if (rp->chosen_description)
     {
        if (val_ret) *val_ret = rp->chosen_description->state.value;
        if (rp->chosen_description->state.name)
          *ret = rp->chosen_description->state.name;
        else
          *ret = "default";
        return;
     }
   else
     {
        if (rp->param1.description)
          {
             if (val_ret) *val_ret = rp->param1.description->state.value;
             if (rp->param1.description->state.name)
               *ret = rp->param1.description->state.name;
             else
               *ret = "default";
             return;
          }
     }
   if (val_ret) *val_ret = 0;
}

EAPI Edje_Drag_Dir
edje_object_part_drag_dir_get(const Evas_Object *obj, const char *part)
{
   if (!obj) return EDJE_DRAG_DIR_NONE;
   Edje_Drag_Dir ret;
   eo_do((Eo *)obj, edje_obj_part_drag_dir_get(part, &ret));
   return ret;
}

void
_part_drag_dir_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Edje_Drag_Dir *ret = va_arg(*list, Edje_Drag_Dir *);
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;
   *ret = EDJE_DRAG_DIR_NONE;

   if ((!ed) || (!part)) return;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->part->dragable.x) && (rp->part->dragable.y)) *ret = EDJE_DRAG_DIR_XY;
   else if (rp->part->dragable.x) *ret = EDJE_DRAG_DIR_X;
   else if (rp->part->dragable.y) *ret = EDJE_DRAG_DIR_Y;
}

EAPI Eina_Bool
edje_object_part_drag_value_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_drag_value_set(part, dx, dy, &ret));
   return ret;
}

void
_part_drag_value_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double dx = va_arg(*list, double);
   double dy = va_arg(*list, double);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Edje_User_Defined *eud;
   Eina_List *l;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (!rp->drag) return;
   if (rp->drag->down.count > 0) return;

   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (eud->type == EDJE_USER_DRAG_VALUE && !strcmp(part, eud->part))
       {
          eud->u.drag_position.x = dx;
          eud->u.drag_position.y = dy;
          break;
       }
   if (!eud)
     {
        eud = _edje_user_definition_new(EDJE_USER_DRAG_VALUE, part, ed);
        if (eud)
          {
             eud->u.drag_position.x = dx;
             eud->u.drag_position.y = dy;
          }
     }

   if (rp->part->dragable.confine_id != -1)
     {
        dx = CLAMP(dx, 0.0, 1.0);
        dy = CLAMP(dy, 0.0, 1.0);
     }
   if (rp->part->dragable.x < 0) dx = 1.0 - dx;
   if (rp->part->dragable.y < 0) dy = 1.0 - dy;
   if ((rp->drag->val.x == FROM_DOUBLE(dx)) && (rp->drag->val.y == FROM_DOUBLE(dy)))
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }
   rp->drag->val.x = FROM_DOUBLE(dx);
   rp->drag->val.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_dragable_pos_set(ed, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(ed, "drag,set", rp->part->name);
   if (ret) *ret = EINA_TRUE;
}

/* FIXME: Should this be x and y instead of dx/dy? */
EAPI Eina_Bool
edje_object_part_drag_value_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_drag_value_get(part, dx, dy, &ret));
   return ret;
}

void
_part_drag_value_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double *dx = va_arg(*list, double *);
   double *dy = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;
   double ddx, ddy;
   *ret = EINA_FALSE;

   if ((!ed) || (!part))
     {
        if (dx) *dx = 0;
        if (dy) *dy = 0;
        return;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp || !rp->drag)
     {
        if (dx) *dx = 0;
        if (dy) *dy = 0;
        return;
     }
   ddx = TO_DOUBLE(rp->drag->val.x);
   ddy = TO_DOUBLE(rp->drag->val.y);
   if (rp->part->dragable.x < 0) ddx = 1.0 - ddx;
   if (rp->part->dragable.y < 0) ddy = 1.0 - ddy;
   if (dx) *dx = ddx;
   if (dy) *dy = ddy;
   *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_size_set(Evas_Object *obj, const char *part, double dw, double dh)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_drag_size_set(part, dw, dh, &ret));
   return ret;
}

void
_part_drag_size_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double dw = va_arg(*list, double);
   double dh = va_arg(*list, double);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Edje_User_Defined *eud;
   Eina_List *l;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (!rp->drag) return;

   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (eud->type == EDJE_USER_DRAG_SIZE && !strcmp(part, eud->part))
       {
          eud->u.drag_size.w = dw;
          eud->u.drag_size.h = dh;
          break;
       }
   if (!eud)
     {
        eud = _edje_user_definition_new(EDJE_USER_DRAG_SIZE, part, ed);
        if (eud)
          {
             eud->u.drag_size.w = dw;
             eud->u.drag_size.h = dh;
          } 
     }

   if (dw < 0.0) dw = 0.0;
   else if (dw > 1.0) dw = 1.0;
   if (dh < 0.0) dh = 0.0;
   else if (dh > 1.0) dh = 1.0;
   if ((rp->drag->size.x == FROM_DOUBLE(dw)) && (rp->drag->size.y == FROM_DOUBLE(dh)))
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }
   rp->drag->size.x = FROM_DOUBLE(dw);
   rp->drag->size.y = FROM_DOUBLE(dh);
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_size_get(const Evas_Object *obj, const char *part, double *dw, double *dh)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_drag_size_get(part, dw, dh, &ret));
   return ret;
}

void
_part_drag_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double *dw = va_arg(*list, double *);
   double *dh = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part))
     {
        if (dw) *dw = 0;
        if (dh) *dh = 0;
        return;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp || !rp->drag)
     {
        if (dw) *dw = 0;
        if (dh) *dh = 0;
        return;
     }
   if (dw) *dw = TO_DOUBLE(rp->drag->size.x);
   if (dh) *dh = TO_DOUBLE(rp->drag->size.y);
   *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_step_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_drag_step_set(part, dx, dy, &ret));
   return ret;
}

void
_part_drag_step_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double dx = va_arg(*list, double);
   double dy = va_arg(*list, double);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Edje_User_Defined *eud;
   Eina_List *l;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (!rp->drag) return;

   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (eud->type == EDJE_USER_DRAG_STEP && !strcmp(part, eud->part))
       {
          eud->u.drag_position.x = dx;
          eud->u.drag_position.y = dy;
          break;
       }
   if (!eud)
     {
        eud = _edje_user_definition_new(EDJE_USER_DRAG_STEP, part, ed);
        if (eud)
          {
             eud->u.drag_position.x = dx;
             eud->u.drag_position.y = dy;
          } 
     }

   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag->step.x = FROM_DOUBLE(dx);
   rp->drag->step.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_step_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_drag_step_get(part, dx, dy, &ret));
   return ret;
}

void
_part_drag_step_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double *dx = va_arg(*list, double *);
   double *dy = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part))
     {
        if (dx) *dx = 0;
        if (dy) *dy = 0;
        return;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp || !rp->drag)
     {
        if (dx) *dx = 0;
        if (dy) *dy = 0;
        return;
     }
   if (dx) *dx = TO_DOUBLE(rp->drag->step.x);
   if (dy) *dy = TO_DOUBLE(rp->drag->step.y);
   *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_page_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_drag_page_set(part, dx, dy, &ret));
   return ret;
}

void
_part_drag_page_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double dx = va_arg(*list, double);
   double dy = va_arg(*list, double);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Edje_User_Defined *eud;
   Eina_List *l;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (!rp->drag) return;

   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (eud->type == EDJE_USER_DRAG_PAGE && !strcmp(part, eud->part))
       {
          eud->u.drag_position.x = dx;
          eud->u.drag_position.y = dy;
          break;
       }
   if (!eud)
     {
        eud = _edje_user_definition_new(EDJE_USER_DRAG_PAGE, part, ed);
        if (eud)
          {
             eud->u.drag_position.x = dx;
             eud->u.drag_position.y = dy;
          } 
     }

   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag->page.x = FROM_DOUBLE(dx);
   rp->drag->page.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_page_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_drag_page_get(part, dx, dy, &ret));
   return ret;
}

void
_part_drag_page_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double *dx = va_arg(*list, double *);
   double *dy = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = (Edje *)_pd;
   Edje_Real_Part *rp;
   *ret = EINA_FALSE;

   if ((!ed) || (!part))
     {
        if (dx) *dx = 0;
        if (dy) *dy = 0;
        return;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp || !rp->drag)
     {
        if (dx) *dx = 0;
        if (dy) *dy = 0;
        return;
     }
   if (dx) *dx = TO_DOUBLE(rp->drag->page.x);
   if (dy) *dy = TO_DOUBLE(rp->drag->page.y);
   *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_step(Evas_Object *obj, const char *part, double dx, double dy)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_drag_step(part, dx, dy, &ret));
   return ret;
}

void
_part_drag_step(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double dx = va_arg(*list, double);
   double dy = va_arg(*list, double);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   FLOAT_T px, py;
   Edje_User_Defined *eud;
   Eina_List *l;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (!rp->drag) return;
   if (rp->drag->down.count > 0) return;

   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (eud->type == EDJE_USER_DRAG_STEP && !strcmp(part, eud->part))
       {
          eud->u.drag_position.x = dx;
          eud->u.drag_position.y = dy;
          break;
       }
   if (!eud)
     {
        eud = _edje_user_definition_new(EDJE_USER_DRAG_STEP, part, ed);
        if (eud)
          {
             eud->u.drag_position.x = dx;
             eud->u.drag_position.y = dy;
          }
     }

   px = rp->drag->val.x;
   py = rp->drag->val.y;
   rp->drag->val.x = ADD(px, MUL(FROM_DOUBLE(dx),
                                 MUL(rp->drag->step.x, rp->part->dragable.x)));
   rp->drag->val.y = ADD(py, MUL(FROM_DOUBLE(dy),
                                 MUL(rp->drag->step.y, rp->part->dragable.y)));
   rp->drag->val.x = CLAMP (rp->drag->val.x, ZERO, FROM_DOUBLE(1.0));
   rp->drag->val.y = CLAMP (rp->drag->val.y, ZERO, FROM_DOUBLE(1.0));
   if ((px == rp->drag->val.x) && (py == rp->drag->val.y))
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_dragable_pos_set(ed, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(ed, "drag,step", rp->part->name);
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_page(Evas_Object *obj, const char *part, double dx, double dy)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_drag_page(part, dx, dy, &ret));
   return ret;
}

void
_part_drag_page(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   double dx = va_arg(*list, double);
   double dy = va_arg(*list, double);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   FLOAT_T px, py;
   Edje_User_Defined *eud;
   Eina_List *l;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (!rp->drag) return;
   if (rp->drag->down.count > 0) return;

   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (eud->type == EDJE_USER_DRAG_PAGE && !strcmp(part, eud->part))
       {
          eud->u.drag_position.x = dx;
          eud->u.drag_position.y = dy;
          break;
       }
   if (!eud)
     {
        eud = _edje_user_definition_new(EDJE_USER_DRAG_PAGE, part, ed);
        if (eud)
          {
             eud->u.drag_position.x = dx;
             eud->u.drag_position.y = dy;
          }
     }

   px = rp->drag->val.x;
   py = rp->drag->val.y;
   rp->drag->val.x = ADD(px, MUL(FROM_DOUBLE(dx), MUL(rp->drag->page.x, rp->part->dragable.x)));
   rp->drag->val.y = ADD(py, MUL(FROM_DOUBLE(dy), MUL(rp->drag->page.y, rp->part->dragable.y)));
   rp->drag->val.x = CLAMP (rp->drag->val.x, ZERO, FROM_DOUBLE(1.0));
   rp->drag->val.y = CLAMP (rp->drag->val.y, ZERO, FROM_DOUBLE(1.0));
   if ((px == rp->drag->val.x) && (py == rp->drag->val.y))
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_dragable_pos_set(ed, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(ed, "drag,page", rp->part->name);
   if (ret) *ret = EINA_TRUE;
}

void
_edje_box_init(void)
{

}

void
_edje_box_shutdown(void)
{
   if (!_edje_box_layout_registry)
     return;

   eina_rbtree_delete
     (_edje_box_layout_registry, _edje_box_layout_external_free, NULL);
   _edje_box_layout_registry = NULL;
}

EAPI Eina_Bool
edje_object_part_box_append(Evas_Object *obj, const char *part, Evas_Object *child)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_box_append(part, child, &ret));
   return ret;
}

void
_part_box_append(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part) || (!child)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return;

   if (_edje_real_part_box_append(ed, rp, child))
     {
        Edje_User_Defined *eud;

        eud = _edje_user_definition_new(EDJE_USER_BOX_PACK, part, ed);
        if (!eud) return;
        eud->u.box.child = child;
        eud->u.box.index = -1;

        evas_object_event_callback_add(child, EVAS_CALLBACK_DEL, _edje_user_def_del_cb, eud);
        if (ret) *ret = EINA_TRUE;
     }
}

EAPI Eina_Bool
edje_object_part_box_prepend(Evas_Object *obj, const char *part, Evas_Object *child)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_box_prepend(part, child, &ret));
   return ret;
}

void
_part_box_prepend(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return;

   if (_edje_real_part_box_prepend(ed, rp, child))
     {
        Edje_User_Defined *eud;

        eud = _edje_user_definition_new(EDJE_USER_BOX_PACK, part, ed);
        if (!eud) return;
        eud->u.box.child = child;

        evas_object_event_callback_add(child, EVAS_CALLBACK_DEL, _edje_user_def_del_cb, eud);
        if (ret) *ret = EINA_TRUE;
     }
}

EAPI Eina_Bool
edje_object_part_box_insert_before(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_box_insert_before(part, child, reference, &ret));
   return ret;
}

void
_part_box_insert_before(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   const Evas_Object *reference = va_arg(*list, const Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return;

   if (_edje_real_part_box_insert_before(ed, rp, child, reference))
     {
        Edje_User_Defined *eud;

        eud = _edje_user_definition_new(EDJE_USER_BOX_PACK, part, ed);
        if (!eud) return;
        eud->u.box.child = child;

        evas_object_event_callback_add(child, EVAS_CALLBACK_DEL, _edje_user_def_del_cb, eud);
        if (ret) *ret = EINA_TRUE;
     }
}

EAPI Eina_Bool
edje_object_part_box_insert_at(Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_box_insert_at(part, child, pos, &ret));
   return ret;
}

void
_part_box_insert_at(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   unsigned int pos = va_arg(*list, unsigned int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return;

   if (_edje_real_part_box_insert_at(ed, rp, child, pos))
     {
        Edje_User_Defined *eud;

        eud = _edje_user_definition_new(EDJE_USER_BOX_PACK, part, ed);
        if (!eud) return;
        eud->u.box.child = child;

        evas_object_event_callback_add(child, EVAS_CALLBACK_DEL, _edje_user_def_del_cb, eud);
        if (ret) *ret = EINA_TRUE;
     }
}

EAPI Evas_Object *
edje_object_part_box_remove(Evas_Object *obj, const char *part, Evas_Object *child)
{
   if (!obj) return NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, edje_obj_part_box_remove(part, child, &ret));
   return ret;
}

void
_part_box_remove(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Evas_Object *r;
   if (ret) *ret = NULL;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return;

   r = _edje_real_part_box_remove(ed, rp, child);

   if (r)
     {
        Edje_User_Defined *eud;
        Eina_List *l;

        EINA_LIST_FOREACH(ed->user_defined, l, eud)
          if (eud->type == EDJE_USER_BOX_PACK && eud->u.box.child == child && !strcmp(eud->part, part))
            {
               _edje_user_definition_free(eud);
               if (ret) *ret = r;
               return;
            }
     }
   if (ret) *ret = r;
}

EAPI Evas_Object *
edje_object_part_box_remove_at(Evas_Object *obj, const char *part, unsigned int pos)
{
   if (!obj) return NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, edje_obj_part_box_remove_at(part, pos, &ret));
   return ret;
}

void
_part_box_remove_at(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   unsigned int pos = va_arg(*list, unsigned int);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Evas_Object *r;
   if (ret) *ret = NULL;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return;

   r = _edje_real_part_box_remove_at(ed, rp, pos);

   if (r)
     {
        Edje_User_Defined *eud;
        Eina_List *l;

        EINA_LIST_FOREACH(ed->user_defined, l, eud)
          if (eud->type == EDJE_USER_BOX_PACK && eud->u.box.child == r && !strcmp(eud->part, part))
            {
               _edje_user_definition_free(eud);
               if (ret) *ret = r;
               return;
            }
     }
   if (ret) *ret = r;
}

EAPI Eina_Bool
edje_object_part_box_remove_all(Evas_Object *obj, const char *part, Eina_Bool clear)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_box_remove_all(part, clear, &ret));
   return ret;
}

void
_part_box_remove_all(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool clear = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return;

   if (_edje_real_part_box_remove_all(ed, rp, clear))
     {
        if (ret) *ret = EINA_TRUE;
        Edje_User_Defined *eud;
        Eina_List *ll, *l;

        EINA_LIST_FOREACH_SAFE(ed->user_defined, l, ll, eud)
          if (eud->type == EDJE_USER_BOX_PACK && !strcmp(eud->part, part))
            {
               _edje_user_definition_free(eud);
               return;
            }
     }
}

EAPI Eina_List *
edje_object_access_part_list_get(const Evas_Object *obj)
{
   if (!obj) return NULL;
   Eina_List *ret = NULL;
   eo_do((Eo *)obj, edje_obj_access_part_list_get(&ret));
   return ret;
}

void
_access_part_list_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_List **ret = va_arg(*list, Eina_List **);
   Edje *ed = _pd;
   Eina_List *access_parts = NULL;
   *ret = NULL;

   if ((!ed)) return;

   unsigned int i;
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;
        rp = ed->table_parts[i];
        if (rp->part->access)
          access_parts = eina_list_append(access_parts, rp->part->name);
     }

   *ret = access_parts;
}

static void
_edje_child_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *child, void *einfo EINA_UNUSED)
{
   Edje_Real_Part *rp = data;
   Edje_User_Defined *eud;
   Eina_List *l;
   Edje *ed = evas_object_data_get(child, ".edje");

   if (!ed) return;
   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (rp->part->type == EDJE_PART_TYPE_BOX)
       {
          if (eud->type == EDJE_USER_BOX_PACK &&
              eud->u.box.child == child &&
              !strcmp(rp->part->name, eud->part))
            {
               _edje_user_definition_free(eud);
               break;
            }
       }
     else if (rp->part->type == EDJE_PART_TYPE_TABLE)
       {
          if (eud->type == EDJE_USER_TABLE_PACK &&
              eud->u.table.child == child &&
              !strcmp(rp->part->name, eud->part))
            {
               _edje_user_definition_free(eud);
               break;
            }
       }

   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
}

static void
_edje_child_add(Edje *ed, Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_add(child, EVAS_CALLBACK_DEL, _edje_child_del_cb, rp);
   evas_object_data_set(child, ".edje", ed);
   if (!ed) return;
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
}

static void
_edje_child_remove(Edje *ed, Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_del_full(child, EVAS_CALLBACK_DEL, _edje_child_del_cb, rp);
   evas_object_data_del(child, ".edje");
   if (!ed) return;
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
}

Eina_Bool
_edje_real_part_box_append(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_append(rp->object, child_obj);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_child_add(ed, rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_prepend(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_prepend(rp->object, child_obj);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_child_add(ed, rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_insert_before(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj, const Evas_Object *ref)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_insert_before(rp->object, child_obj, ref);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_child_add(ed, rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_insert_at(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj, unsigned int pos)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_insert_at(rp->object, child_obj, pos);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_child_add(ed, rp, child_obj);

   return EINA_TRUE;
}

Evas_Object *
_edje_real_part_box_remove(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj)
{
   if (evas_object_data_get(child_obj, "\377 edje.box_item")) return NULL;
   if (!evas_object_box_remove(rp->object, child_obj)) return NULL;
   _edje_box_layout_remove_child(rp, child_obj);
   _edje_child_remove(ed, rp, child_obj);
   return child_obj;
}

Evas_Object *
_edje_real_part_box_remove_at(Edje *ed, Edje_Real_Part *rp, unsigned int pos)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;
   Evas_Object *child_obj;

   priv = eo_data_scope_get(rp->object, EVAS_OBJ_BOX_CLASS);
   opt = eina_list_nth(priv->children, pos);
   if (!opt) return NULL;
   child_obj = opt->obj;
   if (evas_object_data_get(child_obj, "\377 edje.box_item")) return NULL;
   if (!evas_object_box_remove_at(rp->object, pos)) return NULL;
   _edje_box_layout_remove_child(rp, child_obj);
   _edje_child_remove(ed, rp, child_obj);
   return child_obj;
}

Eina_Bool
_edje_real_part_box_remove_all(Edje *ed, Edje_Real_Part *rp, Eina_Bool clear)
{
   Eina_List *children;
   int i = 0;

   children = evas_object_box_children_get(rp->object);
   while (children)
     {
        Evas_Object *child_obj = children->data;
        if (evas_object_data_get(child_obj, "\377 edje.box_item"))
          i++;
        else
          {
             _edje_box_layout_remove_child(rp, child_obj);
             _edje_child_remove(ed, rp, child_obj);
             if (!evas_object_box_remove_at(rp->object, i))
               return EINA_FALSE;
             if (clear)
               evas_object_del(child_obj);
          }
        children = eina_list_remove_list(children, children);
     }
   return EINA_TRUE;
}

EAPI Evas_Object *
edje_object_part_table_child_get(const Evas_Object *obj, const char *part, unsigned int col, unsigned int row)
{
   if (!obj) return NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *)obj, edje_obj_part_table_child_get(part, col, row, &ret));
   return ret;
}

void
_part_table_child_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   unsigned int col = va_arg(*list, unsigned int);
   unsigned int row = va_arg(*list, unsigned int);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   *ret = NULL;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return;

   *ret = evas_object_table_child_get(rp->object, col, row);
}

EAPI Eina_Bool
edje_object_part_table_pack(Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_table_pack(part, child_obj, col, row, colspan, rowspan, &ret));
   return ret;
}

void
_part_table_pack(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *child_obj = va_arg(*list, Evas_Object *);
   unsigned short col = va_arg(*list, unsigned int);
   unsigned short row = va_arg(*list, unsigned int);
   unsigned short colspan = va_arg(*list, unsigned int);
   unsigned short rowspan = va_arg(*list, unsigned int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   Edje_User_Defined *eud;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return;

   if (_edje_real_part_table_pack(ed, rp, child_obj, col, row, colspan, rowspan))
     {
        if (ret) *ret = EINA_TRUE;
        eud = _edje_user_definition_new(EDJE_USER_TABLE_PACK, part, ed);
        if (!eud) return;

        eud->u.table.child = child_obj;
        eud->u.table.col = col;
        eud->u.table.row = row;
        eud->u.table.colspan = colspan;
        eud->u.table.rowspan = rowspan;

        evas_object_event_callback_add(child_obj, EVAS_CALLBACK_DEL, _edje_user_def_del_cb, eud);
     }
}

EAPI Eina_Bool
edje_object_part_table_unpack(Evas_Object *obj, const char *part, Evas_Object *child_obj)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_table_unpack(part, child_obj, &ret));
   return ret;
}

void
_part_table_unpack(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *child_obj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return;

   if (_edje_real_part_table_unpack(ed, rp, child_obj))
     {
        if (ret) *ret = EINA_TRUE;
        Edje_User_Defined *eud;
        Eina_List *l;

        EINA_LIST_FOREACH(ed->user_defined, l, eud)
          if (eud->type == EDJE_USER_TABLE_PACK &&
              eud->u.table.child == child_obj &&
              !strcmp(part, eud->part))
            {
               _edje_user_definition_free(eud);
               break;
            }
     }
}

EAPI Eina_Bool
edje_object_part_table_col_row_size_get(const Evas_Object *obj, const char *part, int *cols, int *rows)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_part_table_col_row_size_get(part, cols, rows, &ret));
   return ret;
}

void
_part_table_col_row_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   int *cols = va_arg(*list, int *);
   int *rows = va_arg(*list, int *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return;

   evas_object_table_col_row_size_get(rp->object, cols, rows);
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_table_clear(Evas_Object *obj, const char *part, Eina_Bool clear)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_part_table_clear(part, clear, &ret));
   return ret;
}

void
_part_table_clear(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Eina_Bool clear = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   Edje_Real_Part *rp;
   if (ret) *ret = EINA_FALSE;

   if ((!ed) || (!part)) return;

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return;

   _edje_real_part_table_clear(ed, rp, clear);
   if (ret) *ret = EINA_TRUE;
}

static void
_edje_perspective_obj_del(void *data, EINA_UNUSED Evas *e, EINA_UNUSED Evas_Object *obj, EINA_UNUSED void *event_info)
{
   Edje_Perspective *ps = data;
   Evas_Object *o;

   EINA_LIST_FREE(ps->users, o)
     {
        Edje *ed;

        ed = eo_data_scope_get(o, EDJE_OBJ_CLASS);
        if (!ed) continue;
        ed->persp = NULL;
        ed->dirty = EINA_TRUE;
        ed->recalc_call = EINA_TRUE;
        _edje_recalc_do(ed);
     }
   free(ps);
}

EAPI Edje_Perspective *
edje_perspective_new(Evas *e)
{
   Edje_Perspective *ps;
   Evas_Coord vx, vy, vw, vh;

   if (!e) return NULL;
   ps = calloc(1, sizeof(Edje_Perspective));
   ps->obj = evas_object_rectangle_add(e);
   evas_object_data_set(ps->obj, "_edje_perspective", ps);
   evas_object_event_callback_add(ps->obj, EVAS_CALLBACK_DEL, _edje_perspective_obj_del, ps);
   evas_output_viewport_get(e, &vx, &vy, &vw, &vh);
   ps->e = e;
   ps->px = vx + (vw / 2);
   ps->py = vy + (vh / 2);
   ps->z0 = 0;
   ps->foc = 1000;
   return ps;
}

EAPI void
edje_perspective_free(Edje_Perspective *ps)
{
   if (!ps) return;
   evas_object_del(ps->obj);
}

EAPI void
edje_perspective_set(Edje_Perspective *ps, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc)
{
   Eina_List *l;
   Evas_Object *o;

   if (!ps) return;
   if ((ps->px == px) && (ps->py == py) && (ps->z0 == z0) && (ps->foc == foc)) return;
   ps->px = px;
   ps->py = py;
   ps->z0 = z0;
   ps->foc = foc;
   EINA_LIST_FOREACH(ps->users, l, o)
     {
        Edje *ed;

        ed = eo_data_scope_get(o, EDJE_OBJ_CLASS);
        if (!ed) continue;
        if (!ed->persp)
          {
             ed->dirty = EINA_TRUE;
             ed->recalc_call = EINA_TRUE;
             _edje_recalc_do(ed);
          }
     }
   if (ps->global)
     {
        EINA_LIST_FOREACH(_edje_edjes, l, o)
          {
             Edje *ed;

             ed = eo_data_scope_get(o, EDJE_OBJ_CLASS);
             if (!ed) continue;
             if (!ed->persp)
               {
                  ed->dirty = EINA_TRUE;
                  ed->recalc_call = EINA_TRUE;
                  _edje_recalc_do(ed);
               }
          }
     }
}

EAPI void
edje_perspective_global_set(Edje_Perspective *ps, Eina_Bool global)
{
   Evas_Object *o;
   Eina_List *l;

   if (!ps) return;
   if (ps->global == global) return;
   if (global)
     {
        o = evas_object_name_find(evas_object_evas_get(ps->obj),
                                  "_edje_perspective");
        if (o) evas_object_name_set(o, NULL);
        evas_object_name_set(ps->obj, "_edje_perspective");
     }
   else
     evas_object_name_set(ps->obj, NULL);
   ps->global = global;
   EINA_LIST_FOREACH(_edje_edjes, l, o)
     {
        Edje *ed;

        ed = eo_data_scope_get(o, EDJE_OBJ_CLASS);
        if (!ed) continue;
        if (!ed->persp)
          {
             ed->dirty = EINA_TRUE;
             ed->recalc_call = EINA_TRUE;
             _edje_recalc_do(ed);
          }
     }
}

EAPI Eina_Bool
edje_perspective_global_get(const Edje_Perspective *ps)
{
   if (!ps) return EINA_FALSE;
   return ps->global;
}

EAPI const Edje_Perspective *
edje_evas_global_perspective_get(const Evas *e)
{
   Evas_Object *obj;

   if (!e) return NULL;
   obj = evas_object_name_find(e, "_edje_perspective");
   if (!obj) return NULL;
   return evas_object_data_get(obj, "_edje_perspective");
}

EAPI void
edje_object_perspective_set(Evas_Object *obj, Edje_Perspective *ps)
{
   if (!obj) return;
   eo_do(obj, edje_obj_perspective_set(ps));
}

void
_perspective_set(Eo *obj, void *_pd, va_list *list)
{
   Edje_Perspective *ps = va_arg(*list, Edje_Perspective *);
   Edje *ed = _pd;

   if (!ed) return;
   if (ed->persp == ps) return;
   if (ed->persp != ps)
     {
        if (ed->persp)
          ed->persp->users = eina_list_remove(ed->persp->users, obj);
     }
   ed->persp = ps;
   if (ps) ps->users = eina_list_append(ps->users, obj);
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
   _edje_recalc_do(ed);
}

EAPI const Edje_Perspective *
edje_object_perspective_get(const Evas_Object *obj)
{
   if (!obj) return NULL;
   const Edje_Perspective *ret = NULL;
   eo_do((Eo *)obj, edje_obj_perspective_get(&ret));
   return ret;
}

void
_perspective_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Edje_Perspective **ret = va_arg(*list, const Edje_Perspective **);
   const Edje *ed = _pd;
   *ret = ed->persp;
}

#define EDJE_PRELOAD_EMISSION "preload,done"
#define EDJE_PRELOAD_SOURCE NULL

EAPI Eina_Bool
edje_object_preload(Evas_Object *obj, Eina_Bool cancel)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, edje_obj_preload(cancel, &ret));
   return ret;
}

void
_preload(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool cancel = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Edje *ed = _pd;
   int count;
   unsigned int i;
   if (ret) *ret = EINA_FALSE;

   if (!ed) return;

   _edje_recalc_do(ed);

   for (i = 0, count = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;
        Edje_Part *ep;

        rp = ed->table_parts[i];
        ep = rp->part;

        if ((ep->type == EDJE_PART_TYPE_IMAGE) ||
            ((ep->type == EDJE_PART_TYPE_GROUP) &&
             ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
              (rp->typedata.swallow)) &&
             (rp->typedata.swallow->swallowed_object)))
          count++;
     }

   ed->preload_count = count;

   if (count > 0)
     {
        for (i = 0; i < ed->table_parts_size; i++)
          {
             Edje_Real_Part *rp;
             Edje_Part *ep;

             rp = ed->table_parts[i];
             ep = rp->part;

             if (ep->type == EDJE_PART_TYPE_IMAGE)
               {
                  const char *file = NULL;
                  const char *key = NULL;

                  evas_object_event_callback_del_full(rp->object, EVAS_CALLBACK_IMAGE_PRELOADED, _edje_object_image_preload_cb, ed);

                  evas_object_image_file_get(rp->object, &file, &key);
                  if (!file && !key)
                    {
                       ed->preload_count--;
                    }
                  else
                    {
                       evas_object_event_callback_add(rp->object, EVAS_CALLBACK_IMAGE_PRELOADED, _edje_object_image_preload_cb, ed);
                       evas_object_image_preload(rp->object, cancel);
                    }
                  count--;
               }
             else if (ep->type == EDJE_PART_TYPE_GROUP)
               {
                  if (((rp->type == EDJE_RP_TYPE_SWALLOW) &&
                       (rp->typedata.swallow)) &&
                      (rp->typedata.swallow->swallowed_object))
                    {
                       char *tmp;

                       if (rp->part->name)
                         {
                            tmp = alloca(strlen(rp->part->name) + 2);
                            sprintf(tmp, "%s:", rp->part->name);

                            edje_object_signal_callback_del(obj, EDJE_PRELOAD_EMISSION, tmp, _edje_object_signal_preload_cb);
                            edje_object_signal_callback_add(obj, EDJE_PRELOAD_EMISSION, tmp, _edje_object_signal_preload_cb, ed);
                            edje_object_preload(rp->typedata.swallow->swallowed_object, cancel);
                         }
                       else
                         {
                            ed->preload_count--;
                         }
                       count--;
                    }
               }
          }
     }
   else
     {
        _edje_emit(ed, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE);
     }

   if (ret) *ret = EINA_TRUE;
}

EAPI void
edje_object_update_hints_set(Evas_Object *obj, Eina_Bool update)
{
   if (!obj) return;
   eo_do(obj, edje_obj_update_hints_set(update));
}

void
_update_hints_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool update = va_arg(*list, int);
   Edje *ed = _pd;

   if (!ed) return;
   if (ed->update_hints == !!update) return;

   ed->update_hints = !!update;
   if (update)
     {
        ed->recalc_hints = EINA_TRUE;
        _edje_recalc(ed);
     }
}

EAPI Eina_Bool
edje_object_update_hints_get(Evas_Object *obj)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, edje_obj_update_hints_get(&ret));
   return ret;
}

void
_update_hints_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   const Edje *ed = _pd;
   *ret = ed->update_hints;
}

Eina_Bool
_edje_real_part_table_pack(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Eina_Bool ret =
     evas_object_table_pack(rp->object, child_obj, col, row, colspan, rowspan);

   _edje_child_add(ed, rp, child_obj);

   return ret;
}

Eina_Bool
_edje_real_part_table_unpack(Edje *ed, Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Eina_Bool ret = evas_object_table_unpack(rp->object, child_obj);

   if (ret)
     _edje_child_remove(ed, rp, child_obj);

   return ret;
}

void
_edje_real_part_table_clear(Edje *ed, Edje_Real_Part *rp, Eina_Bool clear)
{
   Eina_List *children;

   children = evas_object_table_children_get(rp->object);
   while (children)
     {
        Evas_Object *child_obj = children->data;

        _edje_child_remove(ed, rp, child_obj);
        if (!evas_object_data_get(child_obj, "\377 edje.table_item"))
          {
	     evas_object_table_unpack(rp->object, child_obj);
             if (clear)
               evas_object_del(child_obj);
          }
        children = eina_list_remove_list(children, children);
     }
}

Edje_Real_Part *
_edje_real_part_recursive_get(Edje **ed, const char *part)
{
   Edje_Real_Part *rp;
   char **path;

   path = eina_str_split(part, EDJE_PART_PATH_SEPARATOR_STRING, 0);
   if (!path) return NULL;

   rp = _edje_real_part_recursive_get_helper(ed, path);

   free(*path);
   free(path);
   return rp;
}

Evas_Object *
_edje_children_get(Edje_Real_Part *rp, const char *partid)
{
   Evas_Object *child = NULL;
   Eina_Iterator *it = NULL;
   long int v;
   char *p;

   if (!partid) return NULL;

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_EXTERNAL:
        if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
            (!rp->typedata.swallow)) return NULL;
        return _edje_external_content_get
          (rp->typedata.swallow->swallowed_object, partid);
      case EDJE_PART_TYPE_BOX:
         it = evas_object_box_iterator_new(rp->object);
         break;
      case EDJE_PART_TYPE_TABLE:
         it = evas_object_table_iterator_new(rp->object);
         break;
      default:
         return NULL;
     }

   v = strtol(partid, &p, 10);
   if ((*p == '\0') && (v >= 0))
     {
        int i = 0;
        EINA_ITERATOR_FOREACH(it, child)
          {
             if (i == v) break;
             i++;
          }
        if (i != v) child = NULL;
     }
   else
     {
        Evas_Object *cur;

        EINA_ITERATOR_FOREACH(it, cur)
          {
             const char *name = evas_object_name_get(cur);
             if ((name) && (!strcmp(name, partid)))
               {
                  child = cur;
                  break;
               }
          }
     }
   eina_iterator_free(it);

   return child;
}

/* rebuild alternative path */
char *
_edje_merge_path(const char *alias, char * const *path)
{
   char *tmp;
   unsigned int length = 1;
   unsigned int alias_length;
   unsigned int i;

   if (!alias) return NULL;

   alias_length = strlen(alias);

   for (i = 0; path[i]; i++)
     length += strlen(path[i]) + 1;

   tmp = malloc(sizeof (char) * (length + alias_length + 2));
   memcpy(tmp, alias, alias_length);
   tmp[alias_length] = '\0';

   for (i = 0; path[i]; i++)
     {
        strcat(tmp, EDJE_PART_PATH_SEPARATOR_STRING);
        strcat(tmp, path[i]);
     }

   return tmp;
}


Edje_Real_Part *
_edje_real_part_recursive_get_helper(Edje **ed, char **path)
{
   Edje_Real_Part *rp;
   Evas_Object *child;
   char *idx = NULL;

   if (!path[0])
     return NULL;

   if ((*ed)->collection && (*ed)->collection->alias)
     {
        char *alias;

        alias = _edje_merge_path(eina_hash_find((*ed)->collection->alias, path[0]), path + 1);
        if (alias) {
           rp = _edje_real_part_recursive_get(ed, alias);
           free(alias);
           return rp;
        }
     }

   //printf("  lookup: %s on %s\n", path[0], ed->parent ? ed->parent : "-");
   idx = strchr(path[0], EDJE_PART_PATH_SEPARATOR_INDEXL);
   if (idx)
     {
        char *end;

        end = strchr(idx + 1, EDJE_PART_PATH_SEPARATOR_INDEXR);
        if (end)
          {
             *end = '\0';
             *idx = '\0';
             idx++;
          }
     }

   rp = _edje_real_part_get(*ed, path[0]);
   if (!path[1] && !idx) return rp;
   if (!rp) return NULL;

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_GROUP:
         if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
             (!rp->typedata.swallow)) return NULL;
         if (!rp->typedata.swallow->swallowed_object) return NULL;
         *ed = _edje_fetch(rp->typedata.swallow->swallowed_object);
         if (!*ed) return NULL;
         path++;
         return _edje_real_part_recursive_get_helper(ed, path);
      case EDJE_PART_TYPE_BOX:
      case EDJE_PART_TYPE_TABLE:
      case EDJE_PART_TYPE_EXTERNAL:
         if (!idx) return rp;
         path++;

         child = _edje_children_get(rp, idx);

         *ed = _edje_fetch(child);

         if (!*ed) return NULL;
         return _edje_real_part_recursive_get_helper(ed, path);
      default:
         return NULL;
     }
}

/* Private Routines */
Edje_Real_Part *
_edje_real_part_get(const Edje *ed, const char *part)
{
   unsigned int i;

   if (!part) return NULL;

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;

        rp = ed->table_parts[i];
        if ((rp->part->name) && (!strcmp(rp->part->name, part))) return rp;
     }
   return NULL;
}

Edje_Color_Class *
_edje_color_class_find(const Edje *ed, const char *color_class)
{
   Edje_Color_Class *cc = NULL;

   if ((!ed) || (!color_class)) return NULL;

   /* first look through the object scope */
   cc = eina_hash_find(ed->color_classes, color_class);
   if (cc) return cc;

   /* next look through the global scope */
   cc = eina_hash_find(_edje_color_class_hash, color_class);
   if (cc) return cc;

   /* finally, look through the file scope */
   cc = eina_hash_find(ed->file->color_hash, color_class);
   if (cc) return cc;

   return NULL;
}

void
_edje_color_class_member_add(Edje *ed, const char *color_class)
{
   _edje_class_member_add(ed, &ed->members.color_class, &_edje_color_class_member_hash, color_class);
}

void
_edje_color_class_member_direct_del(const char *color_class, void *l)
{
   _edje_class_member_direct_del(color_class, l, _edje_color_class_member_hash);
}

void
_edje_color_class_member_del(Edje *ed, const char *color_class)
{
   if ((!ed) || (!color_class)) return;

   _edje_class_member_del(&ed->members.color_class, &_edje_color_class_member_hash, color_class);
}

void
_edje_color_class_members_free(void)
{
   if (!_edje_color_class_member_hash) return;
   eina_hash_foreach(_edje_color_class_member_hash, member_list_free, NULL);
   eina_hash_free(_edje_color_class_member_hash);
   _edje_color_class_member_hash = NULL;
}

static Eina_Bool
color_class_hash_list_free(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Edje_Color_Class *cc;

   cc = data;
   if (cc->name) eina_stringshare_del(cc->name);
   free(cc);
   return EINA_TRUE;
}

void
_edje_color_class_hash_free(void)
{
   if (!_edje_color_class_hash) return;
   eina_hash_foreach(_edje_color_class_hash, color_class_hash_list_free, NULL);
   eina_hash_free(_edje_color_class_hash);
   _edje_color_class_hash = NULL;
}

void
_edje_color_class_on_del(Edje *ed, Edje_Part *ep)
{
   unsigned int i;

   if ((ep->default_desc) && (ep->default_desc->color_class))
     _edje_color_class_member_del(ed, ep->default_desc->color_class);

   for (i = 0; i < ep->other.desc_count; ++i)
     if (ep->other.desc[i]->color_class)
       _edje_color_class_member_del(ed, ep->other.desc[i]->color_class);
}

Edje_Text_Class *
_edje_text_class_find(Edje *ed, const char *text_class)
{
   Eina_List *l;
   Edje_Text_Class *tc;

   if ((!ed) || (!text_class)) return NULL;
   EINA_LIST_FOREACH(ed->text_classes, l, tc)
     if ((tc->name) && (!strcmp(text_class, tc->name))) return tc;
   return eina_hash_find(_edje_text_class_hash, text_class);
}

void
_edje_text_class_member_direct_del(const char *text_class,
                                   void *l)
{
   _edje_class_member_direct_del(text_class, l, _edje_text_class_member_hash);
}

void
_edje_text_class_member_add(Edje *ed, const char *text_class)
{
   _edje_class_member_add(ed, &ed->members.text_class, &_edje_text_class_member_hash, text_class);
}

void
_edje_text_class_member_del(Edje *ed, const char *text_class)
{
   if ((!ed) || (!text_class)) return;

   _edje_class_member_del(&ed->members.text_class, &_edje_text_class_member_hash, text_class);
}

void
_edje_text_class_members_free(void)
{
   _edje_class_members_free(&_edje_text_class_member_hash);
}

static Eina_Bool
text_class_hash_list_free(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Edje_Text_Class *tc;

   tc = data;
   if (tc->name) eina_stringshare_del(tc->name);
   if (tc->font) eina_stringshare_del(tc->font);
   free(tc);
   return EINA_TRUE;
}

void
_edje_text_class_hash_free(void)
{
   if (!_edje_text_class_hash) return;
   eina_hash_foreach(_edje_text_class_hash, text_class_hash_list_free, NULL);
   eina_hash_free(_edje_text_class_hash);
   _edje_text_class_hash = NULL;
}

Edje *
_edje_fetch(const Evas_Object *obj)
{
   Edje *ed;

   if (!obj || !eo_isa(obj, EDJE_OBJ_CLASS))
     return NULL;
   ed = eo_data_scope_get(obj, EDJE_OBJ_CLASS);
   if ((ed) && (ed->delete_me)) return NULL;
   return ed;
}

int
_edje_freeze(Edje *ed)
{
   ed->freeze++;
//   printf("FREEZE %i\n", ed->freeze);
   return ed->freeze;
}

int
_edje_thaw(Edje *ed)
{
   ed->freeze--;
   if (ed->freeze < 0)
     {
        //	printf("-------------########### OVER THAW\n");
        ed->freeze = 0;
     }
   if ((ed->freeze == 0) && (ed->recalc))
     {
        //	printf("thaw recalc\n");
        _edje_recalc(ed);
     }
   return ed->freeze;
}

int
_edje_block(Edje *ed)
{
   _edje_ref(ed);
   ed->block++;
   return ed->block;
}

int
_edje_unblock(Edje *ed)
{
   int ret = 0;

   if (!ed) return ret;

   ed->block--;
   if (ed->block == 0) ed->block_break = EINA_FALSE;
   ret = ed->block;
   _edje_unref(ed);
   return ret;
}

int
_edje_block_break(Edje *ed)
{
   if (ed->block_break) return 1;
   return 0;
}

void
_edje_block_violate(Edje *ed)
{
   if (ed->block > 0) ed->block_break = EINA_TRUE;
}

void
_edje_object_part_swallow_free_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Edje_User_Defined *eud;
   Eina_List *l;
   Edje *ed;
   Edje_Real_Part *rp;

   ed = evas_object_data_get(obj, ".edje");
   if (!ed) return;
   EINA_LIST_FOREACH(ed->user_defined, l, eud)
     if (eud->type == EDJE_USER_SWALLOW && eud->u.swallow.child == obj)
       {
          _edje_user_definition_free(eud);
          break;
       }
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp && (rp->part->type == EDJE_PART_TYPE_SWALLOW))
     edje_object_part_unswallow(ed->obj, obj);

   return;
}

static void
_edje_real_part_swallow_hints_update(Edje_Real_Part *rp)
{
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return;

   rp->typedata.swallow->swallow_params.min.w = 0;
   rp->typedata.swallow->swallow_params.min.h = 0;
   rp->typedata.swallow->swallow_params.max.w = -1;
   rp->typedata.swallow->swallow_params.max.h = -1;
   if (eo_isa(rp->typedata.swallow->swallowed_object, EDJE_OBJ_CLASS))
     {
        Evas_Coord w, h;

#if 0
        edje_object_size_min_get(rp->typedata.swallow->swallowed_object, &w, &h);
        rp->typedata.swallow->swallow_params.min.w = w;
        rp->typedata.swallow->swallow_params.min.h = h;
#endif
        edje_object_size_max_get(rp->typedata.swallow->swallowed_object, &w, &h);
        rp->typedata.swallow->swallow_params.max.w = w;
        rp->typedata.swallow->swallow_params.max.h = h;
     }
   else if (eo_isa(rp->typedata.swallow->swallowed_object, EVAS_OBJ_TEXT_CLASS) ||
	    eo_isa(rp->typedata.swallow->swallowed_object, EVAS_OBJ_POLYGON_CLASS) ||
	    eo_isa(rp->typedata.swallow->swallowed_object, EVAS_OBJ_LINE_CLASS))
     {
        Evas_Coord w, h;

        evas_object_geometry_get(rp->typedata.swallow->swallowed_object, NULL, NULL, &w, &h);
#if 0
        rp->typedata.swallow->swallow_params.min.w = w;
        rp->typedata.swallow->swallow_params.min.h = h;
#endif
        rp->typedata.swallow->swallow_params.max.w = w;
        rp->typedata.swallow->swallow_params.max.h = h;
     }
     {
        Evas_Coord w1, h1, w2, h2, aw, ah;
        Evas_Aspect_Control am;

        evas_object_size_hint_min_get(rp->typedata.swallow->swallowed_object, &w1, &h1);
        evas_object_size_hint_max_get(rp->typedata.swallow->swallowed_object, &w2, &h2);
        evas_object_size_hint_aspect_get(rp->typedata.swallow->swallowed_object, &am, &aw, &ah);
        rp->typedata.swallow->swallow_params.min.w = w1;
        rp->typedata.swallow->swallow_params.min.h = h1;
        if (w2 > 0) rp->typedata.swallow->swallow_params.max.w = w2;
        if (h2 > 0) rp->typedata.swallow->swallow_params.max.h = h2;
        switch (am)
          {
           case EVAS_ASPECT_CONTROL_NONE:
              rp->typedata.swallow->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_NONE;
              break;
           case EVAS_ASPECT_CONTROL_NEITHER:
              rp->typedata.swallow->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_NEITHER;
              break;
           case EVAS_ASPECT_CONTROL_HORIZONTAL:
              rp->typedata.swallow->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_HORIZONTAL;
              break;
           case EVAS_ASPECT_CONTROL_VERTICAL:
              rp->typedata.swallow->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_VERTICAL;
              break;
           case EVAS_ASPECT_CONTROL_BOTH:
              rp->typedata.swallow->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_BOTH;
              break;
           default:
              break;
          }
        rp->typedata.swallow->swallow_params.aspect.w = aw;
        rp->typedata.swallow->swallow_params.aspect.h = ah;
        evas_object_data_set(rp->typedata.swallow->swallowed_object, "\377 edje.swallowing_part", rp);
     }

#ifdef EDJE_CALC_CACHE
     rp->invalidate = EINA_TRUE;
#endif
}

void
_edje_object_part_swallow_changed_hints_cb(void *data, EINA_UNUSED Evas *e, Evas_Object *obj, EINA_UNUSED void *event_info)
{
   Edje_Real_Part *rp;
   Edje *ed;

   rp = data;
   ed = evas_object_data_get(obj, ".edje");
   if (!ed) return;
   _edje_real_part_swallow_hints_update(rp);
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
   ed->recalc_hints = EINA_TRUE;

   _edje_recalc(ed);
   return;
}

void
_edje_object_part_swallow_image_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Edje_Real_Part *rp = data;
   Evas_Coord w, h;
   Evas_Map *map;

   if (!rp->chosen_description->map.on) return;
   if (evas_object_image_source_get(rp->typedata.swallow->swallowed_object))
     return;

   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return;
   map = (Evas_Map *)evas_object_map_get(rp->typedata.swallow->swallowed_object);
   if (!map) return;

   evas_object_image_size_get(rp->typedata.swallow->swallowed_object, &w, &h);
   evas_map_point_image_uv_set(map, 0, 0, 0);
   evas_map_point_image_uv_set(map, 1, w, 0);
   evas_map_point_image_uv_set(map, 2, w, h);
   evas_map_point_image_uv_set(map, 3, 0, h);

   evas_object_map_set(rp->typedata.swallow->swallowed_object, map);
}

void
_edje_real_part_swallow(Edje *ed,
			Edje_Real_Part *rp,
                        Evas_Object *obj_swallow,
                        Eina_Bool hints_update)
{
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return;
   if (rp->typedata.swallow->swallowed_object)
     {
        if (rp->typedata.swallow->swallowed_object != obj_swallow)
          {
             edje_object_part_unswallow(ed->obj, rp->typedata.swallow->swallowed_object);
          }
        else
          {
             if (hints_update)
               _edje_real_part_swallow_hints_update(rp);
             ed->dirty = EINA_TRUE;
             ed->recalc_call = EINA_TRUE;
             ed->recalc_hints = EINA_TRUE;
             _edje_recalc(ed);
             return;
          }
     }
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   if (!obj_swallow) return;
   rp->typedata.swallow->swallowed_object = obj_swallow;
   evas_object_smart_member_add(rp->typedata.swallow->swallowed_object, ed->obj);
   if (rp->part->clip_to_id >= 0)
     {
       Edje_Real_Part *clip_to;

       clip_to = ed->table_parts[rp->part->clip_to_id % ed->table_parts_size];
       evas_object_clip_set(rp->typedata.swallow->swallowed_object, clip_to->object);
     }
   else evas_object_clip_set(rp->typedata.swallow->swallowed_object, ed->base->clipper);
   //FIXME: I'm not sure why Edje Nested is a parent of the rp->object here. Is it normal case?
   if (evas_object_smart_parent_get(rp->object) == ed->obj)
     evas_object_stack_above(rp->typedata.swallow->swallowed_object, rp->object);
   evas_object_event_callback_add(rp->typedata.swallow->swallowed_object,
                                  EVAS_CALLBACK_DEL,
                                  _edje_object_part_swallow_free_cb,
                                  rp);
   evas_object_event_callback_add(rp->typedata.swallow->swallowed_object,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _edje_object_part_swallow_changed_hints_cb,
                                  rp);
   evas_object_data_set(rp->typedata.swallow->swallowed_object, ".edje", ed);

   //If the map is enabled, uv should be updated when image size is changed.
   if (eo_isa(rp->typedata.swallow->swallowed_object, EVAS_OBJ_IMAGE_CLASS))
     evas_object_event_callback_add(obj_swallow, EVAS_CALLBACK_IMAGE_RESIZE,
                                    _edje_object_part_swallow_image_resize_cb,
                                    rp);

   if (hints_update)
     _edje_real_part_swallow_hints_update(rp);

   if (rp->part->mouse_events)
     {
        _edje_callbacks_add(obj_swallow, ed, rp);
        if (rp->part->repeat_events)
          evas_object_repeat_events_set(obj_swallow, 1);
        if (rp->part->pointer_mode != EVAS_OBJECT_POINTER_MODE_AUTOGRAB)
          evas_object_pointer_mode_set(obj_swallow, rp->part->pointer_mode);
        evas_object_pass_events_set(obj_swallow, 0);
     }
   else
     evas_object_pass_events_set(obj_swallow, 1);
   _edje_callbacks_focus_add(rp->typedata.swallow->swallowed_object, ed, rp);

   if (rp->part->precise_is_inside)
     evas_object_precise_is_inside_set(obj_swallow, 1);

   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
   ed->recalc_hints = EINA_TRUE;
   _edje_recalc(ed);
}

void
_edje_real_part_swallow_clear(Edje *ed, Edje_Real_Part *rp)
{
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return;
   if (!rp->typedata.swallow->swallowed_object) return;
   evas_object_smart_member_del(rp->typedata.swallow->swallowed_object);
   evas_object_event_callback_del_full(rp->typedata.swallow->swallowed_object,
                                       EVAS_CALLBACK_DEL,
                                       _edje_object_part_swallow_free_cb,
                                       rp);
   evas_object_event_callback_del_full(rp->typedata.swallow->swallowed_object,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _edje_object_part_swallow_changed_hints_cb,
                                       rp);
   if (eo_isa(rp->typedata.swallow->swallowed_object, EVAS_OBJ_IMAGE_CLASS))
     evas_object_event_callback_del_full(rp->typedata.swallow->swallowed_object,
                                         EVAS_CALLBACK_IMAGE_RESIZE,
                                         _edje_object_part_swallow_image_resize_cb,
                                         rp);
   evas_object_clip_unset(rp->typedata.swallow->swallowed_object);
   evas_object_data_del(rp->typedata.swallow->swallowed_object, "\377 edje.swallowing_part");
   _edje_callbacks_del(rp->typedata.swallow->swallowed_object, ed);
   _edje_callbacks_focus_del(rp->typedata.swallow->swallowed_object, ed);
   rp->typedata.swallow->swallowed_object = NULL;
}

static void
_edje_object_preload(Edje *ed)
{
   ed->preload_count--;
   if (!ed->preload_count)
     _edje_emit(ed, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE);
}

static void
_edje_object_image_preload_cb(void *data, EINA_UNUSED Evas *e, Evas_Object *obj, EINA_UNUSED void *event_info)
{
   Edje *ed = data;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_IMAGE_PRELOADED, _edje_object_image_preload_cb, ed);
   _edje_object_preload(ed);
}

static void
_edje_object_signal_preload_cb(void *data, Evas_Object *obj, EINA_UNUSED const char *emission, EINA_UNUSED const char *source)
{
   Edje *ed = data;

   edje_object_signal_callback_del(obj, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE, _edje_object_signal_preload_cb);
   _edje_object_preload(ed);
}

/**
 * @internal
 * 
 * for edje_cc
 */
EAPI void
_edje_program_remove(Edje_Part_Collection *edc, Edje_Program *p)
{
   Edje_Program ***array;
   unsigned int *count;
   unsigned int i;

   if (!p->signal && !p->source)
     {
        array = &edc->programs.nocmp;
        count = &edc->programs.nocmp_count;
     }
   else if (p->signal && !strpbrk(p->signal, "*?[\\")
            && p->source && !strpbrk(p->source, "*?[\\"))
     {
        array = &edc->programs.strcmp;
        count = &edc->programs.strcmp_count;
     }
   else if (p->signal && edje_program_is_strncmp(p->signal)
            && p->source && edje_program_is_strncmp(p->source))
     {
        array = &edc->programs.strncmp;
        count = &edc->programs.strncmp_count;
     }
   else if (p->signal && edje_program_is_strrncmp(p->signal)
            && p->source && edje_program_is_strrncmp(p->source))
     {
        array = &edc->programs.strrncmp;
        count = &edc->programs.strrncmp_count;
     }
   else
     {
        array = &edc->programs.fnmatch;
        count = &edc->programs.fnmatch_count;
     }

   for (i = 0; i < *count; ++i)
     if ((*array)[i] == p)
       {
          memmove(*array + i, *array + i + 1, sizeof (Edje_Program *) * (*count - i -1));
          (*count)--;
          break;
       }
}

/**
 * @internal
 * 
 * for edje_cc
 */
EAPI void
_edje_program_insert(Edje_Part_Collection *edc, Edje_Program *p)
{
   Edje_Program ***array;
   unsigned int *count;

   if (!p->signal && !p->source)
     {
        array = &edc->programs.nocmp;
        count = &edc->programs.nocmp_count;
     }
   else if (p->signal && !strpbrk(p->signal, "*?[\\")
            && p->source && !strpbrk(p->source, "*?[\\"))
     {
        array = &edc->programs.strcmp;
        count = &edc->programs.strcmp_count;
     }
   else if (p->signal && edje_program_is_strncmp(p->signal)
            && p->source && edje_program_is_strncmp(p->source))
     {
        array = &edc->programs.strncmp;
        count = &edc->programs.strncmp_count;
     }
   else if (p->signal && edje_program_is_strrncmp(p->signal)
            && p->source && edje_program_is_strrncmp(p->source))
     {
        array = &edc->programs.strrncmp;
        count = &edc->programs.strrncmp_count;
     }
   else
     {
        array = &edc->programs.fnmatch;
        count = &edc->programs.fnmatch_count;
     }

   *array = realloc(*array, sizeof (Edje_Program *) * (*count + 1));
   (*array)[(*count)++] = p;
}

const char *
edje_string_get(const Edje_String *es)
{
   /* FIXME: Handle localization here */
   if (!es) return NULL;
   return es->str;
}

const char *
edje_string_id_get(const Edje_String *es)
{
   /* FIXME: Handle localization here */
   if (!es) return NULL;
   return es->str;
}

static void
_cb_subobj_del(void *data, EINA_UNUSED Evas *e, Evas_Object *obj, EINA_UNUSED void *event_info)
{
   Edje *ed = data;
   _edje_subobj_unregister(ed, obj);
}

void
_edje_subobj_register(Edje *ed, Evas_Object *ob)
{
   ed->subobjs = eina_list_append(ed->subobjs, ob);
   evas_object_event_callback_add(ob, EVAS_CALLBACK_DEL,
                                  _cb_subobj_del, ed);
}

void
_edje_subobj_unregister(Edje *ed, Evas_Object *obj)
{
   ed->subobjs = eina_list_remove(ed->subobjs, obj);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _cb_subobj_del, ed);
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
