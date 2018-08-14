project('efl', ['c','cpp'],
  version: '1.20.99',
  default_options : ['buildtype=plain'],
  meson_version : '>=0.46'
)

if target_machine.system() == 'darwin'
  add_languages('objc')
endif


clean_check = run_command('meson/clean_check.sh')
if clean_check.returncode() == 0
  error('Meson build requires a clean source tree')
endif

pkgconfig = import('pkgconfig')

test_env = environment()
test_env.set('EFL_RUN_IN_TREE', '1')

version_arr = meson.project_version().split('.')

version_major = version_arr[0]
version_minor = version_arr[1]
version_micro = version_arr[2]

cc = meson.get_compiler('c')
host_os = host_machine.system()

if host_os == 'linux'
  if cc.has_header_symbol('features.h', '__UCLIBC__')
    host_os = 'linux-uclibc'
  elif cc.has_header_symbol('features.h', '__dietlibc__')
    host_os = 'linux-dietlibc'
  else
    host_os = 'linux-gnu'
  endif
endif

#prepare a special linker args flag for binaries on macos
bin_linker_args = []
if target_machine.system() == 'darwin'
  bin_linker_args = ['-pagezero_size', '10000', '-image_base', '100000000']
endif

module_arch = '@0@-@1@-@2@'.format(host_os, host_machine.cpu_family(), meson.project_version())
module_files = []
evas_loader_map = []

#install paths
dir_prefix    = get_option('prefix')
dir_sysconf   = join_paths(dir_prefix, get_option('sysconfdir'))
dir_bin       = join_paths(dir_prefix, get_option('bindir'))
dir_data      = join_paths(dir_prefix, get_option('datadir'))
dir_include   = join_paths(dir_prefix, get_option('includedir'))
dir_lib       = join_paths(dir_prefix, get_option('libdir'))

#local paths
local_lib = join_paths('src', 'lib')
local_bin = join_paths('src', 'bin')
local_module = join_paths('src', 'modules')
local_tests = join_paths('src', 'tests')
local_benchmark = join_paths('src', 'benchmarks')
local_examples = join_paths('src', 'examples')
local_scripts = join_paths('src', 'scripts')

add_global_arguments('-DHAVE_CONFIG_H=1', language: 'c')
add_global_arguments('-D_GNU_SOURCE=1', language: 'c')

add_global_arguments('-DEFL_EO_API_SUPPORT=1', language: 'c')
add_global_arguments('-DEFL_BETA_API_SUPPORT=1', language: 'c')
add_global_arguments('-DNEED_RUN_IN_TREE=1', language: 'c')
add_global_arguments('-DEFL_BUILD=1', language: 'c')
add_global_arguments('-DEFL_BUILD=1', language: 'objc')
add_global_arguments('-DEFL_EO_API_SUPPORT=1', language: 'objc')
add_global_arguments('-DEFL_BETA_API_SUPPORT=1', language: 'objc')
add_global_arguments('-DELM_INTERNAL_API_ARGESFSDFEFC=1', language: 'c')

config_h = configuration_data()
config_h.set_quoted('MODULE_ARCH', module_arch)
config_h.set_quoted('PACKAGE', meson.project_name())
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('VERSION', meson.project_version())
config_h.set_quoted('LOCALE_DIR', join_paths([dir_prefix, 'share/locale']))
config_h.set_quoted('PACKAGE_URL', 'https://www.enlightenment.org')
config_h.set_quoted('PACKAGE_TARNAME', meson.project_name())
config_h.set_quoted('PACKAGE_BUGREPORT', 'enlightenment-devel@lists.sourceforge.net')
config_h.set_quoted('PACKAGE_STRING', meson.project_name() + ' ' + meson.project_version())
config_h.set_quoted('PACKAGE_NAME', meson.project_name())
config_h.set_quoted('PACKAGE_BIN_DIR', dir_bin)
config_h.set_quoted('PACKAGE_LIB_DIR', dir_lib)
config_h.set_quoted('PACKAGE_SRC_DIR', meson.source_root())
config_h.set_quoted('PACKAGE_BUILD_DIR', meson.current_build_dir())
config_h.set_quoted('PACKAGE_SYSCONF_DIR', dir_sysconf)
config_h.set_quoted('BINDIR', dir_bin)
config_h.set_quoted('DATADIR', dir_data)
config_h.set10('EFL_HAVE_THREADS', true)
config_h.set('NEED_RUN_IN_TREE', '1')

