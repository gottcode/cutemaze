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
#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QPair>
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QTimer>

#if defined(QTOPIA_PHONE)
#include <QSoftMenuBar>
#else
#include <QMenuBar>
#include <QToolBar>
#endif

// ============================================================================

Window::Window()
{
	setWindowIcon(QIcon(":/cutemaze.png"));

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
	m_pause_action->setCheckable(true);
	connect(m_pause_action, SIGNAL(toggled(bool)), m_board, SLOT(pauseGame(bool)));
	connect(m_board, SIGNAL(pauseAvailable(bool)), m_pause_action, SLOT(setEnabled(bool)));
	connect(m_board, SIGNAL(pauseAvailable(bool)), m_hint_action, SLOT(setEnabled(bool)));
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
	game_menu->addAction(tr("Hint"), m_board, SLOT(hint()));
	game_menu->addAction(tr("High Scores"), m_scores, SLOT(exec()));
	m_pause_action = game_menu->addAction(tr("Pause Game"));
	game_menu->addAction(tr("New Game"), m_board, SLOT(newGame()));
#else
	// Create menubar
	QMenu* game_menu = menuBar()->addMenu(tr("Game"));

	QIcon new_icon(QPixmap(":/22x22/document-new.png"));
	new_icon.addPixmap(QPixmap(":/16x16/document-new.png"));
	QAction* new_action = game_menu->addAction(new_icon, tr("New"), m_board, SLOT(newGame()), tr("Ctrl+N"));

	QIcon pause_icon(QPixmap(":/22x22/media-playback-pause.png"));
	pause_icon.addPixmap(QPixmap(":/16x16/media-playback-pause.png"));
	m_pause_action = game_menu->addAction(pause_icon, tr("Pause"));
	m_pause_action->setShortcut(tr("P"));

	QIcon hint_icon(QPixmap(":/22x22/games-hint.png"));
	hint_icon.addPixmap(QPixmap(":/16x16/games-hint.png"));
	m_hint_action = game_menu->addAction(hint_icon, tr("Hint"), m_board, SLOT(hint()), tr("H"));

	game_menu->addSeparator();

	QIcon scores_icon(QPixmap(":/22x22/games-highscores.png"));
	scores_icon.addPixmap(QPixmap(":/16x16/games-highscores.png"));
	game_menu->addAction(scores_icon, tr("High Scores"), m_scores, SLOT(exec()));

	game_menu->addSeparator();

	QIcon config_icon(QPixmap(":/22x22/games-config-options.png"));
	config_icon.addPixmap(QPixmap(":/16x16/games-config-options.png"));
	game_menu->addAction(config_icon, tr("Settings"), m_settings, SLOT(exec()));

	game_menu->addSeparator();

	QIcon quit_icon(QPixmap(":/22x22/application-exit.png"));
	quit_icon.addPixmap(QPixmap(":/16x16/application-exit.png"));
	game_menu->addAction(quit_icon, tr("Quit"), this, SLOT(close()), tr("Ctrl+Q"));

	QMenu* help_menu = menuBar()->addMenu(tr("Help"));
	help_menu->addAction(tr("About"), this, SLOT(about()));
	help_menu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));

	// Create toolbar
	QToolBar* toolbar = new QToolBar(this);
	toolbar->setIconSize(QSize(22, 22));
	toolbar->setFloatable(false);
	toolbar->setMovable(false);
	toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	toolbar->addAction(new_action);
	toolbar->addAction(m_pause_action);
	toolbar->addAction(m_hint_action);
	addToolBar(toolbar);
	setContextMenuPolicy(Qt::NoContextMenu);
#endif
}

// ============================================================================

void Window::about()
{
	QMessageBox::about(this, tr("About CuteMaze"), tr(
		"<center>"
		"<big><b>CuteMaze 1.0.2</b></big><br/>"
		"A top-down maze game<br/>"
		"<small>Copyright &copy; 2007-2009 Graeme Gott</small><br/><br/>"
		"Toolbar icons are from <a href=\"http://www.oxygen-icons.org/\">Oyxgen</a>"
		"</center>"
	));
}

// ============================================================================
