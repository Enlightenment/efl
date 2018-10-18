evas_lib = library('evas',
    'empty.c',
    #all the source code is coming from evas_pre_lib_dep
    dependencies: [evas_pre_lib_dep, evas_static_list],
    install: true,
    c_args : '-DPACKAGE_DATA_DIR="'+join_paths(dir_data, 'evas')+'"',
    version : meson.project_version()
)

evas = declare_dependency(
   link_with : [evas_lib],
   dependencies : [eina, ecore, ector, emile, evas_pre],
   include_directories : evas_include_directories
)

evas_bin = declare_dependency(
   link_with : [evas_lib],
   dependencies : [eina, ecore, ector, emile, dependency('luajit')],
   include_directories : evas_include_directories
)

automatic_pkgfile = false

pkgconfig.generate(evas_lib,
  name : 'evas',
  subdirs : 'evas-'+version_major,
  version : version_major + '.' + version_minor,
  libraries : [eina, ecore, ector, emile, dependency('luajit')],
)
