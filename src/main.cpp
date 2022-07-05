/*
	SPDX-FileCopyrightText: 2007-2022 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "locale_dialog.h"
#include "scores_dialog.h"
#include "window.h"

#include <QApplication>
#include <QCommandLineParser>

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

	const QString appdir = app.applicationDirPath();
	const QStringList datadirs{
#if defined(Q_OS_MAC)
		appdir + "/../Resources"
#elif defined(Q_OS_UNIX)
		DATADIR,
		appdir + "/../share/cutemaze"
#else
		appdir
#endif
	};

	LocaleDialog::loadTranslator("cutemaze_", datadirs);

	ScoresDialog::migrate();

	QCommandLineParser parser;
	parser.setApplicationDescription(Window::tr("A top-down maze game"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.process(app);

	Window window;
	window.show();
	return app.exec();
}
