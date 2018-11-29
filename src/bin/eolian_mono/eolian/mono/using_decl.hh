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
