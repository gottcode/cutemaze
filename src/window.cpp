/***********************************************************************
 *
 * Copyright (C) 2007, 2008, 2009, 2012 Graeme Gott <graeme@gottcode.org>
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
#include "locale_dialog.h"
#include "new_game_dialog.h"
#include "scores.h"
#include "settings.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPair>
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QToolBar>
#include <QWheelEvent>

// ============================================================================

static QIcon fetchIcon(const QString& name)
{
	QIcon icon(QString(":/oxygen/22x22/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/16x16/%1.png").arg(name));
	return QIcon::fromTheme(name, icon);
}

// ============================================================================

Window::Window()
:	m_pause_action(0)
{
	setWindowIcon(QIcon(":/cutemaze.png"));

	// Create game object
	m_board = new Board(this);
	setCentralWidget(m_board);
	m_board->setFocus();

	// Create scores window
	m_scores = new Scores(this);
	connect(m_board, SIGNAL(finished(int, int, int, int)), m_scores, SLOT(addScore(int, int, int, int)));

	// Create actions
	initActions();
	m_pause_action->setCheckable(true);
	connect(m_pause_action, SIGNAL(toggled(bool)), m_board, SLOT(pauseGame(bool)));
	connect(m_board, SIGNAL(pauseAvailable(bool)), m_pause_action, SLOT(setEnabled(bool)));
	connect(m_board, SIGNAL(pauseChecked(bool)), m_pause_action, SLOT(setChecked(bool)));
	connect(m_board, SIGNAL(hintAvailable(bool)), m_hint_action, SLOT(setEnabled(bool)));

	// Setup window
	setWindowTitle(tr("CuteMaze"));
	restoreGeometry(QSettings().value("Geometry").toByteArray());

	// Create auto-save timer
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), m_board, SLOT(saveGame()));
	timer->start(300000);
}

// ============================================================================

void Window::closeEvent(QCloseEvent* event)
{
	QSettings().setValue("Geometry", saveGeometry());
	m_board->saveGame();
	QMainWindow::closeEvent(event);
}

// ============================================================================

bool Window::event(QEvent* event)
{
	if ((event->type() == QEvent::WindowBlocked || event->type() == QEvent::WindowDeactivate) && m_pause_action && m_pause_action->isEnabled()) {
		m_pause_action->setChecked(true);
	}
	return QMainWindow::event(event);
}

// ============================================================================

void Window::wheelEvent(QWheelEvent* event)
{
	if (event->delta() > 0) {
		m_board->zoomIn();
	} else {
		m_board->zoomOut();
	}
	QMainWindow::wheelEvent(event);
}

// ============================================================================

void Window::initActions()
{
	// Create menubar
#if defined(Q_OS_MAC)
	qApp->setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

	QMenu* game_menu = menuBar()->addMenu(tr("&Game"));
	QAction* new_action = game_menu->addAction(fetchIcon("document-new"), tr("&New"), this, SLOT(newGame()), tr("Ctrl+N"));
	m_pause_action = game_menu->addAction(fetchIcon("media-playback-pause"), tr("&Pause"));
	m_pause_action->setShortcut(tr("P"));
	m_hint_action = game_menu->addAction(fetchIcon("games-hint"), tr("&Hint"), m_board, SLOT(hint()), tr("H"));
	game_menu->addSeparator();
	game_menu->addAction(fetchIcon("games-highscores"), tr("High &Scores"), m_scores, SLOT(exec()));
	game_menu->addSeparator();
	game_menu->addAction(fetchIcon("application-exit"), tr("&Quit"), this, SLOT(close()), tr("Ctrl+Q"));

	QMenu* view_menu = menuBar()->addMenu(tr("View"));
	QAction* zoom_in_action = view_menu->addAction(fetchIcon("zoom-in"), tr("Zoom &In"), m_board, SLOT(zoomIn()), tr("Ctrl++"));
	connect(m_board, SIGNAL(zoomInAvailable(bool)), zoom_in_action, SLOT(setEnabled(bool)));
	QAction* zoom_out_action = view_menu->addAction(fetchIcon("zoom-out"), tr("Zoom &Out"), m_board, SLOT(zoomOut()), tr("Ctrl+-"));
	connect(m_board, SIGNAL(zoomOutAvailable(bool)), zoom_out_action, SLOT(setEnabled(bool)));

	QMenu* settings_menu = menuBar()->addMenu(tr("&Settings"));
	settings_menu->addAction(fetchIcon("preferences-desktop-locale"), tr("Application &Language..."), this, SLOT(setLocale()));
	settings_menu->addAction(fetchIcon("games-config-options"), tr("&Preferences..."), this, SLOT(showSettings()));

	QMenu* help_menu = menuBar()->addMenu(tr("&Help"));
	help_menu->addAction(fetchIcon("help-about"), tr("&About"), this, SLOT(about()));
	help_menu->addAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), qApp, SLOT(aboutQt()));

	// Create toolbar
	QToolBar* toolbar = new QToolBar(this);
	toolbar->setIconSize(QSize(22, 22));
	toolbar->setFloatable(false);
	toolbar->setMovable(false);
	toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	toolbar->addAction(new_action);
	toolbar->addAction(m_pause_action);
	toolbar->addAction(m_hint_action);
	toolbar->addSeparator();
	toolbar->addAction(zoom_in_action);
	toolbar->addAction(zoom_out_action);
	addToolBar(toolbar);
	setContextMenuPolicy(Qt::NoContextMenu);
}

// ============================================================================

void Window::about()
{
	QMessageBox::about(this, tr("About"), tr(
		"<p><center><big><b>CuteMaze %1</b></big><br/>"
		"A top-down maze game<br/>"
		"<small>Copyright &copy; 2007-%2 Graeme Gott</small><br/>"
		"<small>Released under the <a href=\"http://www.gnu.org/licenses/gpl.html\">GPL 3</a> license</small></center></p>"
		"<p><center>Icons are from the <a href=\"http://www.oxygen-icons.org/\">Oxygen</a> theme<br/>"
		"<small>Used under the <a href=\"http://www.gnu.org/licenses/lgpl.html\">LGPL 3</a> license</small></center></p>"
	).arg(QCoreApplication::applicationVersion()).arg("2012"));
}

// ============================================================================

void Window::newGame()
{
	NewGameDialog dialog(this);
	if (dialog.exec() == QDialog::Accepted) {
		m_board->newGame();
	}
}

// ============================================================================

void Window::showSettings()
{
	Settings settings(this);
	connect(&settings, SIGNAL(settingsChanged()), m_board, SLOT(loadSettings()));
	settings.exec();
}

// ============================================================================

void Window::setLocale()
{
	LocaleDialog dialog;
	dialog.exec();
}

// ============================================================================
