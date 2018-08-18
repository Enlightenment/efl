
# ['eina', 'eolian', 'eo'] those three are build in the root meson.build reason for this is the super special handling
cxx_sublibs = [
  ['Eina',        true,  true, false, [eo]],
  ['Eolian',      true,  true, false, []],
  ['Eo',          true,  true, false, []],
  ['Ecore',       true,  true,  true, []],
  ['Eet',         true,  true, false, []],
  ['Efl',         true, false,  true, []],
  ['Eio',        false, false,  true, []],
  ['Evas',       false,  true,  true, []],
  ['Edje',       false, false,  true, []],
  ['Eldbus',      true,  true,  true, []],
  ['Elementary', false,  true,  true, []]
]

increased_dependency = []

foreach lib : cxx_sublibs
  package_name = lib[0].to_lower()
  package_version_name = '-'.join(package_name.split('_')) + '-cxx-' + version_major
  package_c_args = [
    '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, package_name)+'"',
    '-DNEED_RUN_IN_TREE=1'
  ]

  dir_package_include = join_paths(dir_include, package_version_name)
  dir_package_modules = join_paths(dir_lib, package_name, 'modules')
  cxx_generator_target = []
  cxx_header_src = []
  file_location = join_paths('..', '..', 'lib', package_name)
  eo_file_list = []
  eo_file_subdirs = get_variable(package_name + '_eo_subdirs', [''])

  #generate files for each .eo files
  foreach eo_file_subdir : eo_file_subdirs
    if eo_file_subdir != ''
      cxx_pub_eo_files = get_variable(package_name + '_' + eo_file_subdir +'_eo_files')
    else
      cxx_pub_eo_files = get_variable(package_name +'_eo_files')
    endif
    subdir_file_location = join_paths(file_location, eo_file_subdir)
    foreach cxx_gen_file : cxx_pub_eo_files
      cxx_generator_target += custom_target('eolian_cxx_gen_'+cxx_gen_file.underscorify()+'',
        input : join_paths(subdir_file_location, cxx_gen_file),
        output : [cxx_gen_file + '.hh', cxx_gen_file + '.impl.hh'],
        install : true,
        install_dir : join_paths(dir_package_include, eo_file_subdir),
        command : [eolian_cxx_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                                   '-o', join_paths(meson.current_build_dir(), cxx_gen_file + '.hh'),
                                   '@INPUT@'])
      eo_file_list += files(join_paths(subdir_file_location, cxx_gen_file))
    endforeach
  endforeach

  #generate the package.eo.hh file
  if lib[3]
    cxx_generator_target += custom_target('eolian_cxx_gen_'+package_name+'.eo.hh',
          input : eo_file_list,
          output : [lib[0] + '.eo.hh'],
          install : true,
          install_dir : join_paths(dir_package_include),
          command : [eolian_cxx_gen, '-I', meson.current_source_dir(), eolian_include_directories, '-m',
                                     '-o', '@OUTPUT@',
                                     '@INPUT@'])
  endif

  if lib[1]
    inc_dir = include_directories(package_name+'_cxx', '.')
    subdir(package_name+'_cxx')
  else
    inc_dir = include_directories(file_location, '.')
    cxx_header_src += files(join_paths(file_location, lib[0]+'.hh'))
  endif

  dep = declare_dependency(
    include_directories: inc_dir,
    sources : cxx_generator_target,
    dependencies: [get_variable(package_name)] + get_variable(package_name + '_pub_deps') + lib[4] + increased_dependency
  )

  increased_dependency += dep

  set_variable(package_name + '_cxx', dep)

  install_headers(cxx_header_src,
    subdir: dir_package_include
  )
  if lib[2]
    test_dirs += package_name + '_cxx'
  endif
  pkgconfig.generate(
    name : '-'.join(package_name.split('_')) + '-cxx',
    description : lib[0]+' cxx bindings',
    subdirs : package_version_name,
    version : version_major + '.' + version_minor,
    libraries : dep,
    #for now - just drag the whole elementary in
    requires : 'elementary',
  )
endforeach
