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

Edje_Real_Part *_edje_real_part_recursive_get_helper(const Edje *ed, char **path);


static void
_edje_class_member_direct_del(const char *class, Edje_List_Refcount *lookup, Eina_Hash *hash)
{
   Eina_List *members;

   members = eina_hash_find(hash, class);
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
   if (!lookup) return ;
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
   if (!lookup) return ;

   EINA_REFCOUNT_UNREF(lookup)
   {
      members = eina_list_remove_list(members, lookup->lookup);
      eina_hash_set(*ghash, class, members);

      eina_hash_del(*ehash, class, lookup);
      free(lookup);
   }
}

static Eina_Bool
member_list_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
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
   INF("fr ++ ->%i", _edje_freeze_val);
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
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  {
	     Edje *ed2;

	     ed2 = _edje_fetch(rp->swallowed_object);
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
   _edje_freeze_val--;
   INF("fr -- ->%i", _edje_freeze_val);
   if ((_edje_freeze_val <= 0) && (_edje_freeze_calc_count > 0))
     {
        Edje *ed;

	_edje_freeze_calc_count = 0;
	EINA_LIST_FREE(_edje_freeze_calc_list, ed)
	  {
	     _edje_thaw_edje(ed);
             ed->freeze_calc = 0;
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
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return EINA_FALSE;
   if (ed->scale == scale) return EINA_TRUE;
   ed->scale = FROM_DOUBLE(scale);
   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI double
edje_object_scale_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0.0;
   return TO_DOUBLE(ed->scale);
}

EAPI Eina_Bool
edje_object_mirrored_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return EINA_FALSE;

   return ed->is_rtl;
}

void
_edje_object_orientation_inform(Evas_Object *obj)
{
   if (edje_object_mirrored_get(obj))
     edje_object_signal_emit(obj, "edje,state,rtl", "edje");
   else
     edje_object_signal_emit(obj, "edje,state,ltr", "edje");
}

EAPI void
edje_object_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Edje *ed;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return;
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
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!key))
     return NULL;
   if (!ed->collection) return NULL;
   if (!ed->collection->data) return NULL;
   return edje_string_get(eina_hash_find(ed->collection->data, key));
}

EAPI int
edje_object_freeze(Evas_Object *obj)
{
   Edje *ed;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_freeze(rp->swallowed_object);
     }
   return _edje_freeze(ed);
}

EAPI int
edje_object_thaw(Evas_Object *obj)
{
   Edje *ed;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_thaw(rp->swallowed_object);
     }
   return _edje_thaw(ed);
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
	ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	ed->all_part_change = 1;
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
	ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	ed->all_part_change = 1;
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

   memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
   eina_hash_foreach(_edje_color_class_member_hash,
                     _edje_color_class_list_foreach, &fdata);

   return fdata.list;
}

static Eina_Bool
_edje_color_class_list_foreach(const Eina_Hash *hash __UNUSED__, const void *key, void *data __UNUSED__, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = eina_list_append(fd->list, strdup(key));
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_color_class_set(Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Edje *ed;
   Eina_List *l;
   Edje_Color_Class *cc;
   unsigned int i;

   ed = _edje_fetch(obj);
   if ((!ed) || (!color_class)) return EINA_FALSE;
   if (r < 0)        r = 0;
   else if (r > 255) r = 255;
   if (g < 0)        g = 0;
   else if (g > 255) g = 255;
   if (b < 0)        b = 0;
   else if (b > 255) b = 255;
   if (a < 0)        a = 0;
   else if (a > 255) a = 255;
   color_class = eina_stringshare_add(color_class);
   if (!color_class) return EINA_FALSE;
   EINA_LIST_FOREACH(ed->color_classes, l, cc)
     {
	if (cc->name == color_class)
	  {
	     eina_stringshare_del(color_class);

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
	     ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	     ed->all_part_change = 1;
#endif
	     _edje_recalc(ed);
	     return EINA_TRUE;
	  }
     }
   cc = malloc(sizeof(Edje_Color_Class));
   if (!cc)
     {
	eina_stringshare_del(color_class);
	return EINA_FALSE;
     }
   cc->name = color_class;
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
   ed->color_classes = eina_list_append(ed->color_classes, cc);
   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
#endif

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_color_class_set(rp->swallowed_object, color_class,
                                      r, g, b, a, r2, g2, b2, a2, r3, g3, b3,
                                      a3);
     }

   _edje_recalc(ed);
   _edje_emit(ed, "color_class,set", color_class);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_color_class_get(const Evas_Object *obj, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3)
{
   Edje *ed = _edje_fetch(obj);
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
edje_object_color_class_del(Evas_Object *obj, const char *color_class)
{
   Edje *ed;
   Eina_List *l;
   Edje_Color_Class *cc = NULL;
   unsigned int i;

   if (!color_class) return;

   ed = _edje_fetch(obj);
   EINA_LIST_FOREACH(ed->color_classes, l, cc)
     {
	if (!strcmp(cc->name, color_class))
	  {
	     ed->color_classes = eina_list_remove(ed->color_classes, cc);
	     eina_stringshare_del(cc->name);
	     free(cc);
	     return;
	  }
     }

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_color_class_del(rp->swallowed_object, color_class);
     }

   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
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
	return EINA_FALSE;
     }

   /* If the class found is the same just return */
   if ((tc->size == size) && (tc->font) && (!strcmp(tc->font, font)))
     return EINA_TRUE;

   /* Update the class found */
   eina_stringshare_del(tc->font);
   tc->font = eina_stringshare_add(font);
   if (!tc->font)
     {
        eina_hash_del(_edje_text_class_hash, text_class, tc);
	free(tc);
	return EINA_FALSE;
     }
   tc->size = size;

   /* Tell all members of the text class to recalc */
   members = eina_hash_find(_edje_text_class_member_hash, text_class);
   while (members)
     {
	Edje *ed;

	ed = eina_list_data_get(members);
	ed->dirty = 1;
	_edje_textblock_style_all_update(ed);
#ifdef EDJE_CALC_CACHE
	ed->text_part_change = 1;
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
	ed->dirty = 1;
	_edje_textblock_style_all_update(ed);
#ifdef EDJE_CALC_CACHE
	ed->text_part_change = 1;
#endif
	_edje_recalc(ed);
	members = eina_list_next(members);
     }
}

