/*
 * todo
 *  - move to stringshare for stuff when I can
 */

#include <stdbool.h>
#include <sys/mman.h>

#include <stdio.h> // debug

#include <Elementary.h>

#include "elm_priv.h"

#ifdef HAVE_ELEMENTARY_X

# include <X11/X.h>
# include <X11/Xatom.h>


# define ARRAYINIT(foo)  [foo]=

#define DEBUGON	1

#if DEBUGON
#define cnp_debug(x...) printf(__FILE__": " x)
#else
#define cnp_debug(x...)
#endif


#define PROVIDER_SET "__elm_cnp_provider_set"

enum {
     CNP_ATOM_TARGETS = 0,
     CNP_ATOM_text_uri,
     CNP_ATOM_text_urilist,
     CNP_ATOM_image_png,
     CNP_ATOM_image_jpeg,
     CNP_ATOM_XELM,
     CNP_ATOM_text_html_utf8,
     CNP_ATOM_text_html,
     CNP_ATOM_UTF8STRING,
     CNP_ATOM_STRING,
     CNP_ATOM_TEXT,
     CNP_ATOM_text_plain_utf8,
     CNP_ATOM_text_plain,

     CNP_N_ATOMS,
};
struct pasteimage {
     Evas_Object *entry;
     const char *tag;
     const char *file;
     Evas_Object *img;
};


struct _elm_cnp_selection {
   const char *debug;
   Evas_Object *widget;

   Elm_Sel_Format format;
   char *selbuf;

   unsigned int active : 1;

   Ecore_X_Selection ecore_sel;

   Evas_Object *requestwidget;
   Elm_Sel_Format requestformat;

   int (*set)(Ecore_X_Window, const void *data, int size);
   int (*clear)(void);
   void (*request)(Ecore_X_Window, const char *target);
};

/* Optimisation: Turn this into a 256 byte table:
 *	then can lookup in one index, not N checks */
static const struct escapes {
   const char *escape;
   const char value;
} escapes[] = {
   { "<br>",	'\n' },
   { "<\t>",	'\t' },
   { "gt;",	'>' },
   { "lt;",	'<' },
   { "amp;",'&' },
   { "quot;",'\'' },
   { "dquot;", '"' },
};
#define N_ESCAPES ((int)(sizeof(escapes)/sizeof(escapes[0])))


static Eina_Bool _elm_cnp_init(void);
static Eina_Bool selection_clear(void *udata __UNUSED__, int type, void *event);
//static Eina_Bool selection_request(void *udata __UNUSED__, int type, void *event);
static Eina_Bool selection_notify(void *udata __UNUSED__, int type, void *event);
static char *remove_tags(const char *p, int *len);
static char *mark_up(const char *start, int *lenp);

static Evas_Object *image_provider(void *images, Evas_Object *entry, const char *item);
static void
entry_deleted(void *images, Evas *e, Evas_Object *entry, void *unused);


typedef int (*converter_fn)(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);

static int targets_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static int text_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static int html_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static int edje_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static int uri_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static int png_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);

/* FIXME: Which way should this be: Notify or response */
typedef int (*response_handler)(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *);
static int response_handler_targets(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *);

typedef int (*notify_handler)(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *);
static int notify_handler_targets(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *notify);
static int notify_handler_text(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *notify);
static int notify_handler_png(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *notify);
static int notify_handler_uri(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *notify);

static struct pasteimage *pasteimage_alloc(const char *file);
static bool pasteimage_append(struct pasteimage *pi, Evas_Object *entry);
static void pasteimage_free(struct pasteimage *pi);

static struct {
   const char *name;
   Elm_Sel_Format formats;
   /* Called by ecore to do conversion */
   converter_fn converter;
   response_handler response;
   notify_handler notify;
   /* Atom */
   Ecore_X_Atom atom;
} atoms[CNP_N_ATOMS] = {
   [CNP_ATOM_TARGETS] = {
	"TARGETS",
	(Elm_Sel_Format)-1, // everything
	targets_converter,
	response_handler_targets,
	notify_handler_targets,
	0
   },
   [CNP_ATOM_XELM] =  {
	"application/x-elementary-markup",
	ELM_SEL_FORMAT_MARKUP,
	edje_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_text_uri] = {
	"text/uri",
	ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE, /* Either images or entries */
	uri_converter,
	NULL,
	notify_handler_uri,
	0
   },
   [CNP_ATOM_text_urilist] = {
        "text/uri-list",
        ELM_SEL_FORMAT_IMAGE | ELM_SEL_FORMAT_MARKUP,
        uri_converter,
        NULL,
        notify_handler_uri,
        0
   },
   [CNP_ATOM_image_png] = {
	"image/png",
	ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE,
	png_converter,
	NULL,
	notify_handler_png,
	0
   },
   [CNP_ATOM_image_jpeg] = {
	"image/jpeg",
	ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE,
	png_converter,
	NULL,
	notify_handler_png,/* Raw image data is the same */
	0
   },
   [CNP_ATOM_text_html_utf8] = {
	"text/html;charset=utf-8",
	ELM_SEL_FORMAT_MARKUP,
	html_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_text_html] = {
	"text/html",
	ELM_SEL_FORMAT_MARKUP,
	html_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_UTF8STRING] = {
	"UTF8_STRING",
	ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP,
	text_converter,
	NULL,
	notify_handler_text,
	0
   },
   [CNP_ATOM_STRING] = {
	"STRING",
	ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP,
	text_converter,
	NULL,
	notify_handler_text,
	0
   },
   [CNP_ATOM_TEXT] = {
	"TEXT",
	ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP,
	text_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_text_plain_utf8] = {
	"text/plain;charset=ut-8",
	ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP,
	text_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_text_plain] = {
	"text/plain",
	ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP,
	text_converter,
	NULL,
	NULL,
	0
   },
};

