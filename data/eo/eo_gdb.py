# Implement eo_break that'll break on a macro/subid/whatever.

import gdb

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

# Very crude, but works for the meanwhile
class Eo_backtrace(gdb.Command):
   def __init__(self):
      gdb.Command.__init__(self, "eo_backtrace", gdb.COMMAND_OBSCURE)

   def invoke (self, arg, from_tty):
      btrace = gdb.execute("backtrace", False, to_string=True).split('\n')

      for line in btrace:
         if line.find("libeo.so") == -1 and line.find("lib/eo/") == -1:
            print line

Eo_backtrace()
