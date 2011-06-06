#include "edje_private.h"

static void _edje_smart_add(Evas_Object * obj);
static void _edje_smart_del(Evas_Object * obj);
static void _edje_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y);
static void _edje_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h);
static void _edje_smart_show(Evas_Object * obj);
static void _edje_smart_hide(Evas_Object * obj);
static void _edje_smart_calculate(Evas_Object * obj);

static Eina_Bool _edje_smart_file_set(Evas_Object *obj, const char *file, const char *group);

static Edje_Smart_Api _edje_smart_class = EDJE_SMART_API_INIT_NAME_VERSION("edje");
static Evas_Smart_Class _edje_smart_parent;
static Evas_Smart *_edje_smart = NULL;

Eina_List *_edje_edjes = NULL;

/************************** API Routines **************************/

EAPI Evas_Object *
edje_object_add(Evas *evas)
{
   Evas_Object *e;

   if (!_edje_smart)
     {
        memset(&_edje_smart_parent, 0, sizeof(_edje_smart_parent));
        _edje_object_smart_set(&_edje_smart_class);
        _edje_smart =
           evas_smart_class_new((Evas_Smart_Class *)&_edje_smart_class);
     }

   e = evas_object_smart_add(evas, _edje_smart);

   return e;
}

void
_edje_object_smart_set(Edje_Smart_Api *sc)
{
   if (!sc)
     return;

   evas_object_smart_clipped_smart_set(&sc->base);

   _edje_smart_parent.add = sc->base.add; /* Save parent class */
   sc->base.add = _edje_smart_add;
   _edje_smart_parent.del = sc->base.del; /* Save parent class */
   sc->base.del = _edje_smart_del;
   /* we'll handle move thank you */
   sc->base.move = _edje_smart_move;
   sc->base.resize = _edje_smart_resize;
   _edje_smart_parent.show = sc->base.show; /* Save parent class */
   sc->base.show = _edje_smart_show;
   _edje_smart_parent.hide = sc->base.hide; /* Save parent class */
   sc->base.hide = _edje_smart_hide;
   sc->base.calculate = _edje_smart_calculate;
   //sc->base.member_add = NULL;
   //sc->base.member_del = NULL;
   sc->file_set = _edje_smart_file_set;
}

const Edje_Smart_Api *
_edje_object_smart_class_get(void)
{
   static const Edje_Smart_Api *class = NULL;

   if (class)
     return class;

   _edje_object_smart_set(&_edje_smart_class);
   class = &_edje_smart_class;

   return class;
}

/* Private Routines */
static void
_edje_smart_add(Evas_Object *obj)
{
   Edje *ed;
   Evas *tev = evas_object_evas_get(obj);

   evas_event_freeze(tev);
   ed = evas_object_smart_data_get(obj);
   if (!ed)
     {
        const Evas_Smart *smart;
        const Evas_Smart_Class *sc;

        ed = calloc(1, sizeof(Edje));
        if (!ed) goto end_smart_add;

        smart = evas_object_smart_smart_get(obj);
        sc = evas_smart_class_get(smart);
        ed->api = (const Edje_Smart_Api *)sc;

        evas_object_smart_data_set(obj, ed);
     }

   ed->base.evas = evas_object_evas_get(obj);
   ed->base.clipper = evas_object_rectangle_add(ed->base.evas);
   evas_object_static_clip_set(ed->base.clipper, 1);
   evas_object_smart_member_add(ed->base.clipper, obj);
   evas_object_color_set(ed->base.clipper, 255, 255, 255, 255);
   evas_object_move(ed->base.clipper, -10000, -10000);
   evas_object_resize(ed->base.clipper, 20000, 20000);
   evas_object_pass_events_set(ed->base.clipper, 1);
   ed->is_rtl = EINA_FALSE;
   ed->have_objects = 1;
   ed->references = 1;

   evas_object_geometry_get(obj, &(ed->x), &(ed->y), &(ed->w), &(ed->h));
   ed->obj = obj;
   _edje_edjes = eina_list_append(_edje_edjes, obj);
   /*
     {
        Eina_List *l;
        const void *data;

        printf("--- EDJE DUMP [%i]\n", eina_list_count(_edje_edjes));
        EINA_LIST_FOREACH(_edge_edges, l, data)
          {
             ed = _edje_fetch(data);
             printf("EDJE: %80s | %80s\n", ed->path, ed->part);
          }
        printf("--- EDJE DUMP [%i]\n", eina_list_count(_edje_edjes));
     }
   */
end_smart_add:
   evas_event_thaw(tev);
   evas_event_thaw_eval(tev);
}

