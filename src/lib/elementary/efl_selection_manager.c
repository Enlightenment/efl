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
//#define MY_CLASS_NAME "Efl.Selection_Manager"

#define DEBUGON 1
#ifdef DEBUGON
# define sel_debug(fmt, args...) fprintf(stderr, __FILE__":%s:%d : " fmt "\n", __FUNCTION__, __LINE__, ##args)
#else
# define sel_debug(x...) do { } while (0)
#endif


//FIXME: data in selection_set and converter: pd???


static void _set_selection_list(X11_Cnp_Selection *sellist, Seat_Selection *seat_sel);
static Seat_Selection * _seat_selection_init(Efl_Selection_Manager_Data *pd, Efl_Input_Device *seat);
static Ecore_X_Atom _x11_dnd_action_rev_map(Efl_Selection_Action action);


//X11 does not have seat_id in selection_notify, use default one
static Seat_Selection *
_get_seat_selection(Efl_Selection_Manager_Data *pd, unsigned int seat_id)
{
   Eina_List *l = NULL;
   Seat_Selection *seat_sel = NULL;
   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        //FIXME: use pd->seat: multi-seat selection request???
        if (seat_sel->seat_id == pd->seat_id)
          {
             ERR("Request seat: %d", seat_sel->seat_id);
             break;
          }
     }
   if (!seat_sel)
     {
        ERR("Could not find request seat");
     }

   return seat_sel;
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


