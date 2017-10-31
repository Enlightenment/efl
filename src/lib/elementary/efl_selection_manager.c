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
static void _seat_selection_init(Efl_Selection_Manager_Data *pd, Efl_Input_Device *seat);


static Seat_Selection *
_get_seat_selection(Efl_Selection_Manager_Data *pd)
{
   Eina_List *l = NULL;
   Seat_Selection *sl = NULL;
   EINA_LIST_FOREACH(pd->seat_list, l, sl)
     {
        if (!strcmp(sl->seat_name, pd->seat))
          {
             ERR("Request seat: %s", sl->seat_name);
             break;
          }
     }
   if (!sl)
     {
        ERR("Could not find request seat");
     }

   return sl;
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
static Eina_Bool
_efl_sel_manager_x11_selection_notify(void *udata, int type EINA_UNUSED, void *event)
{
   Efl_Selection_Manager_Data *pd = udata;
   Ecore_X_Event_Selection_Notify *ev = event;
   X11_Cnp_Selection *sel;
   Seat_Selection *sl = NULL;
   int i;

   sl =  _get_seat_selection(pd);
   if (!sl)
     return EINA_FALSE;

   sel_debug("selection notify callback: %d",ev->selection);
   switch (ev->selection)
     {
      case ECORE_X_SELECTION_PRIMARY:
        sel = sl->sellist + EFL_SELECTION_TYPE_PRIMARY;
        break;
      case ECORE_X_SELECTION_SECONDARY:
        sel = sl->sellist + EFL_SELECTION_TYPE_SECONDARY;
        break;
      case ECORE_X_SELECTION_XDND:
        sel = sl->sellist + EFL_SELECTION_TYPE_DND;
        break;
      case ECORE_X_SELECTION_CLIPBOARD:
        sel = sl->sellist + EFL_SELECTION_TYPE_CLIPBOARD;
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
#if 0
                            Dropable *dropable;
                            Eina_List *l;
                            sel_debug("drag & drop\n");
                            EINA_LIST_FOREACH(drops, l, dropable)
                              {
                                 if (dropable->obj == sel->request_widget) break;
                                 dropable = NULL;
                              }
                            if (dropable)
                              {
                                 Dropable_Cbs *cbs;
                                 Eina_Inlist *itr;
                                 ddata.x = savedtypes->x;
                                 ddata.y = savedtypes->y;
                                 EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                                    if ((cbs->types & dropable->last.format) && cbs->dropcb)
                                       cbs->dropcb(cbs->dropdata, dropable->obj, &ddata);
                              }
#endif
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
   Seat_Selection *sl = NULL;
   unsigned int i;
   ERR("In");


   /*if (pd->promise)
     {
        Efl_Selection_Type *lt = malloc(sizeof(Efl_Selection_Type));
        *lt = pd->loss_type;
        efl_promise_value_set(pd->promise, lt, _selection_loss_data_clear_cb);
        pd->promise = NULL;
     }*/

   sl = _get_seat_selection(pd);
   if (!sl)
     return EINA_FALSE;

   for (i = ELM_SEL_TYPE_PRIMARY; i <= ELM_SEL_TYPE_CLIPBOARD; i++)
     {
        if (sl->sellist[i].ecore_sel == ev->selection) break;
     }
   sel_debug("selection %d clear", i);
   /* Not me... Don't care */
   if (i > ELM_SEL_TYPE_CLIPBOARD) return ECORE_CALLBACK_PASS_ON;

   sel = sl->sellist + i;

   efl_event_callback_call(sel->owner, EFL_SELECTION_EVENT_SELECTION_LOSS, NULL);
   sel->owner = NULL;

   return ECORE_CALLBACK_PASS_ON;
}

//note: sel_set: data is pd -> converter: data is pd
//->return data from converter: real buffer data ready to paste


static Efl_Selection_Format
_get_selection_type(void *data, Seat_Selection *sl)
{
   Efl_Selection_Manager_Data *pd = *(Efl_Selection_Manager_Data **)data;
   if (pd->has_sel)
     {
        sel_debug("has sel");
        if (pd->active_type > EFL_SELECTION_TYPE_CLIPBOARD)
          return EFL_SELECTION_FORMAT_NONE;
        sel_debug("has active type: %d, pd active_format: %d", pd->active_type, pd->active_format);
        if ((sl->sellist[pd->active_type].format >= EFL_SELECTION_FORMAT_TARGETS) &&
            (sl->sellist[pd->active_type].format <= EFL_SELECTION_FORMAT_HTML))
          return sl->sellist[pd->active_type].format;
     }
   sel_debug("has no sel");
   return EFL_SELECTION_FORMAT_NONE;
}

static Eina_Bool
_x11_general_converter(char *target EINA_UNUSED, void *data, int size EINA_UNUSED, void **data_ret, int *size_ret, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
{
   //Efl_Selection_Manager_Data *pd = *(Efl_Selection_Manager_Data **)data;
   //Seat_Selection *sl = _get_seat_selection(pd);

   X11_Cnp_Selection *sel = *(X11_Cnp_Selection **)data;
   //if (_get_selection_type(data, sl) == EFL_SELECTION_FORMAT_NONE)
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
        //X11_Cnp_Selection *sel = sl->sellist + pd->active_type;
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

   //Seat_Selection *sl = _get_seat_selection(pd);

   sel = *(X11_Cnp_Selection **)data;
   if (!sel) return EINA_FALSE;

   sel_debug("text converter");
   //if (_get_selection_type(data, sl) == EFL_SELECTION_FORMAT_NONE)
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

   //sel = &sl->sellist[pd->active_type];

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
        sel_debug("Image %s", evas_object_type_get(sel->widget));
        sel_debug("Elm type: %s", elm_object_widget_type_get(sel->widget));
        evas_object_image_file_get(elm_photocam_internal_image_get(sel->widget),
                                   (const char **)data_ret, NULL);
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

   pd->active_type = type;
   sel_debug("pd active_type: %d, active_format: %d", pd->active_type, pd->active_format);
   pd->active_format = format;
   X11_Cnp_Selection *sel = seat_sel->sellist + type;
   //TODO: change to sel
   if (seat_sel->sellist[type].selbuf)
     {
        free(seat_sel->sellist[type].selbuf);
     }
   seat_sel->sellist[type].selbuf = malloc(len);
   if (!seat_sel->sellist[type].selbuf)
     {
        ERR("failed to allocate buf");
        return;
     }
   seat_sel->sellist[type].selbuf = memcpy(seat_sel->sellist[type].selbuf, buf, len);
   seat_sel->sellist[type].len = len;
   seat_sel->sellist[type].format = format;

   //set selection
   //seat_sel->sellist[type].set(xwin, &pd, sizeof(&pd));
   //sel_debug("data: %p (%ld)", &pd, sizeof(&pd));
   seat_sel->sellist[type].set(xwin, &sel, sizeof(&sel));
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

   const char *seat_name = NULL;

   if (!seat)
     {
        seat_name = "default";
     }
   else
     {
        seat_name = efl_name_get(seat);
        if (!seat_name) seat_name = "default";
     }
   pd->seat = seat_name;
   pd->active_type = type;
   pd->has_sel = EINA_TRUE;

   _seat_selection_init(pd, seat);


#ifdef HAVE_ELEMENTARY_X
   //X11_Cnp_Selection *sel = pd->sellist + type;
   Seat_Selection *seat_sel = _get_seat_selection(pd);
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
   const char *seat_name = NULL;

   if (!seat)
     {
        seat_name = "default";
     }
   else
     {
        seat_name = efl_name_get(seat);
        if (!seat_name) seat_name = "default";
     }
   pd->seat = seat_name;

   _seat_selection_init(pd, seat);

   Seat_Selection *seat_sel = _get_seat_selection(pd);
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

   if (type > EFL_SELECTION_TYPE_CLIPBOARD)
     {
        ERR("Not supported type: %d", type);
        return;
     }

   const char *seat_name = NULL;

   if (!seat)
     {
        seat_name = "default";
     }
   else
     {
        seat_name = efl_name_get(seat);
        if (!seat_name) seat_name = "default";
     }

   Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;
   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if (!strcmp(seat_sel->seat_name, seat_name))
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
        seat_sel->seat_name = seat_name;
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
        evas_object_del(data);
        return ECORE_CALLBACK_CANCEL;
     }
   else
     {
        int x, y;
        x = pd->drag_win_x_end - (pos * (pd->drag_win_x_end - pd->drag_win_x_start));
        y = pd->drag_win_y_end - (pos * (pd->drag_win_y_end - pd->drag_win_y_start));
        evas_object_move(data, x, y);
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
        //if (dragdonecb) dragdonecb(dragdonedata, dragwidget); //TODO: change to event
        if (pd->drag_win)
          {
             if (pd->drag_obj)
               {
                  if (elm_widget_is(pd->drag_obj))
                    {
                       Evas_Object *win = elm_widget_top_get(pd->drag_obj);
                       if (win && efl_isa(win, EFL_UI_WIN_CLASS))
                         efl_event_callback_del(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED, _x11_win_rotation_changed_cb, pd->drag_win);
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
               }

             pd->drag_win = NULL;  /* if not freed here, free in end of anim */
          }

        pd->drag_obj = NULL;
        pd->accept = EINA_FALSE;
        /*  moved to _drag_cancel_animate
        if (dragwin)
          {
             evas_object_del(dragwin);
             dragwin = NULL;
          }
          */
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
   //if (dragposcb)
   //  dragposcb(dragposdata, dragwidget, pos->position.x, pos->position.y,
   //            dragaction);
   //TODO: change to event
}

static void
_x11_drag_target_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *info EINA_UNUSED)
{
   Efl_Selection_Manager_Data *pd = data;
   //X11_Cnp_Selection *sel = _x11_selections + ELM_SEL_TYPE_XDND;
   Seat_Selection *sl = _get_seat_selection(pd);
   X11_Cnp_Selection *sel = &sl->sellist[pd->active_type];

   if (pd->drag_obj == obj)
     {
        sel->widget = NULL;
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
   //if (dragacceptcb)
   //  dragacceptcb(dragacceptdata, _x11_selections[ELM_SEL_TYPE_XDND].widget,
   //               pd->accept);
   //TODO: change to event
   return EINA_TRUE;
}


static void
_seat_selection_init(Efl_Selection_Manager_Data *pd, Efl_Input_Device *seat)
{
   const char *seat_name = NULL;

   if (!seat)
     {
        seat_name = "default";
     }
   else
     {
        seat_name = efl_name_get(seat);
        if (!seat_name) seat_name = "default";
     }
   ERR("seat name: %s", seat_name);

   Seat_Selection *seat_sel = NULL;
   Eina_List *l = NULL;
   EINA_LIST_FOREACH(pd->seat_list, l, seat_sel)
     {
        if (!strcmp(seat_sel->seat_name, seat_name))
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
        seat_sel->seat_name = seat_name;
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
             return;
          }
        _set_selection_list(seat_sel->sellist, seat_sel);
     }
}

EOLIAN static void
_efl_selection_manager_drag_start(Eo *obj, Efl_Selection_Manager_Data *pd, Efl_Object *drag_obj, Efl_Selection_Format format, const void *buf, int len, Efl_Dnd_Drag_Action action, void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb icon_func_free_cb, Efl_Input_Device *seat)
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

   const char *seat_name = NULL;

   if (!seat)
     {
        seat_name = "default";
     }
   else
     {
        seat_name = efl_name_get(seat);
        if (!seat_name) seat_name = "default";
     }
   pd->seat = seat_name;
   pd->active_type = EFL_SELECTION_TYPE_DND;
   pd->has_sel = EINA_TRUE;

   //
   _seat_selection_init(pd, seat);

   Seat_Selection *sl = _get_seat_selection(pd);
   X11_Cnp_Selection *sel = &sl->sellist[pd->active_type];
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
   sel->widget = drag_obj;
   sel->format = format;
   sel->selbuf = buf ? strdup(buf) : NULL;
   sel->action = action;
   //dragwidget = obj;
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
   sel_debug("sizeof: %d %d", sizeof(&sel), sizeof(X11_Cnp_Selection));
   //actx = _x11_dnd_action_rev_map(dragaction); //TODO
   ecore_x_dnd_source_action_set(actx);
   ecore_x_pointer_grab(xwin);
   pd->mouse_up_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
                                        _x11_drag_mouse_up, pd);
                                        //(void *)(long)xwin);
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
             efl_event_callback_add(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED, _x11_win_rotation_changed_cb, pd->drag_win);
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
_efl_selection_manager_drag_cancel(Eo *obj, Efl_Selection_Manager_Data *pd)
{
   ERR("In");
}

EOLIAN static void
_efl_selection_manager_drag_action_set(Eo *obj, Efl_Selection_Manager_Data *pd , Efl_Dnd_Drag_Action action)
{
   ERR("In");
   //pd->action = action;
}


static Efl_Object *
_efl_selection_manager_efl_object_constructor(Eo *obj, Efl_Selection_Manager_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

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

   pd->atomlist[SELECTION_ATOM_XELM].name = "application/x-elementary-markup";
   pd->atomlist[SELECTION_ATOM_XELM].format = EFL_SELECTION_FORMAT_MARKUP;
#ifdef HAVE_ELEMENTARY_X
   pd->atomlist[SELECTION_ATOM_XELM].x_converter = _x11_general_converter;
   pd->atomlist[SELECTION_ATOM_XELM].x_data_preparer = _x11_data_preparer_markup;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   pd->atomlist[SELECTION_ATOM_XELM].wl_converter = _wl_general_converter;
   pd->atomlist[SELECTION_ATOM_XELM].wl_data_preparer = _wl_data_preparer_markup;
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
