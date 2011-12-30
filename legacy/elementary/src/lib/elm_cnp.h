   /**
    * @addtogroup CopyPaste
    * @{
    */

   typedef struct _Elm_Selection_Data Elm_Selection_Data;
   typedef Eina_Bool (*Elm_Drop_Cb) (void *d, Evas_Object *o, Elm_Selection_Data *data);

   typedef enum _Elm_Sel_Type
   {
      ELM_SEL_TYPE_PRIMARY,
      ELM_SEL_TYPE_SECONDARY,
      ELM_SEL_TYPE_CLIPBOARD,
      ELM_SEL_TYPE_XDND,

      ELM_SEL_TYPE_MAX,
   } Elm_Sel_Type;

   typedef enum _Elm_Sel_Format
   {
      /** Targets: for matching every atom requesting */
      ELM_SEL_FORMAT_TARGETS  = -1,
      /** they come from outside of elm */
      ELM_SEL_FORMAT_NONE     = 0x0,
      /** Plain unformated text: Used for things that don't want rich markup */
      ELM_SEL_FORMAT_TEXT     = 0x01,
      /** Edje textblock markup, including inline images */
      ELM_SEL_FORMAT_MARKUP   = 0x02,
      /** Images */
      ELM_SEL_FORMAT_IMAGE    = 0x04,
      /** Vcards */
      ELM_SEL_FORMAT_VCARD    = 0x08,
      /** Raw HTMLish things for widgets that want that stuff (hello webkit!) */
      ELM_SEL_FORMAT_HTML     = 0x10,

      ELM_SEL_FORMAT_MAX
   } Elm_Sel_Format;

   struct _Elm_Selection_Data
   {
      int                   x, y;
      Elm_Sel_Format   format;
      void                 *data;
      size_t                len;
   };

   /**
    * @brief Set a data of a widget to copy and paste.
    *
    * Append the given callback to the list. This functions will be called
    * called.
    *
    * @param selection selection type for copying and pasting
    * @param widget The source widget pointer
    * @param format Type of selection format
    * @param buf The pointer of data source
    * @return If EINA_TRUE, setting data is success.
    *
    * @ingroup CopyPaste
    *
    */

   EAPI Eina_Bool            elm_cnp_selection_set(Elm_Sel_Type selection, Evas_Object *widget, Elm_Sel_Format format, const void *buf, size_t buflen);

   /**
    * @brief Retrive the data from the widget which is set for copying and pasting.
    *
    * Getting the data from the widget which is set for copying and pasting.
    * Mainly the widget is elm_entry. If then @p datacb and @p udata are
    * can be NULL. If not, @p datacb and @p udata are used for retriving data.
    *
    * @see also elm_cnp_selection_set()
    *
    * @param selection selection type for copying and pasting
    * @param widget The source widget pointer
    * @param datacb The user data callback if the target widget isn't elm_entry
    * @param udata The user data pointer for @p datacb
    * @return If EINA_TRUE, getting data is success.
    *
    * @ingroup CopyPaste
    *
    */

   EAPI Eina_Bool            elm_cnp_selection_get(Elm_Sel_Type selection, Elm_Sel_Format format, Evas_Object *widget, Elm_Drop_Cb datacb, void *udata);

   /**
    * @brief Clear the data in the widget which is set for copying and pasting.
    *
    * Clear the data in the widget. Normally this function isn't need to call.
    *
    * @see also elm_cnp_selection_set()
    *
    * @param selection selection type for copying and pasting
    * @param widget The source widget pointer
    * @return If EINA_TRUE, clearing data is success.
    *
    * @ingroup CopyPaste
    *
    */

   EAPI Eina_Bool            elm_cnp_selection_clear(Elm_Sel_Type selection, Evas_Object *widget);

   /**
    * @}
    */

