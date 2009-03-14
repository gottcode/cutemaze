/***********************************************************************
 *
 * Copyright (C) 2009 Graeme Gott <graeme@gottcode.org>
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

#ifndef PATH_H
#define PATH_H

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
		{ return m_end; }

	QPoint hint(const QPoint& cell) const;

	int length(const QPoint& start) const;

private:
	void findNextCell(QPoint& pos);
	bool leftWall(const QPoint& pos) const;
	bool rightWall(const QPoint& pos) const;
	bool topWall(const QPoint& pos) const;
	bool bottomWall(const QPoint& pos) const;
	int cellWalls(const QPoint& pos) const
		{ return leftWall(pos) + rightWall(pos) + topWall(pos) + bottomWall(pos); }

	Maze* m_maze;
	QPoint m_start;
	QPoint m_end;
	QVector< QVector<bool> > m_cells;
	QList<QPoint> m_solution;
};

#endif // PATH_H
