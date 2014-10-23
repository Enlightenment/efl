#ifndef EOLIAN_KLASS_HH
#define EOLIAN_KLASS_HH

inline std::string name(Eolian_Class const& cls)
{
  return ::eolian_class_name_get(&cls);
}

#endif