static void
_edje_smart_del(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   _edje_block_violate(ed);
   ed->delete_me = 1;
   _edje_edjes = eina_list_remove(_edje_edjes, obj);
   evas_object_smart_data_set(obj, NULL);
   if (_edje_script_only(ed)) _edje_script_only_shutdown(ed);
   if (_edje_lua_script_only(ed)) _edje_lua_script_only_shutdown(ed);
   if (ed->persp) edje_object_perspective_set(obj, NULL);
   _edje_file_del(ed);
   _edje_clean_objects(ed);
   _edje_unref(ed);
}

static void
_edje_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((ed->x == x) && (ed->y == y)) return;
   ed->x = x;
   ed->y = y;
//   evas_object_move(ed->clipper, ed->x, ed->y);

   if (_edje_script_only(ed))
     {
        _edje_script_only_move(ed);
        return;
     }
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_move(ed);
        return;
     }

   if (ed->have_mapped_part)
     {
        ed->dirty = 1;
        _edje_recalc_do(ed);
     }
   else
     {
        unsigned int i;

        for (i = 0; i < ed->table_parts_size; i++)
          {
             Edje_Real_Part *ep;
             Evas_Coord ox, oy;

             ep = ed->table_parts[i];
             evas_object_geometry_get(ep->object, &ox, &oy, NULL, NULL);
             evas_object_move(ep->object, ed->x + ep->x + ep->text.offset.x, ed->y + ep->y + ep->text.offset.y);
             if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
               _edje_entry_real_part_configure(ep);
             if (ep->swallowed_object)
               {
                  evas_object_geometry_get(ep->swallowed_object, &ox, &oy, NULL, NULL);
                  evas_object_move(ep->swallowed_object, ed->x + ep->x + ep->text.offset.x, ed->y + ep->y + ep->text.offset.y);
               }
          }
     }
//   _edje_emit(ed, "move", NULL);
}

static void
_edje_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((w == ed->w) && (h == ed->h)) return;
   ed->w = w;
   ed->h = h;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
#endif
   if (_edje_script_only(ed))
     {
        _edje_script_only_resize(ed);
        return;
     }
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_resize(ed);
        return;
     }
//   evas_object_resize(ed->clipper, ed->w, ed->h);
   ed->dirty = 1;
   _edje_recalc_do(ed);
   _edje_emit(ed, "resize", NULL);
}

static void
_edje_smart_show(Evas_Object * obj)
{
   Edje *ed;

   _edje_smart_parent.show(obj);
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (evas_object_visible_get(obj)) return;
   if (_edje_script_only(ed))
     {
        _edje_script_only_show(ed);
        return;
     }
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_show(ed);
        return;
     }
   _edje_emit(ed, "show", NULL);
}

static void
_edje_smart_hide(Evas_Object * obj)
{
   Edje *ed;

   _edje_smart_parent.hide(obj);
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (!evas_object_visible_get(obj)) return;
   if (_edje_script_only(ed))
     {
        _edje_script_only_hide(ed);
        return;
     }
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_hide(ed);
        return;
     }
   _edje_emit(ed, "hide", NULL);
}

static void
_edje_smart_calculate(Evas_Object *obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   _edje_recalc_do(ed);
}

static Eina_Bool
_edje_smart_file_set(Evas_Object *obj, const char *file, const char *group)
{
   return _edje_object_file_set_internal(obj, file, group, NULL, NULL);
}
