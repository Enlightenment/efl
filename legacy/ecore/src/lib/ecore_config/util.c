/* azundris */

//#include <glib.h>

#include <sys/types.h>
//#include <sys/time.h>    /* gimetimeofday() */
#include <stdio.h>       /* NULL */
#include <stdlib.h>      /* malloc(), free() */
#include <string.h>      /* str...() */
#include <ctype.h>       /* isspace() */

#include <stdarg.h>      /* varargs in dlmulti() */
#include <dlfcn.h>       /* dlopen() and friends for dlmulti() */

/* #ifdef HAVE_GLIB2 */
/* #  include <glib/gmain.h> */
/* #endif */

#include "util.h"
#include "Ecore_Config.h"

#define CHUNKLEN 4096


/*****************************************************************************/
/* MISC */
/********/



int dlmulti(char *name,char *file,int flag,void **libr,const char *fmt, ...) {
#define MAX_SYM_LEN 256
  va_list  ap;
  void    *lib;
  int      ret=ECORE_CONFIG_ERR_SUCC;
  char     buf[MAX_SYM_LEN]="\0";

  if(!libr)
    return ECORE_CONFIG_ERR_FAIL;

  if(!name)
    name=file;

  *libr=NULL;

  if((lib=dlopen(file,flag))) {
    void   **funr,*fun;
    char    *b,*e;
    size_t   l;
    int      required=1;

    va_start(ap,fmt);
    while(*fmt) {
      switch(*fmt) {
        case '?':
          required=0;
          fmt++;
          break;

        case '!':
        case '.':
          required=1;
          fmt++;
          break;

        case '\t':
        case '\n':
        case '\r':
        case ';':
        case ',':
        case ' ':
          fmt++;
          break;

        default:
          e=b=(char *)fmt;
          while(*e&&(strchr("?!.,; \t\n\r",*e)==NULL))
            e++;

          fmt=e;
          if(e==b)
            ret=ECORE_CONFIG_ERR_NODATA;
          else if((l=(e-b))>=MAX_SYM_LEN)
            ret=ECORE_CONFIG_ERR_OOM;
          else {
            memcpy(buf,b,l);
            buf[l]='\0';
            funr=va_arg(ap,void **);
            if(!(fun=dlsym(lib,buf))) {
              if((ret=required?ECORE_CONFIG_ERR_NOTFOUND:ECORE_CONFIG_ERR_PARTIAL)==ECORE_CONFIG_ERR_NOTFOUND)
                E(1,"DLMulti: library/plugin/engine \"%s\" (\"%s\") did not contain required function \"%s\"...\n",name,file,buf); }
            E(2,"DLMulti: %p => %p %c\"%s\"\n",fun,funr,required?'!':'?',buf);
            if(funr)
              *funr=fun; }
          required=1; }}

    va_end(ap);

    if((ret==ECORE_CONFIG_ERR_SUCC)||(ret==ECORE_CONFIG_ERR_PARTIAL))
      *libr=lib;
    else
      dlclose(lib); }
  else
    ret=ECORE_CONFIG_ERR_NODATA;

  return ret; }



/*****************************************************************************/



ulong now(long delay) {
  static struct timeval tv;
  ulong                 r;
  gettimeofday(&tv,NULL);
  r=tv.tv_sec*1000+(((float)tv.tv_usec)/1000.0)+delay;
  return r; }



/*****************************************************************************/



