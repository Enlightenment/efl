subdir('ftraster')
subdir('util')
subdir('scenegraph')
subdir('text')
subdir('lottie')


ssg_sources = ssg_sources_scenegraph
ssg_sources += ssg_sources_util
ssg_sources += ssg_sources_ftraster
ssg_sources += ssg_sources_lottie


library_deps =[]
if (build_machine.system() == 'linux')
    ssg_sources += ssg_sources_text
    pango_dep = dependency('pango')
    library_deps += pango_dep
    pangoft2_dep = dependency('pangoft2')
    library_deps += pangoft2_dep
endif

ssglib = shared_library('ssg',
                        ssg_sources,
                        include_directories : inc,
                        version : ssg_lib_version,
                        dependencies : library_deps,
                        install : true)
