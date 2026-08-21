efl_canvas_wl_deps = [ecore_wl2, ecore_input, ecore, ecore_evas, evas, emile, eina]
efl_canvas_wl_pub_deps = [eo, efl, evas]
efl_canvas_wl_ext_deps = [dependency('wayland-server', version : '>= 1.11.0'), dependency('xkbcommon', version : '>= 0.6.0'), wayland_protocol]

if (get_option('x11'))
  efl_canvas_wl_deps += [ecore_x]
  efl_canvas_wl_ext_deps += [dependency('xkbcommon-x11')]
endif

pub_eo_files = [
 'efl_canvas_wl.eo',
 'efl_canvas_wl_surface.eo',
]

foreach eo_file : pub_eo_files
  # Both generated files are declared, so ninja knows the .eo.c exists and puts
  # it back if it goes missing. Only the header goes into the target list: the
  # .eo.c is #included by a hand-written source, and meson would otherwise
  # compile it alone, where it cannot see the types it needs. Indexing keeps
  # the dependency on the whole target.
  _eo_ct = custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h', eo_file + '.c'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : [dir_package_include, false],
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'D:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchD', '@INPUT@'])
  pub_eo_file_target += _eo_ct[0]
endforeach

efl_canvas_wl_src = files([
 'dmabuf.c',
 'efl_canvas_wl.c',
])

efl_canvas_wl_header_src = ['Efl_Canvas_Wl.h']
eolian_include_directories += ['-I', meson.current_source_dir()]

efl_canvas_wl_lib = library('efl_canvas_wl',
    efl_canvas_wl_src, pub_eo_file_target,
    dependencies: efl_canvas_wl_deps + efl_canvas_wl_pub_deps + efl_canvas_wl_ext_deps,
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

efl_canvas_wl = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: efl_canvas_wl_lib,
  sources: pub_eo_file_target,
  dependencies: efl_canvas_wl_pub_deps,
)

install_headers(efl_canvas_wl_header_src,
  install_dir : dir_package_include,
)

eolian_efl_canvas_wl_dir = join_paths(eolian_include_dir, package_version_name)
if get_option('install-eo-files')
  install_data(pub_eo_files,
    install_dir: eolian_efl_canvas_wl_dir
  )
endif