Eina_List *
edje_text_class_list(void)
{
   Edje_List_Foreach_Data fdata;

   memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
   eina_hash_foreach(_edje_text_class_member_hash,
                     _edje_text_class_list_foreach, &fdata);
   return fdata.list;
}

static Eina_Bool
_edje_text_class_list_foreach(const Eina_Hash *hash __UNUSED__, const void *key, void *data __UNUSED__, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = eina_list_append(fd->list, eina_stringshare_add(key));
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_text_class_set(Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size)
{
   Edje *ed;
   Eina_List *l;
   Edje_Text_Class *tc;
   unsigned int i;

   ed = _edje_fetch(obj);
   if ((!ed) || (!text_class)) return EINA_FALSE;

   /* for each text_class in the edje */
   EINA_LIST_FOREACH(ed->text_classes, l, tc)
     {
	if ((tc->name) && (!strcmp(tc->name, text_class)))
	  {
	     /* Match and the same, return */
	     if ((tc->font) && (font) && (!strcmp(tc->font, font)) &&
		 (tc->size == size))
	       return EINA_TRUE;

	     /* No font but size is the same, return */
	     if ((!tc->font) && (!font) && (tc->size == size)) return EINA_TRUE;

	     /* Update new text class properties */
	     if (tc->font) eina_stringshare_del(tc->font);
	     if (font) tc->font = eina_stringshare_add(font);
	     else tc->font = NULL;
	     tc->size = size;

	     /* Update edje */
	     ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	     ed->text_part_change = 1;
#endif
	     _edje_recalc(ed);
	     return EINA_TRUE;
	  }
     }

   /* No matches, create a new text class */
   tc = calloc(1, sizeof(Edje_Text_Class));
   if (!tc) return EINA_FALSE;
   tc->name = eina_stringshare_add(text_class);
   if (!tc->name)
     {
	free(tc);
	return EINA_FALSE;
     }
   if (font) tc->font = eina_stringshare_add(font);
   else tc->font = NULL;
   tc->size = size;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_text_class_set(rp->swallowed_object, text_class,
                                     font, size);
     }

   /* Add to edje's text class list */
   ed->text_classes = eina_list_append(ed->text_classes, tc);
   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->text_part_change = 1;
#endif
   _edje_textblock_style_all_update(ed);
   _edje_recalc(ed);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_exists(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   return EINA_TRUE;
}

EAPI const Evas_Object *
edje_object_part_object_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   return rp->object;
}

EAPI Eina_Bool
edje_object_part_geometry_get(const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h )
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return EINA_FALSE;
     }
   if (x) *x = rp->x;
   if (y) *y = rp->y;
   if (w) *w = rp->w;
   if (h) *h = rp->h;
   return EINA_TRUE;
}

EAPI void
edje_object_item_provider_set(Evas_Object *obj, Edje_Item_Provider_Cb func, void *data)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return;
   ed->item_provider.func = func;
   ed->item_provider.data = data;
}

/* FIXDOC: New Function */
EAPI void
edje_object_text_change_cb_set(Evas_Object *obj, Edje_Text_Change_Cb func, void *data)
{
   Edje *ed;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return;
   ed->text_change.func = func;
   ed->text_change.data = data;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if ((rp->part->type == EDJE_PART_TYPE_GROUP) && (rp->swallowed_object))
           edje_object_text_change_cb_set(rp->swallowed_object, func, data);
     }
}

Eina_Bool
_edje_object_part_text_raw_set(Evas_Object *obj, Edje_Real_Part *rp, const char *part, const char *text)
{
   if ((!rp->text.text) && (!text))
     return EINA_FALSE;
   if ((rp->text.text) && (text) &&
       (!strcmp(rp->text.text, text)))
     return EINA_FALSE;
   if (rp->text.text)
     {
	eina_stringshare_del(rp->text.text);
	rp->text.text = NULL;
     }
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_text_markup_set(rp, text);
   else
     if (text) rp->text.text = eina_stringshare_add(text);
   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
   if (rp->edje->text_change.func)
     rp->edje->text_change.func(rp->edje->text_change.data, obj, part);
   return EINA_TRUE;
}

Eina_Bool
_edje_object_part_text_raw_append(Evas_Object *obj, Edje_Real_Part *rp, const char *part, const char *text)
{
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_text_markup_append(rp, text);
   else if (text)
     {
        if (rp->text.text)
          {
             char *new = NULL;
             int len_added = strlen(text);
             int len_old = strlen(rp->text.text);
             new = malloc(len_old + len_added + 1);
             memcpy(new, rp->text.text, len_old);
             memcpy(new + len_old, text, len_added);
             new[len_old + len_added] = '\0';
             eina_stringshare_replace(&rp->text.text, new);
             free(new);
          }
        else
          {
             eina_stringshare_replace(&rp->text.text, text);
          }
     }
   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
   if (rp->edje->text_change.func)
     rp->edje->text_change.func(rp->edje->text_change.data, obj, part);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_text_set(Evas_Object *obj, const char *part, const char *text)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return EINA_FALSE;
   return _edje_object_part_text_raw_set(obj, rp, part, text);
}

EAPI const char *
edje_object_part_text_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_text_get(rp);
   else
     {
	if (rp->part->type == EDJE_PART_TYPE_TEXT) return rp->text.text;
	if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
	  return evas_object_textblock_text_markup_get(rp->object);
     }
   return NULL;
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
   Edje *ed;
   Edje_Real_Part *rp;
   Eina_Bool ret = EINA_FALSE;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return ret;
   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return ret;
   if (rp->part->type == EDJE_PART_TYPE_TEXT)
     ret = _edje_object_part_text_raw_set(obj, rp, part, text_to_escape);
   else if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     {
	char *text = _edje_text_escape(text_to_escape);

	ret = _edje_object_part_text_raw_set(obj, rp, part, text);
	free(text);
     }
   return ret;
}

