#include "Ecore_Config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* this should only be built if evas is present */

int
ecore_config_evas_font_path_apply (Evas * evas)
{
  char *font_path, *font_path_tmp, *ptr, *end;

  font_path = ecore_config_get_string ("/e/font/path");

  if (!font_path)
    return ECORE_CONFIG_ERR_NODATA;
  ptr = font_path;
  end = font_path + strlen (font_path);
  font_path_tmp = font_path;
  while (ptr && ptr < end)
    {
      while (*ptr != '|' && ptr < end)
        ptr++;
      if (ptr < end)
        *ptr = '\0';
printf("appending font %s\n", font_path_tmp);
      evas_font_path_append (evas, font_path_tmp);
      ptr++;
      font_path_tmp = ptr;
    }

  free (font_path);

  return ECORE_CONFIG_ERR_SUCC;
}

