# dotnet supports cs files indirectly through custom_targets
if not get_option('dotnet')
  add_languages('cs')
endif


runtime_assemblies = []

mono_sublibs = [
  ['Eina',        true, ], #
  ['Eolian',      true, ], #
  ['Eo',          true, ], #
  ['Ecore',      false, ], #
  ['Efl',         true, ], #
  ['Evas',       false, ], #
  ['Edje',       false, ], #
  ['Eldbus',      true, ], #
  ['Elementary', false, ]  #
]

blacklisted_files = [
  'efl_class.eo',
  'efl_canvas_scene3d.eo',
  'evas_canvas3d_camera.eo',
  'evas_canvas3d_light.eo',
  'evas_canvas3d_material.eo',
  'evas_canvas3d_mesh.eo',
  'evas_canvas3d_node.eo',
  'evas_canvas3d_object.eo',
  'evas_canvas3d_primitive.eo',
  'evas_canvas3d_scene.eo',
  'evas_canvas3d_texture.eo',
  'efl_io_buffer.eo',
  'efl_io_queue.eo',
  'efl_io_sizer.eo',
  'efl_io_closer_fd.eo',
  'efl_io_buffered_stream.eo',
  'efl_io_positioner_fd.eo',
  'efl_io_reader_fd.eo',
  'efl_io_writer_fd.eo',
  'efl_io_copier_fd.eo',
  'efl_io_sizer_fd.eo',
  'efl_io_stdin.eo',
  'efl_io_stdout.eo',
  'efl_io_stderr.eo',
  'efl_io_file.eo',
  'efl_io_copier.eo',
  'efl_object_override.eo',
  'elm_web.eo',
  'elm_map.eo',
  'elm_list.eo',
  'elm_genlist.eo',
  'elm_view_list.eo',
  'elm_genlist_item.eo',
  'elm_gengrid.eo',
  'elm_glview_eo.cs',
  'elm_code_widget.eo',
  'elm_multibuttonentry_part.eo',
  'elm_atspi_bridge.eo',
  'elm_atspi_app_object.eo',
  'elm_interface_scrollable.eo',
  'evas_canvas3d_types.eot',
  'elm_general.eot',
  'efl_thread.eo',
  'efl_threadio.eo',
  'efl_appthread.eo'
]

manual_inheritance_files = [
  'efl_object.eo'
]

beta_option = []
if (get_option('mono-beta'))
   beta_option = '-b'
endif

mono_generator_target = []
mono_files = []
foreach lib : mono_sublibs
  package_name = lib[0].to_lower()
  eo_file_subdirs = get_variable(package_name + '_eo_subdirs')
  file_location = join_paths('..', '..', 'lib', package_name)

  if (package_name != 'eldbus')
    foreach eo_file_subdir : eo_file_subdirs
      if eo_file_subdir != ''
        mono_pub_eo_files = get_variable(package_name + '_' + eo_file_subdir +'_eo_files') + get_variable(package_name + '_' + eo_file_subdir + '_eot_files')
      else
        mono_pub_eo_files = get_variable(package_name +'_eo_files') + get_variable(package_name + '_eot_files')
      endif

      subdir_file_location = join_paths(file_location, eo_file_subdir)
      foreach mono_gen_file : mono_pub_eo_files
        if not blacklisted_files.contains(mono_gen_file)
          partial = []
          if manual_inheritance_files.contains(mono_gen_file)
            partial = '-p'
          endif
          mono_generator_target += custom_target('eolian_mono_gen_'+mono_gen_file.underscorify()+'',
            input : join_paths(subdir_file_location, mono_gen_file),
            output : [mono_gen_file + '.cs'],
            command : [eolian_mono_gen, beta_option, '-I', meson.current_source_dir(), eolian_include_directories,
                                       '--dllimport', package_name,
                                       '-o', join_paths(meson.current_build_dir(), mono_gen_file + '.cs'),
                                       '-e', get_option('mono-examples-dir'), partial,
                                       '@INPUT@'])
        endif
      endforeach
    endforeach
  endif
  if lib[1]
    subdir(package_name + '_mono')
  endif
endforeach

