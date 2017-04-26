#include "evas_map.h"

// FIXME: cur vs. prev is not handled (may be an issue?)
// FIXME: some render artifacts appear when this API is used (green pixels)

#define EINA_INLIST_REMOVE(l,i) do { l = (__typeof__(l)) eina_inlist_remove(EINA_INLIST_GET(l), EINA_INLIST_GET(i)); } while (0)
#define EINA_INLIST_APPEND(l,i) do { l = (__typeof__(l)) eina_inlist_append(EINA_INLIST_GET(l), EINA_INLIST_GET(i)); } while (0)
#define EINA_INLIST_PREPEND(l,i) do { l = (__typeof__(l)) eina_inlist_prepend(EINA_INLIST_GET(l), EINA_INLIST_GET(i)); } while (0)

#define MY_CLASS EFL_GFX_MAP_MIXIN

typedef struct _Gfx_Map               Gfx_Map;
typedef struct _Gfx_Map_Op            Gfx_Map_Op;
typedef struct _Efl_Gfx_Map_Data      Efl_Gfx_Map_Data;
typedef enum _Gfx_Map_Op_Type         Gfx_Map_Op_Type;

enum _Gfx_Map_Op_Type {
   GFX_MAP_ROTATE_2D,
   GFX_MAP_ROTATE_3D,
   GFX_MAP_ROTATE_QUAT,
   GFX_MAP_ZOOM,
   GFX_MAP_TRANSLATE,
   GFX_MAP_LIGHTNING_3D,
   GFX_MAP_PERSPECTIVE_3D,
};

struct _Gfx_Map_Op {
   EINA_INLIST;

   Gfx_Map_Op_Type op;
   union {
      struct {
         double degrees;
      } rotate_2d;
      struct {
         double dx, dy, dz;
      } rotate_3d;
      struct {
         double qx, qy, qz, qw;
      } rotate_quat;
      struct {
         double zx, zy;
      } zoom;
      struct {
         double dx, dy, dz;
      } translate;
      struct {
         uint8_t lr, lg, lb, ar, ag, ab;
      } lightning_3d;
      struct {
         double z0, foc;
      } perspective_3d;
   };
   struct {
      Eo        *eo_obj; // strong or weak ref?
      double     cx, cy, cz;
      Eina_Bool  event_cbs;
      Eina_Bool  is_canvas;
   } pivot;
};

struct _Gfx_Map {
   Gfx_Map_Op *ops;

   struct {
      double u, v;
      double x, y, z;
      uint8_t r, g, b, a;
   } point[4];

   Eina_Bool alpha;
   Eina_Bool smooth;
   Eina_Bool absolute_xy;
   Eina_Bool event_cbs;
};

struct _Efl_Gfx_Map_Data {
   const Gfx_Map *cow;
};

// ----------------------------------------------------------------------------

static Eina_Cow *gfx_map_cow = NULL;
static const Gfx_Map gfx_map_cow_default = {
   NULL,
   {
      { 0.0, 0.0, 0.0, 0.0, 0.0, 255, 255, 255, 255 },
      { 1.0, 0.0, 0.0, 0.0, 0.0, 255, 255, 255, 255 },
      { 1.0, 1.0, 0.0, 0.0, 0.0, 255, 255, 255, 255 },
      { 0.0, 1.0, 0.0, 0.0, 0.0, 255, 255, 255, 255 }
   },
   EINA_TRUE,
   EINA_TRUE,
   EINA_FALSE,
   EINA_FALSE
};

#define MAPCOW_BEGIN(_pd) eina_cow_write(gfx_map_cow, (const Eina_Cow_Data**)&(_pd->cow))
#define MAPCOW_END(_mapcow, _pd) eina_cow_done(gfx_map_cow, (const Eina_Cow_Data**)&(_pd->cow), _mapcow, EINA_TRUE)
#define MAPCOW_WRITE(pd, name, value) do { \
   if (pd->cow->name != (value)) { \
     Gfx_Map *_cow = MAPCOW_BEGIN(pd); \
     _cow->name = (value); \
     MAPCOW_END(_cow, pd); \
   }} while (0)

