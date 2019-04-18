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

def prep_ecore_getopt_example():
  return ["string-1", "120", "apple", "none-optional-arg"]

def prep_eio_file_copy():
  f1 = tempfile.NamedTemporaryFile(delete=False)
  f1.write(b"Simulation")
  return [f1.name, "/tmp/eio_dst_file"]

def prep_eio_file_ls():
  return ["/tmp/"]

def prep_edje_color_class():
  return ["beta", "red", "green", "blue"]

def prep_ecore_con_url_headers_example():
  return ["GET", "www.enlightenment.org"]

def prep_ecore_con_url_download_example():
  return ["www.enlightenment.org"]

def prep_ecore_con_url_cookies_example():
  return ["www.enlightenment.org"]

def prep_ecore_con_client_example():
  return ["enlightenment.org", "80"]

def prep_ecore_con_lookup_example():
  return ["enlightenment.org"]

def prep_ecore_con_server_example():
  return ["enlightenment.org", "1234"]

def prep_ecore_con_client_simple_example():
  return ["enlightenment.org", "80"]

example_preparation = {
  "eina_file_02" : prep_eina_file_02,
  "eina_xattr_01" : prep_eina_xattr_01,
  "eina_xattr_02" : prep_eina_xattr_02,
  "eet-data-simple" : prep_eet_data_simple,
  "eet-data-nested" : prep_eet_data_nested,
  "eet-data-simple" : prep_eet_data_simple,
  "eet-data-file_descriptor_01" : prep_eet_data_file_descriptor_01,
  "eet-data-file_descriptor_02" : prep_eet_data_file_descriptor_02,
  "ecore_getopt_example" : prep_ecore_getopt_example,
  "eio_file_copy" : prep_eio_file_copy,
  "eio_file_ls" : prep_eio_file_ls,
  "edje-color-class" : prep_edje_color_class,
  "ecore_con_url_headers_example" : prep_ecore_con_url_headers_example,
  "ecore_con_url_download_example" : prep_ecore_con_url_download_example,
  "ecore_con_url_cookies_example" : prep_ecore_con_url_cookies_example,
  "ecore_con_client_example" : prep_ecore_con_client_example,
  "ecore_con_lookup_example" : prep_ecore_con_lookup_example,
  "ecore_con_server_example" : prep_ecore_con_server_example,
  "ecore_con_client_simple_example" : prep_ecore_con_client_simple_example,
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

  #meson changed behaviour from 0.49 to 0.50 so we need this:
  if os.path.isabs(example):
    example_dir = example
  else:
    example_dir = os.path.join(G.builddir, example)

  run = subprocess.Popen([example_dir] + args,
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

#meson changed behaviour from 0.49 to 0.50 so we need this:
def meson_fetch_filename(filename_object):
  if isinstance(filename_object, str):
    return filename_object
  else:
    return filename_object[0]


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
examples = [meson_fetch_filename(b["filename"]) for b in build_targets if "examples" in meson_fetch_filename(b["filename"]) and b["type"] == "executable"]
state = State(len(examples))
#simulate all examples in parallel with up to 5 runners
with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
  futures = [executor.submit(simulate_example, example) for example in examples]
  for future in concurrent.futures.as_completed(futures):
    example_run = future.result()
    state.add_run(example_run[0], example_run[1], example_run[2])
state.print_summary()