efl_mono_conf_data = configuration_data()
efl_mono_conf_data.set('EINA', eina_lib.full_path())
efl_mono_conf_data.set('EFL', efl_lib.full_path())
efl_mono_conf_data.set('ECORE', ecore_lib.full_path())
efl_mono_conf_data.set('EO', eo_lib.full_path())
efl_mono_conf_data.set('EVAS', evas_lib.full_path())
efl_mono_conf_data.set('ELDBUS', eldbus_lib.full_path())
efl_mono_conf_data.set('ELEMENTARY', elementary_lib.full_path())

efl_mono_dll_config = configure_file(input : 'efl_mono.dll.config.in',
                                 output : 'efl_mono.dll.config',
                                 configuration : efl_mono_conf_data)

extra_cs_args = runtime_assemblies

if get_option('mono-beta')
  extra_cs_args += '-d:EFL_BETA'
endif

efl_mono_install_dir = join_paths(dir_lib, 'efl-mono-'+version_major)
efl_mono_xml_doc = join_paths(meson.current_build_dir(), 'efl_mono.xml')

if (get_option('dotnet'))
  dotnet = find_program('dotnet')
  warning('Dotnet support is still experimental.')

  lib_csproj_conf_data = configuration_data()

  dotnet_version = run_command(dotnet, ['--version'], check: true)
  message('Found dotnet core version ' + dotnet_version.stdout().strip())

  # The major version should be enough for now while we don't have to deal
  # with minor differences and dotnet core is far from version 10
  dotnet_major_version = dotnet_version.stdout().strip()[0].to_int()
  if dotnet_major_version == 3
    dotnet_core_app_version = '3.0'
    dotnet_standard_version = '2.1'
  elif dotnet_major_version == 2
    dotnet_core_app_version = '2.0'
    dotnet_standard_version = '2.0'
  else
    error('Unsupported dotnet version. Must be at least 2.2')
  endif

  lib_csproj_conf_data.set('NETSTANDARD_VERSION', dotnet_standard_version)
  lib_csproj_conf_data.set('BINDING_SRC', meson.current_source_dir())
  lib_csproj_conf_data.set('EFL_VERSION', meson.project_version())

  if get_option('mono-beta')
    lib_csproj_conf_data.set('EFL_BETA', 'EFL_BETA')
  else
    lib_csproj_conf_data.set('EFL_BETA', '')
  endif

  lib_csproj = configure_file(input: 'efl_sharp.csproj.in',
                              output: 'efl_sharp.csproj',
                              configuration: lib_csproj_conf_data)

  efl_mono = custom_target('efl_mono',
    input: mono_generator_target + mono_files + [efl_src] + [lib_csproj] + ['ca.ruleset'],
    output: 'efl_sharp.dll',
    build_by_default: true,
    command: [dotnet,
             'build',
             '-o', meson.current_build_dir(),
             '-p:BuildingLib=Yes',
             lib_csproj],
    install: true,
    install_dir: efl_mono_install_dir,
  )

  efl_mono_pack = custom_target('efl_mono_nuget',
    input: lib_csproj,
    output: 'Efl.Csharp.' + meson.project_version() + '.nupkg',
    depends: [efl_mono],
    command: [dotnet,
             'pack',
             '-o', meson.current_build_dir(),
             '-p:BuildingLib=No',
             lib_csproj],
    install: true,
    install_dir: efl_mono_install_dir,
  )

else

  efl_mono = library('efl_mono',
      mono_generator_target + mono_files + [efl_src],
      install : true,
      install_dir : efl_mono_install_dir,
      cs_args : extra_cs_args + ['-doc:' + efl_mono_xml_doc, '-warnaserror+']
  )

  meson.add_install_script(join_paths(meson.source_root(), 'meson', 'meson_csharp_docs.sh'),
                           efl_mono_xml_doc,
                           efl_mono_install_dir)
endif

efl_mono_test_suite_path=join_paths(meson.current_build_dir())

pkgconfig.generate(
  name : 'efl-mono',
  description : 'Efl C# bindings',
  version : version_major + '.' + version_minor + '.' + version_micro,
  libraries : ['-r:${assemblies_dir}/efl_mono.dll'],
  variables : ['assemblies_dir='+join_paths(dir_lib, 'efl-mono-'+version_major),
               'mono_libs=-r:${assemblies_dir}/efl_mono.dll']
)
test_dirs += 'efl_mono'
