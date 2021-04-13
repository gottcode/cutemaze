/*
	SPDX-FileCopyrightText: 2007-2019 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_BOARD_H
#define CUTEMAZE_BOARD_H

class Maze;
class Solver;
class Theme;

#include <QElapsedTimer>
#include <QWidget>
class QLabel;
class QMainWindow;
class QTimeLine;
class QTimer;

class Board : public QWidget
{
	Q_OBJECT

public:
	explicit Board(QMainWindow* parent);
	~Board();

signals:
	void hintAvailable(bool available);
	void pauseChecked(bool checked);
	void pauseAvailable(bool run);
	void zoomInAvailable(bool available);
	void zoomOutAvailable(bool available);
	void finished(int seconds, int steps, int algorithm, int size);

public slots:
	void newGame();
	void loadGame();
	void saveGame();
	void pauseGame(bool paused);
	void hint();
	void zoomIn();
	void zoomOut();
	void loadSettings();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void paintEvent(QPaintEvent*) override;
	void resizeEvent(QResizeEvent*) override;

private slots:
	void updateStatusMessage();

private:
	void scale();
	void generate(unsigned int seed);
	void finish();
	void renderBackground();
	void renderMaze();
	void renderDone();
	void renderPause();
	void renderText(QPainter* painter, const QString& message) const;

private:
	bool m_done;
	bool m_paused;

	int m_total_targets;
	Maze* m_maze;
	QPoint m_start;
	QList<QPoint> m_targets;
	Solver* m_solver;
	QLabel* m_status_time_message;
	QLabel* m_status_steps_message;
	QLabel* m_status_remain_message;
	QTimer* m_status_timer;

	bool m_show_path;
	bool m_smooth_movement;
	int m_col_delta;
	int m_row_delta;
	QTimeLine* m_move_animation;

	Theme* m_theme;
	QPixmap m_back;
	int m_unit;
	int m_zoom;
	int m_max_zoom;
	int m_zoom_size;

	// Player
	QPoint m_player;
	int m_player_angle;
	int m_player_steps;
	QElapsedTimer m_player_time;
	int m_player_total_time;
	unsigned int m_controls_up;
	unsigned int m_controls_down;
	unsigned int m_controls_left;
	unsigned int m_controls_right;
	unsigned int m_controls_flag;
	unsigned int m_controls_hint;
	QPoint m_hint;
	int m_hint_angle;
};

#endif // CUTEMAZE_BOARD_H
