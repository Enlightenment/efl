/* by Azundris */
/* Modified for ecore_config by HandyAndE */

#include "Ecore_Config.h"

#include <Ecore.h>
#include <Ecore_Ipc.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_LIBREADLINE
#  define HISTORY "/.ecore/exsh_history"
#  if defined(HAVE_READLINE_READLINE_H)
#    include <readline/readline.h>
#  elif defined(HAVE_READLINE_H)
#    include <readline.h>
#  else
extern char *readline();
#  endif
char *cmdline=NULL;
#endif

#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else
extern void add_history();
extern int  write_history();
extern int  read_history();
#  endif
#endif



#include "errors.h"
#include "ipc.h"
#include "ecore_config_client.h"



#define OUT "< "
#define ERR "! "
#define IN  "> "

#define MI 4096

#define HELP_ALL "exsh -- ecore_config shell"


typedef enum {
  P_NONE=0,
  P_SERIAL=1,
  P_DIR=2,
  P_FILE=4,
  P_KEY=8,
  P_VALUE=16,
  P_SILENT=32,
  P_HELPONLY=64,
  P_LAST=P_HELPONLY } para;


typedef struct _call {
  ex_ipc_call  id;
  char        *name;
  para         signature;
  const char  *help; } call;



static call calls[]={
  { IPC_NONE,         "bundle",           P_HELPONLY, "List of propertyset-related commands" },
  { IPC_BUNDLE_LIST,     "bundle-list",      P_NONE, "bundle.list\nList all property sets, giving bundle id and label for each." },
  { IPC_BUNDLE_NEW,      "bundle-new",       P_DIR, "bundle.new <label>\nCreate a new bundle labeled <label>." },
  { IPC_BUNDLE_LABEL_GET,  "bundle-label-get",   P_SILENT|P_SERIAL, "bundle.label.get <bundle-id>\nShow the label of bundle <bundle-id>." },
  { IPC_BUNDLE_LABEL_GET,  "bundle-label",       P_SERIAL, "bundle.label -- short for bundle.label.get, see there." },
  { IPC_BUNDLE_LABEL_SET,  "bundle-label-set",   P_SILENT|P_SERIAL|P_DIR, "bundle.label.set <bundle-id> <label>\nLabel the bundle <bundle-id> <label>." },
  { IPC_BUNDLE_LABEL_SET,  "bundle-label",       P_SILENT|P_HELPONLY, "List of bundle-label related functions." },
  { IPC_BUNDLE_LABEL_FIND, "bundle-label-find",  P_SILENT|P_DIR, "bundle.label.find <label>\nReturn the ID of the first bundle labeled as <label>." },
  { IPC_BUNDLE_LABEL_FIND, "bundle-find",      P_DIR, "bundle.find -- short for bundle.label.find, see there." },

  { IPC_NONE,         "prop",          P_HELPONLY, "List of property-related commands" },
  { IPC_PROP_LIST,    "prop-list",     P_SERIAL, "prop.list <bundle-id>\nList all properties that are currently registered for bundle <bundle-id>." },
  { IPC_PROP_DESC,    "prop-describe", P_SERIAL|P_KEY, "prop.desc <bundle-id> <prop>\nDescribe property <prop> set on bundle <bundle-id>." },
  { IPC_PROP_GET,     "prop-get",      P_SERIAL|P_KEY, "prop.get <bundle-id> <prop>\nShow the value of property <prop> set on bundle <bundle-id>." },
  { IPC_PROP_SET,     "prop-set",      P_SERIAL|P_KEY|P_VALUE, "prop.set <bundle-id> <prop> <new-value>\nSet the value of property <prop> set on bundle <bundle-id> to <new-value>." },
  { IPC_PROP_LIST,    "list",          P_SILENT|P_SERIAL, "list -- short for prop.list, see there." },
  { IPC_PROP_DESC,    "describe",      P_SILENT|P_SERIAL|P_KEY, "desc -- short for prop.desc, see there." },
  { IPC_PROP_DESC,    "explain",       P_SILENT|P_SERIAL|P_KEY, "desc -- short for prop.desc, see there." },
  { IPC_PROP_GET,     "print",         P_SILENT|P_SERIAL|P_KEY, "get -- short for prop.get, see there." },
  { IPC_PROP_GET,     "get",           P_SILENT|P_SERIAL|P_KEY, "get -- short for prop.get, see there." },
  { IPC_PROP_SET,     "set",           P_SILENT|P_SERIAL|P_KEY|P_VALUE, "set -- short for prop.get, see there." },
  { IPC_PROP_SET,     "let",           P_SILENT|P_SERIAL|P_KEY|P_VALUE, "let -- short for prop.get, see there." },

};



