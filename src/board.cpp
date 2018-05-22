/***********************************************************************
 *
 * Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2016, 2018 Graeme Gott <graeme@gottcode.org>
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
#include "solver.h"
#include "theme.h"

#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QStatusBar>
#include <QTimeLine>
#include <QTimer>

#include <algorithm>
#include <ctime>
#include <random>

// ============================================================================

Board::Board(QMainWindow* parent)
:	QWidget(parent),
	m_done(false),
	m_paused(false),
	m_total_targets(3),
	m_maze(0),
	m_solver(0),
	m_show_path(true),
	m_smooth_movement(true),
	m_col_delta(0),
	m_row_delta(0),
	m_unit(32),
	m_zoom(5),
	m_max_zoom(5),
	m_zoom_size(14),
	m_player_angle(360),
	m_player_steps(0),
	m_player_total_time(0),
	m_hint(-1, -1),
	m_hint_angle(0)
{
	setMinimumSize(448, 448);
	setFocusPolicy(Qt::StrongFocus);

	m_move_animation = new QTimeLine(60, this);
	m_move_animation->setFrameRange(0, 3);
	m_move_animation->setCurveShape(QTimeLine::LinearCurve);
	m_move_animation->setUpdateInterval(15);
	connect(m_move_animation, &QTimeLine::frameChanged, this, static_cast<void (Board::*)()>(&Board::repaint));

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
	connect(m_status_timer, &QTimer::timeout, this, &Board::updateStatusMessage);

	// Setup theme support
	m_theme = new Theme;
	m_theme->setDevicePixelRatio(devicePixelRatio());

	loadSettings();

	// Start or load game
	if (QSettings().contains("Current/Seed")) {
		loadGame();
	} else {
		m_done = true;
		newGame();
	}
}

// ============================================================================

Board::~Board()
{
	delete m_maze;
	delete m_solver;
	delete m_theme;
}

// ============================================================================

void Board::newGame()
{
	// Stop tracking time
	m_status_timer->stop();

	// Fetch new seed
#ifndef Q_OS_WIN
	std::random_device rd;
	unsigned int seed = rd();
#else
	std::mt19937 gen(time(0));
	std::uniform_int_distribution<unsigned int> dist;
	unsigned int seed = dist(gen);
#endif

	// Set values for new game
	QSettings settings;
	settings.remove("Current");
	settings.setValue("Current/Algorithm", settings.value("New/Algorithm", 4).toInt());
	settings.setValue("Current/Seed", seed);
	settings.setValue("Current/Size", settings.value("New/Size", 20).toInt());
	settings.setValue("Current/Targets", settings.value("New/Targets", 3).toInt());
	settings.setValue("Current/Version", 3);
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
	emit hintAvailable(true);
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
	bool success = false;
	if (settings.value("Current/Version").toInt() == 3) {
		generate(settings.value("Current/Seed").toUInt());
		success = m_maze->load();
	}
	if (!success) {
		QMessageBox::warning(this, tr("Sorry"), tr("Unable to load previous game. A new game will be started."));
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
			QPoint target = m_targets.takeAt(i);
			m_solver->removeTarget(target);
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
	emit hintAvailable(!m_paused);
}

// ============================================================================

void Board::hint()
{
	if (m_done || m_paused || (m_smooth_movement && m_move_animation->state() == QTimeLine::Running)) {
		return;
	}

	m_hint = m_solver->hint(m_player);
	if (m_hint.x() < m_player.x()) {
		m_hint_angle = 270;
	} else if (m_hint.x() > m_player.x()) {
		m_hint_angle = 90;
	} else if (m_hint.y() < m_player.y()) {
		m_hint_angle = 360;
	} else {
		m_hint_angle = 180;
	}
	int pos = (m_zoom / 2) + 1;
	m_hint = m_hint - m_player + QPoint(pos, pos);
	update();
}

// ============================================================================

void Board::zoomIn()
{
	if (m_zoom > 5) {
		m_zoom -= 2;
		scale();
		update();
	}
}

// ============================================================================

void Board::zoomOut()
{
	if (m_zoom < m_max_zoom) {
		m_zoom += 2;
		scale();
		update();
	}
}

// ============================================================================

void Board::loadSettings()
{
	QSettings settings;

	// Load zoom
	m_zoom = QSettings().value("Zoom", 5).toInt();
	if ((m_zoom % 2) == 0) {
		m_zoom--;
	}
	m_zoom = std::max(m_zoom, 5);

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
	m_controls_hint = settings.value("Controls/Hint", Qt::Key_H).toUInt();

	// Load theme
	m_theme->load(settings.value("Theme", "Mouse").toString());
	renderBackground();

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
	} else if (keypress == m_controls_hint) {
		hint();
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
		m_hint = QPoint(-1, -1);

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
			QPoint target = m_targets.takeAt(i);
			m_solver->removeTarget(target);
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
			renderMaze();
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
	scale();
}

// ============================================================================

void Board::updateStatusMessage()
{
	if (m_done || m_paused) {
		return;
	}

	QTime t = QTime(0, 0, 0).addMSecs(m_player_time.elapsed() + m_player_total_time);
	m_status_time_message->setText(tr("%1 elapsed") .arg(t.toString("hh:mm:ss")));
	m_status_steps_message->setText(tr("%1 steps taken") .arg(m_player_steps));
	m_status_remain_message->setText(tr("%1 of %2 targets remain") .arg(m_targets.size()) .arg(m_total_targets));
}

// ============================================================================

void Board::scale()
{
	m_zoom_size = (m_zoom * 3) - 1;
	m_unit = std::min(width(), height()) / m_zoom_size;
	m_theme->scale(m_unit);
	renderBackground();
	emit zoomOutAvailable(m_zoom < m_max_zoom);
	emit zoomInAvailable(m_zoom > 5);
	QSettings().setValue("Zoom", m_zoom);
}

// ============================================================================

void Board::generate(unsigned int seed)
{
	QSettings settings;
	int size = qBound(10, settings.value("Current/Size").toInt(), 100);
	m_total_targets = qBound(1, settings.value("Current/Targets").toInt(), 100);
	m_max_zoom = size / 2;
	if ((m_max_zoom % 2) == 0) {
		m_max_zoom--;
	}
	m_zoom = std::min(m_zoom, m_max_zoom);
	scale();

	// Create new maze
	m_targets.clear();
	delete m_maze;
	switch (QSettings().value("Current/Algorithm").toInt()) {
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
	std::mt19937 gen(seed);
	m_maze->generate(size, size, gen);

	// Add player and targets
	QList<QPoint> locations;
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			locations.append(QPoint(x,y));
		}
	}
	std::shuffle(locations.begin(), locations.end(), gen);
	m_player = m_start = locations.first();
	m_targets = locations.mid(1, m_total_targets);

	// Find solutions
	delete m_solver;
	m_solver = new Solver(m_maze, m_start, m_targets);
	m_hint = QPoint(-1, -1);
}

// ============================================================================

void Board::finish()
{
	emit hintAvailable(false);
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

void Board::renderBackground()
{
	int size = (m_zoom_size + 6) * m_unit;
	int ratio = devicePixelRatio();
	m_back = QPixmap(QSize(size, size) * ratio);
	m_back.setDevicePixelRatio(ratio);
	QPainter painter(&m_back);
	m_theme->drawBackground(painter);
}

// ============================================================================

void Board::renderMaze()
{
	int frame = m_smooth_movement ? m_move_animation->currentFrame() : 3;
	Q_ASSERT(frame > -1);
	Q_ASSERT(frame < 5);

	int pos = (m_zoom / 2) + 1;
	int column = m_player.x() - m_col_delta - pos;
	int row = m_player.y() - m_row_delta - pos;
	int columns = m_maze->columns();
	int rows = m_maze->rows();
	Q_ASSERT(m_player.x() > -1);
	Q_ASSERT(m_player.x() < columns);
	Q_ASSERT(m_player.y() > -1);
	Q_ASSERT(m_player.y() < rows);

	// Create painter
	QPainter painter(this);
	int size = m_unit * m_zoom_size;
	painter.setClipRect((width() - size) >> 1, (height() - size) >> 1, size, size);
	painter.translate((width() - size) >> 1, (height() - size) >> 1);
	painter.translate(-3 * m_unit, -3 * m_unit);

	// Shift by frame amount
	painter.save();
	int delta = frame * -m_unit;
	painter.translate(delta * m_col_delta, delta * m_row_delta);

	// Draw background
	painter.drawPixmap(0, 0, m_back);

	// Initialize corners
	int full_view = m_zoom + 3;
	unsigned char corners[full_view][full_view];
	for (int r = 0; r < full_view; ++r) {
		for (int c = 0; c < full_view; ++c) {
			corners[c][r] = 0;
		}
	}

	// Setup columns
	int column_start = 0;
	int column_count = m_zoom + 2;
	if (column < 1) {
		column_start = abs(column);
	} else if ((column + m_zoom + 1) >= columns) {
		column_count = columns - column;
	}

	// Setup rows
	int row_start = 0;
	int row_count = m_zoom + 2;
	if (row < 1) {
		row_start = abs(row);
	} else if ((row + m_zoom + 1) >= rows) {
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
	for (int r = 0; r < full_view; ++r) {
		for (int c = 0; c < full_view; ++c) {
			unsigned char walls = corners[c][r];
			if (walls) {
				m_theme->drawCorner(painter, c, r, walls);
			}
		}
	}

	// Draw start
	QRect view(column, row, m_zoom + 2, m_zoom + 2);
	if (view.contains(m_start)) {
		m_theme->draw(painter, m_start.x() - column, m_start.y() - row, Theme::Start);
	}

	// Draw targets
	for (const QPoint& target : m_targets) {
		if (view.contains(target)) {
			m_theme->draw(painter, target.x() - column, target.y() - row, Theme::Target);
		}
	}

	painter.restore();

	// Draw hint
	if (m_hint.x() != -1) {
		painter.save();
		switch (m_hint_angle) {
		case 90:
			painter.translate(-m_unit, 0);
			break;
		case 180:
			painter.translate(0, -m_unit);
			break;
		case 270:
			painter.translate(m_unit, 0);
			break;
		case 360:
			painter.translate(0, m_unit);
			break;
		default:
			break;
		};
		m_theme->draw(painter, m_hint.x(), m_hint.y(), Theme::Hint, m_hint_angle);
		painter.restore();
	}

	// Draw player
	m_theme->draw(painter, pos, pos, Theme::Player, m_player_angle);
}

// ============================================================================

void Board::renderDone()
{
	int columns = m_maze->columns();
	int rows = m_maze->rows();

	// Determine sizes
	int mcr = std::min(columns, rows);
	int cell_width = std::min(width(), height());
	cell_width -= (mcr + 1);
	cell_width /= mcr;
	cell_width += 1;
	int w = columns * cell_width + 1;
	int h = rows * cell_width + 1;

	// Create painter
	QPainter painter(this);
	painter.save();
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
				painter.fillRect(x1, y1, cell_width, cell_width, Qt::lightGray);
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
	painter.restore();
	renderText(&painter, tr("Success"));
}

// ============================================================================

void Board::renderPause()
{
	int size = m_unit * m_zoom_size;

	// Create painter
	QPainter painter(this);
	painter.save();
	painter.translate((width() - size) >> 1, (height() - size) >> 1);
	painter.fillRect(0, 0, size, size, Qt::white);

	// Draw message
	painter.restore();
	renderText(&painter, tr("Paused"));
}

// ============================================================================

void Board::renderText(QPainter* painter, const QString& message) const
{
	// Find message size
	QFont f = font();
	f.setPointSize(24);
	QFontMetrics metrics(f);
	int width = metrics.boundingRect(message).width();
	int height = metrics.height();

	painter->save();
	painter->translate(rect().center() - QRect(0, 0, width + height, height * 2).center());

	// Draw black background
	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(0, 0, 0, 200));
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->drawRoundedRect(0, 0, width + height, height * 2, 10, 10);

	// Draw message
	painter->setFont(f);
	painter->setPen(Qt::white);
	painter->setRenderHint(QPainter::TextAntialiasing, true);
	painter->drawText(height / 2, height / 2 + metrics.ascent(), message);

	painter->restore();
}

// ============================================================================
