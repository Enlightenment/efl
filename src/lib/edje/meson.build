edje_deps = [
  eina, eo, efl, m,
  luajit, eet, evas, ecore_evas,
  ecore_file, ecore_input,
  ecore_imf, ecore_imf_evas,
  embryo, efreet, eio, intl
]


edje_pub_deps = [m, evas, eo, efl, luajit]

if (get_option('physics'))
  edje_deps += ephysics
  edje_pub_deps += ephysics
endif

config_h.set('EDJE_CALC_CACHE', '1')

if get_option('edje-sound-and-video')
  config_h.set('ENABLE_MULTISENSE', '1')
  edje_deps += ecore_audio
endif

pub_legacy_eo_files = [
  'edje_edit.eo',
  'efl_layout_calc.eo',
  'efl_layout_signal.eo',
  'efl_layout_group.eo',
  'efl_canvas_layout.eo',
  'efl_canvas_layout_part.eo',
  'efl_canvas_layout_part_box.eo',
  'efl_canvas_layout_part_external.eo',
  'efl_canvas_layout_part_invalid.eo',
  'efl_canvas_layout_part_swallow.eo',
  'efl_canvas_layout_part_table.eo',
  'efl_canvas_layout_part_text.eo',
]

pub_eo_files = pub_legacy_eo_files
pub_eo_file_target = []

foreach eo_file : pub_legacy_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
  pub_eo_file_target += custom_target('eolian_gen_legacy_' + eo_file,
    input : eo_file,
    output : [eo_file + '.legacy.h'],
    depfile : eo_file + '.legacy.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'l:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.h'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.d'),
                           '-gld', '@INPUT@'])
endforeach

pub_eo_types_files = [
  'edje_types.eot'
]

foreach eo_file : pub_eo_types_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-ghd', '@INPUT@'])
endforeach

priv_eo_files = [
  'edje_global.eo',
]

priv_eo_file_target = []
foreach eo_file : priv_eo_files
  priv_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach


eolian_include_directories += ['-I', meson.current_source_dir()]

edje_header_src = [
  'Edje.h',
  'Edje_Common.h',
  'Edje_Eo.h',
  'Edje_Legacy.h',
  'Edje_Edit.h'
]

edje_src = [
  'edje_private.h',
  'edje_convert.h',
  'edje_box_layout.c',
  'edje_cache.c',
  'edje_calc.c',
  'edje_callbacks.c',
  'edje_data.c',
  'edje_edit.c',
  'edje_embryo.c',
  'edje_entry.c',
  'edje_external.c',
  'edje_load.c',
  'edje_lua.c',
  'edje_lua2.c',
  'edje_lua_script_only.c',
  'edje_main.c',
  'edje_match.c',
  'edje_message_queue.c',
  'edje_misc.c',
  'edje_module.c',
  'edje_multisense.c',
  'edje_program.c',
  'edje_smart.c',
  'edje_text.c',
  'edje_textblock.c',
  'edje_textblock_styles.c',
  'edje_util.c',
  'edje_legacy.c',
  'edje_var.c',
  'edje_signal.c',
  'edje_part.c',
  'edje_part_helper.h',
  'edje_part_external.c',
  'edje_part_swallow.c',
  'edje_part_table.c',
  'edje_part_box.c',
  'edje_part_text.c',
  'edje_part_invalid.c'

]

edje_lib = library('edje',
    edje_src, pub_eo_file_target, priv_eo_file_target,
    dependencies: [edje_deps],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : [package_c_args],
    version : meson.project_version()
)

edje = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: edje_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: edje_pub_deps,
)

install_data(pub_eo_files + pub_eo_types_files,
  install_dir: join_paths(eolian_include_dir, package_version_name)
)

install_headers(edje_header_src,
  install_dir : dir_package_include,
)
