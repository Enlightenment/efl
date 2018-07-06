
Pyolian: Eolian python bindings and a template-based Eolian generator
=====================================================================

If you need to generate something based on the Eolian database you are in the
right place! Pyolian is the python bindings for Eolian and provide a clean and
pythonic API to Eolian; as a bonus it also provide a template-based
generator to render custom template files, filling them with a full eolian
context available.

The template engine is really powerfull and provide all the functionalities you
would expect from a full blow template engine. It is comparable in syntax and
features to [Jinja2](http://jinja.pocoo.org/) or the Django template system. If
you are interested on the implementation detail this generator is bases on the
great [Pyratemp](https://www.simple-is-better.org/template/pyratemp.html).


Installation
============

There is nothing to install to use the bindings or the generator, everything is
included in the efl source tree and it is intended to work directly inside the
tree, usually at efl compilation time (before the install step).

The only requirement is that **the source tree must be already built** (not
installed) because pyolian search the eolian .so/.dll inside the source tree.

If you built the efl tree in a custom location (fe, you build out-of-tree) you
can tell pyolian where to find the built eolian .so files using the
`EOLIAN_SO_DIR` environment variable.


Command line usage
==================
The simplest way to use the generator is from the command line, using the
`src/scripts/pyolian/generator.py` command, the `--help` option state:

```
usage: generator.py [-h] [--output FILE] [--cls CLASS_NAME] [--ns NAMESPACE]
                    [--struct STRUCT_NAME] [--enum ENUM_NAME]
                    [--alias ALIAS_NAME]
                    template

Pyolian template based generator.

positional arguments:
  template              The template file to use. (REQUIRED)

optional arguments:
  -h, --help            show this help message and exit
  --output FILE, -o FILE
                        Where to write the rendered output. If not given will
                        print to stdout.
  --cls CLASS_NAME      The full name of the class to render, ex:
                        Efl.Loop.Timer
  --ns NAMESPACE        The namespace to render, ex: Efl.Loop
  --struct STRUCT_NAME  The name of the struct to render, ex:
                        Efl.Loop.Arguments
  --enum ENUM_NAME      The name of the enum to render, ex:
                        Efl.Loop.Handler.Flags
  --alias ALIAS_NAME    The name of the alias to render, ex: Efl.Font.Size
```

Basically you just need a template file, then call the generator with at least
the template file and one of the context options (cls, ns, struct, enum or alias)

Two example templates can be found in the `efl/src/scripts/pyolian` folder,
from that directory you can run:

```
./generator.py test_gen_class.template --cls Efl.Loop.Timer
```

This will rendere the template test_gen_class.template with Efl.Loop.Timer as
**cls** available in the template, you can of course pass any other class you
want.

As we did not pass the --output-file parameter the rendered text is printed
on standard out.


Python usage
============

To use pyolian from python code you need to import **eolian** and/or the
**Template** class from the pyolian directory. As this python package is not
installed you need a little hack to import the package, something like this
should work:

```
# Use pyolian from source (not installed)
pyolian_path = os.path.join(<efl_root_path>, 'src', 'scripts')
sys.path.insert(0, pyolian_path)
from pyolian import eolian
from pyolian.generator import Template
```

Now you can use the full eolian library defined in eolian.py (read the file
to see all available classes and their properties) or just the Template class
like this:

```
t = Template('test_gen_class.template')
t.render('output.txt', cls='Efl.Loop.Timer', verbose=True)
```


Template syntax
===============

A template is a normal text file, containing some special placeholders and
control-structures. there are 3 syntax blocks available:


Expressions
-----------

A template needs some kind of "programming-language" to access variables,
calculate things, format data, check conditions etc. inside the template. So,
you could invent and implement an own language therefore -- or you could use an
already existing and well designed language: Python.

The template engine uses embedded Python-expressions. An expression is
everything which evaluates to a value, e.g. variables, arithmetics,
comparisons, boolean expressions, function/method calls, list comprehensions
etc. And such Python expressions can be directly used in the template.

```
${expression}$
```

**expression** can be any expression (e.g. a variable-name, an arithmetic
calculation, a function-/macro-call etc.), and is evaluated when the template
is rendered. Whitespace after `${` and before `}$` is ignored.

Generic examples:

  * numbers, strings, lists, tuples, dictionaries, ...: 12.34, "hello", [1,2,3], ...
  * variable-access: var, mylist[i], mydict["key"], myclass.attr
  * function/method call: myfunc(...), "foobar".upper()
  * comparison: (i < 0  or  j > 1024)
  * arithmetics: 1+2

For details, please read the Python-documentation about Python expressions.

Examples (for a template rendered with cls='Efl.Loop.Timer'):

  * `${cls.full_name}$` => 'Efl.Loop.Timer'
  * `${cls.full_name.replace('.', '_').lower()}$` => 'efl_loop_timer'
  * `${cls.base_class.full_name if cls.base_class else None}$` => 'Efl.Loop.Consumer' or None
  * `${', '.join([i.full_name for i in cls.hierarchy])}$` => 'Efl.Loop.Consumer, Efl.Object'

note the incredible flexibility of **expression**, they are basically small
pieces of python code, so you can use quite all the python syntax in them.

Also note that the context of the template (**cls** in this example) is always
an instance of a class defined in eolian.py (eolian.Class in this example), you
must read this file to understand all the available classes with their
properties and methods.


Comments
--------

Comments can be used in a template, and they do not appear in the result. They
are especially useful for (a) documenting the template, (b) temporarily
disabling parts of the template or (c) suppressing whitespace.

  * `#!...!#` - single-line comment with start and end tag
  * `#!...`   - single-line-comment until end-of-line, incl. newline!

The second version also comments out the newline, and so can be used at the end
of a line to remove that newline in the result.

Comments can contain anything, but comments may not appear inside substitutions
or block-tags.


Blocks
------

The control structures, macros etc. have a special syntax, which consists of a
start tag (which is named according to the block), optional additional tags,
and an end-tag:

```
<!--(...)-->        #! start tag
   ..
   ..
<!--(...)-->        #! optional additional tags (e.g. for elif)
   ..
   ..
<!--(end)-->        #! end tag
```

All tags must stand on their own line, and no code (except a `#!...` comment) is
allowed after the tag.

All tags which belong to the same block **must have the same indent!** The
contents of the block does not need to be indented, although it might improve
the readability (e.g. in HTML) to indent the contents as well.

Nesting blocks is possible, but the tags of nested blocks must have a different
indent than the tags of the enclosing blocks. Note that you should either use
spaces or tabs for indentation. Since the template distinguishes between spaces
and tabs, if you mix spaces and tabs, two indentations might look the same
(e.g. 8 spaces or 1 tab) but still be different, which might lead to unexpected
errors.

There's also a single-line version of a block, which does not need to stand on
its own line, but can be inserted anywhere in the template. But note that this
version does not support nesting :

```
...<!--(...)-->...<!--(...)-->...<!--(end)-->...
```

if/elif/else
------------

```
<!--(if EXPR)-->
...
<!--(elif EXPR)-->
...
<!--(else)-->
...
<!--(end)-->
```

The `elif` and `else` branches are optional, and there can be any number of
`elif` branches.


for/else
--------

```
<!--(for VARS in EXPR)-->
...
<!--(else)-->
...
<!--(end)-->
```

VARS can be a single variable name (e.g. myvar) or a comma-separated list of
variable-names (e.g. i,val).

The `else` branch is optional, and is executed only if the for loop doesn't
iterate at all.


macro
-----

Macros are user-defined "sub-templates", and so can contain anything a template
itself can contain. They can have parameters and are normally used to
encapsulate parts of a template and to create user-defined "functions". Macros
can be used in expressions, just like a normal variable or function.

```
<!--(macro MACRONAME)-->
...
<!--(end)-->
```

Note that the last newline (before `<!--(end)-->`) is removed from the macro, so
that defining and using a macro does not add additional empty lines.

Usage in expressions:

  * `MACRONAME`
  * `MACRONAME(KEYWORD_ARGS)`

KEYWORD_ARGS can be any number of comma-separated name-value-pairs (name=value,
...), and these names then will be locally defined inside the macro, in
addition to those already defined for the whole template.


raw
---

```
<!--(raw)-->
...
<!--(end)-->
```

Everything inside a `raw` block is passed verbatim to the result.


include
-------

```
<!--(include)-->FILENAME<!--(end)-->
```

Include another template-file. Only a single filename (+whitespace) is allowed
inside of the block; if you want to include several files, use several
include-blocks.

Note that inclusion of other templates is only supported when loading the
template from a file. For simplicity and security, FILENAME may not contain a
path, and only files which are in the same directory as the template itself can
be included.



Template context
================

The following Python-built-in values/functions are available by default in the
template:

 * `True`
 * `False`
 * `None`
 * `abs()`
 * `chr()`
 * `divmod()`
 * `hash()`
 * `hex()`
 * `isinstance()`
 * `len()`
 * `max()`
 * `min()`
 * `oct()`
 * `ord()`
 * `pow()`
 * `range()`
 * `round()`
 * `sorted()`
 * `sum()`
 * `unichr()`
 * `zip()`
 * `bool()`
 * `bytes()`
 * `complex()`
 * `dict()`
 * `enumerate()`
 * `float()`
 * `int()`
 * `list()`
 * `long()`
 * `reversed()`
 * `set()`
 * `str()`
 * `tuple()`
 * `unicode()`
 * `dir()`

Additionally, the functions exists(), default(), setvar() and escape() are
defined as follows:

 * `exists("varname")`  Test if a variable (or any other object) with the given name exists
 * `default("expr", default=None)` Tries to evaluate the expression expr.
 If the evaluation succeeds and the result is not None, its value is returned;
 otherwise, if the expression contains undefined variables/attributes, the
 default-value is returned instead. Note that expr has to be quoted.
 * `setvar("name", "expr")` Although there is often a more elegant way,
 sometimes it is useful or necessary to set variables in the template. 
 Can also be used to capture the output of e.g. an evaluated macro.

Moreover all the Eolian classes and enums (as defined in eolian.py) are available
in the template, fe:

 * `Function` eolian.Function (class)
 * `Eolian_Class_Type` eolian.Eolian_Class_Type (enum)

And some other general utilities:

 * `template_file` name of the template used to generate the output
 * `date` python datetime object (generation time)


Where to find more info
=======================

 * read the eolian.py file (it declare the full eolian API)
 * read the generator.py file (it's super simple)
 * read the original [pyratemp docs](https://www.simple-is-better.org/template/pyratemp.html)


Note
====

This markdown file is mirrored in efl src tree (src/scripts/pyolian) and in
phab wiki (phab.enlightenment.org/w/pyolian). Don't forget to update the other
if you change one!
