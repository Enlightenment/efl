
#include <stdbool.h>

#include <stdio.h> // debug

#include <X11/X.h>
#include <X11/Xatom.h>

#include <Elementary.h>
#include <Ecore_X.h>

#include "elm_priv.h"


/* this should be #ifdef 12year old compiler */
#ifdef ELM_WIN32
#  define ARRAYINIT(foo)
#else
#  define ARRAYINIT(foo)  [foo]=
#endif


enum {
  CNP_ATOM_TARGETS = 0,
  CNP_ATOM_XEDJE,
  CNP_ATOM_text_html_utf8,
  CNP_ATOM_text_html,
  CNP_ATOM_UTF8STRING,
  CNP_ATOM_STRING,
  CNP_ATOM_TEXT,
  CNP_ATOM_text_plain_utf8,
  CNP_ATOM_text_plain,

  CNP_N_ATOMS,
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
static int selection_clear(void *udata __UNUSED__, int type, void *event);
static int selection_notify(void *udata __UNUSED__, int type, void *event);
static char *remove_tags(const char *p, int *len);
static char *mark_up(const char *start, int *lenp);


typedef int (*converter_fn)(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);

static int targets_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static int text_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static int html_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static int edje_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);

typedef int (*response_handler)(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *);
static int response_handler_targets(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *);
static int response_handler_text(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *);

static struct {
   const char *name;
   enum _elm_sel_format formats;
   converter_fn converter;
   response_handler response;
   Ecore_X_Atom atom;
} atoms[CNP_N_ATOMS] = {
   ARRAYINIT(CNP_ATOM_TARGETS) { "TARGETS",
		(enum _elm_sel_format)-1, targets_converter,
		response_handler_targets, 0 },
   ARRAYINIT(CNP_ATOM_XEDJE)  { "application/x-edje-string",
			ELM_SEL_MARKUP, edje_converter, NULL, 0},
   ARRAYINIT(CNP_ATOM_text_html_utf8) { "text/html;charset=utf-8",
			ELM_SEL_MARKUP, html_converter, NULL, 0 },
   ARRAYINIT(CNP_ATOM_text_html) { "text/html", ELM_SEL_MARKUP,
			html_converter, NULL, 0 },
   ARRAYINIT(CNP_ATOM_UTF8STRING) { "UTF8_STRING", ELM_SEL_MARKUP,
			text_converter, response_handler_text, 0 },
   ARRAYINIT(CNP_ATOM_STRING) { "STRING", ELM_SEL_MARKUP,
			text_converter, response_handler_text, 0 },
   ARRAYINIT(CNP_ATOM_TEXT) { "TEXT", ELM_SEL_MARKUP,
			text_converter, response_handler_text, 0 },
   ARRAYINIT(CNP_ATOM_text_plain_utf8) { "text/plain;charset=ut-8",
			ELM_SEL_MARKUP, text_converter,
			response_handler_text, 0 },
   ARRAYINIT(CNP_ATOM_text_plain) { "text/plain", ELM_SEL_MARKUP,
			text_converter, response_handler_text, 0 },
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


Eina_Bool
elm_selection_set(enum _elm_sel_type selection, Evas_Object *widget,
			enum _elm_sel_format format, const char *selbuf){
   struct _elm_cnp_selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return false;
   if (!_elm_cnp_init_count) _elm_cnp_init();
   if (!selbuf) return elm_selection_clear(selection, widget);

   sel = selections + selection;

   sel->active = 1;
   sel->widget = widget;
   sel->set(elm_win_xwindow_get(elm_widget_top_get(widget)),
	 &selection, sizeof(int));
   sel->format = format;
   sel->selbuf = strdup(selbuf);
   return true;
}

Eina_Bool
elm_selection_clear(enum _elm_sel_type selection, Evas_Object *widget){
   struct _elm_cnp_selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return false;
   if (!_elm_cnp_init_count) _elm_cnp_init();

   sel = selections + selection;

   /* No longer this selection: Consider it gone! */
   if (!sel->active || sel->widget != widget) return true;

   sel->active = 0;
   sel->widget = NULL;
   sel->clear();

   return true;
}

Eina_Bool
elm_selection_get(enum _elm_sel_type selection, enum _elm_sel_format format,
			Evas_Object *widget){
   Evas_Object *top;
   struct _elm_cnp_selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return false;
   if (!_elm_cnp_init_count) _elm_cnp_init();

   sel = selections + selection;
   top = elm_widget_top_get(widget);
   if (!top) return false;

   sel->request(elm_win_xwindow_get(top), "TARGETS");
   sel->requestformat = format;
   sel->requestwidget = widget;

   return EINA_TRUE;
}

static Eina_Bool
_elm_cnp_init(void){
   int i;
   if (_elm_cnp_init_count ++) return true;

   /* FIXME: Handle XCB */
   for (i = 0 ; i < CNP_N_ATOMS ; i ++)
     {
	atoms[i].atom = ecore_x_atom_get(atoms[i].name);
	ecore_x_selection_converter_atom_add(atoms[i].atom,
	      atoms[i].converter);
     }

   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, selection_clear,NULL);
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,selection_notify,NULL);

    return true;
}

