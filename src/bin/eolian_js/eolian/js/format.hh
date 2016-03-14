#ifndef EOLIAN_JS_FORMAT_HH
#define EOLIAN_JS_FORMAT_HH

#include <eolian/js/domain.hh>

#include <algorithm>
#include <string>
#include <cctype>

namespace eolian { namespace js {

namespace format {

const char* verbs[] =
  {
    "add",
    "get",
    "is",
    "del",
    "thaw",
    "freeze",
    "save",
    "wait",
    "eject",
    "raise",
    "lower",
    "load",
    "dup",
    "reset",
    "unload",
    "close",
    "set",
    "interpolate",
    "has",
    "grab",
    "check",
    "find",
    "ungrab",
    "unset",
    "clear",
    "pop",
    "new",
    "peek",
    "push",
    "update",
    "show",
    "move",
    "hide",
    "calculate",
    "resize",
    "attach",
    "pack",
    "unpack",
    "emit"
  };

const char* not_verbs[] =
  {
    "below",
    "above",
    "name",
    "unfreezable",
    "value",
    "r",
    "g",
    "b",
    "a",
    "finalize",
    "destructor",
    "to",
    "circle",
    "rect",
    "path",
    "commands",
    "type",
    "colorspace"
    "op",
    "type",
    "properties",
    "status",
    "status",
    "relative",
    "ptr",
    "pair",
    "pos",
    "end"
  };
  
std::string format_method(std::string const& in)
{
   std::string r;
   std::string::const_iterator current = in.begin(), last = in.end();
   do
     {
       std::string::const_iterator word_end = std::find(current, last, '_');
       if(word_end == last)
         {
           bool found_verb = false, found_not_verb = false;
           std::string v(current, word_end);
           for(const char** verb = &format::verbs[0]; verb != &format::verbs
                 [sizeof(format::verbs)/sizeof(format::verbs[0])]; ++verb)
             {
               if(!std::lexicographical_compare
                  (current, word_end, *verb, *verb + std::strlen(*verb))
                  && !std::lexicographical_compare
                  (*verb, *verb + std::strlen(*verb), current, word_end))
                 {
                   found_verb = true;
                 }
             }
           if(!found_verb)
           {
             for(const char** not_verb = &format::not_verbs[0]; not_verb != &format::not_verbs
                   [sizeof(format::not_verbs)/sizeof(format::not_verbs[0])]; ++not_verb)
               {
                 if(!std::lexicographical_compare
                    (current, word_end, *not_verb, *not_verb + std::strlen(*not_verb))
                    && !std::lexicographical_compare
                    (*not_verb, *not_verb + std::strlen(*not_verb), current, word_end))
                   {
                     found_not_verb = true;
                   }
               }
             if(!found_not_verb)
               EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
                 << "Last word is NOT a not-verb " << v << std::endl;
           }
           if(found_verb || !found_not_verb)
               r = v + r;
           else
             {
               v[0] = std::toupper(v[0]);
               r += v;
               r[0] = std::tolower(r[0]);
             }
           current = last;
         }
       else
         {
           r += std::toupper(*current++);
           std::copy(current, word_end, std::back_inserter(r));
           current = word_end + 1;
         }
     }
   while(current != last);

   EINA_CXX_DOM_LOG_DBG(eolian::js::domain)
     << "Formatted method " << r << " with input " << in << std::endl;
   
   return r;
}

std::string format_field(std::string const& in)
{
  std::string r;
  bool up = false;
  for (char c : in)
    {
       if (c == '_')
         up = true;
       else if (!up)
         r += c;
       else
         {
            r += std::toupper(c);
            up = false;
         }
    }
  return r;
}

std::string format_class(std::string const& in)
{
   std::string r;
   std::string::const_iterator current = in.begin(), last = in.end();
   std::copy_if(current, last, std::back_insert_iterator<std::string>(r),
                [] (char c)
                {
                   return c != '_';
                });
   return r;
}

std::string format_namespace(std::string const& in)
{
  return format_class(in);
}

std::string format_struct(std::string const& in)
{
  return format_class(in);
}

std::string format_enum(std::string const& in)
{
  return format_class(in);
}

std::string constant(std::string in)
{
   std::transform(in.begin(), in.end(), in.begin(), ::toupper);
   return in;
}

}

} }

#endif
