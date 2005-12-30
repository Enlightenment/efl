#define TIMER_STOP 0
#define TIMER_CONT 1

typedef struct _estring
{
   char               *str;
   int                 alloc, used;
} estring;

estring            *estring_new(int size);
char               *estring_disown(estring * e);
int                 estring_appendf(estring * e, const char *fmt, ...);

int                 esprintf(char **result, const char *fmt, ...);