static Ecore_X_Window
_x11_xwin_get(Evas_Object *obj)
{
   if (!obj) return 0;

   Ecore_X_Window xwin = 0;
   //get top
   Evas_Object *top = obj;
   Evas_Object *parent = obj;
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
_x11_is_uri_type_data(X11_Cnp_Selection *sel EINA_UNUSED, Ecore_X_Event_Selection_Notify *notify)
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

static Eina_Bool
_x11_data_preparer_text(Efl_Selection_Manager_Data *pd, Ecore_X_Event_Selection_Notify *notify,
      Efl_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("text data preparer");
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = EFL_SELECTION_FORMAT_TEXT;
   ddata->data = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->len = data->length;
   return EINA_TRUE;
}

static Eina_Bool
_x11_data_preparer_markup(Efl_Selection_Manager_Data *pd, Ecore_X_Event_Selection_Notify *notify,
      Efl_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("markup data preparer");
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = EFL_SELECTION_FORMAT_MARKUP;
   ddata->data = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->len = data->length;
   return EINA_TRUE;
}

/**
 * So someone is pasting an image into my entry or widget...
 */
static Eina_Bool
_x11_data_preparer_uri(Efl_Selection_Manager_Data *pd, Ecore_X_Event_Selection_Notify *notify,
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
   free(pd->savedtypes->imgfile);
   if (pd->savedtypes->textreq)
     {
        pd->savedtypes->textreq = 0;
        pd->savedtypes->imgfile = stripstr;
     }
   else
     {
        ddata->format = EFL_SELECTION_FORMAT_IMAGE;
        ddata->data = stripstr;
        ddata->len = strlen(stripstr);
        pd->savedtypes->imgfile = NULL;
     }
   return EINA_TRUE;
}

/**
 * Just received an vcard, either through cut and paste, or dnd.
 */
static Eina_Bool
_x11_data_preparer_vcard(Efl_Selection_Manager_Data *pd, Ecore_X_Event_Selection_Notify *notify,
      Efl_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   sel_debug("vcard receive\n");
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = EFL_SELECTION_FORMAT_VCARD;
   ddata->data = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->len = data->length;
   return EINA_TRUE;
}

static Eina_Bool
_x11_data_preparer_image(Efl_Selection_Manager_Data *pd, Ecore_X_Event_Selection_Notify *notify,
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
   ddata->data = strdup(tmp->filename);
   ddata->len = strlen(tmp->filename);
   *tmp_info = tmp;
   return EINA_TRUE;
}

/*
 * Callback to handle a targets response on a selection request:
 * So pick the format we'd like; and then request it.
 */
static Eina_Bool
_x11_notify_handler_targets(Efl_Selection_Manager_Data *pd, X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   sel_debug("notify handler targets");
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atomlist;
   int i, j;

   targets = notify->data;
   atomlist = (Ecore_X_Atom *)(targets->data.data);
   for (j = (SELECTION_ATOM_LISTING_ATOMS + 1); j < SELECTION_N_ATOMS; j++)
     {
        sel_debug("\t%s %d", pd->atomlist[j].name, pd->atomlist[j].x_atom);
        if (!(pd->atomlist[j].format & sel->request_format)) continue;
        for (i = 0; i < targets->data.length; i++)
          {
             if ((pd->atomlist[j].x_atom == atomlist[i]) && (pd->atomlist[j].x_data_preparer))
               {
                  if (j == SELECTION_ATOM_TEXT_URILIST)
                    {
                       if (!_x11_is_uri_type_data(sel, notify)) continue;
                    }
                  sel_debug("Atom %s matches", pd->atomlist[j].name);
                  goto done;
               }
          }
     }
   sel_debug("Couldn't find anything that matches");
   return ECORE_CALLBACK_PASS_ON;
done:
   sel_debug("Sending request for %s, xwin=%#llx",
             pd->atomlist[j].name, (unsigned long long)sel->xwin);
   sel->request(sel->xwin, pd->atomlist[j].name);
   return ECORE_CALLBACK_PASS_ON;
}


/*
 * Response to a selection notify:
 *  - So we have asked for the selection list.
 *  - If it's the targets list, parse it, and fire of what we want,
 *    else it's the data we want.
 */
//NB: x11 does not have seat_id, use 1 as default
static Eina_Bool
_efl_sel_manager_x11_selection_notify(void *udata, int type EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = udata;
   Ecore_X_Event_Selection_Notify *ev = event;
   X11_Cnp_Selection *sel;
   Seat_Selection *seat_sel = NULL;
   int i;

   seat_sel =  _get_seat_selection(pd, 1);
   if (!seat_sel)
     return EINA_FALSE;

   sel_debug("selection notify callback: %d",ev->selection);
   switch (ev->selection)
     {
      case ECORE_X_SELECTION_PRIMARY:
        sel = seat_sel->sellist + EFL_SELECTION_TYPE_PRIMARY;
        break;
      case ECORE_X_SELECTION_SECONDARY:
        sel = seat_sel->sellist + EFL_SELECTION_TYPE_SECONDARY;
        break;
      case ECORE_X_SELECTION_XDND:
        sel = seat_sel->sellist + EFL_SELECTION_TYPE_DND;
        break;
      case ECORE_X_SELECTION_CLIPBOARD:
        sel = seat_sel->sellist + EFL_SELECTION_TYPE_CLIPBOARD;
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
        if (!strcmp(ev->target, pd->atomlist[i].name))
          {
             if (pd->atomlist[i].x_data_preparer)
               {
                  Efl_Selection_Data ddata;
                  Tmp_Info *tmp_info = NULL;
                  Eina_Bool success;
                  ddata.data = NULL;
                  sel_debug("Found something: %s", pd->atomlist[i].name);
                  success = pd->atomlist[i].x_data_preparer(pd, ev, &ddata, &tmp_info);
                  if ((pd->atomlist[i].format == EFL_SELECTION_FORMAT_IMAGE) &&
                      (pd->savedtypes->imgfile))
                    break;
                  if (ev->selection == ECORE_X_SELECTION_XDND)
                    {
                       if (success)
                         {
                            Dropable *dropable;
                            Eina_List *l;
                            sel_debug("drag & drop\n");
                            EINA_LIST_FOREACH(pd->drops, l, dropable)
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
                                      sel_debug("has dropable");
                                      ddata.x = pd->savedtypes->x;
                                      ddata.y = pd->savedtypes->y;
                                      ddata.item = NULL;
                                   }
                                 else
                                   {
                                      sel_debug("Drop on container");
                                      Evas_Coord x0 = 0, y0 = 0;
                                      Evas_Coord xret = 0, yret = 0;
                                      evas_object_geometry_get(dropable->obj, &x0, &y0, NULL, NULL);
                                      //get item
                                      Efl_Object *it = NULL;
                                      if (dropable->item_func)
                                        dropable->item_func(dropable->item_func_data,
                                                  dropable->obj, pd->savedtypes->x + x0, pd->savedtypes->y + y0,
                                                  &xret, &yret);
                                      ddata.x = xret;
                                      ddata.y = yret;
                                      ddata.item = it;
                                   }
                                 EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
                                    if (df->format & dropable->last.format)
                                      {
                                         efl_event_callback_call(dropable->obj, EFL_DND_EVENT_DRAG_DROP, &ddata);
                                      }
                              }
                         }
                       /* We have to finish DnD, no matter what */
                       ecore_x_dnd_send_finished();
                    }
                  else if (sel->data_func && success)
                    {
                       ddata.x = ddata.y = 0;
                       sel->data_func(sel->data_func_data, sel->request_obj, &ddata);
                    }
                  free(ddata.data);
                  if (tmp_info) _tmpinfo_free(tmp_info);
               }
             else sel_debug("Ignored: No handler!");
             break;
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static void
_selection_loss_data_clear_cb(void *data)
{
   Efl_Selection_Type *lt = data;
   free(lt);
}

static Eina_Bool
_x11_selection_clear(void *data, int type EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = data;
   Ecore_X_Event_Selection_Clear *ev = event;
   X11_Cnp_Selection *sel;
   Seat_Selection *seat_sel = NULL;
   unsigned int i;
   ERR("In");


   /*if (pd->promise)
     {
        Efl_Selection_Type *lt = malloc(sizeof(Efl_Selection_Type));
        *lt = pd->loss_type;
        efl_promise_value_set(pd->promise, lt, _selection_loss_data_clear_cb);
        pd->promise = NULL;
     }*/

   seat_sel = _get_seat_selection(pd, 1);
   if (!seat_sel)
     return EINA_FALSE;

   for (i = ELM_SEL_TYPE_PRIMARY; i <= ELM_SEL_TYPE_CLIPBOARD; i++)
     {
        if (seat_sel->sellist[i].ecore_sel == ev->selection) break;
     }
   sel_debug("selection %d clear", i);
   /* Not me... Don't care */
   if (i > ELM_SEL_TYPE_CLIPBOARD) return ECORE_CALLBACK_PASS_ON;

   sel = seat_sel->sellist + i;

   efl_event_callback_call(sel->owner, EFL_SELECTION_EVENT_SELECTION_LOSS, NULL);
   sel->owner = NULL;

   return ECORE_CALLBACK_PASS_ON;
}

//note: sel_set: data is pd -> converter: data is pd
//->return data from converter: real buffer data ready to paste


static Efl_Selection_Format
_get_selection_type(void *data, Seat_Selection *seat_sel)
{
   Efl_Selection_Manager_Data *pd = *(Efl_Selection_Manager_Data **)data;
   if (pd->has_sel)
     {
        sel_debug("has sel");
        if (pd->active_type > EFL_SELECTION_TYPE_CLIPBOARD)
          return EFL_SELECTION_FORMAT_NONE;
        sel_debug("has active type: %d, pd active_format: %d", pd->active_type, pd->active_format);
        if ((seat_sel->sellist[pd->active_type].format >= EFL_SELECTION_FORMAT_TARGETS) &&
            (seat_sel->sellist[pd->active_type].format <= EFL_SELECTION_FORMAT_HTML))
          return seat_sel->sellist[pd->active_type].format;
     }
   sel_debug("has no sel");
   return EFL_SELECTION_FORMAT_NONE;
}

static Eina_Bool
_x11_general_converter(char *target EINA_UNUSED, void *data, int size EINA_UNUSED, void **data_ret, int *size_ret, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
{
   //Efl_Selection_Manager_Data *pd = *(Efl_Selection_Manager_Data **)data;
   //Seat_Selection *seat_sel = _get_seat_selection(pd, 1);

   X11_Cnp_Selection *sel = *(X11_Cnp_Selection **)data;
   //if (_get_selection_type(data, seat_sel) == EFL_SELECTION_FORMAT_NONE)
   if (sel->format == EFL_SELECTION_FORMAT_NONE)
     {
        //FIXME: Check this case: remove or not
        if (data_ret)
          {
             *data_ret = malloc(sel->len * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, sel->selbuf, sel->len);
             ((char**)(data_ret))[0][sel->len] = 0;
          }
        if (size_ret) *size_ret = sel->len;
     }
   else
     {
        //X11_Cnp_Selection *sel = seat_sel->sellist + pd->active_type;
        if (sel->selbuf)
          {
             if (data_ret) *data_ret = strdup(sel->selbuf);
             if (size_ret) *size_ret = strlen(sel->selbuf);
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
   X11_Cnp_Selection *sel;
   Efl_Selection_Format seltype;

   if (!data_ret) return EINA_FALSE;
   //Efl_Selection_Manager_Data *pd = *(Efl_Selection_Manager_Data **)data;
   //seltype = pd->atomlist[pd->active_format].format;

   sel = *(X11_Cnp_Selection **)data;
   seltype = sel->format;
   Seat_Selection *seat_sel = sel->seat_sel;
   Efl_Selection_Manager_Data *pd = seat_sel->pd;

   for (i = SELECTION_ATOM_LISTING_ATOMS + 1, count = 0; i < SELECTION_N_ATOMS ; i++)
     {
        if (seltype & pd->atomlist[i].format) count++;
     }
   aret = malloc(sizeof(Ecore_X_Atom) * count);
   if (!aret) return EINA_FALSE;
   for (i = SELECTION_ATOM_LISTING_ATOMS + 1, count = 0; i < SELECTION_N_ATOMS ; i++)
     {
        if (seltype & pd->atomlist[i].format)
          aret[count ++] = pd->atomlist[i].x_atom;
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
   X11_Cnp_Selection *sel;

   sel_debug("Vcard send called");
   //Efl_Selection_Manager_Data *pd = *(Efl_Selection_Manager_Data **)data;
   //sel = _x11_selections + *((int *)data);
   //sel = &pd->sellist[pd->active_type];
   sel = *(X11_Cnp_Selection **)data;
   if (data_ret) *data_ret = strdup(sel->selbuf);
   if (size_ret) *size_ret = strlen(sel->selbuf);
   return EINA_TRUE;
}

static Eina_Bool
_x11_text_converter(char *target, void *data, int size EINA_UNUSED, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize)
{
   X11_Cnp_Selection *sel;
   //Efl_Selection_Manager_Data *pd = *(Efl_Selection_Manager_Data **)data;

   //Seat_Selection *seat_sel = _get_seat_selection(pd, 1);

   sel = *(X11_Cnp_Selection **)data;
   if (!sel) return EINA_FALSE;

   sel_debug("text converter");
   //if (_get_selection_type(data, seat_sel) == EFL_SELECTION_FORMAT_NONE)
   if (sel->format == EFL_SELECTION_FORMAT_NONE)
     {
        sel_debug("none");
        if (data_ret)
          {
             *data_ret = malloc(sel->len * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, data, sel->len);
             ((char**)(data_ret))[0][sel->len] = 0;
          }
        if (size_ret) *size_ret = sel->len;
        return EINA_TRUE;
     }
   //sel = _x11_selections + *((int *)data);

   //sel = &seat_sel->sellist[pd->active_type];

   if ((sel->format & EFL_SELECTION_FORMAT_MARKUP) ||
       (sel->format & EFL_SELECTION_FORMAT_HTML))
     {
        *data_ret = _elm_util_mkup_to_text(sel->selbuf);
        if (size_ret && *data_ret) *size_ret = strlen(*data_ret);
        sel_debug("markup or html: %s", (const char *)*data_ret);
     }
   else if (sel->format & EFL_SELECTION_FORMAT_TEXT)
     {
        ecore_x_selection_converter_text(target, sel->selbuf,
                                         strlen(sel->selbuf),
                                         data_ret, size_ret,
                                         ttype, typesize);
        sel_debug("text");
     }
   else if (sel->format & EFL_SELECTION_FORMAT_IMAGE)
     {
        //FIXME: remove photocam...
        //sel_debug("Image %s", evas_object_type_get(sel->request_obj));
        //sel_debug("Elm type: %s", elm_object_widget_type_get(sel->request_obj));
        //evas_object_image_file_get(elm_photocam_internal_image_get(sel->request_obj),
        //                           (const char **)data_ret, NULL);
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

#ifdef HAVE_ELEMENTARY_X
static void
_x11_efl_sel_manager_selection_set(Eo *obj, Efl_Selection_Manager_Data *pd,
                                   Efl_Selection_Type type, Efl_Selection_Format format,
                                   //const void *buf, int len, Efl_Input_Device *seat)
                                   const void *buf, int len, Seat_Selection *seat_sel)
{
   Ecore_X_Window xwin = _x11_xwin_get(obj);
   X11_Cnp_Selection *sel = seat_sel->sellist + type;

   pd->active_type = type;
   sel_debug("pd active_type: %d, active_format: %d", pd->active_type, pd->active_format);
   pd->active_format = format;
   if (sel->selbuf)
     {
        free(sel->selbuf);
     }
   sel->selbuf = malloc(len);
   if (!sel->selbuf)
     {
        ERR("failed to allocate buf");
        return;
     }
   sel->selbuf = memcpy(sel->selbuf, buf, len);
   sel->len = len;
   sel->format = format;

   //set selection
   //seat_sel->sellist[type].set(xwin, &pd, sizeof(&pd));
   //sel_debug("data: %p (%ld)", &pd, sizeof(&pd));
   sel->set(xwin, &sel, sizeof(&sel));
   sel_debug("data: %p (%ld)", &sel, sizeof(&sel));
}
#endif

EOLIAN static void
_efl_selection_manager_selection_set(Eo *obj, Efl_Selection_Manager_Data *pd,
                                     Efl_Object *owner, Efl_Selection_Type type,
                                     Efl_Selection_Format format,
                                     const void *buf, int len, Efl_Input_Device *seat)
{
   ERR("In");
   if (type > EFL_SELECTION_TYPE_CLIPBOARD)
     {
        ERR("Not supported format: %d", type);
        return;
     }
   Eina_Bool same_win = EINA_FALSE;

   unsigned int seat_id = 1;

   if (seat)
     {
        seat_id = efl_input_device_seat_id_get(seat);
     }
   pd->seat_id = seat_id;
   pd->active_type = type;
   pd->has_sel = EINA_TRUE;

   Seat_Selection *seat_sel = _seat_selection_init(pd, seat);


#ifdef HAVE_ELEMENTARY_X
   //X11_Cnp_Selection *sel = pd->sellist + type;
   X11_Cnp_Selection *sel = seat_sel->sellist + type;
   Ecore_X_Window xwin = _x11_xwin_get(owner);
   //support 1 app with multiple window, 1 selection manager
   if (sel->xwin == xwin)
     same_win = EINA_TRUE;
#endif
   //check if owner is changed
   if (sel->owner != NULL &&
       sel->owner != owner && same_win)
     {
        /*//call selection_loss callback: should we include prev owner??
        Efl_Selection_Type *lt = malloc(sizeof(Efl_Selection_Type));
        *lt = pd->loss_type;
        efl_promise_value_set(pd->promise, lt, _selection_loss_data_clear_cb);*/

        efl_event_callback_call(sel->owner, EFL_SELECTION_EVENT_SELECTION_LOSS, NULL);
     }

   sel->owner = owner;
   pd->has_sel = EINA_TRUE;
#ifdef HAVE_ELEMENTARY_X
   sel->xwin = xwin;

   return _x11_efl_sel_manager_selection_set(obj, pd, type, format, buf, len, seat_sel);
#endif
#ifdef HAVE_ELEMENTARY_WL2
#endif
#ifdef HAVE_ELEMENTARY_COCOA
#endif
#ifdef HAVE_ELEMENTARY_WIN32
#endif
}

static void
_x11_efl_sel_manager_selection_get(Eo *obj, Efl_Selection_Manager_Data *pd,
                                   //Efl_Selection_Type type, Efl_Selection_Format format, Efl_Input_Device *seat)
                                   Efl_Selection_Type type, Efl_Selection_Format format, Seat_Selection *seat_sel)
{
   Ecore_X_Window xwin = _x11_xwin_get(obj);
   ERR("xwin: %d", xwin);
   //FIXME: use each sel or just pd
   //X11_Cnp_Selection *sel = pd->sellist + type;
   X11_Cnp_Selection *sel = seat_sel->sellist + type;
   sel->request_format = format;
   sel->xwin = xwin;

   if (pd->has_sel)
     {
        if (sel->selbuf &&
            ((format == sel->format) || (xwin == 0)))
          {
             sel_debug("use local data");
             Efl_Selection_Data seldata;

             seldata.data = sel->selbuf;
             seldata.len = sel->len;
             seldata.x = seldata.y = 0;
             seldata.format = sel->format;
             sel->data_func(sel->data_func_data, sel->request_obj, &seldata);
             return;
          }
     }

   sel->request(xwin, ECORE_X_SELECTION_TARGET_TARGETS);
}

EOLIAN static void
_efl_selection_manager_selection_get(Eo *obj, Efl_Selection_Manager_Data *pd,
                                     Efl_Object *owner, Efl_Selection_Type type,
                                     Efl_Selection_Format format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb,
                                     Efl_Input_Device *seat)
{
   ERR("In");
   pd->seat_id = 1;

   if (seat)
     {
        pd->seat_id = efl_input_device_seat_id_get(seat);
     }

   Seat_Selection *seat_sel = _seat_selection_init(pd, seat);

   //X11_Cnp_Selection *sel = pd->sellist + type;
   X11_Cnp_Selection *sel = seat_sel->sellist + type;

   sel->request_obj = obj;
   sel->data_func_data = data_func_data;
   sel->data_func = data_func;
   sel->data_func_free_cb = data_func_free_cb;

   _x11_efl_sel_manager_selection_get(obj, pd, type, format, seat_sel);
}


EOLIAN static void
_efl_selection_manager_selection_clear(Eo *obj, Efl_Selection_Manager_Data *pd,
                                       Efl_Object *owner, Efl_Selection_Type type, Efl_Input_Device *seat)
{
   ERR("In");
   Eina_Bool local = EINA_FALSE;
   unsigned int seat_id = 1;

   if (type > EFL_SELECTION_TYPE_CLIPBOARD)
     {
        ERR("Not supported type: %d", type);
        return;
     }

   if (seat)
     {
        seat_id = efl_input_device_seat_id_get(seat);
     }

   Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;
   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if (seat_id == seat_sel->seat_id)
          {
             break;
          }
     }
   if (!seat_sel)
     {
        seat_sel = malloc(sizeof(Seat_Selection));
        if (!seat_sel)
          {
             ERR("Failed to allocate seat");
             return;
          }
        seat_sel->seat_id = seat_id;
        pd->seat_list = eina_list_append(pd->seat_list, seat_sel);
     }
   if (!seat_sel->sellist)
     {
        seat_sel->sellist = calloc(1, (EFL_SELECTION_TYPE_CLIPBOARD + 1) * sizeof(X11_Cnp_Selection));
        if (!seat_sel->sellist)
          {
             ERR("failed to allocate selection list");
             return;
          }
     }


   //X11_Cnp_Selection *sel = pd->sellist + type;
   X11_Cnp_Selection *sel = seat_sel->sellist + type;
   if (sel->owner != owner)
     {
        return;
     }
   seat_sel->sellist[type].len = 0;
   if (seat_sel->sellist[type].selbuf)
     {
        free(seat_sel->sellist[type].selbuf);
        seat_sel->sellist[type].selbuf = NULL;
     }
#ifdef HAVE_ELEMENTARY_X
   if (sel->xwin != 0)
     local = EINA_TRUE;
#endif
   if (!local)
     {
        seat_sel->sellist[type].clear();
     }
   else
     {
        efl_event_callback_call(sel->owner, EFL_SELECTION_EVENT_SELECTION_LOSS, NULL);
        seat_sel->sellist[type].owner = NULL;
     }
}


/*
EOLIAN static Efl_Future *
_efl_selection_manager_selection_loss_feedback(Eo *obj, Efl_Selection_Manager_Data *pd,
                                               Efl_Object *owner, Efl_Selection_Type type)
{
   ERR("In");
   Efl_Promise *p;
   Eo *loop = efl_loop_get(obj);

   pd->promise = NULL;
   p = efl_add(EFL_PROMISE_CLASS, loop);
   if (!p) return NULL;
   pd->promise = p;
   pd->loss_type = type;

   return efl_promise_future_get(p);
}*/

static Eina_Bool
_drag_cancel_animate(void *data, double pos)
{  /* Animation to "move back" drag-window */
   Efl_Selection_Manager_Data *pd = data;
   if (pos >= 0.99)
     {
#ifdef HAVE_ELEMENTARY_X
        Ecore_X_Window xdragwin = _x11_xwin_get(pd->drag_win);
        ecore_x_window_ignore_set(xdragwin, 0);
#endif
        ERR("Delete drag_win");
        evas_object_del(pd->drag_win);
        pd->drag_win = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   else
     {
        int x, y;
        x = pd->drag_win_x_end - (pos * (pd->drag_win_x_end - pd->drag_win_x_start));
        y = pd->drag_win_y_end - (pos * (pd->drag_win_y_end - pd->drag_win_y_start));
        evas_object_move(pd->drag_win, x, y);
     }

   return ECORE_CALLBACK_RENEW;
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
   Efl_Selection_Manager_Data *pd = data;
   //Ecore_X_Window xwin = (Ecore_X_Window)(long)data;
   Ecore_X_Window xwin = pd->xwin;
   Ecore_Event_Mouse_Button *ev = event;

   if ((ev->buttons == 1) &&
       (ev->event_window == xwin))
     {
        Eina_Bool have_drops = EINA_FALSE;
        Eina_List *l;
        Dropable *dropable;

        ecore_x_pointer_ungrab();
        ELM_SAFE_FREE(pd->mouse_up_handler, ecore_event_handler_del);
        ELM_SAFE_FREE(pd->dnd_status_handler, ecore_event_handler_del);
        ecore_x_dnd_self_drop();

        sel_debug("mouse up, xwin=%#llx\n", (unsigned long long)xwin);

        EINA_LIST_FOREACH(pd->drops, l, dropable)
          {
             if (xwin == _x11_xwin_get(dropable->obj))
               {
                  have_drops = EINA_TRUE;
                  break;
               }
          }
        if (!have_drops) ecore_x_dnd_aware_set(xwin, EINA_FALSE);
        efl_event_callback_call(pd->drag_obj, EFL_DND_EVENT_DRAG_DONE, NULL);
        if (pd->drag_win)
          {
             if (pd->drag_obj)
               {
                  if (elm_widget_is(pd->drag_obj))
                    {
                       Evas_Object *win = elm_widget_top_get(pd->drag_obj);
                       if (win && efl_isa(win, EFL_UI_WIN_CLASS))
                         efl_event_callback_del(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED,
                                                _x11_win_rotation_changed_cb, pd->drag_win);
                    }
               }

             if (!pd->accept)
               {  /* Commit animation when drag cancelled */
                  /* Record final position of dragwin, then do animation */
                  ecore_animator_timeline_add(0.3,
                        _drag_cancel_animate, pd);
               }
             else
               {  /* No animation drop was committed */
                  Ecore_X_Window xdragwin = _x11_xwin_get(pd->drag_win);
                  ecore_x_window_ignore_set(xdragwin, 0);
                  evas_object_del(pd->drag_win);
                  pd->drag_win = NULL;
               }
          }

        pd->drag_obj = NULL;
        pd->accept = EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_x11_drag_move(void *data, Ecore_X_Xdnd_Position *pos)
{
   Efl_Selection_Manager_Data *pd = data;
   evas_object_move(pd->drag_win,
                    pos->position.x - pd->dragx, pos->position.y - pd->dragy);
   pd->drag_win_x_end = pos->position.x - pd->dragx;
   pd->drag_win_y_end = pos->position.y - pd->dragy;
   sel_debug("dragevas: %p -> %p\n",
          pd->drag_obj,
          evas_object_evas_get(pd->drag_obj));
   Efl_Dnd_Drag_Pos dp;
   dp.x = pos->position.x;
   dp.y = pos->position.y;
   dp.action = pd->drag_action;
   //dp.format = ;//
   //for drag side
   efl_event_callback_call(pd->drag_obj, EFL_DND_EVENT_DRAG_POS, &dp);
}

static void
_x11_drag_target_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *info EINA_UNUSED)
{
   Efl_Selection_Manager_Data *pd = data;
   //X11_Cnp_Selection *sel = _x11_selections + ELM_SEL_TYPE_XDND;
   Seat_Selection *seat_sel = _get_seat_selection(pd, 1);
   X11_Cnp_Selection *sel = &seat_sel->sellist[pd->active_type];

   if (pd->drag_obj == obj)
     {
        sel->request_obj = NULL;
        pd->drag_obj = NULL;
     }
}

static Eina_Bool
_x11_dnd_status(void *data, int etype EINA_UNUSED, void *ev)
{
   Efl_Selection_Manager_Data *pd = data;
   Ecore_X_Event_Xdnd_Status *status = ev;

   pd->accept = EINA_FALSE;

   /* Only thing we care about: will accept */
   if ((status) && (status->will_accept))
     {
        sel_debug("Will accept\n");
        pd->accept = EINA_TRUE;
     }
   /* Won't accept */
   else
     {
        sel_debug("Won't accept accept\n");
     }
   efl_event_callback_call(pd->drag_obj, EFL_DND_EVENT_DRAG_ACCEPT, &pd->accept);

   return EINA_TRUE;
}

static Seat_Selection *
_seat_selection_init(Efl_Selection_Manager_Data *pd, Efl_Input_Device *seat)
{
   unsigned int seat_id = 1;
   Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;

   if (seat)
     {
        seat_id = efl_input_device_seat_id_get(seat);
     }
   ERR("seat id: %d", seat_id);

   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if(seat_sel->seat_id == seat_id)
          {
             break;
          }
     }
   if (!seat_sel)
     {
        seat_sel = calloc(1, sizeof(Seat_Selection));
        if (!seat_sel)
          {
             ERR("Failed to allocate seat");
             return NULL;
          }
        seat_sel->seat_id = seat_id;
        seat_sel->pd = pd;
        pd->seat_list = eina_list_append(pd->seat_list, seat_sel);
     }
   if (!seat_sel->sellist)
     {
        //TODO: reduce memory (may be just need one common sellist)
        seat_sel->sellist = calloc(1, (EFL_SELECTION_TYPE_CLIPBOARD + 1) * sizeof(X11_Cnp_Selection));
        if (!seat_sel->sellist)
          {
             ERR("failed to allocate selection list");
             return NULL;
          }
        _set_selection_list(seat_sel->sellist, seat_sel);
     }

   return seat_sel;
}

//TODO: Should we add DRAG_START event???
EOLIAN static void
_efl_selection_manager_drag_start(Eo *obj, Efl_Selection_Manager_Data *pd, Efl_Object *drag_obj, Efl_Selection_Format format, const void *buf, int len, Efl_Selection_Action action, void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb icon_func_free_cb, Efl_Input_Device *seat)
{
   Ecore_X_Window xwin = _x11_xwin_get(drag_obj);
   Ecore_X_Window xdragwin;
   Efl_Selection_Type xdnd = ELM_SEL_TYPE_XDND;
   Ecore_Evas *ee;
   int x, y, x2 = 0, y2 = 0, x3, y3;
   Evas_Object *icon = NULL;
   int w = 0, h = 0;
   int ex, ey, ew, eh;
   Ecore_X_Atom actx;
   int i;
   int xr, yr, rot;

   pd->seat_id = 1;
   if (seat)
     {
        pd->seat_id = efl_input_device_seat_id_get(seat);
     }
   pd->active_type = EFL_SELECTION_TYPE_DND;
   pd->has_sel = EINA_TRUE;

   //
   Seat_Selection *seat_sel = _seat_selection_init(pd, seat);

   X11_Cnp_Selection *sel = &seat_sel->sellist[pd->active_type];
   ecore_x_dnd_types_set(xwin, NULL, 0);
   for (i = SELECTION_ATOM_LISTING_ATOMS + 1; i < SELECTION_N_ATOMS; i++)
     {
        if (format == EFL_SELECTION_FORMAT_TARGETS || (pd->atomlist[i].format & format))
          {
             ecore_x_dnd_type_set(xwin, pd->atomlist[i].name, EINA_TRUE);
             sel_debug("set dnd type: %s\n", pd->atomlist[i].name);
          }
     }

   //sel = _x11_selections + ELM_SEL_TYPE_XDND;
   //sel->active = EINA_TRUE;
   sel->request_obj = drag_obj;
   sel->format = format;
   sel->selbuf = buf ? strdup(buf) : NULL;
   sel->action = action;
   pd->drag_obj = drag_obj;
   pd->drag_action = action;
   pd->xwin = xwin;

   evas_object_event_callback_add(drag_obj, EVAS_CALLBACK_DEL,
                                  _x11_drag_target_del, pd);
   /* TODO BUG: should increase dnd-awareness, in case it's drop target as well. See _x11_drag_mouse_up() */
   ecore_x_dnd_aware_set(xwin, EINA_TRUE);
   ecore_x_dnd_callback_pos_update_set(_x11_drag_move, pd);
   //ecore_x_dnd_self_begin(xwin, (unsigned char *)&xdnd, sizeof(Elm_Sel_Type));
   //ecore_x_dnd_self_begin(xwin, (unsigned char *)&pd, sizeof(Efl_Selection_Manager_Data));
   ecore_x_dnd_self_begin(xwin, (unsigned char *)&sel, sizeof(X11_Cnp_Selection)); //TODO: sizeof
   sel_debug("sizeof: %ld %ld", sizeof(&sel), sizeof(X11_Cnp_Selection));
   actx = _x11_dnd_action_rev_map(pd->drag_action);
   ecore_x_dnd_source_action_set(actx);
   ecore_x_pointer_grab(xwin);
   pd->mouse_up_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
                                        _x11_drag_mouse_up, pd);
   pd->dnd_status_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_STATUS,
                                            _x11_dnd_status, pd);
   pd->drag_win = elm_win_add(NULL, "Elm-Drag", ELM_WIN_DND);
   elm_win_alpha_set(pd->drag_win, EINA_TRUE);
   elm_win_override_set(pd->drag_win, EINA_TRUE);
   xdragwin = _x11_xwin_get(pd->drag_win);
   ecore_x_window_ignore_set(xdragwin, 1);

   /* dragwin has to be rotated as the main window is */
   if (elm_widget_is(drag_obj))
     {
        Evas_Object *win = elm_widget_top_get(drag_obj);
        if (win && efl_isa(win, EFL_UI_WIN_CLASS))
          {
             elm_win_rotation_set(pd->drag_win, elm_win_rotation_get(win));
             efl_event_callback_add(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED,
                                    _x11_win_rotation_changed_cb, pd->drag_win);
          }
     }

   if (icon_func)
     {
        Evas_Coord xoff = 0, yoff = 0;

        icon = icon_func(icon_func_data, pd->drag_win, &xoff, &yoff);
        if (icon)
          {
             x2 = xoff;
             y2 = yoff;
             evas_object_geometry_get(icon, NULL, NULL, &w, &h);
          }
     }
   else
     {
        icon = elm_icon_add(pd->drag_win);
        evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        // need to resize
     }
   elm_win_resize_object_add(pd->drag_win, icon);

   /* Position subwindow appropriately */
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(drag_obj));
   ecore_evas_geometry_get(ee, &ex, &ey, &ew, &eh);
   evas_object_resize(pd->drag_win, w, h);

   evas_object_show(icon);
   evas_object_show(pd->drag_win);
   evas_pointer_canvas_xy_get(evas_object_evas_get(drag_obj), &x3, &y3);

   rot = ecore_evas_rotation_get(ee);
   switch (rot)
     {
      case 90:
         xr = y3;
         yr = ew - x3;
         pd->dragx = y3 - y2;
         pd->dragy = x3 - x2;
         break;
      case 180:
         xr = ew - x3;
         yr = eh - y3;
         pd->dragx = x3 - x2;
         pd->dragy = y3 - y2;
         break;
      case 270:
         xr = eh - y3;
         yr = x3;
         pd->dragx = y3 - y2;
         pd->dragy = x3 - x2;
         break;
      default:
         xr = x3;
         yr = y3;
         pd->dragx = x3 - x2;
         pd->dragy = y3 - y2;
         break;
     }
   x = ex + xr - pd->dragx;
   y = ey + yr - pd->dragy;
   evas_object_move(pd->drag_win, x, y);
   pd->drag_win_x_start = pd->drag_win_x_end = x;
   pd->drag_win_y_start = pd->drag_win_y_end = y;
}

EOLIAN static void
_efl_selection_manager_drag_cancel(Eo *obj, Efl_Selection_Manager_Data *pd, Efl_Object *drag_obj, Efl_Input_Device *seat)
{
   ERR("In");

#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_xwin_get(drag_obj);
   if (xwin)
     {
        ecore_x_pointer_ungrab();
        ELM_SAFE_FREE(pd->mouse_up_handler, ecore_event_handler_del);
        ELM_SAFE_FREE(pd->dnd_status_handler, ecore_event_handler_del);
        ecore_x_dnd_abort(xwin);
        if (pd->drag_obj)
          {
             if (elm_widget_is(pd->drag_obj))
               {
                  Evas_Object *win = elm_widget_top_get(pd->drag_obj);
                  if (win && efl_isa(win, EFL_UI_WIN_CLASS))
                     efl_event_callback_del(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED,
                                            _x11_win_rotation_changed_cb, pd->drag_win);
               }
          }
        pd->drag_obj = NULL;
     }
#endif

   ELM_SAFE_FREE(pd->drag_win, evas_object_del);
}

EOLIAN static void
_efl_selection_manager_drag_action_set(Eo *obj, Efl_Selection_Manager_Data *pd , Efl_Selection_Action action, Efl_Input_Device *seat)
{
   ERR("In");
   Ecore_X_Atom actx;

   if (pd->drag_action == action) return;
   pd->drag_action = action;
   actx = _x11_dnd_action_rev_map(action);
   ecore_x_dnd_source_action_set(actx);
}

static void
_all_drop_targets_cbs_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *info EINA_UNUSED)
{
   Efl_Object *sel_man = data;
   Dropable *dropable = NULL;
   dropable = efl_key_data_get(obj, "__elm_dropable");
   if (dropable)
     {
        Drop_Format *df;
        while (dropable->format_list)
          {
             df = EINA_INLIST_CONTAINER_GET(dropable->format_list, Drop_Format);
             //NOT implemented
             efl_selection_manager_drop_target_del(sel_man, obj, df->format, dropable->seat);
             // If elm_drop_target_del() happened to delete dropabale, then
             // re-fetch it each loop to make sure it didn't
             dropable = efl_key_data_get(obj, "__elm_dropable");
             if (!dropable) break;
          }
        /*Dropable_Cbs *cbs;
        while (dropable->cbs_list)
          {
             cbs = EINA_INLIST_CONTAINER_GET(dropable->cbs_list, Dropable_Cbs);
             elm_drop_target_del(obj, cbs->types,
                                 cbs->entercb, cbs->enterdata, cbs->leavecb, cbs->leavedata,
                                 cbs->poscb, cbs->posdata, cbs->dropcb, cbs->dropdata);
             // If elm_drop_target_del() happened to delete dropabale, then
             // re-fetch it each loop to make sure it didn't
             dropable = efl_key_data_get(obj, "__elm_dropable");
             if (!dropable) break;
          }*/
     }
}

static void
_dropable_coords_adjust(Dropable *dropable, Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas *ee;
   Evas *evas = evas_object_evas_get(dropable->obj);
   int ex = 0, ey = 0, ew = 0, eh = 0;
   Evas_Object *win;

   ee = ecore_evas_ecore_evas_get(evas);
   ecore_evas_geometry_get(ee, &ex, &ey, &ew, &eh);
   *x = *x - ex;
   *y = *y - ey;

   /* For Wayland, frame coords have to be subtracted. */
   Evas_Coord fx, fy;
   evas_output_framespace_get(evas, &fx, &fy, NULL, NULL);
   if (fx || fy) sel_debug("evas frame fx %d fy %d\n", fx, fy);
   *x = *x - fx;
   *y = *y - fy;

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
                   x2 = ew - *y;
                   y2 = *x;
                   break;
                case 180:
                   x2 = ew - *x;
                   y2 = eh - *y;
                   break;
                case 270:
                   x2 = *y;
                   y2 = eh - *x;
                   break;
                default:
                   x2 = *x;
                   y2 = *y;
                   break;
               }
             sel_debug("rotation %d, w %d, h %d - x:%d->%d, y:%d->%d\n",
                       rot, ew, eh, *x, x2, *y, y2);
             *x = x2;
             *y = y2;
          }
     }
}

static void
_x11_dnd_dropable_handle(Efl_Selection_Manager_Data *pd, Dropable *dropable, Evas_Coord x, Evas_Coord y, Efl_Selection_Action action)
{
   Dropable *d, *last_dropable = NULL;
   Eina_List *l;
   //Dropable_Cbs *cbs;
   Eina_Inlist *itr;

   EINA_LIST_FOREACH(pd->drops, l, d)
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
                  pos_data.x = x - ox;
                  pos_data.y = y - oy;
                  pos_data.item = NULL;
               }
             else
               {
                  Evas_Coord xret = 0, yret = 0;
                  Efl_Object *it = NULL;

                  if (dropable->item_func)
                    it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                             x, y, &xret, &yret);
                  pos_data.x = xret;
                  pos_data.y = yret;
                  pos_data.item = it;
               }
             pos_data.format = dropable->last.format;
             pos_data.action = action;
             EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
               {
                if (df->format & dropable->last.format)
                  {
                     efl_event_callback_call(dropable->obj, EFL_DND_EVENT_DRAG_POS, &pos_data);
                  }
               }
             /*EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                if ((cbs->types & dropable->last.format) && cbs->poscb)
                  cbs->poscb(cbs->posdata, dropable->obj, x - ox, y - oy, action);*/
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
                         efl_event_callback_call(dropable->obj, EFL_DND_EVENT_DRAG_ENTER, NULL);
                    }
                  EINA_INLIST_FOREACH_SAFE(last_dropable->format_list, itr, df)
                    {
                       if (df->format & last_dropable->last.format)
                         efl_event_callback_call(last_dropable->obj, EFL_DND_EVENT_DRAG_LEAVE, NULL);
                    }
                  /*EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                     if ((cbs->types & dropable->last.format) && cbs->entercb)
                       cbs->entercb(cbs->enterdata, dropable->obj);
                  EINA_INLIST_FOREACH_SAFE(last_dropable->cbs_list, itr, cbs)
                     if ((cbs->types & last_dropable->last.format) && cbs->leavecb)
                       cbs->leavecb(cbs->leavedata, last_dropable->obj);*/
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
                         efl_event_callback_call(last_dropable->obj, EFL_DND_EVENT_DRAG_LEAVE, NULL);
                    }
                  /*EINA_INLIST_FOREACH_SAFE(last_dropable->cbs_list, itr, cbs)
                     if ((cbs->types & last_dropable->last.format) && cbs->leavecb)
                       cbs->leavecb(cbs->leavedata, last_dropable->obj);*/
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
                  pos_data.x = x - ox;
                  pos_data.y = y - oy;
                  pos_data.item = NULL;
               }
             else
               {
                  Evas_Coord xret = 0, yret = 0;
                  Efl_Object *it = NULL;
                  if (dropable->item_func)
                    it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                             x, y, &xret, &yret);
                  pos_data.x = xret;
                  pos_data.y = yret;
                  pos_data.item = it;
               }
             pos_data.format = dropable->last.format;
             pos_data.action = action;
             EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
               {
                  if (df->format & dropable->last.format)
                    {
                         efl_event_callback_call(dropable->obj, EFL_DND_EVENT_DRAG_ENTER, NULL);
                         efl_event_callback_call(dropable->obj, EFL_DND_EVENT_DRAG_POS, &pos_data);
                    }
               }
             /*EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
               {
                  if (cbs->types & dropable->last.format)
                    {
                       if (cbs->entercb)
                          cbs->entercb(cbs->enterdata, dropable->obj);
                       if (cbs->poscb)
                          cbs->poscb(cbs->posdata, dropable->obj,
                                     x - ox, y - oy, action);
                    }
               }*/
          }
        else
          {
             sel_debug("both dropable & last_dropable are null\n");
          }
     }
}

