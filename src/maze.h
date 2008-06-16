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

#ifndef MAZE_H
#define MAZE_H

#include "cell.h"

#include <QList>
#include <QPoint>
#include <QVector>

class Maze
{
public:
	virtual ~Maze()
		{ }

	int columns() const
		{ return m_columns; }
	int rows() const
		{ return m_rows; }
	const Cell& cell(int column, int row) const
		{ return m_cells.at(column).at(row); }
	Cell& cellMutable(int column, int row)
		{ return m_cells[column][row]; }

	void generate(int columns, int rows);
	bool load();
	void save() const;

protected:
	void mergeCells(const QPoint& cell1, const QPoint& cell2);

private:
	virtual void generate() = 0;

	int m_columns;
	int m_rows;
	QVector< QVector<Cell> > m_cells;
};


class HuntAndKillMaze : public Maze
{
private:
	virtual void generate();
	QPoint hunt();

	QVector< QVector<bool> > m_visited;
	int m_unvisited;
};


class KruskalMaze : public Maze
{
private:
	virtual void generate();

	typedef QList<QPoint> Set;
	QList<Set> m_sets;
	QVector< QVector<Set*> > m_set_ids;
};


class PrimMaze : public Maze
{
private:
	virtual void generate();
	void moveNeighbors(const QPoint& cell);
	void mergeRandomNeighbor(const QPoint& cell);
	QList<QPoint> neighbors(const QPoint& cell);

	QList<QPoint> m_frontier;
	QVector< QVector<int> > m_regions;
};


class RecursiveBacktrackerMaze : public Maze
{
private:
	virtual void generate();
	void makePath(const QPoint& current);

	QVector< QVector<bool> > m_visited;
};


class StackMaze : public Maze
{
private:
	virtual void generate();
	virtual int nextActive(int size);

	QVector< QVector<bool> > m_visited;
};


class Stack2Maze : public StackMaze
{
private:
	virtual int nextActive(int size);
};


class Stack3Maze : public StackMaze
{
private:
	virtual int nextActive(int size);
};


class Stack4Maze : public StackMaze
{
private:
	virtual int nextActive(int size);
};


class Stack5Maze : public StackMaze
{
private:
	virtual int nextActive(int size);
};

#endif // MAZE_H
