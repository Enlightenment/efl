#ifndef ELM_INTEFARCE_SCROLLER_H
#define ELM_INTEFARCE_SCROLLER_H

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-scrollable-interface The Elementary Scrollable Interface
 *
 * This is a common interface for widgets having @b scrollable views.
 * Widgets using/implementing this must use the
 * @c EVAS_SMART_SUBCLASS_IFACE_NEW macro (instead of the
 * @c EVAS_SMART_SUBCLASS_NEW one) when declaring its smart class,
 * so an interface is also declared.
 *
 * The scrollable interface comes built with Elementary and is exposed
 * as #ELM_SCROLLABLE_IFACE.
 *
 * The interface API is explained in details at
 * #Elm_Scrollable_Smart_Interface.
 *
 * An Elementary scrollable interface will handle an internal @b
 * panning object. It has the function of clipping and moving the
 * actual scrollable content around, by the command of the scrollable
 * interface calls. Though it's not the common case, one might
 * want/have to change some aspects of the internal panning object
 * behavior.  For that, we have it also exposed here --
 * #Elm_Pan_Smart_Class. Use elm_pan_smart_class_get() to build your
 * custom panning object, when creating a scrollable widget (again,
 * only if you need a custom panning object) and set it with
 * Elm_Scrollable_Smart_Interface::extern_pan_set.
 */

/**
 * @def ELM_PAN_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Pan_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
 #define ELM_PAN_CLASS(x) ((Elm_Pan_Smart_Class *)x)

/**
 * @def ELM_PAN_SMART_CLASS_VERSION
 *
 * Current version for Elementary pan @b base smart class, a value
 * which goes to _Elm_Pan_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_PAN_SMART_CLASS_VERSION 1

/**
 * @def ELM_PAN_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Pan_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_PAN_SMART_CLASS_INIT_NULL
 * @see ELM_PAN_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_PAN_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_PAN_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, \
   NULL, NULL, NULL}

/**
 * @def ELM_PAN_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Pan_Smart_Class structure.
 *
 * @see ELM_PAN_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_PAN_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_PAN_SMART_CLASS_INIT_NULL \
  ELM_PAN_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_PAN_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Pan_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_PAN_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Pan_Smart_Class (base field)
 * to the latest #ELM_PAN_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_PAN_SMART_CLASS_INIT_NULL
 * @see ELM_PAN_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_PAN_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_PAN_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary scroller panning base smart class. This inherits
 * directly from the Evas smart clipped class (an object clipping
 * children to its viewport/size). It is exposed here only to build
 * widgets needing a custom panning behavior.
 */
typedef struct _Elm_Pan_Smart_Class Elm_Pan_Smart_Class;
struct _Elm_Pan_Smart_Class
{
   Evas_Smart_Class base; /* it's a clipped smart object */

   int              version; /**< Version of this smart class definition */

   void             (*pos_set)(Evas_Object *obj,
                               Evas_Coord x,
                               Evas_Coord y);
   void             (*pos_get)(const Evas_Object *obj,
                               Evas_Coord *x,
                               Evas_Coord *y);
   void             (*pos_max_get)(const Evas_Object *obj,
                                   Evas_Coord *x,
                                   Evas_Coord *y);
   void             (*pos_min_get)(const Evas_Object *obj,
                                   Evas_Coord *x,
                                   Evas_Coord *y);
   void             (*content_size_get)(const Evas_Object *obj,
                                        Evas_Coord *x,
                                        Evas_Coord *y);
   void             (*gravity_set)(Evas_Object *obj,
                                   double x,
                                   double y);
   void             (*gravity_get)(const Evas_Object *obj,
                                   double *x,
                                   double *y);
};

/**
 * Elementary scroller panning base smart data.
 */
typedef struct _Elm_Pan_Smart_Data Elm_Pan_Smart_Data;
struct _Elm_Pan_Smart_Data
{
   Evas_Object_Smart_Clipped_Data base;

   const Elm_Pan_Smart_Class     *api; /**< This is the pointer to the object's class, from where we can reach/call its class functions */

   Evas_Object                   *self;
   Evas_Object                   *content;
   Evas_Coord                     x, y, w, h;
   Evas_Coord                     content_w, content_h, px, py;
   double                         gravity_x, gravity_y;
   Evas_Coord                     prev_cw, prev_ch, delta_posx, delta_posy;
};

