   typedef struct _Elm_Store                      Elm_Store;
   typedef struct _Elm_Store_Filesystem           Elm_Store_Filesystem;
   typedef struct _Elm_Store_Item                 Elm_Store_Item;
   typedef struct _Elm_Store_Item_Filesystem      Elm_Store_Item_Filesystem;
   typedef struct _Elm_Store_Item_Info            Elm_Store_Item_Info;
   typedef struct _Elm_Store_Item_Info_Filesystem Elm_Store_Item_Info_Filesystem;
   typedef struct _Elm_Store_Item_Mapping         Elm_Store_Item_Mapping;
   typedef struct _Elm_Store_Item_Mapping_Empty   Elm_Store_Item_Mapping_Empty;
   typedef struct _Elm_Store_Item_Mapping_Icon    Elm_Store_Item_Mapping_Icon;
   typedef struct _Elm_Store_Item_Mapping_Photo   Elm_Store_Item_Mapping_Photo;
   typedef struct _Elm_Store_Item_Mapping_Custom  Elm_Store_Item_Mapping_Custom;

   typedef Eina_Bool (*Elm_Store_Item_List_Cb) (void *data, Elm_Store_Item_Info *info);
   typedef void      (*Elm_Store_Item_Fetch_Cb) (void *data, Elm_Store_Item *sti);
   typedef void      (*Elm_Store_Item_Unfetch_Cb) (void *data, Elm_Store_Item *sti);
   typedef void     *(*Elm_Store_Item_Mapping_Cb) (void *data, Elm_Store_Item *sti, const char *part);

   typedef enum
     {
        ELM_STORE_ITEM_MAPPING_NONE = 0,
        ELM_STORE_ITEM_MAPPING_LABEL, // const char * -> label
        ELM_STORE_ITEM_MAPPING_STATE, // Eina_Bool -> state
        ELM_STORE_ITEM_MAPPING_ICON, // char * -> icon path
        ELM_STORE_ITEM_MAPPING_PHOTO, // char * -> photo path
        ELM_STORE_ITEM_MAPPING_CUSTOM, // item->custom(it->data, it, part) -> void * (-> any)
        // can add more here as needed by common apps
        ELM_STORE_ITEM_MAPPING_LAST
     } Elm_Store_Item_Mapping_Type;

   struct _Elm_Store_Item_Mapping_Icon
     {
        // FIXME: allow edje file icons
        int                   w, h;
        Elm_Icon_Lookup_Order lookup_order;
        Eina_Bool             standard_name : 1;
        Eina_Bool             no_scale : 1;
        Eina_Bool             smooth : 1;
        Eina_Bool             scale_up : 1;
        Eina_Bool             scale_down : 1;
     };

   struct _Elm_Store_Item_Mapping_Empty
     {
        Eina_Bool             dummy;
     };

   struct _Elm_Store_Item_Mapping_Photo
     {
        int                   size;
     };

   struct _Elm_Store_Item_Mapping_Custom
     {
        Elm_Store_Item_Mapping_Cb func;
     };

   struct _Elm_Store_Item_Mapping
     {
        Elm_Store_Item_Mapping_Type     type;
        const char                     *part;
        int                             offset;
        union
          {
             Elm_Store_Item_Mapping_Empty  empty;
             Elm_Store_Item_Mapping_Icon   icon;
             Elm_Store_Item_Mapping_Photo  photo;
             Elm_Store_Item_Mapping_Custom custom;
             // add more types here
          } details;
     };

   struct _Elm_Store_Item_Info
     {
        Elm_Genlist_Item_Class       *item_class;
        const Elm_Store_Item_Mapping *mapping;
        void                         *data;
        char                         *sort_id;
     };

   struct _Elm_Store_Item_Info_Filesystem
     {
        Elm_Store_Item_Info  base;
        char                *path;
     };

#define ELM_STORE_ITEM_MAPPING_END { ELM_STORE_ITEM_MAPPING_NONE, NULL, 0, { .empty = { EINA_TRUE } } }
#define ELM_STORE_ITEM_MAPPING_OFFSET(st, it) offsetof(st, it)

   EAPI void                    elm_store_free(Elm_Store *st);

   EAPI Elm_Store              *elm_store_filesystem_new(void);
   EAPI void                    elm_store_filesystem_directory_set(Elm_Store *st, const char *dir) EINA_ARG_NONNULL(1);
   EAPI const char             *elm_store_filesystem_directory_get(const Elm_Store *st) EINA_ARG_NONNULL(1);
   EAPI const char             *elm_store_item_filesystem_path_get(const Elm_Store_Item *sti) EINA_ARG_NONNULL(1);

   EAPI void                    elm_store_target_genlist_set(Elm_Store *st, Evas_Object *obj) EINA_ARG_NONNULL(1);

   EAPI void                    elm_store_cache_set(Elm_Store *st, int max) EINA_ARG_NONNULL(1);
   EAPI int                     elm_store_cache_get(const Elm_Store *st) EINA_ARG_NONNULL(1);
   EAPI void                    elm_store_list_func_set(Elm_Store *st, Elm_Store_Item_List_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                    elm_store_fetch_func_set(Elm_Store *st, Elm_Store_Item_Fetch_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                    elm_store_fetch_thread_set(Elm_Store *st, Eina_Bool use_thread) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool               elm_store_fetch_thread_get(const Elm_Store *st) EINA_ARG_NONNULL(1);

   EAPI void                    elm_store_unfetch_func_set(Elm_Store *st, Elm_Store_Item_Unfetch_Cb func, const void *data) EINA_ARG_NONNULL(1, 2);
   EAPI void                    elm_store_sorted_set(Elm_Store *st, Eina_Bool sorted) EINA_ARG_NONNULL(1);
   EAPI Eina_Bool               elm_store_sorted_get(const Elm_Store *st) EINA_ARG_NONNULL(1);
   EAPI void                    elm_store_item_data_set(Elm_Store_Item *sti, void *data) EINA_ARG_NONNULL(1);
   EAPI void                   *elm_store_item_data_get(Elm_Store_Item *sti) EINA_ARG_NONNULL(1);
   EAPI const Elm_Store        *elm_store_item_store_get(const Elm_Store_Item *sti) EINA_ARG_NONNULL(1);
   EAPI const Elm_Genlist_Item *elm_store_item_genlist_item_get(const Elm_Store_Item *sti) EINA_ARG_NONNULL(1);

