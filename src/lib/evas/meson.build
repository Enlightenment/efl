# README
#
# All those stages are defining dependencies with theire source code
# as 'source :', later everything is build as libevas.so.
#

png = dependency('libpng')
tiff = dependency('libtiff-4', required: get_option('evas-loaders-disabler').contains('tiff') == false)
giflib = cc.find_library('gif')
webp = dependency('libwebp', version: ['>=0.5.0'], required: get_option('evas-loaders-disabler').contains('webp') == false)
webpdemux = dependency('libwebpdemux', version: ['>=0.5.0'], required: get_option('evas-loaders-disabler').contains('webp') == false)
libopenjp2 = dependency('libopenjp2', required: get_option('evas-loaders-disabler').contains('jp2k') == false)
libavif = dependency('libavif', required: get_option('evas-loaders-disabler').contains('avif') == false, version: '>= 0.8.2')
heif= dependency('libheif', required: get_option('evas-loaders-disabler').contains('heif') == false)

evas_image_loaders_file = [
     ['avif',    'shared', [libavif]],
     ['bmp',     'shared', []],
     ['eet',     'static', [eet]],
     ['generic', 'shared', [rt]],
     ['gif',     'shared', [giflib]],
     ['heif',    'shared', [heif]],
     ['ico',     'shared', []],
     ['jpeg',    'static', [jpeg]],
     ['jp2k',    'shared', [libopenjp2]],
     ['pmaps',   'shared', []],
     ['png',     'static', [png]],
     ['psd',     'shared', []],
     ['tga',     'shared', []],
     ['tgv',     'shared', [rg_etc, lz4]],
     ['tiff',    'shared', [tiff]],
     ['wbmp',    'shared', []],
     ['webp',    'shared', [webp, webpdemux]],
     ['xpm',     'shared', []]
]

evas_image_savers_file = [
     ['avif',    'shared', [libavif]],
     ['eet',     'static', [eet]],
     ['jpeg',    'static', [jpeg]],
     ['png',     'static', [png]],
     ['tgv',     'shared', [rg_etc, lz4]],
     ['tiff',    'shared', [tiff]],
     ['webp',    'shared', [webp]],
]
json = dependency('rlottie', required: get_option('evas-loaders-disabler').contains('json') == false)

evas_vg_loaders_file = [
  ['eet', [eet]],
  ['json', [json]],
  ['svg', []],
]

evas_vg_savers_file = ['eet', 'svg']

rel_evas_modules = join_paths('..', '..', '..', 'modules', 'evas', 'engines', 'software_generic')
evas_deps = [eo, eet, eina, efl, emile, ector, ecore, draw, buildsystem]
evas_ext_deps = [vg_common, draw_simple]
evas_ext_none_static_deps = [buildsystem_simple, intl, m, valgrind, libunibreak, lua]

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
                           '-e', 'EVAS_API',
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
                           '-e', 'EVAS_API',
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
                           '-e', 'EVAS_API',
                           '-ghd', '@INPUT@'])
endforeach

eolian_include_directories += ['-I', meson.current_source_dir()]

evas_src += pub_eo_file_target

evas_header_src = [
  'evas_api.h',
  'Evas.h',
  'Evas_Common.h',
  'Evas_Eo.h',
  'Evas_GL.h',
  'Evas_Legacy.h',
  'Evas_Loader.h',
  'Efl_Canvas.h',
]

evas_include_directories += [
  include_directories('.'),
  include_directories('common'),
  include_directories('include'),
  include_directories('filters'),
]

evas_src += files([
   'main.c'
])

evas_src_opt = [ ]

evas_ext_none_static_deps += dependency('freetype2')

if (get_option('fontconfig'))
   config_h.set('HAVE_FONTCONFIG', '1')
   evas_ext_none_static_deps += dependency('fontconfig')
endif

if (get_option('fribidi'))
   config_h.set('HAVE_FRIBIDI', '1')
   evas_ext_none_static_deps += dependency('fribidi')
endif

if (get_option('pixman'))
   pixman_support = ['HAVE_PIXMAN', 'PIXMAN_FONT', 'PIXMAN_RECT', 'PIXMAN_LINE', 'PIXMAN_POLY', 'PIXMAN_IMAGE', 'PIXMAN_IMAGE_SCALE_SAMPLE']
   foreach support : pixman_support
     config_h.set(support, '1')
   endforeach
   evas_ext_none_static_deps += dependency('pixman-1')
endif

if (get_option('hyphen'))
   config_h.set('HAVE_HYPHEN', '1')
   hyphen = dependency('hyphen', required : false)
   if hyphen.found() == false
     evas_ext_none_static_deps += cc.find_library('hyphen')
   endif
   evas_ext_none_static_deps += hyphen
   config_h.set_quoted('EVAS_DICTS_HYPHEN_DIR', get_option('dictionaries-hyphen-dir'))
endif