int parse_line(char *in,char **o1,char **o2,char **o3,char **o4) {
#define PLMAX 16
  int   c;
  char *p=in;
  char *r[PLMAX];

  for(c=0;c<PLMAX;c++)
    r[c]=NULL;

  c=0;
  if(!in||!*in)
    goto pl_end;

  do {
    while(isspace(*p))
      *(p++)='\0';

    if(!*p||(strchr("#;",*p)&&(!p[1]||isspace(p[1]))))
      *p='\0';  /* it ends HERE */
    else {
      if(*p=='\"') {
        r[c++]=++p;
        while(*p&&(*p!='\"')) {
          if((*p=='\\')&&p[1])
            p+=2;
          else
            p++; }
        if(*p)
          *(p++)='\0'; }
      else {
        r[c++]=p;
        while(*p&&!isspace(*p))
          p++; }}
  } while(*p);

  pl_end:
  if(o1) *o1=r[0];
  if(o2) *o2=r[1];
  if(o3) *o3=r[2];
  if(o4) *o4=r[3];

  return c; }



/*****************************************************************************/



/*char *unit_size(char *size) {
  gchar *unit="byte";
  long s;

  if((s=atol(size))&&(s>=1024)) {
    if((s>(1024*1024*1024))) {
      unit="GB";
      s=(long)(s/(1024*1024)); }
    else if((s>(1024*1024))) {
      unit="MB";
      s=(long)(s/1024); }
    else
      unit="KB";

    if((s/1024)>31)
      sprintf(size,"%ld %s",(long)(s/1024),unit);
    else
      sprintf(size,"%.1f %s",((float)s)/1024,unit); }
  else
    sprintf(size,"%ld %s",s,unit);

  return size; }

*/

/*****************************************************************************/



void qsrt(void *a[],void *data,int lo,int hi,int (*compare)(const void *,const void *,const void *)) {
  int   h,l;
  void *p,*t;

  if(lo<hi) {
    l=lo;
    h=hi;
    p=a[hi];

    do {
      while((l<h)&&(compare(data,a[l],p)<=0))
        l=l+1;
      while((h>l)&&(compare(data,a[h],p)>=0))
        h=h-1;
      if(l<h) {
        t=a[l];
        a[l]=a[h];
        a[h]=t; }
    } while(l<h);

    t=a[l];
    a[l]=a[hi];
    a[hi]=t;

    qsrt(a,data,lo,l-1,compare);
    qsrt(a,data,l+1,hi,compare); }}





/*****************************************************************************/
/* TIMERS */
/**********/



ulong timeout_add(uint f,int(*fun)(void *),void *data) {
#ifdef HAVE_GLIB2
  return g_timeout_add((guint)f,(GSourceFunc)fun,(gpointer)data);
#endif
  return 0; }



int timeout_remove(ulong handle) {
#ifdef HAVE_GLIB2
  return g_source_remove(handle)?ECORE_CONFIG_ERR_SUCC:ECORE_CONFIG_ERR_FAIL;
#endif
  return ECORE_CONFIG_ERR_NOTSUPP; }






/*****************************************************************************/
/* HASHES */
/**********/



int eslist_free(eslist **l) {
  eslist *e,*f;
  if(!l)
    return ECORE_CONFIG_ERR_NODATA;
  for(e=*l;e;e=f) {
    f=e->next;
    free(e); }
  *l=NULL;
  return ECORE_CONFIG_ERR_SUCC; }



int eslist_next(eslist **e) {
  if(!e||!*e)
    return ECORE_CONFIG_ERR_NODATA;
   *e=(*e)->next;
   return ECORE_CONFIG_ERR_SUCC; }



void *eslist_payload(eslist **e) {
  return (!e||!*e)?NULL:(*e)->payload; }



int eslist_prepend(eslist **e,void *p) {
  eslist *f;

  if(!e)
    return ECORE_CONFIG_ERR_NODATA;

  if(!(f=malloc(sizeof(eslist))))
    return ECORE_CONFIG_ERR_OOM;

  f->payload=p;
  f->next=*e;

  *e=f;

  return ECORE_CONFIG_ERR_SUCC; }



int eslist_append(eslist **e,void *p) {
  eslist *f;

  if(!e)
    return ECORE_CONFIG_ERR_NODATA;

  if(!(f=malloc(sizeof(eslist))))
    return ECORE_CONFIG_ERR_OOM;

  f->payload=p;
  f->next=NULL;

  if(!*e)
    *e=f;
  else {
    eslist *g=*e;
    while(g->next)
      g=g->next;
    g->next=f; }

  return ECORE_CONFIG_ERR_SUCC; }






