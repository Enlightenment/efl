# AGENTS.md

Guidance for AI coding agents, and for the people driving them, who
contribute to the EFL. The aim is simple: a contribution made with an AI
tool must be as easy to review as a careful human one.

The human submitting the change is accountable for every line of it. If you
cannot explain why a line is there, it does not belong in the patch.

## Project overview

The EFL (Enlightenment Foundation Libraries) is a set of C libraries that
sit under Enlightenment, Terminology and other applications. Most of what
it exports is public ABI that distributions ship and programs link against
for years. Breaking it, or growing it carelessly, is the most expensive
mistake you can make here.

The code lives in `src/`:

* `src/lib/<lib>/`: one directory per library (`eina`, `eo`, `ecore`,
  `evas`, `ector`, `edje`, `elementary`, ...)
* `src/modules/<lib>/`: loadable modules: render engines, image loaders
  and savers, ecore_evas backends
* `src/bin/<lib>/`: tools such as `edje_cc`, `eolian_gen`, `elementary_test`
* `src/tests/<lib>/`: the check based test suites
* `src/benchmarks/<lib>/`: benchmarks run by `meson test --benchmark`
* `src/examples/<lib>/`: example programs, built with `-Dbuild-examples=true`
* `data/`: themes (`data/elementary/themes/`), fonts and other resources

Header conventions:

* `Lib.h`, `Lib_Common.h`, `Lib_Legacy.h`: installed, public API
* `*_Internal.h`: shared between EFL libraries, not installed for
  applications
* `*_private.h`: private to one library

Classes are described in `.eo` files and compiled by `eolian_gen` into
`.eo.h`, `.eo.c` and `.eo.legacy.h` files in the build directory. Change
the `.eo` file, never the generated output. An `.eo` class or method marked
`@beta` can still change; anything else is stable API.

## Build and test

```sh
meson setup build
ninja -C build
meson test -C build --print-errorlogs
```

`meson setup` enables the tests by default (`-Dbuild-tests=true`); they need
`check`. Look at `meson_options.txt` for the backends (`-Dwl`, `-Ddrm`,
`-Dopengl`, `-Dx11`) your change touches, and build with them enabled: code
you did not compile is code you did not test.

Building one target is faster while iterating:

```sh
ninja -C build src/lib/evas/libevas.so
ninja -C build src/tests/evas/evas_suite
```

To run one suite, or one test case inside it, from the build tree:

```sh
EFL_RUN_IN_TREE=1 build/src/tests/evas/evas_suite
CK_RUN_CASE="Object Textgrid" EFL_RUN_IN_TREE=1 build/src/tests/evas/evas_suite
```

`EFL_RUN_IN_TREE=1` makes the libraries load their modules and data from
the build tree rather than from an installed EFL. Without it you may be
testing whatever is installed in `/usr`. Add `CK_FORK=no` to run a test in
a debugger or under valgrind.

Read ninja's own final line, not a `grep` of its log: a failed build hidden
behind a pipe leaves a stale library that the tests will happily load.

## Tests are mandatory

Before you claim a change is done, **build it and run the test suites of
every library it touches, and of the libraries built on top of them. Every
test must pass.** Report failures as they are. Do not hide them, skip them,
or weaken a test to make it pass. If a test already failed before your
change, say so and show it fails the same way without your change.

Every bug fix comes with a test that fails without the fix. Every new
feature comes with tests that cover it, edge cases included. A regression
test is there to make sure the same bug never comes back.

### Writing a test

Tests use `check` through the helpers in `src/tests/efl_check.h`:

```c
EFL_START_TEST(evas_textgrid_test_truecolor_setget)
{
   ...
   ck_assert_int_eq(r, 255);
}
EFL_END_TEST

void
evas_test_textgrid(TCase *tc)
{
   tcase_add_test(tc, evas_textgrid_test_truecolor_setget);
}
```

* Put it in the existing `src/tests/<lib>/<lib>_test_<area>.c` file when
  there is one. A new file is added to that directory's `meson.build`, its
  entry function declared in `<lib>_suite.h` and registered in the
  `Efl_Test_Case` table of `<lib>_suite.c`.
* A rendering test draws into a buffer canvas and compares pixels; it does
  not need a display. See `src/tests/evas/evas_buffer_helper.c`.
* Test the behaviour, not the implementation: a test that has to be
  rewritten every time the code is refactored protects nothing.

Changes to memory handling or object lifetime are also run under valgrind,
and must not add errors or leaks:

```sh
CK_FORK=no EFL_RUN_IN_TREE=1 valgrind --error-exitcode=99 \
   build/src/tests/<lib>/<lib>_suite
```

## Performance changes need numbers

A change made for performance must prove it. Either an existing benchmark
shows the improvement, or the change adds a benchmark that does. "It should
be faster" is not enough.

```sh
ninja -C build
meson test -C build --benchmark -v
```

