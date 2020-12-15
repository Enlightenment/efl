#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "eina_fnmatch.h"

// ref: http://kirste.userpage.fu-berlin.de/chemnet/use/info/libc/libc_16.html

int 
test_normal_string(void)
{
   const char * string = "Yep. This is just a n0rmal string!";
   const char * pattern = "*";
   const int flags = 0;

   int ret = 0;

   ret |= eina_fnmatch(pattern, string, flags);
   printf("test normal string (T): %cok!\n", !ret?:'n');

   return ret;
}

int 
test_unix_path(void)
{
   const char * string_valid = "~/linux/usr/include/bits/types/__fpos64_t.h";
   const char * string_invalid = "~/linux/usr/include/bits/types/__fpos64_t_h";
   const char * pattern = "~/linux/usr/include/bits/types/__fpos64_t.h";
   const int flags = FNM_PATHNAME | FNM_NOESCAPE | FNM_PERIOD;

   int ret = 0;

   ret |= eina_fnmatch(pattern, string_valid, flags);
   printf("test unix path (T): %cok!\n", !ret?:'n');

   ret |= !eina_fnmatch(pattern, string_invalid, flags);
   printf("test unix path (F): %cok!\n", !ret?:'n');

   return ret;
}

int 
test_win_path(void)
{
   const char * string_valid = "C:\\Program Files (x86)[1]<3>$4?!\\Git";
   const char * string_invalid = "\\Program Files (x86)\\Git";
   const char * pattern = "?:\\Program Files*\\Git";
   const int flags = FNM_PATHNAME | FNM_NOESCAPE | FNM_PERIOD;
                                                            
   int ret = 0;

   ret |= eina_fnmatch(pattern, string_valid, flags);
   printf("test win path (T): %cok!\n", !ret?:'n');

   ret |= !eina_fnmatch(pattern, string_invalid, flags);
   printf("test win path (F): %cok!\n", !ret?:'n');

   return ret;
}

int 
test_flag_pathname(void)
{
   const char * string = "/usr/share/clang/";
   const char * pattern_valid = "/*/*/*/";
   const char * pattern_invalid = "*";
   const int flags = FNM_PATHNAME;
                                                            
   int ret = 0;

   ret |= eina_fnmatch(pattern_valid, string, 0);
   printf("test FNM_PATHNAME flag (T): %cok!\n", !ret?:'n');
   
   ret |= eina_fnmatch(pattern_invalid, string, 0);
   printf("test FNM_PATHNAME flag (T): %cok!\n", !ret?:'n');

   ret |= eina_fnmatch(pattern_valid, string, flags);
   printf("test FNM_PATHNAME flag (T): %cok!\n", !ret?:'n');
   
   ret |= !eina_fnmatch(pattern_invalid, string, flags);
   printf("test FNM_PATHNAME flag (F): %cok!\n", !ret?:'n'); 

   return ret;
}

int 
test_flag_period(void)
{
   const char * string = ".config/.isolated-storage/d.none.FBD7";
   const char * pattern_valid = ".*/*/*";
   const char * pattern_invalid = "*/*/*";
   const int flags = FNM_PERIOD;
                                                            
   int ret = 0;

   ret |= eina_fnmatch(pattern_valid, string, 0);
   printf("test FNM_PERIOD flag (T): %cok!\n", !ret?:'n');
   
   ret |= eina_fnmatch(pattern_invalid, string, 0);
   printf("test FNM_PERIOD flag (T): %cok!\n", !ret?:'n');

   ret |= eina_fnmatch(pattern_valid, string, flags);
   printf("test FNM_PERIOD flag (T): %cok!\n", !ret?:'n');
   
   ret |= !eina_fnmatch(pattern_invalid, string, flags);
   printf("test FNM_PERIOD flag (F): %cok!\n", !ret?:'n');

   return ret;
} 

int 
test_flag_pathname_period(void)
{
   const char * string = ".config/.isolated-storage/d.none.FBD7";
   const char * pattern_valid = ".*/.*/*";
   const char * pattern_invalid = ".*/*/*";
   const int flags = FNM_PATHNAME | FNM_PERIOD;
                                                            
   int ret = 0;

   ret |= eina_fnmatch(pattern_valid, string, 0);
   printf("test FNM_PATHNAME+FNM_PERIOD flag (T): %cok!\n", !ret?:'n');
   
   ret |= eina_fnmatch(pattern_invalid, string, 0);
   printf("test FNM_PATHNAME+FNM_PERIOD flag (T): %cok!\n", !ret?:'n');

   ret |= eina_fnmatch(pattern_valid, string, flags);
   printf("test FNM_PATHNAME+FNM_PERIOD flag (T): %cok!\n", !ret?:'n');
   
   ret |= !eina_fnmatch(pattern_invalid, string, flags);
   printf("test FNM_PATHNAME+FNM_PERIOD flag (F): %cok!\n", !ret?:'n');

   return ret;
}

int 
test_flag_noescape(void)
{
   const char * string_valid = "C:\\Program Files";
   const char * pattern = "?:\\*";
   const int flags = FNM_NOESCAPE;
                                                            
   int ret = 0;

   ret |= eina_fnmatch(pattern, string_valid, flags);
   printf("test FNM_NOESCAPE flag (T): %cok!\n", !ret?:'n');

   ret |= !eina_fnmatch(pattern, string_valid, 0);
   printf("test FNM_NOESCAPE flag (F): %cok!\n", !ret?:'n');

   return ret;
}
 
int 
main(void)
{
   printf("---\n");
   test_normal_string();
   printf("---\n");
   test_flag_pathname();
   printf("---\n");
   test_flag_period();
   printf("---\n");
   test_flag_pathname_period();
   printf("---\n");
   test_flag_noescape();
   printf("---\n");
   test_unix_path();
   printf("---\n");
   test_win_path();

   return 0;
}       
