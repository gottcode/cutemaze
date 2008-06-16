/***********************************************************************
 *
 * Copyright (C) 2007-2008 Graeme Gott <graeme@gottcode.org>
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

#include "maze.h"

#include <QSettings>

namespace {
// ============================================================================

QPoint randomNeighbor(QVector< QVector<bool> >& visited, const QPoint& cell)
{
	// Find unvisited neighbors
	QPoint neighbors[4];
	int found = 0;
	if (cell.x() > 0) {
		QPoint n(cell.x() - 1, cell.y());
		if (visited.at(n.x()).at(n.y()) == false) {
			neighbors[found] = n;
			found++;
		}
	}
	if (cell.y() > 0) {
		QPoint n(cell.x(), cell.y() - 1);
		if (visited.at(n.x()).at(n.y()) == false) {
			neighbors[found] = n;
			found++;
		}
	}
	if (cell.y() < visited.at(cell.x()).size() - 1) {
		QPoint n(cell.x(), cell.y() + 1);
		if (visited.at(n.x()).at(n.y()) == false) {
			neighbors[found] = n;
			found++;
		}
	}
	if (cell.x() < visited.size() - 1) {
		QPoint n(cell.x() + 1, cell.y());
		if (visited.at(n.x()).at(n.y()) == false) {
			neighbors[found] = n;
			found++;
		}
	}

	// Return random neighbor
	if (found) {
		const QPoint& n = neighbors[rand() % found];
		visited[n.x()][n.y()] = true;
		return n;
	} else {
		return QPoint(-1,-1);
	}
}

// ============================================================================
}

// Hunt and Kill algorithm
// ============================================================================

void HuntAndKillMaze::generate()
{
	m_visited = QVector< QVector<bool> >(columns(), QVector<bool>(rows()));
	m_unvisited = columns() * rows();

	QPoint current(0, rand() % rows());
	m_visited[current.x()][current.y()] = true;
	m_unvisited--;

	QPoint neighbor;
	while (m_unvisited) {
		neighbor = randomNeighbor(m_visited, current);
		if (neighbor.x() != -1) {
			mergeCells(current, neighbor);
			current = neighbor;
			m_unvisited--;
		} else {
			current = hunt();
		}
	}

	m_visited.clear();
}

// ============================================================================

QPoint HuntAndKillMaze::hunt()
{
	static QPoint direction[4] = {
		QPoint(1, 0),
		QPoint(0, 1),
		QPoint(-1, 0),
		QPoint(0, -1)
	};

	QPoint cell, next;
	for (int c = 0; c < columns(); ++c) {
		cell.setX(c);
		for (int r = 0; r < rows(); ++r) {
			cell.setY(r);
			if (m_visited.at(c).at(r)) {
				continue;
			}
			for (int d = 0; d < 4; ++d) {
				next = cell + direction[d];
				if (next.x() < 0 ||
					next.x() >= columns() ||
					next.y() < 0 ||
					next.y() >= rows()) {
					continue;
				}
				if (m_visited.at(next.x()).at(next.y())) {
					mergeCells(cell, next);
					m_visited[c][r] = true;
					m_unvisited--;
					return cell;
				}
			}
		}
	}
	return QPoint(-1, -1);
}

// ============================================================================

// Kruskal's algorithm
// ============================================================================

void KruskalMaze::generate()
{
	// Generate sets
	m_set_ids = QVector< QVector<Set*> >(columns(), QVector<Set*>(rows()));
	for (int c = 0; c < columns(); ++c) {
		for (int r = 0; r < rows(); ++r) {
			m_sets.append(QList<QPoint>() << QPoint(c, r));
			m_set_ids[c][r] = &m_sets.last();
		}
	}

	while (m_sets.size() > 1) {
		Set* set1 = &m_sets.first();

		// Find random cell
		const QPoint& cell = set1->at(rand() % set1->size());

		// Find random neighbor of cell
		QPoint cell2(cell);
		if (rand() % 2) {
			cell2.rx()++;
		} else {
			cell2.ry()++;
		}
		if (cell2.x() >= columns() || cell2.y() >= rows()) {
			continue;
		}

		// Find set containing second cell
		Set* set2 = m_set_ids.at(cell2.x()).at(cell2.y());

		// Merge sets if they are different
		if (set1 != set2) {
			mergeCells(cell, cell2);
			int size = set1->size();
			for (int i = 0; i < size; ++i) {
				const QPoint& cell3 = set1->at(i);
				m_set_ids[cell3.x()][cell3.y()] = set2;
			}
			*set2 += *set1;
			m_sets.removeFirst();
		}
	}

	m_sets.clear();
	m_set_ids.clear();
}

// ============================================================================

// Prim's algorithm
// ============================================================================

void PrimMaze::generate()
{
	// Generate cell lists
	m_regions = QVector< QVector<int> >(columns(), QVector<int>(rows(), 0));

	// Move first cell
	QPoint cell(0, rand() % columns());
	m_regions[0][cell.y()] = 2;
	moveNeighbors(cell);

	// Move remaining cells
	while (!m_frontier.isEmpty()) {
		cell = m_frontier.takeAt( rand() % m_frontier.size() );
		mergeRandomNeighbor(cell);
		m_regions[cell.x()][cell.y()] = 2;
		moveNeighbors(cell);
	}

	m_regions.clear();
}

// ============================================================================

void PrimMaze::moveNeighbors(const QPoint& cell)
{
	QList<QPoint> n = neighbors(cell);
	for (int i = 0; i < n.size(); ++i) {
		const QPoint& current = n.at(i);
		int& ref = m_regions[current.x()][current.y()];
		if (ref == 0) {
			ref = 1;
			m_frontier.append(current);
		}
	}
}

// ============================================================================

void PrimMaze::mergeRandomNeighbor(const QPoint& cell)
{
	QList<QPoint> cells;

	QList<QPoint> n = neighbors(cell);
	for (int i = 0; i < n.size(); ++i) {
		const QPoint& current = n.at(i);
		if (m_regions.at(current.x()).at(current.y()) == 2) {
			cells.append(current);
		}
	}

	mergeCells( cell, cells.at(rand() % cells.size()) );
}

// ============================================================================

QList<QPoint> PrimMaze::neighbors(const QPoint& cell)
{
	QList<QPoint> n;
	if (cell.x() > 0) {
		n.append(cell + QPoint(-1, 0));
	}
	if (cell.y() > 0) {
		n.append(cell + QPoint(0, -1));
	}
	if (cell.y() < rows() - 1) {
		n.append(cell + QPoint(0, 1));
	}
	if (cell.x() < columns() - 1) {
		n.append(cell + QPoint(1, 0));
	}
	return n;
}

// ============================================================================

// Recursive Backtracker algorithm
// ============================================================================

void RecursiveBacktrackerMaze::generate()
{
	m_visited = QVector< QVector<bool> >(columns(), QVector<bool>(rows()));

	QPoint start(0, rand() % rows());
	m_visited[start.x()][start.y()] = true;
	makePath(start);

	m_visited.clear();
}

// ============================================================================

void RecursiveBacktrackerMaze::makePath(const QPoint& current)
{
	QPoint neighbor;
	while ( (neighbor = randomNeighbor(m_visited, current)).x() != -1 ) {
		mergeCells(current, neighbor);
		makePath(neighbor);
	}
}

// ============================================================================

// Stack algorithm
// ============================================================================

void StackMaze::generate()
{
	// Generate cell lists
	m_visited = QVector< QVector<bool> >(columns(), QVector<bool>(rows()));
	QList<QPoint> active;

	// Start maze
	QPoint start(0, rand() % rows());
	m_visited[start.x()][start.y()] = true;
	active.append(start);

	// Loop through active list
	QPoint cell, neighbor;
	int pos;
	while (!active.isEmpty()) {
		pos = nextActive(active.size());
		cell = active.at(pos);
		neighbor = randomNeighbor(m_visited, cell);
		if (neighbor.x() != -1) {
			mergeCells(cell, neighbor);
			active.append(neighbor);
		} else {
			active.takeAt(pos);
		}
	}

	m_visited.clear();
}

// ============================================================================

int StackMaze::nextActive(int size)
{
	return size - 1;
}

// ============================================================================

int Stack2Maze::nextActive(int size)
{
	if (rand() % 2 != 0) {
		return size - 1;
	} else {
		return rand() % (size);
	}
}

// ============================================================================

int Stack3Maze::nextActive(int size)
{
	return rand() % size;
}

// ============================================================================

int Stack4Maze::nextActive(int size)
{
	int recent = 3 < size ? 3 : size;
	return size - (rand() % recent) - 1;
}

// ============================================================================

int Stack5Maze::nextActive(int size)
{
	switch (rand() % 3) {
	case 0:
		return 0;
	case 1:
		return rand() % size;
	case 2:
	default:
		return size - 1;
	}
}

// ============================================================================

// Maze class
// ============================================================================

void Maze::generate(int columns, int rows)
{
	m_columns = columns;
	m_rows = rows;
	m_cells = QVector< QVector<Cell> >(m_columns, QVector<Cell>(m_rows));
	generate();
}

// ============================================================================

bool Maze::load()
{
	// Read data from disk
	QByteArray data = QSettings().value("Current/Progress").toByteArray();
	if (data.isEmpty()) {
		return false;
	}

	// Decompress data
	data = qUncompress(data);
	if (data.isEmpty()) {
		return false;
	}

	// Deserialize data
	QDataStream stream(&data, QIODevice::ReadOnly);
	stream.setVersion(QDataStream::Qt_4_3);
	for (int c = 0; c < m_columns; ++c) {
		for (int r = 0; r < m_rows; ++r) {
			stream >> m_cells[c][r];
			if (stream.status() != QDataStream::Ok) {
				return false;
			}
		}
	}

	if (!stream.atEnd() || stream.status() != QDataStream::Ok) {
		return false;
	}

	return true;
}

// ============================================================================

void Maze::save() const
{
	// Serialize data
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream.setVersion(QDataStream::Qt_4_3);
	for (int c = 0; c < m_columns; ++c) {
		for (int r = 0; r < m_rows; ++r) {
			stream << m_cells[c][r];
		}
	}

	// Compress data
	data = qCompress(data, 9);

	// Write data to disk
	QSettings().setValue("Current/Progress", data);
}

// ============================================================================

void Maze::mergeCells(const QPoint& cell1, const QPoint& cell2)
{
	if (cell1.y() == cell2.y()) {
		if (cell2.x() > cell1.x()) {
			m_cells[cell1.x()][cell1.y()].removeRightWall();
			m_cells[cell2.x()][cell2.y()].removeLeftWall();
		} else if (cell2.x() < cell1.x()) {
			m_cells[cell1.x()][cell1.y()].removeLeftWall();
			m_cells[cell2.x()][cell2.y()].removeRightWall();
		}
	} else if (cell1.x() == cell2.x()) {
		if (cell2.y() > cell1.y()) {
			m_cells[cell1.x()][cell1.y()].removeBottomWall();
			m_cells[cell2.x()][cell2.y()].removeTopWall();
		} else if (cell2.y() < cell1.y()) {
			m_cells[cell1.x()][cell1.y()].removeTopWall();
			m_cells[cell2.x()][cell2.y()].removeBottomWall();
		}
	}
}

// ============================================================================
