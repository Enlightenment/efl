#!/bin/bash -e


if [ "x$1" = "xrelease-ready" ] ; then
  num_buildfiles_changed=0
  if [ -f ~/cachedir/last-distcheck.txt ] ; then
    prev_distcheck=$(cat ~/cachedir/last-distcheck.txt)
    if git show --oneline $prev_distcheck 2>&1 > /dev/null ; then
      num_buildfiles_changed=$(git diff --name-only $prev_distcheck HEAD|grep '\.am\|\.ac\|\.mk'|wc -l)
      manual_trigger=$(git log --grep 'cibuildme' $prev_distcheck..HEAD|wc -l)
    else
      manual_trigger=1
    fi
    if [ "$num_buildfiles_changed" != "0" -o "$manual_trigger" != "0" ]; then
      echo "Running distcheck: forced or build files changed"
      exit 0
    else
      echo "Skipping distcheck: not forced and no build files changed"
      rm -f ~/cachedir/last-distcheck.txt
      git rev-parse HEAD > ~/cachedir/last-distcheck.txt
      exit 1
    fi
  fi
fi

exit 1
