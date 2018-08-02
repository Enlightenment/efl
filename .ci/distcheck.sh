#!/bin/bash

. .ci/travis.sh

scrape_makefile_variable() {
  #try to scrape $1 from Makefile
  num=0
  var=
  while true ; do
    var="$(grep -A${num} -m1 -w $1 Makefile.am)"
    #continually increment number of lines scraped if backslash is detected
    if echo "$var" | tail -n1 | grep -q '\\' ; then
      num=$((++num))
    else
      #cut out line wrapping
      var=$(echo "$var" | tr -d '\n\\' | cut -d= -f2-)
      break
    fi
  done
  echo $var
}

#attempt to keep this as close to original makefile rule as possible
#for ease of future updating

set -e

travis_fold dist "make dist"
#create dist tarball
make dist
travis_endfold dist

#set distdir variable to current package string
distdir="$(grep '^PACKAGE_STRING' config.log|cut -d\' -f2|tr ' ' -)"
DIST_ARCHIVES=${distdir}.tar.xz
MAKE=make

#unpack dist tarball
xz -dc ${distdir}.tar.xz | tar -xf -

chmod -R a-w ${distdir}
chmod u+w ${distdir}
mkdir ${distdir}/_build ${distdir}/_build/sub ${distdir}/_inst
chmod a-w ${distdir}
test -d ${distdir}/_build

dc_install_base=`cd ${distdir}/_inst && pwd | sed -e 's,^[^:\\/]:[\\/],/,'`
dc_destdir="${TMPDIR-/tmp}/am-dc-XXXX"
am__cwd=`pwd`

AM_DISTCHECK_CONFIGURE_FLAGS="$(scrape_makefile_variable AM_DISTCHECK_CONFIGURE_FLAGS)"
AM_MAKEFLAGS="$(scrape_makefile_variable AM_MAKEFLAGS)"

travis_fold configure "configuring distcheck build"
cd ${distdir}/_build/sub
../../configure \
    ${AM_DISTCHECK_CONFIGURE_FLAGS} \
    ${DISTCHECK_CONFIGURE_FLAGS} \
    --srcdir=../.. --prefix="$dc_install_base"
travis_endfold configure
travis_fold make make
make ${AM_MAKEFLAGS}
travis_endfold make
travis_fold dvi dvi
make ${AM_MAKEFLAGS} dvi
travis_endfold dvi
travis_fold check-build check-build
make ${AM_MAKEFLAGS} check-build
travis_endfold check-build
travis_fold check-TESTS check-TESTS
set +e
for tries in 1 2 3 ; do
  make ${AM_MAKEFLAGS} -C src/ -j1 check-TESTS && break
  cat src/test-suite.log
  if [ $tries != 3 ] ; then echo "tests failed, trying again!" ; continue ; fi
  exit 1
done
travis_endfold check-TESTS
set -e
travis_fold make_install "make install"
make ${AM_MAKEFLAGS} install
travis_endfold make_install
travis_fold make_installcheck "make installcheck"
make ${AM_MAKEFLAGS} installcheck
travis_endfold make_installcheck
travis_fold make_uninstall "make uninstall"
make ${AM_MAKEFLAGS} uninstall
travis_endfold make_uninstall
travis_fold distuninstallcheck "make distuninstallcheck"
make ${AM_MAKEFLAGS} distuninstallcheck_dir="$dc_install_base" \
        distuninstallcheck
travis_endfold distuninstallcheck
chmod -R a-w "$dc_install_base"
       (cd ../.. && umask 077 && mktemp -d "$dc_destdir")
     travis_fold dist-install "make dist-install"
     make ${AM_MAKEFLAGS} DESTDIR="$dc_destdir" install
     travis_endfold dist-install
     travis_fold dist-uninstall "make dist-uninstall"
     make ${AM_MAKEFLAGS} DESTDIR="$dc_destdir" uninstall
     travis_endfold dist-uninstall
     travis_fold dist-distuninstallcheck "make dist-distuninstallcheck"
     make ${AM_MAKEFLAGS} DESTDIR="$dc_destdir" \
            distuninstallcheck_dir="$dc_destdir" distuninstallcheck \
      || { rm -rf "$dc_destdir"; exit 1; }
     travis_endfold dist-distuninstallcheck
rm -rf "$dc_destdir"
travis_fold dist-dist "make dist-dist"
make ${AM_MAKEFLAGS} dist
travis_endfold dist-dist
rm -rf ${DIST_ARCHIVES}
travis_fold dist-distcleancheck "make dist-distcleancheck"
make ${AM_MAKEFLAGS} distcleancheck
travis_endfold dist-distcleancheck
cd "$am__cwd"
#$(am__post_remove_distdir)
if test -d "${distdir}"; then
  find "${distdir}" -type d ! -perm -200 -exec chmod u+w {} ';' \
    && rm -rf "${distdir}" \
    || { rm -rf "${distdir}"; }; \
else :; fi
