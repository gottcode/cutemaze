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

#ifndef SOLVER_H
#define SOLVER_H

#include <QList>
#include <QPoint>
class Maze;
class Path;

class Solver
{
public:
	Solver(Maze* maze, const QPoint& start, const QList<QPoint>& targets);

	QPoint hint(const QPoint& current);
	void removeTarget(const QPoint& target);

private:
	Maze* m_maze;
	QList<Path*> m_paths;
};

#endif // SOLVER_H
