
Testgen: Template-based Eolian tests generator
===============================================================================

Testgen is a Python Script using the Pyolian to generate tests rendering
templates with custom files, this can be a easy way to expand the
API test coveraged.

Testgen can generate tests C and to other bingings languages only
adding new language specialized templates.

Installation
============

There is nothing to install to use the generator, everything is included in
the efl source tree and it is intended to work directly inside the tree,
usually at efl tests compilation time (make check).

The only requirement is that **the source tree must be already built** (not
installed) because pyolian search the eolian .so/.dll inside the source tree.

If you built the efl tree in a custom location (fe, you build out-of-tree) you
can tell pyolian where to find the built eolian .so files using the
`EOLIAN_SO_DIR` environment variable.


Command line usage
==================

The simplest way to use the generator is from the command line, using the
`src/scripts/testgen/testgen.py` command, the `--help` option state:

```
usage: testgen.py [-h] testname suitename filename [eofiles [eofiles ...]]

Eolian Test Generator.

positional arguments:
  testname    The Test Name used to find custom and template files. (REQUIRED)
  suitename   The Suite Name used to find custom files. (REQUIRED)
  filename    Generated test file destination. (REQUIRED)
  eofiles     The Eolian Files to use.

optional arguments:
  -h, --help  show this help message and exit
```
Use .c extension in <filename> to generate C tests or .cs to CSharp

To test this generator in `src/scripts/testgen` you can run:
```
./testgen.py automated efl efl_automated_test.c efl_loop.eo
```
This will rendere the automated tests using files in `src/tests/automated` with
suite name `efl_automated` and with Efl.Loop Class as Test Case 
`efl_automated_efl_loop_test`

or run:
```
./testgen.py automated eio eio_automated_test.c eio_sentry.eo eio_model.eo
```
This will rendere with suite name `eio_automated` and with Eio.Sentry and
Eio.Model Class as Test Cases `eio_automated_eio_sentry_test` and
`eio_automated_eio_model_test`


How customise a Generated Test
==============================

Testgen use the filesystem to find custom files if you need customise a test,
add/write follow files in src/tests:

 Suite custom files
 * `src/test/<testname>/`
     |-> <suitename>_custom.c   #add include files, functions or structs
     |-> <suitename>_init.c     #add code in SUITE_INIT 
     |-> <suitename>_shutdown.c #add code in SUITE_SHUTDOWN

 Class Test case custom files
 * `src/test/<testname>/<class_name>/`  #use lowercase and `_` separator
     |-> custom.c       #add include files, functions or structs
     |-> init.c         #add default way to create the object of this class
     |-> shutdown.c     #add default way to free the object

Funtions Tests
- Tests methodes custom files
 * `src/test/<testname>/<class_name>/<method_name>`
     |-> arg_init.c     #initialize method arguments (arg_<argument_name>)
     |-> init.c         #add how to create the object (replace default)
     |-> arg_shutdown.c #free arguments
     |-> shutdown.c     #add how to free the object (replace default)

- Tests properties custom files
 * `src/test/<testname>/<class_name>/<property_name>`
     |   -- Property Get --
     |-> arg_get_init.c     #initialize property get arguments (arg_<argument_name>)
     |-> get_init.c         #how to create the object (replace default)
     |-> arg_get_shutdown.c #free arguments
     |-> get_shutdown.c     #how to free the object (replace default)
     |   -- Property Set --
     |-> arg_set_init.c     #initialize propety set arguments (arg_<argument_name>)
     |-> set_init.c         #how to create the object (replace default)
     |-> arg_set_shutdown.c #free arguments
     |-> set_shutdown.c     #how to free the object (replace default)

to make some custom files you only need a code using:
 `parent`     -> default name of parent object defined as `Eo *`
 `obj`        -> default name of current object
 `arg_<name>` -> replace <name> with functions arguments name 

you can use custom.c (suite or class) to add specilized code, structs and callbacks

-- Use `*.cs` to Emono/CSharp generated code --

Some class or function test don't need a test in some Suite, you can disable test generated
of it with a blank file as following:

use lowercase and `_` as separator
`src/test/<testname>/<class_name>`  #don't generate test for <class_name>
`src/test/<testname>/<class_name>/method_name` #don't generate test for <method_name>
`src/test/<testname>/<class_name>/<property_name>` #don't generate test for this property
`src/test/<testname>/<class_name>/<property_name>_get` #don't generate test for this property get
`src/test/<testname>/<class_name>/<property_name>_set` #don't generate test for this property set


Where to find more info
=======================

 * read the Pyolian README file in EFL scripts
 * read the eolian.py file (it declare the full eolian API)
 * read the generator.py file (it's super simple)
 * read the original [pyratemp docs](https://www.simple-is-better.org/template/pyratemp.html)


Note
====

This markdown file is mirrored in efl src tree (src/scripts/pyolian) and in
phab wiki (phab.enlightenment.org/w/pyolian). Don't forget to update the other
if you change one!
