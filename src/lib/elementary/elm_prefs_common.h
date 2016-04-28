/**
 * @addtogroup Elm_Prefs
 *
 * @{
 */

/**
 * @brief Add a new prefs widget
 *
 * @param parent The parent widget to hold the new one
 * @return The new object or @c NULL, on errors
 *
 * @since 1.8
 */
EAPI Evas_Object       *elm_prefs_add(Evas_Object *parent);

/* API for prefs interface modules, from now on */

typedef struct _Elm_Prefs_Item_Spec_Void Elm_Prefs_Item_Spec_Void;
struct _Elm_Prefs_Item_Spec_Void
{
   char dummy;  //for compatibility (warning: empty struct has size 0 in C, size 1 in C++)
};

typedef struct _Elm_Prefs_Item_Spec_Bool Elm_Prefs_Item_Spec_Bool;
struct _Elm_Prefs_Item_Spec_Bool
{
   Eina_Bool def;
};

typedef struct _Elm_Prefs_Item_Spec_Int Elm_Prefs_Item_Spec_Int;
struct _Elm_Prefs_Item_Spec_Int
{
   int def, min, max;
};

typedef struct _Elm_Prefs_Item_Spec_Float Elm_Prefs_Item_Spec_Float;
struct _Elm_Prefs_Item_Spec_Float
{
   float def, min, max;
};

typedef struct _Elm_Prefs_Item_Spec_String Elm_Prefs_Item_Spec_String;
struct _Elm_Prefs_Item_Spec_String
{
   const char *def;
   const char *placeholder;
   const char *accept;
   const char *deny;

   struct
   {
      int min, max;
   } length;
};

typedef struct _Elm_Prefs_Item_Spec_Date Elm_Prefs_Item_Spec_Date;
struct _Elm_Prefs_Item_Spec_Date
{
   struct
   {
      unsigned char  d; /* day*/
      unsigned char  m; /* month*/
      unsigned short y; /* year */
   } def;

   struct
   {
      unsigned char  d; /* day*/
      unsigned char  m; /* month*/
      unsigned short y; /* year */
   } min;

   struct
   {
      unsigned char  d; /* day*/
      unsigned char  m; /* month*/
      unsigned short y; /* year */
   } max;
};

typedef struct _Elm_Prefs_Item_Spec_Page Elm_Prefs_Item_Spec_Page;
struct _Elm_Prefs_Item_Spec_Page
{
   const char *source;
};

typedef union _Elm_Prefs_Item_Spec Elm_Prefs_Item_Spec;
union _Elm_Prefs_Item_Spec
{
   Elm_Prefs_Item_Spec_Void   v;
   Elm_Prefs_Item_Spec_Bool   b;
   Elm_Prefs_Item_Spec_Int    i;
   Elm_Prefs_Item_Spec_Float  f;
   Elm_Prefs_Item_Spec_String s;
   Elm_Prefs_Item_Spec_Date   d;
   Elm_Prefs_Item_Spec_Page   p;
};

typedef struct _Elm_Prefs_Item_Iface Elm_Prefs_Item_Iface;

typedef void                            (*Elm_Prefs_Item_Changed_Cb)(Evas_Object *it_obj);   /**< Elementary prefs' item widget changed function signature */

/**
 * @struct _Elm_Prefs_Item_Iface
 *
 * @brief Interface between items of the @ref elm-prefs-widget widget
 *        and the real widgets implementing them.
 *
 * This structure defines the interface between the
 * @ref elm-prefs-widget widget's items (all item types in Elementary prefs
 * collections but the @c PAGE one) and Elementary widgets
 * implementing them. @c add() is the only mandatory function an
 * item widget has to implement.
 *
 * @note For items of type @c PAGE, refer to #Elm_Prefs_Page_Iface.
 *
 * @since 1.8
 */
struct _Elm_Prefs_Item_Iface
{
#define ELM_PREFS_ITEM_IFACE_ABI_VERSION (1)
   unsigned int               abi_version; /**< always use:
                                            *  - #ELM_PREFS_ITEM_IFACE_ABI_VERSION to declare.
                                            *  - elm_prefs_widget_iface_abi_version_get() to check.
                                            */

   const Elm_Prefs_Item_Type *types;  /**< types of prefs items supported by the widget, #ELM_PREFS_TYPE_UNKNOWN terminated */

