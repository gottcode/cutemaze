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

#include "board.h"

#include "maze.h"
#include "theme.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QStatusBar>
#include <QTimeLine>
#include <QTimer>

#include <ctime>

// ============================================================================

Board::Board(QMainWindow* parent)
:	QWidget(parent),
	m_done(false),
	m_paused(false),
	m_total_targets(3),
	m_maze(0),
	m_show_path(true),
	m_smooth_movement(true),
	m_col_delta(0),
	m_row_delta(0),
	m_player_angle(360),
	m_player_steps(0),
	m_player_total_time(0)
{
	connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(focusChanged()));
	setMinimumSize(448, 448);

	m_move_animation = new QTimeLine(100, this);
	m_move_animation->setFrameRange(0, 3);
	m_move_animation->setCurveShape(QTimeLine::LinearCurve);
	m_move_animation->setUpdateInterval(25);
	connect(m_move_animation, SIGNAL(frameChanged(int)), this, SLOT(repaint()));

	// Create status messages
	m_status_time_message = new QLabel;
	m_status_time_message->setContentsMargins(10, 0, 10, 0);
	parent->statusBar()->addPermanentWidget(m_status_time_message);

	m_status_steps_message = new QLabel;
	m_status_steps_message->setContentsMargins(10, 0, 10, 0);
	parent->statusBar()->addPermanentWidget(m_status_steps_message);

	m_status_remain_message = new QLabel;
	m_status_remain_message->setContentsMargins(10, 0, 10, 0);
	parent->statusBar()->addPermanentWidget(m_status_remain_message);

	m_status_timer = new QTimer(this);
	m_status_timer->setInterval(1000);
	connect(m_status_timer, SIGNAL(timeout()), this, SLOT(updateStatusMessage()));

	// Setup theme support
	m_theme = new Theme;

	// Start or load game
	if (QSettings().contains("Current/Seed")) {
		loadGame();
	} else {
		m_done = true;
		newGame();
	}

	loadSettings();
}

// ============================================================================

Board::~Board()
{
	delete m_maze;
	delete m_theme;
}

// ============================================================================

