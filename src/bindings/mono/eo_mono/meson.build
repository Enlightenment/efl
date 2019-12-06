mono_files += files(
  'iwrapper.cs',
  'workaround.cs',
  'EoWrapper.cs',
  'FunctionWrapper.cs',
  'NativeModule.cs',
  'StringMarshalers.cs',
  'WrapperSupervisor.cs',
)

if host_machine.system() == 'windows'
   mono_files += files('FunctionWrapper_Windows.cs', 'NativeModule_Windows.cs')
else
   mono_files += files('FunctionWrapper_Unix.cs', 'NativeModule_Unix.cs')
endif
