#include <stdbool.h>

#include <stdio.h> // debug

#include <Elementary.h>

#include "elm_priv.h"

#ifdef HAVE_ELEMENTARY_X

# include <X11/X.h>
# include <X11/Xatom.h>


# define ARRAYINIT(foo)  [foo]=

#define DEBUGON	0

#if DEBUGON
#define cnp_debug(x...) printf(__FILE__": " x)
#else
#define cnp_debug(x...)
#endif


enum {
     CNP_ATOM_TARGETS = 0,
     CNP_ATOM_text_uri,
     CNP_ATOM_image_png,
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

   enum _elm_sel_format format;
   char *selbuf;

   unsigned int active : 1;

   Ecore_X_Selection ecore_sel;

   Evas_Object *requestwidget;
   enum _elm_sel_format requestformat;

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


static struct {
   const char *name;
   enum _elm_sel_format formats;
   /* Called by ecore to do conversion */
   converter_fn converter;
   response_handler response;
   notify_handler notify;
   /* Atom */
   Ecore_X_Atom atom;
} atoms[CNP_N_ATOMS] = {
   [CNP_ATOM_TARGETS] = {
	"TARGETS",
	(enum _elm_sel_format)-1,
	targets_converter,
	response_handler_targets,
	notify_handler_targets,
	0
   },
   [CNP_ATOM_XELM] =  {
	"application/x-elementary-markup",
	ELM_SEL_MARKUP,
	edje_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_text_uri] = {
	"text/uri",
	ELM_SEL_MARKUP | ELM_SEL_IMAGE, /* Either images or entries */
	uri_converter,
	NULL,
	notify_handler_uri,
	0
   },
   [CNP_ATOM_image_png] = {
	"image/png",
	ELM_SEL_IMAGE | ELM_SEL_IMAGE,
	png_converter,
	NULL,
	notify_handler_png,
	0
   },
   [CNP_ATOM_text_html_utf8] = {
	"text/html;charset=utf-8",
	ELM_SEL_MARKUP,
	html_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_text_html] = {
	"text/html",
	ELM_SEL_MARKUP,
	html_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_UTF8STRING] = {
	"UTF8_STRING",
	ELM_SEL_MARKUP,
	text_converter,
	NULL,
	notify_handler_text,
	0
   },
   [CNP_ATOM_STRING] = {
	"STRING",
	ELM_SEL_MARKUP | ELM_SEL_IMAGE,
	text_converter,
	NULL,
	notify_handler_text,
	0
   },
   [CNP_ATOM_TEXT] = {
	"TEXT",
	ELM_SEL_MARKUP | ELM_SEL_IMAGE,
	text_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_text_plain_utf8] = {
	"text/plain;charset=ut-8",
	ELM_SEL_MARKUP,
	text_converter,
	NULL,
	NULL,
	0
   },
   [CNP_ATOM_text_plain] = {
	"text/plain",
	ELM_SEL_MARKUP,
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
   }
};

static int _elm_cnp_init_count = 0;
  /* Gah... who left this out of XAtoms.h */
static Ecore_X_Atom clipboard_atom;

Eina_List *pastedimages;
#endif


Eina_Bool
elm_selection_set(enum _elm_sel_type selection, Evas_Object *widget,
			enum _elm_sel_format format, const char *selbuf)
{
#ifdef HAVE_ELEMENTARY_X
   struct _elm_cnp_selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();
   if (!selbuf && format != ELM_SEL_IMAGE)
     return elm_selection_clear(selection, widget);

   sel = selections + selection;

   sel->active = 1;
   sel->widget = widget;

   sel->set(elm_win_xwindow_get(widget),&selection,sizeof(enum _elm_sel_type));
   sel->format = format;
   sel->selbuf = selbuf ? strdup(selbuf) : NULL;

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

Eina_Bool
elm_selection_clear(enum _elm_sel_type selection, Evas_Object *widget)
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
elm_selection_get(enum _elm_sel_type selection, enum _elm_sel_format format,
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
   sel->request(elm_win_xwindow_get(top), ECORE_X_SELECTION_TARGET_UTF8_STRING);

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
png_converter(char *target __UNUSED__, void *data __UNUSED__, int size __UNUSED__,
              void **data_ret __UNUSED__, int *size_ret __UNUSED__,
              Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
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
   str = mark_up((char*)data->data, NULL);
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
   struct pasteimage *pi;
   char entrytag[100];
   char *p;

   data = notify->data;
   p = (char *)data->data;
   cnp_debug("Got %s\n",p);
   if (strncmp(p,"file://",7) != 0){
	cnp_debug("Doesn't start with ;file;  %s\n",p);
	return 0;
   }

   p += strlen("file://");

   pi = calloc(1,sizeof(struct pasteimage));
   snprintf(entrytag, sizeof(entrytag), "pasteimage-%p",pi);
   pi->tag = strdup(entrytag);
   pi->file = strndup(p,data->length - strlen("file://"));

   elm_entry_item_provider_append(sel->requestwidget, image_provider, pi);
   pastedimages = eina_list_append(pastedimages, pi);

   snprintf(entrytag, sizeof(entrytag), "<item absize=240x180 href=%s>",pi->tag);
   elm_entry_entry_insert(sel->requestwidget, entrytag);

   return 0;
}

static int
notify_handler_png(struct _elm_cnp_selection *sel __UNUSED__,
                   Ecore_X_Event_Selection_Notify *notify __UNUSED__)
{
   cnp_debug("got a png!\n");
   return 0;
}


static int
text_converter(char *target __UNUSED__, void *data, int size __UNUSED__,
               void **data_ret, int *size_ret,
               Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   struct _elm_cnp_selection *sel;

   sel = selections + *(int *)data;
   if (!sel->active)
     {
	return 1;
     }

   if (sel->format == ELM_SEL_MARKUP){
	*data_ret = remove_tags(sel->selbuf, size_ret);
   } else if (sel->format == ELM_SEL_IMAGE){
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


#endif