config_dir = [include_directories('.')]
eolian_include_directories = []

if target_machine.system() == 'linux' or target_machine.system() == 'bsd'
  sys_lib_extension = 'so'
  sys_exe_extension = ''
  sys_mod_extension = 'so'
elif target_machine.system() == 'windows' or target_machine.system() == 'cygwin'
  sys_lib_extension = 'dll'
  sys_exe_extension = 'exe'
  sys_mod_extension = 'dll'
elif target_machine.system() == 'darwin'
  sys_lib_extension = 'dylib'
  sys_exe_extension = ''
  sys_mod_extension = 'so'
endif

crypto = []

if (get_option('crypto') == 'gnutls')
  crypto = dependency('gnutls')
  config_h.set('HAVE_GNUTLS', '1')
elif (get_option('crypto') == 'openssl')
  crypto = dependency('openssl')
  config_h.set('HAVE_OPENSSL', '1')
endif

config_h.set_quoted('SHARED_LIB_SUFFIX', '.'+sys_lib_extension)
config_h.set_quoted('MOD_SUFFIX', '.'+sys_mod_extension)
config_h.set_quoted('EXE_SUFFIX', '.'+sys_exe_extension)
config_h.set('EFL_BUILD', '1')

subdir('header_checks')

if get_option('ecore_wl2') == true or get_option('ecore_buffer') == true
  subdir(join_paths('src', 'wayland_protocol'))
endif

ecore_evas_wayland_engine_include_dir = []

subprojects = [
# name              | mod  | lib  | bin  | bench | tests | examples | name of static libs
['eina'             , false,  true,  true,  true,  true,  true, []],
['eolian'           , false,  true,  true, false,  true, false, []],
['eo'               , false,  true, false,  true,  true, false, []],
['efl'              , false,  true, false, false,  true, false, []],
['emile'            , false,  true, false, false,  true,  true, ['lz4', 'rg_etc']],
['eet'              , false,  true,  true, false,  true,  true, []],
['ecore'            , false,  true, false, false, false, false, ['buildsystem']],
['eldbus'           , false,  true,  true, false,  true,  true, []],
['ecore'            ,  true, false, false, false,  true,  true, []], #ecores modules depend on eldbus
['ecore_audio'      , false,  true, false, false, false, false, []],
['ecore_avahi'      , false,  true, false, false, false,  true, []],
['ecore_con'        , false,  true,  true, false,  true, false, ['http-parser']],
['ecore_file'       , false,  true, false, false, false, false, []],
['eeze'             ,  true,  true, false, false,  true, false, []],
['ecore_input'      , false,  true, false, false, false, false, []],
['ecore_x'          , false,  true, false, false, false, false, []],
['ecore_fb'         , false,  true, false, false, false, false, []],
['ecore_wl2'        , false,  true, false, false, false, false, ['libdrm']],
['ecore_ipc'        , false,  true, false, false, false, false, []],
['ecore_buffer'     ,  true,  true,  true, false, false, false, []],
['ector'            , false,  true, false, false,  true, false, ['draw', 'triangulator', 'freetype']],
['elput'            , false,  true, false, false,  true, false, []],
['ecore_drm2'       , false,  true, false, false, false, false, []],
['evas'             ,  true,  true, false, false,  true,  true, ['vg_common', 'libunibreak']],
['evas_goal'        , false,  true, false, false, false, false, []],
['ecore_cocoa'      , false,  true, false, false, false, false, []],
['ecore_input_evas' , false,  true, false, false, false, false, []],
['ecore_evas'       ,  true,  true, false, false, false, false, []],
['ecore_imf'        ,  true,  true, false, false, false, false, []],
['embryo'           , false,  true,  true, false, false, false, []],
['efreet'           , false,  true, false, false,  true, false, []],
['eio'              , false,  true, false, false,  true,  true, []],
['efreet'           , false, false,  true, false, false, false, []],
['ecore_imf_evas'   , false,  true, false, false, false, false, []],
['ephysics'         , false,  true, false, false, false, false, []],
['edje'             , false,  true,  true, false,  true,  true, []],
['emotion'          ,  true,  true, false, false,  true,  true, []],
['ethumb'           ,  true,  true,  true, false, false, false, []],
['ethumb_client'    , false,  true,  true, false, false,  true, []],
['elementary'       ,  true,  true,  true, false,  true, false, ['atspi']],
]

