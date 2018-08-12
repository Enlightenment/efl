examples = [
  'eet-basic',
  'eet-file',
  'eet-data-simple',
  'eet-data-nested',
  'eet-data-file_descriptor_01',
  'eet-data-file_descriptor_02',
  'eet-data-cipher_decipher'
]

foreach example : examples
  executable(example, example + '.c', dependencies: eet)
endforeach
