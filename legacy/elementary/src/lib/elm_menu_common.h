EAPI Evas_Object                 *elm_menu_item_object_get(const Elm_Object_Item *it);

EAPI void                         elm_menu_item_icon_name_set(Elm_Object_Item *it, const char *icon);

EAPI const char                  *elm_menu_item_icon_name_get(const Elm_Object_Item *it);

EAPI void                         elm_menu_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

EAPI Eina_Bool                    elm_menu_item_selected_get(const Elm_Object_Item *it);

EAPI Eina_Bool                    elm_menu_item_is_separator(Elm_Object_Item *it);

EAPI const Eina_List             *elm_menu_item_subitems_get(const Elm_Object_Item *it);

EAPI void                         elm_menu_item_subitems_clear(Elm_Object_Item *it);

EAPI unsigned int                 elm_menu_item_index_get(const Elm_Object_Item *it);

EAPI Elm_Object_Item             *elm_menu_item_next_get(const Elm_Object_Item *it);

EAPI Elm_Object_Item             *elm_menu_item_prev_get(const Elm_Object_Item *it);

