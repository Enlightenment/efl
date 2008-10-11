
#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Evil.h>
#include <fnmatch.h>

const char *translate_return_value(int ret)
{
   static char buf[256] = "";
   switch (ret)
   {
   case 0:
      strcpy(buf, "Match");
      break;
   case FNM_NOMATCH:
      strcpy(buf, "NoMatch");
      break;
   case FNM_SYNTAXERR:
      strcpy(buf, "SyntaxErr");
      break;
   case FNM_NOMEM:
      strcpy(buf, "NoMem");
      break;
   default:
      sprintf(buf, "%d", ret);
      break;
   }
   return buf;
}

#define TEST_CASE(a) printf(#a " = %s\n", translate_return_value(a));

int main()
{
   TEST_CASE(fnmatch("", "", 0));
   TEST_CASE(fnmatch("a", "a", 0));
   TEST_CASE(fnmatch("a", "b", 0));
   TEST_CASE(fnmatch("a", "ba", 0));
   TEST_CASE(fnmatch("a", "ab", 0));
   TEST_CASE(fnmatch("a", "ba", 0));
   TEST_CASE(fnmatch("a", "b a", 0));
   TEST_CASE(fnmatch("a", "aa", 0));
   TEST_CASE(fnmatch("ab", "ab", 0));
   TEST_CASE(fnmatch("ab", "ab ", 0));
   TEST_CASE(fnmatch("ab", " ab", 0));
   TEST_CASE(fnmatch("ab", "abc", 0));
   TEST_CASE(fnmatch("ab", "ab c", 0));
   TEST_CASE(fnmatch("ab", "ba", 0));

   TEST_CASE(fnmatch("a*", "a", 0));
   TEST_CASE(fnmatch("a*", "b", 0));
   TEST_CASE(fnmatch("a*", "ab", 0));
   TEST_CASE(fnmatch("a*", "ba", 0));
   TEST_CASE(fnmatch("a*", "aba", 0));

   TEST_CASE(fnmatch("a?", "a", 0));
   TEST_CASE(fnmatch("a?", "b", 0));
   TEST_CASE(fnmatch("a?", "ab", 0));
   TEST_CASE(fnmatch("a?", "ba", 0));
   TEST_CASE(fnmatch("a?", "aa", 0));
   TEST_CASE(fnmatch("a?", "aba", 0));
   TEST_CASE(fnmatch("a?", "a a", 0));

   TEST_CASE(fnmatch("*.*", "a", 0));
   TEST_CASE(fnmatch("*.*", "a.", 0));
   TEST_CASE(fnmatch("*.*", ".", 0));
   TEST_CASE(fnmatch("*.*", ".a", 0));
   TEST_CASE(fnmatch("*.*", " . ", 0));
   TEST_CASE(fnmatch("*.*", "aa.aa", 0));

   TEST_CASE(fnmatch("*", "", 0));
   TEST_CASE(fnmatch("*", "a", 0));

   TEST_CASE(fnmatch("*?", "", 0));
   TEST_CASE(fnmatch("*?", "a", 0));

   TEST_CASE(fnmatch("*abc*", "aabbccabcaa", 0));

   TEST_CASE(fnmatch("\\", "", 0));
}
