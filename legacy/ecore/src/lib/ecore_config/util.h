#define TIMER_STOP 0
#define TIMER_CONT 1

typedef struct _estring {
  char *str;
  int   alloc,used;
} estring;

typedef struct _eslist {
  void          *payload;
  struct _eslist *next;
} eslist;


int      parse_line(char *,char **,char **,char **,char **);
char    *unit_size(char *size);
/*ulong    now(long delay);*/
void     qsrt(void *a[],void *data,int lo,int hi,int (*compare)(const void *,const void *,const void *));
int      dlmulti(char *name,char *file,int flag,void **libr,const char *fmt, ...);



typedef  void (*hash_walker)(char *key,void *value,void *data);

void    *hash_table_new(    void (*freekey),void (*freeval));
void    *hash_table_fetch(  void *hashtable,char *key);
int      hash_table_insert( void *hashtable,char *key,void *value);
int      hash_table_replace(void *hashtable,char *key,void *value);
int      hash_table_remove( void *hashtable,char *key);
int      hash_table_dst(    void *hashtable);
int      hash_table_walk(   void *hashtable,hash_walker fun,void *data);

int      eslist_free(eslist **);
int      eslist_next(eslist **);
#define  ESLIST_NEXT(e) (e=e->next)
void    *eslist_payload(eslist **);
#define  ESLIST_PAYLOAD(e) ((e)->payload)
int      eslist_prepend(eslist **,void *);
int      eslist_append(eslist **,void *);

estring *estring_new(int size);
estring *estring_dst(estring *e);
char    *estring_disown(estring *e);
char    *estring_free(estring *e,int release_payload);   /* glib compat */
int      estring_printf(estring *e,char *fmt, ...);
int      estring_appendf(estring *e,char *fmt, ...);
int      estring_truncate(estring *e,int size);
#define  ESTRING_GET_CSTRING(a) ((a)->str)

int      esprintf(char **result,char *fmt, ...);
int      ejoin(char **result,char *delim, ...);
int      ecat(char **result, ...);



/*ulong    timeout_add(uint f,int(*fun)(void *),void *data);*/
/*int      timeout_remove(ulong handle);*/

