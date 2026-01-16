/*
	SPDX-FileCopyrightText: 2007 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_WINDOW_H
#define CUTEMAZE_WINDOW_H

class Board;

#include <QMainWindow>
class QAction;

class Window : public QMainWindow
{
	Q_OBJECT

public:
	explicit Window();

protected:
	void closeEvent(QCloseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private Q_SLOTS:
	void about();
	void newGame();
	void gameFinished(int seconds, int steps, int algorithm, int size);
	void showScores();
	void showSettings();
	void setLocale();

private:
	void initActions();

private:
	Board* m_board;
	QAction* m_pause_action;
	QAction* m_hint_action;
};

#endif // CUTEMAZE_WINDOW_H
