#!/bin/sh


echo -n 'Preparing files...'
cd ..

rm -f cutemaze.desktop.in.bak
cp cutemaze.desktop.in cutemaze.desktop.in.bak
sed -e '/^Icon/ d' -e '/^Keywords/ d' -i cutemaze.desktop.in

rm -f cutemaze.appdata.xml.in.bak
cp cutemaze.appdata.xml.in cutemaze.appdata.xml.in.bak
sed '/<developer_name>/ d' -i cutemaze.appdata.xml.in

cd po
echo ' DONE'


echo -n 'Extracting messages...'
xgettext --from-code=UTF-8 --output=description.pot \
	--package-name='CuteMaze' --copyright-holder='Graeme Gott' \
	../*.in
sed 's/CHARSET/UTF-8/' -i description.pot
echo ' DONE'


echo -n 'Updating translations...'
rm -f LINGUAS
for POFILE in *.po;
do
	echo -n " $POFILE"
	echo -n " ${POFILE%???}" >> LINGUAS
	msgmerge --quiet --update --backup=none $POFILE description.pot
done
echo >> LINGUAS
echo ' DONE'


echo -n 'Cleaning up...'
cd ..

rm -f cutemaze.desktop.in
mv cutemaze.desktop.in.bak cutemaze.desktop.in

rm -f cutemaze.appdata.xml.in
mv cutemaze.appdata.xml.in.bak cutemaze.appdata.xml.in

echo ' DONE'
