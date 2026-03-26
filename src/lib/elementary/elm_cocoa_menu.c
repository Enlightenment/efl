/* elm_cocoa_menu.c
 * Elementary bridge: elm_menu items <-> ecore_cocoa NSMenu.
 * Pure C — no ObjC.
 */
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_ELEMENTARY_COCOA

#include <Elementary.h>
#include <stdint.h>
#include "elm_priv.h"
#include "elm_widget_menu.h"

struct _Elm_Cocoa_Menu
{
   Eo               *menu;       /* the elm_menu Eo object */
   Ecore_Cocoa_Menu *root;       /* top-level NSMenu handle */
   Eina_Hash        *elements;   /* int32 id -> Elm_Menu_Item_Data* */
   Eina_Hash        *submenus;   /* int32 id -> Ecore_Cocoa_Menu*   */
   unsigned          next_id;    /* monotonic counter, initial value 0, first id is 1 */
};

static void
_submenu_hash_free_cb(void *data)
{
   ecore_cocoa_menu_free((Ecore_Cocoa_Menu *)data);
}

/* Forward declared in elm_menu.c Task 3.1 */
void _elm_menu_item_activate(Elm_Object_Item *eo_item);

/* ------------------------------------------------------------------ */
/* Click callback (fires from NSMenuItem action via ecore_cocoa layer) */
/* ------------------------------------------------------------------ */
static void
_item_click_cb(void *data, Ecore_Cocoa_Menu *menu EINA_UNUSED, int idx EINA_UNUSED)
{
   Elm_Object_Item *eo_item = data;
   _elm_menu_item_activate(eo_item);
}

/* ------------------------------------------------------------------ */
/* Internal: recursively add item and its submenu children             */
/* ------------------------------------------------------------------ */
static int
_item_add_recursive(Elm_Cocoa_Menu *cm, Ecore_Cocoa_Menu *ns_parent,
                    Elm_Menu_Item_Data *item)
{
   int id;
   Eina_List *l;
   Elm_Object_Item *eo_sub;

   id = (int)++cm->next_id;

   if (item->separator)
     {
        ecore_cocoa_menu_item_add_separator(ns_parent);
        /* Separators are not tracked in the hash — no cocoa_idx assignment needed */
     }
   else if (item->submenu.items)
     {
        /* Top-level or submenu-bearing item: add placeholder, then recurse */
        int ns_idx;
        Ecore_Cocoa_Menu *sub_ns;

        sub_ns = ecore_cocoa_menu_new(item->label ? item->label : "");
        if (!sub_ns) return -1;

        ns_idx = ecore_cocoa_menu_item_add(ns_parent,
                                           item->label ? item->label : "",
                                           NULL, NULL, NULL);
        if (ns_idx < 0) { ecore_cocoa_menu_free(sub_ns); return -1; }

        /* Assign the logical ID as NSMenuItem tag for stable lookup */
        ecore_cocoa_menu_item_tag_set(ns_parent, ns_idx, id);
        ecore_cocoa_menu_submenu_set(ns_parent, ns_idx, sub_ns);

        if (!eina_hash_add(cm->elements, &id, item))
          {
             ecore_cocoa_menu_item_del(ns_parent, ns_idx);
             ecore_cocoa_menu_free(sub_ns);
             return -1;
          }
        if (!eina_hash_add(cm->submenus, &id, sub_ns))
          {
             eina_hash_del_by_key(cm->elements, &id);
             ecore_cocoa_menu_item_del(ns_parent, ns_idx);
             ecore_cocoa_menu_free(sub_ns);
             return -1;
          }
        item->cocoa_idx  = id;
        item->cocoa_menu = cm;

        EINA_LIST_FOREACH(item->submenu.items, l, eo_sub)
          {
             ELM_MENU_ITEM_DATA_GET(eo_sub, subitem);
             if (_item_add_recursive(cm, sub_ns, subitem) < 0)
               {
                  /* Parent registered OK; child failed. Log and continue
                   * with partial menu rather than tearing down the parent. */
                  WRN("_item_add_recursive: child add failed, partial menu");
                  break;
               }
          }
     }
   else
     {
        /* Leaf item */
        int ns_idx;

        ns_idx = ecore_cocoa_menu_item_add(ns_parent,
                                           item->label ? item->label : "",
                                           NULL,
                                           _item_click_cb,
                                           EO_OBJ(item));
        if (ns_idx < 0) return -1;

        /* Assign the logical ID as NSMenuItem tag for stable lookup */
        ecore_cocoa_menu_item_tag_set(ns_parent, ns_idx, id);

        if (!eina_hash_add(cm->elements, &id, item)) return -1;
        item->cocoa_idx  = id;
        item->cocoa_menu = cm;
     }

   return id;
}