static Dropable *
_x11_dropable_find(Efl_Selection_Manager_Data *pd, Ecore_X_Window win)
{
   Eina_List *l;
   Dropable *dropable;

   if (!pd->drops) return NULL;
   EINA_LIST_FOREACH(pd->drops, l, dropable)
     {
        if (_x11_xwin_get(dropable->obj) == win) return dropable;
     }
   return NULL;
}

static Evas *
_x11_evas_get_from_xwin(Efl_Selection_Manager_Data *pd, Ecore_X_Window win)
{
   /* Find the Evas connected to the window */
   Dropable *dropable = _x11_dropable_find(pd, win);
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

static Efl_Selection_Format
_dnd_types_to_format(Efl_Selection_Manager_Data *pd, const char **types, int ntypes)
{
   Efl_Selection_Format ret_type = 0;
   int i;
   for (i = 0; i < ntypes; i++)
     {
        Efl_Sel_Manager_Atom *atom = eina_hash_find(pd->types_hash, types[i]);
        if (atom) ret_type |= atom->format;
     }
   return ret_type;
}

static Eina_List *
_dropable_list_geom_find(Efl_Selection_Manager_Data *pd, Evas *evas, Evas_Coord px, Evas_Coord py)
{
   Eina_List *itr, *top_objects_list = NULL, *dropable_list = NULL;
   Evas_Object *top_obj;
   Dropable *dropable = NULL;

   if (!pd->drops) return NULL;

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
                  Dropable *d = NULL;
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

static Eina_Bool
_x11_dnd_enter(void *data, int etype EINA_UNUSED, void *ev)
{
   Efl_Selection_Manager_Data *pd = data;
   Ecore_X_Event_Xdnd_Enter *enter = ev;
   int i;
   Dropable *dropable;

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
   pd->savedtypes->ntypes = enter->num_types;
   free(pd->savedtypes->types);
   pd->savedtypes->types = malloc(sizeof(char *) * enter->num_types);
   if (!pd->savedtypes->types) return EINA_FALSE;

   for (i = 0; i < enter->num_types; i++)
     {
        pd->savedtypes->types[i] = eina_stringshare_add(enter->types[i]);
        sel_debug("Type is %s %p %p\n", enter->types[i],
                  pd->savedtypes->types[i], pd->text_uri);
        if (pd->savedtypes->types[i] == pd->text_uri)
          {
             /* Request it, so we know what it is */
             sel_debug("Sending uri request\n");
             pd->savedtypes->textreq = 1;
             ELM_SAFE_FREE(pd->savedtypes->imgfile, free);
             ecore_x_selection_xdnd_request(enter->win, pd->text_uri);
          }
     }

   /* FIXME: Find an object and make it current */
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_position(void *data, int etype EINA_UNUSED, void *ev)
{
   Efl_Selection_Manager_Data *pd = data;
   Ecore_X_Event_Xdnd_Position *pos = ev;
   Ecore_X_Rectangle rect = { 0, 0, 0, 0 };
   Dropable *dropable;
   Efl_Selection_Action act;

   sel_debug("In");
   /* Need to send a status back */
   /* FIXME: Should check I can drop here */
   /* FIXME: Should highlight widget */
   dropable = _x11_dropable_find(pd, pos->win);
   if (dropable)
     {
        Evas_Coord x, y, ox = 0, oy = 0;

        act = _x11_dnd_action_map(pos->action);
        x = pos->position.x;
        y = pos->position.y;
        _dropable_coords_adjust(dropable, &x, &y);
        Evas *evas = _x11_evas_get_from_xwin(pd, pos->win);
        Eina_List *dropable_list = evas ? _dropable_list_geom_find(pd, evas, x, y) : NULL;
        /* check if there is dropable (obj) can accept this drop */
        if (dropable_list)
          {
             Efl_Selection_Format saved_format = _dnd_types_to_format(pd, pd->savedtypes->types, pd->savedtypes->ntypes);
             Eina_List *l;
             Eina_Bool found = EINA_FALSE;

             EINA_LIST_FOREACH(dropable_list, l, dropable)
               {
                  //Dropable_Cbs *cbs;
                  Drop_Format *df;
                  Eina_Inlist *itr;
                  //EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                  EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
                    {
                       //Efl_Selection_Format common_fmt = saved_format & cbs->types;
                       Efl_Selection_Format common_fmt = saved_format & df->format;
                       if (common_fmt)
                         {
                            //We found a target that can accept this type of data
                            int i, min_index = SELECTION_N_ATOMS;
                            //We have to find the first atom that corresponds to one
                            //of the supported data types.
                            for (i = 0; i < pd->savedtypes->ntypes; i++)
                              {
                                 Efl_Sel_Manager_Atom *atom = eina_hash_find(pd->types_hash, pd->savedtypes->types[i]);
                                 if (atom && (atom->format & common_fmt))
                                   {
                                      int atom_idx = (atom - pd->atomlist);
                                      if (min_index > atom_idx) min_index = atom_idx;
                                   }
                              }
                            if (min_index != SELECTION_N_ATOMS)
                              {
                                 sel_debug("Found atom %s\n", pd->atomlist[min_index].name);
                                 found = EINA_TRUE;
                                 dropable->last.type = pd->atomlist[min_index].name;
                                 dropable->last.format = common_fmt;
                                 break;
                              }
                         }
                    }
                  if (found) break;
               }
             if (found)
               {
                  Dropable *d = NULL;
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
                  ecore_x_dnd_send_status(EINA_TRUE, EINA_FALSE, rect, pos->action);
                  sel_debug("dnd position %i %i %p\n", x - ox, y - oy, dropable);
                  _x11_dnd_dropable_handle(pd, dropable, x - ox, y - oy, act);
                  // CCCCCCC: call dnd exit on last obj if obj != last
                  // CCCCCCC: call drop position on obj
               }
             else
               {
                  //if not: send false status
                  ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, pos->action);
                  sel_debug("dnd position (%d, %d) not in obj\n", x, y);
                  _x11_dnd_dropable_handle(pd, NULL, 0, 0, act);
                  // CCCCCCC: call dnd exit on last obj
               }
             eina_list_free(dropable_list);
          }
        else
          {
             ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, pos->action);
             sel_debug("dnd position (%d, %d) has no drop\n", x, y);
             _x11_dnd_dropable_handle(pd, NULL, 0, 0, act);
          }
     }
   else
     {
        ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, pos->action);
     }
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_leave(void *data, int etype EINA_UNUSED, void *ev)
{
   Efl_Selection_Manager_Data *pd = data;
#ifdef DEBUGON
   sel_debug("Leave %x\n", ((Ecore_X_Event_Xdnd_Leave *)ev)->win);
#else
   (void)ev;
#endif
   _x11_dnd_dropable_handle(pd, NULL, 0, 0, EFL_SELECTION_ACTION_UNKNOWN);
   // CCCCCCC: call dnd exit on last obj if there was one
   // leave->win leave->source
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_drop(void *data, int etype EINA_UNUSED, void *ev)
{
   sel_debug("In");
   Efl_Selection_Manager_Data *pd = data;
   Ecore_X_Event_Xdnd_Drop *drop;
   Dropable *dropable = NULL;
   //Elm_Selection_Data ddata;
   Efl_Selection_Data ddata;
   Evas_Coord x = 0, y = 0;
   Efl_Selection_Action act = EFL_SELECTION_ACTION_UNKNOWN;
   Eina_List *l;
   //Dropable_Cbs *cbs;
   Eina_Inlist *itr;
   X11_Cnp_Selection *sel;
   Seat_Selection *seat_sel = NULL;

   drop = ev;

   sel_debug("drops %p (%d)\n", pd->drops, eina_list_count(pd->drops));
   if (!(dropable = _x11_dropable_find(pd, drop->win))) return EINA_TRUE;

   /* Calculate real (widget relative) position */
   // - window position
   // - widget position
   pd->savedtypes->x = drop->position.x;
   pd->savedtypes->y = drop->position.y;
   _dropable_coords_adjust(dropable, &pd->savedtypes->x, &pd->savedtypes->y);

   sel_debug("Drop position is %d,%d\n", pd->savedtypes->x, pd->savedtypes->y);

   EINA_LIST_FOREACH(pd->drops, l, dropable)
     {
        if (dropable->last.in)
          {
             evas_object_geometry_get(dropable->obj, &x, &y, NULL, NULL);
             pd->savedtypes->x -= x;
             pd->savedtypes->y -= y;
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
                  pd->savedtypes->imgfile ? "" : "not ",
                  pd->savedtypes->imgfile);
        if (pd->savedtypes->imgfile)
          {
             Drop_Format *df;

             if (!dropable->is_container)
               {
                  ddata.x = pd->savedtypes->x;
                  ddata.y = pd->savedtypes->y;
                  ddata.item = NULL;
               }
             else
               {
                  //for container
                  Efl_Object *it = NULL;
                  Evas_Coord x0 = 0, y0 = 0;
                  Evas_Coord xret = 0, yret = 0;

                  evas_object_geometry_get(dropable->obj, &x0, &y0, NULL, NULL);
                  if (dropable->item_func)
                    it = dropable->item_func(dropable->item_func_data, dropable->obj,
                                   pd->savedtypes->x + x0, pd->savedtypes->y + y0,
                                   &xret, &yret);
                  ddata.x = xret;
                  ddata.y = yret;
                  ddata.item = it;
               }
             ddata.action = act;

             EINA_INLIST_FOREACH_SAFE(dropable->format_list, itr, df)
               {
                  if (df->format & EFL_SELECTION_FORMAT_IMAGE)
                    {
                       sel_debug("Doing image insert (%s)\n", pd->savedtypes->imgfile);
                       ddata.format = EFL_SELECTION_FORMAT_IMAGE;
                       ddata.data = (char *)pd->savedtypes->imgfile;
                       ddata.len = strlen(ddata.data);
                       if (df->format & dropable->last.format)
                         efl_event_callback_call(dropable->obj, EFL_DND_EVENT_DRAG_DROP, &ddata);
                    }
                  else
                    {
                       sel_debug("Item doesn't support images... passing\n");
                    }
               }
             /*EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
               {
                  //If it's markup that also supports images
                  if (cbs->types & EFL_SELECTION_FORMAT_IMAGE)
                    {
                       sel_debug("Doing image insert (%s)\n", pd->savedtypes.imgfile);
                       ddata.format = EFL_SELECTION_FORMAT_IMAGE;
                       ddata.data = (char *)pd->savedtypes.imgfile;
                       ddata.len = strlen(ddata.data);
                       if ((cbs->types & dropable->last.format) && cbs->dropcb)
                         cbs->dropcb(cbs->dropdata, dropable->obj, &ddata);
                    }
                  else
                    {
                       sel_debug("Item doesn't support images... passing\n");
                    }
               }*/
             ecore_x_dnd_send_finished();
             ELM_SAFE_FREE(pd->savedtypes->imgfile, free);
             return EINA_TRUE;
          }
        else if (pd->savedtypes->textreq)
          {
             /* Already asked: Pretend we asked now, and paste immediately when
              * it comes in */
             pd->savedtypes->textreq = 0;
             ecore_x_dnd_send_finished();
             return EINA_TRUE;
          }
     }

   seat_sel =  _get_seat_selection(pd, 1);
   sel = seat_sel->sellist + EFL_SELECTION_TYPE_DND;
   sel_debug("doing a request then: %s\n", dropable->last.type);
   sel->xwin = drop->win;
   sel->request_obj = dropable->obj;
   sel->request_format = dropable->last.format;
   //sel->active = EINA_TRUE;
   sel->action = act;
   ecore_x_selection_xdnd_request(drop->win, dropable->last.type);

   return EINA_TRUE;
}


//drop side
EOLIAN static Eina_Bool
_efl_selection_manager_drop_target_add(Eo *obj, Efl_Selection_Manager_Data *pd, Efl_Object *target_obj, Efl_Selection_Format format, Efl_Input_Device *seat)
{
   ERR("In");
   Dropable *dropable = NULL;
   //Dropable_Cbs *cbs = NULL;
   Ecore_X_Window xwin = _x11_xwin_get(target_obj);
   Eina_List *l;
   Eina_Bool first = !pd->drops;
   Eina_Bool have_drops = EINA_FALSE;

   /* Is this the first? */
   EINA_LIST_FOREACH(pd->drops, l, dropable)
     {
        if (xwin == _x11_xwin_get(dropable->obj))
          {
             have_drops = EINA_TRUE;
             break;
          }
     }
   dropable = NULL; // In case of error, we don't want to free it

   /*cbs = calloc(1, sizeof(*cbs));
   if (!cbs) return EINA_FALSE;

   cbs->entercb = entercb;
   cbs->enterdata = enterdata;
   cbs->leavecb = leavecb;
   cbs->leavedata = leavedata;
   cbs->poscb = poscb;
   cbs->posdata = posdata;
   cbs->dropcb = dropcb;
   cbs->dropdata = dropdata;
   cbs->types = format;*/

   Drop_Format *df = calloc(1, sizeof(Drop_Format));
   if (!df) return EINA_FALSE;
   df->format = format;

   dropable = efl_key_data_get(obj, "__elm_dropable");
   if (!dropable)
     {
        /* Create new drop */
        dropable = calloc(1, sizeof(Dropable));
        if (!dropable) goto error;
        dropable->last.in = EINA_FALSE;
        pd->drops = eina_list_append(pd->drops, dropable);
        if (!pd->drops) goto error;
        dropable->obj = target_obj;
        efl_key_data_set(target_obj, "__elm_dropable", dropable);
     }
   //dropable->cbs_list = eina_inlist_append(dropable->cbs_list, EINA_INLIST_GET(cbs));
   dropable->format_list = eina_inlist_append(dropable->format_list, EINA_INLIST_GET(df));
   dropable->seat = seat;

   evas_object_event_callback_add(target_obj, EVAS_CALLBACK_DEL,
         _all_drop_targets_cbs_del, obj);
   if (!have_drops) ecore_x_dnd_aware_set(xwin, EINA_TRUE);

   pd->seat_id = 1;
   if (seat)
     {
        pd->seat_id = efl_input_device_seat_id_get(seat);
     }
   _seat_selection_init(pd, seat);

   /* TODO BUG: should handle dnd-aware per window, not just the first
    * window that requested it! */
   /* If not the first: We're done */
   if (!first) return EINA_TRUE;

   sel_debug("Adding drop target calls xwin=%#llx\n", (unsigned long long)xwin);
   pd->enter_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER,
                                           _x11_dnd_enter, pd);
   pd->leave_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_LEAVE,
                                           _x11_dnd_leave, pd);
   pd->pos_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION,
                                         _x11_dnd_position, pd);
   pd->drop_handler = ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP,
                                          _x11_dnd_drop, pd);
   return EINA_TRUE;