# We generate Efl_Config.h and config.h later, they will be available here
config_dir += include_directories('.')

#we have to do that first, eina modules are required by eina
#the other modules require theire package
subdir(join_paths(local_module, 'eina'))

test_dirs = []
example_dirs = []

foreach package : subprojects
  package_name = package[0]
  package_version_name = package_name + '-' + version_major
  package_c_args = [
    '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, package_name)+'"',
    '-DNEED_RUN_IN_TREE=1'
  ]
  if get_option(package_name)
    config_h.set('HAVE_'+package_name.to_upper(), '1')

    dir_package_include = join_paths(dir_include, package_version_name)
    dir_package_modules = join_paths(dir_lib, package_name, 'modules')

    foreach static_lib : package[7]
      subdir(join_paths('src', 'static_libs', static_lib))
    endforeach

    if (package[2])
       subdir(join_paths(local_lib, package_name))
    endif
    if (package[1])
       subdir(join_paths(local_module, package_name))
    endif
    if (package[3])
       subdir(join_paths(local_bin, package_name))
    endif
    if (package[4])
       subdir(join_paths(local_benchmark, package_name))
    endif
    if (package[5])
       test_dirs += [package_name]
    endif
    if (package[6])
       example_dirs += [package_name]
    endif

    set_variable('build_'+package_name, true)

    if package_name != 'evas'
      tmp = get_variable(package_name+'_lib')
      pkgconfig.generate(tmp,
        subdirs : dir_package_include
      )
    endif

  else
    set_variable('build_'+package_name, false)

    message(package_name+' disabled!')

  endif

endforeach

subdir(join_paths('src', 'generic', 'evas'))
subdir(join_paths('src', 'generic', 'emotion'))
subdir(join_paths('data', 'edje', 'include'))
subdir(join_paths('data', 'elementary', 'themes'))
subdir(join_paths('data', 'elementary', 'objects'))


if get_option('build-tests')
  check = dependency('check')
  foreach test : test_dirs
      subdir(join_paths(local_tests, test))
  endforeach
endif

if get_option('build-examples')
  foreach example : example_dirs
      subdir(join_paths(local_examples, example))
  endforeach
endif

subdir(join_paths(local_scripts))

meson.add_install_script('meson/meson_modules.sh', module_files)

foreach evas_loader_map_inst : evas_loader_map
  evas_loader_original = evas_loader_map_inst[0]
  evas_loader_link_types = evas_loader_map_inst[1]

  meson.add_install_script('meson/evas_loader_conf.sh', evas_loader_original, evas_loader_link_types)
endforeach

#
# Configure files
#

efl_config_h = configuration_data()
efl_config_h.set('EFL_VERSION_MAJOR', version_major)
efl_config_h.set('EFL_VERSION_MINOR', version_minor)
efl_config_h.set('EFL_VERSION_MICRO', version_micro)
efl_config_h.set('EFL_BUILD_ID', get_option('build-id'))

#FIXME placeholder
efl_config_h.set('EFL_API_EO_DEF', '#define EFL_API_EO_DEF "FIXME NOT IMPLEMENTED"')
efl_config_h.set('EFL_API_LEGACY_DEF', '#define EFL_API_LEGACY_DEF "FIXME NOT IMPLEMENTED"')

configure_file(
  output: 'config.h',
  install: false,
  configuration: config_h
)

configure_file(
  input: join_paths('src','lib', 'efl', 'Efl_Config.h.in'),
  output: 'Efl_Config.h',
  install_dir : join_paths(dir_include,'efl-'+version_major),
  configuration: efl_config_h
)

configure_file(
  input: 'elm_intro.h.in',
  output: 'elm_intro.h',
  configuration: config_h,
  install_dir : join_paths(dir_include,'elementary-'+version_major)
)