The benchmarks live in `src/benchmarks/` (`eina_bench`, `eo_bench`,
`evas_bench`). Rendering performance is usually measured with `expedite`,
which lives in its own repository. If the code you optimize is not covered,
add a benchmark that exercises it.

* Measure before and after on the same machine and build type
  (`--buildtype=release`), with enough iterations to rise above the noise.
  Numbers drift between runs and between days: rerun the baseline rather
  than reusing an old one.
* Put the before and after numbers in the commit message, with the
  benchmark used.
* Show that no other workload got slower, and that the tests still pass: a
  faster wrong answer is a regression. For rendering changes, compare the
  output pixel for pixel before and after.
* Complexity that only buys a gain within the noise will be rejected.

## API and ABI

Every symbol marked `EAPI`, or with a per-library `<LIB>_API` macro
(`EVAS_API`, `ECTOR_API`, ...), is exported and becomes ABI the moment it
is released.

* Export a symbol only when another library or an application really uses
  it. A helper used inside one library stays `static`, or is declared in
  that library's `*_private.h` without an export macro.
* Exported names carry the library prefix: `evas_`, `ecore_evas_`, `Evas_`.
  An unprefixed exported name pollutes every program that links the EFL.
* Never change the signature, the meaning or the struct layout behind an
  existing stable API. Add a new function instead.
* New public API needs documentation in its header or `.eo` file, a test,
  and a discussion with the maintainers before you write it.
* Installed headers use plain C types (`unsigned int`, `Eina_Bool`), as the
  surrounding public API does. Private code may use `uint32_t` and friends
  where the exact width matters (pixel data, packed buffers, hashes).

## Code that explains itself

The code is the *how*. It must be readable on its own, through the names of
its functions, types and variables and through its structure, not through
comments that paraphrase it.

### Naming

The further a name travels from its declaration, the more it has to say.

* A local used over a few lines can be short: `i`, `x`, `l`.
* A static function, or a variable that spans a long function, needs a
  descriptive name and a leading underscore for functions:
  `_evas_render_phase1_process()`, `_generic_cache_budget()`.
* Anything visible from another file (functions, types, struct fields,
  globals) must be meaningful without reading its implementation, and
  carries its library prefix.
* Types are named through a typedef, `Capitalized_With_Underscores`:
  `typedef struct _Evas_Object_Textgrid Evas_Object_Textgrid;`. Do not use
  a bare `struct _Name` as a type.

Short names follow the conventions already used everywhere in the code:

| Name          | Meaning                                                  |
|---------------|----------------------------------------------------------|
| `i`, `j`, `k` | numeric loop counters                                    |
| `l`, `ll`     | an `Eina_List` node while walking a list                 |
| `it`          | an `Eina_Iterator`                                       |
| `x`, `y`      | a position                                               |
| `w`, `h`      | a width and a height; never `width`, `height`            |
| `r`, `g`, `b`, `a` | color components                                    |
| `e`, `evas`   | the `Evas *` canvas                                      |
| `ee`          | the `Ecore_Evas *`                                       |
| `obj`         | the `Eo *` being worked on                               |
| `pd`          | the private data of an Eo class                          |
| `sd`          | the private data of a smart object or widget             |

When several objects of the same kind are in scope, name them by their role
(`src`, `dst`, `old_w`), not `a`, `b`, `tmp2`.

Keep one numbering per concept. Do not introduce a second enum or set of
defines that mirrors an existing one with different values; reuse the enum,
and where a header cannot see it, mirror it with defines and add a compile
time check where both are visible.

### Comments

Write comments for:

* **A function's contract**, when its name and signature are not enough:
  what it expects, what it returns, who owns what. Public API is always
  documented, in its header or `.eo` file.
* **Quirks that need outside knowledge**: a driver or protocol bug being
  worked around, a spec corner case, a compatibility choice, a non-obvious
  performance trade-off. Link to the source material whenever one exists.

Do not write comments that:

* restate what the next line does (`/* increment i */`)
* narrate the change (`/* now uses the new helper */`, `/* fixed */`)
  -- that belongs in the commit message
* section off trivial code, or add banners, or document every parameter
  of a static helper whose names already say it all

If code needs a comment to be understood, first try a better name, a smaller
function or a clearer structure.

Code, comments and commit messages are **ASCII only**: `-` not an em dash,
`->` not an arrow, `x` not a multiplication sign.

### Style

Look at the surrounding code and match it exactly. The EFL style is:

* 3 space indentation; braces on their own line, indented by 2; the body
  then sits 5 columns in. The return type of a function definition goes on
  its own line.
* One statement per line. No block on a single line: `if (x) { a; b; }`
  becomes a real block. A loop body is always a `{ }` block.
* Multi-line calls and prototypes: first argument right after `(`, the rest
  aligned under it. Never break right after `(`. Wrap lines over about 100
  columns.