/*****************************************************************************/
/* HASHES */
/**********/



void *hash_table_new(void (*freekey),void (*freeval)) {
#ifdef HAVE_GLIB2
  return g_hash_table_new_full(g_str_hash,g_str_equal,freekey,freeval);
#endif
  return NULL; }



void *hash_table_fetch(void *hashtable,char *key) {
#ifdef HAVE_GLIB2
  return g_hash_table_lookup(hashtable,key);
#endif
  return NULL; }



int hash_table_insert(void *hashtable,char *key,void *value) {
#ifdef HAVE_GLIB2
  g_hash_table_insert(hashtable,key,value);
  return ECORE_CONFIG_ERR_SUCC;
#endif
 return ECORE_CONFIG_ERR_NOTSUPP; }



int hash_table_replace(void *hashtable,char *key,void *value) {
#ifdef HAVE_GLIB2
  g_hash_table_replace(hashtable,key,value);
  return ECORE_CONFIG_ERR_SUCC;
#endif
 return ECORE_CONFIG_ERR_NOTSUPP; }



int hash_table_remove(void *hashtable,char *key) {
#ifdef HAVE_GLIB2
  g_hash_table_remove(hashtable,key);
  return ECORE_CONFIG_ERR_SUCC;
#endif
 return ECORE_CONFIG_ERR_NOTSUPP; }



int hash_table_dst(void *hashtable) {
#ifdef HAVE_GLIB2
  g_hash_table_destroy(hashtable);
  return ECORE_CONFIG_ERR_SUCC;
#endif
 return ECORE_CONFIG_ERR_NOTSUPP; }



int hash_table_walk(void *hashtable,hash_walker fun,void *data) {
#ifdef HAVE_GLIB2
  g_hash_table_foreach(hashtable,(GHFunc)fun,data);
  return ECORE_CONFIG_ERR_SUCC;
#endif
 return ECORE_CONFIG_ERR_NOTSUPP; }






/*****************************************************************************/
/* STRINGS */
/***********/



estring *estring_new(int size) {
  estring *e=malloc(sizeof(estring));
  if(e) {
    memset(e,0,sizeof(estring));
    if((size>0)&&(e->str=malloc(size)))
      e->alloc=size; }
  return e; }

estring *estring_dst(estring *e) {
  if(e) {
    if(e->str)
      free(e->str);
    free(e); }
  return NULL; }

char *estring_disown(estring *e) {
  if(e) {
    char *r=e->str;
    free(e);
    return r; }
  return NULL; }

char *estring_free(estring *e,int release_payload) {      /* glib compat */
  if(release_payload) {
    estring_dst(e);
    return NULL; }
  return estring_disown(e); }

int estring_truncate(estring *e,int size) {
  if(!e||(size<0))
    return ECORE_CONFIG_ERR_FAIL;
  if(e->used<=size)
    return e->used;
  e->str[size]='\0';
  e->used=size;
  return size; }

int estring_printf(estring *e,char *fmt, ...) {
  int      need;
  va_list  ap;
  char    *p;

  if(!e)
    return ECORE_CONFIG_ERR_FAIL;

  if(!(e->str)) {
    if(!(e->str=(char*)malloc(e->alloc=512)))
      return ECORE_CONFIG_ERR_OOM; }

retry:
  va_start(ap,fmt);
  need=vsnprintf(e->str,e->alloc,fmt,ap);
  va_end(ap);

  if((need>=e->alloc)||(need<0)) {
    if(need<0)
      need=2*e->alloc;
    else
      need++;
    if(!(p=(char*)realloc(e->str,need))) {
      free(e->str);
      e->alloc=e->used=0;
      return ECORE_CONFIG_ERR_OOM; }
    e->alloc=need;
    e->str=p;
    goto retry; }

  return e->used=need; }

