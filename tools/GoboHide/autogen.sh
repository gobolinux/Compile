#!/bin/sh

autopoint -f
rm po/Makevars.template
rm po/Rules-quot
rm po/boldquot.sed
rm po/en@boldquot.header
rm po/en@quot.header
rm po/insert-header.sin
rm po/quot.sed

aclocal -I m4
autoconf
automake --add-missing --copy

cd po
for f in *.po; do
  if test -r "$f"; then
    lang=`echo $f | sed -e 's,\.po$,,'`
    msgfmt -c -o $lang.gmo $lang.po
  fi
done
cd ..
