#!/bin/bash
# Header hygiene checker for the unified (Efl.*) API.
#
# Installs the tree into a temporary destdir and then, against the *installed*
# headers only:
#
#   1. compiles every generated unified header (efl_*.eo.h / efl_*.eot.h) after
#      its library's public umbrella, with the beta API and every @protected
#      guard the headers themselves mention switched on -- the widest surface a
#      consumer can ask for. A failure means the header depends on something the
#      umbrella does not provide, which every consumer then has to work around by
#      hand.
#   2. greps the same headers for legacy Evas_/Elm_/Ecore_Evas tokens outside
#      comments. A unified header may not require a legacy type.
#
# Usage: scripts/check-headers.sh [build-dir]     (default: build)

set -u

BUILD_DIR="${1:-build}"
if [ ! -f "$BUILD_DIR/build.ninja" ]; then
   echo "usage: $0 [build-dir]   (no meson build found in '$BUILD_DIR')" >&2
   exit 2
fi

DESTDIR=$(mktemp -d)
trap 'rm -rf "$DESTDIR"' EXIT

echo "== installing into $DESTDIR"
if ! meson install -C "$BUILD_DIR" --destdir "$DESTDIR" --quiet > "$DESTDIR/install.log" 2>&1; then
   echo "install failed:" >&2
   tail -20 "$DESTDIR/install.log" >&2
   exit 2
fi

ROOT=$(dirname "$(find "$DESTDIR" -type d -name 'eina-1' | head -1)")
if [ -z "$ROOT" ] || [ ! -d "$ROOT" ]; then
   echo "could not locate the installed include root" >&2
   exit 2
fi

INC=""
for d in $(find "$ROOT" -maxdepth 2 -type d | grep -v -- '-cxx-'); do
   INC="$INC -I$d"
done

# every @protected guard the installed unified headers mention, so that
# protected declarations are exposed too
PROTECTED="$DESTDIR/protected.h"
grep -rhoE '\bEFL_[A-Z0-9_]+_PROTECTED\b' "$ROOT"/*/efl_*.eo.h 2>/dev/null |
   sort -u | sed 's/^/#define /; s/$/ 1/' > "$PROTECTED"
echo "== $(wc -l < "$PROTECTED") protected guards enabled"

# library directory -> public umbrella header to include first
umbrella_for()
{
   case "$1" in
      elementary-1) echo "Efl_Ui.h" ;;
      efl-1)        echo "Efl.h" ;;
      eo-1)         echo "Eo.h" ;;
      ecore-1)      echo "Efl_Core.h" ;;
      evas-1)       echo "Efl_Canvas.h" ;;
      edje-1)       echo "Efl_Layout.h" ;;
      ecore-con-1)  echo "Efl_Net.h" ;;
      eio-1)        echo "Eio.h" ;;
      emotion-1)    echo "Emotion.h" ;;
      *)            echo "" ;;
   esac
}

pass=0; fail=0; failed=""
for lib in efl-1 eo-1 ecore-1 evas-1 edje-1 ecore-con-1 elementary-1 eio-1 emotion-1; do
   [ -d "$ROOT/$lib" ] || continue
   umbrella=$(umbrella_for "$lib")
   [ -n "$umbrella" ] || continue
   for f in $(find "$ROOT/$lib" \( -name 'efl_*.eo.h' -o -name 'efl_*.eot.h' \) | sort); do
      base=$(basename "$f")
      tu="$DESTDIR/tu.c"
      {
         echo '#define EFL_BETA_API_SUPPORT 1'
         cat "$PROTECTED"
         echo "#include <$umbrella>"
         echo "#include <$base>"
         echo 'int main(void) { return 0; }'
      } > "$tu"
      if cc $INC -c "$tu" -o /dev/null 2> "$DESTDIR/err.log"; then
         pass=$((pass + 1))
      else
         fail=$((fail + 1))
         failed="$failed $lib/$base"
         echo "-- FAIL $lib/$base (after $umbrella)"
         grep -m3 'error:' "$DESTDIR/err.log" | sed 's/^/     /'
      fi
   done
done
echo "== self-contained-after-umbrella: $pass ok, $fail failed"

# legacy token leakage
echo "== legacy token scan"
leaks=0
for lib in efl-1 eo-1 ecore-1 evas-1 edje-1 ecore-con-1 elementary-1 eio-1; do
   [ -d "$ROOT/$lib" ] || continue
   for f in $(find "$ROOT/$lib" \( -name 'efl_*.eo.h' -o -name 'efl_*.eot.h' -o -name 'Efl*.h' \) | sort); do
      # -fpreprocessed keeps the directives and drops the comments without
      # expanding any include, so doc comments cannot produce false hits
      hits=$(cc -fpreprocessed -dD -E -P "$f" 2>/dev/null |
             grep -oE '\b(Evas_Object|Evas_Coord|Elm_[A-Za-z_]+|Ecore_Evas)\b' | sort -u | tr '\n' ' ')
      if [ -n "$hits" ]; then
         leaks=$((leaks + 1))
         echo "-- LEAK $lib/$(basename "$f"): $hits"
      fi
   done
done
echo "== legacy tokens: $leaks header(s) affected"

if [ "$fail" -ne 0 ] || [ "$leaks" -ne 0 ]; then
   echo "FAILED:$failed"
   exit 1
fi
echo "ALL CLEAN"
exit 0
