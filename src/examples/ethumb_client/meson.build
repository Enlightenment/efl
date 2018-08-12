files = [
  'ethumb_dbus'
]

foreach file : files
  executable(file,
    file + '.c',
    dependencies: [ethumb, ethumb_client, ecore, eina],
  )
endforeach