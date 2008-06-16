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

#ifndef CELL_H
#define CELL_H

class QDataStream;

class Cell
{
public:
	Cell();

	bool leftWall() const
		{ return m_left_wall; }
	bool rightWall() const
		{ return m_right_wall; }
	bool topWall() const
		{ return m_top_wall; }
	bool bottomWall() const
		{ return m_bottom_wall; }
	void removeLeftWall()
		{ m_left_wall = false; }
	void removeRightWall()
		{ m_right_wall = false; }
	void removeTopWall()
		{ m_top_wall = false; }
	void removeBottomWall()
		{ m_bottom_wall = false; }

	int pathMarker() const
		{ return m_path_marker * 90; }
	void setPathMarker(int angle);

	bool flag() const
		{ return m_flag; }
	void toggleFlag()
		{ m_flag = !m_flag; }

    friend QDataStream& operator<<(QDataStream&, const Cell&);
    friend QDataStream& operator>>(QDataStream&, Cell&);

private:
	bool m_left_wall;
	bool m_right_wall;
	bool m_top_wall;
	bool m_bottom_wall;
	bool m_flag;
	unsigned char m_path_marker;
};

#endif // CELL_H
