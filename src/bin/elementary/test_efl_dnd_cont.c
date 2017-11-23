#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif

#define EO_BETA_API

#include <Elementary.h>

#define ERR(fmt, args...) printf("%s %d: " fmt "\n", __func__, __LINE__, ##args)

#define FILESEP "file://"
#define FILESEP_LEN sizeof(FILESEP) - 1
#define DRAG_TIMEOUT 0.3
#define ANIM_TIME 0.5

static const char *img[9] =
{
   "panel_01.jpg",
   "plant_01.jpg",
   "rock_01.jpg",
   "rock_02.jpg",
   "sky_01.jpg",
   "sky_02.jpg",
   "sky_03.jpg",
   "sky_04.jpg",
   "wood_01.jpg",
};

static Elm_Genlist_Item_Class *itc1;
static Elm_Gengrid_Item_Class *gic;
static Eina_Bool _5s_cancel = EINA_FALSE;

static int
_item_ptr_cmp(const void *d1, const void *d2)
{
   return ((const char *) d1 - (const char *) d2);
}

static char *
_drag_data_extract(char **drag_data)
{
   char *uri = NULL;
   if (!drag_data)
     return uri;

   char *p = *drag_data;
   if (!p)
     return uri;
   char *s = strstr(p, FILESEP);
   if (s)
     p += FILESEP_LEN;
   s = strchr(p, '\n');
   uri = p;
   if (s)
     {
        if (s - p > 0)
          {
             char *s1 = s - 1;
             if (s1[0] == '\r')
               s1[0] = '\0';
             else
               {
                  char *s2 = s + 1;
                  if (s2[0] == '\r')
                    {
                       s[0] = '\0';
                       s++;
                    }
                  else
                    s[0] = '\0';
               }
          }
        else
          s[0] = '\0';
        s++;
     }
   else
     p = NULL;
   *drag_data = s;

   return uri;
}
static const char *
_drag_data_build(Eina_List **items)
{
   const char *drag_data = NULL;
   if (*items)
     {
        Eina_Strbuf *str;
        Eina_List *l;
        Elm_Object_Item *it;
        const char *t;
        int i = 0;

        str = eina_strbuf_new();
        if (!str) return NULL;

        /* drag data in form: file://URI1\nfile://URI2 */
        EINA_LIST_FOREACH(*items, l, it)
          {
             t = (char *)elm_object_item_data_get(it);
             if (t)
               {
                  if (i > 0)
                    eina_strbuf_append(str, "\n");
                  eina_strbuf_append(str, FILESEP);
                  eina_strbuf_append(str, t);
                  i++;
               }
          }
        drag_data = eina_strbuf_string_steal(str);
        eina_strbuf_free(str);
     }
   return drag_data;
}

