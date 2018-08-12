files = [
  'efl_io_manager_ls',
  'efl_io_manager_open',
  'efl_io_manager_open_multi',
  'eio_file_copy',
  'eio_file_ls',
  'eio_sentry'
]

foreach file : files
  executable(file,
    file + '.c',
    dependencies: [ecore, eio],
  )
endforeach