efl_eins_lib = library('efl-eins',
    overall_efl_source,
    include_directories : config_dir,
    dependencies : overall_efl_deps,
    install : true
)

selected_overall_efl_deps = []

foreach package : subprojects
  package_name = package[0]
  if package[1].length() == 0 or get_option(package[1][0])
    if (package[3])
      selected_overall_efl_deps += get_variable(package_name + '_deps')
    endif
  endif
endforeach

efl_eins = declare_dependency(
  include_directories: overall_efl_include_directories + config_dir,
  dependencies : selected_overall_efl_deps,
  link_with : efl_eins_lib,
  version: version_major + '.' + version_minor + '.' + version_micro,
)

include_subdirs = [join_paths('eina-' + version_major, 'eina')]

foreach package : subprojects
  package_name = package[0]
  if (package[3])
    include_subdirs += [package_name + '-' + version_major]
  endif
  set_variable(package_name, efl_eins)
endforeach

pkgconfig.generate(efl_eins_lib,
  name : 'efl-eins',
  subdirs : include_subdirs,
  version : version_major + '.' + version_minor + '.' + version_micro,
  libraries : overall_efl_pub_deps,
)
