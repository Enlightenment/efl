#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#ifdef HAVE_MMAN_H
# include <sys/mman.h>
#endif

#include "efl_selection_manager_private.h"

#define MY_CLASS EFL_SELECTION_MANAGER_CLASS

//#define DEBUGON 1
#ifdef DEBUGON
# define sel_debug(fmt, args...) fprintf(stderr, __FILE__":%s:%d : " fmt "\n", __FUNCTION__, __LINE__, ##args)
#else
# define sel_debug(x...) do { } while (0)
#endif

static void _anim_data_free(Sel_Manager_Drag_Container *dc);
static void _cont_obj_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _cont_obj_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _item_container_del_internal(Sel_Manager_Drag_Container *dc, Eina_Bool full);

void efl_selection_manager_drop_target_del(Eo *obj, Efl_Object *target_obj, Efl_Selection_Format format, unsigned int seat);
void efl_selection_manager_selection_clear(Eo *obj, Efl_Object *owner, Efl_Selection_Type type, unsigned int seat);
void efl_selection_manager_drag_start(Eo *obj, Efl_Object *drag_obj, Efl_Selection_Format format, Eina_Slice data, Efl_Selection_Action action, void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb icon_func_free_cb, unsigned int seat);

static Eina_List *managers;

#ifdef HAVE_ELEMENTARY_X
static void _set_selection_list(Sel_Manager_Selection *sel_list, Sel_Manager_Seat_Selection *seat_sel);
static Ecore_X_Atom _x11_dnd_action_rev_map(Efl_Selection_Action action);
static Ecore_X_Window _x11_xwin_get(const Evas_Object *obj);
#endif

#ifdef HAVE_ELEMENTARY_WL2
static Ecore_Wl2_Window *_wl_window_get(const Evas_Object *obj);
static Ecore_Wl2_Input *_wl_seat_get(Ecore_Wl2_Window *win, Evas_Object *obj, unsigned int seat_id);
#endif

#ifdef HAVE_ELEMENTARY_WIN32
static void _set_selection_list(Sel_Manager_Selection *sel_list, Sel_Manager_Seat_Selection *seat_sel);
#endif

static Sel_Manager_Seat_Selection *
_sel_manager_seat_selection_get(Efl_Selection_Manager_Data *pd, unsigned int seat)
{
   Eina_List *l = NULL;
   Sel_Manager_Seat_Selection *seat_sel = NULL;

   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if (seat_sel->seat == seat)
          break;
     }
   if (!seat_sel)
     ERR("Could not find request seat");

   return seat_sel;
}

static inline void
_owner_change_check(Efl_Selection_Manager *manager, Efl_Object *owner,
                    Sel_Manager_Seat_Selection *seat_sel,
                    Sel_Manager_Selection *sel,
                    Efl_Selection_Type type, Eina_Bool same_win)
{
   if (!same_win)
     {
        Eina_List *l, *l_next;
        Eo *man;

        EINA_LIST_FOREACH_SAFE(managers, l, l_next, man)
          {
             if (man != manager)
               {
                  Eina_List *l2, *l2_next, *l3, *l3_next;
                  Sel_Manager_Selection_Lost *sel_lost;
                  Sel_Manager_Seat_Selection *seat_sel2;
                  Efl_Selection_Manager_Data *pd = efl_data_scope_get(man, MY_CLASS);

                  if (!pd) continue;
                  EINA_LIST_FOREACH_SAFE(pd->seat_list, l3, l3_next, seat_sel2)
                    {
                       EINA_LIST_FOREACH_SAFE(seat_sel2->sel_lost_list, l2, l2_next, sel_lost)
                         {
                            if ((sel_lost->request) &&
                                (sel_lost->type == type))
                              {
                                 eina_promise_resolve(sel_lost->promise, eina_value_uint_init(sel_lost->type));
                                 seat_sel2->sel_lost_list = eina_list_remove(seat_sel2->sel_lost_list, sel_lost);
                                 free(sel_lost);
                              }
                         }
                       seat_sel2->xwin = 0;
#if defined(HAVE_ELEMENTARY_X) || defined(HAVE_ELEMENTARY_WIN32)
                       if (seat_sel2->sel_list)
                         {
                            int i;

                            for (i = 0;
                                 i < (EFL_SELECTION_TYPE_CLIPBOARD + 1)
                                 ; i++)
                              {
#ifdef HAVE_ELEMENTARY_X
                                 seat_sel2->sel_list[i].xwin = 0;
#elif defined (HAVE_ELEMENTARY_WIN32)
                                 seat_sel2->sel_list[i].win = NULL;
#endif
                                 seat_sel2->sel_list[i].active = EINA_FALSE;
                              }
                         }
#endif
#if defined(HAVE_ELEMENTARY_WL2) || defined(HAVE_ELEMENTARY_COCOA)
                       if (seat_sel2->sel)
                         {
                            seat_sel2->sel->win = 0;
                            seat_sel2->sel->active = EINA_FALSE;
                         }
#endif
                    }
               }
          }
     }
   if ((sel->owner != NULL) &&
       (sel->owner != owner) && same_win)
     {
        Eina_List *l, *l_next;
        Sel_Manager_Selection_Lost *sel_lost;
        EINA_LIST_FOREACH_SAFE(seat_sel->sel_lost_list, l, l_next, sel_lost)
          {
             if ((sel_lost->request == sel->owner) &&
                 (sel_lost->type == type))
               {
                  eina_promise_resolve(sel_lost->promise, eina_value_uint_init(sel_lost->type));
                  seat_sel->sel_lost_list = eina_list_remove(seat_sel->sel_lost_list, sel_lost);
                  free(sel_lost);
               }
          }
     }
}

static Sel_Manager_Seat_Selection *
_sel_manager_seat_selection_init(Efl_Selection_Manager_Data *pd, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if(seat_sel->seat == seat)
          break;
     }
   if (!seat_sel)
     {
        seat_sel = calloc(1, sizeof(Sel_Manager_Seat_Selection));
        if (!seat_sel)
          {
             ERR("Failed to allocate seat");
             return NULL;
          }
        seat_sel->saved_types = calloc(1, sizeof(Saved_Type));
        seat_sel->seat = seat;
        seat_sel->pd = pd;
        pd->seat_list = eina_list_append(pd->seat_list, seat_sel);
     }
#ifdef HAVE_ELEMENTARY_X
   if (!seat_sel->sel_list)
     {
        seat_sel->sel_list = calloc(1, (EFL_SELECTION_TYPE_CLIPBOARD + 1) * sizeof(Sel_Manager_Selection));
        if (!seat_sel->sel_list)
          {
             ERR("failed to allocate selection list");
             return NULL;
          }
        _set_selection_list(seat_sel->sel_list, seat_sel);
     }
#endif
#ifdef HAVE_ELEMENTARY_WL2
   if (!seat_sel->sel)
     {
        Sel_Manager_Selection *sel = calloc(1, sizeof(Sel_Manager_Selection));
        if (!sel)
          {
             ERR("failed to allocate selection");
             return NULL;
          }
        sel->seat_sel = seat_sel;
        seat_sel->sel = sel;
     }
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   if (!seat_sel->sel)
     {
        Sel_Manager_Selection *sel = calloc(1, sizeof(Sel_Manager_Selection));
        if (!sel)
          {
             ERR("failed to allocate selection");
             return NULL;
          }
        sel->seat_sel = seat_sel;
        seat_sel->sel = sel;
     }
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   if (!seat_sel->sel_list)
     {
        seat_sel->sel_list = calloc(1, (EFL_SELECTION_TYPE_CLIPBOARD + 1) * sizeof(Sel_Manager_Selection));
        if (!seat_sel->sel_list)
          {
             ERR("failed to allocate selection list");
             return NULL;
          }
        _set_selection_list(seat_sel->sel_list, seat_sel);
     }
#endif

   return seat_sel;
}

static void
_sel_manager_promise_cancel(void *data, const Eina_Promise *dead_future EINA_UNUSED)
{
   Sel_Manager_Selection_Lost *sel_lost = data;
   //FIXME: remove from sel_lost_list in seat_sel
   free(sel_lost);
}

static inline Eina_Future *
_update_sel_lost_list(Efl_Object *obj, Efl_Selection_Type type,
                      Sel_Manager_Seat_Selection *seat_sel)
{
   Eina_Promise *p;
   Sel_Manager_Selection_Lost *sel_lost;

   sel_lost = calloc(1, sizeof(Sel_Manager_Selection_Lost));
   if (!sel_lost)
     return NULL;
   sel_lost->request = obj;
   sel_lost->type = type;
   seat_sel->sel_lost_list = eina_list_append(seat_sel->sel_lost_list, sel_lost);

   p = eina_promise_new(efl_loop_future_scheduler_get(obj),
                        _sel_manager_promise_cancel, NULL);
   eina_promise_data_set(p, sel_lost);
   if (!p) return NULL;
   sel_lost->promise = p;

   return eina_future_new(p);
}

/* TODO: this should not be an actual tempfile, but rather encode the object
 * as http://dataurl.net/ if it's an image or similar. Evas should support
 * decoding it as memfile. */
static Tmp_Info *
_tempfile_new(int size)
{
#ifdef HAVE_MMAN_H
   Tmp_Info *info;
   const char *tmppath = NULL;
   mode_t cur_umask;
   int len;

   info = calloc(1, sizeof(Tmp_Info));
   if (!info) return NULL;
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     tmppath = getenv("TMP");
   if (!tmppath) tmppath = P_tmpdir;
   len = snprintf(NULL, 0, "%s/%sXXXXXX", tmppath, "elmcnpitem-");
   if (len < 0) goto on_error;
   len++;
   info->filename = malloc(len);
   if (!info->filename) goto on_error;
   snprintf(info->filename,len,"%s/%sXXXXXX", tmppath, "elmcnpitem-");
   cur_umask = umask(S_IRWXO | S_IRWXG);
   info->fd = mkstemp(info->filename);
   umask(cur_umask);
   if (info->fd < 0) goto on_error;
# ifdef __linux__
     {
        char *tmp;
        /* And before someone says anything see POSIX 1003.1-2008 page 400 */
        long pid;

        pid = (long)getpid();
        /* Use pid instead of /proc/self: That way if can be passed around */
        len = snprintf(NULL,0,"/proc/%li/fd/%i", pid, info->fd);
        len++;
        tmp = malloc(len);
        if (tmp)
          {
             snprintf(tmp,len, "/proc/%li/fd/%i", pid, info->fd);
             unlink(info->filename);
             free(info->filename);
             info->filename = tmp;
          }
     }
# endif
   sel_debug("filename is %s\n", info->filename);
   if (size < 1) goto on_error;
   /* Map it in */
   if (ftruncate(info->fd, size))
     {
        perror("ftruncate");
        goto on_error;
     }
   eina_mmap_safety_enabled_set(EINA_TRUE);
   info->map = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, info->fd, 0);
   if (info->map == MAP_FAILED)
     {
        perror("mmap");
        goto on_error;
     }
   return info;

 on_error:
   if (info->fd >= 0) close(info->fd);
   info->fd = -1;
   /* Set map to NULL and return */
   info->map = NULL;
   info->len = 0;
   free(info->filename);
   free(info);
   return NULL;
#else
   (void) size;
   return NULL;
#endif
}

static int
_tmpinfo_free(Tmp_Info *info)
{
   if (!info) return 0;
   free(info->filename);
   free(info);
   return 0;
}

static inline void
_drop_target_cbs_del(Efl_Selection_Manager_Data *pd, Sel_Manager_Dropable *dropable, Efl_Object *obj)
{
   if (dropable)
     {
        Drop_Format *df;
        while (dropable->format_list)
          {
             df = EINA_INLIST_CONTAINER_GET(dropable->format_list, Drop_Format);
             efl_selection_manager_drop_target_del(pd->sel_man, obj,
                                                   df->format, dropable->seat);
             // If drop_target_del() happened to delete dropabale, then
             // re-fetch it each loop to make sure it didn't
             dropable = efl_key_data_get(obj, "__elm_dropable");
             if (!dropable) break;
          }
     }
}

static void
_all_drop_targets_cbs_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *info EINA_UNUSED)
{
   Efl_Selection_Manager_Data *pd = data;
   Sel_Manager_Dropable *dropable = NULL;

   if (!pd) return;
   dropable = efl_key_data_get(obj, "__elm_dropable");
   _drop_target_cbs_del(pd, dropable, obj);
}

static void
_dropable_coords_adjust(Sel_Manager_Dropable *dropable, Eina_Position2D *pos)
{
   Ecore_Evas *ee;
   Evas *evas = evas_object_evas_get(dropable->obj);
   int ex = 0, ey = 0, ew = 0, eh = 0;
   Evas_Object *win;

   ee = ecore_evas_ecore_evas_get(evas);
   ecore_evas_geometry_get(ee, &ex, &ey, &ew, &eh);
   pos->x = pos->x - ex;
   pos->y = pos->y - ey;

   /* For Wayland, frame coords have to be subtracted. */
   Evas_Coord fx, fy;
   evas_output_framespace_get(evas, &fx, &fy, NULL, NULL);
   if (fx || fy) sel_debug("evas frame fx %d fy %d\n", fx, fy);
   pos->x = pos->x - fx;
   pos->y = pos->y - fy;

   if (elm_widget_is(dropable->obj))
     {
        win = elm_widget_top_get(dropable->obj);
        if (win && efl_isa(win, EFL_UI_WIN_CLASS))
          {
             Evas_Coord x2, y2;
             int rot = elm_win_rotation_get(win);
             switch (rot)
               {
                case 90:
                   x2 = ew - pos->y;
                   y2 = pos->x;
                   break;
                case 180:
                   x2 = ew - pos->x;
                   y2 = eh - pos->y;
                   break;
                case 270:
                   x2 = pos->y;
                   y2 = eh - pos->x;
                   break;
                default:
                   x2 = pos->x;
                   y2 = pos->y;
                   break;
               }
             sel_debug("rotation %d, w %d, h %d - x:%d->%d, y:%d->%d\n",
                       rot, ew, eh, pos->x, x2, pos->y, y2);
             pos->x = x2;
             pos->y = y2;
          }
     }
}