#define PIVOT_REF(_pivot) (_pivot ? efl_xref(_pivot, eo_obj) : NULL)
#define PIVOT_UNREF(_pivot) (_pivot ? efl_xunref(_pivot, eo_obj) : NULL)

static inline void _map_ops_clean(Eo *eo_obj, Efl_Gfx_Map_Data *pd);

// ----------------------------------------------------------------------------

void
_efl_gfx_map_init(void)
{
   gfx_map_cow = eina_cow_add("Efl.Gfx.Map", sizeof(Gfx_Map), 8,
                              &gfx_map_cow_default, EINA_TRUE);
}

void
_efl_gfx_map_shutdown(void)
{
   eina_cow_del(gfx_map_cow);
   gfx_map_cow = NULL;
}

// ----------------------------------------------------------------------------

EOLIAN static Efl_Object *
_efl_gfx_map_efl_object_constructor(Eo *eo_obj, Efl_Gfx_Map_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   pd->cow = eina_cow_alloc(gfx_map_cow);
   return eo_obj;
}

EOLIAN static void
_efl_gfx_map_efl_object_destructor(Eo *eo_obj, Efl_Gfx_Map_Data *pd)
{
   _map_ops_clean(eo_obj, pd);
   eina_cow_free(gfx_map_cow, (const Eina_Cow_Data **) &pd->cow);
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

// ----------------------------------------------------------------------------

static void
_geometry_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = data;

   obj->gfx_map_update = EINA_TRUE;
}

static inline void
_map_dirty(Eo *eo_obj, Efl_Gfx_Map_Data *pd, Eina_Bool reset)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Gfx_Map_Op *op;

   obj->gfx_map_has = EINA_TRUE;
   obj->gfx_map_update |= !reset;
   _evas_object_map_enable_set(eo_obj, obj, !reset);
   evas_object_change(eo_obj, obj);

   if (!reset && !pd->cow->absolute_xy)
     {
        if (!pd->cow->event_cbs)
          {
             MAPCOW_WRITE(pd, event_cbs, EINA_TRUE);
             efl_event_callback_add(eo_obj, EFL_GFX_EVENT_MOVE, _geometry_changed_cb, obj);
             efl_event_callback_add(eo_obj, EFL_GFX_EVENT_RESIZE, _geometry_changed_cb, obj);
          }
        EINA_INLIST_FOREACH(pd->cow->ops, op)
          {
             if (op->pivot.eo_obj && !op->pivot.event_cbs)
               {
                  op->pivot.event_cbs = EINA_TRUE;
                  if (!op->pivot.is_canvas)
                    efl_event_callback_add(op->pivot.eo_obj, EFL_GFX_EVENT_MOVE, _geometry_changed_cb, obj);
                  efl_event_callback_add(op->pivot.eo_obj, EFL_GFX_EVENT_RESIZE, _geometry_changed_cb, obj);
               }
          }
     }
}