error:
   //free(cbs);
   free(df);
   free(dropable);
   return EINA_FALSE;
}

EOLIAN static void
_efl_selection_manager_drop_target_del(Eo *obj, Efl_Selection_Manager_Data *pd, Efl_Object *target_obj, Efl_Selection_Format format, Efl_Input_Device *seat)
{
   ERR("In");
   Eina_List *l;
   Ecore_X_Window xwin;
   Eina_Bool have_drops = EINA_FALSE;
   Dropable *dropable = NULL;

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
             pd->drops = eina_list_remove(pd->drops, dropable);
             efl_key_data_set(target_obj, "__elm_dropable", NULL);
             free(dropable);
             evas_object_event_callback_del(target_obj, EVAS_CALLBACK_DEL,
                   _all_drop_targets_cbs_del);
          }
     }

   if (!pd->drops) return;
   xwin = _x11_xwin_get(target_obj);
   EINA_LIST_FOREACH(pd->drops, l, dropable)
     {
        if (xwin == _x11_xwin_get(dropable->obj))
          {
             have_drops = EINA_TRUE;
             break;
          }
     }
   if (!have_drops) ecore_x_dnd_aware_set(xwin, EINA_FALSE);
   if (!pd->drops)
     {
        ELM_SAFE_FREE(pd->pos_handler, ecore_event_handler_del);
        ELM_SAFE_FREE(pd->drop_handler, ecore_event_handler_del);
        ELM_SAFE_FREE(pd->enter_handler, ecore_event_handler_del);
        ELM_SAFE_FREE(pd->leave_handler, ecore_event_handler_del);
     }
}

