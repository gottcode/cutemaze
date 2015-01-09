/***********************************************************************
 *
 * Copyright (C) 2007, 2008, 2009, 2015 Graeme Gott <graeme@gottcode.org>
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

#ifndef THEME_H
#define THEME_H

#include <QPixmap>
#include <QString>
class QPainter;
class QSvgRenderer;

class Theme
{
public:
	Theme();
	~Theme();

	QStringList available() const;
	void load(const QString& name);
	void scale(int unit);
	void setDevicePixelRatio(int ratio);

	enum Element {
		Background,
		Flag,
		Start,
		Target,
		TotalElements
	};
	enum RotatedElement {
		Hint,
		Marker,
		Player,
		TotalRotatedElements
	};
	void draw(QPainter& painter, int column, int row, enum Element element) const;
	void draw(QPainter& painter, int column, int row, enum RotatedElement element, int angle) const;
	void drawBackground(QPainter& painter) const;
	void drawCorner(QPainter& painter, int column, int row, unsigned char walls) const;
	void drawWall(QPainter& painter, int column, int row, bool vertical = false) const;

private:
	void cache(const QString& element, QPixmap& pixmap, const QRect& bounds, int angle = 0) const;
	QString findFile(const QString& theme, const QString& file) const;

	QStringList m_locations;
	QSvgRenderer* m_renderer;
	QPixmap m_pixmap[TotalElements];
	QPixmap m_pixmap_rotated[TotalRotatedElements][4];
	QPixmap m_pixmap_corner[15];
	QPixmap m_pixmap_wall[2];
	int m_unit;
	int m_ratio;
};

#endif // THEME_H
