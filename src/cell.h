/*
	SPDX-FileCopyrightText: 2007 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_CELL_H
#define CUTEMAZE_CELL_H

class QDataStream;

class Cell
{
public:
	explicit Cell();

	bool leftWall() const
	{
		return m_left_wall;
	}

	bool rightWall() const
	{
		return m_right_wall;
	}

	bool topWall() const
	{
		return m_top_wall;
	}

	bool bottomWall() const
	{
		return m_bottom_wall;
	}


	void removeLeftWall()
	{
		m_left_wall = false;
	}

	void removeRightWall()
	{
		m_right_wall = false;
	}

	void removeTopWall()
	{
		m_top_wall = false;
	}

	void removeBottomWall()
	{
		m_bottom_wall = false;
	}


	int pathMarker() const
	{
		return m_path_marker * 90;
	}

	void setPathMarker(int angle);


	bool flag() const
	{
		return m_flag;
	}

	void toggleFlag()
	{
		m_flag = !m_flag;
	}


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

#endif // CUTEMAZE_CELL_H