static int
_drop_item_container_cmp(const void *d1, const void *d2)
{
   const Item_Container_Drop_Info *di = d1;
   return (((uintptr_t)di->obj) - ((uintptr_t)d2));
}

static Eina_Bool
_drop_item_container_del(Efl_Selection_Manager_Data *pd, Efl_Object *cont, Eina_Bool full)
{
   Item_Container_Drop_Info *di = eina_list_search_unsorted(pd->cont_drop_list,
                                                            _drop_item_container_cmp, cont);

   if (di)
     {
        _all_drop_targets_cbs_del(NULL, NULL, cont, NULL);
        di->item_func_data = NULL;
        di->item_func = NULL;

        if (full)
          {
             pd->cont_drop_list = eina_list_remove(pd->cont_drop_list, di);
             free(di);
          }
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static void
_efl_selection_manager_drop_item_container_add(Eo *obj, Efl_Selection_Manager_Data *pd, Efl_Object *cont, Efl_Selection_Format format, void *item_func_data, Efl_Dnd_Item_Get item_func, Eina_Free_Cb item_func_free_cb, Efl_Input_Device *seat)
{
   ERR("In");
   Item_Container_Drop_Info *di;
   Dropable *dropable = NULL;

   if (_drop_item_container_del(pd, cont, EINA_FALSE))
     {
        di = eina_list_search_unsorted(pd->cont_drop_list, _drop_item_container_cmp, obj);
        if (!di) return;
     }
   else
     {
        di = calloc(1, sizeof(Item_Container_Drop_Info));
        if (!di) return;

        di->obj = obj;
        pd->cont_drop_list = eina_list_append(pd->cont_drop_list, di);
     }
   di->item_func = item_func;
   di->item_func_data = item_func_data;

   dropable = efl_key_data_get(cont, "__elm_dropable");
   if (!dropable)
     {
        dropable = calloc(1, sizeof(Dropable));
        if (!dropable) return;
        dropable->last.in = EINA_FALSE;
        pd->drops = eina_list_append(pd->drops, dropable);
        if (!pd->drops) return;
        dropable->obj = cont;
        efl_key_data_set(cont, "__elm_dropable", dropable);
     }
   dropable->is_container = EINA_TRUE;
   dropable->item_func = item_func;
   dropable->item_func_data = item_func_data;
   efl_selection_manager_drop_target_add(obj, cont, format, seat);
}

EOLIAN static void
_efl_selection_manager_drop_item_container_del(Eo *obj, Efl_Selection_Manager_Data *pd, Efl_Object *cont, Efl_Input_Device *seat)
{
   ERR("In");
   _drop_item_container_del(pd, cont, EINA_TRUE);
}

static inline Eina_List *
_anim_icons_make(Drag_Container *dc)
{
   Eina_List *list = NULL, *icon_list = NULL;
   Evas_Object *obj;

   if (dc->icon_list_func)
     {
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

        evas_object_geometry_get(obj, &ai->start_x, &ai->start_y, &ai->start_w, &ai->start_h);
        evas_object_show(obj);
        ai->obj = obj;
        list = eina_list_append(list, ai);
     }
   ERR("made icon list");

   return list;
}

static void
_cont_obj_drag_done_cb(void *data, const Efl_Event *ev)
{
   Drag_Container *dc = data;
   elm_widget_scroll_freeze_pop(dc->cont);
}

static Eina_Bool
_cont_obj_drag_start(void *data)
{
   Seat_Selection *seat_sel = data;
   Drag_Container *dc = seat_sel->drag_cont;

   dc->timer = NULL;
   efl_event_callback_add(dc->cont, EFL_DND_EVENT_DRAG_DONE, _cont_obj_drag_done_cb, dc);
   elm_widget_scroll_freeze_push(dc->cont);
   efl_selection_manager_drag_start(seat_sel->pd->sel_man, dc->cont, dc->format,
                                    dc->buf, dc->len, dc->action,
                                    dc->icon_func_data, dc->icon_func, dc->icon_func_free_cb,
                                    dc->seat);
   //ELM_SAFE_FREE(dc->buf, free);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_drag_anim_play(void *data, double pos)
{
   Seat_Selection *seat_sel = data;
   Drag_Container *dc = seat_sel->drag_cont;

   ERR("In");
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

             _cont_obj_drag_start(seat_sel);
             return ECORE_CALLBACK_CANCEL;
          }

        evas_pointer_canvas_xy_get(dc->e, &xm, &ym);
        EINA_LIST_FOREACH(dc->icons, l, ai)
          {
             int x, y, w, h;
             w = ai->start_w - ((dc->final_icon_w - ai->start_w) * pos);
             h = ai->start_h - ((dc->final_icon_h - ai->start_h) * pos);
             x = ai->start_x - (pos * (ai->start_x + (w / 2) - xm));
             y = ai->start_y - (pos * (ai->start_y + (h / 2) - ym));
             evas_object_move(ai->obj, x, y);
             evas_object_resize(ai->obj, w, h);
          }

        return ECORE_CALLBACK_RENEW;
     }

   return ECORE_CALLBACK_CANCEL;
}

static inline void
_drag_anim_start(Seat_Selection *seat_sel)
{
   ERR("In");
   Drag_Container *dc = seat_sel->drag_cont;

   dc->timer = NULL;
   if (dc->icon_func)
     {
        Evas_Object *temp_win = elm_win_add(NULL, "Temp", ELM_WIN_DND);
        Evas_Object *final_icon = dc->icon_func(dc->icon_func_data, temp_win, NULL, NULL);
        evas_object_geometry_get(final_icon, NULL, NULL, &dc->final_icon_w, &dc->final_icon_h);
        evas_object_del(final_icon);
        evas_object_del(temp_win);
     }
   dc->animator = ecore_animator_timeline_add(dc->anim_duration, _drag_anim_play, seat_sel);
}

static Eina_Bool
_cont_obj_anim_start(void *data)
{
   sel_debug("In");
   Seat_Selection *seat_sel = data;
   Drag_Container *dc = seat_sel->drag_cont;
   Efl_Object *it = NULL;
   int xret, yret; //does not use

   if (dc->item_get_func)
     {
        it = dc->item_get_func(dc->item_get_func_data, dc->cont, dc->down_x, dc->down_y, &xret, &yret);
     }
   dc->timer = NULL;
   dc->format = EFL_SELECTION_FORMAT_TARGETS; //default
   dc->buf = NULL;
   dc->len = 0;
   dc->action = EFL_SELECTION_ACTION_COPY; //default
   dc->icons = NULL;

   //failed to get mouse-down item, abort drag
   if (!it)
     return ECORE_CALLBACK_CANCEL;

   if (dc->drag_data_func)
     {
        dc->drag_data_func(dc->drag_data_func_data, dc->cont,
                           &dc->format, &dc->buf, &dc->len, &dc->action);
        if (EINA_DBL_EQ(dc->anim_duration, 0.0))
          {
             _cont_obj_drag_start(seat_sel);
          }
        else
          {
             dc->icons = _anim_icons_make(dc);
             if (dc->icons)
               {
                  _drag_anim_start(seat_sel);
               }
             else
               {
                  // even if we don't manage the icons animation, we have
                  // to wait until it is finished before beginning drag.
                  dc->timer = ecore_timer_add(dc->anim_duration,
                                              _cont_obj_drag_start, seat_sel);
               }
          }
     }

   return ECORE_CALLBACK_CANCEL;
}

static void _cont_obj_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _cont_obj_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _item_container_del_internal(Seat_Selection *seat_sel, Eina_Bool full);
static void _anim_data_free(Seat_Selection *seat_sel);

static void
_abort_drag(Evas_Object *obj, Seat_Selection *seat_sel)
{
   Drag_Container *dc = seat_sel->drag_cont;
   evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_MOVE,
                                       _cont_obj_mouse_move_cb, seat_sel);
   evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_UP,
                                       _cont_obj_mouse_up_cb, seat_sel);
   _item_container_del_internal(seat_sel, EINA_FALSE);

   ELM_SAFE_FREE(dc->timer, ecore_timer_del);
   _anim_data_free(seat_sel);
}

