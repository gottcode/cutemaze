lessThan(QT_VERSION, 4.6) {
	error("FocusWriter requires Qt 4.6 or greater")
}

TEMPLATE = app
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}
QT += svg
CONFIG += warn_on

VERSION = 1.1.1
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

unix: !macx {
	TARGET = cutemaze
} else {
	TARGET = CuteMaze
}

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

TRANSLATIONS = $$files(translations/cutemaze_*.ts)

RESOURCES = icons/icons.qrc themes/theme.qrc preview/preview.qrc
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
	isEmpty(BINDIR) {
		BINDIR = bin
	}

	target.path = $$PREFIX/$$BINDIR/

	icon.files = icons/cutemaze.png
	icon.path = $$PREFIX/share/icons/hicolor/48x48/apps

	desktop.files = icons/cutemaze.desktop
	desktop.path = $$PREFIX/share/applications/

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/cutemaze/translations

	INSTALLS += target icon desktop qm
}