int debug=99;



/*****************************************************************************/



int ex_ipc_server_con(void *data,int type,void *event) {
  connstate                  *cs=(connstate *)data;
  *cs=ONLINE;
  E(1,"exsh: Connected .\n"); /*to %s.\n", pipe_name)); */
  return 1; }



int ex_ipc_server_dis(void *data,int type,void *event) {
  connstate                  *cs=(connstate *)data;
  *cs=OFFLINE;
  ecore_main_loop_quit();
  E(1,"exsh: Disconnected.\n");
  return 1; }



int ex_ipc_server_sent(void *data,int type,void *event) {
  Ecore_Ipc_Event_Server_Data *e=(Ecore_Ipc_Event_Server_Data *)event;
  E(2,"application sent: %2d.%02d  #%03d=>#%03d  \"%s\".%d\n",
    e->major,e->minor,0,0,(char *)e->data,e->size);
  if(e->data&&(e->size>0))
    printf(OUT "%s\n",(char *)e->data);
  else if(e->response<0)
    printf(OUT "error #%d\n",e->response);
  else if(!e->response)
    printf(OUT "OK\n");
  else
    printf(OUT "result: %d\n",e->response);
  ecore_main_loop_quit();
  return 1; }



/*****************************************************************************/
/* parser */



static long get_serial(char **m) {
  long ret=-1;
  if(m&&*m) {
    char *b;
    ret=strtol(*m,&b,10);
    if(b==*m)
      ret=0;
    else {
      while(isspace(*b))
        b++;
      *m=b; }}
  return ret; }



static int get_token(char **beg,char **end,int tol) {
  int     l;
  char   *b=*beg,*e;

  if(!b||!*b)
    return ECORE_CONFIG_ERR_NODATA;

  while(isspace(*b))
    b++;
  e=b;

  if(*e=='\"') {
    do {
      e++;
      if(*e=='\\')
        e++;
    } while(*e&&*e!='\"'); }
  else {
    while(*e&&!isspace(*e)) {
      if(tol&&isupper(*e))
        *e=tolower(*e);
      e++; }}

  if(*e) {
    *(e++)='\0';
    while(*e&&isspace(*e))
      *(e++)='\0';
    *end=e; }
  else
    *end=NULL;
  *beg=b;
  l=e-b;
  return (l<=0)?ECORE_CONFIG_ERR_NODATA:l; }



/*****************************************************************************/



static int handle_any(ex_ipc_server_list **server_list,call *cp,char *line) {
  long  serial=-1;
  int   ret,pars=0,l=0;
  char *b=line,*e,*file=NULL,*k=NULL,*v=NULL,*m=NULL;

  if(cp->signature&P_SERIAL)
    serial=get_serial(&b);

  if(cp->signature&(P_DIR|P_FILE)) {
    if((ret=get_token(&b,&e,FALSE))<0)
      return ret;
    file=b;
    pars++;
    b=e; }
    /*  b=e?e+1:NULL; */

  if(cp->signature&(P_KEY)) {
    if((ret=get_token(&b,&e,TRUE))<0)
      return ret;
    k=b;
    pars++;
    b=e; }
    /*  b=e?e+1:NULL; */
        
  if(cp->signature&(P_VALUE)) {
    if((ret=get_token(&b,&e,FALSE))<0)
      return ret;
    v=b;
    pars++;
    b=e; }
    /*  b=e?e+1:NULL; */
        
  if(file)
    send_append(&m,&l,file);
  if(k)
    send_append(&m,&l,k);
  if(v)
    send_append(&m,&l,v);

  E(3,"found \"%s\" |serial:%ld|file:%s|key:%s|value:%s|\n",cp->name,serial,file,k,v);

  ret=ex_ipc_send(server_list,cp->id,serial,m,l);
  if(m)
    free(m);
  return ret; }






