cmake_minimum_required(VERSION 3.16.0)

# Configure project
project(cutemaze VERSION 1.2.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools Svg Widgets)
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)
endif()

add_compile_definitions(
	QT_STRICT_ITERATORS
	QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
	QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

# Create translations
file(GLOB translations_SRCS translations/*.ts)
qt6_add_translation(translations_QM ${translations_SRCS})

add_custom_target(update_translations
	COMMAND Qt6::lupdate -no-obsolete -locations none src -ts ${translations_SRCS}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	VERBATIM
)

# Create program
qt6_add_executable(cutemaze
	src/board.cpp
	src/cell.cpp
	src/locale_dialog.cpp
	src/main.cpp
	src/maze.cpp
	src/new_game_dialog.cpp
	src/path.cpp
	src/scores_dialog.cpp
	src/settings.cpp
	src/solver.cpp
	src/theme.cpp
	src/window.cpp
	icons/icons.qrc
	themes/theme.qrc
	preview/preview.qrc
	${translations_QM}
)

target_link_libraries(cutemaze PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Svg
	Qt6::Widgets
)

target_compile_definitions(cutemaze PRIVATE VERSIONSTR="${PROJECT_VERSION}")

# Install
if(UNIX AND NOT APPLE)
	target_sources(cutemaze PRIVATE icons/icon.qrc)

	install(TARGETS cutemaze RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES ${translations_QM} DESTINATION ${CMAKE_INSTALL_DATADIR}/cutemaze/translations)
	install(FILES doc/cutemaze.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6 COMPONENT doc)
	install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
	install(FILES icons/cutemaze.xpm DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
	install(FILES icons/cutemaze.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
	install(FILES icons/cutemaze.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
elseif(WIN32)
	# Use Qt6 macro until CMake provides something
	# https://bugreports.qt.io/browse/QTBUG-87618
	set_target_properties(cutemaze PROPERTIES
		WIN32_EXECUTABLE TRUE
		QT_TARGET_VERSION "${PROJECT_VERSION}"
		QT_TARGET_COMPANY_NAME "Graeme Gott"
		QT_TARGET_DESCRIPTION "Simple maze game"
		QT_TARGET_COPYRIGHT "Copyright (C) 2021 Graeme Gott"
		QT_TARGET_PRODUCT_NAME "CuteMaze"
		QT_TARGET_RC_ICONS ${CMAKE_SOURCE_DIR}/icons/cutemaze.ico
	)
	_qt_internal_generate_win32_rc_file(cutemaze)
endif()
