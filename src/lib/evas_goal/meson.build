evas_lib = library('evas',
    'empty.c',
    #all the source code is coming from evas_pre_lib_dep
    dependencies: [evas_pre_lib_dep, evas_static_list],
    install: true,
    c_args : '-DPACKAGE_DATA_DIR="'+join_paths(dir_data, 'evas')+'"',
    version : meson.project_version()
)

evas_pub_deps = [eina, ecore, ector, emile, evas_pre]

evas = declare_dependency(
   link_with : [evas_lib],
   dependencies : evas_pub_deps,
   include_directories : evas_include_directories
)

evas_bin = declare_dependency(
   link_with : [evas_lib],
   dependencies : [eina, ecore, ector, emile, dependency('luajit')],
   include_directories : evas_include_directories
)

tmp_package_subdirs = []
automatic_pkgfile = false

foreach subdir : evas_header_subdirs
  tmp_package_subdirs += join_paths('evas-'+version_major, subdir)
endforeach

pkgconfig.generate(evas_lib,
  name : 'evas',
  subdirs : ['evas-'+version_major] + tmp_package_subdirs,
  version : version_major + '.' + version_minor,
  libraries : [eina, ecore, ector, emile, dependency('luajit')],
)