static Eina_Bool
_drag_cancel_animate(void *data, double pos)
{  /* Animation to "move back" drag-window */
   Sel_Manager_Seat_Selection *seat_sel = data;
   sel_debug("in, pos: %f", pos);
   if (pos >= 0.99)
     {
#ifdef HAVE_ELEMENTARY_X
        Ecore_X_Window xdragwin = _x11_xwin_get(seat_sel->drag_win);
        ecore_x_window_ignore_set(xdragwin, 0);
#endif
        sel_debug("Delete drag_win");
        evas_object_del(seat_sel->drag_win);
        seat_sel->drag_win = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   else
     {
        int x, y;
        x = seat_sel->drag_win_end.x - (pos * (seat_sel->drag_win_end.x - seat_sel->drag_win_start.x));
        y = seat_sel->drag_win_end.y - (pos * (seat_sel->drag_win_end.y - seat_sel->drag_win_start.y));
        evas_object_move(seat_sel->drag_win, x, y);
     }

   return ECORE_CALLBACK_RENEW;
}

static Efl_Selection_Format
_dnd_types_to_format(Efl_Selection_Manager_Data *pd, const char **types, int ntypes)
{
   Efl_Selection_Format ret_type = 0;
   int i;
   for (i = 0; i < ntypes; i++)
     {
        Sel_Manager_Atom *atom = eina_hash_find(pd->type_hash, types[i]);
        if (atom) ret_type |= atom->format;
     }
   return ret_type;
}

static Eina_List *
_dropable_list_geom_find(Efl_Selection_Manager_Data *pd, Evas *evas, Evas_Coord px, Evas_Coord py)
{
   Eina_List *itr, *top_objects_list = NULL, *dropable_list = NULL;
   Evas_Object *top_obj;
   Sel_Manager_Dropable *dropable = NULL;

   if (!pd->drop_list) return NULL;

   /* We retrieve the (non-smart) objects pointed by (px, py) */
   top_objects_list = evas_tree_objects_at_xy_get(evas, NULL, px, py);
   /* We walk on this list from the last because if the list contains more than one
    * element, all but the last will repeat events. The last one can repeat events
    * or not. Anyway, this last one is the first that has to be taken into account
    * for the determination of the drop target.
    */
   EINA_LIST_REVERSE_FOREACH(top_objects_list, itr, top_obj)
     {
        Evas_Object *object = top_obj;
        /* We search for the dropable data into the object. If not found, we search into its parent.
         * For example, if a button is a drop target, the first object will be an (internal) image.
         * The drop target is attached to the button, i.e to image's parent. That's why we need to
         * walk on the parents until NULL.
         * If we find this dropable data, we found our drop target.
         */
        while (object)
          {
             dropable = efl_key_data_get(object, "__elm_dropable");
             if (dropable)
               {
                  Eina_Bool exist = EINA_FALSE;
                  Eina_List *l;
                  Sel_Manager_Dropable *d = NULL;
                  EINA_LIST_FOREACH(dropable_list, l, d)
                    {
                       if (d == dropable)
                         {
                            exist = EINA_TRUE;
                            break;
                         }
                    }
                  if (!exist)
                    dropable_list = eina_list_append(dropable_list, dropable);
                  object = evas_object_smart_parent_get(object);
                  if (dropable)
                    sel_debug("Drop target %p of type %s found\n",
                              dropable->obj, efl_class_name_get(efl_class_get(dropable->obj)));
               }
             else
                object = evas_object_smart_parent_get(object);
          }
     }
   eina_list_free(top_objects_list);
   return dropable_list;
}

#ifdef HAVE_ELEMENTARY_X
static Ecore_X_Window
_x11_xwin_get(const Efl_Object *obj)
{
   if (!obj) return 0;

   Ecore_X_Window xwin = 0;
   //get top
   const Evas_Object *top = obj;
   const Evas_Object *parent = obj;
   while(parent)
     {
        top = parent;
        parent = efl_parent_get(parent);
     }
   if (efl_isa(top, EFL_UI_WIN_CLASS))
     {
        xwin = elm_win_xwindow_get(top);
     }
   if (!xwin)
     {
        Ecore_Evas *ee;
        Evas *evas = evas_object_evas_get(obj);
        if (!evas) return 0;
        ee = ecore_evas_ecore_evas_get(evas);
        if (!ee) return 0;

        while(!xwin)
          {
             const char *engine_name = ecore_evas_engine_name_get(ee);
             if (!strcmp(engine_name, ELM_BUFFER))
               {
                  ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
                  if (!ee) return 0;
                  xwin = _elm_ee_xwin_get(ee);
               }
             else
               {
                  xwin = _elm_ee_xwin_get(ee);
                  if (!xwin) return 0;
               }
          }
     }

   return xwin;
}

static Eina_Bool
_x11_is_uri_type_data(Sel_Manager_Selection *sel EINA_UNUSED, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   char *p;

   data = notify->data;
   sel_debug("data->format is %d %p %p", data->format, notify, data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES) return EINA_TRUE;
   p = (char *)data->data;
   if (!p) return EINA_TRUE;
   sel_debug("Got %s", p);
   if (strncmp(p, "file:/", 6))
     {
        if (*p != '/') return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Sel_Manager_Seat_Selection *
_x11_sel_manager_seat_selection_init(Efl_Selection_Manager_Data *pd, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if(seat_sel->seat == seat)
          break;
     }
   if (!seat_sel)
     {
        seat_sel = calloc(1, sizeof(Sel_Manager_Seat_Selection));
        if (!seat_sel)
          {
             ERR("Failed to allocate seat");
             return NULL;
          }
        seat_sel->saved_types = calloc(1, sizeof(Saved_Type));
        seat_sel->seat = seat;
        seat_sel->pd = pd;
        pd->seat_list = eina_list_append(pd->seat_list, seat_sel);
     }
   if (!seat_sel->sel_list)
     {
        //TODO: reduce memory (may be just need one common sel_list)
        seat_sel->sel_list = calloc(1, (EFL_SELECTION_TYPE_CLIPBOARD + 1) * sizeof(Sel_Manager_Selection));
        if (!seat_sel->sel_list)
          {
             ERR("failed to allocate selection list");
             return NULL;
          }
        _set_selection_list(seat_sel->sel_list, seat_sel);
     }

   return seat_sel;
}

static Eina_Bool
_x11_data_preparer_text(Sel_Manager_Seat_Selection *seat_sel EINA_UNUSED,
                        Ecore_X_Event_Selection_Notify *notify,
                        Efl_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("text data preparer");
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = EFL_SELECTION_FORMAT_TEXT;
   ddata->content.mem = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->content.len = data->length;
   return EINA_TRUE;
}

static Eina_Bool
_x11_data_preparer_markup(Sel_Manager_Seat_Selection *seat_sel EINA_UNUSED,
                          Ecore_X_Event_Selection_Notify *notify,
                          Efl_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("markup data preparer");
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = EFL_SELECTION_FORMAT_MARKUP;
   ddata->content.mem = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->content.len = data->length;
   return EINA_TRUE;
}

/**
 * So someone is pasting an image into my entry or widget...
 */
static Eina_Bool
_x11_data_preparer_uri(Sel_Manager_Seat_Selection *seat_sel, Ecore_X_Event_Selection_Notify *notify,
      Efl_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("uri data preparer");
   Ecore_X_Selection_Data *data;
   Ecore_X_Selection_Data_Files *files;
   char *p, *stripstr = NULL;

   data = notify->data;
   sel_debug("data->format is %d %p %p\n", data->format, notify, data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES)
     {
        Efreet_Uri *uri;
        Eina_Strbuf *strbuf;
        int i;

        sel_debug("got a files list\n");
        files = notify->data;
        /*
        if (files->num_files > 1)
          {
             // Don't handle many items <- this makes mr bigglesworth sad :(
             sel_debug("more then one file: Bailing\n");
             return EINA_FALSE;
          }
        stripstr = p = strdup(files->files[0]);
         */

        strbuf = eina_strbuf_new();
        if (!strbuf)
          return EINA_FALSE;

        for (i = 0; i < files->num_files ; i++)
          {
             uri = efreet_uri_decode(files->files[i]);
             if (uri)
               {
                  eina_strbuf_append(strbuf, uri->path);
                  efreet_uri_free(uri);
               }
             else
               {
                  eina_strbuf_append(strbuf, files->files[i]);
               }
             if (i < (files->num_files - 1))
               eina_strbuf_append(strbuf, "\n");
          }
        stripstr = eina_strbuf_string_steal(strbuf);
        eina_strbuf_free(strbuf);
     }
   else
     {
        Efreet_Uri *uri;

        p = (char *)eina_memdup((unsigned char *)data->data, data->length, EINA_TRUE);
        if (!p) return EINA_FALSE;
        uri = efreet_uri_decode(p);
        if (!uri)
          {
             /* Is there any reason why we care of URI without scheme? */
             if (p[0] == '/') stripstr = p;
             else free(p);
          }
        else
          {
             free(p);
             stripstr = strdup(uri->path);
             efreet_uri_free(uri);
          }
     }

   if (!stripstr)
     {
        sel_debug("Couldn't find a file\n");
        return EINA_FALSE;
     }
   free(seat_sel->saved_types->imgfile);
   if (seat_sel->saved_types->textreq)
     {
        seat_sel->saved_types->textreq = 0;
        seat_sel->saved_types->imgfile = stripstr;
     }
   else
     {
        ddata->format = EFL_SELECTION_FORMAT_IMAGE;
        ddata->content.mem = stripstr;
        ddata->content.len = strlen(stripstr);
        seat_sel->saved_types->imgfile = NULL;
     }
   return EINA_TRUE;
}

/**
 * Just received an vcard, either through cut and paste, or dnd.
 */
static Eina_Bool
_x11_data_preparer_vcard(Sel_Manager_Seat_Selection *seat_sel EINA_UNUSED,
                         Ecore_X_Event_Selection_Notify *notify,
                         Efl_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("vcard receive\n");
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = EFL_SELECTION_FORMAT_VCARD;
   ddata->content.mem = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->content.len = data->length;
   return EINA_TRUE;
}

static Eina_Bool
_x11_data_preparer_image(Sel_Manager_Seat_Selection *seat_sel EINA_UNUSED,
                         Ecore_X_Event_Selection_Notify *notify,
                         Efl_Selection_Data *ddata, Tmp_Info **tmp_info)
{
   Ecore_X_Selection_Data *data = notify->data;
   sel_debug("got a image file!\n");
   sel_debug("Size if %d\n", data->length);

   ddata->format = EFL_SELECTION_FORMAT_IMAGE;
   data = notify->data;

   Tmp_Info *tmp = _tempfile_new(data->length);
   if (!tmp) return EINA_FALSE;
   memcpy(tmp->map, data->data, data->length);
   munmap(tmp->map, data->length);
   ddata->content.mem = strdup(tmp->filename);
   ddata->content.len = strlen(tmp->filename);
   *tmp_info = tmp;
   return EINA_TRUE;
}

static Eina_Bool
_x11_win_filter(Eo *manager EINA_UNUSED, Ecore_X_Window xwin)
{
   Eo *win;
   const Eina_List *l;

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        if (elm_win_window_id_get(win) == xwin) return EINA_FALSE;
     }
   return EINA_TRUE;
}

/*
 * Callback to handle a targets response on a selection request:
 * So pick the format we'd like; and then request it.
 */
static Eina_Bool
_x11_notify_handler_targets(Efl_Selection_Manager_Data *pd, Sel_Manager_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   sel_debug("notify handler targets");
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atom_list;
   int i, j;

   targets = notify->data;
   atom_list = (Ecore_X_Atom *)(targets->data.data);
   for (j = (SELECTION_ATOM_LISTING_ATOMS + 1); j < SELECTION_N_ATOMS; j++)
     {
        sel_debug("\t%s %d", pd->atom_list[j].name, pd->atom_list[j].x_atom);
        if (!(pd->atom_list[j].format & sel->request_format)) continue;
        for (i = 0; i < targets->data.length; i++)
          {
             if ((pd->atom_list[j].x_atom == atom_list[i]) && (pd->atom_list[j].x_data_preparer))
               {
                  if (j == SELECTION_ATOM_TEXT_URILIST)
                    {
                       if (!_x11_is_uri_type_data(sel, notify)) continue;
                    }
                  sel_debug("Atom %s matches", pd->atom_list[j].name);
                  goto done;
               }
          }
     }
   sel_debug("Couldn't find anything that matches");
   return ECORE_CALLBACK_PASS_ON;
done:
   sel_debug("Sending request for %s, xwin=%#llx",
             pd->atom_list[j].name, (unsigned long long)sel->xwin);
   sel->request(sel->xwin, pd->atom_list[j].name);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_x11_fixes_selection_notify(void *data, int t EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = data;
   Efl_Selection_Changed e;
   Ecore_X_Event_Fixes_Selection_Notify *ev = event;
   Sel_Manager_Seat_Selection *seat_sel;
   Efl_Selection_Type type;
   Sel_Manager_Selection *sel;

   if (_x11_win_filter(pd->sel_man, ev->win)) return ECORE_CALLBACK_PASS_ON;

   switch (ev->selection)
     {
      case ECORE_X_SELECTION_CLIPBOARD:
        type = EFL_SELECTION_TYPE_CLIPBOARD;
        break;
      case ECORE_X_SELECTION_PRIMARY:
        type = EFL_SELECTION_TYPE_PRIMARY;
        break;
      default: return ECORE_CALLBACK_RENEW;
     }
   seat_sel = _x11_sel_manager_seat_selection_init(pd, 1);
   if (!seat_sel) return ECORE_CALLBACK_RENEW;
   sel = seat_sel->sel_list + type;
   if (sel->active && (sel->xwin != ev->owner))
     efl_selection_manager_selection_clear(pd->sel_man, sel->owner, type, seat_sel->seat);
   e.type = type;
   e.seat = 1; /* under x11 this is always the default seat */
   e.exist = !!ev->owner;
   efl_event_callback_call(sel->owner, EFL_SELECTION_EVENT_SELECTION_CHANGED, &e);
   //ecore_event_add(ELM_CNP_EVENT_SELECTION_CHANGED, e, NULL, NULL);

   return ECORE_CALLBACK_RENEW;
}

/*
 * Response to a selection notify:
 *  - So we have asked for the selection list.
 *  - If it's the targets list, parse it, and fire of what we want,
 *    else it's the data we want.
 */
//NB: x11 does not have seat, use 1 as default
static Eina_Bool
_efl_sel_manager_x11_selection_notify(void *udata, int type EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = udata;
   Ecore_X_Event_Selection_Notify *ev = event;
   Sel_Manager_Selection *sel;
   Sel_Manager_Seat_Selection *seat_sel = NULL;
   int i;

   if (_x11_win_filter(pd->sel_man, ev->win)) return ECORE_CALLBACK_PASS_ON;

   seat_sel =  _sel_manager_seat_selection_get(pd, 1);
   if (!seat_sel)
     return EINA_FALSE;

   sel_debug("selection notify callback: %d", ev->selection);
   switch (ev->selection)
     {
      case ECORE_X_SELECTION_PRIMARY:
        sel = seat_sel->sel_list + EFL_SELECTION_TYPE_PRIMARY;
        break;
      case ECORE_X_SELECTION_SECONDARY:
        sel = seat_sel->sel_list + EFL_SELECTION_TYPE_SECONDARY;
        break;
      case ECORE_X_SELECTION_XDND:
        sel = seat_sel->sel_list + EFL_SELECTION_TYPE_DND;
        break;
      case ECORE_X_SELECTION_CLIPBOARD:
        sel = seat_sel->sel_list + EFL_SELECTION_TYPE_CLIPBOARD;
        break;
      default:
        return ECORE_CALLBACK_PASS_ON;
     }
   sel_debug("Target is %s", ev->target);

   if (ev->selection != ECORE_X_SELECTION_XDND &&
       (!strcmp(ev->target, "TARGETS") || !strcmp(ev->target, "ATOMS")))
     {
        _x11_notify_handler_targets(pd, sel, ev);
        return ECORE_CALLBACK_PASS_ON;
     }
   for (i = 0; i < SELECTION_N_ATOMS; i++)
     {
        if (!strcmp(ev->target, pd->atom_list[i].name))
          {
             if (pd->atom_list[i].x_data_preparer)
               {
                  Efl_Selection_Data ddata;
                  Tmp_Info *tmp_info = NULL;
                  Eina_Bool success;
                  sel_debug("Found something: %s", pd->atom_list[i].name);

                  success = pd->atom_list[i].x_data_preparer(seat_sel, ev, &ddata, &tmp_info);
                  sel_debug("ddata: %s (%zd)", (const char *)ddata.content.mem, ddata.content.len);
                  if ((pd->atom_list[i].format == EFL_SELECTION_FORMAT_IMAGE) &&
                      (seat_sel->saved_types->imgfile))
                    break;
                  if (ev->selection == ECORE_X_SELECTION_XDND)
                    {
                       if (success)
                         {
                            Sel_Manager_Dropable *dropable;
                            Eina_List *l;
                            sel_debug("drag & drop\n");
                            EINA_LIST_FOREACH(pd->drop_list, l, dropable)
                              {
                                 if (dropable->obj == sel->request_obj) break;
                                 dropable = NULL;
                              }
                            if (dropable)
                              {
                                 Drop_Format *df;
                                 Eina_Inlist *itr;

                                 ddata.action = sel->action;
                                 if (!dropable->is_container)
                                   {
                                      sel_debug("normal dnd, not container");
                                      ddata.pos = seat_sel->saved_types->pos;
                                   }
                                 else
                                   {
                                      sel_debug("Drop on container");
                                      Eina_Position2D pos, posret = {0, 0};
                                      evas_object_geometry_get(dropable->obj, &pos.x, &pos.y, NULL, NULL);
                                      //get item
                                      pos = EINA_POSITION2D(seat_sel->saved_types->pos.x + pos.x,
                                                            seat_sel->saved_types->pos.y + pos.y);
                                      Efl_Object *it = NULL;
                                      if (dropable->item_func)
                                        it = dropable->item_func(dropable->item_func_data,
                                                  dropable->obj, pos, &posret);
                                      ddata.pos = posret;
                                      ddata.item = it;
                                   }
                                 EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
                                   {
                                    if (df->format & dropable->last.format)
                                      {
                                         sel_debug("calling Drop event on: %p", dropable->obj);
                                         efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_DROP, &ddata);
                                      }
                                   }
                              }
                            else
                              {
                                 sel_debug("dnd: has NO dropable");
                              }
                         }
                       /* We have to finish DnD, no matter what */
                       ecore_x_dnd_send_finished();
                    }
                  else if (sel->data_func && success)
                    {
                       ddata.pos.x = ddata.pos.y = 0;
                       sel->data_func(sel->data_func_data, sel->request_obj, &ddata);
                    }
                  free((void *)ddata.content.mem);
                  if (tmp_info) _tmpinfo_free(tmp_info);
               }
             else sel_debug("Ignored: No handler!");
             break;
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_x11_selection_clear(void *data, int type EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = data;
   Ecore_X_Event_Selection_Clear *ev = event;
   Sel_Manager_Selection *sel;
   Sel_Manager_Seat_Selection *seat_sel = NULL;
   Eina_List *l, *l_next;
   Sel_Manager_Selection_Lost *sel_lost;
   unsigned int i;

   if (_x11_win_filter(pd->sel_man, ev->win)) return ECORE_CALLBACK_PASS_ON;

   seat_sel = _sel_manager_seat_selection_get(pd, 1);
   if (!seat_sel)
     return EINA_FALSE;

   for (i = EFL_SELECTION_TYPE_PRIMARY; i <= EFL_SELECTION_TYPE_CLIPBOARD; i++)
     {
        if (seat_sel->sel_list[i].ecore_sel == ev->selection) break;
     }
   sel_debug("selection %d clear", i);
   /* Not me... Don't care */
   if (i > EFL_SELECTION_TYPE_CLIPBOARD) return ECORE_CALLBACK_PASS_ON;

   sel = seat_sel->sel_list + i;

   EINA_LIST_FOREACH_SAFE(seat_sel->sel_lost_list, l, l_next, sel_lost)
     {
        if ((sel_lost->request == sel->owner) &&
            (sel_lost->type == i))
          {
             sel_debug("resolve the promise: %p", sel_lost->promise);
             eina_promise_resolve(sel_lost->promise, eina_value_uint_init(sel_lost->type));
             seat_sel->sel_lost_list = eina_list_remove(seat_sel->sel_lost_list, sel_lost);
             free(sel_lost);
          }
     }
   sel->active = EINA_FALSE;
   sel->owner = NULL;

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_x11_general_converter(char *target EINA_UNUSED, void *data, int size EINA_UNUSED, void **data_ret, int *size_ret, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
{
   sel_debug("general converter");
   Sel_Manager_Selection *sel = *(Sel_Manager_Selection **)data;
   if (sel->format == EFL_SELECTION_FORMAT_NONE)
     {
        if (data_ret)
          {
             *data_ret = malloc(sel->data.len * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, sel->data.mem, sel->data.len);
             ((char**)(data_ret))[0][sel->data.len] = 0;
          }
        if (size_ret) *size_ret = sel->data.len;
     }
   else
     {
        if (sel->data.mem)
          {
             if (data_ret)
               *data_ret = eina_memdup(sel->data.mem, sel->data.len, 1);
             if (size_ret) *size_ret = sel->data.len;
          }
        else
          {
             if (data_ret) *data_ret = NULL;
             if (size_ret) *size_ret = 0;
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_x11_targets_converter(char *target EINA_UNUSED, void *data, int size EINA_UNUSED, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize)
{
   int i, count;
   Ecore_X_Atom *aret;
   Sel_Manager_Selection *sel;
   Efl_Selection_Format seltype;

   sel_debug("target converter");
   if (!data_ret) return EINA_FALSE;

   sel = *(Sel_Manager_Selection **)data;
   seltype = sel->format;
   Sel_Manager_Seat_Selection *seat_sel = sel->seat_sel;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;

   for (i = SELECTION_ATOM_LISTING_ATOMS + 1, count = 0; i < SELECTION_N_ATOMS ; i++)
     {
        if (seltype & pd->atom_list[i].format) count++;
     }
   aret = malloc(sizeof(Ecore_X_Atom) * count);
   if (!aret) return EINA_FALSE;
   for (i = SELECTION_ATOM_LISTING_ATOMS + 1, count = 0; i < SELECTION_N_ATOMS ; i++)
     {
        if (seltype & pd->atom_list[i].format)
          aret[count ++] = pd->atom_list[i].x_atom;
     }

   *data_ret = aret;
   if (typesize) *typesize = 32 /* urk */;
   if (ttype) *ttype = ECORE_X_ATOM_ATOM;
   if (size_ret) *size_ret = count;
   return EINA_TRUE;
}

static Eina_Bool
_x11_image_converter(char *target EINA_UNUSED, void *data EINA_UNUSED, int size EINA_UNUSED, void **data_ret EINA_UNUSED, int *size_ret EINA_UNUSED, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
{
   sel_debug("Image converter called");
   return EINA_TRUE;
}

static Eina_Bool
_x11_vcard_send(char *target EINA_UNUSED, void *data EINA_UNUSED, int size EINA_UNUSED, void **data_ret, int *size_ret, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
{
   Sel_Manager_Selection *sel;

   sel_debug("Vcard send called");
   sel = *(Sel_Manager_Selection **)data;
   if (data_ret) *data_ret = strdup(sel->data.mem);
   if (size_ret) *size_ret = strlen(sel->data.mem);
   return EINA_TRUE;
}

static Eina_Bool
_x11_text_converter(char *target, void *data, int size EINA_UNUSED, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize)
{
   Sel_Manager_Selection *sel;

   sel = *(Sel_Manager_Selection **)data;
   if (!sel) return EINA_FALSE;

   sel_debug("text converter");
   if (sel->format == EFL_SELECTION_FORMAT_NONE)
     {
        sel_debug("none");
        if (data_ret)
          {
             *data_ret = malloc(sel->data.len * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, data, sel->data.len);
             ((char**)(data_ret))[0][sel->data.len] = 0;
          }
        if (size_ret) *size_ret = sel->data.len;
        return EINA_TRUE;
     }

   if (!sel->active) return EINA_TRUE;

   if ((sel->format & EFL_SELECTION_FORMAT_MARKUP) ||
       (sel->format & EFL_SELECTION_FORMAT_HTML))
     {
        char *tmp = malloc(sel->data.len + 1);
        if (tmp)
          {
             strncpy(tmp, sel->data.mem, sel->data.len);
             tmp[sel->data.len] = 0;
             *data_ret = _elm_util_mkup_to_text(tmp);
             if (size_ret && *data_ret) *size_ret = strlen(*data_ret);
             free(tmp);
             sel_debug("markup or html: %s", (const char *)*data_ret);
          }
        else return EINA_FALSE;
     }
   else if (sel->format & EFL_SELECTION_FORMAT_TEXT)
     {
        ecore_x_selection_converter_text(target, sel->data.mem,
                                         sel->data.len,
                                         data_ret, size_ret,
                                         ttype, typesize);
        sel_debug("text");
     }
   else if (sel->format & EFL_SELECTION_FORMAT_IMAGE)
     {
        efl_file_get(sel->request_obj, (const char **)data_ret, NULL);
        if (!*data_ret) *data_ret = strdup("No file");
        else *data_ret = strdup(*data_ret);

        if (!*data_ret)
          {
             ERR("Failed to allocate memory!");
             *size_ret = 0;
             return EINA_FALSE;
          }

        *size_ret = strlen(*data_ret);
     }
   return EINA_TRUE;
}

static Eina_Future *
_x11_efl_sel_manager_selection_set(Efl_Selection_Manager_Data *pd, Efl_Object *owner,
                                   Efl_Selection_Type type, Efl_Selection_Format format, Eina_Slice data,
                                   Ecore_X_Window xwin, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;
   Eina_Bool same_win = EINA_FALSE;

   seat_sel = _x11_sel_manager_seat_selection_init(pd, seat);
   seat_sel->active_type = type;
   sel = seat_sel->sel_list + type;
   //support 1 app with multiple window, 1 selection manager
   if (seat_sel->xwin == xwin)
     same_win = EINA_TRUE;
   _owner_change_check(pd->sel_man, owner, seat_sel, sel, type, same_win);

   sel->owner = owner;
   free(sel->data.mem);
   sel->xwin = xwin;
   sel->data = eina_slice_dup(data);
   sel->active = EINA_TRUE;
   sel->format = format;

   sel->set(xwin, &sel, sizeof(&sel));
   sel_debug("data: %p (%zu)", &sel, sizeof(&sel));

   return _update_sel_lost_list(owner, type, seat_sel);
}

static void
_x11_efl_sel_manager_selection_get(const Efl_Object *request, Efl_Selection_Manager_Data *pd,
                                   Efl_Selection_Type type, Efl_Selection_Format format,
                                   void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb,
                                   Ecore_X_Window xwin, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;;

   sel_debug("request: %p, seat: %u, type: %d, format: %d", request, seat, type, format);
   seat_sel = _sel_manager_seat_selection_init(pd, seat);
   sel = seat_sel->sel_list + type;
   sel->request_obj = (Efl_Object *)request;
   sel->data_func_data = data_func_data;
   sel->data_func = data_func;
   sel->data_func_free_cb = data_func_free_cb;
   sel->request_format = format;
   sel->xwin = xwin;

   if (sel->active)
     {
        if (sel->data.mem &&
            ((format == sel->format) || (xwin == 0)))
          {
             sel_debug("use local data");
             Efl_Selection_Data seldata;

             seldata.content.mem = sel->data.mem;
             seldata.content.len = sel->data.len;
             seldata.pos.x = seldata.pos.y = 0;
             seldata.format = sel->format;
             sel->data_func(sel->data_func_data, sel->request_obj, &seldata);
             return;
          }
     }

   sel->request(xwin, ECORE_X_SELECTION_TARGET_TARGETS);
}

static void
_x11_win_rotation_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *win = data;
   int rot = elm_win_rotation_get(event->object);
   elm_win_rotation_set(win, rot);
}

static Eina_Bool
_x11_drag_mouse_up(void *data, int etype EINA_UNUSED, void *event)
{
   Sel_Manager_Seat_Selection *seat_sel = data;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;
   Ecore_X_Window xwin = seat_sel->xwin;
   Ecore_Event_Mouse_Button *ev = event;

   if ((ev->buttons == 1) &&
       (ev->event_window == xwin))
     {
        Eina_Bool have_drop_list = EINA_FALSE;
        Eina_List *l;
        Sel_Manager_Dropable *dropable;

        ecore_x_pointer_ungrab();
        ELM_SAFE_FREE(seat_sel->mouse_up_handler, ecore_event_handler_del);
        ELM_SAFE_FREE(seat_sel->dnd_status_handler, ecore_event_handler_del);
        ecore_x_dnd_self_drop();

        sel_debug("mouse up, xwin=%#llx\n", (unsigned long long)xwin);

        EINA_LIST_FOREACH(pd->drop_list, l, dropable)
          {
             if (xwin == _x11_xwin_get(dropable->obj))
               {
                  have_drop_list = EINA_TRUE;
                  break;
               }
          }
        if (!have_drop_list) ecore_x_dnd_aware_set(xwin, EINA_FALSE);
        efl_event_callback_call(seat_sel->drag_obj, EFL_UI_DND_EVENT_DRAG_DONE, NULL);
        if (seat_sel->drag_win)
          {
             if (seat_sel->drag_obj)
               {
                  if (elm_widget_is(seat_sel->drag_obj))
                    {
                       Evas_Object *win = elm_widget_top_get(seat_sel->drag_obj);
                       if (win && efl_isa(win, EFL_UI_WIN_CLASS))
                         efl_event_callback_del(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED,
                                                _x11_win_rotation_changed_cb, seat_sel->drag_win);
                    }
               }

             if (!seat_sel->accept)
               {  /* Commit animation when drag cancelled */
                  /* Record final position of dragwin, then do animation */
                  ecore_animator_timeline_add(0.3,
                        _drag_cancel_animate, seat_sel);
               }
             else
               {  /* No animation drop was committed */
                  Ecore_X_Window xdragwin = _x11_xwin_get(seat_sel->drag_win);
                  ecore_x_window_ignore_set(xdragwin, 0);
                  evas_object_del(seat_sel->drag_win);
                  seat_sel->drag_win = NULL;
                  sel_debug("deleted drag_win");
               }
          }

        seat_sel->drag_obj = NULL;
        seat_sel->accept = EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_x11_drag_move(void *data, Ecore_X_Xdnd_Position *pos)
{
   Sel_Manager_Seat_Selection *seat_sel = data;
   Efl_Dnd_Drag_Pos dp;

   evas_object_move(seat_sel->drag_win,
                    pos->position.x - seat_sel->drag_pos.x,
                    pos->position.y - seat_sel->drag_pos.y);
   seat_sel->drag_win_end.x = pos->position.x - seat_sel->drag_pos.x;
   seat_sel->drag_win_end.y = pos->position.y - seat_sel->drag_pos.y;
   sel_debug("dragevas: %p -> %p\n",
          seat_sel->drag_obj,
          evas_object_evas_get(seat_sel->drag_obj));
   dp.pos.x = pos->position.x;
   dp.pos.y = pos->position.y;
   dp.action = seat_sel->drag_action;
   //for drag side
   efl_event_callback_call(seat_sel->drag_obj, EFL_UI_DND_EVENT_DRAG_POS, &dp);
}

static void
_x11_drag_target_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *info EINA_UNUSED)
{
   Sel_Manager_Seat_Selection *seat_sel = data;
   Sel_Manager_Selection *sel = &seat_sel->sel_list[seat_sel->active_type];

   if (seat_sel->drag_obj == obj)
     {
        sel->request_obj = NULL;
        seat_sel->drag_obj = NULL;
     }
}

static Eina_Bool
_x11_dnd_status(void *data, int etype EINA_UNUSED, void *ev)
{
   Sel_Manager_Seat_Selection *seat_sel = data;
   Ecore_X_Event_Xdnd_Status *status = ev;

   seat_sel->accept = EINA_FALSE;

   /* Only thing we care about: will accept */
   if ((status) && (status->will_accept))
     {
        sel_debug("Will accept\n");
        seat_sel->accept = EINA_TRUE;
     }
   /* Won't accept */
   else
     {
        sel_debug("Won't accept accept\n");
     }
   efl_event_callback_call(seat_sel->drag_obj, EFL_UI_DND_EVENT_DRAG_ACCEPT, &seat_sel->accept);

   return EINA_TRUE;
}

static void
_x11_efl_sel_manager_drag_start(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                Efl_Object *drag_obj, Efl_Selection_Format format,
                                Eina_Slice data, Efl_Selection_Action action,
                                void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func,
                                Eina_Free_Cb icon_func_free_cb EINA_UNUSED,
                                Ecore_X_Window xwin, unsigned int seat)
{
   Ecore_X_Window xdragwin;
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;
   Ecore_Evas *ee;
   int x, y, x2 = 0, y2 = 0, x3, y3;
   Evas_Object *icon = NULL;
   int w = 0, h = 0;
   int ex, ey, ew, eh;
   Ecore_X_Atom actx;
   int i;
   int xr, yr, rot;

   seat_sel = _x11_sel_manager_seat_selection_init(pd, seat);
   if (!seat_sel) return;
   seat_sel->active_type = EFL_SELECTION_TYPE_DND;

   sel = &seat_sel->sel_list[seat_sel->active_type];
   ecore_x_dnd_types_set(xwin, NULL, 0);
   for (i = SELECTION_ATOM_LISTING_ATOMS + 1; i < SELECTION_N_ATOMS; i++)
     {
        if (format == EFL_SELECTION_FORMAT_TARGETS || (pd->atom_list[i].format & format))
          {
             ecore_x_dnd_type_set(xwin, pd->atom_list[i].name, EINA_TRUE);
             sel_debug("set dnd type: %s\n", pd->atom_list[i].name);
          }
     }

   sel->active = EINA_TRUE;
   sel->request_obj = drag_obj;
   sel->format = format;
   if (sel->data.mem) free(sel->data.mem);
   sel->data = eina_slice_dup(data);
   sel->action = action;
   seat_sel->drag_obj = drag_obj;
   seat_sel->drag_action = action;
   seat_sel->xwin = xwin;

   evas_object_event_callback_add(drag_obj, EVAS_CALLBACK_DEL,
                                  _x11_drag_target_del, seat_sel);
   /* TODO BUG: should increase dnd-awareness, in case it's drop target as well. See _x11_drag_mouse_up() */
   ecore_x_dnd_aware_set(xwin, EINA_TRUE);
   ecore_x_dnd_callback_pos_update_set(_x11_drag_move, seat_sel);
   ecore_x_dnd_self_begin(xwin, (unsigned char *)&sel, sizeof(Sel_Manager_Selection));
   actx = _x11_dnd_action_rev_map(seat_sel->drag_action);
   ecore_x_dnd_source_action_set(actx);
   ecore_x_pointer_grab(xwin);
   seat_sel->mouse_up_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
                                            _x11_drag_mouse_up, seat_sel);
   seat_sel->dnd_status_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_STATUS,
                                            _x11_dnd_status, seat_sel);
   seat_sel->drag_win = elm_win_add(NULL, "Elm-Drag", ELM_WIN_DND);
   elm_win_alpha_set(seat_sel->drag_win, EINA_TRUE);
   elm_win_override_set(seat_sel->drag_win, EINA_TRUE);
   xdragwin = _x11_xwin_get(seat_sel->drag_win);
   ecore_x_window_ignore_set(xdragwin, 1);

   /* dragwin has to be rotated as the main window is */
   if (elm_widget_is(drag_obj))
     {
        Evas_Object *win = elm_widget_top_get(drag_obj);
        if (win && efl_isa(win, EFL_UI_WIN_CLASS))
          {
             elm_win_rotation_set(seat_sel->drag_win, elm_win_rotation_get(win));
             efl_event_callback_add(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED,
                                    _x11_win_rotation_changed_cb, seat_sel->drag_win);
          }
     }

   if (icon_func)
     {
        Eina_Position2D off;

        icon = icon_func(icon_func_data, seat_sel->drag_win, drag_obj, &off);
        if (icon)
          {
             x2 = off.x;
             y2 = off.y;
             evas_object_geometry_get(icon, NULL, NULL, &w, &h);
          }
     }
   else
     {
        icon = elm_icon_add(seat_sel->drag_win);
        evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        // need to resize
     }
   elm_win_resize_object_add(seat_sel->drag_win, icon);

   /* Position subwindow appropriately */
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(drag_obj));
   ecore_evas_geometry_get(ee, &ex, &ey, &ew, &eh);
   evas_object_resize(seat_sel->drag_win, w, h);

   evas_object_show(icon);
   evas_object_show(seat_sel->drag_win);
   evas_pointer_canvas_xy_get(evas_object_evas_get(drag_obj), &x3, &y3);

   rot = ecore_evas_rotation_get(ee);
   switch (rot)
     {
      case 90:
         xr = y3;
         yr = ew - x3;
         seat_sel->drag_pos.x = y3 - y2;
         seat_sel->drag_pos.y = x3 - x2;
         break;
      case 180:
         xr = ew - x3;
         yr = eh - y3;
         seat_sel->drag_pos.x = x3 - x2;
         seat_sel->drag_pos.y = y3 - y2;
         break;
      case 270:
         xr = eh - y3;
         yr = x3;
         seat_sel->drag_pos.x = y3 - y2;
         seat_sel->drag_pos.y = x3 - x2;
         break;
      default:
         xr = x3;
         yr = y3;
         seat_sel->drag_pos.x = x3 - x2;
         seat_sel->drag_pos.y = y3 - y2;
         break;
     }
   x = ex + xr - seat_sel->drag_pos.x;
   y = ey + yr - seat_sel->drag_pos.y;
   evas_object_move(seat_sel->drag_win, x, y);
   seat_sel->drag_win_start = EINA_POSITION2D(x, y);
   seat_sel->drag_win_end = EINA_POSITION2D(x, y);
}

static void
_x11_dnd_dropable_handle(Efl_Selection_Manager_Data *pd, Sel_Manager_Dropable *dropable, Eina_Position2D pos, Efl_Selection_Action action)
{
   Sel_Manager_Dropable *d, *last_dropable = NULL;
   Eina_List *l;
   Eina_Inlist *itr;

   EINA_LIST_FOREACH(pd->drop_list, l, d)
     {
        if (d->last.in)
          {
             last_dropable = d;
             break;
          }
     }
   if (last_dropable)
     {
        if (last_dropable == dropable) // same
          {
             Evas_Coord ox, oy;
             Drop_Format *df;

             sel_debug("same obj dropable %p\n", dropable->obj);
             evas_object_geometry_get(dropable->obj, &ox, &oy, NULL, NULL);
             Efl_Dnd_Drag_Pos pos_data;
             if (!dropable->is_container)
               {
                  pos_data.pos = EINA_POSITION2D(pos.x - ox, pos.y - oy);
                  pos_data.item = NULL;
               }
             else
               {
                  Eina_Position2D posret = {0, 0};
                  Efl_Object *it = NULL;

                  if (dropable->item_func)
                    it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                             pos, &posret);
                  pos_data.pos = posret;
                  pos_data.item = it;
               }
             pos_data.format = dropable->last.format;
             pos_data.action = action;
             EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
               {
                  if (df->format & dropable->last.format)
                    efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_POS, &pos_data);
               }
          }
        else
          {
             if (dropable) // leave last obj and enter new one
               {
                  sel_debug("leave %p\n", last_dropable->obj);
                  sel_debug("enter %p\n", dropable->obj);
                  last_dropable->last.in = EINA_FALSE;
                  last_dropable->last.type = NULL;
                  dropable->last.in = EINA_TRUE;

                  Drop_Format *df;
                  EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
                    {
                       if (df->format &dropable->last.format)
                         efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_ENTER, NULL);
                    }
                  EINA_INLIST_FOREACH_SAFE(last_dropable->format_list, itr, df)
                    {
                       if (df->format & last_dropable->last.format)
                         efl_event_callback_call(last_dropable->obj, EFL_UI_DND_EVENT_DRAG_LEAVE, NULL);
                    }
               }
             else // leave last obj
               {
                  sel_debug("leave %p\n", last_dropable->obj);
                  last_dropable->last.in = EINA_FALSE;
                  last_dropable->last.type = NULL;

                  Drop_Format *df;
                  EINA_INLIST_FOREACH_SAFE(last_dropable->format_list, itr, df)
                    {
                       if (df->format & last_dropable->last.format)
                         efl_event_callback_call(last_dropable->obj, EFL_UI_DND_EVENT_DRAG_LEAVE, NULL);
                    }
               }
          }
     }
   else
     {
        if (dropable) // enter new obj
          {
             Evas_Coord ox, oy;

             sel_debug("enter %p\n", dropable->obj);
             evas_object_geometry_get(dropable->obj, &ox, &oy, NULL, NULL);
             dropable->last.in = EINA_TRUE;

             Drop_Format *df;
             Efl_Dnd_Drag_Pos pos_data;
             if (!dropable->is_container)
               {
                  pos_data.pos = EINA_POSITION2D(pos.x - ox, pos.y - oy);
                  pos_data.item = NULL;
               }
             else
               {
                  Eina_Position2D posret = {0, 0};
                  Efl_Object *it = NULL;
                  if (dropable->item_func)
                    it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                             pos, &posret);
                  pos_data.pos = posret;
                  pos_data.item = it;
               }
             pos_data.format = dropable->last.format;
             pos_data.action = action;
             EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
               {
                  if (df->format & dropable->last.format)
                    {
                       efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_ENTER, NULL);
                       efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_POS, &pos_data);
                    }
               }
          }
        else
          {
             sel_debug("both dropable & last_dropable are null\n");
          }
     }
}

static Sel_Manager_Dropable *
_x11_dropable_find(Efl_Selection_Manager_Data *pd, Ecore_X_Window win)
{
   Eina_List *l;
   Sel_Manager_Dropable *dropable;

   if (!pd->drop_list) return NULL;
   EINA_LIST_FOREACH(pd->drop_list, l, dropable)
     {
        if (_x11_xwin_get(dropable->obj) == win) return dropable;
     }
   return NULL;
}

static Evas *
_x11_evas_get_from_xwin(Efl_Selection_Manager_Data *pd, Ecore_X_Window win)
{
   /* Find the Evas connected to the window */
   Sel_Manager_Dropable *dropable = _x11_dropable_find(pd, win);
   return dropable ? evas_object_evas_get(dropable->obj) : NULL;
}

static Efl_Selection_Action
_x11_dnd_action_map(Ecore_X_Atom action)
{
   Efl_Selection_Action act = EFL_SELECTION_ACTION_UNKNOWN;

   if (action == ECORE_X_ATOM_XDND_ACTION_COPY)
     act = EFL_SELECTION_ACTION_COPY;
   else if (action == ECORE_X_ATOM_XDND_ACTION_MOVE)
     act = EFL_SELECTION_ACTION_MOVE;
   else if (action == ECORE_X_ATOM_XDND_ACTION_PRIVATE)
     act = EFL_SELECTION_ACTION_PRIVATE;
   else if (action == ECORE_X_ATOM_XDND_ACTION_ASK)
     act = EFL_SELECTION_ACTION_ASK;
   else if (action == ECORE_X_ATOM_XDND_ACTION_LIST)
     act = EFL_SELECTION_ACTION_LIST;
   else if (action == ECORE_X_ATOM_XDND_ACTION_LINK)
     act = EFL_SELECTION_ACTION_LINK;
   else if (action == ECORE_X_ATOM_XDND_ACTION_DESCRIPTION)
     act = EFL_SELECTION_ACTION_DESCRIPTION;
   return act;
}

static Ecore_X_Atom
_x11_dnd_action_rev_map(Efl_Selection_Action action)
{
   Ecore_X_Atom act = ECORE_X_ATOM_XDND_ACTION_MOVE;

   if (action == EFL_SELECTION_ACTION_COPY)
     act = ECORE_X_ATOM_XDND_ACTION_COPY;
   else if (action == EFL_SELECTION_ACTION_MOVE)
     act = ECORE_X_ATOM_XDND_ACTION_MOVE;
   else if (action == EFL_SELECTION_ACTION_PRIVATE)
     act = ECORE_X_ATOM_XDND_ACTION_PRIVATE;
   else if (action == EFL_SELECTION_ACTION_ASK)
     act = ECORE_X_ATOM_XDND_ACTION_ASK;
   else if (action == EFL_SELECTION_ACTION_LIST)
     act = ECORE_X_ATOM_XDND_ACTION_LIST;
   else if (action == EFL_SELECTION_ACTION_LINK)
     act = ECORE_X_ATOM_XDND_ACTION_LINK;
   else if (action == EFL_SELECTION_ACTION_DESCRIPTION)
     act = ECORE_X_ATOM_XDND_ACTION_DESCRIPTION;
   return act;
}

static Eina_Bool
_x11_dnd_enter(void *data, int etype EINA_UNUSED, void *ev)
{
   Sel_Manager_Seat_Selection *seat_sel = data;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;
   Ecore_X_Event_Xdnd_Enter *enter = ev;
   Sel_Manager_Dropable *dropable;
   int i;

   sel_debug("In");
   if (!enter) return EINA_TRUE;
   dropable = _x11_dropable_find(pd, enter->win);
   if (dropable)
     {
        sel_debug("Enter %x\n", enter->win);
     }
   /* Skip it */
   sel_debug("enter types=%p (%d)\n", enter->types, enter->num_types);
   if ((!enter->num_types) || (!enter->types)) return EINA_TRUE;

   sel_debug("Types\n");
   seat_sel->saved_types->ntypes = enter->num_types;
   free(seat_sel->saved_types->types);
   seat_sel->saved_types->types = malloc(sizeof(char *) * enter->num_types);
   if (!seat_sel->saved_types->types) return EINA_FALSE;

   for (i = 0; i < enter->num_types; i++)
     {
        seat_sel->saved_types->types[i] = eina_stringshare_add(enter->types[i]);
        sel_debug("Type is %s %p %p\n", enter->types[i],
                  seat_sel->saved_types->types[i], pd->text_uri);
        if (seat_sel->saved_types->types[i] == pd->text_uri)
          {
             /* Request it, so we know what it is */
             sel_debug("Sending uri request\n");
             seat_sel->saved_types->textreq = 1;
             ELM_SAFE_FREE(seat_sel->saved_types->imgfile, free);
             ecore_x_selection_xdnd_request(enter->win, pd->text_uri);
          }
     }

   /* FIXME: Find an object and make it current */
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_position(void *data, int etype EINA_UNUSED, void *ev)
{
   Sel_Manager_Seat_Selection *seat_sel = data;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;
   Ecore_X_Event_Xdnd_Position *xpos = ev;
   Ecore_X_Rectangle rect = { 0, 0, 0, 0 };
   Sel_Manager_Dropable *dropable;
   Efl_Selection_Action act;

   sel_debug("In");
   /* Need to send a status back */
   /* FIXME: Should check I can drop here */
   /* FIXME: Should highlight widget */
   dropable = _x11_dropable_find(pd, xpos->win);
   if (dropable)
     {
        Evas_Coord ox = 0, oy = 0;
        Eina_Position2D pos;

        act = _x11_dnd_action_map(xpos->action);
        pos.x = xpos->position.x;
        pos.y = xpos->position.y;
        _dropable_coords_adjust(dropable, &pos);
        Evas *evas = _x11_evas_get_from_xwin(pd, xpos->win);
        Eina_List *dropable_list = evas ? _dropable_list_geom_find(pd, evas, pos.x, pos.y) : NULL;
        /* check if there is dropable (obj) can accept this drop */
        if (dropable_list)
          {
             Efl_Selection_Format saved_format = _dnd_types_to_format(pd, seat_sel->saved_types->types, seat_sel->saved_types->ntypes);
             Eina_List *l;
             Eina_Bool found = EINA_FALSE;

             EINA_LIST_FOREACH(dropable_list, l, dropable)
               {
                  Drop_Format *df;
                  Eina_Inlist *itr;
                  EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
                    {
                       Efl_Selection_Format common_fmt = saved_format & df->format;
                       if (common_fmt)
                         {
                            //We found a target that can accept this type of data
                            int i, min_index = SELECTION_N_ATOMS;
                            //We have to find the first atom that corresponds to one
                            //of the supported data types.
                            for (i = 0; i < seat_sel->saved_types->ntypes; i++)
                              {
                                 Sel_Manager_Atom *atom = eina_hash_find(pd->type_hash, seat_sel->saved_types->types[i]);
                                 if (atom && (atom->format & common_fmt))
                                   {
                                      int atom_idx = (atom - pd->atom_list);
                                      if (min_index > atom_idx) min_index = atom_idx;
                                   }
                              }
                            if (min_index != SELECTION_N_ATOMS)
                              {
                                 sel_debug("Found atom %s\n", pd->atom_list[min_index].name);
                                 found = EINA_TRUE;
                                 dropable->last.type = pd->atom_list[min_index].name;
                                 dropable->last.format = common_fmt;
                                 break;
                              }
                         }
                    }
                  if (found) break;
               }
             if (found)
               {
                  Sel_Manager_Dropable *d = NULL;
                  Eina_Rectangle inter_rect = {0, 0, 0, 0};
                  int idx = 0;
                  EINA_LIST_FOREACH(dropable_list, l, d)
                    {
                       if (idx == 0)
                         {
                            evas_object_geometry_get(d->obj, &inter_rect.x, &inter_rect.y,
                                                     &inter_rect.w, &inter_rect.h);
                         }
                       else
                         {
                            Eina_Rectangle cur_rect;
                            evas_object_geometry_get(d->obj, &cur_rect.x, &cur_rect.y,
                                                     &cur_rect.w, &cur_rect.h);
                            if (!eina_rectangle_intersection(&inter_rect, &cur_rect)) continue;
                         }
                       idx++;
                    }
                  rect.x = inter_rect.x;
                  rect.y = inter_rect.y;
                  rect.width = inter_rect.w;
                  rect.height = inter_rect.h;
                  ecore_x_dnd_send_status(EINA_TRUE, EINA_FALSE, rect, xpos->action);
                  sel_debug("dnd position %i %i %p\n", pos.x - ox, pos.y - oy, dropable);
                  pos.x = pos.x - ox;
                  pos.y = pos.y - oy;
                  _x11_dnd_dropable_handle(pd, dropable, pos, act);
                  // CCCCCCC: call dnd exit on last obj if obj != last
                  // CCCCCCC: call drop position on obj
               }
             else
               {
                  //if not: send false status
                  ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, xpos->action);
                  sel_debug("dnd position (%d, %d) not in obj\n", pos.x, pos.y);
                  _x11_dnd_dropable_handle(pd, NULL, EINA_POSITION2D(0, 0), act);
                  // CCCCCCC: call dnd exit on last obj
               }
             eina_list_free(dropable_list);
          }
        else
          {
             ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, xpos->action);
             sel_debug("dnd position (%d, %d) has no drop\n", pos.x, pos.y);
             _x11_dnd_dropable_handle(pd, NULL, EINA_POSITION2D(0, 0), act);
          }
     }
   else
     {
        ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, xpos->action);
     }
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_leave(void *data, int etype EINA_UNUSED, void *ev)
{
   Sel_Manager_Seat_Selection *seat_sel = data;
   Eina_Position2D pos = {0, 0};
#ifdef DEBUGON
   sel_debug("Leave %x\n", ((Ecore_X_Event_Xdnd_Leave *)ev)->win);
#else
   (void)ev;
#endif
   _x11_dnd_dropable_handle(seat_sel->pd, NULL, pos, EFL_SELECTION_ACTION_UNKNOWN);
   // CCCCCCC: call dnd exit on last obj if there was one
   // leave->win leave->source
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_drop(void *data, int etype EINA_UNUSED, void *ev)
{
   sel_debug("In");
   Sel_Manager_Seat_Selection *seat_sel = data;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;
   Ecore_X_Event_Xdnd_Drop *drop;
   Sel_Manager_Dropable *dropable = NULL;
   Efl_Selection_Data ddata;
   Evas_Coord x = 0, y = 0;
   Efl_Selection_Action act = EFL_SELECTION_ACTION_UNKNOWN;
   Eina_List *l;
   Eina_Inlist *itr;
   Sel_Manager_Selection *sel;

   drop = ev;
   sel_debug("drop_list %p (%d)\n", pd->drop_list, eina_list_count(pd->drop_list));
   if (!(dropable = _x11_dropable_find(pd, drop->win))) return EINA_TRUE;

   /* Calculate real (widget relative) position */
   // - window position
   // - widget position
   seat_sel->saved_types->pos = EINA_POSITION2D(drop->position.x, drop->position.y);
   _dropable_coords_adjust(dropable, &seat_sel->saved_types->pos);

   sel_debug("Drop position is %d,%d\n", seat_sel->saved_types->pos.x, seat_sel->saved_types->pos.y);

   EINA_LIST_FOREACH(pd->drop_list, l, dropable)
     {
        if (dropable->last.in)
          {
             evas_object_geometry_get(dropable->obj, &x, &y, NULL, NULL);
             seat_sel->saved_types->pos.x -= x;
             seat_sel->saved_types->pos.y -= y;
             goto found;
          }
     }

   sel_debug("Didn't find a target\n");
   return EINA_TRUE;

found:
   sel_debug("0x%x\n", drop->win);

   act = _x11_dnd_action_map(drop->action);

   dropable->last.in = EINA_FALSE;
   sel_debug("Last type: %s - Last format: %X\n", dropable->last.type, dropable->last.format);
   if ((!strcmp(dropable->last.type, pd->text_uri)))
     {
        sel_debug("We found a URI... (%scached) %s\n",
                  seat_sel->saved_types->imgfile ? "" : "not ",
                  seat_sel->saved_types->imgfile);
        if (seat_sel->saved_types->imgfile)
          {
             Drop_Format *df;

             if (!dropable->is_container)
               {
                  ddata.pos = seat_sel->saved_types->pos;
                  ddata.item = NULL;
               }
             else
               {
                  //for container
                  Efl_Object *it = NULL;
                  Evas_Coord x0 = 0, y0 = 0;
                  Eina_Position2D pos, posret = {0, 0};

                  evas_object_geometry_get(dropable->obj, &x0, &y0, NULL, NULL);
                  pos = EINA_POSITION2D(seat_sel->saved_types->pos.x + x0,
                                        seat_sel->saved_types->pos.y + y0);
                  if (dropable->item_func)
                    it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                             pos, &posret);
                  ddata.pos = posret;
                  ddata.item = it;
               }
             ddata.action = act;

             EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
               {
                  if (df->format & EFL_SELECTION_FORMAT_IMAGE)
                    {
                       sel_debug("Doing image insert (%s)\n", seat_sel->saved_types->imgfile);
                       ddata.format = EFL_SELECTION_FORMAT_IMAGE;
                       ddata.content.mem = (char *)seat_sel->saved_types->imgfile;
                       ddata.content.len = strlen(ddata.content.mem);
                       if (df->format & dropable->last.format)
                         efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_DROP, &ddata);
                    }
                  else
                    {
                       sel_debug("Item doesn't support images... passing\n");
                    }
               }
             ecore_x_dnd_send_finished();
             ELM_SAFE_FREE(seat_sel->saved_types->imgfile, free);
             return EINA_TRUE;
          }
        else if (seat_sel->saved_types->textreq)
          {
             /* Already asked: Pretend we asked now, and paste immediately when
              * it comes in */
             seat_sel->saved_types->textreq = 0;
             ecore_x_dnd_send_finished();
             return EINA_TRUE;
          }
     }

   sel = seat_sel->sel_list + EFL_SELECTION_TYPE_DND;
   sel_debug("doing a request then: %s\n", dropable->last.type);
   sel->xwin = drop->win;
   sel->request_obj = dropable->obj;
   sel->request_format = dropable->last.format;
   sel->active = EINA_TRUE;
   sel->action = act;
   ecore_x_selection_xdnd_request(drop->win, dropable->last.type);

   return EINA_TRUE;
}