static void
_map_update(Eo *eo_obj, Efl_Gfx_Map_Data *pd)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Evas_Map *m = NULL;
   int imw, imh;

   if (!obj->gfx_map_update) return;
   if (pd->cow == &gfx_map_cow_default)
     goto end;

   m = evas_map_new(4);
   m->alpha = pd->cow->alpha;
   m->smooth = pd->cow->smooth;
   m->move_sync.enabled = EINA_FALSE;

   if (pd->cow->absolute_xy)
     {
        for (int k = 0; k < 4; k++)
          {
             Evas_Map_Point *p = &(m->points[k]);
             p->px = p->x = pd->cow->point[k].x;
             p->py = p->y = pd->cow->point[k].y;
          }
     }
   else
     {
        _evas_map_util_points_populate(m, obj->cur->geometry.x, obj->cur->geometry.y,
                                       obj->cur->geometry.w, obj->cur->geometry.h, 0);
     }

   if (efl_isa(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS))
     efl_gfx_view_size_get(eo_obj, &imw, &imh);
   else
     efl_gfx_size_get(eo_obj, &imw, &imh);

   for (int k = 0; k < 4; k++)
     {
        Evas_Map_Point *p = &(m->points[k]);
        p->u = pd->cow->point[k].u * imw;
        p->v = pd->cow->point[k].v * imh;
        p->z = pd->cow->point[k].z;
        p->r = pd->cow->point[k].r;
        p->g = pd->cow->point[k].g;
        p->b = pd->cow->point[k].b;
        p->a = pd->cow->point[k].a;
     }

   if (!pd->cow->absolute_xy)
     {
        Gfx_Map_Op *op;

        EINA_INLIST_FOREACH(pd->cow->ops, op)
          {
             int px = 0, py = 0, pw = 1, ph = 1;
             double cx, cy, cz;
             Efl_Gfx *pivot;

             pivot = op->pivot.eo_obj ?: eo_obj;
             if (!op->pivot.is_canvas)
               {
                  efl_gfx_geometry_get(pivot, &px, &py, &pw, &ph);
               }
             else
               {
                  // Note: pivot can not be an Evas when using pure EO API
                  if (efl_isa(pivot, EVAS_CANVAS_CLASS))
                    evas_output_size_get(pivot, &pw, &ph);
                  else
                    efl_gfx_size_get(pivot, &pw, &ph);
               }
             cx = (double) px + (double) pw * op->pivot.cx;
             cy = (double) py + (double) ph * op->pivot.cy;
             cz = op->pivot.cz;

             switch (op->op)
               {
                case GFX_MAP_ROTATE_2D:
                  _map_util_rotate(m, op->rotate_2d.degrees, cx, cy);
                  break;
                case GFX_MAP_ROTATE_3D:
                  _map_util_3d_rotate(m, op->rotate_3d.dx, op->rotate_3d.dy,
                                      op->rotate_3d.dz, cx, cy, cz);
                  break;
                case GFX_MAP_ROTATE_QUAT:
                  _map_util_quat_rotate(m, op->rotate_quat.qx, op->rotate_quat.qy,
                                        op->rotate_quat.qz, op->rotate_quat.qw,
                                        cx, cy, cz);
                  break;
                case GFX_MAP_ZOOM:
                  _map_util_zoom(m, op->zoom.zx, op->zoom.zy, cx, cy);
                  break;
                case GFX_MAP_TRANSLATE:
                  _map_util_translate(m, op->translate.dx, op->translate.dy,
                                      op->translate.dz);
                  break;
                case GFX_MAP_LIGHTNING_3D:
                  _map_util_3d_lighting(m, cx, cy, cz, op->lightning_3d.lr,
                                        op->lightning_3d.lg, op->lightning_3d.lb,
                                        op->lightning_3d.ar, op->lightning_3d.ag,
                                        op->lightning_3d.ab);
                  break;
                case GFX_MAP_PERSPECTIVE_3D:
                  _map_util_3d_perspective(m, cx, cy, op->perspective_3d.z0,
                                           op->perspective_3d.foc);
                  break;
               }
          }
     }
   else if (pd->cow->ops)
     {
        ERR("Map absolute coordinates override all high-level transformations, "
            "such as rotate, perspective, etc...");
     }

end:
   evas_object_map_set(eo_obj, m);
   if (m) evas_map_free(m);
   obj->gfx_map_update = EINA_FALSE;
}

void
_efl_gfx_map_update(Eo *eo_obj)
{
   Efl_Gfx_Map_Data *pd = efl_data_scope_get(eo_obj, MY_CLASS);

   _map_update(eo_obj, pd);
}

static inline void
_map_ops_clean(Eo *eo_obj, Efl_Gfx_Map_Data *pd)
{
   if (pd->cow->ops)
     {
        Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
        Gfx_Map_Op *op;
        Gfx_Map *mcow;

        mcow = MAPCOW_BEGIN(pd);
        EINA_INLIST_FREE(mcow->ops, op)
          {
             EINA_INLIST_REMOVE(mcow->ops, op);
             if (op->pivot.event_cbs)
               {
                  op->pivot.event_cbs = EINA_FALSE;
                  if (!op->pivot.is_canvas)
                    efl_event_callback_del(op->pivot.eo_obj, EFL_GFX_EVENT_MOVE, _geometry_changed_cb, obj);
                  efl_event_callback_del(op->pivot.eo_obj, EFL_GFX_EVENT_RESIZE, _geometry_changed_cb, obj);
               }
             PIVOT_UNREF(op->pivot.eo_obj);
          }
        MAPCOW_END(mcow, pd);
     }
}

