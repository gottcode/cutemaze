/*
	SPDX-FileCopyrightText: 2007-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "locale_dialog.h"
#include "scores_dialog.h"
#include "window.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char** argv)
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#if !defined(Q_OS_MAC)
	if (!qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO")
			&& !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
			&& !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
			&& !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	}
#endif
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
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

	LocaleDialog::loadTranslator("cutemaze_");

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
