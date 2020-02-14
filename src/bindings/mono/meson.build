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
efl_mono_conf_data.set('EFLCUSTOMEXPORTSMONO', efl_mono_lib.full_path())

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
  styles = ['CA1000', 'CA1030', 'CA1031', 'CA1032', 'CA1034', 'CA1036', 'CA1040',
            'CA1043', 'CA1044', 'CA1051', 'CA1052', 'CA1062', 'CA1063', 'CA1064',
            'CA1065', 'CA1303', 'CA1305', 'CA1307', 'CA1401', 'CA1507', 'CA1707',
            'CA1710', 'CA1715', 'CA1716', 'CA1717', 'CA1720', 'CA1721', 'CA1724',
            'CA1801', 'CA1806', 'CA1810', 'CA1812', 'CA1815', 'CA1816', 'CA1822',
            'CA1825', 'CA1827', 'CA1829', 'CA2000', 'CA2007', 'CA2101', 'CA2200',
            'CA2207', 'CA2208', 'CA2214', 'CA2225', 'CA2227', 'SA0001', 'SA1000',
            'SA1001', 'SA1002', 'SA1003', 'SA1004', 'SA1005',
            'SA1006', 'SA1007', 'SA1008', 'SA1009', 'SA1010', 'SA1011', 'SA1012',
            'SA1013', 'SA1014', 'SA1015', 'SA1016', 'SA1017', 'SA1018', 'SA1019',
            'SA1020', 'SA1021', 'SA1022', 'SA1023', 'SA1024', 'SA1025', 'SA1026',
            'SA1027', 'SA1028', 'SA1100', 'SA1101', 'SA1102', 'SA1103', 'SA1104',
            'SA1105', 'SA1106', 'SA1107', 'SA1108', 'SA1109', 'SA1110', 'SA1111',
            'SA1112', 'SA1113', 'SA1114', 'SA1115', 'SA1116', 'SA1117', 'SA1118',
            'SA1119', 'SA1120', 'SA1121', 'SA1122', 'SA1123', 'SA1124', 'SA1125',
            'SA1126', 'SA1127', 'SA1128', 'SA1129', 'SA1130', 'SA1131', 'SA1132',
            'SA1133', 'SA1134', 'SA1135', 'SA1136', 'SA1137', 'SA1138', 'SA1139',
            'SA1200', 'SA1201', 'SA1202', 'SA1203', 'SA1204', 'SA1205', 'SA1206',
            'SA1207', 'SA1208', 'SA1209', 'SA1210', 'SA1211', 'SA1212', 'SA1213',
            'SA1214', 'SA1215', 'SA1216', 'SA1217', 'SA1300', 'SA1301', 'SA1302',
            'SA1303', 'SA1304', 'SA1305', 'SA1306', 'SA1307', 'SA1308', 'SA1309',
            'SA1310', 'SA1311', 'SA1312', 'SA1313', 'SA1314', 'SA1400', 'SA1401',
            'SA1402', 'SA1403', 'SA1404', 'SA1405', 'SA1406', 'SA1407', 'SA1408',
            'SA1409', 'SA1410', 'SA1411', 'SA1412', 'SA1413', 'SA1500', 'SA1501',
            'SA1502', 'SA1503', 'SA1504', 'SA1505', 'SA1506', 'SA1507', 'SA1508',
            'SA1509', 'SA1510', 'SA1511', 'SA1512', 'SA1513', 'SA1514', 'SA1515',
            'SA1516', 'SA1517', 'SA1518', 'SA1519', 'SA1520', 'SA1600', 'SA1601',
            'SA1602', 'SA1603', 'SA1604', 'SA1605', 'SA1606', 'SA1607', 'SA1608',
            'SA1609', 'SA1610', 'SA1611', 'SA1612', 'SA1613', 'SA1614', 'SA1615',
            'SA1616', 'SA1617', 'SA1618', 'SA1619', 'SA1620', 'SA1621', 'SA1622',
            'SA1623', 'SA1624', 'SA1625', 'SA1626', 'SA1627', 'SA1628', 'SA1629',
            'SA1630', 'SA1631', 'SA1632', 'SA1633', 'SA1634', 'SA1635', 'SA1636',
            'SA1637', 'SA1638', 'SA1639', 'SA1640', 'SA1641', 'SA1642', 'SA1643',
            'SA1644', 'SA1645', 'SA1646', 'SA1647', 'SA1648', 'SA1649', 'SA1650',
            'SA1651']

  efl_stylecop_data = configuration_data()
  watched_styles = []
  severity_styles = get_option('dotnet-stylecop-severity')
  if get_option('dotnet-stylecop') == '*'
    watched_styles = styles
  elif get_option('dotnet-stylecop') != ''
    s = get_option('dotnet-stylecop').to_upper().split(',')
    foreach cc : s
      if cc.startswith('CA') or cc.startswith('SA')
        watched_styles += cc
      endif
    endforeach
  endif

  foreach s : styles
    if s in watched_styles
      efl_stylecop_data.set('@0@'.format(s), '<Rule Id="@0@" Action="@1@"/>'.format(s, severity_styles))
    else
      efl_stylecop_data.set('@0@'.format(s), '<Rule Id="@0@" Action="None"/>'.format(s))
    endif
  endforeach

  ca_ruleset = configure_file(input : 'ca.ruleset.in',
                              output : 'ca.ruleset',
                              configuration : efl_stylecop_data)

  dotnet = find_program('dotnet')

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
  lib_csproj_conf_data.set('CA_RULESET', join_paths(meson.build_root(), '@0@'.format(ca_ruleset)))
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
    input: mono_generator_target + mono_files + [efl_src] + [lib_csproj] + [ca_ruleset],
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
  extra_cs_args += '-d:MONO'

  efl_mono = library('efl_mono',
      mono_generator_target + mono_files + [efl_src],
      install : true,
      install_dir : efl_mono_install_dir,
      cs_args : extra_cs_args + ['-doc:' + efl_mono_xml_doc, '-warnaserror+'],
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
