/*
	SPDX-FileCopyrightText: 2007-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_WINDOW_H
#define CUTEMAZE_WINDOW_H

#include <QMainWindow>
class Board;
class QAction;

class Window : public QMainWindow
{
	Q_OBJECT

public:
	Window();

protected:
	virtual void closeEvent(QCloseEvent* event);
	virtual bool event(QEvent* event);
	virtual void wheelEvent(QWheelEvent* event);

private slots:
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
