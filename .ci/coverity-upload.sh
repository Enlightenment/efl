#!/bin/sh

curl --form token=$COVERITY_SCAN_TOKEN \
     --form email=stefan@datenfreihafen.org \
     --form file=@efl-$(git rev-parse --short HEAD).xz \
     --form version=$(git rev-parse --short HEAD) \
     --form description="Submission from Travis CI" \
     https://scan.coverity.com/builds?project=Enlightenment+Foundation+Libraries
