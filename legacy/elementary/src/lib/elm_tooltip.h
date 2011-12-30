   /**
    * @defgroup Tooltips Tooltips
    *
    * The Tooltip is an (internal, for now) smart object used to show a
    * content in a frame on mouse hover of objects(or widgets), with
    * tips/information about them.
    *
    * @{
    */

   /**
    * Called back when a widget's tooltip is activated and needs content.
    * @param data user-data given to elm_object_tooltip_content_cb_set()
    * @param obj owner widget.
    * @param tooltip The tooltip object (affix content to this!)
    */
   typedef Evas_Object *(*Elm_Tooltip_Content_Cb) (void *data, Evas_Object *obj, Evas_Object *tooltip);
   
   /**
    * Called back when a widget's item tooltip is activated and needs content.
    * @param data user-data given to elm_object_tooltip_content_cb_set()
    * @param obj owner widget.
    * @param tooltip The tooltip object (affix content to this!)
    * @param item context dependent item. As an example, if tooltip was
    *        set on Elm_List_Item, then it is of this type.
    */
   typedef Evas_Object *(*Elm_Tooltip_Item_Content_Cb) (void *data, Evas_Object *obj, Evas_Object *tooltip, void *item);

   EAPI double       elm_tooltip_delay_get(void);
   EAPI Eina_Bool    elm_tooltip_delay_set(double delay);
   EAPI void         elm_object_tooltip_show(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void         elm_object_tooltip_hide(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void         elm_object_tooltip_text_set(Evas_Object *obj, const char *text) EINA_ARG_NONNULL(1, 2);
   EAPI void         elm_object_tooltip_domain_translatable_text_set(Evas_Object *obj, const char *domain, const char *text) EINA_ARG_NONNULL(1, 3);
#define elm_object_tooltip_translatable_text_set(obj, text) elm_object_tooltip_domain_translatable_text_set((obj), NULL, (text))
   EAPI void         elm_object_tooltip_content_cb_set(Evas_Object *obj, Elm_Tooltip_Content_Cb func, const void *data, Evas_Smart_Cb del_cb) EINA_ARG_NONNULL(1);
   EAPI void         elm_object_tooltip_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI void         elm_object_tooltip_style_set(Evas_Object *obj, const char *style) EINA_ARG_NONNULL(1);
   EAPI const char  *elm_object_tooltip_style_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool    elm_object_tooltip_window_mode_set(Evas_Object *obj, Eina_Bool disable) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool    elm_object_tooltip_window_mode_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @}
    */