int estring_appendf(estring *e,char *fmt, ...) {
  int      need;
  va_list  ap;
  char    *p;

  if(!e)
    return ECORE_CONFIG_ERR_FAIL;

  if(!e->str) {
    e->used=e->alloc=0;
    if(!(e->str=(char*)malloc(e->alloc=512)))
      return ECORE_CONFIG_ERR_OOM; }

retry:
  va_start(ap,fmt);
  need=vsnprintf(e->str+e->used,e->alloc-e->used,fmt,ap);
  va_end(ap);

  if((need>=(e->alloc-e->used))||(need<0)) {
    if(need<0)
      need=2*e->alloc;
    else
      need++;
    need+=e->used;
    need+=(CHUNKLEN-(need%CHUNKLEN));

    if(!(p=(char*)realloc(e->str,need))) {
      free(e->str);
      e->alloc=e->used=0;
      return ECORE_CONFIG_ERR_OOM; }
    e->alloc=need;
    e->str=p;
    goto retry; }

  return e->used+=need; }



int esprintf(char **result,char *fmt, ...) {
  int      need,have;
  va_list  ap;
  char    *n;

  if(!result)
    return ECORE_CONFIG_ERR_FAIL;

  if(!(n=(char*)malloc(have=512)))
    return ECORE_CONFIG_ERR_OOM;

retry:
  va_start(ap,fmt);
  need=vsnprintf(n,have,fmt,ap);
  va_end(ap);

  if((need>=have)||(need<0)) {
    char *p;
    if(need<0)
      need=2*have;
    else
      need++;
    if(!(p=(char*)realloc(n,need))) {
      free(n);
      return ECORE_CONFIG_ERR_OOM; }
    have=need;
    n=p;
    goto retry; }

  if(*result)
    free(*result);
  *result=n;

  return need; }



#if 0
int ejoin(char **result,char *delim, ...) {
  int      dl,cl,ret=ECORE_CONFIG_ERR_SUCC;
  va_list  ap;
  char    *e,*n;

  if(!result)
    return ECORE_CONFIG_ERR_FAIL;
  if(!delim)
    delim="";
  dl=strlen(delim);

  va_start(ap,delim);
  cl=-dl;
  while((e=va_arg(ap,char *)))
    cl+=strlen(e)+dl;
  va_end(ap);

  if(cl<=0) {
    if(!(n=strdup("")))
      ret=ECORE_CONFIG_ERR_OOM; }
  else if(!(n=malloc(cl+1)))
    ret=ECORE_CONFIG_ERR_OOM;
  else {
    char *p=n;

    va_start(ap,delim);
    while((e=va_arg(ap,char *))) {
      if(dl&&(p!=n)) {
	strcpy(p,delim);
	p+=dl; }
      strcpy(p,e);
      p+=strlen(p); }
    va_end(ap); }

  if(*result)
    free(*result);
  *result=n;

  return ret; }



int ecat(char **result, ...) {
  int      cl,ret=ECORE_CONFIG_ERR_SUCC;
  va_list  ap;
  char    *e,*n;

  if(!result)
    return ECORE_CONFIG_ERR_FAIL;

  va_start(ap,result);
  cl=0;
  while((e=va_arg(ap,char *)))
    cl+=strlen(e);
  va_end(ap);

  if(cl<=0) {
    if(!(n=strdup("")))
      ret=ECORE_CONFIG_ERR_OOM; }
  else if(!(n=malloc(cl+1)))
    ret=ECORE_CONFIG_ERR_OOM;
  else {
    char *p=n;

    va_start(ap,result);
    while((e=va_arg(ap,char *))) {
      strcpy(p,e);
      p+=strlen(p); }
    va_end(ap); }

  if(*result)
    free(*result);
  *result=n;

  return ret; }
#endif






/*****************************************************************************/