EAPI char *
edje_object_part_text_unescaped_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
	const char *t = _edje_entry_text_get(rp);
	return _edje_text_unescape(t);
     }
   else
     {
	if (rp->part->type == EDJE_PART_TYPE_TEXT) return strdup(rp->text.text);
	if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
	  {
	     const char *t = evas_object_textblock_text_markup_get(rp->object);
	     return _edje_text_unescape(t);
	  }
     }
   return NULL;
}

EAPI const char *
edje_object_part_text_selection_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_selection_get(rp);
   return NULL;
}

EAPI void
edje_object_part_text_select_none(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_none(rp);
}

EAPI void
edje_object_part_text_select_all(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_all(rp);
}

EAPI void
edje_object_part_text_insert(Evas_Object *obj, const char *part, const char *text)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   if (rp->part->entry_mode <= EDJE_ENTRY_EDIT_MODE_NONE) return;
   _edje_entry_text_markup_insert(rp, text);
   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
   if (rp->edje->text_change.func)
     rp->edje->text_change.func(rp->edje->text_change.data, obj, part);
}

EAPI void
edje_object_part_text_append(Evas_Object *obj, const char *part, const char *text)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   _edje_object_part_text_raw_append(obj, rp, part, text);
   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
   if (rp->edje->text_change.func)
     rp->edje->text_change.func(rp->edje->text_change.data, obj, part);
}

EAPI const Eina_List *
edje_object_part_text_anchor_list_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_anchors_list(rp);
   return NULL;
}

EAPI const Eina_List *
edje_object_part_text_anchor_geometry_get(const Evas_Object *obj, const char *part, const char *anchor)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_anchor_geometry_get(rp, anchor);
   return NULL;
}

EAPI const Eina_List *
edje_object_part_text_item_list_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_items_list(rp);
   return NULL;
}

EAPI Eina_Bool
edje_object_part_text_item_geometry_get(const Evas_Object *obj, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_item_geometry_get(rp, item, cx, cy, cw, ch);
   return EINA_FALSE;
}

EAPI void
edje_object_part_text_cursor_geometry_get(const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
	_edje_entry_cursor_geometry_get(rp, x, y, w, h);
	if (x) *x -= rp->edje->x;
	if (y) *y -= rp->edje->y;
     }
   return;
}

EAPI void
edje_object_part_text_select_allow_set(const Evas_Object *obj, const char *part, Eina_Bool allow)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_allow_set(rp, allow);
}

EAPI void
edje_object_part_text_select_abort(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_abort(rp);
}

EAPI void
edje_object_part_text_select_begin(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_begin(rp);
}

EAPI void
edje_object_part_text_select_extend(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_extend(rp);
}

EAPI Eina_Bool
edje_object_part_text_cursor_next(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_next(rp, cur);
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_prev(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_prev(rp, cur);
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_up(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_up(rp, cur);
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_down(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_down(rp, cur);
     }
   return EINA_FALSE;
}

EAPI void
edje_object_part_text_cursor_begin_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_begin(rp, cur);
     }
}

EAPI void
edje_object_part_text_cursor_end_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_end(rp, cur);
     }
}

EAPI void
edje_object_part_text_cursor_copy(Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_copy(rp, src, dst);
     }
}

EAPI void
edje_object_part_text_cursor_line_begin_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_line_begin(rp, cur);
     }
}

EAPI void
edje_object_part_text_cursor_line_end_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_line_end(rp, cur);
     }
}

EAPI Eina_Bool
edje_object_part_text_cursor_coord_set(Evas_Object *obj, const char *part,
		Edje_Cursor cur, Evas_Coord x, Evas_Coord y)
{
 Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_coord_set(rp, cur, x, y);
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_is_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_is_format_get(rp, cur);
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return 0;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return 0;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_is_visible_format_get(rp, cur);
     }
   return 0;
}

EAPI const char *
edje_object_part_text_cursor_content_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_content_get(rp, cur);
     }
   return NULL;
}

EAPI void
edje_object_part_text_cursor_pos_set(Evas_Object *obj, const char *part, Edje_Cursor cur, int pos)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_pos_set(rp, cur, pos);
     }
}

EAPI int
edje_object_part_text_cursor_pos_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return 0;
   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return 0;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_pos_get(rp, cur);
     }
   return 0;
}

EAPI void
edje_object_text_insert_filter_callback_add(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data)
{
   Edje *ed;
   Edje_Text_Insert_Filter_Callback *cb;

   ed = _edje_fetch(obj);
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
   Edje *ed;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func))
          {
             void *data = cb->data;
             ed->text_insert_filter_callbacks =
                eina_list_remove_list(ed->text_insert_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             return data;
          }
     }
   return NULL;
}

EAPI void *
edje_object_text_insert_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data)
{
   Edje *ed;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
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
             return tmp;
          }
     }
   return NULL;
}

EAPI Eina_Bool
edje_object_part_swallow(Evas_Object *obj, const char *part, Evas_Object *obj_swallow)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   /* Need to recalc before providing the object. */
   // XXX: I guess this is not required, removing for testing purposes
   // XXX: uncomment if you see glitches in e17 or others.
   // XXX: by Gustavo, January 21th 2009.
   // XXX: I got a backtrace with over 30000 calls without this,
   // XXX: only with 32px shelves. The problem is probably somewhere else,
   // XXX: but until it's found, leave this here.
   // XXX: by Sachiel, January 21th 2009, 19:30 UTC
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_SWALLOW)
     {
	ERR("cannot unswallow part %s: not swallow type!", rp->part->name);
	return EINA_FALSE;
     }
   _edje_real_part_swallow(rp, obj_swallow, EINA_TRUE);
   return EINA_TRUE;
}

