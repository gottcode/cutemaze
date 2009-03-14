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

#include "path.h"

#include "maze.h"

// ============================================================================

Path::Path(Maze* maze, const QPoint& start, const QPoint& end)
:	m_maze(maze),
	m_start(start),
	m_end(end),
	m_cells(maze->columns(), QVector<bool>(maze->rows(), false))
{
	// Fill in dead ends
	for (int r = 0; r < m_maze->rows(); ++r) {
		for (int c = 0; c < m_maze->columns(); ++c) {
			QPoint pos(c, r);
			while (cellWalls(pos) == 3 && pos != m_start && pos != m_end) {
				m_cells[pos.x()][pos.y()] = true;
				findNextCell(pos);
			}
		}
	}

	// Follow path and record cells
	QPoint pos = start;
	pos = start;
	do {
		m_cells[pos.x()][pos.y()] = true;
		m_solution.append(pos);
		findNextCell(pos);
	} while (pos != end);
	m_solution.append(pos);
}

// ============================================================================

QPoint Path::hint(const QPoint& cell) const
{
	int pos = m_solution.indexOf(cell);
	if (pos != -1 && pos < m_solution.count() - 1) {
		return m_solution.at(pos + 1);
	} else {
		return QPoint(-1,-1);
	}
}

// ============================================================================

int Path::length(const QPoint& start) const
{
	int pos = m_solution.indexOf(start);
	if (pos != -1) {
		return m_solution.count() - pos - 1;
	} else {
		return -1;
	}
}

// ============================================================================

void Path::findNextCell(QPoint& pos)
{
	if (!leftWall(pos)) {
		pos.rx()--;
	} else if (!rightWall(pos)) {
		pos.rx()++;
	} else if (!topWall(pos)) {
		pos.ry()--;
	} else if (!bottomWall(pos)) {
		pos.ry()++;
	}
}

// ============================================================================

bool Path::leftWall(const QPoint& pos) const
{
	return (m_maze->cell(pos.x(), pos.y()).leftWall() || (pos.x() > 0 && m_cells[pos.x() - 1][pos.y()] == true));
}

// ============================================================================

bool Path::rightWall(const QPoint& pos) const
{
	return (m_maze->cell(pos.x(), pos.y()).rightWall() || (pos.x() < m_maze->columns() - 1 && m_cells[pos.x() + 1][pos.y()] == true));
}

// ============================================================================

bool Path::topWall(const QPoint& pos) const
{
	return (m_maze->cell(pos.x(), pos.y()).topWall() || (pos.y() > 0 && m_cells[pos.x()][pos.y() - 1] == true));
}

// ============================================================================

bool Path::bottomWall(const QPoint& pos) const
{
	return (m_maze->cell(pos.x(), pos.y()).bottomWall() || (pos.y() < m_maze->rows() - 1 && m_cells[pos.x()][pos.y() + 1] == true));
}

// ============================================================================
