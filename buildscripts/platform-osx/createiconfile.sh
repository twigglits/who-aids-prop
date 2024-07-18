#!/bin/bash

rm -f Icon$'\r'
rm -f icon.rsrc

cp SimpactIcon.icns customIcon.icns
sips -i customIcon.icns
DeRez -only icns customIcon.icns >icon.rsrc
Rez -append icon.rsrc -o Icon$'\r'


