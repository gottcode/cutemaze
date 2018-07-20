lessThan(QT_MAJOR_VERSION, 5) {
	error("CuteMaze requires Qt 5.2 or greater")
}
equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 2) {
	error("CuteMaze requires Qt 5.2 or greater")
}

TEMPLATE = app
QT += svg widgets
CONFIG += c++11

CONFIG(debug, debug|release) {
	CONFIG += warn_on
	DEFINES += QT_DEPRECATED_WARNINGS
	DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051100
	DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
}

# Allow in-tree builds
!win32 {
	MOC_DIR = build
	OBJECTS_DIR = build
	RCC_DIR = build
}

# Set program version
VERSION = 1.2.4
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

# Generate translations
TRANSLATIONS = $$files(translations/cutemaze_*.ts)
qtPrepareTool(LRELEASE, lrelease)
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

# Install program data
RESOURCES = icons/icons.qrc themes/theme.qrc preview/preview.qrc
macx {
	ICON = icons/cutemaze.icns
} else:win32 {
	RC_FILE = icons/icon.rc
} else:unix {
	RESOURCES += icons/icon.qrc

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	isEmpty(BINDIR) {
		BINDIR = bin
	}

	target.path = $$PREFIX/$$BINDIR/

	pixmap.files = icons/cutemaze.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	icon.files = icons/hicolor/*
	icon.path = $$PREFIX/share/icons/hicolor/

	desktop.files = icons/cutemaze.desktop
	desktop.path = $$PREFIX/share/applications/

	appdata.files = icons/cutemaze.appdata.xml
	appdata.path = $$PREFIX/share/metainfo/

	qm.files = $$replace(TRANSLATIONS, .ts, .qm)
	qm.path = $$PREFIX/share/cutemaze/translations
	qm.CONFIG += no_check_exist

	man.files = doc/cutemaze.6
	man.path = $$PREFIX/share/man/man6

	INSTALLS += target pixmap icon desktop appdata qm man
}