static void
_cont_obj_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Seat_Selection *seat_sel = data;
   Drag_Container *dc = seat_sel->drag_cont;
   Evas_Event_Mouse_Move *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        _abort_drag(obj, seat_sel);
        //free(seat_sel->drag_cont);
        //seat_sel->drag_cont = NULL;
     }
   if (seat_sel->drag_cont &&
       (evas_device_class_get(ev->dev) == EVAS_DEVICE_CLASS_TOUCH))
     {
        int dx, dy;
        int fs = elm_config_finger_size_get();

        dx = ev->cur.canvas.x - dc->down_x;
        dy = ev->cur.canvas.y - dc->down_y;
        if ((dx * dx + dy * dy) > (fs * fs))
          {
             sel_debug("mouse moved too much - have to cancel DnD");
             _abort_drag(obj, seat_sel);
             //free(seat_sel->drag_cont);
             //seat_sel->drag_cont = NULL;
          }
     }
}

static void
_anim_data_free(Seat_Selection *seat_sel)
{
   Drag_Container *dc = seat_sel->drag_cont;
   if (dc)
     {
        ELM_SAFE_FREE(dc->animator, ecore_animator_del);
        Anim_Icon *ai;
        Eo *icon;

        EINA_LIST_FREE(dc->icons, ai)
          {
             evas_object_del(ai->obj);
             free(ai);
          }
        dc->icons = NULL;
        //free(dc);
        //seat_sel->drag_cont = NULL;
     }
}

