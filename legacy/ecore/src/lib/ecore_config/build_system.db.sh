#!/bin/sh
DB=system.db
edb_ed $DB add /e/theme/name str "winter"
if [ $BROWSER ]; then
edb_ed $DB add /apps/web/browser str "$BROWSER"
else
edb_ed $DB add /apps/web/browser str "`which firefox 2>/dev/null || 
                                       which phoenix 2>/dev/null || 
                                       which mozilla 2>/dev/null || 
                                       which opera 2>/dev/null || 
                                       which konqueror 2>/dev/null || 
                                       which epiphany 2>/dev/null`"
fi
edb_ed $DB add /apps/web/email str "`which thunderbird 2>/dev/null || 
                                     which mozilla 2>/dev/null || 
                                     which kmail 2>/dev/null || 
                                     which sylpheed 2>/dev/null || 
                                     which evolution 2>/dev/null`"
