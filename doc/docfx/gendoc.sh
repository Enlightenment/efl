#!/bin/bash

# This script invokes the locally-installed DocFX to produce the reference
# documentation. Run it every time EO documentation changes.

# DocFX Step 1: Generate metadata (*.yml) files
mono bin/docfx.exe metadata docfx.json

# Process the generated metadata files:
# DocFX uses short names for all <see> links which is inconvenient for us
# because we have multiple types with the same name in different namespaces
# ("Object" can be "Efl.Object" or "Efl.Canvas.Object").
# MarkDown files can add "?displayProperty=fullName" to crefs to show them
# fully-qualified, but this is stripped out from triple-slash comments in C#
# source files.
#
# This script adds the displayProperty suffix to all cref links from the
# metadata yml files (by that time links have been turned into <xref href="">
# tags).
for f in `ls api/*.yml`; do
  sed -e 's/\(<xref href="[^"]*\)"/\1?displayProperty=fullName"/g' -i $f
  sed -e 's/\(<xref href=\\"[^\\]*\)\\"/\1?displayProperty=fullName\\"/g' -i $f
done;

# DocFX Step 2: Generate HTML files
mono bin/docfx.exe build docfx.json && echo "Docs ready in the _site folder!"