static Eina_Bool
_x11_sel_manager_drop_target_add(Efl_Selection_Manager_Data *pd, Efl_Object *target_obj,
                                 Efl_Selection_Format format, Ecore_X_Window xwin,
                                 unsigned int seat)
{
   Sel_Manager_Dropable *dropable = NULL;
   Eina_List *l;
   Eina_Bool have_drop_list = EINA_FALSE;
   Sel_Manager_Seat_Selection *seat_sel = NULL;

   /* Is this the first? */
   EINA_LIST_FOREACH(pd->drop_list, l, dropable)
     {
        if (xwin == _x11_xwin_get(dropable->obj))
          {
             have_drop_list = EINA_TRUE;
             break;
          }
     }
   dropable = NULL; // In case of error, we don't want to free it


   Drop_Format *df = calloc(1, sizeof(Drop_Format));
   if (!df) return EINA_FALSE;
   df->format = format;

   dropable = efl_key_data_get(target_obj, "__elm_dropable");
   if (!dropable)
     {
        /* Create new drop */
        dropable = calloc(1, sizeof(Sel_Manager_Dropable));
        if (!dropable) goto error;
        dropable->last.in = EINA_FALSE;
        pd->drop_list = eina_list_append(pd->drop_list, dropable);
        if (!pd->drop_list) goto error;
        dropable->obj = target_obj;
        efl_key_data_set(target_obj, "__elm_dropable", dropable);
     }
   dropable->format_list = eina_inlist_append(dropable->format_list, EINA_INLIST_GET(df));
   dropable->seat = seat;

   evas_object_event_callback_add(target_obj, EVAS_CALLBACK_DEL,
                                  _all_drop_targets_cbs_del, pd);
   if (!have_drop_list) ecore_x_dnd_aware_set(xwin, EINA_TRUE);

   seat_sel = _x11_sel_manager_seat_selection_init(pd, seat);

   if (seat_sel->enter_handler) return EINA_TRUE;
   sel_debug("Adding drop target calls xwin=%#llx", (unsigned long long)xwin);
   seat_sel->enter_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER,
                                               _x11_dnd_enter, seat_sel);
   seat_sel->leave_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_LEAVE,
                                               _x11_dnd_leave, seat_sel);
   seat_sel->pos_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION,
                                             _x11_dnd_position, seat_sel);
   seat_sel->drop_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP,
                                              _x11_dnd_drop, seat_sel);
   return EINA_TRUE;
