/*
	SPDX-FileCopyrightText: 2007-2015 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_THEME_H
#define CUTEMAZE_THEME_H

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

private:
	QStringList m_locations;
	QSvgRenderer* m_renderer;
	QPixmap m_pixmap[TotalElements];
	QPixmap m_pixmap_rotated[TotalRotatedElements][4];
	QPixmap m_pixmap_corner[15];
	QPixmap m_pixmap_wall[2];
	int m_unit;
	int m_ratio;
};

#endif // CUTEMAZE_THEME_H