/**
 * Elementary scrollable interface base data.
 */
typedef struct _Elm_Scrollable_Smart_Interface_Data
  Elm_Scrollable_Smart_Interface_Data;
struct _Elm_Scrollable_Smart_Interface_Data
{
   Evas_Coord          x, y, w, h;
   Evas_Coord          wx, wy, ww, wh; /**< Last "wanted" geometry */

   Evas_Object        *obj;
   Evas_Object        *content;
   Evas_Object        *pan_obj;
   Evas_Object        *edje_obj;
   Evas_Object        *event_rect;

   Evas_Object        *parent_widget;

   Elm_Scroller_Policy hbar_flags, vbar_flags;

   struct
   {
      Evas_Coord x, y;
      Evas_Coord sx, sy;
      Evas_Coord dx, dy;
      Evas_Coord pdx, pdy;
      Evas_Coord bx, by;
      Evas_Coord ax, ay;
      Evas_Coord bx0, by0;
      Evas_Coord b0x, b0y;
      Evas_Coord b2x, b2y;

      struct
      {
         Evas_Coord x, y;
         double     timestamp, localtimestamp;
      } history[60];

      struct
      {
         double tadd, dxsum, dysum;
         double est_timestamp_diff;
      } hist;

      double          anim_start;
      double          anim_start2;
      double          anim_start3;
      double          onhold_vx, onhold_vy, onhold_tlast,
                      onhold_vxe, onhold_vye;
      double          extra_time;

      Evas_Coord      hold_x, hold_y;
      Evas_Coord      locked_x, locked_y;
      int             hdir, vdir;

      Ecore_Animator *hold_animator;
      Ecore_Animator *onhold_animator;
      Ecore_Animator *momentum_animator;
      Ecore_Animator *bounce_x_animator;
      Ecore_Animator *bounce_y_animator;

      Eina_Bool       bounce_x_hold : 1;
      Eina_Bool       bounce_y_hold : 1;
      Eina_Bool       dragged_began : 1;
      Eina_Bool       want_dragged : 1;
      Eina_Bool       hold_parent : 1;
      Eina_Bool       want_reset : 1;
      Eina_Bool       cancelled : 1;
      Eina_Bool       dragged : 1;
      Eina_Bool       locked : 1;
      Eina_Bool       scroll : 1;
      Eina_Bool       dir_x : 1;
      Eina_Bool       dir_y : 1;
      Eina_Bool       hold : 1;
      Eina_Bool       now : 1;
   } down;

   struct
   {
      Evas_Coord w, h;
      Eina_Bool  resized : 1;
   } content_info;

   struct
   {
      Evas_Coord x, y;
   } step, page;

   struct
   {
      void (*drag_start)(Evas_Object *obj,
                         void *data);
      void (*drag_stop)(Evas_Object *obj,
                        void *data);
      void (*animate_start)(Evas_Object *obj,
                            void *data);
      void (*animate_stop)(Evas_Object *obj,
                           void *data);
      void (*scroll)(Evas_Object *obj,
                     void *data);
      void (*edge_left)(Evas_Object *obj,
                        void *data);
      void (*edge_right)(Evas_Object *obj,
                         void *data);
      void (*edge_top)(Evas_Object *obj,
                       void *data);
      void (*edge_bottom)(Evas_Object *obj,
                          void *data);
      void (*content_min_limit)(Evas_Object *obj,
                                Eina_Bool w,
                                Eina_Bool h);
   } cb_func;

   struct
   {
      struct
      {
         Evas_Coord      start, end;
         double          t_start, t_end;
         Ecore_Animator *animator;
      } x, y;
   } scrollto;

   double     pagerel_h, pagerel_v;
   Evas_Coord pagesize_h, pagesize_v;