static struct _elm_cnp_selection selections[ELM_SEL_MAX] = {
   ARRAYINIT(ELM_SEL_PRIMARY) {
       .debug = "Primary",
       .ecore_sel = ECORE_X_SELECTION_PRIMARY,
       .set = ecore_x_selection_primary_set,
       .clear = ecore_x_selection_primary_clear,
       .request = ecore_x_selection_primary_request,
   },
   ARRAYINIT(ELM_SEL_SECONDARY) {
       .debug = "Secondary",
       .ecore_sel = ECORE_X_SELECTION_SECONDARY,
       .set = ecore_x_selection_secondary_set,
       .clear = ecore_x_selection_secondary_clear,
       .request = ecore_x_selection_secondary_request,
   },
   ARRAYINIT(ELM_SEL_CLIPBOARD) {
       .debug = "Clipboard",
       .ecore_sel = ECORE_X_SELECTION_CLIPBOARD,
       .set = ecore_x_selection_clipboard_set,
       .clear = ecore_x_selection_clipboard_clear,
       .request = ecore_x_selection_clipboard_request,
   },
   ARRAYINIT(ELM_SEL_XDND) {
       .debug = "XDnD",
       .ecore_sel = ECORE_X_SELECTION_XDND,
       .request = ecore_x_selection_xdnd_request,
   },
};

/* Data for DND in progress */
/* FIXME: BEtter name */
struct {
   int ntypes;
   const char **types;
   unsigned int textreq: 1;
   struct pasteimage *pi;
   int x,y;
} savedtypes =  { 0, NULL, 0, NULL, 0, 0 };

static void (*dragdonecb)(void *,Evas_Object*);
static void *dragdonedata;

static int _elm_cnp_init_count = 0;
  /* Gah... who left this out of XAtoms.h */
static Ecore_X_Atom clipboard_atom;

Eina_List *pastedimages;
Eina_List *providedobjs;
#endif

/* Stringshared, so I can just compare pointers later */
static const char *text_uri;