error:
   free(df);
   free(dropable);
   return EINA_FALSE;
}

#endif

//Wayland
#ifdef HAVE_ELEMENTARY_WL2
static Sel_Manager_Seat_Selection *
_wl_sel_manager_seat_selection_init(Efl_Selection_Manager_Data *pd, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if(seat_sel->seat == seat)
          break;
     }
   if (!seat_sel)
     {
        seat_sel = calloc(1, sizeof(Sel_Manager_Seat_Selection));
        if (!seat_sel)
          {
             ERR("Failed to allocate seat");
             return NULL;
          }
        seat_sel->saved_types = calloc(1, sizeof(Saved_Type));
        seat_sel->seat = seat;
        seat_sel->pd = pd;
        pd->seat_list = eina_list_append(pd->seat_list, seat_sel);
     }
   if (!seat_sel->sel)
     {
        Sel_Manager_Selection *sel = calloc(1, sizeof(Sel_Manager_Selection));
        if (!sel)
          {
             ERR("failed to allocate selection");
             return NULL;
          }
        sel->seat_sel = seat_sel;
        seat_sel->sel = sel;
     }

   return seat_sel;
}

static void
_wl_drag_source_del(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Sel_Manager_Seat_Selection *seat_sel = data;
   if (seat_sel->drag_obj == obj)
     seat_sel->drag_obj = NULL;
}

static void
_wl_efl_sel_manager_drag_start(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd, Efl_Object *drag_obj,
                               Efl_Selection_Format format, Eina_Slice data,
                               Efl_Selection_Action action, void *icon_func_data,
                               Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb icon_func_free_cb EINA_UNUSED,
                               Ecore_Wl2_Window *win, unsigned int seat)
{
   Ecore_Evas *ee;
   Evas_Object *icon = NULL;
   int x, y, x2 = 0, y2 = 0, x3, y3, w = 0, h = 0;
   const char *types[SELECTION_N_ATOMS + 1];
   int i, nb_types = 0;
   Ecore_Wl2_Window *parent = NULL;
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;

   sel_debug("In");
   seat_sel = _wl_sel_manager_seat_selection_init(pd, seat);
   if (!seat_sel) return;
   seat_sel->active_type = EFL_SELECTION_TYPE_DND;
   sel = seat_sel->sel;

   sel_debug("checking drag_win: %p", seat_sel->drag_win);
   /* if we already have a drag, get out */
   if (seat_sel->drag_win) return;

   for (i = SELECTION_ATOM_LISTING_ATOMS + 1; i < SELECTION_N_ATOMS; i++)
     {
        if (format == EFL_SELECTION_FORMAT_TARGETS || (pd->atom_list[i].format & format))
          {
             types[nb_types++] = pd->atom_list[i].name;
             sel_debug("set dnd type: %s\n", pd->atom_list[i].name);
          }
     }
   types[nb_types] = NULL;

   ecore_wl2_dnd_drag_types_set(_wl_seat_get(win, drag_obj, seat), types);

   /* set the drag data used when a drop occurs */
   free(sel->data.mem);
   sel->data.len = 0;
   sel->data = eina_slice_dup(data);

   /* setup callback to notify if this object gets deleted */
   evas_object_event_callback_add(drag_obj, EVAS_CALLBACK_DEL,
                                  _wl_drag_source_del, sel);

   seat_sel->drag_obj = drag_obj;
   seat_sel->drag_action = action;

   seat_sel->drag_win = elm_win_add(NULL, "Elm-Drag", ELM_WIN_DND);
   elm_win_alpha_set(seat_sel->drag_win, EINA_TRUE);
   elm_win_borderless_set(seat_sel->drag_win, EINA_TRUE);
   elm_win_override_set(seat_sel->drag_win, EINA_TRUE);

   win = elm_win_wl_window_get(seat_sel->drag_win);

   if (icon_func)
     {
        Eina_Position2D off;

        icon = icon_func(icon_func_data, seat_sel->drag_win, drag_obj, &off);
        if (icon)
          {
             x2 = off.x;
             y2 = off.y;
             evas_object_geometry_get(icon, NULL, NULL, &w, &h);
          }
     }
   else
     {
        icon = elm_icon_add(seat_sel->drag_win);
        evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
     }

   elm_win_resize_object_add(seat_sel->drag_win, icon);
   evas_object_show(icon);

   /* Position subwindow appropriately */
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(drag_obj));
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   x += x2;
   y += y2;
   seat_sel->drag_win_start.x = seat_sel->drag_win_end.x = x;
   seat_sel->drag_win_start.y = seat_sel->drag_win_end.y = y;

   evas_object_move(seat_sel->drag_win, x, y);
   evas_object_resize(seat_sel->drag_win, w, h);
   evas_object_show(seat_sel->drag_win);

   evas_pointer_canvas_xy_get(evas_object_evas_get(drag_obj), &x3, &y3);
   seat_sel->drag_pos.x = x3 - x2;
   seat_sel->drag_pos.y = y3 - y2;

   if (elm_widget_is(drag_obj))
     {
        Evas_Object *top;

        top = elm_widget_top_get(drag_obj);
        if (!top) top = elm_widget_top_get(elm_widget_parent_widget_get(drag_obj));
        if (top && (efl_isa(top, EFL_UI_WIN_CLASS)))
          parent = elm_win_wl_window_get(top);
     }
   if (!parent)
     {
        Evas *evas;

        if (!(evas = evas_object_evas_get(drag_obj)))
          return;
        if (!(ee = ecore_evas_ecore_evas_get(evas)))
          return;

        parent = ecore_evas_wayland2_window_get(ee);
     }

   sel->drag_serial = ecore_wl2_dnd_drag_start(_wl_seat_get(win, drag_obj, seat), parent, win);
}

