#!/usr/bin/python3
import os
import sys
import subprocess
import json
import time
import concurrent.futures
import argparse
import tempfile

#
# preparation calls for the examples
#

def prep_eina_file_02():
  f = tempfile.NamedTemporaryFile(delete=False)
  f.write(b"Simulation")
  return [f.name, "/tmp/copy_file"]

def prep_eina_xattr_01():
  f = tempfile.NamedTemporaryFile(delete=False)
  f.write(b"Simulation")
  return ["list", f.name]

def prep_eina_xattr_02():
  f1 = tempfile.NamedTemporaryFile(delete=False)
  f1.write(b"Simulation")
  f2 = tempfile.NamedTemporaryFile(delete=False)
  f2.write(b"Simulation2")
  return [f1.name, f2.name]

def prep_eet_data_simple():
  f1 = tempfile.NamedTemporaryFile(delete=False)
  f1.write(b"Simulation")
  f2 = tempfile.NamedTemporaryFile(delete=False)
  f2.write(b"Simulation2")
  return [f1.name, f2.name]

def prep_eet_data_nested():
  f1 = tempfile.NamedTemporaryFile(delete=False)
  f1.write(b"Simulation")
  f2 = tempfile.NamedTemporaryFile(delete=False)
  f2.write(b"Simulation2")
  return [f1.name, f2.name]

def prep_eet_data_file_descriptor_01():
  f1 = tempfile.NamedTemporaryFile(delete=False)
  f1.write(b"Simulation")
  f2 = tempfile.NamedTemporaryFile(delete=False)
  f2.write(b"Simulation2")
  return [f1.name, f2.name, "acc", "Example-Simulation"]

def prep_eet_data_file_descriptor_02():
  f1 = tempfile.NamedTemporaryFile(delete=False)
  f1.write(b"Simulation")
  f2 = tempfile.NamedTemporaryFile(delete=False)
  f2.write(b"Simulation2")
  return [f1.name, f2.name, "union", "5", "Example-Simulation"]

example_preparation = {
  "eina_file_02" : prep_eina_file_02,
  "eina_xattr_01" : prep_eina_xattr_01,
  "eina_xattr_02" : prep_eina_xattr_02,
  "eet-data-simple" : prep_eet_data_simple,
  "eet-data-nested" : prep_eet_data_nested,
  "eet-data-simple" : prep_eet_data_simple,
  "eet-data-file_descriptor_01" : prep_eet_data_file_descriptor_01,
  "eet-data-file_descriptor_02" : prep_eet_data_file_descriptor_02,
}

#
# Holds up the state of the ran examples
#

class State:
  def __init__(self, examples):
    self.max_n = examples
    self.n = 1
    self.count_fail = 0
    self.count_success = 0
    self.count_err_output = 0
    print("Found "+str(self.max_n)+" Examples")

  def add_run(self, command, error_in_output, exitcode):
    print("{}/{} {} {} {} ".format(self.n, self.max_n, ("SUCCESS" if exitcode == 0 else "FAIL"), ("CLEAN" if error_in_output == False else "ERR"), command))
    self.n = self.n + 1
    if exitcode != 0:
      self.count_fail += 1
    if error_in_output == True:
      self.count_err_output += 1
    if exitcode == 0 and error_in_output == False:
      self.count_success += 1

  def print_summary(self):
    print("Summary")
    print("  Failed: "+str(self.count_fail)+"/"+str(self.max_n))
    print("  Errored: "+str(self.count_err_output)+"/"+str(self.max_n))
    print("  Success: "+str(self.count_success)+"/"+str(self.max_n))

#
# this simulates the startup of the example, and the closing after 1s
#

def simulate_example(example):
  args = []
  if os.path.basename(example) in example_preparation:
    args = example_preparation[os.path.basename(example)]()
  run = subprocess.Popen([G.builddir + "/" + example] + args,
      stdout = subprocess.PIPE,
      stderr = subprocess.PIPE,
  )
  time.sleep(1)
  run.terminate()
  try:
    outs, errs = run.communicate(timeout=2)
  except Exception as e:
    run.kill()
    return (example, True, -1)
  else:
    return (example, True if b'ERR' in outs or b'ERR' in errs else False, run.poll())


parser = argparse.ArgumentParser(description='Run the examples of efl')
parser.add_argument('builddir', metavar='build', help='the path where to find the meson build directory')

G = parser.parse_args()
#Run meson to fetch all examples
meson_introspect = subprocess.Popen(["meson", "introspect", G.builddir, "--targets"],
      stdout = subprocess.PIPE,
      stderr = subprocess.PIPE,
)
meson_introspect.poll()
build_targets = json.loads(meson_introspect.stdout.read())
examples = [b["filename"] for b in build_targets if "examples" in b["filename"] and b["type"] == "executable"]
state = State(len(examples))
#simulate all examples in parallel with up to 5 runners
with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
  futures = [executor.submit(simulate_example, example) for example in examples]
  for future in concurrent.futures.as_completed(futures):
    example_run = future.result()
    state.add_run(example_run[0], example_run[1], example_run[2])
state.print_summary()
