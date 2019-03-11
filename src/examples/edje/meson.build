edc_files = [
  'animations.edc',
  'animations2.edc',
  'basic.edc',
  'basic2.edc',
  'bezier-transition-example.edc',
  'box.edc',
  'box_example.edc',
  'color-class.edc',
  'drag.edc',
  'dynamic_multiseat.edc',
  'edje-group.edc',
  'edje-nested.edc',
  'edje-threshold.edc',
  'edje_example.edc',
  'embryo_custom_state.edc',
  'embryo_pong.edc',
  'embryo_run_program.edc',
  'embryo_set_state.edc',
  'embryo_set_state_anim.edc',
  'embryo_set_text.edc',
  'embryo_timer.edc',
  'embryo_tween_anim.edc',
  'entry.edc',
  'external_elm_anchorblock.edc',
  'external_elm_button.edc',
  'external_elm_check.edc',
  'external_elm_panes.edc',
  'external_emotion_elm.edc',
  'focus.edc',
  'lua_script.edc',
  'messages_echo.edc',
  'multiseat.edc',
  'multiseat_custom_names.edc',
  'multiseat_filter.edc',
  'perspective.edc',
  'signals-messages.edc',
  'signalsBubble.edc',
  'sigtest.edc',
  'svg.edc',
  'swallow.edc',
  'table.edc',
  'text.edc',
  'textblock-hyphen.edc',
  'toggle_using_filter.edc'
]

if (get_option('physics'))
  edc_files += [
    'physics_3d.edc',
    'physics_actions.edc',
    'physics_backcull.edc',
    'physics_basic.edc',
    'physics_complex.edc',
    'physics_soft_bodies.edc'
  ]
endif

themes = []

foreach edc_file : edc_files
   themes += custom_target('edje_cc_' + edc_file,
    input : edc_file,
    output : '@BASENAME@.edj',
    command : edje_cc_exe + [ '-beta', '-fastcomp',
              '-id', meson.current_source_dir(),
              '-fd', meson.current_source_dir(),
              '-sd', meson.current_source_dir(),
              '-vd', meson.current_source_dir(),
              '-dd', meson.current_source_dir(),
              '-md', meson.current_source_dir(),
              '-td', meson.current_source_dir(),
              '@INPUT@', '@OUTPUT@'],
    depends : edje_cc, )
endforeach

codegen = custom_target('edje_cc_codegen_edc',
  input : 'codegen.edc',
  output : '@BASENAME@.edj',
  command : edje_cc_exe + [ '-beta', '-fastcomp',
            '-id', meson.current_source_dir(),
            '-fd', meson.current_source_dir(),
            '-sd', meson.current_source_dir(),
            '-vd', meson.current_source_dir(),
            '-dd', meson.current_source_dir(),
            '-md', meson.current_source_dir(),
            '-td', meson.current_source_dir(),
            '@INPUT@', '@OUTPUT@'],
  depends : edje_cc, )

themes += custom_target('edje_codegen_codegen.edj',
            input : codegen,
            output : ['@BASENAME@_example_generated.c', '@BASENAME@_example_generated.h'],
            command : edje_codegen_exe + [ '--prefix=codegen_example',  '@INPUT@',
                       'example_group', '@OUTPUT0@', '@OUTPUT1@'],
            depends : [edje_codegen, themes])

edje_examples = [
  'animations2',
  'edje-animations',
  'edje-basic',
  'edje-basic2',
  'edje-box',
  'edje-box2',
  'edje-codegen-example',
  'edje-color-class',
  'edje-drag',
  'edje-dynamic-multiseat',
  'edje-edit-part-box',
  'edje-entry',
  'edje-focus',
  'edje-multiseat',
  'edje-multiseat-custom-names',
  'edje-multiseat-filter',
  'edje-multisense',
  'edje-perspective',
  'edje-signals-messages',
  'edje-swallow',
  'edje-swallow2',
  'edje-table',
  'edje-text',
  'edje-textblock-hyphenation',
  'edje_example',
  'signals2',
  'sigtest',
  'svg-test'
]

foreach edje_example : edje_examples
  executable(edje_example,
    [edje_example + '.c'] +  themes,
    dependencies: [edje, ecore_evas],
    c_args: ['-DPACKAGE_DATA_DIR="'+ meson.current_build_dir()+'"',]
  )
endforeach