void Board::newGame()
{
	// Prompt user
	if (!m_done) {
		bool paused = m_paused;
		emit pauseChecked(true);
		if (QMessageBox::question(this, tr("CuteMaze"), tr("Abort current game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
			emit pauseChecked(paused);
			return;
		}
	}

	// Stop tracking time
	m_status_timer->stop();

	// Fetch new seed
	srand(time(0));
	unsigned int seed = rand();

	// Set values for new game
	QSettings settings;
	settings.remove("Current");
	settings.setValue("Current/Seed", seed);
	m_total_targets = settings.value("New/Targets", 3).toInt();
	m_total_targets = m_total_targets > 0 ? m_total_targets : 1;
	m_total_targets = m_total_targets < 100 ? m_total_targets : 99;
	settings.setValue("Current/Targets", m_total_targets);
	settings.setValue("Current/Size", settings.value("New/Size", 50).toInt());
	settings.setValue("Current/Algorithm", settings.value("New/Algorithm", 4).toInt());
	m_player_angle = 360;
	m_player_steps = 0;

	// Create new game
	m_done = false;
	generate(seed);
	saveGame();

	// Begin tracking time
	m_player_total_time = 0;
	m_player_time.start();
	m_status_timer->start();

	// Show
	update();
	updateStatusMessage();
	m_status_remain_message->setVisible(true);

	m_paused = false;
	emit pauseAvailable(true);
	emit pauseChecked(false);
}

// ============================================================================

void Board::loadGame()
{
	m_status_time_message->clear();
	m_status_steps_message->clear();
	m_status_remain_message->clear();

	QSettings settings;

	// Load maze
	m_total_targets = settings.value("Current/Targets", 3).toInt();
	m_total_targets = m_total_targets > 0 ? m_total_targets : 1;
	m_total_targets = m_total_targets < 100 ? m_total_targets : 99;
	generate(settings.value("Current/Seed").toUInt());
	if (!m_maze->load()) {
		QMessageBox::warning(this, tr("CuteMaze"), tr("Unable to load previous game. A new game will be started."));
		m_done = true;
		return newGame();
	}

	// Place player at last location
	m_player = settings.value("Current/Player").toPoint();
	m_player_angle = settings.value("Current/Rotation", 360).toInt();
	if (m_player_angle % 90 != 0 || m_player_angle < 0 || m_player_angle > 360) {
		m_player_angle = 360;
	}
	m_player_steps = settings.value("Current/Steps", 0).toInt();

	// Resume tracking time
	m_player_total_time = settings.value("Current/Time", 0).toInt();
	m_player_time.start();
	m_status_timer->start();

	// Remove any targets with matching movement
	for (int i = 0; i < m_targets.size(); ++i) {
		if (m_maze->cell(m_targets[i].x(), m_targets[i].y()).pathMarker() || m_player == m_targets[i]) {
			m_targets.takeAt(i);
			--i;
		}
	}

	// Show
	update();
	updateStatusMessage();
	m_status_remain_message->setVisible(true);

	// Should not happen, but handle a finished game
	if (m_targets.isEmpty()) {
		finish();
	}
}

// ============================================================================

void Board::saveGame()
{
	if (!m_done) {
		m_maze->save();
		QSettings settings;
		settings.setValue("Current/Player", m_player);
		settings.setValue("Current/Rotation", m_player_angle);
		settings.setValue("Current/Steps", m_player_steps);
		int msecs = m_player_total_time;
		if (!m_paused) {
			msecs += m_player_time.elapsed();
		}
		settings.setValue("Current/Time", msecs);
	}
}

// ============================================================================

void Board::pauseGame(bool paused)
{
	m_paused = paused;
	if (paused) {
		m_status_timer->stop();
		m_player_total_time += m_player_time.elapsed();
	} else {
		m_player_time.start();
		m_status_timer->start();
		updateStatusMessage();
	}
	update();
}

// ============================================================================

void Board::loadSettings()
{
	QSettings settings;

	// Load gameplay settings
	m_status_steps_message->setVisible(settings.value("Show Steps", true).toBool());
	m_status_time_message->setVisible(settings.value("Show Time", true).toBool());
	m_show_path = settings.value("Show Path", true).toBool();
	m_smooth_movement = settings.value("Smooth Movement", true).toBool();

	// Load player controls
	m_controls_up = settings.value("Controls/Up", Qt::Key_Up).toUInt();
	m_controls_down = settings.value("Controls/Down", Qt::Key_Down).toUInt();
	m_controls_left = settings.value("Controls/Left", Qt::Key_Left).toUInt();
	m_controls_right = settings.value("Controls/Right", Qt::Key_Right).toUInt();
	m_controls_flag = settings.value("Controls/Flag", Qt::Key_Space).toUInt();

	// Load theme
	m_theme->load(settings.value("Theme", "Mouse").toString());

	// Show
	update();
	updateStatusMessage();
}

// ============================================================================

void Board::keyPressEvent(QKeyEvent* event)
{
	// Prevent player from changing a paused or finished maze
	if (m_done || m_paused) {
		return;
	}

	// Prevent movement during animation
	if (m_smooth_movement && m_move_animation->state() == QTimeLine::Running) {
		return;
	}

	m_col_delta = m_row_delta = 0;
	QPoint position = m_player;
	const Cell& cell = m_maze->cell(m_player.x(), m_player.y());

	unsigned int keypress = event->key();
	if (keypress == m_controls_left) {
		m_player_angle = 270;
		if (!cell.leftWall()) {
			Q_ASSERT(m_player.x() > 0);
			m_player.rx()--;
		}
	} else if (keypress == m_controls_right) {
		m_player_angle = 90;
		if (!cell.rightWall()) {
			Q_ASSERT(m_player.x() < m_maze->columns() - 1);
			m_player.rx()++;
		}
	} else if (keypress == m_controls_up) {
		m_player_angle = 360;
		if (!cell.topWall()) {
			Q_ASSERT(m_player.y() > 0);
			m_player.ry()--;
		}
	} else if (keypress == m_controls_down) {
		m_player_angle = 180;
		if (!cell.bottomWall()) {
			Q_ASSERT(m_player.y() < m_maze->rows() - 1);
			m_player.ry()++;
		}
	} else if (keypress == m_controls_flag) {
		m_maze->cellMutable(m_player.x(), m_player.y()).toggleFlag();
	} else {
		return;
	}

	// Handle player movement
	if (position != m_player) {
		m_player_steps++;
		m_col_delta = m_player.x() - position.x();
		m_row_delta = m_player.y() - position.y();
		if (m_smooth_movement) {
			m_move_animation->start();
		}

		// Add path marker
		if (m_maze->cell(position.x(), position.y()).pathMarker() == 0) {
			int angle = 0;
			if (m_col_delta) {
				angle = 180 - (m_col_delta * 90);
			} else {
				angle = 360 - ((m_row_delta + 1) * 90);
			}
			m_maze->cellMutable(position.x(), position.y()).setPathMarker(angle);
		}
	}

	// Check for collisions with targets
	for (int i = 0; i < m_targets.size(); ++i) {
		if (m_player == m_targets.at(i)) {
			m_targets.takeAt(i);
			--i;
		}
	}

	// Show updated maze
	update();
	updateStatusMessage();

	// Handle finishing a maze
	if (m_targets.isEmpty()) {
		m_maze->cellMutable(m_player.x(), m_player.y()).setPathMarker(m_player_angle);
		finish();
	}
}

// ============================================================================

void Board::paintEvent(QPaintEvent*)
{
	if (!m_paused) {
		if (!m_done) {
			renderMaze(m_move_animation->currentFrame());
		} else {
			renderDone();
		}
	} else {
		renderPause();
	}
}

// ============================================================================

void Board::resizeEvent(QResizeEvent*)
{
	int size = qMin(width(), height());
	size -= (size % 14);
	float scale = static_cast<float>(size) / 448.0f;
	m_unit = scale * 32;
	m_theme->scale(m_unit);
}

// ============================================================================

void Board::focusChanged()
{
	if (!m_done && !qApp->activeWindow()) {
		emit pauseChecked(true);
	}
}

// ============================================================================

void Board::updateStatusMessage()
{
	if (m_done || m_paused) {
		return;
	}

	QTime t(0, 0, 0);
	t = t.addMSecs(m_player_time.elapsed() + m_player_total_time);

	m_status_time_message->setText(tr("%1 elapsed") .arg(t.toString("hh:mm:ss")));
	m_status_steps_message->setText(tr("%1 steps taken") .arg(m_player_steps));
	m_status_remain_message->setText(tr("%1 of %2 targets remain") .arg(m_targets.size()) .arg(m_total_targets));
}

// ============================================================================

void Board::generate(unsigned int seed)
{
	int columns = QSettings().value("Current/Size", 50).toInt();
	columns = columns > 9 ? columns : 10;
	columns = columns < 100 ? columns : 99;
	int rows = columns;

	// Create new maze
	m_targets.clear();
	srand(seed);
	delete m_maze;
	switch (QSettings().value("Current/Algorithm", 4).toInt()) {
	case 0:
		m_maze = new HuntAndKillMaze;
		break;
	case 1:
		m_maze = new KruskalMaze;
		break;
	case 2:
		m_maze = new PrimMaze;
		break;
	case 3:
		m_maze = new RecursiveBacktrackerMaze;
		break;
	case 5:
		m_maze = new Stack2Maze;
		break;
	case 6:
		m_maze = new Stack3Maze;
		break;
	case 7:
		m_maze = new Stack4Maze;
		break;
	case 8:
		m_maze = new Stack5Maze;
		break;
	case 4:
	default:
		m_maze = new StackMaze;
		break;
	}
	m_maze->generate(columns, rows);

	// Add player
	m_player.setX(rand() % (columns - 1));
	m_player.setY(rand() % (rows - 1));
	m_start = m_player;

	// Add targets
	QList<QPoint> locations;
	if (columns * rows > m_total_targets * 2) {
		locations.append(m_start);
		QPoint target;
		for (int i = 0; i < m_total_targets; ++i) {
			do {
				target.setX(rand() % (columns - 1));
				target.setY(rand() % (rows - 1));
			} while (locations.contains(target));
			locations.append(target);
			m_targets.append(target);
		}
	// Handle if targets cover half or more of the maze
	} else {
		for (int c = 0; c < columns; ++c) {
			for (int r = 0; r < rows; ++r) {
				locations.append(QPoint(c, r));
			}
		}
		locations.removeAll(m_player);
		int pos;
		for (int i = 0; i < m_total_targets; ++i) {
			pos = rand() % locations.size();
			m_targets.append(locations.takeAt(pos));
		}
	}
}

// ============================================================================

void Board::finish()
{
	emit pauseAvailable(false);
	m_move_animation->stop();
	m_move_animation->setCurrentTime(m_move_animation->duration());

	QSettings settings;
	settings.beginGroup("Current");

	// Get score values
	int seconds = (m_player_total_time + m_player_time.elapsed()) / 1000;
	int algorithm = settings.value("Algorithm").toInt();
	int size = settings.value("Size").toInt();

	// Remove game from disk
	m_done = true;
	settings.remove("");

	// Show congratulations
	m_status_timer->stop();
	update();

	// Add high score
	emit finished(m_player_steps, seconds, algorithm, size);
}

// ============================================================================

void Board::renderMaze(int frame)
{
	int column = m_player.x() - m_col_delta - 3;
	int row = m_player.y() - m_row_delta - 3;
	int columns = m_maze->columns();
	int rows = m_maze->rows();

	Q_ASSERT(m_player.x() > -1);
	Q_ASSERT(m_player.x() < columns);
	Q_ASSERT(m_player.y() > -1);
	Q_ASSERT(m_player.y() < rows);
	Q_ASSERT(frame > -1);
	Q_ASSERT(frame < 5);

	// Create painter
	QPainter painter(this);
	int size = m_unit * 14;
	painter.setClipRect((width() - size) >> 1, (height() - size) >> 1, size, size);
	painter.translate((width() - size) >> 1, (height() - size) >> 1);
	painter.translate(-3 * m_unit, -3 * m_unit);

	// Shift by frame amount
	painter.save();
	int delta = frame * -m_unit;
	if (!m_smooth_movement) {
		delta = -3 * m_unit;
	}
	painter.translate(delta * m_col_delta, delta * m_row_delta);

	// Draw background
	for (int r = 0; r < 8; ++r) {
		for (int c = 0; c < 8; ++c) {
			m_theme->draw(painter, c, r, Theme::Background);
		}
	}

	// Initialize corners
	unsigned char corners[8][8];
	for (int r = 0; r < 8; ++r) {
		for (int c = 0; c < 8; ++c) {
			corners[c][r] = 0;
		}
	}

	// Setup columns
	int column_start = 0;
	int column_count = 7;
	if (column < 1) {
		column_start = abs(column);
	} else if (column + 6 >= columns) {
		column_count = columns - column;
	}

	// Setup rows
	int row_start = 0;
	int row_count = 7;
	if (row < 1) {
		row_start = abs(row);
	} else if (row + 6 >= rows) {
		row_count = rows - row;
	}

	// Draw cells
	int angle = 0;
	for (int r = row_start; r < row_count; ++r) {
		for (int c = column_start; c < column_count; ++c) {

			const Cell& cell = m_maze->cell(column + c, row + r);

			// Draw walls
			if (cell.topWall()) {
				m_theme->drawWall(painter, c, r);
			}
			if (cell.leftWall()) {
				m_theme->drawWall(painter, c, r, true);
			}
			if (column + c + 1 == columns) {
				m_theme->drawWall(painter, c + 1, r, true);
			}
			if (row + r + 1 == rows) {
				m_theme->drawWall(painter, c, r + 1);
			}

			// Draw marker
			if (m_show_path) {
				angle = cell.pathMarker();
				if (angle) {
					m_theme->draw(painter, c, r, Theme::Marker, angle);
				}
			}

			// Draw flag
			if (cell.flag()) {
				m_theme->draw(painter, c, r, Theme::Flag);
			}

			// Configure corners
			unsigned char& corner1 = corners[c][r];
			corner1 |= (cell.topWall() << 1);
			corner1 |= (cell.leftWall() << 2);
			unsigned char& corner2 = corners[c + 1][r];
			corner2 |= (cell.topWall() << 3);
			corner2 |= (cell.rightWall() << 2);
			unsigned char& corner3 = corners[c + 1][r + 1];
			corner3 |= (cell.rightWall() << 0);
			corner3 |= (cell.bottomWall() << 3);
			unsigned char& corner4 = corners[c][r + 1];
			corner4 |= (cell.leftWall() << 0);
			corner4 |= (cell.bottomWall() << 1);
		}
	}

	// Draw corners
	for (int r = 0; r < 8; ++r) {
		for (int c = 0; c < 8; ++c) {
			unsigned char walls = corners[c][r];
			if (walls) {
				m_theme->drawCorner(painter, c, r, walls);
			}
		}
	}

	// Draw start
	QRect view(column, row, 7, 7);
	if (view.contains(m_start)) {
		m_theme->draw(painter, m_start.x() - column, m_start.y() - row, Theme::Start);
	}

	// Draw targets
	foreach (QPoint target, m_targets) {
		if (view.contains(target)) {
			m_theme->draw(painter, target.x() - column, target.y() - row, Theme::Target);
		}
	}

	painter.restore();

	// Draw player
	m_theme->draw(painter, 3, 3, Theme::Player, m_player_angle);
}

// ============================================================================

void Board::renderDone()
{
	int columns = m_maze->columns();
	int rows = m_maze->rows();

	// Determine sizes
	int mcr = qMin(columns, rows);
	int cell_width = qMin(width(), height());
	cell_width -= (mcr + 1);
	cell_width /= mcr;
	cell_width += 1;
	int w = columns * cell_width + 1;
	int h = rows * cell_width + 1;

	// Create pixmap
	QPainter painter(this);
	painter.translate((width() - w) >> 1, (height() - h) >> 1);
	painter.fillRect(0, 0, w, h, Qt::white);

	// Draw image
	int x1, x2, y1, y2;
	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < columns; ++c) {
			const Cell& cell = m_maze->cell(c, r);
			x1 = c  * cell_width;
			x2 = x1 + cell_width;
			y1 = r  * cell_width;
			y2 = y1 + cell_width;
			if (cell.pathMarker()) {
				painter.fillRect(x1 + 1, y1 + 1, cell_width, cell_width, Qt::lightGray);
			}
			if (cell.topWall()) {
				painter.drawLine(x1, y1, x2, y1);
			}
			if (cell.leftWall()) {
				painter.drawLine(x1, y1, x1, y2);
			}
		}
	}
	painter.drawLine(0, rows * cell_width, columns * cell_width, rows * cell_width);
	painter.drawLine(columns * cell_width, 0, columns * cell_width, rows * cell_width);

	// Draw congratulations
	renderText(&painter, tr("Success"));
}

// ============================================================================

void Board::renderPause()
{
	// Create painter
	QPainter painter(this);
	int size = m_unit * 14;
	painter.translate((width() - size) >> 1, (height() - size) >> 1);
	painter.fillRect(0, 0, size, size, Qt::white);

	// Draw message
	renderText(&painter, tr("Paused"));
}

// ============================================================================

void Board::renderText(QPainter* painter, const QString& message) const
{
	painter->setFont(QFont("Sans", 24));
	QRect rect = painter->fontMetrics().boundingRect(message);
	int size = (m_unit * 14) >> 1;
	int x1 = size - ((rect.width() + rect.height()) >> 1);
	int y1 = size - rect.height();
	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(0, 0, 0, 200));
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->drawRoundRect(x1, y1, rect.width() + rect.height(), rect.height() * 2, 10);
	painter->setPen(Qt::white);
	painter->setRenderHint(QPainter::TextAntialiasing, true);
	painter->drawText((rect.height() >> 1) + x1, ((rect.height() * 5) >> 2) + y1, message);
}

// ============================================================================
