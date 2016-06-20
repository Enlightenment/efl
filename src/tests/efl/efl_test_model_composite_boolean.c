/* EFL - EFL library
 * Copyright (C) 2013 Cedric Bail
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

#include "efl_suite.h"

#include <Efl.h>

START_TEST(efl_model_composite_boolean)
{
   eo_init();

   Efl_Model_Composite_Boolean* model
     = eo_add(EFL_MODEL_COMPOSITE_BOOLEAN_CLASS, NULL);

   eo_shutdown();
}
END_TEST

void
efl_test_model_composite_boolean(TCase *tc)
{
   tcase_add_test(tc, efl_model_composite_boolean);
}
