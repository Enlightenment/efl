/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LIBEFL_MONO_NATIVE_TEST_H
#define LIBEFL_MONO_NATIVE_TEST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define EFL_PART_PROTECTED

#include <Ecore.h>
#include <Eo.h>

#ifdef _WIN32
# ifndef EFL_MONO_TEST_STATIC
#  ifdef EFL_MONO_TEST_BUILD
#   define EFL_MONO_TEST_API __declspec(dllexport)
#  else
#   define EFL_MONO_TEST_API __declspec(dllimport)
#  endif
# else
#  define EFL_MONO_TEST_API
# endif
# define EFL_MONO_TEST_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EFL_MONO_TEST_API __attribute__ ((visibility("default")))
#   define EFL_MONO_TEST_API_WEAK __attribute__ ((weak))
#  else
#   define EFL_MONO_TEST_API
#   define EFL_MONO_TEST_API_WEAK
#  endif
# else
#  define EFL_MONO_TEST_API
#  define EFL_MONO_TEST_API_WEAK
# endif
#endif

#include "dummy_test_iface.eo.h"
#include "dummy_inherit_iface.eo.h"
#include "dummy_numberwrapper.eo.h"
#include "dummy_test_object.eo.h"
#include "dummy_child.eo.h"
#include "dummy_inherit_helper.eo.h"
#include "dummy_part_holder.eo.h"
#include "dummy_event_manager.eo.h"
#include "dummy_constructible_object.eo.h"
#include "dummy_hidden_object.eo.h"

#include <interfaces/efl_part.eo.h>

#define EQUAL(a, b) ((a) == (b) ? 1 : (fprintf(stderr, "NOT EQUAL! %s:%i (%s)", __FILE__, __LINE__, __func__), fflush(stderr), 0))
#define STR_EQUAL(a, b) (strcmp((a), (b)) == 0 ? 1 : (fprintf(stderr, "NOT EQUAL! %s:%i (%s) '%s' != '%s'", __FILE__, __LINE__, __func__, (a), (b)), fflush(stderr), 0))

#endif
