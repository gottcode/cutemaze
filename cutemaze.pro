TEMPLATE = app
QT += svg
CONFIG += warn_on release
macx {
	# Uncomment the following line to compile on PowerPC Macs
	# QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
	CONFIG += x86 ppc
}

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

unix: !macx {
	TARGET = cutemaze
} else {
	TARGET = CuteMaze
}

HEADERS = src/board.h \
	src/cell.h \
	src/maze.h \
	src/scores.h \
	src/settings.h \
	src/theme.h \
	src/window.h

SOURCES = src/board.cpp \
	src/cell.cpp \
	src/main.cpp \
	src/maze.cpp \
	src/scores.cpp \
	src/settings.cpp \
	src/theme.cpp \
	src/window.cpp

RESOURCES = themes/theme.qrc preview/preview.qrc
macx {
	ICON = icons/cutemaze.icns
}
win32 {
	RC_FILE = icons/icon.rc
}

unix: !macx {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	binary.path = $$PREFIX/bin/
	binary.files = cutemaze

	icon.path = $$PREFIX/share/icons/hicolor/48x48/apps
	icon.files = icons/cutemaze.png

	desktop.path = $$PREFIX/share/applications/
	desktop.files = icons/cutemaze.desktop

	INSTALLS += binary icon desktop
}