static Eina_Bool
_wl_is_uri_type_data(const char *data, int len)
{
   char *p;
   if (len < 6) return EINA_FALSE;

   p = (char *)data;
   if (!p) return EINA_FALSE;
   if (strncmp(p, "file:/", 6))
     {
        if (*p != '/') return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Efl_Selection_Action
_wl_to_elm(Ecore_Wl2_Drag_Action action)
{
   #define CONV(wl, elm) if (action == wl) return elm;
   CONV(ECORE_WL2_DRAG_ACTION_COPY, EFL_SELECTION_ACTION_COPY);
   CONV(ECORE_WL2_DRAG_ACTION_MOVE, EFL_SELECTION_ACTION_MOVE);
   CONV(ECORE_WL2_DRAG_ACTION_ASK, EFL_SELECTION_ACTION_ASK);
   #undef CONV
   return EFL_SELECTION_ACTION_UNKNOWN;
}

static Eina_Bool
_wl_targets_converter(char *target, Sel_Manager_Selection *sel, void *data EINA_UNUSED, int size EINA_UNUSED, void **data_ret, int *size_ret)
{
   sel_debug("in\n");
   if (!data_ret) return EINA_FALSE;

   const char *sep = "\n";
   char *aret;
   int len = 0;
   int i = 0;
   Sel_Manager_Seat_Selection *seat_sel = sel->seat_sel;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;
   Efl_Selection_Format format = EFL_SELECTION_FORMAT_NONE;
   Eina_Bool is_uri = EINA_FALSE;

   if (sel->format)
     {
        format = sel->format;
        is_uri = _wl_is_uri_type_data(sel->data.mem, sel->data.len);
     }
   else
     {
        Sel_Manager_Atom *atom = eina_hash_find(pd->type_hash, target);
        if (atom)
          format = atom->format;
     }
   for (i = 0; i < SELECTION_N_ATOMS; i++)
     {
        if (format & pd->atom_list[i].format)
          {
             if ((is_uri) ||
                 ((!is_uri) && strcmp(pd->atom_list[i].name, "text/uri-list")))
               len += strlen(pd->atom_list[i].name) + strlen(sep);
          }
     }
   len++;  //terminating null byte
   aret = calloc(1, len * sizeof(char));
   if (!aret) return EINA_FALSE;
   for (i = 0; i < SELECTION_N_ATOMS; i++)
     {
        if (format & pd->atom_list[i].format)
          {
             if ((is_uri) ||
                 ((!is_uri) && strcmp(pd->atom_list[i].name, "text/uri-list")))
               {
                  aret = strcat(aret, pd->atom_list[i].name);
                  aret = strcat(aret, sep);
               }
          }
     }
   *data_ret = aret;
   if (size_ret) *size_ret = len;

   return EINA_TRUE;
}

static Eina_Bool
_wl_general_converter(char *target, Sel_Manager_Selection *sel, void *data, int size, void **data_ret, int *size_ret)
{
   Efl_Selection_Format format = EFL_SELECTION_FORMAT_NONE;
   Sel_Manager_Atom *atom = NULL;
   Sel_Manager_Seat_Selection *seat_sel = sel->seat_sel;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;

   sel_debug("in\n");

   atom = eina_hash_find(pd->type_hash, target);
   if (atom)
     format = atom->format;
   if (format == EFL_SELECTION_FORMAT_NONE)
     {
        if (data_ret)
          {
             *data_ret = malloc(size * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, data, size);
             ((char**)(data_ret))[0][size] = 0;
          }
        if (size_ret) *size_ret = size;
     }
   else
     {
        if ((data) && (size > 0))
          {
             char *tmp = malloc(size);
             if (tmp)
               {
                  memcpy(tmp, data, size);
                  if (data_ret) *data_ret = tmp;
                  if (size_ret) *size_ret = size;
               }
          }
        else
          {
             if (data_ret) *data_ret = NULL;
             if (size_ret) *size_ret = 0;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_wl_text_converter(char *target, Sel_Manager_Selection *sel, void *data, int size, void **data_ret, int *size_ret)
{
   Efl_Selection_Format format = EFL_SELECTION_FORMAT_NONE;
   Sel_Manager_Atom *atom = NULL;
   Sel_Manager_Seat_Selection *seat_sel = sel->seat_sel;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;

   sel_debug("in\n");

   atom = eina_hash_find(pd->type_hash, target);
   if (atom)
     format = atom->format;
   if (format == EFL_SELECTION_FORMAT_NONE)
     {
        if (data_ret)
          {
             *data_ret = malloc(size * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, data, size);
             ((char**)(data_ret))[0][size] = 0;
             if (size_ret) *size_ret = size;
             return EINA_TRUE;
          }
     }
   else if ((format & EFL_SELECTION_FORMAT_MARKUP) ||
            (format & EFL_SELECTION_FORMAT_HTML))
     {
        char *tmp = malloc(size + 1);
        if (tmp)
          {
             strncpy(tmp, data, size);
             tmp[size] = 0;
             *data_ret = _elm_util_mkup_to_text(tmp);
             if (size_ret && *data_ret) *size_ret = strlen(*data_ret);
             free(tmp);
          }
        else return EINA_FALSE;
     }
   else if (format & EFL_SELECTION_FORMAT_TEXT)
     {
        char *tmp = malloc(size + 1);
        if (tmp)
          {
             strncpy(tmp, data, size);
             tmp[size] = 0;
             *data_ret = tmp;
             if (size_ret && *data_ret) *size_ret = strlen(*data_ret);
          }
        else return EINA_FALSE;
     }
   else if (format & EFL_SELECTION_FORMAT_IMAGE)
     {
        sel_debug("Image %s\n", evas_object_type_get(sel->request_obj));
        efl_file_get(sel->request_obj, (const char **)data_ret, NULL);
        if (!*data_ret) *data_ret = strdup("No file");
        else *data_ret = strdup(*data_ret);

        if (!*data_ret)
          {
             ERR("Failed to allocate memory!");
             *size_ret = 0;
             return EINA_FALSE;
          }

        if (size_ret) *size_ret = strlen(*data_ret);
     }
   return EINA_TRUE;
}

static void
_wl_sel_obj_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Sel_Manager_Selection *sel = data;
   if (sel->owner == obj)
     {
        sel->owner = NULL;
     }
   //if (dragwidget == obj) dragwidget = NULL;
}

static Eina_Future *
_wl_efl_sel_manager_selection_set(Efl_Selection_Manager_Data *pd,
                                  Efl_Object *owner, Efl_Selection_Type type,
                                  Efl_Selection_Format format,
                                  Eina_Slice data,
                                  Ecore_Wl2_Window *win,
                                  unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;
   int i = 0, count = 0;
   Eina_Bool is_uri = EINA_FALSE;
   const char **types;

   if ((!data.mem) && (format != EFL_SELECTION_FORMAT_IMAGE))
     {
        efl_selection_manager_selection_clear(pd->sel_man, owner, type, seat);
        return NULL;
     }

   if (data.len <= 0)
     return NULL;

   seat_sel = _wl_sel_manager_seat_selection_init(pd, seat);
   seat_sel->active_type = type;
   sel = seat_sel->sel;

   if (sel->owner != owner)
     {
        Eina_List *l, *l_next;
        Sel_Manager_Selection_Lost *sel_lost;

        EINA_LIST_FOREACH_SAFE(seat_sel->sel_lost_list, l, l_next, sel_lost)
          {
             if ((sel_lost->request == sel->owner) &&
                 (sel_lost->type == type))
               {
                  eina_promise_resolve(sel_lost->promise, eina_value_uint_init(sel_lost->type));
                  seat_sel->sel_lost_list = eina_list_remove(seat_sel->sel_lost_list, sel_lost);
                  free(sel_lost);
               }
          }
     }

   if (sel->owner)
     evas_object_event_callback_del_full(sel->owner, EVAS_CALLBACK_DEL,
                                         _wl_sel_obj_del, sel);
   sel->active = EINA_TRUE;
   sel->owner = owner;
   sel->win = win;
   /* sel->set(win, &selection, sizeof(Elm_Sel_Type)); */
   sel->format = format;

   evas_object_event_callback_add
     (sel->owner, EVAS_CALLBACK_DEL, _wl_sel_obj_del, &sel);

   sel->data = eina_slice_dup(data);
   if (!sel->data.mem)
     {
        efl_selection_manager_selection_clear(pd->sel_man, owner, type, seat_sel->seat);
        return NULL;
     }

   is_uri = _wl_is_uri_type_data(sel->data.mem, sel->data.len);
   types = malloc(sizeof(char *));
   if (!types) return NULL;
   for (i = 0, count = 1; i < SELECTION_N_ATOMS; i++)
     {
        if (format & pd->atom_list[i].format)
          {
             if ((is_uri) ||
                 ((!is_uri) && strcmp(pd->atom_list[i].name, "text/uri-list")))
               {
                  const char **t = NULL;

                  types[count - 1] = pd->atom_list[i].name;
                  count++;
                  t = realloc(types, sizeof(char *) * count);
                  if (!t)
                    {
                       free(types);
                       return NULL;
                    }
                  types = t;
               }
          }
     }
   types[count - 1] = 0;

   sel->selection_serial = ecore_wl2_dnd_selection_set(_wl_seat_get(win, owner, seat_sel->seat), types);
   ERR("serial: %d", sel->selection_serial);

   free(types);
   //return _local_elm_cnp_selection_set(obj, selection, format, buf, buflen);

   return _update_sel_lost_list(owner, type, seat_sel);
}

/*static void
_wl_selection_changed_free(void *data, void *ev EINA_UNUSED)
{
   ecore_wl2_display_disconnect(data);
}*/

static Eina_Bool
_wl_selection_changed(void *data, int type EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = data;
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;
   Efl_Selection_Changed e;
   Ecore_Wl2_Event_Seat_Selection *ev = event;
   Ecore_Wl2_Input *seat;

   seat_sel = _wl_sel_manager_seat_selection_init(pd, ev->seat);
   sel_debug("seat: %d", ev->seat);
   if (!seat_sel) return ECORE_CALLBACK_RENEW;
   sel = seat_sel->sel;

   seat = ecore_wl2_display_input_find(ev->display, ev->seat);
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, ECORE_CALLBACK_RENEW);
   e.type = EFL_SELECTION_TYPE_CLIPBOARD;
   e.seat = ev->seat;
   /* connect again to add ref */
   e.display = ecore_wl2_display_connect(ecore_wl2_display_name_get(ev->display));
   e.exist = !!ecore_wl2_dnd_selection_get(seat);
   //ecore_event_add(ELM_CNP_EVENT_SELECTION_CHANGED, e, _wl_selection_changed_free, ev->display);
   efl_event_callback_call(sel->request_obj, EFL_SELECTION_EVENT_SELECTION_CHANGED, &e);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_wl_selection_send(void *data, int type EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = data;
   char *buf;
   int ret, len_remained;
   int len_written = 0;
   Ecore_Wl2_Event_Data_Source_Send *ev;
   int seat;
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;
   void *data_ret = NULL;
   int len_ret = 0;
   int i = 0;

   ev = event;
   seat = ev->seat;
   sel_debug("seat: %d, type: %d", seat, type);
   seat_sel = _wl_sel_manager_seat_selection_init(pd, seat);
   if (!seat_sel) return ECORE_CALLBACK_RENEW;
   sel = seat_sel->sel;

   if ((ev->serial != sel->selection_serial) &&
       (ev->serial != sel->drag_serial))
    return ECORE_CALLBACK_RENEW;

   for (i = 0; i < SELECTION_N_ATOMS; i++)
     {
        if (!strcmp(pd->atom_list[i].name, ev->type))
          {
             sel_debug("Found a type: %s\n", pd->atom_list[i].name);
             Sel_Manager_Dropable *drop;
             drop = efl_key_data_get(sel->request_obj, "__elm_dropable");
             if (drop)
               drop->last.type = pd->atom_list[i].name;
             if (pd->atom_list[i].wl_converter)
               {
                  pd->atom_list[i].wl_converter(ev->type, sel, sel->data.mem,
                                         sel->data.len, &data_ret, &len_ret);
               }
             else
               {
                  data_ret = eina_memdup(sel->data.mem, sel->data.len, 0);
                  len_ret = sel->data.len;
               }
             break;
          }
     }

   len_remained = len_ret;
   buf = data_ret;

   while (len_written < len_ret)
     {
        ret = write(ev->fd, buf, len_remained);
        if (ret == -1) break;
        buf += ret;
        len_written += ret;
        len_remained -= ret;
     }
   free(data_ret);

   close(ev->fd);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_end(void *data, int type EINA_UNUSED, void *event)
{
   sel_debug("In");
   Efl_Selection_Manager_Data *pd = data;
   Ecore_Wl2_Event_Data_Source_End *ev;
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;

   ev = event;
   seat_sel = _wl_sel_manager_seat_selection_init(pd, ev->seat);
   sel = seat_sel->sel;
   if (ev->serial != sel->drag_serial)
    return ECORE_CALLBACK_RENEW;

   if (seat_sel->active_type != EFL_SELECTION_TYPE_DND)
     return ECORE_CALLBACK_RENEW;

   efl_event_callback_call(seat_sel->drag_obj, EFL_UI_DND_EVENT_DRAG_DONE, NULL);
   if (seat_sel->drag_win)
     {
        if (!seat_sel->accept)
          {
             /* Commit animation when drag cancelled */
             /* Record final position of dragwin, then do animation */
             ecore_animator_timeline_add(0.3, _drag_cancel_animate, seat_sel);
          }
        else
          {
             /* No animation drop was committed */
             evas_object_del(seat_sel->drag_win);
             seat_sel->drag_win = NULL;
          }
     }

   seat_sel->accept = EINA_FALSE;

   return ECORE_CALLBACK_PASS_ON;
}

static Ecore_Wl2_Input *
_wl_seat_get(Ecore_Wl2_Window *win, Evas_Object *obj, unsigned int seat_id)
{
   Eo *seat, *parent2, *ewin;
   Ecore_Wl2_Input *input = NULL;

   input = ecore_wl2_display_input_find(ecore_wl2_window_display_get(win), seat_id);
   if (input) return input;

   if (obj)
     {
        // FIXME (there might be a better solution):
        // In case of inwin, we want to use the main wl2 window for cnp, but obj
        // obj belongs to the buffer canvas, so the default seat for obj does not
        // match the window win.
        Eo *top = elm_widget_top_get(obj);
        if (efl_isa(top, EFL_UI_WIN_INLINED_CLASS))
          {
             parent2 = efl_ui_win_inlined_parent_get(top);
             if (parent2) obj = elm_widget_top_get(parent2) ?: parent2;
          }
        /* fake win means canvas seat id will not match protocol seat id */
        ewin = elm_win_get(obj);
        if (elm_win_type_get(ewin) == ELM_WIN_FAKE) obj = NULL;
     }

   if (!obj)
     {
        Eina_Iterator *it;
        it = ecore_wl2_display_inputs_get(ecore_wl2_window_display_get(win));
        EINA_ITERATOR_FOREACH(it, input) break;
        eina_iterator_free(it);
        return input;
     }

   seat = evas_default_device_get(evas_object_evas_get(obj), EFL_INPUT_DEVICE_TYPE_SEAT);
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);
   return ecore_wl2_display_input_find(ecore_wl2_window_display_get(win),
     evas_device_seat_id_get(seat));
}

static Ecore_Wl2_Window *
_wl_window_get(const Evas_Object *obj)
{
   Evas_Object *top;
   Ecore_Wl2_Window *win = NULL;

   if (elm_widget_is(obj))
     {
        top = elm_widget_top_get(obj);
        if (!top) top = elm_widget_top_get(elm_widget_parent_widget_get(obj));
        if (top && (efl_isa(top, EFL_UI_WIN_CLASS)))
            win = elm_win_wl_window_get(top);
     }
   if (!win)
     {
        Ecore_Evas *ee;
        Evas *evas;
        const char *engine_name;

        if (!(evas = evas_object_evas_get(obj)))
          return NULL;
        if (!(ee = ecore_evas_ecore_evas_get(evas)))
          return NULL;

        engine_name = ecore_evas_engine_name_get(ee);
        if (!strcmp(engine_name, ELM_BUFFER))
          {
             ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
             if (!ee) return NULL;
             engine_name = ecore_evas_engine_name_get(ee);
          }
        if (!strncmp(engine_name, "wayland", sizeof("wayland") - 1))
          {
             /* In case the engine is not a buffer, we want to check once. */
             win = ecore_evas_wayland2_window_get(ee);
             if (!win) return NULL;
          }
     }

   return win;
}

static void
_wl_selection_receive_timeout(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Sel_Manager_Selection *sel = data;

   if (sel->request_obj != obj) return;

   ecore_event_handler_del(sel->offer_handler);
}

static Eina_Bool
_wl_selection_receive(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Offer_Data_Ready *ev = event;
   Sel_Manager_Selection *sel = data;

   if (sel->sel_offer != ev->offer) return ECORE_CALLBACK_PASS_ON;

   if (sel->data_func)
     {
        Efl_Selection_Data sel_data;

        sel_data.pos.x = sel_data.pos.y = 0;
        if (((sel->format & EFL_SELECTION_FORMAT_MARKUP) ||
             (sel->format & EFL_SELECTION_FORMAT_HTML)) &&
            (sel->want_format == EFL_SELECTION_FORMAT_TEXT))
          {
             char *tmp = malloc(ev->len + 1);
             sel_data.format = sel->format;
             sel_data.content.mem = NULL;
             sel_data.content.len = 0;
             if (tmp)
               {
                  sel_data.format = sel->want_format;
                  strncpy(tmp, ev->data, ev->len);
                  tmp[ev->len] = 0;
                  sel_data.content.mem = _elm_util_mkup_to_text(tmp);
                  if (sel_data.content.mem)
                    sel_data.content.len = strlen(sel_data.content.mem);
                  free(tmp);
               }
          }
        else
          {
             sel_data.format = sel->format;
             sel_data.content.mem = ev->data;
             sel_data.content.len = ev->len;
          }
        sel_data.action = _wl_to_elm(ecore_wl2_offer_action_get(sel->sel_offer));
        sel->data_func(sel->data_func_data,
                       sel->request_obj,
                       &sel_data);
     }
   else
     {
        char *stripstr, *mkupstr;

        stripstr = malloc(ev->len + 1);
        if (!stripstr) return ECORE_CALLBACK_CANCEL;
        strncpy(stripstr, (char *)ev->data, ev->len);
        stripstr[ev->len] = '\0';
        mkupstr = _elm_util_text_to_mkup((const char *)stripstr);
        /* TODO BUG: should never NEVER assume it's an elm_entry! */
        _elm_entry_entry_paste(sel->request_obj, mkupstr);
        free(stripstr);
        free(mkupstr);
     }

   evas_object_event_callback_del_full(sel->request_obj,
                                        EVAS_CALLBACK_DEL,
                                        _wl_selection_receive_timeout, sel);

   ecore_event_handler_del(sel->offer_handler);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_wl_efl_sel_manager_selection_get(const Efl_Object *request, Efl_Selection_Manager_Data *pd,
                                  Efl_Selection_Type type, Efl_Selection_Format format,
                                  void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb,
                                  Ecore_Wl2_Window *win, unsigned int seat)
{
   sel_debug("In, format: %d", format);
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Offer *offer;
   int i = 0;

   if (type == EFL_SELECTION_TYPE_DND) return EINA_FALSE;

   //if (sel->active)
     //return _local_elm_cnp_selection_get(obj, selection, format, datacb, udata);
   seat_sel = _sel_manager_seat_selection_init(pd, seat);
   sel = seat_sel->sel;
   sel->request_obj = (Efl_Object *)request;
   sel->data_func_data = data_func_data;
   sel->data_func = data_func;
   sel->data_func_free_cb = data_func_free_cb;

   input = _wl_seat_get(win, (Efl_Object *)request, seat_sel->seat);
   offer = ecore_wl2_dnd_selection_get(input);

   //there can be no selection available
   if (!offer) return EINA_FALSE;

   for (i = 0; sm_wl_convertion[i].translates; i++)
     {
       int j = 0;
//       if (!(format & sm_wl_convertion[i].format)) continue;

       for (j = 0; sm_wl_convertion[i].translates[j]; j++)
         {
            if (!ecore_wl2_offer_supports_mime(offer, sm_wl_convertion[i].translates[j])) continue;

            //we have found matching mimetypes
            sel->sel_offer = offer;
            sel->format = sm_wl_convertion[i].format;
            sel->want_format = format;

            sel_debug("request type: %s", (char *)sm_wl_convertion[i].translates[j]);
            evas_object_event_callback_add(sel->request_obj, EVAS_CALLBACK_DEL,
                                           _wl_selection_receive_timeout, sel);
            sel->offer_handler = ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY,
                                                     _wl_selection_receive, sel);

            ecore_wl2_offer_receive(offer, (char*)sm_wl_convertion[i].translates[j]);
            return EINA_TRUE;
         }
     }

   sel_debug("no type match");
   return EINA_FALSE;
}

static void
_wl_sel_obj_del2(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Sel_Manager_Selection *sel = data;
   if (sel->request_obj == obj) sel->request_obj = NULL;
}

static Sel_Manager_Dropable *
_wl_dropable_find(Efl_Selection_Manager_Data *pd, Ecore_Wl2_Window *win)
{
   Eina_List *l;
   Sel_Manager_Dropable *dropable;

   if (!pd->drop_list) return NULL;

   if (!win) return NULL;

   EINA_LIST_FOREACH(pd->drop_list, l, dropable)
     if (_wl_window_get(dropable->obj) == win)
       return dropable;

   return NULL;
}

static Evas *
_wl_evas_get_from_win(Efl_Selection_Manager_Data *pd, Ecore_Wl2_Window *win)
{
   Sel_Manager_Dropable *dropable = _wl_dropable_find(pd, win);
   return dropable ? evas_object_evas_get(dropable->obj) : NULL;
}

static Eina_Bool
_wl_drops_accept(Sel_Manager_Seat_Selection *seat_sel, const char *type)
{
   Efl_Selection_Manager_Data *pd;
   Sel_Manager_Selection *sel;
   Eina_List *l;
   Sel_Manager_Dropable *drop;
   Eina_Bool will_accept = EINA_FALSE;

   if (!type) return EINA_FALSE;

   pd = seat_sel->pd;
   sel = seat_sel->sel;
   EINA_LIST_FOREACH(pd->drop_list, l, drop)
     {
        Drop_Format *df;
        EINA_INLIST_FOREACH(drop->format_list, df)
          {
             for (int i = 0; sm_wl_convertion[i].translates ; ++i)
               {
                  if (!(sm_wl_convertion[i].format & df->format)) continue;

                  for (int j = 0; sm_wl_convertion[i].translates[j]; ++j)
                    {
                       if (!strncmp(type, sm_wl_convertion[i].translates[j], strlen(sm_wl_convertion[i].translates[j])))
                         {
                            sel->request_obj = drop->obj;
                            return EINA_TRUE;
                         }
                    }
               }
          }
     }

   return will_accept;
}

static void
_wl_selection_parser(void *_data, int size, char ***ret_data, int *ret_count)
{
   char **files = NULL;
   int num_files = 0;
   char *data = NULL;

   data = malloc(size);
   if (data && (size > 0))
     {
        int i, is;
        char *tmp;
        char **t2;

        memcpy(data, _data, size);
        if (data[size - 1])
          {
             char *t;

             /* Isn't nul terminated */
             size++;
             t = realloc(data, size);
             if (!t) goto done;
             data = t;
             data[size - 1] = 0;
          }

        tmp = malloc(size);
        if (!tmp) goto done;
        i = 0;
        is = 0;
        while ((is < size) && (data[is]))
          {
             if ((i == 0) && (data[is] == '#'))
               for (; ((data[is]) && (data[is] != '\n')); is++) ;
             else
               {
                  if ((data[is] != '\r') && (data[is] != '\n'))
                    tmp[i++] = data[is++];
                  else
                    {
                       while ((data[is] == '\r') || (data[is] == '\n'))
                         is++;
                       tmp[i] = 0;
                       num_files++;
                       t2 = realloc(files, num_files * sizeof(char *));
                       if (t2)
                         {
                            files = t2;
                            files[num_files - 1] = strdup(tmp);
                         }
                       else
                         {
                            num_files--;
                            goto freetmp;
                         }
                       tmp[0] = 0;
                       i = 0;
                    }
               }
          }
        if (i > 0)
          {
             tmp[i] = 0;
             num_files++;
             t2 = realloc(files, num_files * sizeof(char *));
             if (t2)
               {
                  files = t2;
                  files[num_files - 1] = strdup(tmp);
               }
             else
               {
                  num_files--;
                  goto freetmp;
               }
          }
freetmp:
        free(tmp);
     }
done:
   free(data);
   if (ret_data) *ret_data = files;
   else
     {
        int i;

        for (i = 0; i < num_files; i++) free(files[i]);
        free(files);
     }
   if (ret_count) *ret_count = num_files;
}

static Eina_Bool
_wl_data_preparer_markup(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("In\n");

   ddata->format = EFL_SELECTION_FORMAT_MARKUP;
   ddata->content.mem = eina_memdup((unsigned char *)ev->data, ev->len, EINA_TRUE);
   ddata->content.len = ev->len;
   ddata->action = sel->action;

   return EINA_TRUE;
}

static Eina_Bool
_wl_data_preparer_uri(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info EINA_UNUSED)
{
   Sel_Manager_Seat_Selection *seat_sel;
   char *p, *stripstr = NULL;
   char *data = ev->data;
   Sel_Manager_Dropable *drop;
   const char *type = NULL;

   sel_debug("In\n");

   seat_sel = sel->seat_sel;
   drop = efl_key_data_get(sel->request_obj, "__elm_dropable");
   if (drop) type = drop->last.type;

   if ((type) && (!strcmp(type, "text/uri-list")))
     {
        int num_files = 0;
        char **files = NULL;
        Efreet_Uri *uri;
        Eina_Strbuf *strbuf;
        int i;

        strbuf = eina_strbuf_new();
        if (!strbuf) return EINA_FALSE;

        _wl_selection_parser(ev->data, ev->len, &files, &num_files);
        sel_debug("got a files list\n");

        for (i = 0; i < num_files; i++)
          {
             uri = efreet_uri_decode(files[i]);
             if (uri)
               {
                  eina_strbuf_append(strbuf, uri->path);
                  efreet_uri_free(uri);
               }
             else
               {
                  eina_strbuf_append(strbuf, files[i]);
               }
             if (i < (num_files - 1))
               eina_strbuf_append(strbuf, "\n");
             free(files[i]);
          }
        free(files);
        stripstr = eina_strbuf_string_steal(strbuf);
        eina_strbuf_free(strbuf);
     }
   else
     {
        Efreet_Uri *uri;

        p = (char *)eina_memdup((unsigned char *)data, ev->len, EINA_TRUE);
        if (!p) return EINA_FALSE;
        uri = efreet_uri_decode(p);
        if (!uri)
          {
             /* Is there any reason why we care of URI without scheme? */
             if (p[0] == '/') stripstr = p;
             else free(p);
          }
        else
          {
             free(p);
             stripstr = strdup(uri->path);
             efreet_uri_free(uri);
          }
     }

   if (!stripstr)
     {
        sel_debug("Couldn't find a file\n");
        return EINA_FALSE;
     }
   free(seat_sel->saved_types->imgfile);

   ddata->content.mem = stripstr;
   ddata->content.len = strlen(stripstr);
   ddata->action = sel->action;
   ddata->format = sel->request_format;

   return EINA_TRUE;
}

static Eina_Bool
_wl_data_preparer_vcard(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("In\n");

   ddata->format = EFL_SELECTION_FORMAT_VCARD;
   ddata->content.mem = eina_memdup((unsigned char *)ev->data, ev->len, EINA_TRUE);
   ddata->content.len = ev->len;
   ddata->action = sel->action;

   return EINA_TRUE;
}

static Eina_Bool
_wl_data_preparer_image(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info)
{
   sel_debug("In\n");
   Tmp_Info *tmp;
   int len = 0;

   tmp = _tempfile_new(ev->len);
   if (!tmp)
     return EINA_FALSE;
   memcpy(tmp->map, ev->data, ev->len);
   munmap(tmp->map, ev->len);

   len = strlen(tmp->filename);
   ddata->format = EFL_SELECTION_FORMAT_IMAGE;
   ddata->content.mem = eina_memdup((unsigned char*)tmp->filename, len, EINA_TRUE);
   ddata->content.len = len;
   ddata->action = sel->action;
   *tmp_info = tmp;

   return EINA_TRUE;
}

static Eina_Bool
_wl_data_preparer_text(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("In\n");

   ddata->format = EFL_SELECTION_FORMAT_TEXT;
   ddata->content.mem = eina_memdup((unsigned char *)ev->data, ev->len, EINA_TRUE);
   ddata->content.len = ev->len;
   ddata->action = sel->action;

   return EINA_TRUE;
}


static void
_wl_dropable_handle(Sel_Manager_Seat_Selection *seat_sel, Sel_Manager_Dropable *dropable, Evas_Coord x, Evas_Coord y)
{
   Sel_Manager_Dropable *d, *last_dropable = NULL;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;
   Sel_Manager_Selection *sel;
   Eina_Inlist *itr;
   Eina_List *l;
   Eina_Position2D pos;

   EINA_LIST_FOREACH(pd->drop_list, l, d)
     {
        if (d->last.in)
          {
             last_dropable = d;
             break;
          }
     }

   sel = seat_sel->sel;
   pos = EINA_POSITION2D(x, y);
   /* If we are on the same object, just update the position */
   if ((dropable) && (last_dropable == dropable))
     {
        Evas_Coord ox, oy;
        Efl_Dnd_Drag_Pos pos_data;
        Drop_Format *df;

        evas_object_geometry_get(dropable->obj, &ox, &oy, NULL, NULL);
        if (!dropable->is_container)
          {
             pos_data.pos = EINA_POSITION2D(x - ox, y - oy);
             pos_data.item = NULL;
          }
        else
          {
             Efl_Object *it = NULL;

             if (dropable->item_func)
               it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                        pos, &pos_data.pos);
             pos_data.item = it;
          }
        pos_data.format = dropable->last.format;
        pos_data.action = sel->action;
        EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
          {
             if (df->format & dropable->last.format)
               efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_POS, &pos_data);
          }

        return;
     }
   /* We leave the last dropable */
   if (last_dropable)
     {
        Drop_Format *df;
        sel_debug("leave %p\n", last_dropable->obj);
        last_dropable->last.in = EINA_FALSE;

        EINA_INLIST_FOREACH_SAFE(last_dropable->format_list, itr, df)
          {
             if (df->format & last_dropable->last.format)
               efl_event_callback_call(last_dropable->obj, EFL_UI_DND_EVENT_DRAG_LEAVE, NULL);
          }
     }
   /* We enter the new dropable */
   if (dropable)
     {
        sel_debug("enter %p\n", dropable->obj);
        Evas_Coord ox, oy;
        Efl_Dnd_Drag_Pos pos_data;
        Drop_Format *df;

        dropable->last.in = EINA_TRUE;
        evas_object_geometry_get(dropable->obj, &ox, &oy, NULL, NULL);
        if (!dropable->is_container)
          {
             pos_data.pos = EINA_POSITION2D(x - ox, y - oy);
             pos_data.item = NULL;
          }
        else
          {
             Efl_Object *it = NULL;

             if (dropable->item_func)
               it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                        pos, &pos_data.pos);
             pos_data.item = it;
          }
        pos_data.format = dropable->last.format;
        pos_data.action = sel->action;

        EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
          {
             if (df->format & dropable->last.format)
               {
                  efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_ENTER, NULL);
                  efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_POS, &pos_data);
               }
          }
     }
}

static void
_wl_dropable_all_clean(Sel_Manager_Seat_Selection *seat_sel, Ecore_Wl2_Window *win)
{
   Eina_List *l;
   Sel_Manager_Dropable *dropable;

   if (!win) return;

   EINA_LIST_FOREACH(seat_sel->pd->drop_list, l, dropable)
     {
        if (_wl_window_get(dropable->obj) == win)
          {
             dropable->last.pos.x = 0;
             dropable->last.pos.y = 0;
             dropable->last.in = EINA_FALSE;
          }
     }
}

static void
_wl_dropable_data_handle(Sel_Manager_Selection *sel, Ecore_Wl2_Event_Offer_Data_Ready *ev)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Efl_Selection_Manager_Data *pd;
   Sel_Manager_Dropable *drop;
   Ecore_Wl2_Window *win;

   sel_debug("In\n");
   seat_sel = sel->seat_sel;
   pd = seat_sel->pd;
   drop = efl_key_data_get(sel->request_obj, "__elm_dropable");
   if (drop)
     {
        Sel_Manager_Atom *atom = NULL;

        atom = eina_hash_find(pd->type_hash, drop->last.type);
        if (atom && atom->wl_data_preparer)
          {
             Efl_Selection_Data ddata;
             Tmp_Info *tmp_info = NULL;
             Eina_Bool success;

             sel_debug("Call notify for: %s\n", atom->name);
             success = atom->wl_data_preparer(sel, &ddata, ev, &tmp_info);
             if (success)
               {
                  Sel_Manager_Dropable *dropable;
                  Eina_List *l;

                  EINA_LIST_FOREACH(pd->drop_list, l, dropable)
                    {
                       if (dropable->obj == sel->request_obj) break;
                       dropable = NULL;
                    }
                  if (dropable)
                    {
                       Drop_Format *df;
                       Eina_Inlist *itr;

                       if (!dropable->is_container)
                         {
                            ddata.pos.x = seat_sel->saved_types->pos.x;
                            ddata.pos.y = seat_sel->saved_types->pos.y;
                         }
                       else
                         {
                            //for container
                            Efl_Object *it = NULL;
                            Evas_Coord x0 = 0, y0 = 0;
                            Eina_Position2D pos, posret = {};

                            evas_object_geometry_get(dropable->obj, &x0, &y0, NULL, NULL);
                            pos = EINA_POSITION2D(seat_sel->saved_types->pos.x + x0,
                                                  seat_sel->saved_types->pos.y + y0);
                            if (dropable->item_func)
                              it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                                       pos, &posret);
                            ddata.pos = posret;
                            ddata.item = it;
                         }
                       ddata.action = seat_sel->drag_action;

                       EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
                         {
                            if (df->format & dropable->last.format)
                              efl_event_callback_call(dropable->obj, EFL_UI_DND_EVENT_DRAG_DROP, &ddata);
                         }
                    }
               }
             win = _wl_window_get(sel->request_obj);
             ecore_wl2_dnd_drag_end(_wl_seat_get(win, NULL, seat_sel->seat));
             if (tmp_info) _tmpinfo_free(tmp_info);
             return;
          }
     }

   win = _wl_window_get(sel->request_obj);
   ecore_wl2_dnd_drag_end(_wl_seat_get(win, NULL, seat_sel->seat));
   seat_sel->saved_types->textreq = 0;
}