static int
selection_clear(void *udata __UNUSED__, int type, void *event){
   Ecore_X_Event_Selection_Clear *ev = event;
   struct _elm_cnp_selection *sel;
   int i;

   for (i = 0 ; i < ELM_SEL_MAX ; i ++)
     {
	if (selections[i].ecore_sel == ev->selection) break;
     }
   /* Not me... Don't care */
   if (i == ELM_SEL_MAX) return true;

   sel = selections + i;
   sel->active = 0;
   sel->widget = NULL;
   sel->selbuf = NULL;

   return true;
}

/**
 * Response to a selection notify
 */
static int
selection_notify(void *udata __UNUSED__, int type, void *event){
   Ecore_X_Event_Selection_Notify *ev = event;
   struct _elm_cnp_selection *sel;
   int i;

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
	 return 1;
   }

   for (i = 0 ; i < CNP_N_ATOMS ; i ++)
     {
	if (strcmp(ev->target, atoms[i].name) == 0)
	  {
	     if (atoms[i].response){
		  atoms[i].response(sel, ev);
	     } else {
		  printf("Ignored: No handler!\n");
	     }
	  }
     }

   return 1;
}


static int
targets_converter(char *target, void *data, int size,
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
	if (sel->format && atoms[i].formats)
	  aret[count ++] = atoms[i].atom;

   if (data_ret) *data_ret = aret;
   if (typesize) *typesize = 32 /* urk */;
   if (ttype) *ttype = XA_ATOM;
   if (size_ret) *size_ret = count;

   return 1;
}


static int
response_handler_targets(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *notify){
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
	printf("No matching type found\n");
	return 0;
     }

   top = elm_widget_top_get(sel->requestwidget);
   if (!top) return 0;

   sel->request(elm_win_xwindow_get(top), atoms[j].name);
   return 0;
}

static int
response_handler_text(struct _elm_cnp_selection *sel,
      Ecore_X_Event_Selection_Notify *notify){
   Ecore_X_Selection_Data_Text *text;
   char *str;

   text = notify->data;
   str = mark_up(text->text, NULL);
   elm_entry_entry_insert(sel->requestwidget, str);
   free(str);

   return 0;
}


static int
text_converter(char *target, void *data, int size,
		void **data_ret, int *size_ret,
		Ecore_X_Atom *ttype, int *typesize){
   struct _elm_cnp_selection *sel;

   sel = selections + *(int *)data;
   if (!sel->active)
     {
	return 1;
     }
   *data_ret = remove_tags(sel->selbuf, size_ret);
   return 1;
}

static int
edje_converter(char *target, void *data, int size, void **data_ret,
	    int *size_ret, Ecore_X_Atom *ttype, int *typesize){
   struct _elm_cnp_selection *sel;

   sel = selections + *(int *)data;
   if (data_ret) *data_ret = strdup(sel->selbuf);
   if (size_ret) *size_ret = strlen(sel->selbuf);

   return 1;
}


static int
html_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize){
     struct _elm_cnp_selection *sel;

     sel = selections + *(int *)data;
     if (data_ret) *data_ret = strdup(sel->selbuf);
     if (size_ret) *size_ret = strlen(sel->selbuf);

     return 1;
}



static char *
remove_tags(const char *p, int *len){
   char *q,*ret;
   int i;
   if (!p) return NULL;

   q = malloc(strlen(p));
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
		  if (strncmp(p,escapes[i].escape, strlen(escapes[i].escape))){
		       p += strlen(escapes[i].escape);
		       *q = escapes[i].value;
		       q ++;
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

  /* Second pass: Count characters */
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

/* vim: set ts=8 sw=3 sts=8 noexpandtab cino=>5n-3f0^-2{2 : */