/* ------------------------------------------------------------------ */
/* Public internal API                                                  */
/* ------------------------------------------------------------------ */

Elm_Cocoa_Menu *
_elm_cocoa_menu_register(Eo *obj)
{
   Elm_Cocoa_Menu *cm;
   Eina_List *l;
   Elm_Object_Item *eo_item;

   ELM_MENU_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   if (sd->cocoa_menu) return sd->cocoa_menu; /* already registered */

   cm = calloc(1, sizeof(*cm));
   if (EINA_UNLIKELY(!cm)) return NULL;

   cm->menu = obj;
   cm->next_id = 0;

   cm->elements = eina_hash_int32_new(NULL);
   if (!cm->elements) { free(cm); return NULL; }

   cm->submenus = eina_hash_int32_new(_submenu_hash_free_cb);
   if (!cm->submenus) { eina_hash_free(cm->elements); free(cm); return NULL; }

   cm->root = ecore_cocoa_menu_new("MainMenu");
   if (!cm->root)
     {
        eina_hash_free(cm->submenus);
        eina_hash_free(cm->elements);
        free(cm);
        return NULL;
     }

   /* Set sd->cocoa_menu before the walk so that any re-entrant calls
    * from EO callbacks during the walk will find the bridge in place. */
   sd->cocoa_menu = cm;

   /* Walk existing items (app may add items before or after register) */
   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        int ret;

        ELM_MENU_ITEM_DATA_GET(eo_item, item);
        ret = _item_add_recursive(cm, cm->root, item);
        if (ret < 0)
          WRN("Failed to add menu item '%s' to Cocoa menu — skipping",
              item->label ? item->label : "(null)");
     }

   ecore_cocoa_menu_main_set(cm->root);

   return cm;
}

void
_elm_cocoa_menu_unregister(Eo *obj)
{
   ELM_MENU_DATA_GET_OR_RETURN(obj, sd);

   if (!sd->cocoa_menu) return;

   /* submenus hash has a free callback that calls ecore_cocoa_menu_free()
    * on each value — free it before the root to avoid double-free since
    * Cocoa also releases child NSMenus when the parent is released. */
   eina_hash_free(sd->cocoa_menu->submenus);
   ecore_cocoa_menu_free(sd->cocoa_menu->root);
   eina_hash_free(sd->cocoa_menu->elements);
   free(sd->cocoa_menu);
   sd->cocoa_menu = NULL;
}

/* Promote a leaf NSMenuItem into a submenu-bearing item.
 * Called when a child is added to an item that was originally created
 * as a leaf (no children at creation time). */
