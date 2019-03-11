mono_files += files(
  'iwrapper.cs',
  'workaround.cs',
  'FunctionWrapper.cs',
  'NativeModule.cs'
)

if host_machine.system() == 'windows'
   mono_files += files('FunctionWrapper_Windows.cs', 'NativeModule_Windows.cs')
else
   mono_files += files('FunctionWrapper_Unix.cs', 'NativeModule_Unix.cs')
endif