   Eina_Bool  momentum_animator_disabled : 1;
   Eina_Bool  bounce_animator_disabled : 1;
   Eina_Bool  one_direction_at_a_time : 1;
   Eina_Bool  wheel_disabled : 1;
   Eina_Bool  hbar_visible : 1;
   Eina_Bool  vbar_visible : 1;
   Eina_Bool  bounce_horiz : 1;
   Eina_Bool  bounce_vert : 1;
   Eina_Bool  is_mirrored : 1;
   Eina_Bool  extern_pan : 1;
   Eina_Bool  bouncemey : 1;
   Eina_Bool  bouncemex : 1;
   Eina_Bool  freeze : 1;
   Eina_Bool  hold : 1;
   Eina_Bool  min_w : 1;
   Eina_Bool  min_h : 1;
   Eina_Bool  go_left : 1;
   Eina_Bool  go_right : 1;
   Eina_Bool  go_up : 1;
   Eina_Bool  go_down : 1;
};

typedef struct _Elm_Scrollable_Smart_Interface Elm_Scrollable_Smart_Interface;
struct _Elm_Scrollable_Smart_Interface
{
   Evas_Smart_Interface base;

   void       (*objects_set)(Evas_Object *obj,
                             Evas_Object *edje_obj,
                             Evas_Object *hit_rectangle);
   void       (*content_set)(Evas_Object *obj,
                             Evas_Object *content);

   void       (*extern_pan_set)(Evas_Object *obj,
                                Evas_Object *pan);

   void       (*drag_start_cb_set)(Evas_Object *obj,
                                   void (*d_start_cb)(Evas_Object *obj,
                                                         void *data));
   void       (*drag_stop_cb_set)(Evas_Object *obj,
                                  void (*d_stop_cb)(Evas_Object *obj,
                                                       void *data));
   void       (*animate_start_cb_set)(Evas_Object *obj,
                                      void (*a_start_cb)(Evas_Object *obj,
                                                               void *data));
   void       (*animate_stop_cb_set)(Evas_Object *obj,
                                     void (*a_stop_cb)(Evas_Object *obj,
                                                             void *data));
   void       (*scroll_cb_set)(Evas_Object *obj,
                               void (*s_cb)(Evas_Object *obj,
                                                 void *data));
   void       (*edge_left_cb_set)(Evas_Object *obj,
                                  void (*e_left_cb)(Evas_Object *obj,
                                                       void *data));
   void       (*edge_right_cb_set)(Evas_Object *obj,
                                   void (*e_right_cb)(Evas_Object *obj,
                                                         void *data));
   void       (*edge_top_cb_set)(Evas_Object *obj,
                                 void (*e_top_cb)(Evas_Object *obj,
                                                     void *data));
   void       (*edge_bottom_cb_set)(Evas_Object *obj,
                                    void (*e_bottom_cb)(Evas_Object *obj,
                                                           void *data));

   void       (*content_min_limit_cb_set)(Evas_Object *obj,
                                          void (*c_limit_cb)(Evas_Object *obj,
                                                             Eina_Bool w,
                                                             Eina_Bool h));

   /* set the position of content object inside the scrolling region,
    * immediately */
   void       (*content_pos_set)(Evas_Object *obj,
                                 Evas_Coord x,
                                 Evas_Coord y);
   void       (*content_pos_get)(const Evas_Object *obj,
                                 Evas_Coord *x,
                                 Evas_Coord *y);

   void       (*content_region_show)(Evas_Object *obj,
                                     Evas_Coord x,
                                     Evas_Coord y,
                                     Evas_Coord w,
                                     Evas_Coord h);
   void       (*content_region_set)(Evas_Object *obj,
                                    Evas_Coord x,
                                    Evas_Coord y,
                                    Evas_Coord w,
                                    Evas_Coord h);

   void       (*content_size_get)(const Evas_Object *obj,
                                  Evas_Coord *w,
                                  Evas_Coord *h);

   /* get the size of the actual viewport area (swallowed into
    * scroller Edje object) */
   void       (*content_viewport_size_get)(const Evas_Object *obj,
                                           Evas_Coord *w,
                                           Evas_Coord *h);

   /* this one issues the respective callback, only */
   void       (*content_min_limit)(Evas_Object *obj,
                                   Eina_Bool w,
                                   Eina_Bool h);

