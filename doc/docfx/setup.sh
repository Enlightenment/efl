#!/bin/bash
#
# This script installs DocFX and brings all content pages from EFL's documentation
# content repository, transforming them from DokuWiki's format to DocFX's.
# This step only needs to be done once (or every time the content pages change).
# After this step, you can run the gendoc.sh script as many times as you want,
# to generate the actual HTML pages, including the reference guide extracted from
# libefl_mono.dll and libefl_mono.xml
#
# This script could be improved a lot, but it gets the job done.
# Xavi (xartigas@yahoo.es)

#
# Check mono version since DocFX has been known to fail with mono 4.
#
mono_version="$(mono --version | awk '/version/ { print $5 }')"
if [ $mono_version \< 5 ];
then
  echo "Requires at least mono version 5, detected $mono_version"
  exit 1
fi;

#
# Download and extract DocFX
#
if [ ! -d "bin" ]; then
  rm -rf docfx.zip bin
  wget https://github.com/dotnet/docfx/releases/download/v2.40.4/docfx.zip
  unzip -q docfx.zip -d bin
  rm docfx.zip
else
  echo "Skipping DocFX download because 'bin' folder already exists."
fi

#
# Clone whole Content site
#
rm -rf www-content
git clone --depth 1 git+ssh://git@git.enlightenment.org/website/www-content.git www-content

#
# Copy all pages related to C# (those inside a folder called 'csharp') to the articles folder
#
rm -rf articles
mkdir articles
# Copy them
find www-content -wholename "*/csharp/*.md.txt" -exec cp --parents {} articles \;
# Except tutorials and guides summary pages (start.md)
find articles/www-content/pages/develop/tutorials -name "start.md.txt" -exec rm {} \;
find articles/www-content/pages/develop/guides -name "start.md.txt" -exec rm {} \;
# Remove the trailing .txt from filenames (DocFX wants only the .md)
for f in `find articles -name "*.md.txt"`; do mv $f $(echo $f | rev | cut -c5- | rev) ; done
# Copy all media files to the images folder
rm -rf images
mkdir images
cp -r www-content/media/* images
# Remove git clone now that we have everything we wanted
rm -rf www-content

#
# Parse all Markdown files to adapt them to DocFX
#
# Remove special Dokuwiki headers like ~~NOCACHE~~ because DocFX complains about them.
find articles -name "*.md" | xargs sed -i -e "s/^~~.*~~$//g"
# Transform media links in articles from /_media/* to ~/images/*
find articles -name "*.md" | xargs sed -i -e "s#/_media#~/images#g"
# Transform API root links from /develop/api/ to @api-reference-root (the UID of the API root)
find articles -name "*.md" | xargs sed -i -e "s#\(\[[^]]*\](\)/develop/api/)#\1xref:api-reference-root)#g"
# Transform API class links from */api/namespace/class to @Namespace.Class (that should be the UID of the reference page)
find articles -name "*.md" | xargs sed -i -e "s#\(\[[^]]*\](\)[^)]*/api/\([^/.]*\)/\([^/.]*\))#\1xref:\u\2.\u\3)#g"
# Point links to Legacy API to the www.enlightenment.org site
find articles -name "*.md" | xargs sed -i -e "s#\(\[[^]]*\](\)\(/develop/legacy/api/[^)]*)\)#\1https://www.enlightenment.org\2#g"
# Point the rest of links to /develop/api/* to the root the API, since we don't know how to transform them
find articles -name "*.md" | xargs sed -i -e "s#\(\[[^]]*\](\)/develop/api/[^)]*)#\1xref:api-reference-root)#g"
# Add start.md to links pointing to a folder inside ~/develop/
find articles -name "*.md" | xargs sed -i -e "s#\(\[[^]]*\](/develop/[^)]*/\))#\1start.md)#g"
# Transform absolute links from /develop/* to ~/articles/www-content/pages/develop/*
find articles -name "*.md" | xargs sed -i -e "s#\(\[[^]]*\]\)(\(/develop/[^)]*\))#\1(~/articles/www-content/pages\2)#g"
# Transform page anchor links from [label](#Anchor_Title) to [label](#anchor-title).
for f in `find articles -name "*.md"`; do
  awk -v RS=' ' '{ if ($0 ~ ".*\(#.*\)") { gsub( /_/, "-" ); printf "%s ", tolower($0) } else { printf "%s ",$0 } }' $f > /tmp/efl_docfx_setup.tmp
  mv /tmp/efl_docfx_setup.tmp $f
done

#
# Populate articles TOC file from the articles' titles
#
cd articles
rm -f toc.yml
echo "- name: Setup Guide" >> toc.yml
echo "  href: www-content/pages/develop/setup/csharp/start.md" >> toc.yml
echo "- name: Tutorials" >> toc.yml
echo "  items:" >> toc.yml
find www-content/pages/develop/tutorials -name "*.md" -exec sh -c "cat {} | grep '^# ' && echo '    href: {}'" \; >> toc.yml
echo "- name: Guides" >> toc.yml
echo "  items:" >> toc.yml
find www-content/pages/develop/guides -name "*.md" -exec sh -c "cat {} | grep '^# ' && echo '    href: {}'" \; >> toc.yml
sed -i -e "s/^# \(.*\) #/  - name: '\1'/g" toc.yml
sed -i -e "s/ in C#//g" toc.yml
cd ..
