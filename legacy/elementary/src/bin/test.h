#ifndef _TEST_H
#define _TEST_H

#define fail_if(expr) \
   do { \
        if ( expr ) \
          { \
             FILE *fp; \
             char buf[1024]; \
             sprintf(buf, "fail_%s.txt", elm_win_title_get(win)); \
             if ((fp = fopen(buf, "a")) != NULL) \
               { \
                  fprintf(fp, "Failed at %s:%d on <%s>\n", \
                        __FILE__, __LINE__, #expr); \
                  fclose(fp); \
               } \
          } \
   } while(0)
#endif

#define CRITICAL(...) printf(__VA_ARGS__)
#define ERR(...)      printf(__VA_ARGS__)
#define WRN(...)      printf(__VA_ARGS__)
#define INF(...)      printf(__VA_ARGS__)
#define DBG(...)      printf(__VA_ARGS__)
