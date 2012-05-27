#!/bin/bash

APP='CuteMaze'
BUNDLE="$APP.app"
VERSION=$(git rev-parse --short HEAD)

macdeployqt $BUNDLE -dmg -no-plugins
mv "$APP.dmg" "${APP}_$VERSION.dmg"
