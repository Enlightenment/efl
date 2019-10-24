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
#ifndef EOLIAN_MONO_USING_DECL_HH
#define EOLIAN_MONO_USING_DECL_HH

#include <grammar/generator.hpp>
#include <grammar/context.hpp>
#include <grammar/kleene.hpp>
#include <grammar/integral.hpp>
#include <grammar/string.hpp>
#include <grammar/html_escaped_string.hpp>

namespace eolian_mono {

namespace grammar = efl::eolian::grammar;
using grammar::as_generator;
using grammar::string;
using grammar::html_escaped_string;
using grammar::operator<<;
using grammar::operator%;
using grammar::operator*;
using grammar::scope_tab;
using grammar::lower_case;
using grammar::upper_case;
using grammar::lit;
using grammar::qualifier_info;
using grammar::context_find_tag;
using grammar::attribute_conditional;

}

#endif
