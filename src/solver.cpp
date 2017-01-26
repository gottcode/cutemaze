/***********************************************************************
 *
 * Copyright (C) 2009, 2014, 2017 Graeme Gott <graeme@gottcode.org>
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

#include "solver.h"

#include "maze.h"
#include "path.h"

#include <algorithm>

// ============================================================================

namespace
{
	QPoint compare_start;

	bool pathShorter(const Path* path1, const Path* path2)
	{
		return path1->length(compare_start) < path2->length(compare_start);
	}
}

// ============================================================================

Solver::Solver(Maze* maze, const QPoint& start, const QList<QPoint>& targets)
:	m_maze(maze)
{
	for (const QPoint& target : targets) {
		m_paths.append(new Path(m_maze, start, target));
	}
	compare_start = start;
	std::sort(m_paths.begin(), m_paths.end(), pathShorter);
}

// ============================================================================

QPoint Solver::hint(const QPoint& current)
{
	forever {
		QPoint result = m_paths.first()->hint(current);
		if (result.x() != -1) {
			return result;
		} else {
			QPoint target = m_paths.first()->end();
			delete m_paths.takeFirst();
			m_paths.append(new Path(m_maze, current, target));
			compare_start = current;
			std::sort(m_paths.begin(), m_paths.end(), pathShorter);
		}
	}
}

// ============================================================================

void Solver::removeTarget(const QPoint& target)
{
	for (int i = 0; i < m_paths.count(); ++i) {
		if (m_paths.at(i)->end() == target) {
			delete m_paths.takeAt(i);
			break;
		}
	}
}

// ============================================================================