   Evas_Object              * (*add)(const Elm_Prefs_Item_Iface * iface,
                                     Evas_Object * prefs,
                                     const Elm_Prefs_Item_Type type,
                                     const Elm_Prefs_Item_Spec spec,
                                     Elm_Prefs_Item_Changed_Cb it_changed_cb); /**< Function to instantiate the item widget. It must return the widget handle, which should be the @c obj argument on the functions which follow. That object argument, by the way, should always have their respective #Elm_Prefs_Item_Node handle accessible via a @c "prefs_item" Evas object data value. The return value of the following functions should be @c EINA_TRUE, on success or @c EINA_FALSE, otherwise. */

   Eina_Bool                  (*value_set)(Evas_Object *obj,
                                           Eina_Value *value); /**< Function to set the value on the item widget. Note that for items of type #ELM_PREFS_TYPE_SEPARATOR, this function has a special meaning of making the separator widget a @b horizontal one */

   Eina_Bool                  (*value_get)(Evas_Object *obj,
                                           Eina_Value *value); /**< Function to set the value on the item widget. Note that for items of type #ELM_PREFS_TYPE_SEPARATOR, this function has a special meaning of making the separator widget a @b vertical one */

   Eina_Bool                  (*value_validate)(Evas_Object *obj); /** < Function to validate the value from the item widget before saving it. The return value of the following function should be @c EINA_TRUE, if the value conforms with the expected or @c EINA_FALSE, otherwise. */

   Eina_Bool                  (*label_set)(Evas_Object *obj,
                                           const char *label); /**< function to set a label on the item widget */

   Eina_Bool                  (*icon_set)(Evas_Object *obj,
                                          const char *icon); /**< function to set an icon on the item widget */

   Eina_Bool                  (*editable_set)(Evas_Object *obj,
                                              Eina_Bool val); /**< function to set an item widget as editable or not */

   Eina_Bool                  (*editable_get)(Evas_Object *obj); /**< function to retrieve whether an item widget is editable or not */

   Eina_Bool                  (*expand_want)(Evas_Object *obj); /**< function to get whether the item implementation needs to be expanded in the page's longitudinal axis or not */
};

typedef struct _Elm_Prefs_Item_Iface_Info Elm_Prefs_Item_Iface_Info;
/**
 * Convenience struct used to mass-register widgets implementing
 * prefs @b items interfaces.
 *
 * To be used with elm_prefs_item_iface_register() and
 * elm_prefs_item_iface_unregister().
 */
struct _Elm_Prefs_Item_Iface_Info
{
   const char                 *widget_name; /**< The name of the widget implementing the interface, to be exposed on the prefs collections language. */
   const Elm_Prefs_Item_Iface *info; /**< The type interface's implementation. */
};

/**
 * Mass-register widgets implementing prefs @b items interfaces.
 *
 * @param array An array of #Elm_Prefs_Iface_Info structs, @c NULL
 * terminated.
 *
 * This will register all item interfaces declared on @a array in
 * Elementary, so that the prefs widget will recognize them on @c .epc
 * files @c 'widget:' (item) declarations.
 *
 * @see elm_prefs_item_iface_unregister()
 *
 * @since 1.8
 */
EAPI void      elm_prefs_item_iface_register(const Elm_Prefs_Item_Iface_Info *array);

/**
 * Mass-unregister widgets implementing prefs @b items interfaces.
 *
 * @param array An array of #Elm_Prefs_Iface_Info structs, @c NULL
 * terminated.
 *
 * This will unregister all item interfaces declared on @a array in
 * Elementary, given they had been previously registered.
 *
 * @see elm_prefs_item_iface_register() for more details
 *
 * @since 1.8
 */
EAPI void      elm_prefs_item_iface_unregister(const Elm_Prefs_Item_Iface_Info *array);

EAPI Eina_Bool elm_prefs_item_widget_common_add(Evas_Object *prefs,
                                                Evas_Object *obj);

typedef struct _Elm_Prefs_Page_Iface Elm_Prefs_Page_Iface;
/**
 * @struct _Elm_Prefs_Page_Iface
 *
 * @brief Interface between pages of the @ref elm-prefs-widget widget
 *        and the real widgets implementing them.
 *
 * This structure defines the interface between the
 * @ref elm-prefs-widget widget's pages and Elementary widgets
 * implementing them.
 *
 * It is @b mandatory that the following functions be implemented, at
 * least, for a page widget:
 *
 * - #Elm_Prefs_Page_Iface::add
 * - #Elm_Prefs_Page_Iface::item_pack
 * - #Elm_Prefs_Page_Iface::item_unpack
 * - #Elm_Prefs_Page_Iface::item_pack_before
 * - #Elm_Prefs_Page_Iface::item_pack_after
 *
 * @note For regular, non-page prefs items, refer to #Elm_Prefs_Item_Iface.
 *
 * @since 1.8
 */
