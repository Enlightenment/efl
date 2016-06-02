/* EINA - EFL data type library
 * Copyright (C) 2016 Expertise Solutions
 *                    Larry Lira Jr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Eina.h>

#include "eina_suite.h"


static Eina_Bool
eina_simple_json_parser_parse_with_custom_callback_obj_cb(void*, Eina_Simple_JSON_Value_Type, const char*, const char*, unsigned);

static const char *get_file_full_path(const char *filename)
{
   static char full_path[PATH_MAX] = "";
   struct stat st;

   eina_str_join(full_path, sizeof(full_path), '/', TESTS_SRC_DIR, filename);

   if (stat(full_path, &st) == 0)
     return full_path;

   if (full_path[0] != '/')
     {
        snprintf(full_path, sizeof(full_path), "%s/%s/%s", TESTS_WD, TESTS_SRC_DIR, filename);

        if (stat(full_path, &st) == 0)
          return full_path;
     }

   return NULL;
}

enum simple_json_parser_current_state
{
    simple_json_parser_current_state_begin,
    simple_json_parser_current_state_gpx,
    simple_json_parser_current_state_metadata,
    simple_json_parser_current_state_link,
    simple_json_parser_current_state_text,
    simple_json_parser_current_state_visible,
    simple_json_parser_current_state_hidden,
    simple_json_parser_current_state_parent,
    simple_json_parser_current_state_time,
    simple_json_parser_current_state_trk,
    simple_json_parser_current_state_name,
    simple_json_parser_current_state_trkseg,
    simple_json_parser_current_state_trkpt,
    simple_json_parser_current_state_lon_1,
    simple_json_parser_current_state_lon_2,
    simple_json_parser_current_state_lon_3,
    simple_json_parser_current_state_lon_4,
    simple_json_parser_current_state_end
};

static Eina_Bool
eina_simple_json_parser_parse_with_custom_callback_cb(void *data, Eina_Simple_JSON_Type type, const char *content, unsigned length)
{
   fail_if(type != EINA_SIMPLE_JSON_OBJECT);

   eina_simple_json_object_parse(content, length, eina_simple_json_parser_parse_with_custom_callback_obj_cb, data);
   return EINA_TRUE;
}

static Eina_Bool
eina_simple_json_parser_parse_with_custom_callback_array_cb(void *data, Eina_Simple_JSON_Value_Type type, const char *content, unsigned length)
{
   fail_if(type != EINA_SIMPLE_JSON_VALUE_OBJECT);

   eina_simple_json_object_parse(content, length, eina_simple_json_parser_parse_with_custom_callback_obj_cb, data);
   return EINA_TRUE;
}

static Eina_Bool
eina_simple_json_parser_parse_with_custom_callback_obj_cb(void *data,
                                                         Eina_Simple_JSON_Value_Type type,
                                                         const char *key,
                                                         const char *content,
                                                         unsigned length)
{
    int* parse_current_state = (int*) data;

    fail_if(type == EINA_SIMPLE_JSON_VALUE_ERROR);

    if (!strcmp("gpx", key))
      {
          fail_if(*parse_current_state != simple_json_parser_current_state_begin);
          *parse_current_state = simple_json_parser_current_state_gpx;
      }
    else if (!strcmp("metadata", key))
      {
          fail_if(*parse_current_state != simple_json_parser_current_state_gpx);
          *parse_current_state = simple_json_parser_current_state_metadata;
      }
    else if (!strcmp("link", key))
      {
          fail_if(*parse_current_state != simple_json_parser_current_state_metadata);
          *parse_current_state = simple_json_parser_current_state_link;
      }
    else if (!strcmp("text", key))
      {
          fail_if(type != EINA_SIMPLE_JSON_VALUE_STRING);
          fail_if(*parse_current_state != simple_json_parser_current_state_link);
          *parse_current_state = simple_json_parser_current_state_text;
      }
    else if (!strcmp("time", key))
      {
          fail_if(type != EINA_SIMPLE_JSON_VALUE_STRING);
          fail_if(*parse_current_state != simple_json_parser_current_state_text);
          *parse_current_state = simple_json_parser_current_state_time;
      }
    else if (!strcmp("visible", key))
      {
          fail_if(type != EINA_SIMPLE_JSON_VALUE_BOOLEAN);
          fail_if(*parse_current_state != simple_json_parser_current_state_time);
          *parse_current_state = simple_json_parser_current_state_visible;
      }
    else if (!strcmp("hidden", key))
      {
          fail_if(type != EINA_SIMPLE_JSON_VALUE_BOOLEAN);
          fail_if(*parse_current_state != simple_json_parser_current_state_visible);
          *parse_current_state = simple_json_parser_current_state_hidden;
      }
    else if (!strcmp("parent", key))
      {
          fail_if(type != EINA_SIMPLE_JSON_VALUE_NULL);
          fail_if(*parse_current_state != simple_json_parser_current_state_hidden);
          *parse_current_state = simple_json_parser_current_state_parent;
      }
    else if (!strcmp("trk", key))
      {
          fail_if(*parse_current_state != simple_json_parser_current_state_parent);
          *parse_current_state = simple_json_parser_current_state_trk;
      }
    else if (!strcmp("name", key))
      {
          fail_if(*parse_current_state != simple_json_parser_current_state_trk);
          *parse_current_state = simple_json_parser_current_state_name;
      }
    else if (!strcmp("trkseg", key))
      {
          fail_if(*parse_current_state != simple_json_parser_current_state_name);
          *parse_current_state = simple_json_parser_current_state_trkseg;
      }
    else if (!strcmp("trkpt", key))
      {
          fail_if(type != EINA_SIMPLE_JSON_VALUE_ARRAY);
          fail_if(*parse_current_state != simple_json_parser_current_state_trkseg);
          *parse_current_state = simple_json_parser_current_state_trkpt;
          eina_simple_json_array_parse(content, length,
                      eina_simple_json_parser_parse_with_custom_callback_array_cb,
                      parse_current_state);

          fail_if(*parse_current_state != simple_json_parser_current_state_lon_4);
          *parse_current_state = simple_json_parser_current_state_end;
      }
    else if (!strcmp("@lon", key))
      {
          fail_if(type != EINA_SIMPLE_JSON_VALUE_DOUBLE);
          switch(*parse_current_state)
            {
              case simple_json_parser_current_state_trkpt:
              case simple_json_parser_current_state_lon_1:
              case simple_json_parser_current_state_lon_2:
              case simple_json_parser_current_state_lon_3:
                *parse_current_state = *parse_current_state+1;
                break;
              default:
                ck_abort_msg("Error Parse Json elements");
            }
      }

    if (type == EINA_SIMPLE_JSON_VALUE_OBJECT)
      {
          eina_simple_json_object_parse(content, length,
                    eina_simple_json_parser_parse_with_custom_callback_obj_cb,
                    parse_current_state);
      }

    return EINA_TRUE;
}

START_TEST(eina_simple_json_parser_parse_with_custom_callback)
{
    FILE *f;

    eina_init();
    f = fopen(get_file_full_path("sample.json"), "rb");

    if (f)
      {
        long sz;

        fseek(f, 0, SEEK_END);
        sz = ftell(f);

        if (sz > 0)
          {
            char *buf;

            fseek(f, 0, SEEK_SET);
            buf = malloc(sz + 1);

            if (buf)
              {
                if (fread(buf, 1, sz, f))
                  {
                    int parse_current_state = simple_json_parser_current_state_begin;
                    eina_simple_json_parse(buf,
                                          sz,
                                          eina_simple_json_parser_parse_with_custom_callback_cb,
                                          &parse_current_state);
                    fail_if(parse_current_state != simple_json_parser_current_state_end);
                  }
                free(buf);
              }
          }

        fclose(f);
      }

    eina_shutdown();
}
END_TEST

START_TEST(eina_simple_json_parser_parse_empty_object)
{
   const char *str = "{  }";
   int length = strlen(str);
   eina_init();
   ck_assert(eina_simple_json_object_parse("{ }", length, NULL, NULL));

   eina_shutdown();
}
END_TEST

START_TEST(eina_simple_json_parser_parse_empty_invalid_object)
{
   const char *str = "{  }";
   int length = strlen(str);
   eina_init();
   ck_assert(!eina_simple_json_object_parse("{\" }", length, NULL, NULL));

   eina_shutdown();
}
END_TEST

void
eina_test_simple_json_parser(TCase *tc)
{
   tcase_add_test(tc, eina_simple_json_parser_parse_with_custom_callback);
   tcase_add_test(tc, eina_simple_json_parser_parse_empty_object);
   tcase_add_test(tc, eina_simple_json_parser_parse_empty_invalid_object);
}
