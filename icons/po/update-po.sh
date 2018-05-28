#!/bin/sh


echo -n 'Preparing files...'
cd ..

rm -f cutemaze.desktop.in
cp cutemaze.desktop cutemaze.desktop.in
sed -e '/^Name\[/ d' \
	-e '/^GenericName\[/ d' \
	-e '/^Comment\[/ d' \
	-e 's/^Name/_Name/' \
	-e 's/^GenericName/_GenericName/' \
	-e 's/^Comment/_Comment/' \
	-i cutemaze.desktop.in

rm -f cutemaze.appdata.xml.in
cp cutemaze.appdata.xml cutemaze.appdata.xml.in
sed -e '/p xml:lang/ d' \
	-e '/summary xml:lang/ d' \
	-e '/name xml:lang/ d' \
	-e 's/<p>/<_p>/' \
	-e 's/<\/p>/<\/_p>/' \
	-e 's/<summary>/<_summary>/' \
	-e 's/<\/summary>/<\/_summary>/' \
	-e 's/<name>/<_name>/' \
	-e 's/<\/name>/<\/_name>/' \
	-i cutemaze.appdata.xml.in

cd po
echo ' DONE'


echo -n 'Updating translations...'
for POFILE in *.po;
do
	echo -n " $POFILE"
	msgmerge --quiet --update --backup=none $POFILE description.pot
done
echo ' DONE'


echo -n 'Merging translations...'
cd ..

intltool-merge --quiet --desktop-style po cutemaze.desktop.in cutemaze.desktop
rm -f cutemaze.desktop.in

intltool-merge --quiet --xml-style po cutemaze.appdata.xml.in cutemaze.appdata.xml
echo >> cutemaze.appdata.xml
rm -f cutemaze.appdata.xml.in

echo ' DONE'