Eina_Bool
elm_selection_set(Elm_Sel_Type selection, Evas_Object *widget,
			Elm_Sel_Format format, const char *selbuf)
{
#ifdef HAVE_ELEMENTARY_X
   struct _elm_cnp_selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();
   if (!selbuf && format != ELM_SEL_FORMAT_IMAGE)
     return elm_selection_clear(selection, widget);

   sel = selections + selection;

   sel->active = 1;
   sel->widget = widget;

   sel->set(elm_win_xwindow_get(widget),&selection,sizeof(Elm_Sel_Type));
   sel->format = format;
   sel->selbuf = selbuf ? strdup(selbuf) : NULL;

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

Eina_Bool
elm_selection_clear(Elm_Sel_Type selection, Evas_Object *widget)
{
#ifdef HAVE_ELEMENTARY_X
   struct _elm_cnp_selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();

   sel = selections + selection;

   /* No longer this selection: Consider it gone! */
   if (!sel->active || sel->widget != widget) return EINA_TRUE;

   sel->active = 0;
   sel->widget = NULL;
   sel->clear();

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

Eina_Bool
elm_selection_get(Elm_Sel_Type selection, Elm_Sel_Format format,
			Evas_Object *widget)
{
#ifdef HAVE_ELEMENTARY_X
   Evas_Object *top;
   struct _elm_cnp_selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();

   sel = selections + selection;
   top = elm_widget_top_get(widget);
   if (!top) return EINA_FALSE;

   sel->requestformat = format;
   sel->requestwidget = widget;
   sel->request(elm_win_xwindow_get(top), ECORE_X_SELECTION_TARGET_TARGETS);

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

#ifdef HAVE_ELEMENTARY_X

static Eina_Bool
_elm_cnp_init(void){
   int i;
   if (_elm_cnp_init_count ++) return EINA_TRUE;

   /* FIXME: Handle XCB */
   for (i = 0 ; i < CNP_N_ATOMS ; i ++)
     {
	atoms[i].atom = ecore_x_atom_get(atoms[i].name);
	ecore_x_selection_converter_atom_add(atoms[i].atom,
	      atoms[i].converter);
     }
   clipboard_atom = ecore_x_atom_get("CLIPBOARD");

   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, selection_clear,NULL);
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,selection_notify,NULL);

   text_uri = eina_stringshare_add("text/uri-list");
   return EINA_TRUE;
}

static Eina_Bool
selection_clear(void *udata __UNUSED__, int type __UNUSED__, void *event){
   Ecore_X_Event_Selection_Clear *ev = event;
   struct _elm_cnp_selection *sel;
   int i;

   for (i = 0 ; i < ELM_SEL_MAX ; i ++)
     {
	if (selections[i].ecore_sel == ev->selection) break;
     }
cnp_debug("selection %d clear\n",i);
   /* Not me... Don't care */
   if (i == ELM_SEL_MAX) return ECORE_CALLBACK_PASS_ON;

   sel = selections + i;
   sel->active = 0;
   sel->widget = NULL;
   sel->selbuf = NULL;

   return ECORE_CALLBACK_PASS_ON;
}

#if 0
/**
 * Response to a selection request
 */
static Eina_Bool
selection_request(void *udata __UNUSED__, int type, void *event){
   Ecore_X_Event_Selection_Request *ev = event;
   struct _elm_cnp_selection *sel;
   int i;
printf("selection request callback: %d\n",ev->selection);
printf("selection request callback: %d\n",ev->target);

   if (ev->selection == clipboard_atom){
	 sel = selections + ELM_SEL_CLIPBOARD;
   } else if (ev->selection == XA_PRIMARY){
	 sel = selections + ELM_SEL_PRIMARY;
   } else if (ev->selection ==  XA_SECONDARY){
	 sel = selections + ELM_SEL_SECONDARY;
   } else {
	 return ECORE_CALLBACK_PASS_ON;
   }
   return ECORE_CALLBACK_PASS_ON;

   for (i = 0 ; i < CNP_N_ATOMS ; i ++)
     {
	if (ev->target == atoms[i].atom)
	  {
	     if (atoms[i].response){
		  atoms[i].response(sel, ev);
	     } else {
		  printf("Ignored: No handler!\n");
	     }
	  }
     }

   return ECORE_CALLBACK_PASS_ON;
}
#endif


/*
 * Response to a selection notify:
 *	- So we have asked for the selection list.
 *	- If it's the targets list, parse it, and fire of what we want,
 *	else it's the data we want.
 */
static Eina_Bool
selection_notify(void *udata __UNUSED__, int type __UNUSED__, void *event){
   Ecore_X_Event_Selection_Notify *ev = event;
   struct _elm_cnp_selection *sel;
   int i;

   cnp_debug("selection notify callback: %d\n",ev->selection);
   switch (ev->selection){
      case ECORE_X_SELECTION_CLIPBOARD:
	 sel = selections + ELM_SEL_CLIPBOARD;
	 break;
      case ECORE_X_SELECTION_PRIMARY:
	 sel = selections + ELM_SEL_PRIMARY;
	 break;
      case ECORE_X_SELECTION_SECONDARY:
	 sel = selections + ELM_SEL_SECONDARY;
	 break;
      case ECORE_X_SELECTION_XDND:
         sel = selections + ELM_SEL_XDND;
         break;
      default:
	 return ECORE_CALLBACK_PASS_ON;
   }
   cnp_debug("Target is %s\n",ev->target);

   for (i = 0 ; i < CNP_N_ATOMS ; i ++)
     {
	if (strcmp(ev->target, atoms[i].name) == 0)
	  {
	     if (atoms[i].notify){
		  cnp_debug("Found something: %s\n", atoms[i].name);
		  atoms[i].notify(sel, ev);
	     } else {
		  printf("Ignored: No handler!\n");
	     }
	  }
     }

   return ECORE_CALLBACK_PASS_ON;
}



static int
targets_converter(char *target __UNUSED__, void *data, int size __UNUSED__,
                  void **data_ret, int *size_ret,
                  Ecore_X_Atom *ttype, int *typesize){
   int i,count;
   Ecore_X_Atom *aret;
   struct _elm_cnp_selection *sel;

   if (!data_ret) return -1;

   sel = selections + *(int*)data;

   for (i = 0, count = 0 ; i < CNP_N_ATOMS ; i ++)
	if (sel->format & atoms[i].formats)
	   count ++;

   aret = malloc(sizeof(Ecore_X_Atom) * count);
   for (i = 0, count = 0 ; i < CNP_N_ATOMS ; i ++)
	if (sel->format & atoms[i].formats)
	  aret[count ++] = atoms[i].atom;

   *data_ret = aret;
   if (typesize) *typesize = 32 /* urk */;
   if (ttype) *ttype = XA_ATOM;
   if (size_ret) *size_ret = count;

   return 1;
}

static int
png_converter(char *target __UNUSED__, void *data, int size,
              void **data_ret __UNUSED__, int *size_ret __UNUSED__,
              Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   cnp_debug("Png converter called\n");
   return 1;
}

/*
 * Callback to handle a targets response on a selection request:
 * So pick the format we'd like; and then request it.
 */
static int
notify_handler_targets(struct _elm_cnp_selection *sel,
                       Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atomlist;
   int i,j;

   targets = notify->data;
   atomlist = (Ecore_X_Atom *)(targets->data.data);

   for (j = 1; j < CNP_N_ATOMS ; j ++)
     {
	cnp_debug("\t%s %d\n",atoms[j].name, atoms[j].atom);
	if (!(atoms[j].formats & sel->requestformat)) continue;
	for (i = 0 ; i < targets->data.length ; i ++)
	  {
	     if (atoms[j].atom == atomlist[i])
	       {
		  cnp_debug("Atom %s matches\n",atoms[j].name);
		  goto done;
	       }
	  }
     }

   cnp_debug("Couldn't find anything that matches\n");
   return ECORE_CALLBACK_PASS_ON;

   done:
   cnp_debug("Sending request for %s\n",atoms[j].name);
   sel->request(elm_win_xwindow_get(sel->requestwidget), atoms[j].name);

   return ECORE_CALLBACK_PASS_ON;
}

static int
response_handler_targets(struct _elm_cnp_selection *sel,
                         Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atomlist;
   Evas_Object *top;
   int i,j;
   int prio, selected;

   targets = notify->data;
   atomlist = (Ecore_X_Atom *)(targets->data.data);

   prio = -1;
   selected = -1;
   /* Start from 1: Skip targets */
   for (j = 1 ; j < CNP_N_ATOMS ; j ++)
     {
	if (!(atoms[j].formats & sel->requestformat)) continue;
	for (i = 0 ; i < targets->data.length ; i ++)
	  {
	     if (atoms[j].atom == atomlist[i] && atoms[j].response){
		  /* Found a match: Use it */
		  goto found;
	     }
	}
     }
	found:
   if (j == CNP_N_ATOMS)
     {
	cnp_debug("No matching type found\n");
	return 0;
     }

   top = elm_widget_top_get(sel->requestwidget);
   if (!top) return 0;

   sel->request(elm_win_xwindow_get(top), atoms[j].name);
   return 0;
}


static int
notify_handler_text(struct _elm_cnp_selection *sel,
                    Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   char *str;

   data = notify->data;
   cnp_debug("Notify handler text %d %d %p\n",data->format,data->length,data->data);
   str = mark_up((char*)data->data, NULL);
   cnp_debug("String is %s (from %s)\n",str,data->data);
   elm_entry_entry_insert(sel->requestwidget, str);
   free(str);

   return 0;
}


/**
 * So someone is pasting an image into my entry or widget...
 */
static int
notify_handler_uri(struct _elm_cnp_selection *sel,
	 Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   Ecore_X_Selection_Data_Files *files;
   struct pasteimage *pi;
   char *p;

   data = notify->data;
   printf("data->format is %d %p %p\n",data->format,notify,data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES)
     {
        cnp_debug("got a files list\n");
        files = notify->data;
        if (files->num_files > 1)
          {
             /* Don't handle many items */
             cnp_debug("more then one file: Bailing\n");
             return 0;
          }
        p = files->files[0];
     }
   else
      p = (char *)data->data;
   if (!p)
     {
        cnp_debug("Couldn't find a file\n");
        return 0;
      }
   cnp_debug("Got %s\n",p);
   if (strncmp(p,"file://",7) != 0)
     {
        /* Try and continue if it looks sane */
        if (*p != '/') return 0;
     }
   else
      p += strlen("file://");

   if (!strstr(p,".png") && !strstr(p,".jpg"))
     {
        /* FIXME: Better test: Load it in evasw & see is probably best */
        cnp_debug("No png, ignoring\n");
        if (savedtypes.textreq) savedtypes.textreq = 0;
        return 0;
     }

   if (savedtypes.pi) pasteimage_free(savedtypes.pi);

   pi = pasteimage_alloc(p);

   if (savedtypes.textreq)
     {
        savedtypes.textreq = 0;
        savedtypes.pi = pi;
     }
   else
     {
        pasteimage_append(pi, sel->requestwidget);
        savedtypes.pi = NULL;
     }

   return 0;
}

static int
notify_handler_png(struct _elm_cnp_selection *sel __UNUSED__,
                   Ecore_X_Event_Selection_Notify *notify __UNUSED__)
{
   Ecore_X_Selection_Data *data;
   char *fname,*tmppath;
   struct pasteimage *pi;
   int fd,len;
   void *map;

   cnp_debug("got a png (or a jpeg)!\n");
   data = notify->data;

   cnp_debug("Size if %d\n",data->length);

   /* generate tmp name */
   tmppath = getenv("TMP");
   if (!tmppath) tmppath = P_tmpdir;
   if (!tmppath) tmppath = "/tmp";
   len = snprintf(NULL,0,"%s/%sXXXXXX",tmppath, "elmcnpimage-");
   if (len < 0) return 1;
   len ++;
   fname = malloc(len);
   if (!fname) return 1;
   len = snprintf(fname,len,"%s/%sXXXXXX",tmppath, "elmcnpimage-");

   fd = mkstemp(fname);
   if (fd < 0)
     {
        free(fname);
        return 1;
     }

   if (ftruncate(fd, data->length))
     {
        perror("ftruncate");
        unlink(fname);
        free(fname);
        close(fd);
     }

   map = mmap(NULL,data->length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
   if (map == MAP_FAILED)
     {
        perror("mmap");
        unlink(fname);
        free(fname);
        close(fd);
     }
   memcpy(map, data->data, data->length);
   munmap(map,data->length);
   close(fd);

   /* FIXME: add clean up function */
   // on_exit: file name + pid
   //    need pid, as forked children inheret list :-(

   /* FIXME: Add to paste image data to clean up */

   pi = pasteimage_alloc(fname);
   pasteimage_append(pi, sel->requestwidget);

   return 0;
}


static int
text_converter(char *target __UNUSED__, void *data, int size __UNUSED__,
               void **data_ret, int *size_ret,
               Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   struct _elm_cnp_selection *sel;

   cnp_debug("text converter\n");
   sel = selections + *(int *)data;
   if (!sel->active) return 1;

   if (sel->format == ELM_SEL_FORMAT_MARKUP){
	*data_ret = remove_tags(sel->selbuf, size_ret);
   } else if (sel->format == ELM_SEL_FORMAT_IMAGE){
	cnp_debug("Image %s\n",evas_object_type_get(sel->widget));
	cnp_debug("Elm type: %s\n",elm_object_widget_type_get(sel->widget));
	evas_object_image_file_get(elm_photocam_internal_image_get(sel->widget), (const char **)data_ret, NULL);
	if (!*data_ret) *data_ret = strdup("No file");
	else *data_ret = strdup(*data_ret);
	*size_ret = strlen(*data_ret);
   }
   return 1;
}

static int
edje_converter(char *target __UNUSED__, void *data, int size __UNUSED__,
               void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__,
               int *typesize __UNUSED__)
{
   struct _elm_cnp_selection *sel;

   sel = selections + *(int *)data;
   if (data_ret) *data_ret = strdup(sel->selbuf);
   if (size_ret) *size_ret = strlen(sel->selbuf);

   return 1;
}


static int
html_converter(char *target __UNUSED__, void *data, int size __UNUSED__,
               void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__,
               int *typesize __UNUSED__)
{
   struct _elm_cnp_selection *sel;

   sel = selections + *(int *)data;
   if (data_ret) *data_ret = strdup(sel->selbuf);
   if (size_ret) *size_ret = strlen(sel->selbuf);

   return 1;
}

static int
uri_converter(char *target __UNUSED__, void *data, int size __UNUSED__,
              void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__,
              int *typesize __UNUSED__)
{
    struct _elm_cnp_selection *sel;
    sel = selections + *(int *)data;
    cnp_debug("Uri converter\n");
    if (data_ret) *data_ret = strdup(sel->selbuf);
    if (size_ret) *size_ret = strlen(sel->selbuf);
    return 1;
}


/*
 * Image paste provide
 */

/* FIXME: Should add provider for each pated item: Use data to store it
 * much easier */
static Evas_Object *
image_provider(void *images __UNUSED__, Evas_Object *entry, const char *item)
{
   struct pasteimage *pi;
   Eina_List *l;

   cnp_debug("image provider for %s called\n",item);
   EINA_LIST_FOREACH(pastedimages, l, pi)
     {
	cnp_debug("is it %s?\n",pi->tag);
	if (strcmp(pi->tag,item) == 0){
	     /* Found it */
	     Evas_Object *o;
	     o = evas_object_image_filled_add(evas_object_evas_get(entry));
	     /* FIXME: Handle eets */
	     cnp_debug("file is %s (object is %p)\n",pi->file,o);
	     evas_object_image_file_set(o, pi->file, NULL);
	     evas_object_show(o);
	     return o;
	}
     }
   return NULL;
}


static struct pasteimage *
pasteimage_alloc(const char *file)
{
   struct pasteimage *pi;
   int len;
   char *buf;

   pi = calloc(1,sizeof(struct pasteimage));
   if (!pi) return NULL;

   len = snprintf(NULL, 0, "pasteimage-%p",pi);
   len ++;
   buf = malloc(len);
   if (!buf)
     {
        free(pi);
        return NULL;
     }
   snprintf(buf, len, "pasteimage-%p",pi);
   pi->tag = buf;

   if (file)
     {
        if (strstr(file,"file://")) file += strlen("file://");
        pi->file = strdup(file);
     }

   return pi;
}

static void
pasteimage_free(struct pasteimage *pi)
{
   if (!pi) return;
   if (pi->file) free((void*)pi->file);
   if (pi->tag) free((void*)pi->tag);
   free(pi);
}

static bool
pasteimage_provider_set(Evas_Object *entry)
{
   void *v;
   const char *type;
   if (!entry) return false;
   type = elm_widget_type_get(entry);
   printf("type is %s\n",type);
   if (!type || strcmp(type,"entry") != 0) return false;

   v = evas_object_data_get(entry, PROVIDER_SET);
   if (!v)
     {
        evas_object_data_set(entry, PROVIDER_SET, pasteimage_provider_set);
        elm_entry_item_provider_append(entry, image_provider, NULL);
        evas_object_event_callback_add(entry, EVAS_CALLBACK_FREE,
                                       entry_deleted, NULL);
     }
   return true;
}


static bool
pasteimage_append(struct pasteimage *pi, Evas_Object *entry)
{
   char entrytag[100];

   if (!pi) return false;
   if (!entry) return false;

   pasteimage_provider_set(entry);

   pastedimages = eina_list_append(pastedimages, pi);
   snprintf(entrytag, sizeof(entrytag),"<item absize=240x180 href=%s>",pi->tag);
   elm_entry_entry_insert(entry, entrytag);

   return true;
}

static void
entry_deleted(void *images __UNUSED__, Evas *e __UNUSED__, Evas_Object *entry, void *unused __UNUSED__)
{
   struct pasteimage *pi;
   Eina_List *l,*next;

   EINA_LIST_FOREACH_SAFE(pastedimages, l, next, pi)
     {
	if (pi->entry == entry){
	     pastedimages = eina_list_remove_list(pastedimages, l);
	}
     }
}


static char *
remove_tags(const char *p, int *len){
   char *q,*ret;
   int i;
   if (!p) return NULL;

   q = malloc(strlen(p) + 1);
   if (!q) return NULL;
   ret = q;

   while (*p)
     {
	if (*p != '<' && *p != '&'){
	     *q ++ = *p ++;
	} else if (*p == '<') {
	     if (p[1] == 'b' && p[2] == 'r' &&
			(p[3] == ' ' || p[3] == '/' || p[3] == '>'))
		*q++ = '\n';
	     while (*p && *p != '>') p ++;
	     p ++;
	} else if (*p == '&') {
	     p ++;
	     for (i = 0 ; i < N_ESCAPES ; i ++){
		  if (!strncmp(p,escapes[i].escape, strlen(escapes[i].escape))){
		       p += strlen(escapes[i].escape);
		       *q = escapes[i].value;
		       q ++;
		       break;
		  }
	     }
	     if (i == N_ESCAPES)
	       *q ++ = '&';
	}
     }
   *q = 0;
   if (len) *len = q - ret;
   return ret;
}

/* Mark up */
static char *
mark_up(const char *start, int *lenp){
  int l,i;
  const char *p;
  char *q,*ret;

  if (!start) return NULL;
  /* First pass: Count characters */
  for (l = 0, p = start ; *p ; p ++)
    {
    for (i = 0 ; i < N_ESCAPES ; i ++)
      {
	 if (*p == escapes[i].value)
	   {
	      l += strlen(escapes[i].escape);
	      break;
	   }
      }
    if (i == N_ESCAPES)
	 l ++;
  }

  q = ret = malloc(l + 1);

  /* Second pass: Change characters */
  for (p = start ; *p ; )
    {
    for (i = 0 ; i < N_ESCAPES ; i ++)
      {
	 if (*p == escapes[i].value)
	   {
	      strcpy(q, escapes[i].escape);
	      q += strlen(escapes[i].escape);
	      p ++;
	      break;
	   }
      }
    if (i == N_ESCAPES)
	 *q ++ = *p ++;
  }
  *q = 0;

  if (lenp) *lenp = l;
  return ret;
}


/**
 * Drag & Drop functions
 */
struct dropable {
     Evas_Object *obj;
     /* FIXME: Cache window */
     Elm_Sel_Format types;
     Elm_Drop_Cb dropcb;
     void *cbdata;
};
/* FIXME: Way too many globals */
Eina_List *drops = NULL;
Evas_Object *dragwin;
int _dragx, _dragy;
Ecore_Event_Handler *handler_pos, *handler_drop, *handler_enter,
                    *handler_status;

struct dropable *cur;

static Eina_Bool
_dnd_enter(void *data, int etype, void *ev)
{
   Ecore_X_Event_Xdnd_Enter *enter = ev;
   int i;

   /* Skip it */
   if (enter->num_types == 0 || enter->types == NULL) return true;

   cnp_debug("Types\n");
   savedtypes.ntypes = enter->num_types;
   if (savedtypes.types) free(savedtypes.types);
   savedtypes.types = malloc(sizeof(char *) * enter->num_types);
   for (i = 0 ; i < enter->num_types ; i ++)
     {
        savedtypes.types[i] = eina_stringshare_add(enter->types[i]);
        cnp_debug("Type is %s %p %p\n",enter->types[i],
                  savedtypes.types[i],text_uri);
        if (savedtypes.types[i] == text_uri)
          {
             /* Request it, so we know what it is */
             cnp_debug("Sending uri request\n");
             savedtypes.textreq = 1;
             savedtypes.pi = NULL; /* FIXME: Free? */
             ecore_x_selection_xdnd_request(enter->win, text_uri);
          }
     }

   /* FIXME: Find an object and make it current */

   return true;
}

static Eina_Bool
_dnd_drop(void *data, int etype, void *ev)
{
   struct _Ecore_X_Event_Xdnd_Drop *drop;
   struct dropable *dropable;
   Eina_List *l;
   Ecore_Evas *ee;
   Ecore_X_Window xwin;
   Elm_Drop_Data ddata;
   int x,y,w,h;

   int i,j;

   drop = ev;

   // check we still have something to drop
   if (!drops) return true;

   /* Find any widget in our window; then work out geometry rel to our window */
   for (l = drops ; l ; l = l->next)
     {
        dropable = l->data;
        xwin = (Ecore_X_Window)ecore_evas_window_get(ecore_evas_ecore_evas_get(
                                             evas_object_evas_get(
                                                dropable->obj)));
        if (xwin == drop->win)
           break;
     }
   /* didn't find a window */
   if (l == NULL)
      return true;


   /* Calculate real (widget relative) position */
      // - window position
      // - widget position
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(dropable->obj));
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   savedtypes.x = drop->position.x - x;
   savedtypes.y = drop->position.y - y;

   printf("Drop position is %d,%d\n",savedtypes.x,savedtypes.y);

   for ( ; l ; l = l->next)
     {
        dropable = l->data;
        evas_object_geometry_get(dropable->obj, &x, &y, &w, &h);
        if (savedtypes.x >= x && savedtypes.y >= y &&
            savedtypes.x < x + w && savedtypes.y < y + h)
           break; /* found! */
     }

   if (!l) /* didn't find one */
      return true;

   evas_object_geometry_get(dropable->obj, &x, &y, NULL, NULL);
   savedtypes.x -= x;
   savedtypes.y -= y;

   /* Find our type from the previous list */
   for (i = 0 ; i < CNP_N_ATOMS ; i ++)
     {
	for (j = 0 ; j < savedtypes.ntypes ; j ++)
          {
             if (strcmp(savedtypes.types[j], atoms[i].name) == 0)
               {
                  goto found;
               }
          }
     }

   cnp_debug("Didn't find a target\n");
   return true;

found:
   printf("Found a target we'd like: %s\n",atoms[i].name);
       cnp_debug("0x%x\n",xwin);

   if (i == CNP_ATOM_text_urilist)
     {
        cnp_debug("We found a URI... (%scached)\n",savedtypes.pi?"":"not ");
        if (savedtypes.pi)
          {
             char entrytag[100];

             ddata.x = savedtypes.x;
             ddata.y = savedtypes.y;

             if (dropable->types & ELM_SEL_FORMAT_IMAGE)
               {
                  cnp_debug("Doing image insert (%s)\n",savedtypes.pi->file);
                  ddata.format = ELM_SEL_FORMAT_IMAGE;
                  ddata.data = (char *)savedtypes.pi->file;
                  dropable->dropcb(dropable->cbdata, dropable->obj, &ddata);
                  ecore_x_dnd_send_finished();

                  pasteimage_free(savedtypes.pi);
                  savedtypes.pi = NULL;

                  return true;
               }
             else if (dropable->types & ELM_SEL_FORMAT_MARKUP)
               {
                  ddata.format = ELM_SEL_FORMAT_MARKUP;
                  pasteimage_provider_set(dropable->obj);

                  pastedimages = eina_list_append(pastedimages, savedtypes.pi);
                  snprintf(entrytag, sizeof(entrytag),
                           "<item absize=240x180 href=%s>",savedtypes.pi->tag);
                  ddata.data = entrytag;
                  cnp_debug("Insert %s\n",(char *)ddata.data);
                  dropable->dropcb(dropable->cbdata, dropable->obj, &ddata);
                  ecore_x_dnd_send_finished();
                  return true;
               }
          }
        else if (savedtypes.textreq)
          {
             /* Already asked: Pretend we asked now, and paste immediately when
              * it comes in */
             savedtypes.textreq = 0;
             ecore_x_dnd_send_finished();
             return true;
          }
     }

   cnp_debug("doing a request then\n");
   selections[ELM_SEL_XDND].requestwidget = dropable->obj;
   selections[ELM_SEL_XDND].requestformat = ELM_SEL_FORMAT_MARKUP;
   selections[ELM_SEL_XDND].active = true;

   ecore_x_selection_xdnd_request(xwin, atoms[i].name);

   return true;
}
static Eina_Bool
_dnd_position(void *data, int etype, void *ev)
{
   struct _Ecore_X_Event_Xdnd_Position *pos;
   Ecore_X_Rectangle rect;

   pos = ev;

   /* Need to send a status back */
   /* FIXME: Should check I can drop here */
   /* FIXME: Should highlight widget */
   rect.x = pos->position.x - 5;
   rect.y = pos->position.y - 5;
   rect.width = 10;
   rect.height = 10;
   ecore_x_dnd_send_status(true, false, rect, pos->action);

   return true;
}

/**
 * When dragging this is callback response from the destination.
 * The important thing we care about: Can we drop; thus update cursor
 * appropriately.
 */
static Eina_Bool
_dnd_status(void *data, int etype, void *ev)
{
   struct _Ecore_X_Event_Xdnd_Status *status = ev;

   if (!status) return true;

   /* Only thing we care about: will accept */
   if (status->will_accept)
     {
         printf("Will accept\n");
     }
   else
     { /* Won't accept */
         printf("Won't accept accept\n");
     }
   return true;
}

/**
 * Add a widget as drop target.
 */
Eina_Bool
elm_drop_target_add(Evas_Object *obj, Elm_Sel_Type format,
                    Elm_Drop_Cb dropcb, void *cbdata)
{
   struct dropable *drop;
   Ecore_X_Window xwin;
   int first;

   if (!obj) return false;
   if (!_elm_cnp_init_count) _elm_cnp_init();

   /* Is this the first? */
   first = (drops == NULL) ? 1 : 0;

   drop = calloc(1,sizeof(struct dropable));
   if (!drop) return false;
   drop->dropcb = dropcb;
   drop->cbdata = cbdata;

   /* FIXME: Check it's not already there */

   /* FIXME: Check for eina's deranged error method */
   drops = eina_list_append(drops, drop);

   if (!drops/* || or other error */)
     {
        free(drop);
        return false;
     }

   drop->obj = obj;
   /* Something for now */
   drop->types = format;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  /* I love C and varargs */
                                  (Evas_Object_Event_Cb)elm_drop_target_del,
                                  obj);
   /* FIXME: Handle resizes */

   /* If not the first: We're done */
   if (!first) return true;

   xwin = (Ecore_X_Window)ecore_evas_window_get(ecore_evas_ecore_evas_get(
         evas_object_evas_get(obj)));
   ecore_x_dnd_aware_set(xwin, true);

   printf("Adding drop target calls\n");
   handler_enter = ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER,
                                           _dnd_enter, NULL);
   handler_pos = ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION,
                                         _dnd_position, NULL);
   handler_drop = ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP,
                                          _dnd_drop, NULL);

   return true;
}