/*****************************************************************************/



static char *clean(char *c) {
  char   *p=c;
  size_t  l;

  while(*p) {
    if(*p=='\"') {
      do {
        p++;
        if(*p=='\\')
          p++;
      } while(*p&&*p!='\"');
      if(!*p) {
        E(0,"unterminated string in input?\n");
        *c='\0';
        return c; }
      else
        p++; }
    else if(isspace(*p)) {    /* turn any sort of space into a single ' ' */
      l=strlen(p);
      while(isspace(p[1]))
        memmove(p,p+1,l--);
      *(p++)=' '; }
    else
      p++; }

  if((l=strlen(c))) {
    while(isspace(*c))
      memmove(c,c+1,l--);
    while(isspace(c[l-1]))
      c[l-1]='\0'; }

  p=c;
  while(*p&&!isspace(*p)) {
    if(!isalpha(*p)) {
      while(p[1]&&!isalpha(p[1]))
        memmove(p,p+1,strlen(p));
      *p='-'; }
    p++; }

  return c; }



static int abbrevcmp(char *a,char *t) {
  size_t  l1;
  char   *p,*q;
  int     ret;

  while(1) {
    if(!(p=strchr(a,'-')))
      p=a+strlen(a);
    l1=p-a;
    if((ret=strncasecmp(a,t,l1)))
      return ret;

    a=p;
    if(*a)
      a++;

    if(!(q=strchr(t,'-')))
      t=t+strlen(t);
    else
      t=q+1;

    if(!*a)
      return *t?-1:0;
    else if(!*t)
      return 1; }}



static call *find_call(char *b) {
  int          nc=sizeof(calls)/sizeof(call);
  call        *cp=calls,*r=NULL;
  while(nc-->0) {
    if(!abbrevcmp(b,cp->name)) {
      if(r) {
        printf(ERR "\"%s\" is not unique.\n",b);
        return NULL; }
      r=cp; }
    cp++; }
  return r; }



static call *find_abbrev(char **beg,char **end) {
  char        *b=*beg,*m,*q;
  size_t       l=strlen(b);
  call        *cp=calls;

  if(!(q=m=malloc(l*2)))
    return NULL;

  while(*b) {
    *(q++)=*(b++);
    *(q++)='-'; }
  *(--q)='\0';

  cp=find_call(m);

  free(m);
  return cp; }



static call *find_with_delims(char **beg,char **end) {
  char *b=*beg;
  int   d=0;

  while(*b) {
    if(*b=='-')
      d++;
    b++; }
  return d?find_call(*beg):NULL; }



static call *find_with_spaces(char **beg,char **end) {
  call *cp=NULL,*lcp=NULL;
  char *e=*end,*b=*beg,*d=NULL;

  if(!*end)
    return find_call(b);

  while(1) {
    if(!(cp=find_call(b))) {
      *e=' ';
      if(d) {
        *d='\0';
        *end=d; }
      return lcp; }
    lcp=cp;
    d=e;
    *(e++)='-';
    if(!(e=strchr(e,' '))) {
      if((cp=find_call(b))) {
        *end=NULL;
        return cp; }
      *end=d;
      return lcp; }
    *e='\0'; }}