EOLIAN Eina_Bool
_efl_gfx_map_map_has(Eo *eo_obj EINA_UNUSED, Efl_Gfx_Map_Data *pd EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);

   if (!obj->map->cur.usemap) return EINA_FALSE;
   if (pd->cow == &gfx_map_cow_default) return EINA_FALSE;
   if (pd->cow->ops) return EINA_TRUE;
   if (memcmp(&pd->cow->point, &gfx_map_cow_default.point, sizeof(pd->cow->point)))
     return EINA_TRUE;
   if (pd->cow->absolute_xy) return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static void
_efl_gfx_map_map_reset(Eo *eo_obj, Efl_Gfx_Map_Data *pd)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   Eina_Bool alpha, smooth;

   alpha = pd->cow->alpha;
   smooth = pd->cow->smooth;
   _map_ops_clean(eo_obj, pd);
   if (pd->cow->event_cbs)
     {
        efl_event_callback_del(eo_obj, EFL_GFX_EVENT_MOVE, _geometry_changed_cb, obj);
        efl_event_callback_del(eo_obj, EFL_GFX_EVENT_RESIZE, _geometry_changed_cb, obj);
     }

   eina_cow_memcpy(gfx_map_cow, (const Eina_Cow_Data * const *) &pd->cow,
                   (const Eina_Cow_Data *) &gfx_map_cow_default);
   _map_dirty(eo_obj, pd, EINA_TRUE);
   MAPCOW_WRITE(pd, alpha, alpha);
   MAPCOW_WRITE(pd, smooth, smooth);
   obj->gfx_map_has = EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_clockwise_get(Eo *eo_obj, Efl_Gfx_Map_Data *pd)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_TRUE);

   _map_update(eo_obj, pd);
   if (!obj->map->cur.map) return EINA_TRUE;
   return evas_map_util_clockwise_get(obj->map->cur.map);
}

