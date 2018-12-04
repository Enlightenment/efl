ecore_audio_deps = [ecore]
ecore_audio_pub_deps = [eina, eo]

pub_eo_files = [
  'ecore_audio.eo',
  'ecore_audio_in.eo',
  'ecore_audio_out.eo',
  'ecore_audio_in_sndfile.eo',
  'ecore_audio_out_sndfile.eo',
  'ecore_audio_out_pulse.eo',
  'ecore_audio_in_tone.eo',
  'ecore_audio_out_wasapi.eo'
]

foreach eo_file : pub_eo_files
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

pub_eo_types_files = []

ecore_audio_header_src = [
  'Ecore_Audio.h',
  'ecore_audio_obj.h',
  'ecore_audio_obj_in.h',
  'ecore_audio_obj_out.h',
  'ecore_audio_obj_in_tone.h',
  'ecore_audio_protected.h',
  'ecore_audio_obj_out_pulse.h',
  'ecore_audio_obj_in_sndfile.h',
  'ecore_audio_obj_out_sndfile.h',
  'ecore_audio_obj_out_wasapi.h'
]

ecore_audio_src = [
  'ecore_audio.c',
  'ecore_audio_obj.c',
  'ecore_audio_obj_in.c',
  'ecore_audio_obj_out.c',
  'ecore_audio_obj_in_tone.c',
  'ecore_audio_private.h'
]

ecore_audio_src += [
  'ecore_audio_obj_in_sndfile.c',
  'ecore_audio_obj_out_sndfile.c',
  'ecore_audio_sndfile_vio.c'
]

ecore_audio_deps += dependency('sndfile')
config_h.set('HAVE_SNDFILE', '1')

if get_option('pulseaudio')
  ecore_audio_deps += dependency('libpulse')

  ecore_audio_src += [
    'ecore_audio_pulse_ml.c',
    'ecore_audio_obj_out_pulse.c'
  ]

  config_h.set('HAVE_PULSE', '1')
endif

if sys_windows == true
  ecore_audio_src += [
    'ecore_audio_obj_out_wasapi.c'
  ]
  ecore_audio_deps += [cc.find_library('ksuser', required: true), cc.find_library('winmm', required: true)]
endif

ecore_audio_lib = library('ecore_audio',
    ecore_audio_src, pub_eo_file_target,
    dependencies: ecore_audio_pub_deps + [m] + ecore_audio_deps,
    include_directories : config_dir,
    install: true,
    version : meson.project_version()
)

ecore_audio = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_audio_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_audio_pub_deps,
)

install_data(pub_eo_files + pub_eo_types_files,
  install_dir: eolian_ecore_dir
)

install_headers(ecore_audio_header_src,
  install_dir : dir_package_include,
)
