/*
// 	SPDX-FileCopyrightText: 2007-2025 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "window.h"

#include "board.h"
#include "locale_dialog.h"
#include "new_game_dialog.h"
#include "scores_dialog.h"
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

//-----------------------------------------------------------------------------

static QIcon fetchIcon(const QString& name)
{
	QIcon icon(QString(":/oxygen/64x64/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/48x48/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/32x32/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/22x22/%1.png").arg(name));
	icon.addFile(QString(":/oxygen/16x16/%1.png").arg(name));
	return QIcon::fromTheme(name, icon);
}

//-----------------------------------------------------------------------------

Window::Window()
	: m_pause_action(nullptr)
{
	// Create game object
	m_board = new Board(this);
	setCentralWidget(m_board);
	m_board->setFocus();
	connect(m_board, &Board::finished, this, &Window::gameFinished);

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

//-----------------------------------------------------------------------------

void Window::closeEvent(QCloseEvent* event)
{
	QSettings().setValue("Geometry", saveGeometry());
	m_board->saveGame();
	QMainWindow::closeEvent(event);
}

//-----------------------------------------------------------------------------

void Window::wheelEvent(QWheelEvent* event)
{
	if (event->angleDelta().y() > 0) {
		m_board->zoomIn();
	} else {
		m_board->zoomOut();
	}
	QMainWindow::wheelEvent(event);
}

//-----------------------------------------------------------------------------

void Window::initActions()
{
	// Create menubar
#if defined(Q_OS_MAC)
	qApp->setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

	QMenu* game_menu = menuBar()->addMenu(tr("&Game"));
	QAction* new_action = game_menu->addAction(fetchIcon("document-new"), tr("&New"), this, &Window::newGame);
	new_action->setShortcut(QKeySequence::New);
	m_pause_action = game_menu->addAction(fetchIcon("media-playback-pause"), tr("&Pause"));
	m_pause_action->setShortcut(tr("P"));
	m_hint_action = game_menu->addAction(fetchIcon("games-hint"), tr("&Hint"), m_board, &Board::hint);
	game_menu->addSeparator();
	QAction* scores_action = game_menu->addAction(fetchIcon("games-highscores"), tr("High &Scores"), this, &Window::showScores);
	scores_action->setShortcut(tr("Ctrl+H"));
	game_menu->addSeparator();
	QAction* quit_action = game_menu->addAction(fetchIcon("application-exit"), tr("&Quit"), this, &Window::close);
	quit_action->setShortcut(QKeySequence::Quit);
	quit_action->setMenuRole(QAction::QuitRole);

	QMenu* view_menu = menuBar()->addMenu(tr("View"));
	QAction* zoom_in_action = view_menu->addAction(fetchIcon("zoom-in"), tr("Zoom &In"), m_board, &Board::zoomIn);
	zoom_in_action->setShortcut(tr("Ctrl++"));
	connect(m_board, &Board::zoomInAvailable, zoom_in_action, &QAction::setEnabled);
	QAction* zoom_out_action = view_menu->addAction(fetchIcon("zoom-out"), tr("Zoom &Out"), m_board, &Board::zoomOut);
	zoom_out_action->setShortcut(tr("Ctrl+-"));
	connect(m_board, &Board::zoomOutAvailable, zoom_out_action, &QAction::setEnabled);

	QMenu* settings_menu = menuBar()->addMenu(tr("&Settings"));
	settings_menu->addAction(fetchIcon("preferences-desktop-locale"), tr("Application &Language..."), this, &Window::setLocale);
	settings_menu->addAction(fetchIcon("games-config-options"), tr("&Preferences..."), this, &Window::showSettings);

	QMenu* help_menu = menuBar()->addMenu(tr("&Help"));
	QAction* about_action = help_menu->addAction(fetchIcon("help-about"), tr("&About"), this, &Window::about);
	about_action->setMenuRole(QAction::AboutRole);
	about_action = help_menu->addAction(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), qApp, &QApplication::aboutQt);
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

//-----------------------------------------------------------------------------

void Window::about()
{
	QMessageBox::about(this, tr("About"), QString("<p><center><big><b>%1 %2</b></big><br/>%3<br/><small>%4<br/>%5</small></center></p><p><center>%6<br/><small>%7</small></center></p>")
		.arg(tr("CuteMaze"), QCoreApplication::applicationVersion(),
			tr("A top-down maze game"),
			tr("Copyright &copy; 2007-%1 Graeme Gott").arg("2025"),
			tr("Released under the <a href=%1>GPL 3</a> license").arg("\"http://www.gnu.org/licenses/gpl.html\""),
			tr("Icons are from the <a href=%1>Oxygen</a> theme").arg("\"http://www.oxygen-icons.org/\""),
			tr("Used under the <a href=%1>LGPL 3</a> license").arg("\"http://www.gnu.org/licenses/lgpl.html\""))
	);
}

//-----------------------------------------------------------------------------

void Window::newGame()
{
	NewGameDialog dialog(this);
	if (dialog.exec() == QDialog::Accepted) {
		m_board->newGame();
	}
}

//-----------------------------------------------------------------------------

void Window::gameFinished(int seconds, int steps, int algorithm, int size)
{
	ScoresDialog scores(this);
	if (scores.addScore(seconds, steps, algorithm, size)) {
		scores.exec();
	}
}

//-----------------------------------------------------------------------------

void Window::showScores()
{
	ScoresDialog scores(this);
	scores.exec();
}

//-----------------------------------------------------------------------------

void Window::showSettings()
{
	Settings settings(this);
	connect(&settings, &Settings::settingsChanged, m_board, &Board::loadSettings);
	settings.exec();
}

//-----------------------------------------------------------------------------

void Window::setLocale()
{
	LocaleDialog dialog;
	dialog.exec();
}

//-----------------------------------------------------------------------------