EOLIAN static void
_efl_gfx_map_map_smooth_set(Eo *eo_obj, Efl_Gfx_Map_Data *pd, Eina_Bool smooth)
{
   if (pd->cow->smooth == smooth) return;

   MAPCOW_WRITE(pd, smooth, smooth);

   _map_dirty(eo_obj, pd, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_smooth_get(Eo *eo_obj EINA_UNUSED, Efl_Gfx_Map_Data *pd)
{
   return pd->cow->smooth;
}

EOLIAN static void
_efl_gfx_map_map_alpha_set(Eo *eo_obj, Efl_Gfx_Map_Data *pd, Eina_Bool alpha)
{
   if (pd->cow->alpha == alpha) return;

   MAPCOW_WRITE(pd, alpha, alpha);

   _map_dirty(eo_obj, pd, EINA_FALSE);
}

EOLIAN static Eina_Bool
_efl_gfx_map_map_alpha_get(Eo *eo_obj EINA_UNUSED, Efl_Gfx_Map_Data *pd)
{
   return pd->cow->alpha;
}

EOLIAN static void
_efl_gfx_map_map_raw_coord_set(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                               int idx, double x, double y, double z)
{
   Gfx_Map *mcow;

   EINA_SAFETY_ON_FALSE_RETURN((idx >= 0) && (idx < 4));

   if (EINA_DBL_EQ(pd->cow->point[idx].x, x) &&
       EINA_DBL_EQ(pd->cow->point[idx].y, y) &&
       EINA_DBL_EQ(pd->cow->point[idx].z, z) &&
       pd->cow->absolute_xy)
     return;

   mcow = MAPCOW_BEGIN(pd);
   mcow->point[idx].x = x;
   mcow->point[idx].y = y;
   mcow->point[idx].x = x;
   mcow->absolute_xy = EINA_TRUE;
   MAPCOW_END(mcow, pd);

   _map_dirty(eo_obj, pd, EINA_FALSE);
}

EOLIAN static void
_efl_gfx_map_map_raw_coord_get(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                               int idx, double *x, double *y, double *z)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   const Evas_Map *m = obj->map->cur.map;

   EINA_SAFETY_ON_FALSE_RETURN((idx >= 0) && (idx < 4));

   if (pd->cow->absolute_xy)
     {
        if (x) *x = pd->cow->point[idx].x;
        if (y) *y = pd->cow->point[idx].y;
        if (z) *z = pd->cow->point[idx].z;
        return;
     }

   if (!m)
     {
        int X, Y, W, H;

        X = obj->cur->geometry.x;
        Y = obj->cur->geometry.y;
        W = obj->cur->geometry.w;
        H = obj->cur->geometry.h;

        if (x)
          {
             if ((idx == 0) || (idx == 3)) *x = X;
             else *x = X + W;
          }
        if (y)
          {
             if ((idx == 0) || (idx == 1)) *y = Y;
             else *y = Y + H;
          }
        if (z) *z = 0;
        return;
     }

   _map_point_coord_get(m, idx, x, y, z);
}

EOLIAN static void
_efl_gfx_map_map_uv_set(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                        int idx, double u, double v)
{
   Gfx_Map *mcow;

   EINA_SAFETY_ON_FALSE_RETURN((idx >= 0) && (idx < 4));

   if (EINA_DBL_EQ(pd->cow->point[idx].u, u) &&
       EINA_DBL_EQ(pd->cow->point[idx].v, v))
     return;

   mcow = MAPCOW_BEGIN(pd);
   mcow->point[idx].u = CLAMP(0.0, u, 1.0);
   mcow->point[idx].v = CLAMP(0.0, v, 1.0);
   MAPCOW_END(mcow, pd);

   _map_dirty(eo_obj, pd, EINA_FALSE);
}

EOLIAN static void
_efl_gfx_map_map_uv_get(Eo *eo_obj EINA_UNUSED, Efl_Gfx_Map_Data *pd,
                        int idx, double *u, double *v)
{
   EINA_SAFETY_ON_FALSE_RETURN((idx >= 0) && (idx < 4));

   if (u) *u = pd->cow->point[idx].u;
   if (v) *v = pd->cow->point[idx].v;
}

EOLIAN static void
_efl_gfx_map_map_color_set(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                           int idx, int r, int g, int b, int a)
{
   int kmin = 0, kmax = 3;
   Gfx_Map *mcow;

   EINA_SAFETY_ON_FALSE_RETURN((idx >= -1) && (idx < 4));

   if ((idx != -1) &&
       (pd->cow->point[idx].r == r) && (pd->cow->point[idx].g == g) &&
       (pd->cow->point[idx].b == b) && (pd->cow->point[idx].a == a))
     return;

   if (idx >= 0)
     kmin = kmax = idx;

   mcow = MAPCOW_BEGIN(pd);
   for (int k = kmin; k <= kmax; k++)
     {
        mcow->point[k].r = r;
        mcow->point[k].g = g;
        mcow->point[k].b = b;
        mcow->point[k].a = a;
     }
   MAPCOW_END(mcow, pd);

   _map_dirty(eo_obj, pd, EINA_FALSE);
}

EOLIAN static void
_efl_gfx_map_map_color_get(Eo *eo_obj EINA_UNUSED, Efl_Gfx_Map_Data *pd,
                           int idx, int *r, int *g, int *b, int *a)
{
   EINA_SAFETY_ON_FALSE_RETURN((idx >= 0) && (idx < 4));

   if (r) *r = pd->cow->point[idx].r;
   if (g) *g = pd->cow->point[idx].g;
   if (b) *b = pd->cow->point[idx].b;
   if (a) *a = pd->cow->point[idx].a;
}

static Gfx_Map_Op *
_gfx_map_op_add(Eo *eo_obj, Efl_Gfx_Map_Data *pd, Gfx_Map_Op_Type type,
                Efl_Gfx *pivot, double cx, double cy, double cz)
{
   Gfx_Map_Op *op;
   Gfx_Map *mcow;

   op = calloc(1, sizeof(*op));
   if (!op) return NULL;

   if (pivot == eo_obj)
     pivot = NULL;

   op->op = type;
   op->pivot.eo_obj = PIVOT_REF(pivot);
   op->pivot.cx = cx;
   op->pivot.cy = cy;
   op->pivot.cz = cz;

   if (pivot && efl_isa(pivot, EFL_CANVAS_INTERFACE))
     op->pivot.is_canvas = EINA_TRUE;

   mcow = MAPCOW_BEGIN(pd);
   EINA_INLIST_APPEND(mcow->ops, op);
   MAPCOW_END(mcow, pd);

   _map_dirty(eo_obj, pd, EINA_FALSE);

   return op;
}

EOLIAN static void
_efl_gfx_map_rotate(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                    double degrees, Efl_Gfx *pivot, double cx, double cy)
{
   Gfx_Map_Op *op;

   op = _gfx_map_op_add(eo_obj, pd, GFX_MAP_ROTATE_2D, pivot, cx, cy, 0);
   if (!op) return;

   op->rotate_2d.degrees = degrees;
}

EOLIAN static void
_efl_gfx_map_rotate_3d(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                       double dx, double dy, double dz,
                       Efl_Gfx *pivot, double cx, double cy, double cz)
{
   Gfx_Map_Op *op;

   op = _gfx_map_op_add(eo_obj, pd, GFX_MAP_ROTATE_3D, pivot, cx, cy, cz);
   if (!op) return;

   op->rotate_3d.dx = dx;
   op->rotate_3d.dy = dy;
   op->rotate_3d.dz = dz;
}

EOLIAN static void
_efl_gfx_map_rotate_quat(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                         double qx, double qy, double qz, double qw,
                         Efl_Gfx *pivot, double cx, double cy, double cz)
{
   Gfx_Map_Op *op;

   op = _gfx_map_op_add(eo_obj, pd, GFX_MAP_ROTATE_QUAT, pivot, cx, cy, cz);
   if (!op) return;

   op->rotate_quat.qx = qx;
   op->rotate_quat.qy = qy;
   op->rotate_quat.qz = qz;
   op->rotate_quat.qw = qw;
}

EOLIAN static void
_efl_gfx_map_zoom(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                  double zoomx, double zoomy,
                  Efl_Gfx *pivot, double cx, double cy)
{
   Gfx_Map_Op *op;

   op = _gfx_map_op_add(eo_obj, pd, GFX_MAP_ZOOM, pivot, cx, cy, 0);
   if (!op) return;

   op->zoom.zx = zoomx;
   op->zoom.zy = zoomy;
}

EOLIAN static void
_efl_gfx_map_translate(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                       double dx, double dy, double dz)
{
   Gfx_Map_Op *op;

   op = _gfx_map_op_add(eo_obj, pd, GFX_MAP_TRANSLATE, NULL, 0, 0, 0);
   if (!op) return;

   op->translate.dx = dx;
   op->translate.dy = dy;
   op->translate.dz = dz;
}

EOLIAN static void
_efl_gfx_map_lightning_3d(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                          Efl_Gfx *pivot, double lx, double ly, double lz,
                          int lr, int lg, int lb, int ar, int ag, int ab)
{
   Gfx_Map_Op *op;

   op = _gfx_map_op_add(eo_obj, pd, GFX_MAP_LIGHTNING_3D, pivot, lx, ly, lz);
   if (!op) return;

   op->lightning_3d.lr = lr;
   op->lightning_3d.lg = lg;
   op->lightning_3d.lb = lb;
   op->lightning_3d.ar = ar;
   op->lightning_3d.ag = ag;
   op->lightning_3d.ab = ab;
}

EOLIAN static void
_efl_gfx_map_perspective_3d(Eo *eo_obj, Efl_Gfx_Map_Data *pd,
                            Efl_Gfx *pivot, double px, double py,
                            double z0, double foc)
{
   Gfx_Map_Op *op;

   if (foc <= 0.0)
     {
        ERR("Focal length must be greater than 0!");
        return;
     }

   op = _gfx_map_op_add(eo_obj, pd, GFX_MAP_PERSPECTIVE_3D, pivot, px, py, 0);
   if (!op) return;

   op->perspective_3d.z0 = z0;
   op->perspective_3d.foc = foc;
}

#include "canvas/efl_gfx_map.eo.c"
