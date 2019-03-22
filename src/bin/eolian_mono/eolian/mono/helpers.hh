#ifndef EOLIAN_MONO_HELPERS_HH
#define EOLIAN_MONO_HELPERS_HH

#include "grammar/klass_def.hpp"
#include "blacklist.hh"
#include "name_helpers.hh"

namespace eolian_mono {

namespace helpers {

/* General helpers, not related directly with generating strings (those go in the name_helpers.hh). */

namespace attributes = efl::eolian::grammar::attributes;

inline bool need_struct_conversion(attributes::regular_type_def const* regular)
{
   return regular && regular->is_struct() && !blacklist::is_struct_blacklisted(*regular);
}

inline bool need_struct_conversion(attributes::parameter_def const& param, attributes::regular_type_def const* regular)
{
   if (param.direction == attributes::parameter_direction::in && param.type.has_own)
     return false;

   return need_struct_conversion(regular);
}

inline bool need_struct_conversion_in_return(attributes::type_def const& ret_type, attributes::parameter_direction const& direction)
{
   auto regular = efl::eina::get<attributes::regular_type_def>(&ret_type.original_type);

   if (!regular->is_struct())
     return false;

   if (regular->is_struct() && (direction == attributes::parameter_direction::out || direction == attributes::parameter_direction::unknown))
     return false;

   if (ret_type.has_own)
     return false;

   return true;
}

inline bool need_pointer_conversion(attributes::regular_type_def const* regular)
{
   if (!regular)
     return false;

   if (regular->is_enum()
       || (regular->is_struct() && name_helpers::type_full_eolian_name(*regular) != "Eina.Binbuf")
      )
     return true;

   std::set<std::string> const types {
     "bool", "char"
     , "byte" , "short" , "int" , "long" , "llong" , "int8" , "int16" , "int32" , "int64" , "ssize"
     , "ubyte", "ushort", "uint", "ulong", "ullong", "uint8", "uint16", "uint32", "uint64", "size"
     , "ptrdiff"
     , "float", "double"
   };
   if (types.find(regular->base_type) != types.end())
     return true;

   return false;
}

// While klass_def has immediate_inherits, we need a way to get all interfaces inherited by an interface
// either directly or through another interface.
std::set<attributes::klass_name, attributes::compare_klass_name_by_name> interface_inherits(attributes::klass_def const& cls)
{
   std::set<attributes::klass_name, attributes::compare_klass_name_by_name> inherits;

   std::function<void(attributes::klass_name const&)> inherit_algo =
       [&] (attributes::klass_name const& klass)
       {
          // TODO we could somehow cache klass_def instantiations
          attributes::klass_def c(get_klass(klass, cls.unit), cls.unit);
          for(auto&& inherit : c.immediate_inherits)
            {
               switch(inherit.type)
                 {
                 case attributes::class_type::mixin:
                 case attributes::class_type::interface_:
                   inherits.insert(inherit);
                   inherit_algo(inherit);
                   break;
                 case attributes::class_type::regular:
                 case attributes::class_type::abstract_:
                   inherit_algo(inherit);
                 default:
                   break;
                 }
            }
       };

   inherit_algo(get_klass_name(cls));


   return inherits;
}

// Returns the set of interfaces implemented by this type that haven't been implemented
// by a regular parent class.
template<typename Context>
std::set<attributes::klass_name, attributes::compare_klass_name_by_name> non_implemented_interfaces(attributes::klass_def const& cls, Context const& context)
{
   auto options = efl::eolian::grammar::context_find_tag<options_context>(context);
   std::set<attributes::klass_name, attributes::compare_klass_name_by_name> implemented_interfaces;
   std::set<attributes::klass_name, attributes::compare_klass_name_by_name> interfaces;

   std::function<void(attributes::klass_name const&, bool)> inherit_algo =
       [&] (attributes::klass_name const& klass, bool is_implemented)
       {
          // TODO we could somehow cache klass_def instantiations
          attributes::klass_def c(get_klass(klass, cls.unit), cls.unit);
          for(auto&& inherit : c.immediate_inherits)
            {
               if (inherit.is_beta && !options.want_beta)
                   continue;

               switch(inherit.type)
                 {
                 case attributes::class_type::mixin:
                 case attributes::class_type::interface_:
                   interfaces.insert(inherit);
                   if (is_implemented)
                     implemented_interfaces.insert(inherit);
                   inherit_algo(inherit, is_implemented);
                   break;
                 case attributes::class_type::abstract_:
                 case attributes::class_type::regular:
                   inherit_algo(inherit, true);
                 default:
                   break;
                 }
            }
       };

   inherit_algo(get_klass_name(cls), false);

   for (auto&& inherit : implemented_interfaces)
     interfaces.erase(inherit);


   return interfaces;
}


/*
 * Determines whether this class has any regular ancestor or not
 */
bool has_regular_ancestor(attributes::klass_def const& cls)
{
   auto inherits = cls.inherits;
   std::function<bool(attributes::klass_name const&)> is_regular =
       [&] (attributes::klass_name const& klass)
       {
          return klass.type == attributes::class_type::regular || klass.type == attributes::class_type::abstract_;
       };

   return std::any_of(inherits.begin(), inherits.end(), is_regular);
}

/*
 * Gets all methods that this class should implement (i.e. that come from an unimplemented interface/mixin and the class itself)
 */
std::vector<attributes::function_def> get_all_implementable_methods(attributes::klass_def const& cls)
{
   std::vector<attributes::function_def> ret;

   std::copy(cls.functions.begin(), cls.functions.end(), std::back_inserter(ret));

   // Non implemented interfaces
   std::set<attributes::klass_name, attributes::compare_klass_name_by_name> implemented_interfaces;
   std::set<attributes::klass_name, attributes::compare_klass_name_by_name> interfaces;
   std::function<void(attributes::klass_name const&, bool)> inherit_algo =
       [&] (attributes::klass_name const &klass, bool is_implemented)
       {
           attributes::klass_def c(get_klass(klass, cls.unit), cls.unit);
           for (auto&& inherit: c.immediate_inherits)
             {
                switch(inherit.type)
                  {
                  case attributes::class_type::mixin:
                  case attributes::class_type::interface_:
                    interfaces.insert(inherit);
                    if (is_implemented)
                      implemented_interfaces.insert(inherit);
                    inherit_algo(inherit, is_implemented);
                    break;
                  case attributes::class_type::abstract_:
                  case attributes::class_type::regular:
                    inherit_algo(inherit, true);
                  default:
                    break;
                  }
             }
       };

   inherit_algo(attributes::get_klass_name(cls), false);

   for (auto&& inherit : implemented_interfaces)
     interfaces.erase(inherit);

    for (auto&& inherit : interfaces)
    {
        attributes::klass_def klass(get_klass(inherit, cls.unit), cls.unit);
        std::copy(klass.functions.cbegin(), klass.functions.cend(), std::back_inserter(ret));
    }

  return ret;
}

/*
 * Checks whether the given is unique going up the inheritance tree from leaf_klass
 */
inline bool is_unique_event(attributes::event_def const& evt
                         , attributes::klass_def const& leaf_klass)
{
  auto events = leaf_klass.get_all_events();
  int i = 1;
  return !std::any_of(events.cbegin(), events.cend(),
                         [&evt, &i](const attributes::event_def &other) {
                            return evt.name == other.name && i++ == 2;
                     });
}

inline std::vector<attributes::constructor_def> reorder_constructors(std::vector<attributes::constructor_def> constructors)
{
  auto is_required = [](attributes::constructor_def const& ctr) { return !ctr.is_optional; };
  std::stable_partition(constructors.begin(), constructors.end(), is_required);
  return constructors;
}

} // namespace helpers

} // namespace eolian_mono

#endif
