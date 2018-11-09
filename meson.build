project('efl', ['c','cpp'],
  version: '1.21.99',
  default_options : ['buildtype=plain'],
  meson_version : '>=0.47'
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
version_name = 'v-'+version_major + '.' + version_minor

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
local_bindings = join_paths('src', 'bindings')
local_bin = join_paths('src', 'bin')
local_module = join_paths('src', 'modules')
local_tests = join_paths('src', 'tests')
local_benchmark = join_paths('src', 'benchmarks')
local_examples = join_paths('src', 'examples')
local_scripts = join_paths('src', 'scripts')

foreach lang : ['c', 'objc', 'cpp']
  add_global_arguments('-DHAVE_CONFIG_H=1', language: lang)
  add_global_arguments('-D_GNU_SOURCE=1', language: lang)
  add_global_arguments('-DEFL_EO_API_SUPPORT=1', language: lang)
  add_global_arguments('-DEFL_BETA_API_SUPPORT=1', language: lang)
  add_global_arguments('-DNEED_RUN_IN_TREE=1', language: lang)
  add_global_arguments('-DEFL_BUILD=1', language: lang)
  add_global_arguments('-DELM_INTERNAL_API_ARGESFSDFEFC=1', language: lang)
endforeach

config_h = configuration_data()
config_h.set_quoted('MODULE_ARCH', version_name)
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
config_h.set10('SLOPPY_SPEC', true)

## have to get compiler again for this to work
compiler = meson.get_compiler('c')
code = '''#define _GNU_SOURCE 1
#include <unistd.h>
#include <stdio.h>

extern char **environ;

void func(void) { printf("%p\n", environ); }
'''
if compiler.compiles(code, args : '-lc', name : 'environ check') == true
  config_h.set10('HAVE_ENVIRON', true)
endif

## or should this be target_machine?
cpu_mmx = false
cpu_sse3 = false
cpu_neon = false
cpu_neon_intrinsics = false
cpu_altivec = false
evas_opt_c_args = [ ]
machine_c_args = [ ]
compiler = meson.get_compiler('c')
if host_machine.cpu_family() == 'x86' or host_machine.cpu_family() == 'x86_64'
  if compiler.check_header('immintrin.h') == true
    if (get_option('cpu-mmx') == true)
      config_h.set10('BUILD_MMX', true)
      cpu_mmx = true
      message('x86 build - MMX enabled')
      if (get_option('cpu-sse3') == true)
        config_h.set10('BUILD_SSE3', true)
        evas_opt_c_args += [ '-msse3' ]
        cpu_sse3 = true
        message('x86 build - SSE3 enabled')
      endif
    endif
  endif
elif host_machine.cpu_family() == 'arm'
  if compiler.check_header('arm_neon.h') == true
    if (get_option('cpu-neon') == true)
      config_h.set10('BUILD_NEON', true)
      machine_c_args += ['-mfpu=neon', '-ftree-vectorize']
      cpu_neon = true
      message('ARM build - NEON enabled')
    endif
  endif
elif host_machine.cpu_family() == 'aarch64'
  if compiler.check_header('arm_neon.h') == true
    if (get_option('cpu-neon') == true)
      config_h.set10('BUILD_NEON', true)
      config_h.set10('BUILD_NEON_INTRINSICS', true)
      machine_c_args += ['-ftree-vectorize']
      cpu_neon = true
      cpu_neon_intrinsics = true
      message('ARM64 build - NEON + intrinsics enabled')
    endif
  endif
elif host_machine.cpu_family() == 'ppc' or host_machine.cpu_family() == 'ppc64'
  if compiler.check_header('altivec.h') == true
    if (get_option('cpu-akltivec') == true)
      config_h.set10('BUILD_ALTIVEC', true)
      machine_c_args += [ '-maltivec' ]
      cpu_altivec = true
      message('PPC/POWER build - ALTIVEC enabled')
    endif
  endif
endif

config_dir = [include_directories('.')]
eolian_include_directories = []

windows = ['windows', 'cygwin']
#bsd for meson 0.46 and 0.47
bsd = ['bsd', 'freebsd', 'dragonfly', 'netbsd']
linux = ['linux']
osx = ['darwin']

sys_linux = linux.contains(target_machine.system())
sys_bsd = bsd.contains(target_machine.system())
sys_windows = windows.contains(target_machine.system())
sys_osx = osx.contains(target_machine.system())

if sys_linux == true or sys_bsd == true
  sys_lib_extension = 'so'
  sys_exe_extension = ''
  sys_mod_extension = 'so'
elif sys_windows == true
  sys_lib_extension = 'dll'
  sys_exe_extension = 'exe'
  sys_mod_extension = 'dll'
elif sys_osx == true
  sys_lib_extension = 'dylib'
  sys_exe_extension = ''
  sys_mod_extension = 'dylib'
else
  error('System '+target_machine.system()+' not known')
endif

if (get_option('crypto') == 'gnutls')
  # gcrypt does not want to provide a pkg config file so we try the lib
  crypto = [dependency('gnutls'), cc.find_library('gcrypt')]
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
subdir('po')

if get_option('wl') == true
  subdir(join_paths('src', 'wayland_protocol'))
endif

ecore_evas_wayland_engine_include_dir = []

subprojects = [
# name              |   option              | mod  | lib  | bin  | bench | tests | examples | pkg-config options | name of static libs
['eina'             ,[]                    , false,  true,  true,  true,  true,  true, [], []],
['eolian'           ,[]                    , false,  true,  true, false,  true, false, ['eina'], []],
['eo'               ,[]                    , false,  true, false,  true,  true, false, ['eina'], []],
['efl'              ,[]                    , false,  true, false, false,  true, false, ['eo'], []],
['emile'            ,[]                    , false,  true, false, false,  true,  true, ['eina', 'efl'], ['lz4', 'rg_etc']],
['eet'              ,[]                    , false,  true,  true, false,  true,  true, ['eina', 'emile', 'efl'], []],
['ecore'            ,[]                    , false,  true, false, false, false, false, ['eina', 'eo', 'efl'], ['buildsystem']],
['eldbus'           ,[]                    , false,  true,  true, false,  true,  true, ['eina', 'eo', 'efl'], []],
['ecore'            ,[]                    ,  true, false, false, false,  true,  true, ['eina', 'eo', 'efl'], []], #ecores modules depend on eldbus
['ecore_audio'      ,[]                    , false,  true, false, false, false, false, ['eina', 'eo'], []],
['ecore_avahi'      ,['avahi']             , false,  true, false, false, false,  true, ['eina', 'ecore'], []],
['ecore_con'        ,[]                    , false,  true,  true, false,  true, false, ['eina', 'eo', 'efl', 'ecore'], ['http-parser']],
['ecore_file'       ,[]                    , false,  true, false, false, false, false, ['eina'], []],
['eeze'             ,['eeze']              ,  true,  true,  true, false,  true, false, ['eina', 'efl'], []],
['ecore_input'      ,[]                    , false,  true, false, false, false, false, ['eina', 'eo'], []],
['ecore_x'          ,['x11']               , false,  true, false, false, false, false, ['eina', 'efl'], []],
['ecore_fb'         ,['fb']                , false,  true, false, false, false, false, ['eina'], []],
['ecore_wl2'        ,['wl']                ,  true,  true, false, false, false, false, ['eina'], ['libdrm']],
['ecore_sdl'        ,['sdl']               , false,  true, false, false, false, false, ['eina'], []],
['ecore_ipc'        ,[]                    , false,  true, false, false, false, false, ['eina'], []],
['ecore_buffer'     ,['buffer']            ,  true,  true,  true, false, false, false, ['eina'], []],
['ector'            ,[]                    , false,  true, false, false,  true, false, ['eina', 'efl'], ['draw', 'triangulator', 'freetype']],
['elput'            ,['drm']               , false,  true, false, false,  true, false, ['eina', 'eldbus'], []],
['ecore_drm2'       ,['drm']               , false,  true, false, false, false, false, ['ecore'], ['libdrm']],
['ecore_cocoa'      ,['cocoa']             , false,  true, false, false, false, false, ['eina'], []],
['evas'             ,[]                    ,  true,  true, false, false,  true,  true, ['eina', 'efl', 'eo'], ['vg_common', 'libunibreak']],
['evas_goal'        ,[]                    , false,  true, false, false, false, false, ['eina', 'efl', 'eo'], []],
['ecore_input_evas' ,[]                    , false,  true, false, false, false, false, ['eina', 'evas'], []],
['ecore_evas'       ,[]                    ,  true,  true,  true, false, false, false, ['evas', 'ector'], []],
['ecore_imf'        ,[]                    ,  true,  true, false, false, false, false, ['eina'], []],
['embryo'           ,[]                    , false,  true,  true, false, false, false, ['eina', 'efl', 'eo'], []],
['efreet'           ,[]                    , false,  true, false, false,  true, false, ['eina', 'efl', 'eo'], []],
['eio'              ,[]                    , false,  true, false, false,  true,  true, ['eina', 'eet'], []],
['efreet'           ,[]                    , false, false,  true, false, false, false, ['eina', 'efl', 'eo'], []],
['ecore_imf_evas'   ,[]                    , false,  true, false, false, false, false, ['eina', 'efl', 'eo'], []],
['ephysics'         ,['physics']           , false,  true, false, false, false, false, ['eina', 'efl', 'eo'], []],
['edje'             ,[]                    , false,  true,  true, false,  true,  true, ['evas', 'eo', 'efl', 'luajit'], []],
['emotion'          ,[]                    ,  true,  true, false, false,  true,  true, ['eina', 'efl', 'eo'], []],
['ethumb'           ,[]                    ,  true,  true,  true, false, false, false, ['eina', 'efl', 'eo'], []],
['ethumb_client'    ,[]                    , false,  true,  true, false, false,  true, ['eina', 'efl', 'eo', 'ethumb'], []],
['elocation'        ,[]                    , false,  true, false, false, false, false, ['ecore', 'eldbus'], []],
['elementary'       ,[]                    ,  true,  true,  true, false,  true, false, ['eina', 'efl', 'eo', 'eet', 'evas', 'ecore', 'ecore-evas', 'ecore-file', 'ecore-input', 'edje', 'ethumb-client', 'emotion', 'ecore-imf', 'ecore-con', 'eldbus', 'efreet', 'efreet-mime', 'efreet-trash', 'eio', 'elocation'], ['atspi']],
['efl_wl'           ,['wl']                , false,  true,  true, false, false, false, ['evas', 'ecore'], []],
['elua'             ,['elua']              , false,  true,  true, false,  true, false, ['eina', 'luajit'], []],
['ecore_wayland'    ,['wl-deprecated']     , false,  true, false, false, false, false, ['eina'], []],
['ecore_drm'        ,['drm-deprecated']    , false,  true, false, false, false, false, ['eina'], []],
]

# We generate Efl_Config.h and config.h later, they will be available here
config_dir += include_directories('.')

#we have to do that first, eina modules are required by eina
#the other modules require theire package
subdir(join_paths(local_module, 'eina'))

test_dirs = []
example_dirs = []
tmp_empty = declare_dependency()


foreach package : subprojects
  package_name = package[0]
  package_version_name = '-'.join(package_name.split('_')) + '-' + version_major
  package_c_args = [
    '-DPACKAGE_DATA_DIR="'+ join_paths(dir_data, package_name)+'"',
    '-DNEED_RUN_IN_TREE=1',
    machine_c_args
  ]
  automatic_pkgfile = true
  if package[1].length() == 0 or get_option(package[1][0])
    config_h.set('HAVE_'+package_name.to_upper(), '1')

    dir_package_include = join_paths(dir_include, package_version_name)
    dir_package_modules = join_paths(dir_lib, package_name, 'modules')

    #ensure that we really dont copy the eo file targets from a previous library
    #Those are the variables that can be used to reflect the libraries speical handlings
    # -> at the end is used to indicate where to find this variable outside of this for loop

    #public eo and eot files - which probebly have to be used later for bindings
    pub_eo_files = []           # -> package_name + '_eo_files'
    pub_eo_types_files = []     # -> package_name + '_eot_files'
    #All subdirs where eo files that are listed in the pub_* variables can be found
    #For every element != '' a variable called package_name + '_' + subir + '_eot_files' and package_name + '_' + subir + '_eo_files' must exist.
    package_eo_subdirs = ['']     # -> package_name + '_eo_subdirs'
    #All subdirs that should be included in order to include every requried header
    package_header_subdirs = [] # -> package_name + '_include_subdirs'
    #eo file targets, this list of targets can be used to ensure the files are created before accessed
    pub_eo_file_target = []
    #private eo files target - never use this :)
    priv_eo_file_target = []
    #use this variable to store custom variables in that should be placed in the .pc file
    package_pc_variables = []

    foreach static_lib : package[9]
      if get_variable(static_lib, tmp_empty) == tmp_empty
        subdir(join_paths('src', 'static_libs', static_lib))
      endif
    endforeach

    if (package[3])
      subdir(join_paths(local_lib, package_name))
      set_variable(package_name + '_eo_files', pub_eo_files)
      set_variable(package_name + '_eot_files', pub_eo_types_files)
      set_variable(package_name + '_header_subdirs', package_header_subdirs)
      set_variable(package_name + '_eo_subdirs', package_eo_subdirs)
    endif
    if (package[2])
       subdir(join_paths(local_module, package_name))
    endif
    if (package[4])
       subdir(join_paths(local_bin, package_name))
    endif
    if (package[5])
       subdir(join_paths(local_benchmark, package_name))
    endif
    if (package[6])
       test_dirs += [package_name]
    endif
    if (package[7])
       example_dirs += [package_name]
    endif

    set_variable('build_'+package_name, true)

    # generate automatic pc files for libraries
    if automatic_pkgfile == true and package[3]
      tmp_lib = get_variable(package_name+'_lib')
      tmp_deps = get_variable(package_name+'_deps')
      tmp_pub_deps = get_variable(package_name+'_pub_deps')
      tmp_package_subdirs = []

      foreach subdir : package_header_subdirs
        tmp_package_subdirs += join_paths(package_version_name, subdir)
      endforeach

      pkgconfig.generate(tmp_lib,
        name : '-'.join(package_name.split('_')),
        subdirs : [package_version_name] + tmp_package_subdirs,
        version : version_major + '.' + version_minor,
        libraries : tmp_pub_deps,
        requires : package[8],
        variables : package_pc_variables
      )
      if package_name == 'ethumb_client'
        pkgconfig.generate(tmp_lib,
          name : package_name,
          subdirs : [package_version_name] + tmp_package_subdirs,
          version : version_major + '.' + version_minor,
          libraries : tmp_pub_deps,
          requires : package[8],
        )
      endif
    endif
  else
    message(package_name+' disabled!')
  endif

endforeach

#build this later, as the debug services are depending on ecore
subdir(join_paths('src', 'bin', 'efl'))

subdir(join_paths('src', 'generic', 'evas'))
subdir(join_paths('src', 'generic', 'emotion'))

bindings = ['luajit', 'cxx', 'mono']

foreach binding : bindings
  if get_option(binding)
    subdir(join_paths('src', 'bindings', binding))
  endif
endforeach

subdir(join_paths('src', 'edje_external'))


subdir(join_paths('data'))

if get_option('build-tests')
  check = dependency('check')
  subdir(join_paths('src', 'tests'))
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

subdir(join_paths('systemd-services'))
subdir(join_paths('dbus-services'))

#output the three new efl-* .pc files
efl_20_pc_files = [
  ['efl-ui', ['elementary']],
  ['efl-core', ['ecore', 'efl', 'emile']],
  ['efl-net', ['ecore', 'ecore-con', 'emile']],
]

foreach pc_file : efl_20_pc_files
  name = pc_file[0]
  libraries = pc_file[1]
  pkgconfig.generate(
    name : '-'.join(name.split('_')),
    description: name+' configutation file',
    requires : libraries,
  )
endforeach
