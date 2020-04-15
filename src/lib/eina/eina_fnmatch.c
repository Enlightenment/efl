#include "eina_fnmatch.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pcreposix.h>

inline static int
_is_escapable(char c)
{
   switch (c)
     {
      case '[': case '|': case '(': case ')': case '*': case '?':
      case '!': case '^': case '$': case '.': case '+': case '\\': 
        return 1;
      default:
        return 0;
     }
}

inline static char *
_wildcards_to_regex(const char *pattern, int flags)
{

   // Counts how many additional chars needs to be allocated
   int pattern_length = strlen(pattern);
   int count = 2;  // starts with '^' and ends with '$'
   for (int j = 0; j < pattern_length; ++j)
     {
        if (pattern[j] == '\\')
          {
             if (flags & FNM_NOESCAPE) count++;
             else count += 2;
          }
        else if ((pattern[j] == '*') || (pattern[j] == '?'))
          {
             if (flags & FNM_PATHNAME)
               {
                  if (flags & FNM_PERIOD)  // PATHNAME + PERIOD
                    {
                       if (pattern[j] == '*')
                         {
                            if (j == 0) count += 10;
                            else if (pattern[j - 1] == '/') count += 10;
                            else count += 5;
                         }
                       else if ((j == 0) || (pattern[j - 1] == '/')) count += 10;
                       else count += 4;
                    }
                  else if (pattern[j] == '*') count += 5;   // PATHNAME 
                  else count += 4;
               }
             else if (flags & FNM_PERIOD)
               {
                  if (j == 0)  // period at init
                    {
                       if (pattern[j] == '*') count += 5;
                       else count += 4;
                    }
                  // period at other places  
                  else if (pattern[j] == '*') count += 2;
                  else count++;
               }
             else if (pattern[j] == '*') count += 2;  // NORMAL 
             else count++;
          }
        else if (pattern[j] == '.') count += 4;
        else count++;  // OTHERS
     }

   int reg_length = pattern_length + count;

   // Translates wildcards to regex
   char *reg_pattern = (char *)malloc(reg_length * sizeof(char));
   if (reg_pattern == NULL) exit(ENOMEM);
   reg_pattern[0] = '^';
   int i = 1;
   for (int j = 0; j < pattern_length; ++j)
     {
        if (pattern[j] == '\\')
          {
             if (flags & FNM_NOESCAPE)
                reg_pattern[i++] = '/';
             else if (_is_escapable(pattern[j + 1]))
               {
                  reg_pattern[i++] = '\\';
                  reg_pattern[i++] = pattern[++j];
               }
             else
               {
                  reg_pattern[i++] = '\\';
                  reg_pattern[i++] = '/';
               }
          }
        else if ((pattern[j] == '*') || (pattern[j] == '?'))
          {
             if (flags & FNM_PATHNAME)
               {
                  if (flags & FNM_PERIOD)  // PATHNAME + PERIOD
                    {
                       if (pattern[j] == '*')
                         {
                            if ( (j == 0) || (pattern[j - 1] == '/') )
                              {
                                 strcpy_s(reg_pattern + i,
                                          sizeof(reg_pattern + i), 
                                          "[^\\.][^/]*");
                                 i += 10;
                              }
                            else
                              {
                                 strcpy_s(reg_pattern + i, 
                                         sizeof(reg_pattern + i), 
                                         "[^/]*");
                                 i += 5;
                              }
                         }
                       else if (j == 0)
                         {
                            strcpy_s(reg_pattern + i, 
                                     sizeof(reg_pattern + i),
                                     "[^\\.][^/]?");
                            i += 10;
                         }
                       else if (pattern[j - 1] == '/')
                         {
                            strcpy_s(reg_pattern + i,
                                     sizeof(reg_pattern + i),
                                     "[^\\.][^/]?");
                            i += 10;
                         }
                       else
                         {
                            strcpy_s(reg_pattern + i,
                                     sizeof(reg_pattern + i),
                                     "[^/]");
                            i += 4;
                         }
                    }
                  else if (pattern[j] == '*')  // PATHNAME 
                    {
                       strcpy_s(reg_pattern + i,
                                sizeof(reg_pattern + i),
                                "[^/]*");
                       i += 5;
                    }
                  else
                    {
                       strcpy_s(reg_pattern + i,
                                sizeof(reg_pattern + i),
                                "[^/]");
                       i += 4;
                    }
               }
             else if (flags & FNM_PERIOD)
               {
                  if (j == 0)  // period at init
                    {
                       if (pattern[j] == '*')
                         {
                            strcpy_s(reg_pattern + i,
                                     sizeof(reg_pattern + i),
                                     "[\\.]*");
                            i += 5;
                         }
                       else
                         {
                            strcpy_s(reg_pattern + i,
                                     sizeof(reg_pattern + i),
                                     "[\\.]");
                            i += 4;
                         }
                    }
                  else if (pattern[j] == '*')  // period at other places 
                    {
                       strcpy_s(reg_pattern + i,
                                sizeof(reg_pattern + i),
                                ".*");
                       i += 2;
                    }
                  else
                    reg_pattern[i++] = '.';
               }
             else if (pattern[j] == '*')  // NORMAL
               {
                  strcpy_s(reg_pattern + i,
                           sizeof(reg_pattern + i),
                           ".*");
                  i += 2;
               }
             else
               {
                  strcpy_s(reg_pattern + i,
                           sizeof(reg_pattern + i),
                           ".");
                  i++;
               }
          }
        else if (pattern[j] == '.')
          {
             strcpy_s(reg_pattern + i, 
                      sizeof(reg_pattern + i),
                      "[\\.]");
             i += 4;
          }
        else reg_pattern[i++] = pattern[j];  // OTHERS
     }
   strcpy_s(reg_pattern + i, 
            sizeof(reg_pattern + i),
            "$\0");

   return reg_pattern;
}

int
eina_fnmatch(const char *pattern, const char *string, int flags)
{
   // Converts wildcard pattern to regex pattern
   char *reg_pattern = _wildcards_to_regex(pattern, flags);

   // Configures regex
   int regex_flags = (REG_NOSUB)  // Report only success/fail in regexec()
                     || ((flags & FNM_CASEFOLD) ? REG_ICASE : 0);

   // Compiles regex
   regex_t regex;
   if (regcomp(&regex, reg_pattern, regex_flags))
     {
        regfree(&regex);
        return FNM_NOMATCH;
     }

   // Replaces '\\' with '/'
   int string_length = strlen(string);
   char *unix_path = (char *)malloc(string_length * sizeof(char));
   if (unix_path == NULL) exit(ENOMEM);
   unix_path[string_length] = '\0';
   for (int i = 0; i < string_length; ++i)
     unix_path[i] = (string[i] == '\\') ? '/' : string[i];

   // Executes regex
   int result = regexec(&regex, unix_path, 0, NULL, 0);

   // Cleans-up and returns
   free(unix_path);
   free(reg_pattern);
   regfree(&regex);
   return result ? FNM_NOMATCH : 0;
}
