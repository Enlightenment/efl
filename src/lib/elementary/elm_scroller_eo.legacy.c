
EAPI void
elm_scroller_custom_widget_base_theme_set(Elm_Scroller *obj, const char *klass, const char *group)
{
   elm_obj_scroller_custom_widget_base_theme_set(obj, klass, group);
}

EAPI void
elm_scroller_page_scroll_limit_set(const Elm_Scroller *obj, int page_limit_h, int page_limit_v)
{
   elm_obj_scroller_page_scroll_limit_set(obj, page_limit_h, page_limit_v);
}

EAPI void
elm_scroller_page_scroll_limit_get(const Elm_Scroller *obj, int *page_limit_h, int *page_limit_v)
{
   elm_obj_scroller_page_scroll_limit_get(obj, page_limit_h, page_limit_v);
}
