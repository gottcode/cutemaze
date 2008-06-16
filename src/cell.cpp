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

#include "cell.h"

#include <QDataStream>

// ============================================================================

Cell::Cell()
:	m_left_wall(true),
	m_right_wall(true),
	m_top_wall(true),
	m_bottom_wall(true),
	m_flag(false),
	m_path_marker(0)
{
}

// ============================================================================

void Cell::setPathMarker(int angle)
{
	Q_ASSERT(angle == 90 || angle == 180 || angle == 270 || angle == 360);
	m_path_marker = angle / 90;
}

// ============================================================================

QDataStream& operator<<(QDataStream& stream, const Cell& cell)
{
	return stream << cell.m_path_marker << cell.m_flag;
}

QDataStream& operator>>(QDataStream& stream, Cell& cell)
{
	return stream >> cell.m_path_marker >> cell.m_flag;
}

// ============================================================================