static Ecore_Cocoa_Menu *
_promote_to_submenu(Elm_Cocoa_Menu *cm, Elm_Menu_Item_Data *parent_item)
{
   Ecore_Cocoa_Menu *parent_ns;
   Ecore_Cocoa_Menu *sub_ns;
   int               ns_idx;
   int               parent_tag;

   parent_tag = parent_item->cocoa_idx;

   /* Find which NSMenu contains the parent item */
   if (parent_item->parent && parent_item->parent->cocoa_idx > 0)
     {
        parent_ns = eina_hash_find(cm->submenus, &parent_item->parent->cocoa_idx);
        if (!parent_ns) parent_ns = cm->root;
     }
   else
     parent_ns = cm->root;

   /* Create a new NSMenu for the children */
   sub_ns = ecore_cocoa_menu_new(parent_item->label ? parent_item->label : "");
   if (!sub_ns) return NULL;

   /* Attach the submenu to the existing NSMenuItem identified by tag */
   ns_idx = ecore_cocoa_menu_item_index_by_tag(parent_ns, parent_tag);
   if (ns_idx < 0)
     {
        ecore_cocoa_menu_free(sub_ns);
        return NULL;
     }
   ecore_cocoa_menu_submenu_set(parent_ns, ns_idx, sub_ns);

   /* Track the submenu in our hash */
   if (!eina_hash_add(cm->submenus, &parent_tag, sub_ns))
     {
        /* sub_ns is attached to the native menu via setSubmenu — Cocoa retains
         * it, so ecore_cocoa_menu_free just drops our reference. */
        ecore_cocoa_menu_free(sub_ns);
        return NULL;
     }

   return sub_ns;
}

int
_elm_cocoa_menu_item_add(Elm_Cocoa_Menu *cm, Elm_Object_Item *eo_item)
{
   Ecore_Cocoa_Menu *ns_parent;
   int id;

   if (!cm || !eo_item) return -1;

   ELM_MENU_ITEM_DATA_GET(eo_item, item);

   /* Determine which NSMenu is the parent */
   if (item->parent && item->parent->cocoa_idx > 0)
     {
        ns_parent = eina_hash_find(cm->submenus, &item->parent->cocoa_idx);
        if (!ns_parent)
          {
             /* Parent was created as a leaf — promote it to a submenu */
             ns_parent = _promote_to_submenu(cm, item->parent);
             if (!ns_parent) ns_parent = cm->root;
          }
     }
   else
     ns_parent = cm->root;

   id = _item_add_recursive(cm, ns_parent, item);
   return id;
}

void
_elm_cocoa_menu_item_delete(Elm_Cocoa_Menu *cm, int id)
{
   Elm_Menu_Item_Data *item;
   Ecore_Cocoa_Menu   *ns_menu;
   Ecore_Cocoa_Menu   *sub;

   if (!cm || id <= 0) return;

   item = eina_hash_find(cm->elements, &id);
   if (!item) return;

   /* Determine which NSMenu contains this item */
   if (item->parent && item->parent->cocoa_idx > 0)
     {
        ns_menu = eina_hash_find(cm->submenus, &item->parent->cocoa_idx);
        if (!ns_menu) ns_menu = cm->root;
     }
   else
     ns_menu = cm->root;

   /* Use tag-based deletion — cocoa_idx is a logical ID stored as NSMenuItem.tag,
    * not a positional index (which drifts after deletions). */
   ecore_cocoa_menu_item_del_by_tag(ns_menu, item->cocoa_idx);
   eina_hash_del_by_key(cm->elements, &id);

   /* If it had a submenu, remove from hash (free callback handles ecore_cocoa_menu_free) */
   sub = eina_hash_find(cm->submenus, &id);
   if (sub)
     eina_hash_del_by_key(cm->submenus, &id);
}

void
_elm_cocoa_menu_update(Elm_Cocoa_Menu *cm)
{
   Eina_Iterator *it;
   Elm_Menu_Item_Data *item;

   if (!cm) return;

   /* Sync enabled state for all tracked items */
   it = eina_hash_iterator_data_new(cm->elements);
   EINA_ITERATOR_FOREACH(it, item)
     {
        Ecore_Cocoa_Menu *ns_menu = cm->root;
        if (item->parent && item->parent->cocoa_idx > 0)
          {
             Ecore_Cocoa_Menu *sub = eina_hash_find(cm->submenus,
                                                    &item->parent->cocoa_idx);
             if (sub) ns_menu = sub;
          }
        /* Use tag-based enabled_set — cocoa_idx is a logical tag, not a position */
        ecore_cocoa_menu_item_enabled_set_by_tag(ns_menu, item->cocoa_idx,
                                                 !elm_object_item_disabled_get(EO_OBJ(item)));
     }
   eina_iterator_free(it);
}

#endif /* HAVE_ELEMENTARY_COCOA */