Eina_Bool
elm_drop_target_del(Evas_Object *obj)
{
   struct dropable *drop,*del;
   Eina_List *item;
   Ecore_X_Window xwin;

   del = NULL;
   EINA_LIST_FOREACH(drops, item, drop)
     {
        if (drop->obj == obj)
          {
             drops = eina_list_remove_list(drops, item);
             del = drop;
             break;
          }
     }
   if (!del) return false;

   evas_object_event_callback_del(obj, EVAS_CALLBACK_FREE,
                                  (Evas_Object_Event_Cb)elm_drop_target_del);
   free(drop);
   /* If still drops there: All fine.. continue */
   if (drops != NULL) return true;

   cnp_debug("Disabling DND\n");
   xwin = (Ecore_X_Window)ecore_evas_window_get(ecore_evas_ecore_evas_get(
         evas_object_evas_get(obj)));
   ecore_x_dnd_aware_set(xwin, false);

   ecore_event_handler_del(handler_pos);
   ecore_event_handler_del(handler_drop);
   ecore_event_handler_del(handler_enter);

   if (savedtypes.pi)
     {
        pasteimage_free(savedtypes.pi);
        savedtypes.pi = NULL;
     }

   return true;
}


static void
_drag_mouse_up(void *un, Evas *e, Evas_Object *obj, void *data)
{
   evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP, _drag_mouse_up);
   ecore_x_dnd_drop();
   if (dragdonecb)
     {
        dragdonecb(dragdonecb,selections[ELM_SEL_XDND].widget);
        dragdonecb = NULL;
     }
   if (dragwin)
     {
        evas_object_del(dragwin);
        dragwin = NULL;
     }

}

