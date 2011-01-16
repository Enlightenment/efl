/* EINA - EFL data type library
 * Copyright (C) 2010 Cedric Bail
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

#include <math.h>
#include <stdio.h>

#include "eina_suite.h"
#include "Eina.h"

START_TEST(eina_fp_cos)
{
   Eina_F32p32 fc;
   Eina_F32p32 fl;
   Eina_F32p32 step;
   Eina_F32p32 fresult;
   double dc;
   double dresult;
   double delta;

        fail_if(!eina_init());

   fl = eina_f32p32_scale(EINA_F32P32_PI, 4);
   step = eina_f32p32_div(fl, eina_f32p32_int_from(2048));

   for (fc = 0; fc < fl; fc += step)
     {
        fresult = eina_f32p32_cos(fc);
        dc = eina_f32p32_double_to(fc);
        dresult = cos(dc);

        delta = fabs(dresult - eina_f32p32_double_to(fresult));
        fail_if(delta > 0.005);
     }

        eina_shutdown();
}
END_TEST

START_TEST(eina_fp_sin)
{
   Eina_F32p32 fc;
   Eina_F32p32 fl;
   Eina_F32p32 step;
   Eina_F32p32 fresult;
   double dc;
   double dresult;
   double delta;

        fail_if(!eina_init());

   fl = eina_f32p32_scale(EINA_F32P32_PI, 4);
   step = eina_f32p32_div(fl, eina_f32p32_int_from(2048));

   for (fc = 0; fc < fl; fc += step)
     {
        fresult = eina_f32p32_sin(fc);
        dc = eina_f32p32_double_to(fc);
        dresult = sin(dc);

        delta = fabs(dresult - eina_f32p32_double_to(fresult));
        fail_if(delta > 0.005);
     }


   eina_shutdown();
}
END_TEST

void
eina_test_fp(TCase *tc)
{
   tcase_add_test(tc, eina_fp_cos);
   tcase_add_test(tc, eina_fp_sin);
}