static Eina_Bool
_wl_dnd_enter(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Dnd_Enter *ev;
   Eina_Array *known, *available;
   Sel_Manager_Seat_Selection *seat_sel = data;
   unsigned int i = 0;

   ev = event;

   available = ecore_wl2_offer_mimes_get(ev->offer);

   free(seat_sel->saved_types->types);

   seat_sel->saved_types->ntypes = eina_array_count(available);
   seat_sel->saved_types->types = malloc(sizeof(char *) * seat_sel->saved_types->ntypes);
   if (!seat_sel->saved_types->types) return EINA_FALSE;

   known = eina_array_new(5);

   for (i = 0; i < eina_array_count(available); i++)
     {
        seat_sel->saved_types->types[i] =
          eina_stringshare_add(eina_array_data_get(available, i));
        if (seat_sel->saved_types->types[i] == seat_sel->pd->text_uri)
          {
             seat_sel->saved_types->textreq = 1;
             ELM_SAFE_FREE(seat_sel->saved_types->imgfile, free);
          }
     }

   seat_sel->accept = EINA_FALSE;
   for (i = 0; i < eina_array_count(available); i++)
     {
        if (_wl_drops_accept(seat_sel, eina_array_data_get(available, i)))
          {
             eina_array_push(known, strdup(eina_array_data_get(available, i)));
          }
     }

   ecore_wl2_offer_mimes_set(ev->offer, known);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_leave(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Dnd_Leave *ev;
   Sel_Manager_Seat_Selection *seat_sel = data;
   Sel_Manager_Dropable *drop;
   sel_debug("In\n");

   ev = event;
   if ((drop = _wl_dropable_find(seat_sel->pd, ev->win)))
     {
        _wl_dropable_handle(seat_sel, NULL, 0, 0);
        _wl_dropable_all_clean(seat_sel, ev->win);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_position(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Dnd_Motion *ev;
   Sel_Manager_Seat_Selection *seat_sel = data;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;
   Sel_Manager_Dropable *drop;
   Eina_Bool will_accept = EINA_FALSE;

   ev = event;

   sel_debug("mouse pos %i %i\n", ev->x, ev->y);
   seat_sel->drag_win_end.x = ev->x - seat_sel->drag_pos.x;
   seat_sel->drag_win_end.y = ev->y - seat_sel->drag_pos.y;

   drop = _wl_dropable_find(pd, ev->win);

   if (drop)
     {
        Eina_Position2D pos = EINA_POSITION2D(ev->x, ev->y);
        Evas *evas = NULL;
        Eina_List *dropable_list = NULL;

        _dropable_coords_adjust(drop, &pos);
        evas = _wl_evas_get_from_win(pd, ev->win);
        if (evas)
          dropable_list = _dropable_list_geom_find(pd, evas, pos.x, pos.y);

        /* check if there is dropable (obj) can accept this drop */
        if (dropable_list)
          {
             Efl_Selection_Format saved_format;
             Eina_List *l;
             Eina_Bool found = EINA_FALSE;
             Sel_Manager_Dropable *dropable = NULL;

             saved_format =
               _dnd_types_to_format(pd, seat_sel->saved_types->types, seat_sel->saved_types->ntypes);

             EINA_LIST_FOREACH(dropable_list, l, dropable)
               {
                  Drop_Format *df;
                  Eina_Inlist *itr;

                  EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
                    {
                       Efl_Selection_Format common_fmt = saved_format & df->format;

                       if (common_fmt)
                         {
                            /* We found a target that can accept this type of data */
                            int i, min_index = SELECTION_N_ATOMS;

                            /* We have to find the first atom that corresponds to one
                             * of the supported data types. */
                            for (i = 0; i < seat_sel->saved_types->ntypes; i++)
                              {
                                 Sel_Manager_Atom *atom;

                                 atom = eina_hash_find(pd->type_hash,
                                                       seat_sel->saved_types->types[i]);

                                 if (atom && (atom->format & common_fmt))
                                   {
                                      int atom_idx = (atom - pd->atom_list);

                                      if (min_index > atom_idx)
                                        min_index = atom_idx;
                                   }
                              }
                            if (min_index != SELECTION_N_ATOMS)
                              {
                                 sel_debug("Found atom %s\n", pd->atom_list[min_index].name);
                                 found = EINA_TRUE;
                                 dropable->last.type = pd->atom_list[min_index].name;
                                 dropable->last.format = common_fmt;
                                 break;
                              }
                         }
                    }
                  if (found) break;
               }
             if (found)
               {
                  Sel_Manager_Selection *sel = seat_sel->sel;
                  Evas_Coord ox = 0, oy = 0;

                  evas_object_geometry_get(dropable->obj, &ox, &oy, NULL, NULL);

                  sel_debug("Candidate %p (%s)\n",
                        dropable->obj, efl_class_name_get(efl_class_get(dropable->obj)));
                  _wl_dropable_handle(seat_sel, dropable, pos.x - ox, pos.y - oy);
                  sel->request_obj = dropable->obj;
                  will_accept = EINA_TRUE;
               }
             else
               {
                  //if not: send false status
                  sel_debug("dnd position (%d, %d) not in obj\n", pos.x, pos.y);
                  _wl_dropable_handle(seat_sel, NULL, 0, 0);
                  // CCCCCCC: call dnd exit on last obj
               }
             eina_list_free(dropable_list);
          }
     }

   seat_sel->accept = will_accept;
   efl_event_callback_call(seat_sel->drag_obj, EFL_UI_DND_EVENT_DRAG_ACCEPT, &seat_sel->accept);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_receive(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Offer_Data_Ready *ev;
   Sel_Manager_Seat_Selection *seat_sel = data;
   Sel_Manager_Selection *sel;
   Ecore_Wl2_Offer *offer;
   sel_debug("In\n");

   ev = event;
   sel = seat_sel->sel;
   offer = sel->dnd_offer;

   if (offer != ev->offer) return ECORE_CALLBACK_PASS_ON;

   if (sel->request_obj)
     {
           Ecore_Wl2_Drag_Action action;

           action = ecore_wl2_offer_action_get(ev->offer);
           if (action == ECORE_WL2_DRAG_ACTION_ASK)
             ecore_wl2_offer_actions_set(ev->offer, ECORE_WL2_DRAG_ACTION_COPY, ECORE_WL2_DRAG_ACTION_COPY);
           action = ecore_wl2_offer_action_get(ev->offer);
           sel->action = _wl_to_elm(action);

           _wl_dropable_data_handle(sel, ev);
           evas_object_event_callback_del_full(sel->request_obj,
                                               EVAS_CALLBACK_DEL,
                                               _wl_sel_obj_del2, sel);
           sel->request_obj = NULL;
     }

   ecore_wl2_offer_finish(ev->offer);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_wl_dnd_drop(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Dnd_Drop *ev;
   Sel_Manager_Seat_Selection *seat_sel = data;
   Efl_Selection_Manager_Data *pd;
   Sel_Manager_Selection *sel;
   Ecore_Wl2_Window *win;
   Sel_Manager_Dropable *drop;
   Eina_List *l;

   sel_debug("In\n");
   ev = event;
   seat_sel->saved_types->pos = EINA_POSITION2D(ev->x, ev->y);
   pd = seat_sel->pd;
   sel = seat_sel->sel;
   sel->dnd_offer = ev->offer;

   EINA_LIST_FOREACH(pd->drop_list, l, drop)
     {
        if (drop->last.in)
          {
             sel_debug("Request data of type %s; drop: %p\n", drop->last.type, drop);
             sel->request_obj = drop->obj;
             sel->request_format = drop->last.format;
             evas_object_event_callback_add(sel->request_obj,
                                            EVAS_CALLBACK_DEL, _wl_sel_obj_del2,
                                            sel);
             win = _wl_window_get(drop->obj);
             ecore_wl2_offer_receive(ev->offer, (char*)drop->last.type);
             ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY, _wl_dnd_receive, seat_sel);

             return ECORE_CALLBACK_PASS_ON;
          }
     }

   ecore_wl2_dnd_drag_end(_wl_seat_get(ev->win, NULL, seat_sel->seat));
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_sel_manager_drop_target_add(Efl_Selection_Manager_Data *pd, Efl_Object *target_obj,
                                Efl_Selection_Format format, unsigned int seat)
{
   Sel_Manager_Dropable *dropable = NULL;
   Sel_Manager_Seat_Selection *seat_sel = NULL;
   Drop_Format *df;

   df = calloc(1, sizeof(Drop_Format));
   if (!df) return EINA_FALSE;
   df->format = format;
   dropable = efl_key_data_get(target_obj, "__elm_dropable");
   if (!dropable)
     {
        //Create new drop
        dropable = calloc(1, sizeof(Sel_Manager_Dropable));
        if (!dropable)
          {
             free(df);
             return EINA_FALSE;
          }
        pd->drop_list = eina_list_append(pd->drop_list, dropable);
        if (!pd->drop_list)
          {
             free(dropable);
             free(df);
             return EINA_FALSE;
          }
        dropable->obj = target_obj;
        efl_key_data_set(target_obj, "__elm_dropable", dropable);
     }

   dropable->format_list = eina_inlist_append(dropable->format_list, EINA_INLIST_GET(df));
   dropable->seat = seat;
   seat_sel = _wl_sel_manager_seat_selection_init(pd, seat);

   evas_object_event_callback_add(target_obj, EVAS_CALLBACK_DEL,
                                  _all_drop_targets_cbs_del, pd);

   if (!seat_sel->enter_handler)
     {
        seat_sel->enter_handler =
          ecore_event_handler_add(ECORE_WL2_EVENT_DND_ENTER,
                                  _wl_dnd_enter, seat_sel);
        seat_sel->leave_handler =
          ecore_event_handler_add(ECORE_WL2_EVENT_DND_LEAVE,
                                  _wl_dnd_leave, seat_sel);
        seat_sel->pos_handler =
          ecore_event_handler_add(ECORE_WL2_EVENT_DND_MOTION,
                                  _wl_dnd_position, seat_sel);
        seat_sel->drop_handler =
          ecore_event_handler_add(ECORE_WL2_EVENT_DND_DROP,
                                  _wl_dnd_drop, seat_sel);
     }

   return EINA_TRUE;
}
#endif

#ifdef HAVE_ELEMENTARY_COCOA
static Sel_Manager_Seat_Selection *
_cocoa_sel_manager_seat_selection_init(Efl_Selection_Manager_Data *pd, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if(seat_sel->seat == seat)
          break;
     }
   if (!seat_sel)
     {
        seat_sel = calloc(1, sizeof(Sel_Manager_Seat_Selection));
        if (!seat_sel)
          {
             ERR("Failed to allocate seat");
             return NULL;
          }
        seat_sel->saved_types = calloc(1, sizeof(Saved_Type));
        seat_sel->seat = seat;
        seat_sel->pd = pd;
        pd->seat_list = eina_list_append(pd->seat_list, seat_sel);
     }
   if (!seat_sel->sel)
     {
        Sel_Manager_Selection *sel = calloc(1, sizeof(Sel_Manager_Selection));
        if (!sel)
          {
             ERR("failed to allocate selection");
             return NULL;
          }
        sel->seat_sel = seat_sel;
        seat_sel->sel = sel;
     }

   return seat_sel;
}

static Ecore_Cocoa_Window *
_cocoa_window_get(const Evas_Object *obj)
{
   Evas_Object *top, *par;
   Ecore_Cocoa_Window *win = NULL;

   if (elm_widget_is(obj))
     {
         top = elm_widget_top_get(obj);
         if (!top)
           {
              par = elm_widget_parent_widget_get(obj);
              if (par) top = elm_widget_top_get(par);
           }
         if ((top) && (efl_isa(top, EFL_UI_WIN_CLASS)))
           win = elm_win_cocoa_window_get(top);
     }
   if (!win)
     {
        // FIXME
        CRI("WIN has not been retrieved!!!");
     }

   return win;
}

static Ecore_Cocoa_Cnp_Type
_sel_format_to_ecore_cocoa_cnp_type(Efl_Selection_Format fmt)
{
   Ecore_Cocoa_Cnp_Type type = 0;

   if ((fmt & EFL_SELECTION_FORMAT_TEXT) ||
       (fmt & EFL_SELECTION_FORMAT_VCARD))
     type |= ECORE_COCOA_CNP_TYPE_STRING;
   if (fmt & EFL_SELECTION_FORMAT_MARKUP)
     type |= ECORE_COCOA_CNP_TYPE_MARKUP;
   if (fmt & EFL_SELECTION_FORMAT_HTML)
     type |= ECORE_COCOA_CNP_TYPE_HTML;
   if (fmt & EFL_SELECTION_FORMAT_IMAGE)
     type |= ECORE_COCOA_CNP_TYPE_IMAGE;

   return type;
}

static void
_cocoa_sel_obj_del_req_cb(void        *data,
                          Evas        *e       EINA_UNUSED,
                          Evas_Object *obj,
                          void        *ev_info EINA_UNUSED)
{
   Sel_Manager_Selection *sel = data;
   if (sel->request_obj == obj) sel->request_obj = NULL;
}

static void
_cocoa_sel_obj_del_cb(void        *data,
                      Evas        *e       EINA_UNUSED,
                      Evas_Object *obj,
                      void        *ev_info EINA_UNUSED)
{
   Sel_Manager_Selection *sel = data;
   if (sel->owner == obj)
     {
        sel->owner = NULL;
     }
   //if (dragwidget == obj) dragwidget = NULL;
}

static void
_job_pb_cb(void *data)
{
   Sel_Manager_Selection *sel = data;
   Efl_Selection_Data ddata;
   Ecore_Cocoa_Cnp_Type type, get_type;
   void *pbdata;
   int pbdata_len;

   if (sel->data_func)
     {
        ddata.pos.x = 0;
        ddata.pos.y = 0;

        /* Pass to cocoa clipboard */
        type = _sel_format_to_ecore_cocoa_cnp_type(sel->request_format);
        pbdata = ecore_cocoa_clipboard_get(&pbdata_len, type, &get_type);

        ddata.format = EFL_SELECTION_FORMAT_NONE;
        if (get_type & ECORE_COCOA_CNP_TYPE_STRING)
          ddata.format |= EFL_SELECTION_FORMAT_TEXT;
        if (get_type & ECORE_COCOA_CNP_TYPE_MARKUP)
          ddata.format |= EFL_SELECTION_FORMAT_MARKUP;
        if (get_type & ECORE_COCOA_CNP_TYPE_IMAGE)
          ddata.format |= EFL_SELECTION_FORMAT_IMAGE;
        if (get_type & ECORE_COCOA_CNP_TYPE_HTML)
          ddata.format |= EFL_SELECTION_FORMAT_HTML;

        ddata.content.mem = pbdata;
        ddata.content.len = pbdata_len;
        ddata.action = EFL_SELECTION_ACTION_UNKNOWN;
        sel->data_func(sel->data_func_data, sel->request_obj, &ddata);
        free(pbdata);
     }
}

static Eina_Future *
_cocoa_efl_sel_manager_selection_set(Efl_Selection_Manager_Data *pd,
                                     Evas_Object         *owner,
                                     Efl_Selection_Type   type,
                                     Efl_Selection_Format format,
                                     Eina_Slice data,
                                     Ecore_Cocoa_Window *win,
                                     unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;
   Ecore_Cocoa_Cnp_Type ecore_type;
   Eina_List *l, *l_next;
   Sel_Manager_Selection_Lost *sel_lost;

   seat_sel = _cocoa_sel_manager_seat_selection_init(pd, seat);
   seat_sel->active_type = type;
   sel = seat_sel->sel;
   if ((!data.mem) && (format != EFL_SELECTION_FORMAT_IMAGE))
     {
        efl_selection_manager_selection_clear(pd->sel_man, owner, type, seat);
        return NULL;
     }
   if (data.len <= 0) return NULL;

   EINA_LIST_FOREACH_SAFE(seat_sel->sel_lost_list, l, l_next, sel_lost)
     {
        if ((sel_lost->request == sel->owner) &&
            (sel_lost->type == type))
          {
             eina_promise_resolve(sel_lost->promise, eina_value_uint_init(sel_lost->type));
             seat_sel->sel_lost_list = eina_list_remove(seat_sel->sel_lost_list, sel_lost);
             free(sel_lost);
          }
     }
   if (sel->owner)
     evas_object_event_callback_del_full(sel->owner, EVAS_CALLBACK_DEL,
                                         _cocoa_sel_obj_del_cb, sel);

   sel->owner = owner;
   sel->win = win;
   sel->format = format;

   evas_object_event_callback_add(sel->owner, EVAS_CALLBACK_DEL,
                                  _cocoa_sel_obj_del_cb, sel);
   ELM_SAFE_FREE(sel->data.mem, free);
   sel->data = eina_slice_dup(data);
   if (sel->data.mem)
     {
        ecore_type = _sel_format_to_ecore_cocoa_cnp_type(format);
        ecore_cocoa_clipboard_set(sel->data.mem, sel->data.len, ecore_type);
     }
   else
     {
        CRI("Failed to allocate memory!");
        efl_selection_manager_selection_clear(pd->sel_man, owner, type, seat_sel->seat);
        return NULL;
     }

   return _update_sel_lost_list(owner, type, seat_sel);
}

static void
_cocoa_efl_sel_manager_selection_get(const Efl_Object  *request,
                                     Efl_Selection_Manager_Data *pd,
                                     Efl_Selection_Type        type EINA_UNUSED,
                                     Efl_Selection_Format      format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb,
                                     Ecore_Cocoa_Window *win, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;

   seat_sel = _sel_manager_seat_selection_init(pd, seat);
   sel = seat_sel->sel;
   sel->request_format = format;
   sel->request_obj = (Efl_Object *)request;
   sel->data_func_data = data_func_data;
   sel->data_func = data_func;
   sel->data_func_free_cb = data_func_free_cb;
   if (sel->request_obj)
     evas_object_event_callback_del_full(sel->request_obj, EVAS_CALLBACK_DEL,
                                         _cocoa_sel_obj_del_req_cb, sel);

   sel->win = win;
   ecore_job_add(_job_pb_cb, sel);

   evas_object_event_callback_add(sel->request_obj, EVAS_CALLBACK_DEL,
                                  _cocoa_sel_obj_del_req_cb, sel);
}

#endif

// win32 specific stuff
////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_ELEMENTARY_WIN32
static Sel_Manager_Seat_Selection *
_win32_sel_manager_seat_selection_init(Efl_Selection_Manager_Data *pd, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if(seat_sel->seat == seat)
          break;
     }
   if (!seat_sel)
     {
        seat_sel = calloc(1, sizeof(Sel_Manager_Seat_Selection));
        if (!seat_sel)
          {
             ERR("Failed to allocate seat");
             return NULL;
          }
        seat_sel->saved_types = calloc(1, sizeof(Saved_Type));
        seat_sel->seat = seat;
        seat_sel->pd = pd;
        pd->seat_list = eina_list_append(pd->seat_list, seat_sel);
     }
   if (!seat_sel->sel_list)
     {
        seat_sel->sel_list = calloc(1, (EFL_SELECTION_TYPE_CLIPBOARD + 1) * sizeof(Sel_Manager_Selection));
        if (!seat_sel->sel_list)
          {
             ERR("failed to allocate selection list");
             return NULL;
          }
        _set_selection_list(seat_sel->sel_list, seat_sel);
     }

   return seat_sel;
}

static char *
_win32_text_n_to_rn(char *intext)
{
   size_t size = 0, newlines = 0;
   char *outtext = NULL, *p, *o;

   if (!intext) return NULL;
   for (p = intext; *p; p++)
     {
        if (*p == '\n') newlines++;
        size++;
     }
   outtext = malloc(size + newlines + 1);
   if (!outtext) return intext;
   for (p = intext, o = outtext; *p; p++, o++)
     {
        if (*p == '\n')
          {
             o++;
             *p = '\r';
          }
        *o = *p;
     }
   *o = '\0';
   free(intext);
   return outtext;
}

static void
_win32_sel_obj_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Sel_Manager_Selection *sel = data;
   if (sel->owner == obj) sel->owner = NULL;
}

static void
_win32_sel_obj_del2(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Sel_Manager_Selection *sel = data;
   if (sel->request_obj == obj) sel->request_obj = NULL;
}

static Ecore_Win32_Window *
_win32_window_get(const Evas_Object *obj)
{
   Evas_Object *top;
   Ecore_Win32_Window *win = NULL;

   if (elm_widget_is(obj))
     {
        top = elm_widget_top_get(obj);
        if (!top)
          {
             Evas_Object *par;
             par = elm_widget_parent_widget_get(obj);
             if (par) top = elm_widget_top_get(par);
          }
        if (top && (efl_isa(top, EFL_UI_WIN_CLASS)))
          win = elm_win_win32_window_get(top);
     }

   if (!win)
     {
        Ecore_Evas *ee;
        Evas *evas;
        const char *engine_name;

        evas = evas_object_evas_get(obj);
        if (!evas) return NULL;

        ee = ecore_evas_ecore_evas_get(evas);
        if (!ee) return NULL;

        engine_name = ecore_evas_engine_name_get(ee);
        if (!strcmp(engine_name, ELM_BUFFER))
          {
             ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
             if (!ee) return NULL;
             win = ecore_evas_win32_window_get(ee);
          }
        else
          {
             if ((strcmp(engine_name, ELM_SOFTWARE_WIN32) == 0) ||
                 (strcmp(engine_name, ELM_SOFTWARE_DDRAW) == 0))
               return ecore_evas_win32_window_get(ee);
          }
     }

   return win;
}

static Eina_Future *
_win32_efl_sel_manager_selection_set(Efl_Selection_Manager_Data *pd,
                                     Evas_Object *owner,
                                     Efl_Selection_Type type,
                                     Efl_Selection_Format format,
                                     Eina_Slice data,
                                     Ecore_Win32_Window *win,
                                     unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;

   if (type != EFL_SELECTION_TYPE_CLIPBOARD)
     return NULL;

   if ((!data.mem) && (format != EFL_SELECTION_FORMAT_IMAGE))
     {
        efl_selection_manager_selection_clear(pd->sel_man, owner, type, seat);
        return NULL;
     }

   seat_sel = _win32_sel_manager_seat_selection_init(pd, seat);
   seat_sel->active_type = type;
   sel = seat_sel->sel_list + type;
   if (sel->owner != owner)
     {
        Eina_List *l, *l_next;
        Sel_Manager_Selection_Lost *sel_lost;

        EINA_LIST_FOREACH_SAFE(seat_sel->sel_lost_list, l, l_next, sel_lost)
          {
             if ((sel_lost->request == sel->owner) &&
                 (sel_lost->type == type))
               {
                  eina_promise_resolve(sel_lost->promise, eina_value_uint_init(sel_lost->type));
                  seat_sel->sel_lost_list = eina_list_remove(seat_sel->sel_lost_list, sel_lost);
                  free(sel_lost);
               }
          }

     }
   if (sel->owner)
     evas_object_event_callback_del_full(sel->owner, EVAS_CALLBACK_DEL,
                                         _win32_sel_obj_del, sel);
   sel->active = EINA_TRUE;
   sel->owner = owner;
   sel->win = win;
   if (sel->set) sel->set(win, sel->data.mem, sel->data.len);
   sel->format = format;

   evas_object_event_callback_add
     (sel->owner, EVAS_CALLBACK_DEL, _win32_sel_obj_del, sel);

   ELM_SAFE_FREE(sel->data.mem, free);
   sel->data = eina_slice_dup(data);
   if (!sel->data.mem)
     {
        efl_selection_manager_selection_clear(pd->sel_man, owner, type, seat_sel->seat);
        return NULL;
     }

   return _update_sel_lost_list(owner, type, seat_sel);
}

static void
_win32_efl_sel_manager_selection_clear(Efl_Selection_Manager_Data *pd,
                                       Evas_Object *owner,
                                       Efl_Selection_Type type,
                                       Sel_Manager_Seat_Selection *seat_sel)
{
   Sel_Manager_Selection *sel;
   Ecore_Win32_Window *win;

   if (type != EFL_SELECTION_TYPE_CLIPBOARD)
     return;

   sel = seat_sel->sel_list + type;

   /* No longer this selection: Consider it gone! */
   if ((!sel->active) || (sel->owner != owner))
     return;

   if (sel->owner)
     evas_object_event_callback_del_full(sel->owner, EVAS_CALLBACK_DEL,
                                         _win32_sel_obj_del, sel);
   if (sel->request_obj)
     evas_object_event_callback_del_full(sel->request_obj, EVAS_CALLBACK_DEL,
                                         _win32_sel_obj_del2, sel);
   sel->owner = NULL;
   sel->request_obj = NULL;
   sel->active = EINA_FALSE;
   ELM_SAFE_FREE(sel->data.mem, free);
   /* sel->clear(win); */
}

static Eina_Bool
_win32_efl_sel_manager_selection_get(const Efl_Object *request,
                                     Efl_Selection_Manager_Data *pd,
                                     Efl_Selection_Type type, Efl_Selection_Format format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func,
                                     Eina_Free_Cb data_func_free_cb,
                                     Ecore_Win32_Window *win, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel;
   void *data;
   int size;

   if (type != EFL_SELECTION_TYPE_CLIPBOARD)
     return EINA_FALSE;

   seat_sel = _sel_manager_seat_selection_init(pd, seat);
   sel = seat_sel->sel_list + type;

   if (sel->request_obj)
     evas_object_event_callback_del_full(sel->request_obj, EVAS_CALLBACK_DEL,
                                         _win32_sel_obj_del2, sel);
   sel->request_format = format;
   sel->request_obj = (Evas_Object *)request;
   sel->win = win;
   sel->data_func_data = data_func_data;
   sel->data_func = data_func;
   sel->data_func_free_cb = data_func_free_cb;
   sel->get(win, &data, &size);

   if (!data || (size <= 0))
     goto cb_add;

   if ((sel->format & EFL_SELECTION_FORMAT_MARKUP) ||
       (sel->format & EFL_SELECTION_FORMAT_HTML))
     {
        char *str;

        str = (char *)malloc(size + 1);
        if (str)
          {
             memcpy(str, data, size);
             str[size] = '\0';
             data = _win32_text_n_to_rn(_elm_util_mkup_to_text(str));
             free(str);
             if (data)
               size = strlen(data);
             else
               size = 0;
          }
        else
          {
             free(data);
             data = NULL;
          }
     }

   if (sel->data_func && data && (size > 0))
     {
        Efl_Selection_Data sdata;

        sdata.pos.x = sdata.pos.y = 0;
        sdata.format = EFL_SELECTION_FORMAT_TEXT;
        sdata.content.mem = data;
        sdata.content.len = size;
        sdata.action = sel->action;
        sel->data_func(sel->data_func_data, sel->request_obj, &sdata);
     }

   if (data)
     free(data);

 cb_add:
   evas_object_event_callback_add
     (sel->request_obj, EVAS_CALLBACK_DEL, _win32_sel_obj_del2, sel);

   return EINA_TRUE;
}

#endif /* HAVE_ELEMENTARY_WIN32 */


static int
_drop_item_container_cmp(const void *d1, const void *d2)
{
   const Item_Container_Drop_Info *di = d1;
   return (((uintptr_t)di->obj) - ((uintptr_t)d2));
}

static Eina_Bool
_drop_item_container_del(Efl_Selection_Manager_Data *pd, Efl_Object *cont, Eina_Bool full)
{
   Item_Container_Drop_Info *di;

   di = eina_list_search_unsorted(pd->drop_cont_list,
                                  _drop_item_container_cmp, cont);
   if (di)
     {
        _all_drop_targets_cbs_del(pd, NULL, cont, NULL);
        di->item_func_data = NULL;
        di->item_func = NULL;

        if (full)
          {
             pd->drop_cont_list = eina_list_remove(pd->drop_cont_list, di);
             free(di);
          }
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static inline Eina_List *
_anim_icons_make(Sel_Manager_Drag_Container *dc)
{
   Eina_List *list = NULL, *icon_list = NULL;
   Evas_Object *obj;

   if (dc->icon_list_func)
     {
        ERR("calling icon_list_func");
        icon_list = dc->icon_list_func(dc->icon_list_func_data, dc->cont);
     }
   EINA_LIST_FREE(icon_list, obj)
     {
        ERR("one obj in icon_list");
        Anim_Icon *ai = calloc(1, sizeof(Anim_Icon));
        if (!ai)
          {
             ERR("Failed to allocate for icon!");
             continue;
          }

        evas_object_geometry_get(obj, &ai->start.x, &ai->start.y, &ai->start.w, &ai->start.h);
        evas_object_show(obj);
        ai->obj = obj;
        list = eina_list_append(list, ai);
     }
   ERR("made icon list");

   return list;
}

static void
_cont_obj_drag_done_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Sel_Manager_Drag_Container *dc = data;
   elm_widget_scroll_freeze_pop(dc->cont);
}

static Eina_Bool
_cont_obj_drag_start(void *data)
{
   ERR("going to start draging");
   Sel_Manager_Drag_Container *dc = data;

   dc->timer = NULL;
   efl_event_callback_add(dc->cont, EFL_UI_DND_EVENT_DRAG_DONE, _cont_obj_drag_done_cb, dc);
   elm_widget_scroll_freeze_push(dc->cont);
   efl_selection_manager_drag_start(dc->pd->sel_man, dc->cont, dc->format,
                                    eina_rw_slice_slice_get(dc->data), dc->action,
                                    dc->icon_func_data, dc->icon_func, dc->icon_func_free_cb,
                                    dc->seat);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_drag_anim_play(void *data, double pos)
{
   Sel_Manager_Drag_Container *dc = data;

   if (dc->animator)
     {
        Eina_List *l;
        Anim_Icon *ai;
        Evas_Coord xm, ym;

        if (pos > 0.99)
          {
             dc->animator = NULL;
             EINA_LIST_FOREACH(dc->icons, l, ai)
                evas_object_hide(ai->obj);

             _cont_obj_drag_start(dc);
             return ECORE_CALLBACK_CANCEL;
          }

        evas_pointer_canvas_xy_get(dc->e, &xm, &ym);
        EINA_LIST_FOREACH(dc->icons, l, ai)
          {
             int x, y, w, h;
             w = ai->start.w - ((dc->final_icon.w - ai->start.w) * pos);
             h = ai->start.h - ((dc->final_icon.h - ai->start.h) * pos);
             x = ai->start.x - (pos * (ai->start.x + (w / 2) - xm));
             y = ai->start.y - (pos * (ai->start.y + (h / 2) - ym));
             evas_object_move(ai->obj, x, y);
             evas_object_resize(ai->obj, w, h);
          }

        return ECORE_CALLBACK_RENEW;
     }

   return ECORE_CALLBACK_CANCEL;
}

static inline void
_drag_anim_start(Sel_Manager_Drag_Container *dc)
{

   dc->timer = NULL;
   if (dc->icon_func)
     {
        Eina_Position2D pos_ret;
        Evas_Object *temp_win = elm_win_add(NULL, "Temp", ELM_WIN_DND);
        Evas_Object *final_icon = dc->icon_func(dc->icon_func_data, temp_win, dc->cont, &pos_ret);
        evas_object_geometry_get(final_icon, NULL, NULL, &dc->final_icon.w, &dc->final_icon.h);
        evas_object_del(final_icon);
        evas_object_del(temp_win);
     }
   dc->animator = ecore_animator_timeline_add(dc->anim_duration, _drag_anim_play, dc);
}

static Eina_Bool
_cont_obj_anim_start(void *data)
{
   sel_debug("In");
   Sel_Manager_Drag_Container *dc = data;
   Efl_Object *it = NULL;
   Eina_Position2D posret; //does not use

   if (dc->item_get_func)
       it = dc->item_get_func(dc->item_get_func_data, dc->cont, dc->down, &posret);
   dc->timer = NULL;
   dc->format = EFL_SELECTION_FORMAT_TARGETS; //default
   dc->data.len = 0;
   dc->action = EFL_SELECTION_ACTION_COPY; //default
   dc->icons = NULL;

   //failed to get mouse-down item, abort drag
   if (!it)
     return ECORE_CALLBACK_CANCEL;

   if (dc->drag_data_func)
     {
        dc->drag_data_func(dc->drag_data_func_data, dc->cont,
                           &dc->format, &dc->data, &dc->action);
        if (EINA_DBL_EQ(dc->anim_duration, 0.0))
          {
             _cont_obj_drag_start(dc);
          }
        else
          {
             dc->icons = _anim_icons_make(dc);
             if (dc->icons)
               {
                  _drag_anim_start(dc);
               }
             else
               {
                  // even if we don't manage the icons animation, we have
                  // to wait until it is finished before beginning drag.
                  dc->timer = ecore_timer_add(dc->anim_duration,
                                              _cont_obj_drag_start, dc);
               }
          }
     }

   return ECORE_CALLBACK_CANCEL;
}

static void
_abort_drag(Evas_Object *obj EINA_UNUSED, Sel_Manager_Drag_Container *dc)
{
   evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_MOVE,
                                       _cont_obj_mouse_move_cb, dc);
   evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_UP,
                                       _cont_obj_mouse_up_cb, dc);
   _item_container_del_internal(dc, EINA_FALSE);

   ELM_SAFE_FREE(dc->timer, ecore_timer_del);
   _anim_data_free(dc);
}

static void
_cont_obj_mouse_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Sel_Manager_Drag_Container *dc = data;
   Evas_Event_Mouse_Move *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        _abort_drag(obj, dc);
     }
   if (dc &&
       (evas_device_class_get(ev->dev) == EVAS_DEVICE_CLASS_TOUCH))
     {
        int dx, dy;
        int fs = elm_config_finger_size_get();

        dx = ev->cur.canvas.x - dc->down.x;
        dy = ev->cur.canvas.y - dc->down.y;
        if ((dx * dx + dy * dy) > (fs * fs))
          {
             sel_debug("mouse moved too much - have to cancel DnD");
             _abort_drag(obj, dc);
          }
     }
}

