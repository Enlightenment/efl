examples = [
  'connman-list-services',
  'ofono-dial',
  'banshee',
  'complex-types',
  'complex-types-server',
  'server',
  'client',
  'simple-signal-emit',
  'complex-types-client-eina-value',
  'connect-address',
  'dbusmodel',
]

foreach example : examples
  executable(example, example + '.c', dependencies: [eldbus, ecore])
endforeach
