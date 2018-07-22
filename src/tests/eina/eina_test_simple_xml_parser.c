/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

EFL_START_TEST(eina_simple_xml_parser_node_dump)
{
   FILE *f;

   f = fopen(get_file_full_path("sample.gpx"), "rb");
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
                       Eina_Simple_XML_Node_Root *root = eina_simple_xml_node_load
                         (buf, sz, EINA_TRUE);
                       buf[sz] = '\0';
                       char *out = eina_simple_xml_node_dump(&root->base, "  ");
                       //puts(out);
                       ck_assert_str_eq(out, buf);
                       free(out);
                       eina_simple_xml_node_root_free(root);
                    }
                  free(buf);
               }
          }
        fclose(f);
     }

}
EFL_END_TEST

EFL_START_TEST(eina_simple_xml_parser_null_node_dump)
{
   
   char *out = eina_simple_xml_node_dump(NULL, "  ");
   fail_if(out != NULL);

}
EFL_END_TEST

EFL_START_TEST(eina_simple_xml_parser_childs_count)
{

    const char *buf = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
	    "<test version=\"0.1\"><child>I'm a child.</child><child><![CDATA[I'm a 2-nd child.]]></child><!-- Some comment --></test>";
    const int sz = strlen(buf);
    Eina_Simple_XML_Node_Root *root = eina_simple_xml_node_load(buf, sz, EINA_TRUE);
    fail_if(root == NULL);
    fail_if(root->children == NULL);
    fail_if(eina_inlist_count(root->children) != 2);
    eina_simple_xml_node_root_free(root);

}
EFL_END_TEST

enum simple_xml_parser_current_state
{
    simple_xml_parser_current_state_begin,
    simple_xml_parser_current_state_gpx,
    simple_xml_parser_current_state_metadata,
    simple_xml_parser_current_state_link,
    simple_xml_parser_current_state_text,
    simple_xml_parser_current_state_time,
    simple_xml_parser_current_state_trk,
    simple_xml_parser_current_state_name,
    simple_xml_parser_current_state_trkseg,
    simple_xml_parser_current_state_trkpt,
    simple_xml_parser_current_state_comment,
    simple_xml_parser_current_state_cdata,
    simple_xml_parser_current_state_end
};

static Eina_Bool
eina_simple_xml_parser_parse_with_custom_callback_tag_cb(void *data,
                                                         Eina_Simple_XML_Type type,
                                                         const char *content,
                                                         unsigned offset EINA_UNUSED,
                                                         unsigned length EINA_UNUSED)
{
    int* parse_current_state = (int*) data;

    if (type == EINA_SIMPLE_XML_OPEN)
      {
        if (!strncmp("gpx ", content, strlen("gpx ")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_begin);
            *parse_current_state = simple_xml_parser_current_state_gpx;
          }
        else if (!strncmp("metadata>", content, strlen("metadata>")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_gpx);
            *parse_current_state = simple_xml_parser_current_state_metadata;
          }
        else if (!strncmp("link ", content, strlen("link ")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_metadata);
            *parse_current_state = simple_xml_parser_current_state_link;
          }
        else if (!strncmp("text>", content, strlen("text>")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_link);
            *parse_current_state = simple_xml_parser_current_state_text;
          }
        else if (!strncmp("time>", content, strlen("time>")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_text);
            *parse_current_state = simple_xml_parser_current_state_time;
          }
        else if (!strncmp("trk>", content, strlen("trk>")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_time);
            *parse_current_state = simple_xml_parser_current_state_trk;
          }
        else if (!strncmp("name>", content, strlen("name>")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_trk);
            *parse_current_state = simple_xml_parser_current_state_name;
          }
        else if (!strncmp("trkseg>", content, strlen("trkseg>")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_name);
            *parse_current_state = simple_xml_parser_current_state_trkseg;
          }
        else
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_trkpt);
          }
      }
    else if (type == EINA_SIMPLE_XML_OPEN_EMPTY)
      {
        if (!strncmp("trkpt ", content, strlen("trkpt ")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_trkseg &&
                    *parse_current_state != simple_xml_parser_current_state_trkpt);
            *parse_current_state = simple_xml_parser_current_state_trkpt;
          }
      }
    else if (type == EINA_SIMPLE_XML_COMMENT)
      {
        fail_if(*parse_current_state != simple_xml_parser_current_state_trkpt);
        *parse_current_state = simple_xml_parser_current_state_comment;
      }
    else if (type == EINA_SIMPLE_XML_CDATA)
      {
        fail_if(*parse_current_state != simple_xml_parser_current_state_comment);
        *parse_current_state = simple_xml_parser_current_state_cdata;
      }
    else if (type == EINA_SIMPLE_XML_CLOSE)
      {
        if (!strncmp("gpx", content, strlen("gpx")))
          {
            fail_if(*parse_current_state != simple_xml_parser_current_state_cdata);
            *parse_current_state = simple_xml_parser_current_state_end;
          }
      }
    return EINA_TRUE;
}

EFL_START_TEST(eina_simple_xml_parser_parse_with_custom_callback)
{
    FILE *f;

    f = fopen(get_file_full_path("sample.gpx"), "rb");

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
                    int parse_current_state = simple_xml_parser_current_state_begin;
                    eina_simple_xml_parse(buf,
                                          sz,
                                          EINA_TRUE,
                                          eina_simple_xml_parser_parse_with_custom_callback_tag_cb,
                                          &parse_current_state);
                    fail_if(parse_current_state != simple_xml_parser_current_state_end);
                  }
                free(buf);
              }
          }

        fclose(f);
      }

}
EFL_END_TEST

void
eina_test_simple_xml_parser(TCase *tc)
{
   tcase_add_test(tc, eina_simple_xml_parser_node_dump);
   tcase_add_test(tc, eina_simple_xml_parser_null_node_dump);
   tcase_add_test(tc, eina_simple_xml_parser_childs_count);
   tcase_add_test(tc, eina_simple_xml_parser_parse_with_custom_callback);
}
