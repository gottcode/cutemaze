/***********************************************************************
 *
 * Copyright (C) 2007-2021 Graeme Gott <graeme@gottcode.org>
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
