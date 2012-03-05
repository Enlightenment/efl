/* TEMPORARY: DOCS WILL BE FILLED IN WITH CNP/SED */
// XXX: elm_gen -> kill.
typedef struct Elm_Gen_Item             Elm_Gen_Item;
typedef struct _Elm_Gen_Item_Class      Elm_Gen_Item_Class;
typedef struct _Elm_Gen_Item_Class_Func Elm_Gen_Item_Class_Func;    /**< Class functions for gen item classes. */
typedef char                         *(*Elm_Gen_Item_Text_Get_Cb)(void *data, Evas_Object *obj, const char *part); /**< Label fetching class function for gen item classes. */
typedef Evas_Object                  *(*Elm_Gen_Item_Content_Get_Cb)(void *data, Evas_Object *obj, const char *part); /**< Content(swallowed object) fetching class function for gen item classes. */
typedef Eina_Bool                     (*Elm_Gen_Item_State_Get_Cb)(void *data, Evas_Object *obj, const char *part); /**< State fetching class function for gen item classes. */
typedef void                          (*Elm_Gen_Item_Del_Cb)(void *data, Evas_Object *obj); /**< Deletion class function for gen item classes. */
struct _Elm_Gen_Item_Class
{
   int version;
   unsigned int refcount;
   Eina_Bool delete_me : 1;
   const char *item_style;
   const char *mode_item_style;
   const char *edit_item_style;
   struct _Elm_Gen_Item_Class_Func
   {
      Elm_Gen_Item_Text_Get_Cb    text_get;
      Elm_Gen_Item_Content_Get_Cb content_get;
      Elm_Gen_Item_State_Get_Cb   state_get;
      Elm_Gen_Item_Del_Cb         del;
   } func;
};
