#ifndef EOLIAN_MONO_NAME_HELPERS_HH
#define EOLIAN_MONO_NAME_HELPERS_HH

#include <algorithm>
#include <cctype>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace eolian_mono {

namespace name_helpers {

static const std::vector<std::string> verbs =
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
    "emit",
    "call",
    "append"
  };

const std::vector<std::string> not_verbs =
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

void reorder_verb(std::vector<std::string> &names)
{
  if (names.size() <= 1)
    return;

  std::string verb = names.back();

  if (std::find(verbs.begin(), verbs.end(), verb) != verbs.end())
    {
       names.pop_back();
       names.insert(names.begin(), verb);
    }
}

std::vector<std::string> split(std::string const &input, char delim)
{
  std::stringstream ss(input);
  std::string name;
  std::vector<std::string> names;

  while (std::getline(ss, name, delim)) {
    if (!name.empty())
      names.push_back(name);
  }
  return names;
}

std::string pascal_case(const std::vector<std::string> &names)
{
    std::vector<std::string> outv(names.size());
    std::stringstream osstream;

    std::transform(names.begin(), names.end(), outv.begin(),
          [](std::string name) { 
            name[0] = std::toupper(name[0]);
            return name;
          });

    std::copy(outv.begin(), outv.end(), std::ostream_iterator<std::string>(osstream, ""));

    return osstream.str();
}

} // namespace name_helpers

} // namespace eolian_mono

#endif
