#!/bin/sh
DB=system.db
edb_ed $DB add /ewl/theme/name str "default"
if [ $BROWSER ]; then
edb_ed $DB add /apps/web/browser str "$BROWSER"
else
edb_ed $DB add /apps/web/browser str "`which MozillaFirebird || which phoenix || which mozilla || which opera || which konqueror || which epiphany`"
fi
edb_ed $DB add /apps/web/email str "`which MozillaThunderbird || which mozilla || which kmail || which sylpheed || which evolution`"
