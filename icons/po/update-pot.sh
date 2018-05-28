#!/bin/sh


echo -n 'Preparing files...'
cd ..

rm -f cutemaze.desktop.in
cp cutemaze.desktop cutemaze.desktop.in
sed -e '/^Name\[/ d' \
	-e '/^GenericName\[/ d' \
	-e '/^Comment\[/ d' \
	-e '/^Icon/ d' \
	-e '/^Keywords/ d' \
	-i cutemaze.desktop.in

rm -f cutemaze.appdata.xml.in
cp cutemaze.appdata.xml cutemaze.appdata.xml.in
sed -e '/p xml:lang/ d' \
	-e '/summary xml:lang/ d' \
	-e '/name xml:lang/ d' \
	-e '/<developer_name>/ d' \
	-i cutemaze.appdata.xml.in

cd po
echo ' DONE'


echo -n 'Extracting messages...'
xgettext --from-code=UTF-8 --output=description.pot \
	--package-name='CuteMaze' --copyright-holder='Graeme Gott' \
	../*.in
sed 's/CHARSET/UTF-8/' -i description.pot
echo ' DONE'


echo -n 'Cleaning up...'
cd ..

rm -f cutemaze.desktop.in
rm -f cutemaze.appdata.xml.in

echo ' DONE'
