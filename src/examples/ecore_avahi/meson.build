examples = [
  'ecore_avahi_browse_example',
  'ecore_avahi_example',
]

foreach example : examples
  executable(example, example + '.c', dependencies: [ecore_avahi_deps, ecore_avahi])
endforeach