* One blank line between blocks, never two in a row.
* Parenthesize mixed-precedence expressions:
  `if ((a == b) || (c == d))`, `x = (a * b) + c`, `(a > b) ? a : b`.
  A chain of the same operator, or a plain `x = a + b`, stays as is.
* `!strcmp(a, b)` and `!memcmp(...)`, not `strcmp(a, b) == 0`.
* Return early instead of wrapping a whole function body in an `if`.
  Declarations stay at the top of their block.

Use the EFL facilities rather than reinventing them:

* `EINA_LIST_FOREACH`, `EINA_LIST_FOREACH_SAFE` when the body removes the
  node, `EINA_LIST_FREE` to empty a list. The first element's data is
  `eina_list_data_get(list)`, never `eina_list_nth(list, 0)`.
* `Eina_Bool`, `Eina_Stringshare`, `Eina_Hash`, `Eina_Inarray`, `Eina_Rectangle`.
* `EINA_SAFETY_ON_NULL_RETURN(_VAL)` to check arguments of public API.
* The library's log domain (`ERR()`, `WRN()`, `DBG()`), never `printf`.

Every struct you add is laid out for memory:

1. Fields ordered by size: pointers and 8 byte members first, then 4, 2
   and 1 byte members.
2. The smallest type that safely holds the value: enum-like fields and
   small counters in `unsigned char`, counts bounded by a constant in
   `unsigned short`.
3. Fields that only hold 0 or 1 become `Eina_Bool name : 1;` at the end.

Leave alone structs whose layout is a contract: public structs, anything
uploaded to the GPU, written to disk, hashed or copied as raw bytes.

### Keep the diff minimal

* Only touch what your change needs. No drive-by reformatting, reordering
  or renaming of existing code, even inside a function you modified and
  even when it breaks the rules above. Mention it instead; a cleanup is its
  own commit.
* Do not add dead code: no unreachable fallbacks, no magic constants
  guarding a value that is always set. If a case cannot happen, do not
  handle it silently; if it can, handle it visibly.
* The EFL draws every frame and parses every theme: think about
  allocations and per-pixel, per-object and per-frame work in rendering,
  layout and event paths.

## Commits

One commit is one logical change. It builds and passes the tests on its own
so that `git bisect` keeps working. A test may come in its own commit, as in
`evas:tests: add textgrid tests, including true color`.

### Subject

`<lib>[:<area>]: <what changed>`, lowercase, imperative mood, no trailing
period, around 70 characters at most:

```
evas:textgrid: add per-cell true color buffer and render support
ecore_evas: request a tick when changes wait for one
ector: only export the symbol evas actually needs
evas:benchmarks: wire up evas_bench, add a textgrid bench
```

You will also find `evas - textblock - fix ...` in the history. Either form
is fine; follow the one used in the recent log of the area you touch
(`git log --oneline -- src/lib/<lib>`).

### Body: the *why*, not the *how*

The diff already shows how. The body explains what the reader cannot get
from it:

* why the change is needed: the bug, its symptom, who hits it
* why this approach over the obvious alternatives
* the spec, protocol or bug report that decides the behaviour, with a link
* anything surprising a reviewer should know: an ABI or behaviour change, a
  corner case deliberately left out, before and after benchmark numbers

Do not list the functions you edited, walk through the diff, or paste test
output. Wrap at 72 columns. A trivial change may have no body at all.

Good:

```
ecore_evas: request a tick when changes wait for one

With render sync, a canvas with changes but no tick of its own skips
rendering until its next tick. When the engine was already ticking,
nothing asks for that tick: the frame request that kicks the ticks
is only made when ticking starts, or after a render.

This happens when a tick from another source runs animators that
change the canvas, while an animator keeps the engine ticking: on
Wayland, the window freezes for good. Terminology hits it at startup
with its cursor animation.
```

Bad:

```
ecore_evas: fix rendering

- Added _ecore_evas_tick_request() function
- Modified _ecore_evas_idle_enter() to call it
- All tests pass
```

### AI disclosure

Commits written with the help of an AI tool say so with a trailer naming the
tool, for example:

```
Co-Authored-By: <Tool and model name> <noreply@example.com>
```

## Review

* Keep a change focused on one topic. API additions, new dependencies and
  design changes are discussed with the maintainers first.
* In the description, explain the problem and how the change was tested:
  which suites, which backends, which benchmark.
* Answer review comments yourself, in your own words. Reviewers are
  discussing with you, not with your tool. Do not just re-run the prompt
  and push whatever comes out.
* Apply a review comment to the code your change added, not to the code
  around it that was already there, unless the reviewer asks for that.

## Do not

* submit code you have not built and tested
* claim tests pass without running them
* break or extend the stable API or ABI without discussing it first
* add dependencies without discussing it first
* edit generated files (`*.eo.h`, `*.eo.c`, `*.eo.legacy.h`, `.edj`) by
  hand; change their source and rebuild
* commit build artifacts, profiling output, benchmark logs or screenshots