static void
_cont_obj_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Seat_Selection *seat_sel = data;
   Drag_Container *dc = seat_sel->drag_cont;

   if (((Evas_Event_Mouse_Up *)event_info)->button != 1)
     return;

   evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_MOVE,
                                       _cont_obj_mouse_move_cb, seat_sel);
   evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_UP,
                                       _cont_obj_mouse_up_cb, seat_sel);
   ELM_SAFE_FREE(dc->timer, ecore_timer_del);

   _anim_data_free(seat_sel);
}

static void
_cont_obj_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Seat_Selection *seat_sel = data;
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->button != 1)
     return;

   Drag_Container *dc = seat_sel->drag_cont;
   dc->e = e;
   dc->down_x = ev->canvas.x;
   dc->down_y = ev->canvas.y;

   evas_object_event_callback_add(dc->cont, EVAS_CALLBACK_MOUSE_UP,
                                  _cont_obj_mouse_up_cb, seat_sel);
   ecore_timer_del(dc->timer);
   if (dc->time_to_drag)
     {
        dc->timer = ecore_timer_add(dc->time_to_drag, _cont_obj_anim_start, seat_sel);
        evas_object_event_callback_add(dc->cont, EVAS_CALLBACK_MOUSE_MOVE,
                                       _cont_obj_mouse_move_cb, seat_sel);
     }
   else
     {
        _cont_obj_anim_start(seat_sel);
     }
}

static void
_item_container_del_internal(Seat_Selection *seat_sel, Eina_Bool full)
{
   Drag_Container *dc = seat_sel->drag_cont;
   if (dc)
     {
        ELM_SAFE_FREE(dc->timer, ecore_timer_del);
        if (dc->animator)
          _anim_data_free(seat_sel);
        evas_object_event_callback_del_full(dc->cont, EVAS_CALLBACK_MOUSE_DOWN,
                                            _cont_obj_mouse_down_cb, seat_sel);
        if (full)
          {
             dc->item_get_func = NULL;
             dc->item_get_func_data = NULL;
             free(dc);
             seat_sel->drag_cont = NULL;
          }
     }
}

//the data is not ready at the time calling this func,
//therefore, must use function pointer to get it
EOLIAN static void
_efl_selection_manager_drag_item_container_add(Eo *obj, Efl_Selection_Manager_Data *pd,
               Efl_Object *cont, double time_to_drag, double anim_duration,
               void *data_func_data, Efl_Dnd_Drag_Data_Get data_func, Eina_Free_Cb data_func_free_cb,
               void *item_get_func_data, Efl_Dnd_Item_Get item_get_func, Eina_Free_Cb item_get_func_free_cb,
               void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb  icon_func_free_cb,
               void *icon_list_func_data, Efl_Dnd_Drag_Icon_List_Create icon_list_func, Eina_Free_Cb icon_list_func_free_cb,
               Efl_Input_Device *seat)
{
   ERR("In");
   //TODO: remove previous drag one
   Drag_Container *dc = calloc(1, sizeof(Drag_Container));
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

   Seat_Selection *seat_sel = _seat_selection_init(pd, seat);
   seat_sel->drag_cont = dc;

   evas_object_event_callback_add(cont, EVAS_CALLBACK_MOUSE_DOWN,
                                  _cont_obj_mouse_down_cb, seat_sel);
}

//FIXME: 1 seat can have multiple drag_containers
EOLIAN static void
_efl_selection_manager_drag_item_container_del(Eo *obj, Efl_Selection_Manager_Data *pd, Efl_Object *cont, Efl_Input_Device *seat)
{
   Seat_Selection *seat_sel = _seat_selection_init(pd, seat);
   if (seat_sel)
     _item_container_del_internal(seat_sel, EINA_TRUE);
}

static Efl_Object *
_efl_selection_manager_efl_object_constructor(Eo *obj, Efl_Selection_Manager_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->sel_man = obj;
   pd->atomlist = calloc(1, SELECTION_N_ATOMS * sizeof(Efl_Sel_Manager_Atom));
   if (!pd->atomlist)
     {
        ERR("failed to allocate atomlist");
        return NULL;
     }
   pd->atomlist[SELECTION_ATOM_TARGETS].name = "TARGETS";
   pd->atomlist[SELECTION_ATOM_TARGETS].format = EFL_SELECTION_FORMAT_TARGETS;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_TARGETS].x_converter = _x11_targets_converter;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_TARGETS].wl_converter = _wl_targets_converter;
#endif
   pd->atomlist[SELECTION_ATOM_ATOM].name = "ATOM"; // for opera browser
   pd->atomlist[SELECTION_ATOM_ATOM].format = EFL_SELECTION_FORMAT_TARGETS;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_ATOM].x_converter = _x11_targets_converter;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_ATOM].wl_converter = _wl_targets_converter;
#endif

   pd->atomlist[SELECTION_ATOM_ELM].name = "application/x-elementary-markup";
   pd->atomlist[SELECTION_ATOM_ELM].format = EFL_SELECTION_FORMAT_MARKUP;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_ELM].x_converter = _x11_general_converter;
   pd->atomlist[SELECTION_ATOM_ELM].x_data_preparer = _x11_data_preparer_markup;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_ELM].wl_converter = _wl_general_converter;
   pd->atomlist[SELECTION_ATOM_ELM].wl_data_preparer = _wl_data_preparer_markup;
#endif

   pd->atomlist[SELECTION_ATOM_TEXT_URILIST].name = "text/uri-list";
   pd->atomlist[SELECTION_ATOM_TEXT_URILIST].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_TEXT_URILIST].x_converter = _x11_general_converter;
   pd->atomlist[SELECTION_ATOM_TEXT_URILIST].x_data_preparer = _x11_data_preparer_uri;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_TEXT_URILIST].wl_converter = _wl_general_converter;
   pd->atomlist[SELECTION_ATOM_TEXT_URILIST].wl_data_preparer = _wl_data_preparer_uri;