subdir('include')
subdir('common')
subdir('canvas')
subdir('gesture')
subdir('filters')
subdir('cache')
subdir('file')
subdir('vg')

gl_deps = []

if get_option('harfbuzz')
  evas_ext_none_static_deps += dependency('harfbuzz')
  config_h.set('HAVE_HARFBUZZ', '1')
endif

if get_option('wl')
  evas_ext_none_static_deps += wayland_protocol
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

evas_link = [ ]
evas_pub_deps = [eina, ecore, ector, emile]

if  cpu_sse3 == true or cpu_neon == true and cpu_neon_intrinsics == false
  evas_opt = static_library('evas_opt',
    sources: [evas_src_opt, pub_eo_file_target, priv_eo_file_target],
    include_directories:
      [ include_directories('../../..') ] +
      evas_include_directories +
      [vg_common_inc_dir],
    c_args: native_arch_opt_c_args,
    dependencies: [eina, eo, ector, emile, evas_deps, evas_ext_none_static_deps],
  )
  evas_link += [ evas_opt ]
endif

foreach loader_inst : evas_image_loaders_file
  loader = loader_inst[0]
  loader_type = loader_inst[1]
  loader_deps = loader_inst[2]
  if get_option('evas-loaders-disabler').contains(loader) == false
    if loader_type == 'static'
      config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
      file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'image_loaders', loader, 'evas_image_load_'+loader+'.c')
      config_h.set('EVAS_STATIC_BUILD_'+loader.to_upper(), '1')
      evas_static_list += [declare_dependency(
        sources: file,
        dependencies: loader_deps,
      )]
    endif
  else
    message('Image loader '+loader+' disabled')
  endif
endforeach

foreach loader_inst : evas_image_savers_file
  loader = loader_inst[0]
  loader_type = loader_inst[1]
  loader_deps = loader_inst[2]
  if get_option('evas-loaders-disabler').contains(loader) == false
    if loader_type == 'static'
      config_h.set('BUILD_LOADER_'+loader.to_upper(), '1')
      file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'image_savers', loader, 'evas_image_save_'+loader+'.c')
      config_h.set('EVAS_STATIC_BUILD_'+loader.to_upper(), '1')
      evas_static_list += [declare_dependency(
        sources: file,
        dependencies: loader_deps,
      )]
    endif
  else
    message('Image saver '+loader+' disabled')
  endif
endforeach

foreach loader_inst : evas_vg_loaders_file
  loader = loader_inst[0]
  loader_deps = loader_inst[1]

  if (get_option('evas-loaders-disabler').contains(loader) == false)
    file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'vg_loaders', loader, 'evas_vg_load_'+loader+'.c')
    evas_static_list += [declare_dependency(
      sources: file,
      dependencies: loader_deps,
    )]
    config_h.set('EVAS_STATIC_BUILD_VG_'+loader.to_upper(), '1')
  endif
endforeach

foreach loader : evas_vg_savers_file
  if (get_option('evas-loaders-disabler').contains(loader) == false)
    file = join_paths(meson.source_root(), 'src', 'modules', 'evas', 'vg_savers', loader, 'evas_vg_save_'+loader+'.c')
    evas_static_list += [declare_dependency(
      sources: file,
      dependencies: [eo, ector, emile, freetype, eet],
    )]
    config_h.set('EVAS_STATIC_BUILD_VG_'+loader.to_upper(), '1')
  endif
endforeach

evas_ext_deps += evas_static_list

evas_ext_deps += declare_dependency(
  link_with: evas_link,
)

evas_ext_deps += evas_ext_none_static_deps

evas_lib = library('evas',
    include_directories: evas_include_directories + [vg_common_inc_dir],
    sources : [evas_src, pub_eo_file_target, priv_eo_file_target],
    dependencies: [evas_deps, evas_ext_deps],
    link_with: evas_link,
    install: true,
    c_args : [package_c_args, '-DEVAS_BUILD'],
    version : meson.project_version()
)

evas = declare_dependency(
   link_with : [evas_lib],
   sources: pub_eo_file_target,
   dependencies : evas_pub_deps,
   include_directories: evas_include_directories + [vg_common_inc_dir] + [include_directories(join_paths('..', '..', 'modules', 'evas', 'engines', 'buffer'))],
)

evas_bin = declare_dependency(
   link_with : [evas_lib],
   dependencies : [eina, ecore, ector, emile],
   include_directories : evas_include_directories
)

if get_option('install-eo-files')
  install_data(pub_evas_eo_files + pub_evas_eot_files,
    install_dir: join_paths(eolian_include_dir, package_version_name)
  )
endif

install_headers(evas_header_src,
  install_dir : dir_package_include,
)

pub_eo_files = []
package_eo_subdirs += ['canvas', 'gesture']
package_header_subdirs += ['canvas', 'gesture']
#special case becase evas has a more structures subfolder
efl_one_include_dirs += evas_include_directories