struct _Elm_Prefs_Page_Iface
{
#define ELM_PREFS_PAGE_IFACE_ABI_VERSION (1)
   unsigned int  abi_version; /**< always use:
                               *  - #ELM_PREFS_PAGE_IFACE_ABI_VERSION to declare.
                               *  - elm_prefs_widget_iface_abi_version_get() to check.
                               */

   Evas_Object * (*add)(const Elm_Prefs_Page_Iface * iface,
                        Evas_Object * prefs); /**< Function to instantiate the page widget. It must return the widget handle, which should be the @c obj argument on the functions which follow. That object argument, by the way, should always have their respective #Elm_Prefs_Page_Node handle accessible via a @c "prefs_page" Evas object data value. The return value of the following functions should be @c EINA_TRUE, on success or @c EINA_FALSE, otherwise. */

   Eina_Bool     (*title_set)(Evas_Object *obj,
                              const char *title); /**< function to set a title on the page widget */

   Eina_Bool     (*sub_title_set)(Evas_Object *obj,
                                  const char *sub_title); /**< function to set a sub-title on the page widget */

   Eina_Bool     (*icon_set)(Evas_Object *obj,
                             const char *icon); /**< function to set an icon on the page widget */

   Eina_Bool     (*item_pack)(Evas_Object *obj,
                              Evas_Object *it,
                              const Elm_Prefs_Item_Type type,
                              const Elm_Prefs_Item_Iface *iface); /**< function to pack an item (widget) on the page widget */


   Eina_Bool     (*item_unpack)(Evas_Object *obj,
                                Evas_Object *it); /**< function to unpack an item (widget) on the page widget */

   Eina_Bool     (*item_pack_before)(Evas_Object *obj,
                                     Evas_Object *it,
                                     Evas_Object *it_before,
                                     const Elm_Prefs_Item_Type type,
                                     const Elm_Prefs_Item_Iface *iface); /**< function to pack an item (widget) on the page widget, before a pre-existing, referential, packed one */

   Eina_Bool     (*item_pack_after)(Evas_Object *obj,
                                    Evas_Object *it,
                                    Evas_Object *it_after, /**< function to pack an item (widget) on the page widget, after a pre-existing, referential, packed one */
                                    const Elm_Prefs_Item_Type type,
                                    const Elm_Prefs_Item_Iface *iface);
};

typedef struct _Elm_Prefs_Page_Iface_Info Elm_Prefs_Page_Iface_Info;
/**
 * Convenience struct used to mass-register widgets implementing
 * prefs @b pages interfaces.
 *
 * To be used with elm_prefs_page_iface_register() and
 * elm_prefs_page_iface_unregister().
 */
struct _Elm_Prefs_Page_Iface_Info
{
   const char                 *widget_name; /**< The name of the widget implementing the interface, to be exposed on the prefs collections language. */
   const Elm_Prefs_Page_Iface *info; /**< The interface's implementation. */
};

/**
 * Mass-register widgets implementing prefs @b pages interfaces.
 *
 * @param array An array of #Elm_Prefs_Iface_Info structs, @c NULL
 * terminated.
 *
 * This will register all page interfaces declared on @a array in
 * Elementary, so that the prefs widget will recognize them on @c .epc
 * files @c 'widget:' (page) declarations.
 *
 * @see elm_prefs_page_iface_unregister()
 *
 * @since 1.8
 */
EAPI void      elm_prefs_page_iface_register(const Elm_Prefs_Page_Iface_Info *array);

/**
 * Mass-unregister widgets implementing prefs @b pages interfaces.
 *
 * @param array An array of #Elm_Prefs_Iface_Info structs, @c NULL
 * terminated.
 *
 * This will unregister all page interfaces declared on @a array in
 * Elementary, given they had been previously registered.
 *
 * @see elm_prefs_page_iface_register() for more details
 *
 * @since 1.8
 */
EAPI void      elm_prefs_page_iface_unregister(const Elm_Prefs_Page_Iface_Info *array);

/**
 * @}
 */