#endif

   pd->atomlist[SELECTION_ATOM_TEXT_X_VCARD].name = "text/x-vcard";
   pd->atomlist[SELECTION_ATOM_TEXT_X_VCARD].format = EFL_SELECTION_FORMAT_VCARD;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_TEXT_X_VCARD].x_converter = _x11_vcard_send;
   pd->atomlist[SELECTION_ATOM_TEXT_X_VCARD].x_data_preparer = _x11_data_preparer_vcard;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_TEXT_X_VCARD].wl_data_preparer = _wl_data_preparer_vcard;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_PNG].name = "image/png";
   pd->atomlist[SELECTION_ATOM_IMAGE_PNG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_PNG].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_PNG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_PNG].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_JPEG].name = "image/jpeg";
   pd->atomlist[SELECTION_ATOM_IMAGE_JPEG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_JPEG].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_JPEG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_JPEG].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_BMP].name = "image/x-ms-bmp";
   pd->atomlist[SELECTION_ATOM_IMAGE_BMP].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_BMP].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_BMP].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_BMP].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_GIF].name = "image/gif";
   pd->atomlist[SELECTION_ATOM_IMAGE_GIF].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_GIF].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_GIF].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_GIF].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_TIFF].name = "image/tiff";
   pd->atomlist[SELECTION_ATOM_IMAGE_TIFF].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_TIFF].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_TIFF].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_TIFF].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_SVG].name = "image/svg+xml";
   pd->atomlist[SELECTION_ATOM_IMAGE_SVG].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_SVG].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_SVG].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_SVG].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_XPM].name = "image/x-xpixmap";
   pd->atomlist[SELECTION_ATOM_IMAGE_XPM].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_XPM].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_XPM].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_XPM].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_TGA].name = "image/x-tga";
   pd->atomlist[SELECTION_ATOM_IMAGE_TGA].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_TGA].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_TGA].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_TGA].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_IMAGE_PPM].name = "image/x-portable-pixmap";
   pd->atomlist[SELECTION_ATOM_IMAGE_PPM].format = EFL_SELECTION_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_IMAGE_PPM].x_converter = _x11_image_converter;
   pd->atomlist[SELECTION_ATOM_IMAGE_PPM].x_data_preparer = _x11_data_preparer_image;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_IMAGE_PPM].wl_data_preparer = _wl_data_preparer_image;
#endif

   pd->atomlist[SELECTION_ATOM_UTF8STRING].name = "UTF8_STRING";
   pd->atomlist[SELECTION_ATOM_UTF8STRING].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_UTF8STRING].x_converter = _x11_text_converter;
   pd->atomlist[SELECTION_ATOM_UTF8STRING].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_UTF8STRING].wl_converter = _wl_text_converter;
   pd->atomlist[SELECTION_ATOM_UTF8STRING].wl_data_preparer = _wl_data_preparer_text,
#endif

   pd->atomlist[SELECTION_ATOM_STRING].name = "STRING";
   pd->atomlist[SELECTION_ATOM_STRING].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_STRING].x_converter = _x11_text_converter;
   pd->atomlist[SELECTION_ATOM_STRING].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_STRING].wl_converter = _wl_text_converter;
   pd->atomlist[SELECTION_ATOM_STRING].wl_data_preparer = _wl_data_preparer_text;
#endif

   pd->atomlist[SELECTION_ATOM_COMPOUND_TEXT].name = "COMPOUND_TEXT";
   pd->atomlist[SELECTION_ATOM_COMPOUND_TEXT].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_COMPOUND_TEXT].x_converter = _x11_text_converter;
   pd->atomlist[SELECTION_ATOM_COMPOUND_TEXT].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_COMPOUND_TEXT].wl_converter = _wl_text_converter;
   pd->atomlist[SELECTION_ATOM_COMPOUND_TEXT].wl_data_preparer = _wl_data_preparer_text;
#endif

   pd->atomlist[SELECTION_ATOM_TEXT].name = "TEXT";
   pd->atomlist[SELECTION_ATOM_TEXT].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_TEXT].x_converter = _x11_text_converter;
   pd->atomlist[SELECTION_ATOM_TEXT].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_TEXT].wl_converter = _wl_text_converter;
   pd->atomlist[SELECTION_ATOM_TEXT].wl_data_preparer = _wl_data_preparer_text;
#endif

   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].name = "text/plain;charset=utf-8";
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].x_converter = _x11_text_converter;
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].wl_converter = _wl_text_converter;
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN_UTF8].wl_data_preparer = _wl_data_preparer_text;
#endif

   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN].name = "text/plain";
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN].format = EFL_SELECTION_FORMAT_TEXT | EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_HTML;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN].x_converter = _x11_text_converter;
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN].x_data_preparer = _x11_data_preparer_text;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN].wl_converter = _wl_text_converter;
   pd->atomlist[SELECTION_ATOM_TEXT_PLAIN].wl_data_preparer = _wl_data_preparer_text;
#endif


#ifdef HAVE_ELEMENTARY_X
   int i;
   for (i = 0; i < SELECTION_N_ATOMS; i++)
      {
         pd->atomlist[i].x_atom = ecore_x_atom_get(pd->atomlist[i].name);
         ecore_x_selection_converter_atom_add
            (pd->atomlist[i].x_atom, pd->atomlist[i].x_converter);
      }
#endif

   /*pd->sellist = calloc(1, (EFL_SELECTION_TYPE_CLIPBOARD + 1) * sizeof(X11_Cnp_Selection));
   if (!pd->sellist)
     {
        ERR("cannot allocate sellist");
        return NULL;
     }
   pd->sellist[EFL_SELECTION_TYPE_PRIMARY].debug = "Primary";
   pd->sellist[EFL_SELECTION_TYPE_PRIMARY].ecore_sel = ECORE_X_SELECTION_PRIMARY;
   pd->sellist[EFL_SELECTION_TYPE_PRIMARY].set = ecore_x_selection_primary_set;
   pd->sellist[EFL_SELECTION_TYPE_PRIMARY].clear = ecore_x_selection_primary_clear;
   pd->sellist[EFL_SELECTION_TYPE_PRIMARY].request = ecore_x_selection_primary_request;

   pd->sellist[EFL_SELECTION_TYPE_SECONDARY].debug = "Secondary";
   pd->sellist[EFL_SELECTION_TYPE_SECONDARY].ecore_sel = ECORE_X_SELECTION_SECONDARY;
   pd->sellist[EFL_SELECTION_TYPE_SECONDARY].set = ecore_x_selection_secondary_set;
   pd->sellist[EFL_SELECTION_TYPE_SECONDARY].clear = ecore_x_selection_secondary_clear;
   pd->sellist[EFL_SELECTION_TYPE_SECONDARY].request = ecore_x_selection_secondary_request;

   pd->sellist[EFL_SELECTION_TYPE_DND].debug = "DnD";
   pd->sellist[EFL_SELECTION_TYPE_DND].ecore_sel = ECORE_X_SELECTION_PRIMARY;
   pd->sellist[EFL_SELECTION_TYPE_DND].request = ecore_x_selection_xdnd_request;

   pd->sellist[EFL_SELECTION_TYPE_CLIPBOARD].debug = "Clipboard";
   pd->sellist[EFL_SELECTION_TYPE_CLIPBOARD].ecore_sel = ECORE_X_SELECTION_CLIPBOARD;
   pd->sellist[EFL_SELECTION_TYPE_CLIPBOARD].set = ecore_x_selection_clipboard_set;
   pd->sellist[EFL_SELECTION_TYPE_CLIPBOARD].clear = ecore_x_selection_clipboard_clear;
   pd->sellist[EFL_SELECTION_TYPE_CLIPBOARD].request = ecore_x_selection_clipboard_request;
   */

   pd->savedtypes = calloc(1, sizeof(Saved_Type));
   pd->types_hash = eina_hash_string_small_new(NULL);
   for (i = 0; i < SELECTION_N_ATOMS; i++)
     {
        eina_hash_add(pd->types_hash, pd->atomlist[i].name, &pd->atomlist[i]);
     }
   pd->text_uri = eina_stringshare_add("text/uri-list");

   pd->notify_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,
                                                _efl_sel_manager_x11_selection_notify, pd);
   pd->clear_handler = ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR,
                                            _x11_selection_clear, pd);
   return obj;
}


static void
_efl_selection_manager_efl_object_destructor(Eo *obj, Efl_Selection_Manager_Data *pd)
{
   ecore_event_handler_del(pd->notify_handler);
   ecore_event_handler_del(pd->clear_handler);
   free(pd->atomlist);
   free(pd->savedtypes);
   eina_hash_free(pd->types_hash);
   eina_stringshare_del(pd->text_uri);

   efl_destructor(efl_super(obj, MY_CLASS));
}


static void
_set_selection_list(X11_Cnp_Selection *sellist, Seat_Selection *seat_sel)
{
   sellist[EFL_SELECTION_TYPE_PRIMARY].debug = "Primary";
   sellist[EFL_SELECTION_TYPE_PRIMARY].ecore_sel = ECORE_X_SELECTION_PRIMARY;
   sellist[EFL_SELECTION_TYPE_PRIMARY].set = ecore_x_selection_primary_set;
   sellist[EFL_SELECTION_TYPE_PRIMARY].clear = ecore_x_selection_primary_clear;
   sellist[EFL_SELECTION_TYPE_PRIMARY].request = ecore_x_selection_primary_request;
   sellist[EFL_SELECTION_TYPE_PRIMARY].seat_sel = seat_sel;

   sellist[EFL_SELECTION_TYPE_SECONDARY].debug = "Secondary";
   sellist[EFL_SELECTION_TYPE_SECONDARY].ecore_sel = ECORE_X_SELECTION_SECONDARY;
   sellist[EFL_SELECTION_TYPE_SECONDARY].set = ecore_x_selection_secondary_set;
   sellist[EFL_SELECTION_TYPE_SECONDARY].clear = ecore_x_selection_secondary_clear;
   sellist[EFL_SELECTION_TYPE_SECONDARY].request = ecore_x_selection_secondary_request;
   sellist[EFL_SELECTION_TYPE_SECONDARY].seat_sel = seat_sel;

   sellist[EFL_SELECTION_TYPE_DND].debug = "DnD";
   sellist[EFL_SELECTION_TYPE_DND].ecore_sel = ECORE_X_SELECTION_PRIMARY;
   sellist[EFL_SELECTION_TYPE_DND].request = ecore_x_selection_xdnd_request;
   sellist[EFL_SELECTION_TYPE_DND].seat_sel = seat_sel;

   sellist[EFL_SELECTION_TYPE_CLIPBOARD].debug = "Clipboard";
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].ecore_sel = ECORE_X_SELECTION_CLIPBOARD;
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].set = ecore_x_selection_clipboard_set;
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].clear = ecore_x_selection_clipboard_clear;
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].request = ecore_x_selection_clipboard_request;
   sellist[EFL_SELECTION_TYPE_CLIPBOARD].seat_sel = seat_sel;
}



#include "efl_selection_manager.eo.c"
