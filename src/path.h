/*
	SPDX-FileCopyrightText: 2009 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_PATH_H
#define CUTEMAZE_PATH_H

#include <QList>
#include <QPoint>
#include <QVector>
class Cell;
class Maze;

class Path
{
public:
	Path(Maze* maze, const QPoint& start, const QPoint& end);

	QPoint end() const
	{
		return m_end;
	}

	QPoint hint(const QPoint& cell) const;

	int length(const QPoint& start) const;

private:
	void findNextCell(QPoint& pos);
	bool leftWall(const QPoint& pos) const;
	bool rightWall(const QPoint& pos) const;
	bool topWall(const QPoint& pos) const;
	bool bottomWall(const QPoint& pos) const;

	int cellWalls(const QPoint& pos) const
	{
		return leftWall(pos) + rightWall(pos) + topWall(pos) + bottomWall(pos);
	}

private:
	Maze* m_maze;
	QPoint m_start;
	QPoint m_end;
	QVector<QVector<bool>> m_cells;
	QList<QPoint> m_solution;
};

#endif // CUTEMAZE_PATH_H