static Eina_List *
//_grid_icons_get(void *data)
_grid_icon_list_get_cb(void *data, Efl_Object *obj)
{  /* Start icons animation before actually drag-starts */
   printf("<%s> <%d>\n", __func__, __LINE__);

   Efl_Object *grid = data;
   Eina_List *l;
   Eina_List *icons = NULL;
   Evas_Coord xm, ym;

   evas_pointer_canvas_xy_get(evas_object_evas_get(grid), &xm, &ym);
   Eina_List *items = eina_list_clone(elm_gengrid_selected_items_get(grid));
   Elm_Object_Item *gli = elm_gengrid_at_xy_item_get(grid,
                                                     xm, ym, NULL, NULL);
   if (gli)
     {  /* Add the item mouse is over to the list if NOT seleced */
        void *p = eina_list_search_unsorted(items, _item_ptr_cmp, gli);
        if (!p)
          items = eina_list_append(items, gli);
     }

   EINA_LIST_FOREACH(items, l, gli)
     {  /* Now add icons to animation window */
        Evas_Object *o = elm_object_item_part_content_get(gli,
              "elm.swallow.icon");
        ERR("one item: %p", gli);

        if (o)
          {
             ERR("item has icon part");
             int x, y, w, h;
             const char *f, *g;
             elm_image_file_get(o, &f, &g);
             Evas_Object *ic = elm_icon_add(grid);
             elm_image_file_set(ic, f, g);
             evas_object_geometry_get(o, &x, &y, &w, &h);
             evas_object_size_hint_align_set(ic,
                   EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(ic,
                   EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

             evas_object_move(ic, x, y);
             evas_object_resize(ic, w, h);
             evas_object_show(ic);

             icons =  eina_list_append(icons, ic);
          }
        else
             ERR("item does not have icon part");
     }

   eina_list_free(items);
   return icons;
}

//static Elm_Object_Item *
static Efl_Object *
_grid_item_get_cb(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *xposret, int *yposret)
{  /* This function returns pointer to item under (x,y) coords */
   //printf("<%s> <%d> obj=<%p>\n", __func__, __LINE__, obj);
   Elm_Object_Item *item = NULL;
   item = elm_gengrid_at_xy_item_get(obj, x, y, xposret, yposret);
   if (item)
     {
     printf("over <%s>, item=<%p> xposret %i yposret %i\n",
           (char *)elm_object_item_data_get(item), item, *xposret, *yposret);
     }
   else
     {
     printf("over none, xposret %i yposret %i\n", *xposret, *yposret);
     }
   return item;
}

static inline char *
_strndup(const char *str, size_t len)
{
   size_t slen = strlen(str);
   char *ret;

   if (slen > len) slen = len;
   ret = malloc (slen + 1);
   if (!ret) return NULL;

   if (slen > 0) memcpy(ret, str, slen);
   ret[slen] = '\0';
   return ret;
}

static Evas_Object *
//_gl_createicon(void *data, Evas_Object *win, Evas_Coord *xoff, Evas_Coord *yoff)
_grid_icon_get_cb(void *data, Eo *win, int *xoff, int *yoff)
{
   printf("<%s> <%d>\n", __func__, __LINE__);
   Evas_Object *grid = data;
   Elm_Object_Item *gi;
   Evas_Coord xm = 0, ym = 0;

   evas_pointer_canvas_xy_get(evas_object_evas_get(grid), &xm, &ym);
   ERR("x, y: %d %d", xm, ym);
   gi = elm_gengrid_at_xy_item_get(grid, xm, ym, NULL, NULL);
   if (!gi)
     ERR("cannot get item");
   Evas_Object *icon = NULL;
   Evas_Object *o = elm_object_item_part_content_get(gi, "elm.swallow.icon");

   ERR("In");
   if (o)
     {
        int xm, ym, w = 30, h = 30;
        const char *f;
        const char *g;
        elm_image_file_get(o, &f, &g);
        printf("%s %d: image file: %s %s\n", __func__, __LINE__, f, g);
        evas_pointer_canvas_xy_get(evas_object_evas_get(o), &xm, &ym);
        if (xoff) *xoff = xm - (w/2);
        if (yoff) *yoff = ym - (h/2);
        icon = elm_icon_add(win);
        elm_image_file_set(icon, f, g);
        evas_object_size_hint_align_set(icon,
              EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(icon,
              EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        if (xoff && yoff) evas_object_move(icon, *xoff, *yoff);
        evas_object_resize(icon, w, h);
     }
   else
     {
        ERR("icon part does not exist for grid item");
     }

   return icon;
}

//static const char *
static void
//_grid_get_drag_data(Evas_Object *obj, Elm_Object_Item *it, Eina_List **items)
_grid_data_get_cb(void *data, Evas_Object *obj, Efl_Selection_Format *format, void **buf, int *len, Efl_Selection_Action *action)
{  /* Construct a string of dragged info, user frees returned string */
   const char *drag_data = NULL;
   printf("<%s> <%d>\n", __func__, __LINE__);
   int xm, ym;

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &xm, &ym);
   Eina_List *items = eina_list_clone(elm_gengrid_selected_items_get(obj));
   Elm_Object_Item *gli = elm_gengrid_at_xy_item_get(obj, xm, ym, NULL, NULL);
   if (gli)
     {  /* Add the item mouse is over to the list if NOT seleced */
        void *p = eina_list_search_unsorted(items, _item_ptr_cmp, gli);
        if (!p)
          items = eina_list_append(items, gli);
     }


   drag_data = _drag_data_build(&items);
   printf("<%s> <%d> Sending <%s>\n", __func__, __LINE__, drag_data);

   int data_len = 0;
   if (drag_data)
     data_len = strlen(drag_data);
   if (format)
     {
        *format = EFL_SELECTION_FORMAT_TEXT;
     }
   if (buf && drag_data)
     {
        *buf = calloc(1, data_len + 1);
        strncpy(*buf, drag_data, data_len);
     }
   if (len)
     {
        *len = data_len;
     }
   if (action)
     {
        *action = EFL_SELECTION_ACTION_COPY;
     }
}

static char *
gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup(data);
}

static Evas_Object *
gl_content_get(void *data, Evas_Object *obj, const char *part)
{
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *icon = elm_icon_add(obj);
        elm_image_file_set(icon, data, NULL);
        evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        evas_object_show(icon);
        return icon;
     }
   return NULL;
}

static void
_win_del(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("<%s> <%d> will del <%p>\n", __func__, __LINE__, data);
   efl_dnd_drag_item_container_del(data);
   efl_dnd_drop_item_container_del(data);

   if (gic) elm_gengrid_item_class_free(gic);
   gic = NULL;
   if (itc1) elm_genlist_item_class_free(itc1);
   itc1 = NULL;
}


static Eina_Bool _drop_box_button_new_cb(void *data, Evas_Object *obj, Elm_Selection_Data *ev)
{
   Evas_Object *win = data;
   if (ev->len <= 0)
     return EINA_FALSE;

   char *dd = _strndup(ev->data, ev->len);
   if (!dd) return EINA_FALSE;
   char *p = dd;
   char *s = _drag_data_extract(&p);
   while (s)
     {
        Evas_Object *ic = elm_icon_add(win);
        elm_image_file_set(ic, s, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        Evas_Object *bt = elm_button_add(win);
        elm_object_text_set(bt, "Dropped button");
        elm_object_part_content_set(bt, "icon", ic);
        elm_box_pack_end(obj, bt);
        evas_object_show(bt);
        evas_object_show(ic);
        s = _drag_data_extract(&p);
     }
   free(dd);

   return EINA_TRUE;
}

void _enter_but_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{
   printf("Entered %s - drop it here and I will never print this line anymore.\n", __FUNCTION__);
}

static Eina_Bool _drop_but_icon_change_cb(void *data, Evas_Object *obj, Elm_Selection_Data *ev)
{
   Evas_Object *win = data;
   Evas_Object *ic;
   if (ev->len <= 0)
     return EINA_FALSE;

   char *dd = _strndup(ev->data, ev->len);
   if (!dd) return EINA_FALSE;
   char *p = dd;
   char *s = _drag_data_extract(&p);
   ic = elm_icon_add(win);
   elm_image_file_set(ic, s, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_del(elm_object_part_content_get(obj, "icon"));
   elm_object_part_content_set(obj, "icon", ic);
   evas_object_show(ic);
   free(dd);

   return EINA_TRUE;
}

/* Callback used to test multi-callbacks feature */
static Eina_Bool _drop_but_cb_remove_cb(void *data EINA_UNUSED, Evas_Object *obj, Elm_Selection_Data *ev EINA_UNUSED)
{
   printf("Second callback called - removing it\n");
   //elm_drop_target_del(obj, ELM_SEL_FORMAT_TARGETS, _enter_but_cb, NULL, NULL, NULL, NULL, NULL, _drop_but_cb_remove_cb, NULL);
   return EINA_TRUE;
}

static Eina_Bool _drop_bg_change_cb(void *data EINA_UNUSED, Evas_Object *obj, Elm_Selection_Data *ev)
{
   if (ev->len <= 0)
     return EINA_FALSE;

   char *dd = _strndup(ev->data, ev->len);
   if (!dd) return EINA_FALSE;
   char *p = dd;
   char *s = _drag_data_extract(&p);
   elm_bg_file_set(obj, s, NULL);
   free(dd);

   return EINA_TRUE;
}

static void
_5s_cancel_ck_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   _5s_cancel = elm_check_state_get(obj);
}

static void
_box_drag_enter_cb(void *data, const Efl_Event *ev)
{
   ERR("box enter");
}

static void
_box_drag_leave_cb(void *data, const Efl_Event *ev)
{
   ERR("box leave");
}

static void
_box_drag_drop_cb(void *data, const Efl_Event *ev)
{
   ERR("box drop");
   Efl_Object *grid = data;
   Efl_Selection_Data *sd = ev->info;

   ERR("x,y: %d %d, data: %s, len: %d, format: %d, action: %d, item: %p", sd->x, sd->y, sd->data, sd->len, sd->format, sd->action, sd->item);
}

#if 0
{
   if (sd->len < 0) return;
   char *dd = _strndup(sd->data, sd->len);
   if (!dd) return EINA_FALSE;
   char *s = _drag_data_extract(&dd);
   while(s)
     {
        if (sd->item)
          elm_gengrid_item_insert_after(grid, gic, eina_stringshare_add(s), sd->item, NULL, NULL);
        else
          elm_gengrid_item_append(grid, gic, eina_stringshare_add(s), NULL, NULL);
        s = _drag_data_extract(&dd);
     }
   free(dd);
}
#endif


static void
_bg_enter_cb(void *data, const Efl_Event *ev)
{
   ERR("bg enter");
}

static void
_bg_pos_cb(void *data, const Efl_Event *ev)
{
   ERR("bg pos");
}

static void
_bg_leave_cb(void *data, const Efl_Event *ev)
{
   ERR("bg leave");
}

static void
_bg_drop_cb(void *data, const Efl_Event *ev)
{
   ERR("bg drop");
}

static void
_grid_enter_cb(void *data, const Efl_Event *ev)
{
   ERR("grid enter");
}

static void
_grid_leave_cb(void *data, const Efl_Event *ev)
{
   ERR("grid leave");
}

static void
_grid_pos_cb(void *data, const Efl_Event *ev)
{
   ERR("grid pos");
}

static void
_grid_drop_cb(void *data, const Efl_Event *ev)
{
   Efl_Object *grid = data;
   Efl_Selection_Data *sd = ev->info;

   ERR("grid drop: x,y: %d %d, data: %s, len: %d, format: %d, action: %d, item: %p", sd->x, sd->y, sd->data, sd->len, sd->format, sd->action, sd->item);
   if (sd->len < 0) return;
   char *dd = _strndup(sd->data, sd->len);
   if (!dd) return EINA_FALSE;
   char *s = _drag_data_extract(&dd);
   while (s)
     {
        if (sd->item)
          elm_gengrid_item_insert_after(grid, gic, eina_stringshare_add(s), sd->item, NULL, NULL);
        else
          elm_gengrid_item_append(grid, gic, eina_stringshare_add(s), NULL, NULL);
        s = _drag_data_extract(&dd);
     }
   free(dd);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   char buf[PATH_MAX];
   Evas_Object *win, *bxx, *bg;
   int i;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   win = elm_win_util_standard_add("dnd-multi-features", "DnD-Multi Features");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   efl_event_callback_add(bg, EFL_DND_EVENT_DRAG_ENTER, _bg_enter_cb, bg);
   efl_event_callback_add(bg, EFL_DND_EVENT_DRAG_POS, _bg_pos_cb, bg);
   efl_event_callback_add(bg, EFL_DND_EVENT_DRAG_LEAVE, _bg_leave_cb, bg);
   efl_event_callback_add(bg, EFL_DND_EVENT_DRAG_DROP, _bg_drop_cb, bg);
   efl_dnd_drop_target_add(bg, EFL_SELECTION_FORMAT_TEXT);
   //elm_drop_target_add(bg, ELM_SEL_FORMAT_TARGETS, NULL, NULL, NULL, NULL, NULL, NULL, _drop_bg_change_cb, NULL);
   elm_win_resize_object_add(win, bg);

   /* And show the background. */
   evas_object_show(bg);
   bxx = elm_box_add(win);
   elm_box_horizontal_set(bxx, EINA_TRUE);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   //grid
   Evas_Object *grid = elm_gengrid_add(bxx);
   evas_object_smart_callback_add(win, "delete,request", _win_del, grid);
   elm_gengrid_item_size_set(grid,
                             ELM_SCALE_SIZE(100), ELM_SCALE_SIZE(100));
   elm_gengrid_horizontal_set(grid, EINA_FALSE);
   elm_gengrid_reorder_mode_set(grid, EINA_FALSE);
   elm_gengrid_multi_select_set(grid, EINA_TRUE); /* We allow multi drag */
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = gl_text_get;
   gic->func.content_get = gl_content_get;

   //elm_drag_item_container_add(grid, ANIM_TIME, DRAG_TIMEOUT,
   //                            _grid_item_getcb, _grid_data_getcb);
   efl_dnd_drag_item_container_add(grid, ANIM_TIME, DRAG_TIMEOUT,
                                   grid, _grid_data_get_cb, NULL,
                                   grid, _grid_item_get_cb, NULL,
                                   grid, _grid_icon_get_cb, NULL,
                                   grid, _grid_icon_list_get_cb, NULL);

   efl_event_callback_add(grid, EFL_DND_EVENT_DRAG_ENTER, _grid_enter_cb, grid);
   efl_event_callback_add(grid, EFL_DND_EVENT_DRAG_LEAVE, _grid_leave_cb, grid);
   efl_event_callback_add(grid, EFL_DND_EVENT_DRAG_POS, _grid_pos_cb, grid);
   efl_event_callback_add(grid, EFL_DND_EVENT_DRAG_DROP, _grid_drop_cb, grid);
   efl_dnd_drop_item_container_add(grid, EFL_SELECTION_FORMAT_TEXT,
                                   grid, _grid_item_get_cb, NULL);

   for (i = 0; i < 10; i++)
     {
        //snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[(i % 9)]);
        snprintf(buf, sizeof(buf), "%s/images/%s", "~/upgit/update/efl/data/elementary", img[(i % 9)]);
        printf("image dir: %s\n", buf);
        const char *path = eina_stringshare_add(buf);
        elm_gengrid_item_append(grid, gic, path, NULL, NULL);
     }
   elm_box_pack_end(bxx, grid);
   evas_object_show(grid);

   //genlist
   Evas_Object *ic, *bt;
   Evas_Object *vert_box = elm_box_add(bxx);
   evas_object_size_hint_weight_set(vert_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bxx, vert_box);
   evas_object_show(vert_box);
   efl_event_callback_add(vert_box, EFL_DND_EVENT_DRAG_ENTER, _box_drag_enter_cb, vert_box);
   efl_event_callback_add(vert_box, EFL_DND_EVENT_DRAG_LEAVE, _box_drag_leave_cb, vert_box);
   efl_event_callback_add(vert_box, EFL_DND_EVENT_DRAG_DROP, _box_drag_drop_cb, vert_box);
   efl_dnd_drop_target_add(vert_box, EFL_SELECTION_FORMAT_TEXT);
   //elm_drop_target_add(vert_box, ELM_SEL_FORMAT_TARGETS, NULL, NULL, NULL, NULL, NULL, NULL, _drop_box_button_new_cb, win);

   _5s_cancel = EINA_FALSE;
   Evas_Object *ck = elm_check_add(vert_box);
   elm_object_style_set(ck, "toggle");
   elm_object_text_set(ck, "Cancel after 5s:");
   elm_check_state_set(ck, _5s_cancel);
   evas_object_smart_callback_add(ck, "changed", _5s_cancel_ck_changed, NULL);
   elm_box_pack_end(vert_box, ck);
   evas_object_show(ck);

   ic = elm_icon_add(win);
   //snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", "~/upgit/update/efl/data/elementary");
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Multi-callbacks check");
   //elm_drop_target_add(bt, ELM_SEL_FORMAT_TARGETS, NULL, NULL, NULL, NULL, NULL, NULL, _drop_but_icon_change_cb, win);
   //elm_drop_target_add(bt, ELM_SEL_FORMAT_TARGETS, _enter_but_cb, NULL, NULL, NULL, NULL, NULL, _drop_but_cb_remove_cb, NULL);
   elm_object_part_content_set(bt, "icon", ic);
   elm_box_pack_end(vert_box, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   //snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Drop into me to change my icon");
   //elm_drop_target_add(bt, ELM_SEL_FORMAT_TARGETS, NULL, NULL, NULL, NULL, NULL, NULL, _drop_but_icon_change_cb, win);
   elm_object_part_content_set(bt, "icon", ic);
   elm_box_pack_end(vert_box, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   //snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "No action on drop");
   elm_object_part_content_set(bt, "icon", ic);
   elm_box_pack_end(vert_box, bt);
   evas_object_show(bt);
   evas_object_show(ic);


   evas_object_resize(win, 680, 800);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
