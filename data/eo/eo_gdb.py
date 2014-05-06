# Implement eo_break that'll break on a macro/subid/whatever.

import gdb

def symbol_equal_to_string(symbol, string):
   return (symbol != None) and (symbol.name == string)

class Eo_step(gdb.Command):
   STEP_LIMIT = 10
   def __init__(self):
      gdb.Command.__init__(self, "eo_step", gdb.COMMAND_OBSCURE)
      self.START_FUNC = "_eo_call_resolve"
      self.SKIP_FUNC = "_eo_do_start"

   def invoke (self, arg, from_tty):
      # Get to the call resolve function.
      i = 0
      while not symbol_equal_to_string(gdb.selected_frame().function(), self.START_FUNC):
         if symbol_equal_to_string(gdb.selected_frame().function(), self.SKIP_FUNC):
            gdb.execute("finish", False, to_string=True)

         if i > Eo_step.STEP_LIMIT:
             break
         else:
             i += 1
         gdb.execute("step", False, to_string=True)

      # If we found the function, return from it, otherwise, fail.
      if symbol_equal_to_string(gdb.selected_frame().function(), self.START_FUNC):
         gdb.execute("finish", False, to_string=True)
      else:
         print "Search limit reached, you tried calling eo_step too far from an eo_do."
         return

      # Step until we move to a different function. FIXME: The hook can confuse us, needs to be solved.
      cur_func = gdb.selected_frame().function()
      while gdb.selected_frame().function() == cur_func:
         gdb.execute("stepi", False, to_string=True)

      # One last call to skip into the implementation
      gdb.execute("step", True)

Eo_step()
