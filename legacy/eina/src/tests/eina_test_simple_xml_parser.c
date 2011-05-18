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

#include "eina_suite.h"
#include "Eina.h"

START_TEST(eina_simple_xml_parser_node_dump)
{
   FILE *f;

   eina_init();
   f = fopen("sample.gpx", "rb");
   if (f)
     {
        long sz;

        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        if (sz > 0)
          {
             char *buf;

             fseek(f, 0, SEEK_SET);
             buf = malloc(sz);
             if (buf)
               {
                  if (fread(buf, 1, sz, f))
                    {
                       Eina_Simple_XML_Node_Root *root = eina_simple_xml_node_load
                         (buf, sz, EINA_TRUE);
                       char *out = eina_simple_xml_node_dump(&root->base, "  ");
                       puts(out);
                       free(out);
                       eina_simple_xml_node_root_free(root);
                       free(buf);
                    }
               }
          }
        fclose(f);
     }

   eina_shutdown();
}
END_TEST

void
eina_test_simple_xml_parser(TCase *tc)
{
   tcase_add_test(tc, eina_simple_xml_parser_node_dump);
}
