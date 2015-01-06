lessThan(QT_VERSION, 5.2) {
	error("CuteMaze requires Qt 5.2 or greater")
}

TEMPLATE = app widgets
QT += svg
CONFIG += warn_on c++11

# Allow in-tree builds
!win32 {
	MOC_DIR = build
	OBJECTS_DIR = build
	RCC_DIR = build
}

# Set program version
VERSION = 1.1.1
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

# Set program name
unix: !macx {
	TARGET = cutemaze
} else {
	TARGET = CuteMaze
}

# Specify program sources
HEADERS = src/board.h \
	src/cell.h \
	src/locale_dialog.h \
	src/maze.h \
	src/new_game_dialog.h \
	src/path.h \
	src/scores.h \
	src/settings.h \
	src/solver.h \
	src/theme.h \
	src/window.h

SOURCES = src/board.cpp \
	src/cell.cpp \
	src/locale_dialog.cpp \
	src/main.cpp \
	src/maze.cpp \
	src/new_game_dialog.cpp \
	src/path.cpp \
	src/scores.cpp \
	src/settings.cpp \
	src/solver.cpp \
	src/theme.cpp \
	src/window.cpp

# Allow for updating translations
TRANSLATIONS = $$files(translations/cutemaze_*.ts)

# Install program data
RESOURCES = icons/icons.qrc themes/theme.qrc preview/preview.qrc
macx {
	ICON = icons/cutemaze.icns
} else:win32 {
	RC_FILE = icons/icon.rc
} else:unix {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	isEmpty(BINDIR) {
		BINDIR = bin
	}

	target.path = $$PREFIX/$$BINDIR/

	icon.files = icons/cutemaze.png
	icon.path = $$PREFIX/share/icons/hicolor/48x48/apps

	desktop.files = icons/cutemaze.desktop
	desktop.path = $$PREFIX/share/applications/

	appdata.files = icons/cutemaze.appdata.xml
	appdata.path = $$PREFIX/share/appdata/

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/cutemaze/translations

	man.files = doc/cutemaze.6
	man.path = $$PREFIX/share/man/man6

	INSTALLS += target icon desktop appdata qm man
}
