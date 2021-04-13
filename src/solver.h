/*
	SPDX-FileCopyrightText: 2009 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_SOLVER_H
#define CUTEMAZE_SOLVER_H

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

#endif // CUTEMAZE_SOLVER_H
