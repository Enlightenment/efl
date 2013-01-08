#include "Efreet.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
ef_cb_ini_parse(void)
{
    int ret = 1;
    Efreet_Ini *ini;

    putenv("LC_ALL=en_US");

    ini = efreet_ini_new(PACKAGE_DATA_DIR"/test/test.ini");
    if (!ini)
    {
        printf("efreet_ini_parse() Failed to initialize Efreet_Ini\n");
        return 0;
    }

    if (efreet_ini_section_set(ini, "contact"))
    {
        const char *val;
        int ival;
        unsigned int bval;

        val = efreet_ini_string_get(ini, "Name");
        if (!val || strcmp(val, "Foo Bar"))
        {
            printf("efreet_ini_string_get() Name parsed incorrectly\n");
            ret = 0;
        }

        val = efreet_ini_localestring_get(ini, "Name");
        if (!val || strcmp(val, "English Foo Bar"))
        {
            printf("efreet_ini_localestring_get() Name parsed incorrectly\n");
            ret = 0;
        }

        val = efreet_ini_string_get(ini, "Email");
        if (!val || strcmp(val, "foo@bar.com"))
        {
            printf("efreet_ini_string_get() Email parsed incorrectly\n");
            ret = 0;
        }

        val = efreet_ini_localestring_get(ini, "Email");
        if (!val || strcmp(val, "foo@bar.com"))
        {
            printf("efreet_ini_localestring_get() Email parsed incorrectly\n");
            ret = 0;
        }

        ival = efreet_ini_int_get(ini, "Age");
        if (ival != 30)
        {
            printf("efreet_ini_int_get() Age parsed incorrectly\n");
            ret = 0;
        }

        bval = efreet_ini_boolean_get(ini, "TrueBoolean");
        if (!bval)
        {
            printf("efreet_ini_boolean_get() TrueBoolean parsed incorrectly\n");
            ret = 0;
        }

        bval = efreet_ini_boolean_get(ini, "FalseBoolean");
        if (bval)
        {
            printf("efreet_ini_boolean_get() FalseBoolean parsed incorrectly\n");
            ret = 0;
        }

        bval = efreet_ini_boolean_get(ini, "InvalidBoolean");
        if (bval)
        {
            printf("efreet_ini_boolean_get() InvalidBoolean parsed incorrectly\n");
            ret = 0;
        }

        val = efreet_ini_string_get(ini, "Escaped");
        if (!val || strcmp(val, "line1\nline2\r\nline3\ttabbed \\ with a backslash and spaces"))
        {
            printf("efreet_ini_unescape() improperly unescaped value\n");
            ret = 0;
        }
    }
    else
    {
        printf("efreet_ini_section_set() Failed to set 'contact' section\n");
        ret = 0;
    }

    efreet_ini_free(ini);

    return ret;
}

int
ef_cb_ini_long_line(void)
{
  Efreet_Ini *ini;
  int ret = 1;

  struct
  {
    char *key;
    int len;
  } tests[] = {
    {"key", 5099},
    {"key2", 5099},
    {NULL, 0}
  };

  ini = efreet_ini_new(PACKAGE_DATA_DIR"/test/long.ini");
  if (!ini)
  {
    printf("Ini failed to parse.\n");
    ret = 0;
  }

  if (ret) ret = efreet_ini_section_set(ini, "section");
  if (ret)
  {
    const char *val;
    int i, len;

    for (i = 0; tests[i].key; i++)
    {
      val = efreet_ini_string_get(ini, tests[i].key);
      if (val)
      {
        len = strlen(val);
        if (len != tests[i].len)
        {
          printf("Invalid long line parsing. Value length: %d (expected %d)\n", len, tests[i].len);
          ret = 0;
        }
      }
      else
      {
        printf("Key missing: %s.", tests[i].key);
        ret = 0;
      }
    }
  }
  else
  {
    printf("Section missing: 'section'.");
  }

  if (ini) efreet_ini_free(ini);
  return ret;
}

int
ef_cb_ini_garbage(void)
{
  Efreet_Ini *ini;
  int ret = 1;
   
  ini = efreet_ini_new(PACKAGE_DATA_DIR"/test/test_garbage");
  if (!ini)
  {
    printf("Ini failed to parse.\n");
    return 0;
  }
  if (ini->data) ret = 0;
  efreet_ini_free(ini);
  return ret;
}
