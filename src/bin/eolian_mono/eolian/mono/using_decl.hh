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
using efl::eolian::grammar::as_generator;
using efl::eolian::grammar::string;
using efl::eolian::grammar::html_escaped_string;
using efl::eolian::grammar::operator<<;
using efl::eolian::grammar::operator%;
using efl::eolian::grammar::operator*;
using efl::eolian::grammar::scope_tab;
using efl::eolian::grammar::lower_case;
using efl::eolian::grammar::upper_case;
using efl::eolian::grammar::lit;
using efl::eolian::grammar::qualifier_info;
using efl::eolian::grammar::context_find_tag;

}

#endif
