TEMPLATE = app
QT += svg
CONFIG += warn_on release
macx {
	# Uncomment the following line to compile on PowerPC Macs
	# QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
	CONFIG += x86 ppc
}

qws {
	qtopia_project(qtopia app)
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
	src/new_game_dialog.h \
	src/path.h \
	src/scores.h \
	src/settings.h \
	src/solver.h \
	src/theme.h \
	src/window.h

SOURCES = src/board.cpp \
	src/cell.cpp \
	src/main.cpp \
	src/maze.cpp \
	src/new_game_dialog.cpp \
	src/path.cpp \
	src/scores.cpp \
	src/settings.cpp \
	src/solver.cpp \
	src/theme.cpp \
	src/window.cpp

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

	icon.files = icons/cutemaze.png
	desktop.files = icons/cutemaze.desktop

	qws {
		target.path = /bin/
		icon.path = /pics/cutemaze
		icon.hint = pics
		desktop.path =	/apps/Games
		desktop.hint = desktop
	} else {
		target.path = $$PREFIX/bin/
		icon.path = $$PREFIX/share/icons/hicolor/48x48/apps
		desktop.path = $$PREFIX/share/applications/
	}

	INSTALLS += target icon desktop
}