   void       (*step_size_set)(Evas_Object *obj,
                               Evas_Coord x,
                               Evas_Coord y);
   void       (*step_size_get)(const Evas_Object *obj,
                               Evas_Coord *x,
                               Evas_Coord *y);
   void       (*page_size_set)(Evas_Object *obj,
                               Evas_Coord x,
                               Evas_Coord y);
   void       (*page_size_get)(const Evas_Object *obj,
                               Evas_Coord *x,
                               Evas_Coord *y);
   void       (*policy_set)(Evas_Object *obj,
                            Elm_Scroller_Policy hbar,
                            Elm_Scroller_Policy vbar);
   void       (*policy_get)(const Evas_Object *obj,
                            Elm_Scroller_Policy *hbar,
                            Elm_Scroller_Policy *vbar);

   void       (*single_direction_set)(Evas_Object *obj,
                                      Eina_Bool single_dir);
   Eina_Bool  (*single_direction_get)(const Evas_Object *obj);

   void       (*mirrored_set)(Evas_Object *obj,
                              Eina_Bool mirrored);

   void       (*hold_set)(Evas_Object *obj,
                          Eina_Bool hold);
   void       (*freeze_set)(Evas_Object *obj,
                            Eina_Bool freeze);

   void       (*bounce_allow_set)(Evas_Object *obj,
                                  Eina_Bool horiz,
                                  Eina_Bool vert);
   void       (*bounce_allow_get)(const Evas_Object *obj,
                                  Eina_Bool *horiz,
                                  Eina_Bool *vert);

   void       (*paging_set)(Evas_Object *obj,
                            double pagerel_h,
                            double pagerel_v,
                            Evas_Coord pagesize_h,
                            Evas_Coord pagesize_v);
   void       (*paging_get)(const Evas_Object *obj,
                            double *pagerel_h,
                            double *pagerel_v,
                            Evas_Coord *pagesize_h,
                            Evas_Coord *pagesize_v);
   void       (*current_page_get)(const Evas_Object *obj,
                                  int *pagenumber_h,
                                  int *pagenumber_v);
   void       (*last_page_get)(const Evas_Object *obj,
                               int *pagenumber_h,
                               int *pagenumber_v);
   void       (*page_show)(Evas_Object *obj,
                           int pagenumber_h,
                           int pagenumber_v);
   void       (*page_bring_in)(Evas_Object *obj,
                               int pagenumber_h,
                               int pagenumber_v);

   void       (*region_bring_in)(Evas_Object *obj,
                                 Evas_Coord x,
                                 Evas_Coord y,
                                 Evas_Coord w,
                                 Evas_Coord h);

   void       (*gravity_set)(Evas_Object *obj,
                             double x,
                             double y);
   void       (*gravity_get)(const Evas_Object *obj,
                             double *x,
                             double *y);

   Eina_Bool  (*momentum_animator_disabled_get)(const Evas_Object *obj);
   void       (*momentum_animator_disabled_set)(Evas_Object *obj,
                                                Eina_Bool disabled);

   void       (*bounce_animator_disabled_set)(Evas_Object *obj,
                                              Eina_Bool disabled);
   Eina_Bool  (*bounce_animator_disabled_get)(const Evas_Object *obj);

   Eina_Bool  (*wheel_disabled_get)(const Evas_Object *obj);
   void       (*wheel_disabled_set)(Evas_Object *obj,
                                    Eina_Bool disabled);
};

EAPI extern const char ELM_SCROLLABLE_IFACE_NAME[];
EAPI extern const Elm_Scrollable_Smart_Interface ELM_SCROLLABLE_IFACE;

EAPI const Elm_Pan_Smart_Class *elm_pan_smart_class_get(void);

#define ELM_SCROLLABLE_IFACE_GET(obj, iface)    \
  const Elm_Scrollable_Smart_Interface * iface; \
  iface = evas_object_smart_interface_get(obj, ELM_SCROLLABLE_IFACE_NAME);

#define ELM_SCROLLABLE_CHECK(obj, ...)                                       \
  const Elm_Scrollable_Smart_Interface * s_iface =                           \
    evas_object_smart_interface_get(obj, ELM_SCROLLABLE_IFACE_NAME);         \
                                                                             \
  if (!s_iface)                                                              \
    {                                                                        \
       ERR("Passing object (%p) of type '%s' in function %s, but it doesn't" \
           " implement the Elementary scrollable interface.", obj,           \
           elm_widget_type_get(obj), __func__);                              \
       if (getenv("ELM_ERROR_ABORT")) abort();                               \
       return __VA_ARGS__;                                                   \
    }

/**
 * @}
 */

#endif
