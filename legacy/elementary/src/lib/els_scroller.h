typedef enum _Elm_Smart_Scroller_Policy
{
   ELM_SMART_SCROLLER_POLICY_AUTO,
   ELM_SMART_SCROLLER_POLICY_ON,
   ELM_SMART_SCROLLER_POLICY_OFF
}
Elm_Smart_Scroller_Policy;

Evas_Object *elm_smart_scroller_add             (Evas *evas);
void elm_smart_scroller_child_set               (Evas_Object *obj, Evas_Object *child);
void elm_smart_scroller_extern_pan_set          (Evas_Object *obj, Evas_Object *pan, void (*pan_set) (Evas_Object *obj, Evas_Coord x, Evas_Coord y), void (*pan_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_max_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_min_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_child_size_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y));
void elm_smart_scroller_custom_edje_file_set    (Evas_Object *obj, char *file, char *group);
void elm_smart_scroller_child_pos_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
void elm_smart_scroller_child_pos_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
void elm_smart_scroller_child_region_show       (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
void elm_smart_scroller_child_region_set        (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
void elm_smart_scroller_child_viewport_size_get (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
void elm_smart_scroller_step_size_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
void elm_smart_scroller_step_size_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
void elm_smart_scroller_page_size_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
void elm_smart_scroller_page_size_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
void elm_smart_scroller_policy_set              (Evas_Object *obj, Elm_Smart_Scroller_Policy hbar, Elm_Smart_Scroller_Policy vbar);
void elm_smart_scroller_policy_get              (Evas_Object *obj, Elm_Smart_Scroller_Policy *hbar, Elm_Smart_Scroller_Policy *vbar);
Evas_Object *elm_smart_scroller_edje_object_get (Evas_Object *obj);
void elm_smart_scroller_single_dir_set          (Evas_Object *obj, Eina_Bool single_dir);
Eina_Bool elm_smart_scroller_single_dir_get     (Evas_Object *obj);
void elm_smart_scroller_object_theme_set        (Evas_Object *parent, Evas_Object *obj, const char *clas, const char *group, const char *style);
void elm_smart_scroller_mirrored_set            (Evas_Object *obj, Eina_Bool mirrored);
void elm_smart_scroller_hold_set                (Evas_Object *obj, Eina_Bool hold);
void elm_smart_scroller_freeze_set              (Evas_Object *obj, Eina_Bool freeze);
void elm_smart_scroller_bounce_allow_set        (Evas_Object *obj, Eina_Bool horiz, Eina_Bool vert);
void elm_smart_scroller_bounce_allow_get        (const Evas_Object *obj, Eina_Bool *horiz, Eina_Bool *vert);
void elm_smart_scroller_paging_set              (Evas_Object *obj, double pagerel_h, double pagerel_v, Evas_Coord pagesize_h, Evas_Coord pagesize_v);
void elm_smart_scroller_paging_get              (Evas_Object *obj, double *pagerel_h, double *pagerel_v, Evas_Coord *pagesize_h, Evas_Coord *pagesize_v);
void elm_smart_scroller_current_page_get        (Evas_Object *obj, int *pagenumber_h, int *pagenumber_v);
void elm_smart_scroller_last_page_get           (Evas_Object *obj, int *pagenumber_h, int *pagenumber_v);
void elm_smart_scroller_page_show               (Evas_Object *obj, int pagenumber_h, int pagenumber_v);
void elm_smart_scroller_page_bring_in           (Evas_Object *obj, int pagenumber_h, int pagenumber_v);
void elm_smart_scroller_region_bring_in         (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
void elm_smart_scroller_widget_set              (Evas_Object *obj, Evas_Object *wid);
void elm_smart_scroller_gravity_set             (Evas_Object *obj, double x, double y);
void elm_smart_scroller_gravity_get             (Evas_Object *obj, double *x, double *y);
Eina_Bool elm_smart_scroller_momentum_animator_disabled_get   (Evas_Object *obj);
void elm_smart_scroller_momentum_animator_disabled_set             (Evas_Object *obj, Eina_Bool disabled);
void elm_smart_scroller_bounce_animator_disabled_set               (Evas_Object *obj, Eina_Bool disabled);
Eina_Bool elm_smart_scroller_bounce_animator_disabled_get     (Evas_Object *obj);
Eina_Bool elm_smart_scroller_wheel_disabled_get (Evas_Object *obj);
void elm_smart_scroller_wheel_disabled_set      (Evas_Object *obj, Eina_Bool disabled);
