/***********************************************************************
 *
 * Copyright (C) 2007-2008 Graeme Gott <graeme@gottcode.org>
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
#include <QEvent>
#include <QIcon>
#include <QList>
#include <QMenuBar>
#include <QPair>
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QToolBar>

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
namespace {
// ============================================================================

bool less(const QString& s1, const QString& s2)
{
	return s1.left(s1.indexOf('x')).toInt() < s2.left(s2.indexOf('x')).toInt();
}

// ============================================================================

// Copied from src/gui/styles/qplastiquestyle.cpp in Qt 4.3.4
// Copyright (C) 1992-2008 Trolltech ASA.
QString kdeHome()
{
	QString home = QString::fromLocal8Bit(qgetenv("KDEHOME"));
	if (home.isEmpty())
		home = QDir::homePath() + QLatin1String("/.kde");
	return home;
}

// ============================================================================

QString kdeDir()
{
	QString dir = QString::fromLocal8Bit(qgetenv("KDEDIR"));
	if (dir.isEmpty()) {
		QProcess kdeconfig;
		kdeconfig.start(QLatin1String("kde-config --prefix"));
		if (kdeconfig.waitForFinished())
			dir = QLatin1String(kdeconfig.readLine().trimmed());
		if (dir.isEmpty())
			dir = QLatin1String("/usr");
	}
	return dir;
}

// ============================================================================

QStringList findLocations()
{
	QString xdg;
	xdg = getenv("$XDG_DATA_HOME");
	if (xdg.isEmpty()) {
		xdg = QDir::homePath() + "/.local/share/";
	}
	QStringList locations = xdg.split(":");
	xdg = getenv("$XDG_DATA_DIRS");
	if (xdg.isEmpty()) {
		xdg = "/usr/local/share/:/usr/share/";
	}
	locations += xdg.split(":");
	for (int i = 0; i < locations.size(); ++i) {
		locations[i] += "/icons/";
	}
	locations.prepend(kdeDir() + "/share/icons/");
	locations.prepend(kdeHome() + "/share/icons/");
	locations.prepend(QDir::homePath() + "/.icons/");
	return locations;
}

// ============================================================================

QStringList findParentThemes(const QString& theme, const QStringList& locations)
{
	QStringList themes(theme);
	QString file;
	QStringList inherits;
	for (int i = 0; i < themes.count(); ++i) {
		foreach (QString dir, locations) {
			if (QFileInfo(dir + "/" + themes[i] + "/index.theme").exists()) {
				file = dir + "/" + themes[i] + "/index.theme";
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
		foreach (QString parent, inherits) {
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

bool findIcons(const QString& theme_path, QList<QPair<QString, int> >& icons, const QList<QPair<QString, QString> >& icon_names, int& found_icons, const QString& size)
{
	int count = icons.count();

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
	QString path, subpath;
	QStringList dirs;
	foreach (const QString& size, sizes) {
		path = theme_path + "/" + size;
		dirs = QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		// Check each subdirectory of size
		foreach (const QString& dir, dirs) {
			subpath = path + "/" + dir;
			for (int i = 0; i < count; ++i) {
				QPair<QString, int>& icon = icons[i];
				if (icon.second == 2) {
					continue;
				}
				const QPair<QString, QString>& lookup = icon_names.at(i);

				// Check for first choice
				if (icon.second < 2) {
					info.setFile(subpath + "/" + lookup.first);
					if (info.exists() && !info.isDir()) {
						icon.first = info.canonicalFilePath();
						if (icon.second == 0) {
							found_icons++;
						}
						icon.second = 2;
					}
				}

				// Check for second choice
				if (icon.second < 1) {
					info.setFile(subpath + "/" + lookup.second);
					if (info.exists() && !info.isDir()) {
						icon.first = info.canonicalFilePath();
						found_icons++;
						icon.second = 1;
					}
				}
			}

			// Done if the icons are found
			if (found_icons == count) {
				return true;
			}
		}
	}
	return false;
}

// ============================================================================

QList<QIcon> findNativeIcons(const QList<QPair<QString, QString> >& lookup)
{
	QList<QPair<QString, int> > icons;
	for (int i = 0; i < lookup.count(); ++i) {
		icons.append(qMakePair(QString(), 0));
	}
	int found_icons = 0;

	// Find native theme
	QString theme = QSettings().value("Icon Theme").toString();
	QString size = QSettings().value("Icon Size", "22x22").toString();
	if (theme.isEmpty()) {
		if (getenv("KDE_FULL_SESSION")) {
			size = "22x22";

			// Copied from src/gui/styles/qplastiquestyle.cpp in Qt 4.3.4
			// Copyright (C) 1992-2008 Trolltech ASA.
			QProcess kreadconfig;
			kreadconfig.start(QLatin1String("kreadconfig --file kdeglobals --group Icons --key Theme --default crystalsvg"));
			if (kreadconfig.waitForFinished())
				theme = QLatin1String(kreadconfig.readLine().trimmed());
		} else {
			size = "24x24";

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
	QStringList locations = findLocations();
	QStringList themes = findParentThemes(theme, locations);
	foreach (QString current, themes) {
		foreach (QString location, locations) {
			if (findIcons(location + current, icons, lookup, found_icons, size)) {
				goto done;
			}
		}
	}

	// Return list of icons
	done:
	QList<QIcon> result;
	int count = icons.count();
	for (int i = 0; i < count; ++i) {
		result.append(QIcon(icons.at(i).first));
	}
	return result;
}

// ============================================================================
}
#endif

// ============================================================================

Window::Window()
:	m_pause_count(0),
	m_was_paused(false),
	m_pause_available(true)
{
	// Create game object
	m_board = new Board(this);
	setCentralWidget(m_board);
	m_board->setFocus();

	// Create settings window
	m_settings = new Settings(this);
	connect(m_settings, SIGNAL(settingsChanged()), m_board, SLOT(loadSettings()));
	m_settings->installEventFilter(this);

	// Create scores window
	m_scores = new Scores(this);
	connect(m_board, SIGNAL(finished(int, int, int, int)), m_scores, SLOT(addScore(int, int, int, int)));
	m_scores->installEventFilter(this);

	// Create actions
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	initToolBar();
#else
	initMenuBar();
#endif
	m_pause_action->setShortcut(Qt::Key_P);
	m_pause_action->setCheckable(true);
	connect(m_pause_action, SIGNAL(toggled(bool)), m_board, SLOT(pauseGame(bool)));
	connect(m_board, SIGNAL(pauseAvailable(bool)), m_pause_action, SLOT(setEnabled(bool)));
	connect(m_board, SIGNAL(pauseChecked(bool)), m_pause_action, SLOT(setChecked(bool)));

	// Setup window
	setWindowTitle(tr("CuteMaze"));
	resize(QSettings().value("Size", QSize(448, 448)).toSize());

	// Create auto-save timer
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), m_board, SLOT(saveGame()));
	timer->start(300000);
}

// ============================================================================

bool Window::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == m_settings || watched == m_scores) {
		// Pause game when a dialog is shown
		if (event->type() == QEvent::Show) {
			m_pause_count++;
			if (m_pause_count == 1) {
				m_pause_available = m_pause_action->isEnabled();
				if (m_pause_available) {
					m_was_paused = m_pause_action->isChecked();
				} else {
					m_was_paused = true;
				}
			}
			if (m_pause_action->isEnabled()) {
				m_pause_action->setChecked(true);
			}
		// Unpause game when all dialogs are hidden
		} else if (event->type() == QEvent::Hide) {
			m_pause_count--;
			if ( m_pause_count == 0 && ( !m_was_paused || (!m_pause_available && m_pause_action->isEnabled()) ) ) {
				m_pause_action->setChecked(false);
			}
		}
		return false;
	} else {
		return QMainWindow::eventFilter(watched, event);
	}
}

// ============================================================================

void Window::closeEvent(QCloseEvent* event)
{
	QSettings().setValue("Size", size());
	m_board->saveGame();
	QMainWindow::closeEvent(event);
}

// ============================================================================

void Window::initMenuBar()
{
	QMenu* game_menu = menuBar()->addMenu(tr("Game"));
	game_menu->addAction(tr("New Game"), m_board, SLOT(newGame()), tr("Ctrl+N"));
	m_pause_action = game_menu->addAction(tr("Pause Game"));
	game_menu->addAction(tr("High Scores"), m_scores, SLOT(show()), tr("Ctrl+H"));
	game_menu->addAction(tr("Settings"), m_settings, SLOT(show()));
	game_menu->addAction(tr("Quit"), this, SLOT(close()), tr("Ctrl+Q"));
}

// ============================================================================

void Window::initToolBar()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	// Fetch icons for toolbar
	QList<QIcon> icons = findNativeIcons(
		QList<QPair<QString, QString> >()
		<< qMakePair(QString("document-new.png"), QString("filenew.png"))
		<< qMakePair(QString("player_pause.png"), QString())
		<< qMakePair(QString("spreadsheet.png"), QString())
		<< qMakePair(QString("configure.png"), QString("document-properties.png"))
		<< qMakePair(QString("application-exit.png"), QString("exit.png"))
	);
	Q_ASSERT(icons.count() == 5);

	// Create toolbar
	QAction* action;
	QToolBar* toolbar = new QToolBar(this);
	toolbar->setFloatable(false);
	toolbar->setMovable(false);
	toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	action = toolbar->addAction(icons.at(0), tr("New"), m_board, SLOT(newGame()));
	action->setShortcut(tr("Ctrl+N"));
	m_pause_action = toolbar->addAction(icons.at(1), tr("Pause"));
	action = toolbar->addAction(icons.at(2), tr("Scores"), m_scores, SLOT(show()));
	action->setShortcut(tr("Ctrl+H"));
	action = toolbar->addAction(icons.at(3), tr("Settings"), m_settings, SLOT(show()));
	action->setShortcut(tr("Ctrl+S"));
	action = toolbar->addAction(icons.at(4), tr("Quit"), this, SLOT(close()));
	action->setShortcut(tr("Ctrl+Q"));
	addToolBar(toolbar);
	setContextMenuPolicy(Qt::NoContextMenu);
#endif
}

// ============================================================================
