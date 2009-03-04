/***********************************************************************
 *
 * Copyright (C) 2007-2009 Graeme Gott <graeme@gottcode.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "window.h"

#include "board.h"
#include "scores.h"
#include "settings.h"

#include <QAction>
#include <QDir>
#include <QIcon>
#include <QList>
#include <QPair>
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QTimer>

#if defined(QTOPIA_PHONE)
#include <QApplication>
#include <QSoftMenuBar>
#include <QMenu>
#else
#include <QMenuBar>
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(QTOPIA_PHONE)
#include <QToolBar>

namespace {
// ============================================================================

bool less(const QString& s1, const QString& s2)
{
	return s1.left(s1.indexOf('x')).toInt() < s2.left(s2.indexOf('x')).toInt();
}

// ============================================================================

QStringList findLocations()
{
	// Add XDG icon directories
	QString xdg;
	xdg = getenv("$XDG_DATA_HOME");
	if (xdg.isEmpty()) {
		xdg = QDir::homePath() + "/.local/share";
	}
	QStringList locations = xdg.split(':');
	xdg = getenv("$XDG_DATA_DIRS");
	if (xdg.isEmpty()) {
		xdg = "/usr/local/share:/usr/share";
	}
	locations += xdg.split(':');
	for (int i = 0; i < locations.size(); ++i) {
		locations[i] += "/icons/";
	}

	// Add KDE icon directories
	if (getenv("KDE_FULL_SESSION")) {
		QString prefix;
		QString process = getenv("KDE_SESSION_VERSION") == QLatin1String("4") ? "kde4-config" : "kde-config";
		QProcess kdeconfig;
		kdeconfig.start(process + " --prefix");
		if (kdeconfig.waitForFinished()) {
			prefix = QLatin1String(kdeconfig.readLine().trimmed());
		}
		if (prefix.isEmpty()) {
			prefix = QDir::homePath() + QLatin1String("/.kde");
		}
		locations.prepend(prefix + "/share/icons/");

		kdeconfig.start(process + " --localprefix");
		if (kdeconfig.waitForFinished()) {
			prefix = QLatin1String(kdeconfig.readLine().trimmed());
		}
		if (prefix.isEmpty()) {
			prefix = QLatin1String("/usr");
		}
		locations.prepend(prefix + "/share/icons/");
	}

	// Add legacy home icon directory
	locations.prepend(QDir::homePath() + "/.icons/");

	// Remove duplicated directories
	int pos;
	for (int i = 0; i < locations.count(); ++i) {
		const QString& value = locations.at(i);
		while ((pos = locations.indexOf(value, i + 1)) != -1) {
			locations.removeAt(pos);
		}
	}

	return locations;
}

// ============================================================================

QStringList findParentThemes(const QString& theme, const QStringList& locations)
{
	QStringList themes(theme);
	QString file;
	QStringList inherits;
	for (int i = 0; i < themes.count(); ++i) {
		foreach (const QString& dir, locations) {
			if (QFileInfo(dir + '/' + themes[i] + "/index.theme").exists()) {
				file = dir + '/' + themes[i] + "/index.theme";
				break;
			}
		}
		if (file.isEmpty()) {
			themes.removeAt(i);
			--i;
			continue;
		}
		QSettings ini(file, QSettings::IniFormat);
		inherits = ini.value("Icon Theme/Inherits").toStringList();
		foreach (const QString& parent, inherits) {
			if (parent == "hicolor" || themes.contains(parent)) {
				continue;
			}
			themes.append(parent);
		}
	}
	themes.append("hicolor");
	return themes;
}

// ============================================================================

bool findIcons(const QString& theme_path, QStringList& icons, QStringList& icon_names, int& found_icons, const QString& size)
{
	int count = icon_names.count();

	// Find list of sizes
	QStringList sizes = QDir(theme_path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	sizes = sizes.filter(QRegExp("\\d+x\\d+"));
	qSort(sizes.begin(), sizes.end(), &less);
	int pos = sizes.indexOf(size);
	if (pos > 0) {
		for (int i = pos - 1; i >= 0; --i) {
			sizes.append(sizes.takeAt(i));
		}
	}

	// Search through sizes for icons
	QFileInfo info;
	QString path, subpath, dir;
	QStringList dirs, child_dirs;
	foreach (const QString& size, sizes) {
		path = theme_path + '/' + size;
		dirs = QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		// Check each subdirectory of size
		for (int i = 0; i < dirs.count(); ++i) {
			dir = dirs.at(i);
			subpath = path + '/' + dir;
			for (int j = 0; j < count; ++j) {
				QString& lookup = icon_names[j];
				if (lookup.isEmpty()) {
					continue;
				}

				info.setFile(subpath + '/' + lookup);
				if (info.exists() && !info.isDir()) {
					icons[j] = info.canonicalFilePath();
					lookup.clear();
					found_icons++;
				}
			}

			// Done if the icons are found
			if (found_icons == count) {
				return true;
			}

			// Append any child directories
			child_dirs = QDir(subpath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
			foreach (const QString& child, child_dirs) {
				dirs.append(dir + '/' + child);
			}
		}
	}
	return false;
}

// ============================================================================

QList<QIcon> findNativeIcons(int& actual_size)
{
	// Find native theme
	QStringList lookup;
	QString theme = QSettings().value("Icon Theme").toString();
	QString size = QSettings().value("Icon Size", "22x22").toString();
	actual_size = size.section('x', 0, 0).toInt();
	if (theme.isEmpty()) {
		if (getenv("KDE_FULL_SESSION")) {
			size = "22x22";
			actual_size = 22;

			QString default_theme;
			if (getenv("KDE_SESSION_VERSION") == QLatin1String("4")) {
				default_theme = "oxygen";
				lookup = QStringList() << "document-new.png" << "media-playback-pause.png" << "games-highscores.png" << "configure.png" << "application-exit.png";
			} else {
				default_theme = "crystalsvg";
				lookup = QStringList() << "filenew.png" << "player_pause.png" << "spreadsheet.png" << "configure.png" << "exit.png";
			}

			// Modified from src/gui/styles/qplastiquestyle.cpp in Qt 4.3.4
			// Copyright (C) 1992-2008 Trolltech ASA.
			QProcess kreadconfig;
			kreadconfig.start(QString("kreadconfig --group Icons --key Theme --default %1").arg(default_theme));
			if (kreadconfig.waitForFinished())
				theme = QLatin1String(kreadconfig.readLine().trimmed());
		} else {
			size = "24x24";
			actual_size = 24;

			lookup = QStringList() << "document-new.png" << "player_pause.png" << "stock_scores.png" << "preferences-system.png" << "application-exit.png";

			// Copied from src/gui/styles/qcleanlooksstyle.cpp in Qt 4.3.4
			// Copyright (C) 1992-2008 Trolltech ASA.
			QProcess gconftool;
			gconftool.start(QLatin1String("gconftool-2 --get /desktop/gnome/interface/icon_theme"));
			if (gconftool.waitForStarted(2000) && gconftool.waitForFinished(2000))
				theme = QLatin1String(gconftool.readLine().trimmed());
			if (theme.isEmpty())
				theme = QLatin1String("gnome");
		}
	}

	// Lookup icons
	QStringList icons;
	for (int i = 0; i < lookup.count(); ++i) {
		icons.append("");
	}
	QStringList locations = findLocations();
	QStringList themes = findParentThemes(theme, locations);
	int found_icons = 0;
	foreach (const QString& current, themes) {
		foreach (const QString& location, locations) {
			if (findIcons(location + current, icons, lookup, found_icons, size)) {
				goto done;
			}
		}
	}

	// Return list of icons
	done:
	QList<QIcon> result;
	foreach (const QString& icon, icons) {
		result.append(QIcon(icon));
	}
	return result;
}

// ============================================================================
}
#endif

// ============================================================================

Window::Window()
{
	// Create game object
	m_board = new Board(this);
	setCentralWidget(m_board);
	m_board->setFocus();
#if defined(QTOPIA_PHONE)
	connect(qApp, SIGNAL(aboutToQuit()), m_board, SLOT(saveGame()));
#endif

	// Create settings window
	m_settings = new Settings(this);
	connect(m_settings, SIGNAL(settingsChanged()), m_board, SLOT(loadSettings()));

	// Create scores window
	m_scores = new Scores(this);
	connect(m_board, SIGNAL(finished(int, int, int, int)), m_scores, SLOT(addScore(int, int, int, int)));

	// Create actions
	initActions();
#if !defined(QTOPIA_PHONE)
	m_pause_action->setShortcut(tr("P"));
#endif
	m_pause_action->setCheckable(true);
	connect(m_pause_action, SIGNAL(toggled(bool)), m_board, SLOT(pauseGame(bool)));
	connect(m_board, SIGNAL(pauseAvailable(bool)), m_pause_action, SLOT(setEnabled(bool)));
	connect(m_board, SIGNAL(pauseChecked(bool)), m_pause_action, SLOT(setChecked(bool)));

	// Setup window
	setWindowTitle(tr("CuteMaze"));
#if !defined(QTOPIA_PHONE)
	resize(QSettings().value("Size", QSize(448, 448)).toSize());
#endif

	// Create auto-save timer
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), m_board, SLOT(saveGame()));
	timer->start(300000);
}

// ============================================================================

void Window::closeEvent(QCloseEvent* event)
{
	QSettings().setValue("Size", size());
	m_board->saveGame();
	QMainWindow::closeEvent(event);
}

// ============================================================================

void Window::initActions()
{
#if defined(QTOPIA_PHONE)
	QMenu* game_menu = QSoftMenuBar::menuFor(this);

	game_menu->addAction(tr("Quit Game"), qApp, SLOT(quit()));
	game_menu->addAction(tr("Settings"), m_settings, SLOT(exec()));
	game_menu->addAction(tr("High Scores"), m_scores, SLOT(exec()));
	m_pause_action = game_menu->addAction(tr("Pause Game"));
	game_menu->addAction(tr("New Game"), m_board, SLOT(newGame()));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	// Fetch icons for toolbar
	int actual_size = 0;
	QList<QIcon> icons = findNativeIcons(actual_size);

	// Create toolbar
	QAction* action;
	QToolBar* toolbar = new QToolBar(this);
	toolbar->setIconSize(QSize(actual_size,actual_size));
	toolbar->setFloatable(false);
	toolbar->setMovable(false);
	toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	action = toolbar->addAction(icons.at(0), tr("New"), m_board, SLOT(newGame()));
	action->setShortcut(tr("Ctrl+N"));
	m_pause_action = toolbar->addAction(icons.at(1), tr("Pause"));
	toolbar->addAction(icons.at(2), tr("Scores"), m_scores, SLOT(exec()));
	toolbar->addAction(icons.at(3), tr("Settings"), m_settings, SLOT(exec()));
	action = toolbar->addAction(icons.at(4), tr("Quit"), this, SLOT(close()));
	action->setShortcut(tr("Ctrl+Q"));
	addToolBar(toolbar);
	setContextMenuPolicy(Qt::NoContextMenu);
#else
	QMenu* game_menu = menuBar()->addMenu(tr("Game"));

	game_menu->addAction(tr("New"), m_board, SLOT(newGame()), tr("Ctrl+N"));
	m_pause_action = game_menu->addAction(tr("Pause"));
	game_menu->addAction(tr("Scores"), m_scores, SLOT(exec()));
	game_menu->addAction(tr("Settings"), m_settings, SLOT(exec()));
	game_menu->addAction(tr("Quit"), this, SLOT(close()), tr("Ctrl+Q"));
#endif
}

// ============================================================================