static int parse_line(ex_ipc_server_list **server_list,char *line) {
  call        *cp=NULL;
  char        *c=NULL,*b=line,*e=NULL,*p;
  int          ret=ECORE_CONFIG_ERR_SUCC,h=0;

  if(!line||!*line)
    return ECORE_CONFIG_ERR_IGNORED;

  if(!(c=strdup(line)))
    return ECORE_CONFIG_ERR_OOM;

  if((p=clean(c))) {
    if(!strcasecmp(p,"help")) {
      int   nc=sizeof(calls)/sizeof(call);
      cp=calls;
      puts(HELP_ALL);
      while(nc-->0) {
        if((cp->signature&P_HELPONLY)&&!(cp->signature&P_SILENT))
          printf("  %s\n",cp->name);
        cp++; }
      goto done; }

    else if(!strncasecmp(p,"help ",5)) {
      h++;
      p+=5; }

    b=p;
    if((e=strchr(b,' ')))
      *e='\0';
    if(!(cp=find_with_delims(&b,&e))) {
      if(!(cp=find_with_spaces(&b,&e))) {
        if(e)
          *e='\0';
        cp=find_abbrev(&b,&e); }}
    if(!cp)
      ret=ECORE_CONFIG_ERR_NOTFOUND;
    else {
      if(h)
        puts(cp->help);
      else if(cp->signature&P_HELPONLY) {
        int   nc=sizeof(calls)/sizeof(call),l=strlen(cp->name);
        call *r=calls;

        if(cp->help&&*cp->help)
          puts(cp->help);
        while(nc-->0) {
          if(!strncmp(cp->name,r->name,l)&&(r->name[l]=='-')&&!(r->signature&P_SILENT))
            printf("  %s\n",r->name);
          r++; }}
      else {
        E(3,"parsed: \"%s\" => %s { \"%s\" }\n",b,cp->name,e?e:"");
        if((ret=handle_any(server_list,cp,e?e+1:""))==ECORE_CONFIG_ERR_NODATA)
          E(0,ERR "Insufficient parameters for %s: %s\n",cp->name,cp->help?cp->help:"");
        else E(0,"ret: %d\n",ret); }}}
  else
    ret=ECORE_CONFIG_ERR_FAIL;

 done:
  free(c);
  return ret; }






/*****************************************************************************/



int main(int argc,char **argv) {
  ex_ipc_server_list *server=NULL;
  int                 ret=ECORE_CONFIG_ERR_SUCC,cc=0;
  connstate           cs=OFFLINE;
  char               *p,*f=NULL;
  char               *pipe_name=NULL;

  if (argc <= 1  || (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0))  {
    print_usage();
    return 0;
  }

  pipe_name=argv[1];
  printf("exsh: connecting to %s.\n", pipe_name);
  
#ifndef HAVE_LIBREADLINE
  char              buf[MI];
#elif defined(HAVE_READLINE_HISTORY)
  char             *history=NULL;

  using_history();
  stifle_history(96);
  if((p=getenv("HOME"))) {
    if((history=malloc(strlen(p)+sizeof(HISTORY)))) {
      strcat(strcpy(history,p),HISTORY);
      read_history(history); }}
#endif

  ecore_init();
  ecore_app_args_set(argc,(const char **)argv);
  ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT,ex_ipc_sigexit,&cs);

reconnect:
  cc++;
  if((ret=ex_ipc_init(&server,pipe_name,&cs))!=ECORE_CONFIG_ERR_SUCC)
    E(0,"exsh: %sconnect to %s failed: %d\n",(cc>1)?"re":"",pipe_name,ret);
  else {
    while(1) {
      p=NULL;
      ecore_main_loop_iterate();
      if(cs==OFFLINE) {
        ex_ipc_exit(&server);
        cs=ONLINE;
        goto reconnect; }

#ifdef HAVE_LIBREADLINE
      if((p=readline(IN))) {
#else
      printf(IN);
      if((f=p=fgets(buf,MI,stdin))) {
#endif
        char *q=strchr(p,'\n');
        if(q)
          *q='\0';
        if(!strcasecmp(p,"exit")||!strcasecmp(p,"quit")) {
          if(f)
            free(f);
          break; }

#ifdef HAVE_READLINE_HISTORY
        add_history(p);
#endif
        ret=parse_line(&server,p);
        if((ret<ECORE_CONFIG_ERR_SUCC)&&(ret!=ECORE_CONFIG_ERR_IGNORED))
          printf(ERR "error #%d: \"%s.\"\n",-ret,ecore_config_error(ret));
        if(f)
          free(f); }
      else
        break; }}

  ex_ipc_exit(&server);
  ecore_shutdown();

#ifdef HAVE_READLINE_HISTORY
  if(history) {
    write_history(history);
    free(history); }
#endif

  return ret; }

print_usage(void) {
  printf("Examine Shell - ecore_config Configuration Client\n");
  printf("Version 1.0.0 (Dec 6 2003)\n");
  printf("(c)2002-2003 by Azundris et al.\n");
  printf("Modified for ecore_config 2003 by HandyAndE.\n");
  printf("Usage: exsh [options] target\n\n");
  printf("Supported Options:\n");
  printf("-h, --help        Print this help text\n");

}

/*****************************************************************************/