static void
_recalc_extern_parent(Evas_Object *obj)
{
   Evas_Object *parent;
   Edje *ed;

   parent = evas_object_smart_parent_get(obj);
   ed = _edje_fetch(parent);

   ed->dirty = 1;
   _edje_recalc(ed);
}

EAPI void
edje_extern_object_min_size_set(Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   Edje_Real_Part *rp;

   evas_object_size_hint_min_set(obj, minw, minh);
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp)
     {
	rp->swallow_params.min.w = minw;
	rp->swallow_params.min.h = minh;

	_recalc_extern_parent(obj);
     }
}

EAPI void
edje_extern_object_max_size_set(Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh)
{
   Edje_Real_Part *rp;

   evas_object_size_hint_max_set(obj, maxw, maxh);
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp)
     {
	rp->swallow_params.max.w = maxw;
	rp->swallow_params.max.h = maxh;

	_recalc_extern_parent(obj);
     }
}

EAPI void
edje_extern_object_aspect_set(Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah)
{
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
	rp->swallow_params.aspect.mode = aspect;
	rp->swallow_params.aspect.w = aw;
	rp->swallow_params.aspect.h = ah;
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
_edje_box_layout_external_node_cmp(const Eina_Rbtree *left, const Eina_Rbtree *right, __UNUSED__ void *data)
{
   Edje_Box_Layout *l = (Edje_Box_Layout *)left;
   Edje_Box_Layout *r = (Edje_Box_Layout *)right;

   if (strcmp(l->name, r->name) < 0)
     return EINA_RBTREE_RIGHT;
   else
     return EINA_RBTREE_LEFT;
}

static int
_edje_box_layout_external_find_cmp(const Eina_Rbtree *node, const void *key, __UNUSED__ int length, __UNUSED__ void *data)
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
_edje_box_layout_external_free(Eina_Rbtree *node, __UNUSED__ void *data)
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
   if (!l)
     {
	perror("malloc");
	return NULL;
     }

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
edje_object_part_unswallow(Evas_Object *obj __UNUSED__, Evas_Object *obj_swallow)
{
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
   if (rp->swallowed_object == obj_swallow)
     {
	evas_object_smart_member_del(rp->swallowed_object);
	evas_object_event_callback_del_full(rp->swallowed_object,
                                            EVAS_CALLBACK_FREE,
                                            _edje_object_part_swallow_free_cb,
                                            rp->edje->obj);
	evas_object_event_callback_del_full(rp->swallowed_object,
                                            EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _edje_object_part_swallow_changed_hints_cb,
                                            rp);
	evas_object_clip_unset(rp->swallowed_object);
	evas_object_data_del(rp->swallowed_object, "\377 edje.swallowing_part");

	if (rp->part->mouse_events)
	  _edje_callbacks_del(rp->swallowed_object, rp->edje);
	_edje_callbacks_focus_del(rp->swallowed_object, rp->edje);

	rp->swallowed_object = NULL;
	rp->swallow_params.min.w = 0;
	rp->swallow_params.min.h = 0;
	rp->swallow_params.max.w = 0;
	rp->swallow_params.max.h = 0;
	rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
	rp->invalidate = 1;
#endif
	_edje_recalc_do(rp->edje);
	return;
     }
}

EAPI Evas_Object *
edje_object_part_swallow_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   return rp->swallowed_object;
}

EAPI void
edje_object_size_min_get(const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   Edje *ed;

   ed = _edje_fetch(obj);
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
   Edje *ed;

   ed = _edje_fetch(obj);
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
   Edje *ed;
   int pf, pf2;

   ed = _edje_fetch(obj);
   if (!ed) return;
   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
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
   edje_object_size_min_restricted_calc(obj, minw, minh, 0, 0);
}

EAPI Eina_Bool
edje_object_parts_extends_calc(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Edje *ed;
   Evas_Coord x1 = INT_MAX, y1 = INT_MAX;
   Evas_Coord x2 = 0, y2 = 0;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return EINA_FALSE;
     }

   ed->calc_only = 1;

   /* Need to recalc before providing the object. */
   ed->dirty = 1;
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

	if (x1 > rpx1) x1 = rpx1;
	if (y1 > rpy1) y1 = rpy1;
	if (x2 < rpx2) x2 = rpx2;
	if (y2 < rpy2) y2 = rpy2;
     }

   ed->calc_only = 0;

   *x = x1;
   *y = y1;
   *w = x2 - x1;
   *h = y2 - y1;

   return EINA_TRUE;
}

EAPI void
edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh)
{
   Edje *ed;
   Evas_Coord pw, ph;
   int maxw, maxh;
   int okw, okh;
   int reset_maxwh;
   Edje_Real_Part *pep = NULL;
   Eina_Bool has_non_fixed_tb = EINA_FALSE;

   ed = _edje_fetch(obj);
   if ((!ed) || (!ed->collection))
     {
	if (minw) *minw = restrictedw;
	if (minh) *minh = restrictedh;
	return;
     }
   reset_maxwh = 1;
   ed->calc_only = 1;
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
	ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	ed->all_part_change = 1;
#endif
	_edje_recalc_do(ed);
	if (reset_maxwh)
	  {
	     maxw = 0;
	     maxh = 0;
	  }
	pep = NULL;
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
		       if ((ep->part->type == EDJE_PART_TYPE_TEXTBLOCK))
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
   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
#endif
   _edje_recalc(ed);
   ed->calc_only = 0;
}

/* FIXME: Correctly return other states */
EAPI const char *
edje_object_part_state_get(const Evas_Object *obj, const char *part, double *val_ret)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (val_ret) *val_ret = 0;
	return "";
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (val_ret) *val_ret = 0;
	INF("part not found");
	return "";
     }
   if (rp->chosen_description)
     {
	if (val_ret) *val_ret = rp->chosen_description->state.value;
	if (rp->chosen_description->state.name)
	  return rp->chosen_description->state.name;
	return "default";
     }
   else
     {
	if (rp->param1.description)
	  {
	     if (val_ret) *val_ret = rp->param1.description->state.value;
	     if (rp->param1.description->state.name)
	       return rp->param1.description->state.name;
	     return "default";
	  }
     }
   if (val_ret) *val_ret = 0;
   return "";
}

