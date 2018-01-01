/***********************************************************************
 *
 * Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2016, 2017, 2018 Graeme Gott <graeme@gottcode.org>
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
	QIcon icon(QString(":/oxygen/64x64/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/48x48/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/32x32/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/22x22/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/16x16/%1.png").arg(name));
	return QIcon::fromTheme(name, icon);
}

// ============================================================================

Window::Window()
:	m_pause_action(0)
{
	// Create game object
	m_board = new Board(this);
	setCentralWidget(m_board);
	m_board->setFocus();

	// Create scores window
	m_scores = new Scores(this);
	connect(m_board, &Board::finished, m_scores, &Scores::addScore);

	// Create actions
	if (iconSize().width() == 26) {
		setIconSize(QSize(24, 24));
	}
	initActions();
	m_pause_action->setCheckable(true);
	connect(m_pause_action, &QAction::toggled, m_board, &Board::pauseGame);
	connect(m_board, &Board::pauseAvailable, m_pause_action, &QAction::setEnabled);
	connect(m_board, &Board::pauseChecked, m_pause_action, &QAction::setChecked);
	connect(m_board, &Board::hintAvailable, m_hint_action, &QAction::setEnabled);

	// Setup window
	restoreGeometry(QSettings().value("Geometry").toByteArray());

	// Create auto-save timer
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, m_board, &Board::saveGame);
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
	QAction* new_action = game_menu->addAction(fetchIcon("document-new"), tr("&New"), this, SLOT(newGame()), QKeySequence::New);
	m_pause_action = game_menu->addAction(fetchIcon("media-playback-pause"), tr("&Pause"));
	m_pause_action->setShortcut(tr("P"));
	m_hint_action = game_menu->addAction(fetchIcon("games-hint"), tr("&Hint"), m_board, SLOT(hint()));
	game_menu->addSeparator();
	game_menu->addAction(fetchIcon("games-highscores"), tr("High &Scores"), m_scores, SLOT(exec()));
	game_menu->addSeparator();
	QAction* quit_action = game_menu->addAction(fetchIcon("application-exit"), tr("&Quit"), this, SLOT(close()), QKeySequence::Quit);
	quit_action->setMenuRole(QAction::QuitRole);

	QMenu* view_menu = menuBar()->addMenu(tr("View"));
	QAction* zoom_in_action = view_menu->addAction(fetchIcon("zoom-in"), tr("Zoom &In"), m_board, SLOT(zoomIn()), tr("Ctrl++"));
	connect(m_board, &Board::zoomInAvailable, zoom_in_action, &QAction::setEnabled);
	QAction* zoom_out_action = view_menu->addAction(fetchIcon("zoom-out"), tr("Zoom &Out"), m_board, SLOT(zoomOut()), tr("Ctrl+-"));
	connect(m_board, &Board::zoomOutAvailable, zoom_out_action, &QAction::setEnabled);

	QMenu* settings_menu = menuBar()->addMenu(tr("&Settings"));
	settings_menu->addAction(fetchIcon("preferences-desktop-locale"), tr("Application &Language..."), this, SLOT(setLocale()));
	settings_menu->addAction(fetchIcon("games-config-options"), tr("&Preferences..."), this, SLOT(showSettings()));

	QMenu* help_menu = menuBar()->addMenu(tr("&Help"));
	QAction* about_action = help_menu->addAction(fetchIcon("help-about"), tr("&About"), this, SLOT(about()));
	about_action->setMenuRole(QAction::AboutRole);
	about_action = help_menu->addAction(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), qApp, SLOT(aboutQt()));
	about_action->setMenuRole(QAction::AboutQtRole);

	// Create toolbar
	QToolBar* toolbar = new QToolBar(this);
	toolbar->setFloatable(false);
	toolbar->setMovable(false);
	toolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
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
	QMessageBox::about(this, tr("About"), QString("<p><center><big><b>%1 %2</b></big><br/>%3<br/><small>%4<br/>%5</small></center></p><p><center>%6<br/><small>%7</small></center></p>")
		.arg(tr("CuteMaze"), QCoreApplication::applicationVersion(),
			tr("A top-down maze game"),
			tr("Copyright &copy; 2007-%1 Graeme Gott").arg("2018"),
			tr("Released under the <a href=%1>GPL 3</a> license").arg("\"http://www.gnu.org/licenses/gpl.html\""),
			tr("Icons are from the <a href=%1>Oxygen</a> theme").arg("\"http://www.oxygen-icons.org/\""),
			tr("Used under the <a href=%1>LGPL 3</a> license").arg("\"http://www.gnu.org/licenses/lgpl.html\""))
	);
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
	connect(&settings, &Settings::settingsChanged, m_board, &Board::loadSettings);
	settings.exec();
}

// ============================================================================

void Window::setLocale()
{
	LocaleDialog dialog;
	dialog.exec();
}

// ============================================================================