static void
_anim_data_free(Sel_Manager_Drag_Container *dc)
{
   if (dc)
     {
        ELM_SAFE_FREE(dc->animator, ecore_animator_del);
        Anim_Icon *ai;

        EINA_LIST_FREE(dc->icons, ai)
          {
             evas_object_del(ai->obj);
             free(ai);
          }
        dc->icons = NULL;
     }
}

static void
_cont_obj_mouse_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Sel_Manager_Drag_Container *dc = data;

   if (((Evas_Event_Mouse_Up *)event_info)->button != 1)
     return;

   evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_MOVE,
                                       _cont_obj_mouse_move_cb, dc);
   evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_UP,
                                       _cont_obj_mouse_up_cb, dc);
   ELM_SAFE_FREE(dc->timer, ecore_timer_del);

   _anim_data_free(dc);
}

static void
_cont_obj_mouse_down_cb(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Sel_Manager_Drag_Container *dc = data;
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->button != 1)
     return;

   dc->e = e;
   dc->down.x = ev->canvas.x;
   dc->down.y = ev->canvas.y;

   evas_object_event_callback_add(dc->cont, EVAS_CALLBACK_MOUSE_UP,
                                  _cont_obj_mouse_up_cb, dc);
   ecore_timer_del(dc->timer);
   if (dc->time_to_drag)
     {
        dc->timer = ecore_timer_add(dc->time_to_drag, _cont_obj_anim_start, dc);
        evas_object_event_callback_add(dc->cont, EVAS_CALLBACK_MOUSE_MOVE,
                                       _cont_obj_mouse_move_cb, dc);
     }
   else
     {
        _cont_obj_anim_start(dc);
     }
}

static void
_item_container_del_internal(Sel_Manager_Drag_Container *dc, Eina_Bool full)
{
   if (dc)
     {
        ELM_SAFE_FREE(dc->timer, ecore_timer_del);
        if (dc->animator)
          _anim_data_free(dc);
        evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_DOWN,
                                            _cont_obj_mouse_down_cb, dc);
        if (full)
          {
             dc->item_get_func = NULL;
             dc->item_get_func_data = NULL;
             free(dc);
          }
     }
}

static int
_drag_item_container_cmp(const void *d1, const void *d2)
{
   const Sel_Manager_Drag_Container *dc = d1;
   return (((uintptr_t)dc->cont) - ((uintptr_t)d2));
}
//exposed APIs
EOLIAN static Eina_Future *
_efl_selection_manager_selection_set(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                     Efl_Object *owner, Efl_Selection_Type type,
                                     Efl_Selection_Format format,
                                     Eina_Slice data, unsigned int seat)
{
   Eina_Future *p = NULL;

   sel_debug("owner: %p, seat: %d, type: %d, format: %d", owner, seat, type, format);
   if (type > EFL_SELECTION_TYPE_CLIPBOARD)
     {
        ERR("Not supported format: %d", type);
        return NULL;
     }

#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_xwin_get(owner);
   if (xwin)
     p = _x11_efl_sel_manager_selection_set(pd, owner, type, format, data, xwin, seat);
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win = _wl_window_get(owner);
   if (win)
     p = _wl_efl_sel_manager_selection_set(pd, owner, type, format, data, win, seat);
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   Ecore_Cocoa_Window *win = _cocoa_window_get(owner);
   if (win)
     p = _cocoa_efl_sel_manager_selection_set(pd, owner, type, format, data, win, seat);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   Ecore_Win32_Window *win = _win32_window_get(owner);
   if (win)
     p = _win32_efl_sel_manager_selection_set(pd, owner, type, format, data, win, seat);
#endif

   return p;
}

//TODO: add support for local
EOLIAN static void
_efl_selection_manager_selection_get(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                     const Efl_Object *request, Efl_Selection_Type type,
                                     Efl_Selection_Format format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func,
                                     Eina_Free_Cb data_func_free_cb, unsigned int seat)
{
   sel_debug("request: %p, seat: %d, type: %d, format: %d", request, seat, type, format);
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_xwin_get(request);
   if (xwin)
     _x11_efl_sel_manager_selection_get(request, pd, type, format, data_func_data,
                                        data_func, data_func_free_cb, xwin, seat);
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win = _wl_window_get(request);
   if (win)
     _wl_efl_sel_manager_selection_get(request, pd, type, format, data_func_data,
                                       data_func, data_func_free_cb, win, seat);
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   Ecore_Cocoa_Window *win = _cocoa_window_get(request);
   if (win)
     _cocoa_efl_sel_manager_selection_get(request, pd, type, format, data_func_data,
                                          data_func, data_func_free_cb, win, seat);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   Ecore_Win32_Window *win = _win32_window_get(request);
   if (win)
     _win32_efl_sel_manager_selection_get(request, pd, type, format, data_func_data,
                                          data_func, data_func_free_cb, win, seat);
#endif
}

EOLIAN static void
_efl_selection_manager_selection_clear(Eo *obj, Efl_Selection_Manager_Data *pd,
                                       Efl_Object *owner, Efl_Selection_Type type, unsigned int seat)
{
   Eina_Bool local = EINA_FALSE;
   Sel_Manager_Seat_Selection *seat_sel;
   Sel_Manager_Selection *sel = NULL;

   sel_debug("owner: %p, seat: %d, type: %d", owner, seat, type);
   seat_sel = _sel_manager_seat_selection_init(pd, seat);
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_xwin_get(obj);
   if (xwin)
     {
        sel = seat_sel->sel_list + type;
     }
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win = _wl_window_get(obj);
   if (win)
     {
        sel = seat_sel->sel;
     }
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   Ecore_Cocoa_Window *win = _cocoa_window_get(obj);
   if (win)
     {
        sel = seat_sel->sel;
     }
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   Ecore_Win32_Window *win = _win32_window_get(obj);
     {
        sel = seat_sel->sel_list + type;
     }
#endif
   if (!sel) return;
   if ((!sel->active) && (sel->owner != owner))
     {
        return;
     }
   sel->active = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_X
   if (xwin)
     {
        seat_sel->sel_list[type].data.len = 0;
        if (seat_sel->sel_list[type].data.mem)
          {
             free(seat_sel->sel_list[type].data.mem);
             seat_sel->sel_list[type].data.mem = NULL;
          }
        if (sel->xwin != 0) local = EINA_TRUE;
        if (!local) seat_sel->sel_list[type].clear();
        else
          {
             Eina_List *l, *l_next;
             Sel_Manager_Selection_Lost *sel_lost;

             EINA_LIST_FOREACH_SAFE(seat_sel->sel_lost_list, l, l_next, sel_lost)
               {
                  if ((sel_lost->request == sel->owner) &&
                      (sel_lost->type == type))
                    {
                       eina_promise_resolve(sel_lost->promise, eina_value_uint_init(sel_lost->type));
                       seat_sel->sel_lost_list = eina_list_remove(seat_sel->sel_lost_list, sel_lost);
                       free(sel_lost);
                    }
               }
             seat_sel->sel_list[type].owner = NULL;
          }
     }
#endif
#ifdef HAVE_ELEMENTARY_WL2
   if (win)
     {
        sel->selection_serial = ecore_wl2_dnd_selection_clear(_wl_seat_get(_wl_window_get(owner), owner, seat));
     }
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   if (win)
     {
        if (sel->owner)
          evas_object_event_callback_del_full(sel->owner, EVAS_CALLBACK_DEL,
                                              _cocoa_sel_obj_del_cb, sel);
        if (sel->request_obj)
          evas_object_event_callback_del_full(sel->request_obj, EVAS_CALLBACK_DEL,
                                              _cocoa_sel_obj_del_req_cb, sel);
        sel->owner = NULL;
        sel->request_obj = NULL;
        ELM_SAFE_FREE(sel->data.mem, free);
        sel->data.len = 0;

        ecore_cocoa_clipboard_clear();
     }
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   if (win)
     {
        _win32_efl_sel_manager_selection_clear(pd, owner, type, seat_sel);
     }
#endif
}

EOLIAN static Eina_Bool
_efl_selection_manager_selection_has_owner(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd EINA_UNUSED,
                                           Efl_Object *request, Efl_Selection_Type type,
                                           unsigned int seat)
{
#ifdef HAVE_ELEMENTARY_X
   (void)seat;
   if (_x11_xwin_get(request))
     {
        Ecore_X_Atom xtype;
        switch (type)
          {
           case EFL_SELECTION_TYPE_PRIMARY:
              xtype = ECORE_X_ATOM_SELECTION_PRIMARY;
              break;
           case EFL_SELECTION_TYPE_SECONDARY:
              xtype = ECORE_X_ATOM_SELECTION_SECONDARY;
              break;
           case EFL_SELECTION_TYPE_DND:
              xtype = ECORE_X_ATOM_SELECTION_XDND;
              break;
           default:
              xtype = ECORE_X_ATOM_SELECTION_CLIPBOARD;
          }
        return !!ecore_x_selection_owner_get(xtype);
     }
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win;

   win = _wl_window_get(request);
   if (win)
     return !!ecore_wl2_dnd_selection_get(_wl_seat_get(win, request, seat));
#endif
   return EINA_FALSE;
}

EOLIAN static void
_efl_selection_manager_drag_start(Eo *obj, Efl_Selection_Manager_Data *pd,
                                  Efl_Object *drag_obj, Efl_Selection_Format format,
                                  Eina_Slice data, Efl_Selection_Action action,
                                  void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func,
                                  Eina_Free_Cb icon_func_free_cb, unsigned int seat)
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_xwin_get(drag_obj);
   if (xwin)
     _x11_efl_sel_manager_drag_start(obj, pd, drag_obj, format, data, action,
                                     icon_func_data, icon_func, icon_func_free_cb,
                                     xwin, seat);
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win = _wl_window_get(drag_obj);
   if (win)
     _wl_efl_sel_manager_drag_start(obj, pd, drag_obj, format, data, action,
                                    icon_func_data, icon_func, icon_func_free_cb,
                                    win, seat);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
#endif
#ifdef HAVE_ELEMENTARY_COCOA
#endif
}

EOLIAN static void
_efl_selection_manager_drag_cancel(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                   Efl_Object *drag_obj, unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel = _sel_manager_seat_selection_init(pd, seat);

#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_xwin_get(drag_obj);
   if (xwin)
     {
        ecore_x_pointer_ungrab();
        ELM_SAFE_FREE(seat_sel->mouse_up_handler, ecore_event_handler_del);
        ELM_SAFE_FREE(seat_sel->dnd_status_handler, ecore_event_handler_del);
        ecore_x_dnd_abort(xwin);
        if (seat_sel->drag_obj)
          {
             if (elm_widget_is(seat_sel->drag_obj))
               {
                  Evas_Object *win = elm_widget_top_get(seat_sel->drag_obj);
                  if (win && efl_isa(win, EFL_UI_WIN_CLASS))
                     efl_event_callback_del(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED,
                                            _x11_win_rotation_changed_cb, seat_sel->drag_win);
               }
          }
        seat_sel->drag_obj = NULL;
     }
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win;

   win = _wl_window_get(drag_obj);
   if (win)
     ecore_wl2_dnd_drag_end(_wl_seat_get(win, drag_obj, seat));
#endif

   ELM_SAFE_FREE(seat_sel->drag_win, evas_object_del);
}

EOLIAN static void
_efl_selection_manager_drag_action_set(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                       Efl_Object *drag_obj, Efl_Selection_Action action,
                                       unsigned int seat)
{
   Sel_Manager_Seat_Selection *seat_sel = _sel_manager_seat_selection_init(pd, seat);
   if (seat_sel->drag_action == action) return;
   seat_sel->drag_action = action;
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Atom actx;
   Ecore_X_Window xwin = _x11_xwin_get(drag_obj);
   if (xwin)
     {
        actx = _x11_dnd_action_rev_map(action);
        ecore_x_dnd_source_action_set(actx);
     }
#endif
}

//drop side
EOLIAN static Eina_Bool
_efl_selection_manager_drop_target_add(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                       Efl_Object *target_obj, Efl_Selection_Format format,
                                       unsigned int seat)
{
   Eina_Bool ret = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_xwin_get(target_obj);
   if (xwin)
     ret = _x11_sel_manager_drop_target_add(pd, target_obj, format, xwin, seat);
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win = _wl_window_get(target_obj);
   if (win)
     ret = _wl_sel_manager_drop_target_add(pd, target_obj, format, seat);
#endif
   return ret;
}

EOLIAN static void
_efl_selection_manager_drop_target_del(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                       Efl_Object *target_obj, Efl_Selection_Format format,
                                       unsigned int seat)
{
   Sel_Manager_Dropable *dropable = NULL;
   Sel_Manager_Seat_Selection *seat_sel;

   dropable = efl_key_data_get(target_obj, "__elm_dropable");
   if (dropable)
     {
        Eina_Inlist *itr;
        Drop_Format *df;
        EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
          {
             if (df->format == format)
               {
                  dropable->format_list = eina_inlist_remove(dropable->format_list,
                                                             EINA_INLIST_GET(df));
                  free(df);
               }
          }
        if (!dropable->format_list)
          {
             pd->drop_list = eina_list_remove(pd->drop_list, dropable);
             efl_key_data_set(target_obj, "__elm_dropable", NULL);
             free(dropable);
             evas_object_event_callback_del(target_obj, EVAS_CALLBACK_DEL,
                                            _all_drop_targets_cbs_del);
          }
     }

   if (!pd->drop_list) return;
#ifdef HAVE_ELEMENTARY_X
   Eina_List *l;
   Ecore_X_Window xwin;
   Eina_Bool have_drop_list = EINA_FALSE;

   xwin = _x11_xwin_get(target_obj);
   EINA_LIST_FOREACH(pd->drop_list, l, dropable)
     {
        if (xwin == _x11_xwin_get(dropable->obj))
          {
             have_drop_list = EINA_TRUE;
             break;
          }
     }
   if (!have_drop_list) ecore_x_dnd_aware_set(xwin, EINA_FALSE);
#endif
   seat_sel = _sel_manager_seat_selection_init(pd, seat);
   ELM_SAFE_FREE(seat_sel->pos_handler, ecore_event_handler_del);
   ELM_SAFE_FREE(seat_sel->drop_handler, ecore_event_handler_del);
   ELM_SAFE_FREE(seat_sel->enter_handler, ecore_event_handler_del);
   ELM_SAFE_FREE(seat_sel->leave_handler, ecore_event_handler_del);
}

