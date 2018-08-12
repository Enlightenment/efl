files = [
  'emotion_basic_example',
  'emotion_border_example',
  'emotion_generic_example',
  'emotion_generic_subtitle_example',
  'emotion_signals_example',
]

foreach file : files
  executable(file,
    file + '.c',
    dependencies: [emotion, evas, ecore_evas, ecore],
  )
endforeach