generic_src = files([
  'emotion_gstreamer.h',
  'emotion_gstreamer.c',
  'emotion_alloc.c',
  'emotion_convert.c',
  'emotion_sink.c'
])

generic_deps = [
  dependency('gstreamer-1.0'),
  dependency('gstreamer-plugins-base-1.0'),
  dependency('gstreamer-video-1.0'),
  dependency('gstreamer-audio-1.0'),
  dependency('gstreamer-tag-1.0'),
  dependency('gstreamer-pbutils-1.0'),
  ]

# This module registers a GType, and a GType can never be unregistered. The
# name it registers is a string inside this shared object, and GLib keeps
# the pointer for the life of the process. Unloading the module unmaps the
# string while GLib is still holding it, and the next load walks the same
# hash bucket and reads through it. Tell the linker not to unload.
gstreamer1_link_args = []
if cc.has_link_argument('-Wl,-z,nodelete')
  gstreamer1_link_args += '-Wl,-z,nodelete'
endif

shared_module(emotion_loader,
    generic_src,
    include_directories : config_dir,
    dependencies: [eina, evas, emotion, generic_deps],
    install: true,
    install_dir : mod_install_dir,
    c_args : package_c_args,
    link_args : gstreamer1_link_args,
    name_suffix : sys_mod_extension
)
