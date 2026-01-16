/*
	SPDX-FileCopyrightText: 2007 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "cell.h"

#include <QDataStream>

//-----------------------------------------------------------------------------

Cell::Cell()
	: m_left_wall(true)
	, m_right_wall(true)
	, m_top_wall(true)
	, m_bottom_wall(true)
	, m_flag(false)
	, m_path_marker(0)
{
}

//-----------------------------------------------------------------------------

void Cell::setPathMarker(int angle)
{
	Q_ASSERT(angle == 90 || angle == 180 || angle == 270 || angle == 360);
	m_path_marker = angle / 90;
}

//-----------------------------------------------------------------------------

QDataStream& operator<<(QDataStream& stream, const Cell& cell)
{
	return stream << cell.m_path_marker << cell.m_flag;
}

QDataStream& operator>>(QDataStream& stream, Cell& cell)
{
	return stream >> cell.m_path_marker >> cell.m_flag;
}

//-----------------------------------------------------------------------------