static void
_drag_move(void *data __UNUSED__, Ecore_X_Xdnd_Position *pos){
   evas_object_move(dragwin, pos->position.x - _dragx,
                    pos->position.y - _dragy);
}


Eina_Bool
elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, const char *data,
               void (*dragdone)(void *data, Evas_Object *),void *donecbdata)
{
   Ecore_X_Window xwin;
   struct _elm_cnp_selection *sel;
   Elm_Sel_Type xdnd = ELM_SEL_XDND;
   Ecore_Evas *ee;
   int x,y,x2,y2,x3,y3;
   Evas_Object *icon;
   int w,h;

   if (!_elm_cnp_init_count) _elm_cnp_init();

   xwin = elm_win_xwindow_get(obj);

   cnp_debug("starting drag...\n");

   ecore_x_dnd_type_set(xwin, "text/uri-list", 1);
      /* FIXME: just call elm_selection_set */
   sel = selections + ELM_SEL_XDND;
   sel->active = 1;
   sel->widget = obj;
   sel->format = format;
   sel->selbuf = data ? strdup(data) : NULL;
   dragdonecb = dragdone;
   dragdonedata = donecbdata;

   ecore_x_dnd_callback_pos_update_set(_drag_move, NULL);
   ecore_x_dnd_begin(xwin, (unsigned char *)&xdnd, sizeof(Elm_Sel_Type));
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                  _drag_mouse_up, NULL);

   handler_status = ecore_event_handler_add(ECORE_X_EVENT_XDND_STATUS,
                                            _dnd_status, NULL);

   dragwin = elm_win_add(NULL, "Elm Drag Object",ELM_WIN_UTILITY);
   elm_win_override_set(dragwin,1);

   /* FIXME: Images only */
   icon = elm_icon_add(dragwin);
   elm_icon_file_set(icon, data + 7, NULL); /* 7!? */
   elm_win_resize_object_add(dragwin,icon);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);

   /* Position subwindow appropriately */
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   evas_object_geometry_get(obj, &x2, &y2, &w, &h);
   x += x2;
   y += y2;
   evas_object_move(dragwin, x, y);
   evas_object_resize(icon,w,h);
   evas_object_resize(dragwin,w,h);

   evas_object_show(icon);
   evas_object_show(dragwin);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x3, &y3);
   _dragx = x3 - x2;
   _dragy = y3 - y2;

   return true;
}




#else
/* Stubs for windows */
Eina_Bool
elm_drag_start(Evas_Object *o, Elm_Sel_Format f, const char *d,
               void (*donecb)(void *, Evas_Object *),void *cbdata)
{
   return false;
}
Eina_Bool
elm_drop_target_add(Evas_Object *obj, Elm_Sel_Type format,
                    Elm_Drop_Cb dropcb, void *cbdata){
   return false;
}

Eina_Bool
elm_drop_target_del(Evas_Object *o)
{
   return true;
}
#endif

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
