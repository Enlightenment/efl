class Eo_step(gdb.Command):
   def __init__(self):
      gdb.Command.__init__(self, "eo_step", gdb.COMMAND_OBSCURE)

   def invoke (self, arg, from_tty):
      # While libeo is not reached, we step into
      while gdb.solib_name(gdb.selected_frame().pc()).find("libeo.so") == -1:
         # step by one assembly instruction, no print
         gdb.execute("stepi", False, to_string=True)

      # While we are in libeo or in an unknown function, we step into
      while (gdb.selected_frame().function() == None) or (gdb.solib_name(gdb.selected_frame().pc()).find("libeo.so") != -1):
         # step by one assembly instruction, no print
         gdb.execute("stepi", False, to_string=True)

      print "Stopped at file " + gdb.selected_frame().find_sal().symtab.filename+ " line " + str(gdb.selected_frame().find_sal().line) + " function " + str(gdb.selected_frame().function())
Eo_step()