EAPI Edje_Drag_Dir
edje_object_part_drag_dir_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EDJE_DRAG_DIR_NONE;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EDJE_DRAG_DIR_NONE;
   if ((rp->part->dragable.x) && (rp->part->dragable.y)) return EDJE_DRAG_DIR_XY;
   else if (rp->part->dragable.x) return EDJE_DRAG_DIR_X;
   else if (rp->part->dragable.y) return EDJE_DRAG_DIR_Y;
   return EDJE_DRAG_DIR_NONE;
}

EAPI Eina_Bool
edje_object_part_drag_value_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (rp->drag->down.count > 0) return EINA_FALSE;
   if (rp->part->dragable.confine_id != -1)
     {
	dx = CLAMP(dx, 0.0, 1.0);
	dy = CLAMP(dy, 0.0, 1.0);
     }
   if (rp->part->dragable.x < 0) dx = 1.0 - dx;
   if (rp->part->dragable.y < 0) dy = 1.0 - dy;
   if ((rp->drag->val.x == FROM_DOUBLE(dx)) && (rp->drag->val.y == FROM_DOUBLE(dy))) return EINA_TRUE;
   rp->drag->val.x = FROM_DOUBLE(dx);
   rp->drag->val.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_dragable_pos_set(rp->edje, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(rp->edje, "drag,set", rp->part->name);
   return EINA_TRUE;
}

/* FIXME: Should this be x and y instead of dx/dy? */
EAPI Eina_Bool
edje_object_part_drag_value_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   double ddx, ddy;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp || !rp->drag)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }
   ddx = TO_DOUBLE(rp->drag->val.x);
   ddy = TO_DOUBLE(rp->drag->val.y);
   if (rp->part->dragable.x < 0) ddx = 1.0 - ddx;
   if (rp->part->dragable.y < 0) ddy = 1.0 - ddy;
   if (dx) *dx = ddx;
   if (dy) *dy = ddy;
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_size_set(Evas_Object *obj, const char *part, double dw, double dh)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (dw < 0.0) dw = 0.0;
   else if (dw > 1.0) dw = 1.0;
   if (dh < 0.0) dh = 0.0;
   else if (dh > 1.0) dh = 1.0;
   if ((rp->drag->size.x == FROM_DOUBLE(dw)) && (rp->drag->size.y == FROM_DOUBLE(dh))) return EINA_TRUE;
   rp->drag->size.x = FROM_DOUBLE(dw);
   rp->drag->size.y = FROM_DOUBLE(dh);
   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_size_get(const Evas_Object *obj, const char *part, double *dw, double *dh)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dw) *dw = 0;
	if (dh) *dh = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp || !rp->drag)
     {
	if (dw) *dw = 0;
	if (dh) *dh = 0;
	return EINA_FALSE;
     }
   if (dw) *dw = TO_DOUBLE(rp->drag->size.x);
   if (dh) *dh = TO_DOUBLE(rp->drag->size.y);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_step_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag->step.x = FROM_DOUBLE(dx);
   rp->drag->step.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_step_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp || !rp->drag)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }
   if (dx) *dx = TO_DOUBLE(rp->drag->step.x);
   if (dy) *dy = TO_DOUBLE(rp->drag->step.y);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_page_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag->page.x = FROM_DOUBLE(dx);
   rp->drag->page.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_page_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp || !rp->drag)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }
   if (dx) *dx = TO_DOUBLE(rp->drag->page.x);
   if (dy) *dy = TO_DOUBLE(rp->drag->page.y);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_step(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   FLOAT_T px, py;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (rp->drag->down.count > 0) return EINA_FALSE;
   px = rp->drag->val.x;
   py = rp->drag->val.y;
   rp->drag->val.x = ADD(px, MUL(FROM_DOUBLE(dx),
				 MUL(rp->drag->step.x, rp->part->dragable.x)));
   rp->drag->val.y = ADD(py, MUL(FROM_DOUBLE(dy),
				 MUL(rp->drag->step.y, rp->part->dragable.y)));
   rp->drag->val.x = CLAMP (rp->drag->val.x, ZERO, FROM_DOUBLE(1.0));
   rp->drag->val.y = CLAMP (rp->drag->val.y, ZERO, FROM_DOUBLE(1.0));
   if ((px == rp->drag->val.x) && (py == rp->drag->val.y)) return EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_dragable_pos_set(rp->edje, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(rp->edje, "drag,step", rp->part->name);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_drag_page(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   FLOAT_T px, py;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (rp->drag->down.count > 0) return EINA_FALSE;
   px = rp->drag->val.x;
   py = rp->drag->val.y;
   rp->drag->val.x = ADD(px, MUL(FROM_DOUBLE(dx), MUL(rp->drag->page.x, rp->part->dragable.x)));
   rp->drag->val.y = ADD(py, MUL(FROM_DOUBLE(dy), MUL(rp->drag->page.y, rp->part->dragable.y)));
   rp->drag->val.x = CLAMP (rp->drag->val.x, ZERO, FROM_DOUBLE(1.0));
   rp->drag->val.y = CLAMP (rp->drag->val.y, ZERO, FROM_DOUBLE(1.0));
   if ((px == rp->drag->val.x) && (py == rp->drag->val.y)) return EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_dragable_pos_set(rp->edje, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(rp->edje, "drag,page", rp->part->name);
   return EINA_TRUE;
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
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part) || (!child)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_append(rp, child);
}

EAPI Eina_Bool
edje_object_part_box_prepend(Evas_Object *obj, const char *part, Evas_Object *child)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_prepend(rp, child);
}

EAPI Eina_Bool
edje_object_part_box_insert_before(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_insert_before(rp, child, reference);
}

EAPI Eina_Bool
edje_object_part_box_insert_at(Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_insert_at(rp, child, pos);
}

EAPI Evas_Object *
edje_object_part_box_remove(Evas_Object *obj, const char *part, Evas_Object *child)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return NULL;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return NULL;

   return _edje_real_part_box_remove(rp, child);
}

