/***********************************************************************
 *
 * Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015 Graeme Gott <graeme@gottcode.org>
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

#include "theme.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMainWindow>
#include <QPainter>
#include <QSet>
#include <QStandardPaths>
#include <QSvgRenderer>

namespace {
// ============================================================================

struct CornerType
{
	unsigned char renderer;
	unsigned char transform;
};
CornerType corners[15] = {
	{4, 1},
	{4, 2},
	{3, 0},
	{4, 3},
	{2, 1},
	{3, 1},
	{1, 3},
	{4, 0},
	{3, 3},
	{2, 0},
	{1, 2},
	{3, 2},
	{1, 1},
	{1, 0},
	{0, 0}
};

// ============================================================================
}

// ============================================================================

Theme::Theme() :
	m_unit(32),
	m_ratio(1)
{
	m_renderer = new QSvgRenderer;

	// Load theme locations
#if defined(Q_OS_MAC)
	m_locations = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
#elif defined(Q_OS_UNIX)
	m_locations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
	for (int i = 0; i < m_locations.size(); ++i) {
		m_locations[i] += "/games/cutemaze";
	}
	m_locations.prepend(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#elif defined(Q_OS_WIN)
	m_locations = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	m_locations.append(QCoreApplication::applicationDirPath() + "/Themes");
#endif
	m_locations.append(":/games/cutemaze");
}

// ============================================================================

Theme::~Theme()
{
	delete m_renderer;
}

// ============================================================================

QStringList Theme::available() const
{
	QSet<QString> files;
	QDir dir("", "*.svg");
	for (const QString& path : m_locations) {
		if (dir.cd(path)) {
			files += dir.entryList(QDir::Files).toSet();
		}
	}

	QStringList list = QStringList::fromSet(files);
	int count = list.count();
	for (int i = 0; i < count; ++i) {
		QString& theme = list[i];
		theme.remove(theme.length() - 4, 4);
	}
	list.sort();

	return list;
}

// ============================================================================

void Theme::load(const QString& name)
{
	QString theme = name;

	QFileInfo info;
	for (const QString& location : m_locations) {
		info.setFile(location + '/' + name + ".svg");
		if (info.exists()) {
			theme = info.canonicalFilePath();
			break;
		}
	}

	m_renderer->load(theme);
	scale(m_unit);
}

// ============================================================================

void Theme::scale(int unit)
{
	m_unit = unit;

	QRect bounds(0, 0, unit * 2, unit * 2);
	cache("flag", m_pixmap[Flag], bounds);
	cache("start", m_pixmap[Start], bounds);
	cache("target", m_pixmap[Target], bounds);
	for (int i = 0; i < 4; ++i) {
		cache("hint", m_pixmap_rotated[Hint][i], bounds, i * 90);
		cache("marker", m_pixmap_rotated[Marker][i], bounds, i * 90);
		cache("player", m_pixmap_rotated[Player][i], bounds, i * 90);
	}

	bounds.setSize(QSize(unit, unit));
	for (int i = 0; i < 15; ++i) {
		const CornerType& corner = corners[i];
		cache(QString("corner%1").arg(corner.renderer), m_pixmap_corner[i], bounds, corner.transform * 90);
	}

	bounds.setSize(QSize(unit * 2, unit));
	cache("wall", m_pixmap_wall[0], bounds);
	cache("wall", m_pixmap_wall[1], bounds, 90);

	bounds.setSize(QSize(unit * 3, unit * 3));
	cache("background", m_pixmap[Background], bounds);
}

// ============================================================================

void Theme::setDevicePixelRatio(int ratio)
{
	m_ratio = ratio;
	scale(m_unit);
}

// ============================================================================

void Theme::draw(QPainter& painter, int column, int row, enum Element element) const
{
	Q_ASSERT(element != TotalElements);
	painter.drawPixmap(column * 3 * m_unit, row * 3 * m_unit, m_pixmap[element]);
}

// ============================================================================

void Theme::draw(QPainter& painter, int column, int row, enum RotatedElement element, int angle) const
{
	Q_ASSERT(element != TotalRotatedElements);
	Q_ASSERT(angle == 90 || angle == 180 || angle == 270 || angle == 360);
	angle /= 90;
	if (angle == 4) {
		angle = 0;
	}
	painter.drawPixmap(column * 3 * m_unit, row * 3 * m_unit, m_pixmap_rotated[element][angle]);
}

// ============================================================================

void Theme::drawBackground(QPainter& painter) const
{
	painter.fillRect(0, 0, painter.device()->width(), painter.device()->height(), m_pixmap[Background]);
}

// ============================================================================

void Theme::drawCorner(QPainter& painter, int column, int row, unsigned char walls) const
{
	Q_ASSERT(walls > 0);
	Q_ASSERT(walls < 16);
	painter.drawPixmap((column * 3 * m_unit) - m_unit, (row * 3 * m_unit) - m_unit, m_pixmap_corner[walls - 1]);
}

// ============================================================================

void Theme::drawWall(QPainter& painter, int column, int row, bool vertical) const
{
	if (vertical) {
		painter.drawPixmap((column * 3 * m_unit) - m_unit, row * 3 * m_unit, m_pixmap_wall[1]);
	} else {
		painter.drawPixmap(column * 3 * m_unit, (row * 3 * m_unit) - m_unit, m_pixmap_wall[0]);
	}
}

// ============================================================================

void Theme::cache(const QString& element, QPixmap& pixmap, const QRect& bounds, int angle) const
{
	pixmap = QPixmap(bounds.size() * m_ratio);
	pixmap.setDevicePixelRatio(m_ratio);
	pixmap.fill(QColor(255, 255, 255, 0));
	QPainter painter(&pixmap);
	m_renderer->render(&painter, element, bounds);

	// Handle rotated images
	if (angle) {
		Q_ASSERT(angle == 90 || angle == 180 || angle == 270);
		painter.end();
		pixmap = pixmap.transformed(QTransform().rotate(angle));
		pixmap.setDevicePixelRatio(m_ratio);
	}
}

// ============================================================================
