#!/bin/bash

APP='CuteMaze'
BUNDLE="$APP.app"
VERSION='1.1.0'

macdeployqt $BUNDLE -dmg -no-plugins
mv "$APP.dmg" "${APP}_$VERSION.dmg"