EOLIAN static void
_efl_selection_manager_container_drop_item_add(Eo *obj, Efl_Selection_Manager_Data *pd,
                                               Efl_Object *cont, Efl_Selection_Format format,
                                               void *item_func_data, Efl_Dnd_Item_Get item_func,
                                               Eina_Free_Cb item_func_free_cb EINA_UNUSED,
                                               unsigned int seat)
{
   Item_Container_Drop_Info *di;
   Sel_Manager_Dropable *dropable = NULL;

   if (_drop_item_container_del(pd, cont, EINA_FALSE))
     {
        di = eina_list_search_unsorted(pd->drop_cont_list, _drop_item_container_cmp, obj);
        if (!di) return;
     }
   else
     {
        di = calloc(1, sizeof(Item_Container_Drop_Info));
        if (!di) return;

        di->obj = obj;
        pd->drop_cont_list = eina_list_append(pd->drop_cont_list, di);
     }
   di->item_func = item_func;
   di->item_func_data = item_func_data;

   dropable = efl_key_data_get(cont, "__elm_dropable");
   if (!dropable)
     {
        dropable = calloc(1, sizeof(Sel_Manager_Dropable));
        if (!dropable) return;
        dropable->last.in = EINA_FALSE;
        pd->drop_list = eina_list_append(pd->drop_list, dropable);
        if (!pd->drop_list) return;
        dropable->obj = cont;
        efl_key_data_set(cont, "__elm_dropable", dropable);
     }
   dropable->is_container = EINA_TRUE;
   dropable->item_func = item_func;
   dropable->item_func_data = item_func_data;
   _efl_selection_manager_drop_target_add(obj, pd, cont, format, seat);
}

EOLIAN static void
_efl_selection_manager_container_drop_item_del(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                               Efl_Object *cont, unsigned int seat EINA_UNUSED)
{
   _drop_item_container_del(pd, cont, EINA_TRUE);
}

EOLIAN static void
_efl_selection_manager_container_drag_item_add(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
               Efl_Object *cont, double time_to_drag, double anim_duration,
               void *data_func_data, Efl_Dnd_Drag_Data_Get data_func, Eina_Free_Cb data_func_free_cb,
               void *item_get_func_data, Efl_Dnd_Item_Get item_get_func, Eina_Free_Cb item_get_func_free_cb,
               void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb  icon_func_free_cb,
               void *icon_list_func_data, Efl_Dnd_Drag_Icon_List_Create icon_list_func, Eina_Free_Cb icon_list_func_free_cb,
               unsigned int seat)
{
   //TODO: remove previous drag one
   Sel_Manager_Drag_Container *dc = calloc(1, sizeof(Sel_Manager_Drag_Container));
   if (!dc)
     {
        ERR("Failed to allocate memory");
        return;
     }
   dc->cont = cont;
   dc->time_to_drag = time_to_drag;
   dc->anim_duration = anim_duration;
   dc->drag_data_func_data = data_func_data;
   dc->drag_data_func = data_func;
   dc->drag_data_func_free_cb = data_func_free_cb;
   dc->item_get_func_data = item_get_func_data;
   dc->item_get_func = item_get_func;
   dc->item_get_func_free_cb = item_get_func_free_cb;
   dc->icon_func_data = icon_func_data;
   dc->icon_func = icon_func;
   dc->icon_func_free_cb = icon_func_free_cb;
   dc->icon_list_func_data = icon_list_func_data;
   dc->icon_list_func = icon_list_func;
   dc->icon_list_func_free_cb = icon_list_func_free_cb;
   dc->seat = seat;
   dc->pd = pd;

   _sel_manager_seat_selection_init(pd, seat);

   pd->drag_cont_list = eina_list_append(pd->drag_cont_list, dc);

   evas_object_event_callback_add(cont, EVAS_CALLBACK_MOUSE_DOWN,
                                  _cont_obj_mouse_down_cb, dc);
}

EOLIAN static void
_efl_selection_manager_container_drag_item_del(Eo *obj EINA_UNUSED, Efl_Selection_Manager_Data *pd,
                                               Efl_Object *cont, unsigned int seat EINA_UNUSED)
{
   Sel_Manager_Drag_Container *dc = eina_list_search_unsorted(pd->drag_cont_list,
                                      _drag_item_container_cmp, cont);
   if (dc)
     _item_container_del_internal(dc, EINA_TRUE);
}

static Eo *
_efl_selection_manager_efl_object_constructor(Eo *obj, Efl_Selection_Manager_Data *pd)
{
#if defined(HAVE_ELEMENTARY_X) || defined(HAVE_ELEMENTARY_WL2)
   const char *ev = getenv("ELM_DISPLAY");
#endif

#ifdef HAVE_ELEMENTARY_X
   Eina_Bool init_x = EINA_FALSE;
   Eina_Bool have_display = !!getenv("DISPLAY");

   if (ev) /* If ELM_DISPLAY is specified */
     {
        if (!strcmp(ev, "x11")) /* and it is X11 */
          {
             if (!have_display) /* if there is no $DISPLAY */
               {
                  ERR("$ELM_DISPLAY is set to x11 but $DISPLAY is not set");
                  init_x = EINA_FALSE;
               }
             else /* if there is */
               init_x = EINA_TRUE;
          }
        else /* not X11 */
          init_x = EINA_FALSE;
     }
   else /* ELM_DISPLAY not specified */
     {
        if (have_display) /* If there is a $DISPLAY */
          init_x = EINA_TRUE;
        else /* No $DISPLAY */
          init_x = EINA_FALSE;
     }
   if (init_x)
     {
        ecore_x_init(NULL);
     }
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Eina_Bool init_wl = EINA_FALSE;
   Eina_Bool have_wl_display = !!getenv("WAYLAND_DISPLAY");

   if (ev) /* If ELM_DISPLAY is specified */
     {
        if (!strcmp(ev, "wl")) /* and it is WL */
          {
             /* always try to connect to wl when it is enforced */
             init_wl = EINA_TRUE;
          }
        else /* not wl */
          init_wl = EINA_FALSE;
     }
   else /* ELM_DISPLAY not specified */
     {
        /* If there is a $WAYLAND_DISPLAY */
        if ((have_wl_display) && (!getenv("DISPLAY")))
          init_wl = EINA_TRUE;
        else /* No $WAYLAND_DISPLAY */
          init_wl = EINA_FALSE;
     }
   if (init_wl)
     {
        if (!ecore_wl2_init())
          {
             ERR("Could not initialize Ecore_Wl2");
             return NULL;
          }
        _elm_wl_display = ecore_wl2_display_connect(NULL);
        if (!_elm_wl_display)
          {
             ERR("Could not connect to Wayland Display");
             ecore_wl2_shutdown();
             return NULL;
          }
     }
#endif

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->sel_man = obj;
   pd->atom_list = calloc(1, SELECTION_N_ATOMS * sizeof(Sel_Manager_Atom));
   if (!pd->atom_list)
     {
        ERR("failed to allocate atom_list");
        return NULL;
     }
   pd->atom_list[SELECTION_ATOM_TARGETS].name = "TARGETS";
   pd->atom_list[SELECTION_ATOM_TARGETS].format = EFL_SELECTION_FORMAT_TARGETS;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_TARGETS].x_converter = _x11_targets_converter;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_TARGETS].wl_converter = _wl_targets_converter;
#endif
   pd->atom_list[SELECTION_ATOM_ATOM].name = "ATOM"; // for opera browser
   pd->atom_list[SELECTION_ATOM_ATOM].format = EFL_SELECTION_FORMAT_TARGETS;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_ATOM].x_converter = _x11_targets_converter;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_ATOM].wl_converter = _wl_targets_converter;
#endif

   pd->atom_list[SELECTION_ATOM_ELM].name = "application/x-elementary-markup";
   pd->atom_list[SELECTION_ATOM_ELM].format = EFL_SELECTION_FORMAT_MARKUP;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_ELM].x_converter = _x11_general_converter;
   pd->atom_list[SELECTION_ATOM_ELM].x_data_preparer = _x11_data_preparer_markup;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_ELM].wl_converter = _wl_general_converter;
   pd->atom_list[SELECTION_ATOM_ELM].wl_data_preparer = _wl_data_preparer_markup;
#endif

   pd->atom_list[SELECTION_ATOM_TEXT_URILIST].name = "text/uri-list";
   pd->atom_list[SELECTION_ATOM_TEXT_URILIST].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_TEXT_URILIST].x_converter = _x11_general_converter;
   pd->atom_list[SELECTION_ATOM_TEXT_URILIST].x_data_preparer = _x11_data_preparer_uri;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_TEXT_URILIST].wl_converter = _wl_general_converter;
   pd->atom_list[SELECTION_ATOM_TEXT_URILIST].wl_data_preparer = _wl_data_preparer_uri;
#endif

   pd->atom_list[SELECTION_ATOM_TEXT_X_VCARD].name = "text/x-vcard";
   pd->atom_list[SELECTION_ATOM_TEXT_X_VCARD].format = EFL_SELECTION_FORMAT_VCARD;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_TEXT_X_VCARD].x_converter = _x11_vcard_send;
   pd->atom_list[SELECTION_ATOM_TEXT_X_VCARD].x_data_preparer = _x11_data_preparer_vcard;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_TEXT_X_VCARD].wl_data_preparer = _wl_data_preparer_vcard;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_PNG].name = "image/png";
   pd->atom_list[SELECTION_ATOM_IMAGE_PNG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_PNG].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_PNG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_PNG].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_JPEG].name = "image/jpeg";
   pd->atom_list[SELECTION_ATOM_IMAGE_JPEG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_JPEG].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_JPEG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_JPEG].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_BMP].name = "image/x-ms-bmp";
   pd->atom_list[SELECTION_ATOM_IMAGE_BMP].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_BMP].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_BMP].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_BMP].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_GIF].name = "image/gif";
   pd->atom_list[SELECTION_ATOM_IMAGE_GIF].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_GIF].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_GIF].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_GIF].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_TIFF].name = "image/tiff";
   pd->atom_list[SELECTION_ATOM_IMAGE_TIFF].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_TIFF].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_TIFF].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_TIFF].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_SVG].name = "image/svg+xml";
   pd->atom_list[SELECTION_ATOM_IMAGE_SVG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_SVG].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_SVG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_SVG].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_XPM].name = "image/x-xpixmap";
   pd->atom_list[SELECTION_ATOM_IMAGE_XPM].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_XPM].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_XPM].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_XPM].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_TGA].name = "image/x-tga";
   pd->atom_list[SELECTION_ATOM_IMAGE_TGA].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_TGA].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_TGA].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_TGA].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_IMAGE_PPM].name = "image/x-portable-pixmap";
   pd->atom_list[SELECTION_ATOM_IMAGE_PPM].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_IMAGE_PPM].x_converter = _x11_image_converter;
   pd->atom_list[SELECTION_ATOM_IMAGE_PPM].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_IMAGE_PPM].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atom_list[SELECTION_ATOM_UTF8STRING].name = "UTF8_STRING";
   pd->atom_list[SELECTION_ATOM_UTF8STRING].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_UTF8STRING].x_converter = _x11_text_converter;
   pd->atom_list[SELECTION_ATOM_UTF8STRING].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_UTF8STRING].wl_converter = _wl_text_converter;
   pd->atom_list[SELECTION_ATOM_UTF8STRING].wl_data_preparer = _wl_data_preparer_text,
#endif

   pd->atom_list[SELECTION_ATOM_STRING].name = "STRING";
   pd->atom_list[SELECTION_ATOM_STRING].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_STRING].x_converter = _x11_text_converter;
   pd->atom_list[SELECTION_ATOM_STRING].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_STRING].wl_converter = _wl_text_converter;
   pd->atom_list[SELECTION_ATOM_STRING].wl_data_preparer = _wl_data_preparer_text;
#endif

   pd->atom_list[SELECTION_ATOM_COMPOUND_TEXT].name = "COMPOUND_TEXT";
   pd->atom_list[SELECTION_ATOM_COMPOUND_TEXT].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_COMPOUND_TEXT].x_converter = _x11_text_converter;
   pd->atom_list[SELECTION_ATOM_COMPOUND_TEXT].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_COMPOUND_TEXT].wl_converter = _wl_text_converter;
   pd->atom_list[SELECTION_ATOM_COMPOUND_TEXT].wl_data_preparer = _wl_data_preparer_text;
#endif

   pd->atom_list[SELECTION_ATOM_TEXT].name = "TEXT";
   pd->atom_list[SELECTION_ATOM_TEXT].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_TEXT].x_converter = _x11_text_converter;
   pd->atom_list[SELECTION_ATOM_TEXT].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_TEXT].wl_converter = _wl_text_converter;
   pd->atom_list[SELECTION_ATOM_TEXT].wl_data_preparer = _wl_data_preparer_text;
#endif

   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN_UTF8].name = "text/plain;charset=utf-8";
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN_UTF8].format = EFL_SELECTION_FORMAT_TEXT;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN_UTF8].x_converter = _x11_text_converter;
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN_UTF8].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN_UTF8].wl_converter = _wl_text_converter;
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN_UTF8].wl_data_preparer = _wl_data_preparer_text;
#endif

   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN].name = "text/plain";
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN].format = EFL_SELECTION_FORMAT_TEXT;
#ifdef HAVE_ELEMENTARY_X
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN].x_converter = _x11_text_converter;
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN].wl_converter = _wl_text_converter;
   pd->atom_list[SELECTION_ATOM_TEXT_PLAIN].wl_data_preparer = _wl_data_preparer_text;
#endif


   int i;
#ifdef HAVE_ELEMENTARY_X
   if (ecore_x_display_get())
     {
        for (i = 0; i < SELECTION_N_ATOMS; i++)
           {
              pd->atom_list[i].x_atom = ecore_x_atom_get(pd->atom_list[i].name);
              ecore_x_selection_converter_atom_add
                 (pd->atom_list[i].x_atom, pd->atom_list[i].x_converter);
           }
        pd->notify_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,
                                                     _efl_sel_manager_x11_selection_notify, pd);
        pd->clear_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR,
                                                    _x11_selection_clear, pd);
        pd->fix_handler = ecore_event_handler_add(ECORE_X_EVENT_FIXES_SELECTION_NOTIFY,
                                                  _x11_fixes_selection_notify, pd);
     }
#endif

   pd->type_hash = eina_hash_string_small_new(NULL);
   for (i = 0; i < SELECTION_N_ATOMS; i++)
     {
        eina_hash_add(pd->type_hash, pd->atom_list[i].name, &pd->atom_list[i]);
     }
   pd->text_uri = eina_stringshare_add("text/uri-list");

#ifdef HAVE_ELEMENTARY_WL2
   pd->send_handler = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND,
                           _wl_selection_send, pd);
   pd->changed_handler = ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_SELECTION,
                           _wl_selection_changed, pd);
   pd->end_handler = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_END,
                                             _wl_dnd_end, pd);
#endif
   managers = eina_list_append(managers, obj);
   return obj;
}

static void
_efl_selection_manager_efl_object_destructor(Eo *obj, Efl_Selection_Manager_Data *pd)
{
   Sel_Manager_Seat_Selection *seat_sel;
   Eina_List *l;
   Sel_Manager_Dropable *dropable;

   managers = eina_list_remove(managers, obj);
   EINA_LIST_FOREACH(pd->drop_list, l, dropable)
     {
        _drop_target_cbs_del(pd, dropable, dropable->obj);
     }
#ifdef HAVE_ELEMENTARY_X
   ecore_event_handler_del(pd->notify_handler);
   ecore_event_handler_del(pd->clear_handler);
#endif
#ifdef HAVE_ELEMENTARY_WL2
   ecore_event_handler_del(pd->send_handler);
   ecore_event_handler_del(pd->changed_handler);
   ecore_event_handler_del(pd->end_handler);
#endif
   free(pd->atom_list);
   EINA_LIST_FREE(pd->seat_list, seat_sel)
     {
        ecore_event_handler_del(seat_sel->pos_handler);
        ecore_event_handler_del(seat_sel->drop_handler);
        ecore_event_handler_del(seat_sel->enter_handler);
        ecore_event_handler_del(seat_sel->leave_handler);
#ifdef HAVE_ELEMENTARY_X
        free(seat_sel->sel_list);
#endif
#ifdef HAVE_ELEMENTARY_WL2
        free(seat_sel->sel);
#endif
#ifdef HAVE_ELEMENTARY_COCOA
        free(seat_sel->sel);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
        free(seat_sel->sel_list);
#endif
        free(seat_sel->saved_types->types);
        free(seat_sel->saved_types->imgfile);
        free(seat_sel->saved_types);
     }
   eina_hash_free(pd->type_hash);
   eina_stringshare_del(pd->text_uri);

   efl_destructor(efl_super(obj, MY_CLASS));
#if defined(HAVE_ELEMENTARY_X) || defined(HAVE_ELEMENTARY_WL2) || defined(HAVE_ELEMENTARY_WIN32)
   const char *ev = getenv("ELM_DISPLAY");
#endif

#ifdef HAVE_ELEMENTARY_X
   Eina_Bool init_x = EINA_FALSE;
   Eina_Bool have_display = !!getenv("DISPLAY");

   if (ev) /* If ELM_DISPLAY is specified */
     {
        if (!strcmp(ev, "x11")) /* and it is X11 */
          {
             if (!have_display) /* if there is no $DISPLAY */
               {
                  ERR("$ELM_DISPLAY is set to x11 but $DISPLAY is not set");
                  init_x = EINA_FALSE;
               }
             else /* if there is */
               init_x = EINA_TRUE;
          }
        else /* not X11 */
          init_x = EINA_FALSE;
     }
   else /* ELM_DISPLAY not specified */
     {
        if (have_display) /* If there is a $DISPLAY */
          init_x = EINA_TRUE;
        else /* No $DISPLAY */
          init_x = EINA_FALSE;
     }
   if (init_x)
     {
        if (ecore_x_display_get())
          ecore_x_shutdown();
     }
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Eina_Bool init_wl = EINA_FALSE;
   Eina_Bool have_wl_display = !!getenv("WAYLAND_DISPLAY");

   if (ev) /* If ELM_DISPLAY is specified */
     {
        if (!strcmp(ev, "wl")) /* and it is WL */
          {
             /* always try to connect to wl when it is enforced */
             init_wl = EINA_TRUE;
          }
        else /* not wl */
          init_wl = EINA_FALSE;
     }
   else /* ELM_DISPLAY not specified */
     {
        /* If there is a $WAYLAND_DISPLAY */
        if ((have_wl_display) && (!getenv("DISPLAY")))
          init_wl = EINA_TRUE;
        else /* No $WAYLAND_DISPLAY */
          init_wl = EINA_FALSE;
     }
   if (init_wl)
     {
        if (!managers) ecore_wl2_display_disconnect(_elm_wl_display);
        ecore_wl2_shutdown();
     }
#endif
}

#ifdef HAVE_ELEMENTARY_X
static void
_set_selection_list(Sel_Manager_Selection *sel_list, Sel_Manager_Seat_Selection *seat_sel)
{
   sel_list[EFL_SELECTION_TYPE_PRIMARY].debug = "Primary";
   sel_list[EFL_SELECTION_TYPE_PRIMARY].ecore_sel = ECORE_X_SELECTION_PRIMARY;
   sel_list[EFL_SELECTION_TYPE_PRIMARY].set = ecore_x_selection_primary_set;
   sel_list[EFL_SELECTION_TYPE_PRIMARY].clear = ecore_x_selection_primary_clear;
   sel_list[EFL_SELECTION_TYPE_PRIMARY].request = ecore_x_selection_primary_request;
   sel_list[EFL_SELECTION_TYPE_PRIMARY].seat_sel = seat_sel;

   sel_list[EFL_SELECTION_TYPE_SECONDARY].debug = "Secondary";
   sel_list[EFL_SELECTION_TYPE_SECONDARY].ecore_sel = ECORE_X_SELECTION_SECONDARY;
   sel_list[EFL_SELECTION_TYPE_SECONDARY].set = ecore_x_selection_secondary_set;
   sel_list[EFL_SELECTION_TYPE_SECONDARY].clear = ecore_x_selection_secondary_clear;
   sel_list[EFL_SELECTION_TYPE_SECONDARY].request = ecore_x_selection_secondary_request;
   sel_list[EFL_SELECTION_TYPE_SECONDARY].seat_sel = seat_sel;

   sel_list[EFL_SELECTION_TYPE_DND].debug = "DnD";
   sel_list[EFL_SELECTION_TYPE_DND].ecore_sel = ECORE_X_SELECTION_PRIMARY;
   sel_list[EFL_SELECTION_TYPE_DND].request = ecore_x_selection_xdnd_request;
   sel_list[EFL_SELECTION_TYPE_DND].seat_sel = seat_sel;

   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].debug = "Clipboard";
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].ecore_sel = ECORE_X_SELECTION_CLIPBOARD;
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].set = ecore_x_selection_clipboard_set;
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].clear = ecore_x_selection_clipboard_clear;
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].request = ecore_x_selection_clipboard_request;
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].seat_sel = seat_sel;
}
#endif
#ifdef HAVE_ELEMENTARY_WIN32
static void
_set_selection_list(Sel_Manager_Selection *sel_list, Sel_Manager_Seat_Selection *seat_sel)
{
   sel_list[EFL_SELECTION_TYPE_PRIMARY].debug = "Primary";
   sel_list[EFL_SELECTION_TYPE_PRIMARY].ecore_sel = ECORE_WIN32_SELECTION_PRIMARY;
   sel_list[EFL_SELECTION_TYPE_PRIMARY].set = ecore_win32_clipboard_set;
   sel_list[EFL_SELECTION_TYPE_PRIMARY].clear = ecore_win32_clipboard_clear;
   sel_list[EFL_SELECTION_TYPE_PRIMARY].get = ecore_win32_clipboard_get;
   sel_list[EFL_SELECTION_TYPE_PRIMARY].seat_sel = seat_sel;

   sel_list[EFL_SELECTION_TYPE_SECONDARY].debug = "Secondary";
   sel_list[EFL_SELECTION_TYPE_SECONDARY].ecore_sel = ECORE_WIN32_SELECTION_OTHER;
   sel_list[EFL_SELECTION_TYPE_SECONDARY].seat_sel = seat_sel;

   sel_list[EFL_SELECTION_TYPE_DND].debug = "DnD";
   sel_list[EFL_SELECTION_TYPE_DND].ecore_sel = ECORE_WIN32_SELECTION_OTHER;
   sel_list[EFL_SELECTION_TYPE_DND].seat_sel = seat_sel;

   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].debug = "Clipboard";
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].ecore_sel = ECORE_WIN32_SELECTION_CLIPBOARD;
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].set = ecore_win32_clipboard_set;
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].clear = ecore_win32_clipboard_clear;
   sel_list[EFL_SELECTION_TYPE_CLIPBOARD].seat_sel = seat_sel;
}
#endif

#include "efl_selection_manager.eo.c"
