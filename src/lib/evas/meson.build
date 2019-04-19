# README
#
# Evas build is a bit more complex.
# the final shared library is build in src/lib/evas_goal/meson.build.
#
# The content of lib/evas is build as static_library as
# are all the engines loaders and savers.
#
# All those stages are defining dependencies with theire source code
# as 'source :', later everything is build as libevas.so.
#
# For now loaders and savers are ALWAYS build statically.
#

rel_evas_modules = join_paths('..', '..', '..', 'modules', 'evas', 'engines', 'software_generic')
evas_deps = [eo, eet, eina, efl, emile, ector, ecore, buildsystem, intl]
pub_eo_file_target = []
priv_eo_files = []
evas_include_directories = []
pub_eo_files = []
pub_evas_eo_files = []
pub_evas_eot_files = []

pub_legacy_eo_files = [

]
evas_src = []

subdir('software_generic')

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
endforeach

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file.underscorify(),
    input : eo_file,
    output : [eo_file.underscorify() + '.h'],
    depfile : eo_file.underscorify() + '.d',
    install : true,
    install_dir : dir_package_include,
    command : eolian_gen + [ '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach


pub_eo_types_files = [

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

eolian_include_directories += ['-I', meson.current_source_dir()]

evas_src += pub_eo_file_target

evas_header_src = [
  'Evas.h',
  'Evas_Common.h',
  'Evas_Eo.h',
  'Evas_GL.h',
  'Evas_Legacy.h',
  'Evas_Loader.h',
]

evas_include_directories += [
  include_directories('.'),
  include_directories('common'),
  include_directories('common3d'),
  include_directories(join_paths('common3d', 'save_load')),
  include_directories('include'),
  include_directories('filters'),
  vg_common_inc_dir
]

evas_src += [
   'main.c'
]

evas_src_opt = [ ]

evas_src += vg_common_src

evas_deps += dependency('freetype2')

if (get_option('fontconfig'))
   config_h.set('HAVE_FONTCONFIG', '1')
   evas_deps += dependency('fontconfig')
endif

if (get_option('fribidi'))
   config_h.set('HAVE_FRIBIDI', '1')
   evas_deps += dependency('fribidi')
endif

if (get_option('pixman'))
   pixman_support = ['HAVE_PIXMAN', 'PIXMAN_FONT', 'PIXMAN_RECT', 'PIXMAN_LINE', 'PIXMAN_POLY', 'PIXMAN_IMAGE', 'PIXMAN_IMAGE_SCALE_SAMPLE']
   foreach support : pixman_support
     config_h.set(support, '1')
   endforeach
   evas_deps += dependency('pixman-1')
endif

if (get_option('hyphen'))
   config_h.set('HAVE_HYPHEN', '1')
   hyphen = dependency('hyphen', required : false)
   if hyphen.found() == false
     evas_deps += cc.find_library('hyphen')
   endif
   evas_deps += hyphen
   config_h.set_quoted('EVAS_DICTS_HYPHEN_DIR', get_option('dictionaries-hyphen-dir'))
endif

subdir('include')
subdir('common')
subdir('canvas')
subdir('gesture')
subdir('filters')
subdir('cache')
subdir('common3d')
subdir('file')
subdir('vg')

gl_deps = []

if get_option('harfbuzz')
  evas_deps += dependency('harfbuzz')
  config_h.set('HAVE_HARFBUZZ', '1')
endif

if get_option('wl')
  evas_deps += wayland_protocol
endif


if get_option('opengl') != 'none'
  if get_option('opengl') == 'es-egl'
    config_h.set('GL_GLES', '1')
    gl_deps += dependency('glesv2')
    gl_deps += dependency('egl')
  else
    gl_deps += dependency('gl')
  endif
endif

evas_pre = declare_dependency(
  include_directories: evas_include_directories + [vg_common_inc_dir],
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: [eina, eo, ector, emile, evas_deps, m],
)

evas_link = [ ]

if cpu_sse3 == true or cpu_neon == true and cpu_neon_intrinsics == false
  evas_opt = static_library('evas_opt',
    sources: evas_src_opt,
    include_directories:
      [ include_directories('../../..') ] +
      evas_include_directories +
      [vg_common_inc_dir],
    c_args: native_arch_opt_c_args,
    dependencies: [eina, eo, ector, emile, evas_deps, m],
  )
  evas_link += [ evas_opt ]
endif

evas_pre_lib_dep = declare_dependency(
  include_directories: evas_include_directories + [vg_common_inc_dir],
  sources : [evas_src, pub_eo_file_target],
  link_with: evas_link,
  dependencies: [evas_deps, m, draw, valgrind, libunibreak]
)

if get_option('install-eo-files')
  install_data(pub_evas_eo_files + pub_evas_eot_files,
    install_dir: join_paths(eolian_include_dir, package_version_name)
  )
endif

install_headers(evas_header_src,
  install_dir : dir_package_include,
)

automatic_pkgfile = false
pub_eo_files = []
package_eo_subdirs += ['canvas', 'gesture']
package_header_subdirs += ['canvas', 'gesture']
