/*
	SPDX-FileCopyrightText: 2007-2022 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "locale_dialog.h"
#include "scores_dialog.h"
#include "window.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	app.setApplicationName("CuteMaze");
	app.setApplicationVersion(VERSIONSTR);
	app.setApplicationDisplayName(Window::tr("CuteMaze"));
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
	app.setWindowIcon(QIcon::fromTheme("cutemaze", QIcon(":/cutemaze.png")));
	app.setDesktopFileName("cutemaze");
#endif

	// Find application data
	const QString appdir = app.applicationDirPath();
	const QString datadir = QDir::cleanPath(appdir + "/" + CUTEMAZE_DATADIR);

	// Handle portability
#ifdef Q_OS_MAC
	const QFileInfo portable(appdir + "/../../../Data");
#else
	const QFileInfo portable(appdir + "/Data");
#endif
	if (portable.exists() && portable.isWritable()) {
		QSettings::setDefaultFormat(QSettings::IniFormat);
		QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, portable.absoluteFilePath() + "/Settings");
	}

	// Load application language
	LocaleDialog::loadTranslator("cutemaze_", datadir);

	// Handle commandline
	QCommandLineParser parser;
	parser.setApplicationDescription(Window::tr("A top-down maze game"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.process(app);

	// Convert old scores to new format
	ScoresDialog::migrate();

	// Create main window
	Window window;
	window.show();

	return app.exec();
}
