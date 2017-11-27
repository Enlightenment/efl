project('scenegraph library',
        'cpp',
        license : 'MIT',
        default_options : ['libdir=lib/ssg', 'c_std=c11', 'cpp_std=c++11'])
ssg_lib_version = '0.0.1'

compiler_flags = ['-Wall','-std=c++11']
if (build_machine.system() == 'linux')
    compiler_flags += ['-pthread']
    add_global_link_arguments('-pthread', language: 'cpp')
endif

add_global_arguments(compiler_flags, language: 'cpp')

inc = include_directories('include')

subdir('include')
subdir('src')

subdir('example')

if get_option('test') == true
   subdir('test')
endif

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : ssglib,
                 version : '1.0',
                 name : 'libssg',
                 filebase : 'ssg',
                 description : 'A Library for scenegraph management.')
