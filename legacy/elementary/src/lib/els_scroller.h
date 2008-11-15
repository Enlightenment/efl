typedef enum _Elm_Smart_Scroller_Policy
{
   ELM_SMART_SCROLLER_POLICY_OFF,
     ELM_SMART_SCROLLER_POLICY_ON,
     ELM_SMART_SCROLLER_POLICY_AUTO
}
Elm_Smart_Scroller_Policy;

Evas_Object *elm_smart_scroller_add             (Evas *evas);
void elm_smart_scroller_child_set               (Evas_Object *obj, Evas_Object *child);
void elm_smart_scroller_extern_pan_set          (Evas_Object *obj, Evas_Object *pan, void (*pan_set) (Evas_Object *obj, Evas_Coord x, Evas_Coord y), void (*pan_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_max_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_child_size_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y));
void elm_smart_scroller_custom_edje_file_set    (Evas_Object *obj, char *file, char *group);
void elm_smart_scroller_child_pos_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
void elm_smart_scroller_child_pos_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
void elm_smart_scroller_child_region_show       (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
void elm_smart_scroller_child_viewport_size_get (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
void elm_smart_scroller_step_size_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
void elm_smart_scroller_step_size_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
void elm_smart_scroller_page_size_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
void elm_smart_scroller_page_size_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
void elm_smart_scroller_policy_set              (Evas_Object *obj, Elm_Smart_Scroller_Policy hbar, Elm_Smart_Scroller_Policy vbar);
void elm_smart_scroller_policy_get              (Evas_Object *obj, Elm_Smart_Scroller_Policy *hbar, Elm_Smart_Scroller_Policy *vbar);
Evas_Object *elm_smart_scroller_edje_object_get (Evas_Object *obj);
void elm_smart_scroller_single_dir_set          (Evas_Object *obj, Evas_Bool single_dir);
Evas_Bool elm_smart_scroller_single_dir_get     (Evas_Object *obj);
void elm_smart_scroller_theme_set               (Evas_Object *obj, const char *style);
    