EAPI Evas_Object *
edje_object_part_box_remove_at(Evas_Object *obj, const char *part, unsigned int pos)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return NULL;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return NULL;

   return _edje_real_part_box_remove_at(rp, pos);
}

EAPI Eina_Bool
edje_object_part_box_remove_all(Evas_Object *obj, const char *part, Eina_Bool clear)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_remove_all(rp, clear);

}

static void
_edje_box_child_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *child __UNUSED__, void *einfo __UNUSED__)
{
   Edje_Real_Part *rp = data;

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

static void
_edje_box_child_add(Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_add
     (child, EVAS_CALLBACK_DEL, _edje_box_child_del_cb, rp);

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

static void
_edje_box_child_remove(Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_del_full
     (child, EVAS_CALLBACK_DEL, _edje_box_child_del_cb, rp);

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

Eina_Bool
_edje_real_part_box_append(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_append(rp->object, child_obj);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_box_child_add(rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_prepend(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_prepend(rp->object, child_obj);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_box_child_add(rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_insert_before(Edje_Real_Part *rp, Evas_Object *child_obj, const Evas_Object *ref)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_insert_before(rp->object, child_obj, ref);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_box_child_add(rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_insert_at(Edje_Real_Part *rp, Evas_Object *child_obj, unsigned int pos)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_insert_at(rp->object, child_obj, pos);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_box_child_add(rp, child_obj);

   return EINA_TRUE;
}

Evas_Object *
_edje_real_part_box_remove(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   if (evas_object_data_get(child_obj, "\377 edje.box_item")) return NULL;
   if (!evas_object_box_remove(rp->object, child_obj)) return NULL;
   _edje_box_layout_remove_child(rp, child_obj);
   _edje_box_child_remove(rp, child_obj);
   return child_obj;
}

Evas_Object *
_edje_real_part_box_remove_at(Edje_Real_Part *rp, unsigned int pos)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;
   Evas_Object *child_obj;

   priv = evas_object_smart_data_get(rp->object);
   opt = eina_list_nth(priv->children, pos);
   if (!opt) return NULL;
   child_obj = opt->obj;
   if (evas_object_data_get(child_obj, "\377 edje.box_item")) return NULL;
   if (!evas_object_box_remove_at(rp->object, pos)) return NULL;
   _edje_box_layout_remove_child(rp, child_obj);
   _edje_box_child_remove(rp, child_obj);
   return child_obj;
}

Eina_Bool
_edje_real_part_box_remove_all(Edje_Real_Part *rp, Eina_Bool clear)
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
	     _edje_box_child_remove(rp, child_obj);
	     if (!evas_object_box_remove_at(rp->object, i))
	       return EINA_FALSE;
	     if (clear)
	       evas_object_del(child_obj);
	  }
	children = eina_list_remove_list(children, children);
     }
   return EINA_TRUE;
}

static void
_edje_table_child_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *child __UNUSED__, void *einfo __UNUSED__)
{
   Edje_Real_Part *rp = data;

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

static void
_edje_table_child_add(Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_add
     (child, EVAS_CALLBACK_DEL, _edje_table_child_del_cb, rp);

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

static void
_edje_table_child_remove(Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_del_full
     (child, EVAS_CALLBACK_DEL, _edje_table_child_del_cb, rp);

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

EAPI Evas_Object *
edje_object_part_table_child_get(Evas_Object *obj, const char *part, unsigned int col, unsigned int row)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return NULL;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return NULL;

   return evas_object_table_child_get(rp->object, col, row);
}

EAPI Eina_Bool
edje_object_part_table_pack(Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE;

   return _edje_real_part_table_pack(rp, child_obj, col, row, colspan, rowspan);
}

EAPI Eina_Bool
edje_object_part_table_unpack(Evas_Object *obj, const char *part, Evas_Object *child_obj)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE;

   return _edje_real_part_table_unpack(rp, child_obj);
}

EAPI Eina_Bool
edje_object_part_table_col_row_size_get(const Evas_Object *obj, const char *part, int *cols, int *rows)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE;

   evas_object_table_col_row_size_get(rp->object, cols, rows);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_table_clear(Evas_Object *obj, const char *part, Eina_Bool clear)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE;

   _edje_real_part_table_clear(rp, clear);
   return EINA_TRUE;
}

static void
_edje_perspective_obj_del(void *data, __UNUSED__ Evas *e, __UNUSED__ Evas_Object *obj, __UNUSED__ void *event_info)
{
   Edje_Perspective *ps = data;
   Evas_Object *o;

   EINA_LIST_FREE(ps->users, o)
     {
        Edje *ed;

        ed = evas_object_smart_data_get(o);
        if (!ed) continue;
        ed->persp = NULL;
        ed->dirty = 1;
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

        ed = evas_object_smart_data_get(o);
        if (!ed) continue;
        if (!ed->persp)
          {
             ed->dirty = 1;
             _edje_recalc_do(ed);
          }
     }
   if (ps->global)
     {
        EINA_LIST_FOREACH(_edje_edjes, l, o)
          {
             Edje *ed;

             ed = evas_object_smart_data_get(o);
             if (!ed) continue;
             if (!ed->persp)
               {
                  ed->dirty = 1;
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

        ed = evas_object_smart_data_get(o);
        if (!ed) continue;
        if (!ed->persp)
          {
             ed->dirty = 1;
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
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (ed->persp == ps) return;
   if (ed->persp != ps)
     {
        if (ed->persp)
          ed->persp->users = eina_list_remove(ed->persp->users, obj);
     }
   ed->persp = ps;
   if (ps) ps->users = eina_list_append(ps->users, obj);
   ed->dirty = 1;
   _edje_recalc_do(ed);
}

EAPI const Edje_Perspective *
edje_object_perspective_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return NULL;
   return ed->persp;
}

#define EDJE_PRELOAD_EMISSION "preload,done"
#define EDJE_PRELOAD_SOURCE NULL

EAPI Eina_Bool
edje_object_preload(Evas_Object *obj, Eina_Bool cancel)
{
   Edje *ed;
   int count;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return EINA_FALSE;

   _edje_recalc_do(ed);

   for (i = 0, count = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	Edje_Part *ep;

	rp = ed->table_parts[i];
	ep = rp->part;

	if (ep->type == EDJE_PART_TYPE_IMAGE ||
	    (ep->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object))
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
		  if (rp->swallowed_object) {
		     edje_object_signal_callback_del(rp->swallowed_object, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE, _edje_object_signal_preload_cb);
		     edje_object_signal_callback_add(rp->swallowed_object, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE, _edje_object_signal_preload_cb, ed);
		     edje_object_preload(rp->swallowed_object, cancel);

		     count--;
		  }
	       }
	  }
     }
   else
     {
	_edje_emit(ed, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE);
     }

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_table_pack(Edje_Real_Part *rp, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Eina_Bool ret =
     evas_object_table_pack(rp->object, child_obj, col, row, colspan, rowspan);

   _edje_table_child_add(rp, child_obj);

   return ret;
}

Eina_Bool
_edje_real_part_table_unpack(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Eina_Bool ret = evas_object_table_unpack(rp->object, child_obj);

   if (ret)
     _edje_table_child_remove(rp, child_obj);

   return ret;
}

void
_edje_real_part_table_clear(Edje_Real_Part *rp, Eina_Bool clear)
{
   Eina_List *children;

   children = evas_object_table_children_get(rp->object);
   while (children)
     {
	Evas_Object *child_obj = children->data;

	_edje_table_child_remove(rp, child_obj);
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
_edje_real_part_recursive_get(const Edje *ed, const char *part)
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
   Evas_Object *child;
   Eina_List *l;
   long int v;
   char *p;

   if (!partid) return NULL;

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_EXTERNAL:
         return _edje_external_content_get(rp->swallowed_object, partid);
      case EDJE_PART_TYPE_BOX:
         l = evas_object_box_children_get(rp->object);
         break;
      case EDJE_PART_TYPE_TABLE:
         l = evas_object_table_children_get(rp->object);
         break;
      default:
         return NULL;
     }

   v = strtol(partid, &p, 10);
   if ((*p == '\0') && (v >= 0))
     {
        child = eina_list_nth(l, v);
     }
   else
     {
        Evas_Object *cur;
        child = NULL;
        EINA_LIST_FREE(l, cur)
          {
             const char *name = evas_object_name_get(cur);
             if ((name) && (!strcmp(name, partid)))
               {
                  child = cur;
                  break;
               }
          }
     }
   eina_list_free(l);

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
_edje_real_part_recursive_get_helper(const Edje *ed, char **path)
{
   Edje_Real_Part *rp;
   Evas_Object *child;
   char *idx = NULL;

   if (!path[0])
     return NULL;

   if (ed->collection && ed->collection->alias)
     {
        char *alias;

        alias = _edje_merge_path(eina_hash_find(ed->collection->alias, path[0]), path + 1);
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

   rp = _edje_real_part_get(ed, path[0]);
   if (!path[1] && !idx) return rp;
   if (!rp) return NULL;

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_GROUP:
	 if (!rp->swallowed_object) return NULL;
	 ed = _edje_fetch(rp->swallowed_object);
	 if (!ed) return NULL;
	 path++;
	 return _edje_real_part_recursive_get_helper(ed, path);
      case EDJE_PART_TYPE_BOX:
      case EDJE_PART_TYPE_TABLE:
      case EDJE_PART_TYPE_EXTERNAL:
	 if (!idx) return rp;
	 path++;

	 child = _edje_children_get(rp, idx);

         ed = _edje_fetch(child);

	 if (!ed) return NULL;
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
_edje_color_class_find(Edje *ed, const char *color_class)
{
   Eina_List *l;
   Edje_Color_Class *cc = NULL;

   if ((!ed) || (!color_class)) return NULL;

   /* first look through the object scope */
   EINA_LIST_FOREACH(ed->color_classes, l, cc)
     if ((cc->name) && (!strcmp(color_class, cc->name))) return cc;

   /* next look through the global scope */
   cc = eina_hash_find(_edje_color_class_hash, color_class);
   if (cc) return cc;

   /* finally, look through the file scope */
   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if ((cc->name) && (!strcmp(color_class, cc->name))) return cc;

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
color_class_hash_list_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
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
text_class_hash_list_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
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

   if (!evas_object_smart_type_check(obj, "edje"))
     return NULL;
   ed = evas_object_smart_data_get(obj);
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
   if (ed->block == 0) ed->block_break = 0;
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
   if (ed->block > 0) ed->block_break = 1;
}

void
_edje_object_part_swallow_free_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *edje_obj;

   edje_obj = data;
   edje_object_part_unswallow(edje_obj, obj);
   return;
}

static void
_edje_real_part_swallow_hints_update(Edje_Real_Part *rp)
{
   const char *type;

   type = evas_object_type_get(rp->swallowed_object);

   rp->swallow_params.min.w = 0;
   rp->swallow_params.min.h = 0;
   rp->swallow_params.max.w = -1;
   rp->swallow_params.max.h = -1;
   if ((type) && (!strcmp(type, "edje")))
     {
	Evas_Coord w, h;

	edje_object_size_min_get(rp->swallowed_object, &w, &h);
	rp->swallow_params.min.w = w;
	rp->swallow_params.min.h = h;
	edje_object_size_max_get(rp->swallowed_object, &w, &h);
	rp->swallow_params.max.w = w;
	rp->swallow_params.max.h = h;
     }
   else if ((type) && ((!strcmp(type, "text")) || (!strcmp(type, "polygon")) ||
		       (!strcmp(type, "line"))))
     {
	Evas_Coord w, h;

	evas_object_geometry_get(rp->swallowed_object, NULL, NULL, &w, &h);
	rp->swallow_params.min.w = w;
	rp->swallow_params.min.h = h;
	rp->swallow_params.max.w = w;
	rp->swallow_params.max.h = h;
     }
     {
	Evas_Coord w1, h1, w2, h2, aw, ah;
	Evas_Aspect_Control am;

	evas_object_size_hint_min_get(rp->swallowed_object, &w1, &h1);
	evas_object_size_hint_max_get(rp->swallowed_object, &w2, &h2);
	evas_object_size_hint_aspect_get(rp->swallowed_object, &am, &aw, &ah);
	rp->swallow_params.min.w = w1;
	rp->swallow_params.min.h = h1;
	if (w2 > 0) rp->swallow_params.max.w = w2;
	if (h2 > 0) rp->swallow_params.max.h = h2;
  	switch (am)
	  {
	   case EVAS_ASPECT_CONTROL_NONE:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_NONE;
             break;
	   case EVAS_ASPECT_CONTROL_NEITHER:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_NEITHER;
             break;
	   case EVAS_ASPECT_CONTROL_HORIZONTAL:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_HORIZONTAL;
             break;
	   case EVAS_ASPECT_CONTROL_VERTICAL:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_VERTICAL;
             break;
	   case EVAS_ASPECT_CONTROL_BOTH:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_BOTH;
             break;
	   default:
             break;
	  }
	rp->swallow_params.aspect.w = aw;
	rp->swallow_params.aspect.h = ah;
	evas_object_data_set(rp->swallowed_object, "\377 edje.swallowing_part", rp);
     }

#ifdef EDJE_CALC_CACHE
     rp->invalidate = 1;
#endif
}

void
_edje_object_part_swallow_changed_hints_cb(void *data, __UNUSED__ Evas *e, __UNUSED__ Evas_Object *obj, __UNUSED__ void *event_info)
{
   Edje_Real_Part *rp;

   rp = data;
   _edje_real_part_swallow_hints_update(rp);
   rp->edje->dirty = 1;
   _edje_recalc(rp->edje);
   return;
}

void
_edje_real_part_swallow(Edje_Real_Part *rp,
			Evas_Object *obj_swallow,
			Eina_Bool hints_update)
{
   if (rp->swallowed_object)
     {
        if (rp->swallowed_object != obj_swallow)
          {
             _edje_real_part_swallow_clear(rp);
             rp->swallowed_object = NULL;
          }
        else
          {
             if (hints_update)
               _edje_real_part_swallow_hints_update(rp);
             rp->edje->dirty = 1;
             _edje_recalc(rp->edje);
             return;
          }
     }
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   if (!obj_swallow) return;
   rp->swallowed_object = obj_swallow;
   evas_object_smart_member_add(rp->swallowed_object, rp->edje->obj);
   if (rp->clip_to)
     evas_object_clip_set(rp->swallowed_object, rp->clip_to->object);
   else evas_object_clip_set(rp->swallowed_object, rp->edje->base.clipper);
   evas_object_stack_above(rp->swallowed_object, rp->object);
   evas_object_event_callback_add(rp->swallowed_object,
                                  EVAS_CALLBACK_FREE,
				  _edje_object_part_swallow_free_cb,
				  rp->edje->obj);
   evas_object_event_callback_add(rp->swallowed_object,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _edje_object_part_swallow_changed_hints_cb,
				  rp);

   if (hints_update)
     _edje_real_part_swallow_hints_update(rp);

   if (rp->part->mouse_events)
     {
        _edje_callbacks_add(obj_swallow, rp->edje, rp);
	if (rp->part->repeat_events)
           evas_object_repeat_events_set(obj_swallow, 1);
	if (rp->part->pointer_mode != EVAS_OBJECT_POINTER_MODE_AUTOGRAB)
	  evas_object_pointer_mode_set(obj_swallow, rp->part->pointer_mode);
	evas_object_pass_events_set(obj_swallow, 0);
     }
   else
     evas_object_pass_events_set(obj_swallow, 1);
   _edje_callbacks_focus_add(rp->swallowed_object, rp->edje, rp);

   if (rp->part->precise_is_inside)
     evas_object_precise_is_inside_set(obj_swallow, 1);

   rp->edje->dirty = 1;
   _edje_recalc(rp->edje);
}

void
_edje_real_part_swallow_clear(Edje_Real_Part *rp)
{
   evas_object_smart_member_del(rp->swallowed_object);
   evas_object_event_callback_del_full(rp->swallowed_object,
                                       EVAS_CALLBACK_FREE,
                                       _edje_object_part_swallow_free_cb,
                                       rp->edje->obj);
   evas_object_event_callback_del_full(rp->swallowed_object,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _edje_object_part_swallow_changed_hints_cb,
                                       rp);
   evas_object_clip_unset(rp->swallowed_object);
   evas_object_data_del(rp->swallowed_object, "\377 edje.swallowing_part");
   if (rp->part->mouse_events)
     _edje_callbacks_del(rp->swallowed_object, rp->edje);
   _edje_callbacks_focus_del(rp->swallowed_object, rp->edje);
}

static void
_edje_object_preload(Edje *ed)
{
   ed->preload_count--;
   if (!ed->preload_count)
     _edje_emit(ed, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE);
}

static void
_edje_object_image_preload_cb(void *data, __UNUSED__ Evas *e, Evas_Object *obj, __UNUSED__ void *event_info)
{
   Edje *ed = data;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_IMAGE_PRELOADED, _edje_object_image_preload_cb, ed);
   _edje_object_preload(ed);
}

static void
_edje_object_signal_preload_cb(void *data, Evas_Object *obj, __UNUSED__ const char *emission, __UNUSED__ const char *source)
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


/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
