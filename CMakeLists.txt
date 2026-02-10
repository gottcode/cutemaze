# SPDX-FileCopyrightText: 2021 Graeme Gott <graeme@gottcode.org>
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

# Configure project
project(cutemaze VERSION 1.3.6 LANGUAGES CXX)

set(project_copyright "2007-2026 Graeme Gott")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools Svg Widgets)
include(GNUInstallDirs)

add_compile_definitions(
	QT_NO_KEYWORDS
	$<$<CONFIG:DEBUG>:QT_STRICT_ITERATORS>
	$<$<CONFIG:DEBUG>:QT_NO_NARROWING_CONVERSIONS_IN_CONNECT>
	$<$<CONFIG:DEBUG>:QT_DISABLE_DEPRECATED_BEFORE=0x061000>
)
add_compile_options(
	$<$<CONFIG:DEBUG>:-Wshadow>
)

# Version number
include(cmake/AddVersionCompileDefinition.cmake)
add_version_compile_definition(src/main.cpp VERSIONSTR)

# Create program
qt_add_executable(cutemaze
	# Headers
	src/board.h
	src/cell.h
	src/locale_dialog.h
	src/maze.h
	src/new_game_dialog.h
	src/path.h
	src/scores_dialog.h
	src/settings.h
	src/solver.h
	src/theme.h
	src/window.h
	# Sources
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
	# Resources
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

# Create translations
file(GLOB translations_SRCS translations/*.ts)
qt_add_translations(cutemaze
	TS_FILES ${translations_SRCS}
	QM_FILES_OUTPUT_VARIABLE translations_QM
	LUPDATE_OPTIONS -no-obsolete -locations none
)

# Optimize build
option(ENABLE_LINK_TIME_OPTIMIZATION "Enable link time optimization" OFF)
if(ENABLE_LINK_TIME_OPTIMIZATION)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if(result)
		set_target_properties(cutemaze PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

option(ENABLE_STRIP "Enable automatic stripping of builds" OFF)
if(ENABLE_STRIP)
	add_custom_command(TARGET cutemaze
		POST_BUILD
		COMMAND ${CMAKE_STRIP} $<TARGET_FILE:cutemaze>
	)
endif()

# Install
if(APPLE)
	set(datadir "../Resources")

	set_target_properties(cutemaze PROPERTIES
		OUTPUT_NAME CuteMaze
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/mac/Info.plist.in
	)

	include(cmake/BundleResources.cmake)
	bundle_data(cutemaze ${CMAKE_SOURCE_DIR}/icons/cutemaze.icns Resources)
	bundle_translations(cutemaze "${translations_QM}")
elseif(WIN32)
	set(datadir ".")

	# Use Qt6 macro until CMake provides something
	# https://bugreports.qt.io/browse/QTBUG-87618
	set_target_properties(cutemaze PROPERTIES
		OUTPUT_NAME CuteMaze
		WIN32_EXECUTABLE TRUE
		QT_TARGET_VERSION "${PROJECT_VERSION}"
		QT_TARGET_COMPANY_NAME "Graeme Gott"
		QT_TARGET_DESCRIPTION "Simple maze game"
		QT_TARGET_COPYRIGHT "\\xA9 ${project_copyright}"
		QT_TARGET_PRODUCT_NAME "CuteMaze"
		QT_TARGET_RC_ICONS ${CMAKE_SOURCE_DIR}/icons/cutemaze.ico
	)
	_qt_internal_generate_win32_rc_file(cutemaze)
else()
	file(RELATIVE_PATH datadir ${CMAKE_INSTALL_FULL_BINDIR} ${CMAKE_INSTALL_FULL_DATADIR}/cutemaze)

	target_sources(cutemaze PRIVATE icons/icon.qrc)

	install(TARGETS cutemaze RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES ${translations_QM} DESTINATION ${CMAKE_INSTALL_DATADIR}/cutemaze/translations)
	install(FILES doc/cutemaze.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6 COMPONENT doc)
	install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

	include(cmake/TranslateMetainfo.cmake)
	process_and_install_metainfo(PO_DIR ${CMAKE_SOURCE_DIR}/icons/po)
endif()

set_property(SOURCE src/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS CUTEMAZE_DATADIR="${datadir}")
