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

#include "theme.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMainWindow>
#include <QPainter>
#include <QSet>
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

Theme::Theme()
:	m_unit(32)
{
	for (int i = 0; i < TotalElements; ++i) {
		m_svg[i] = new QSvgRenderer;
	}
	for (int i = 0; i < TotalRotatedElements; ++i) {
		m_svg_rotated[i] = new QSvgRenderer;
	}
	for (int i = 0; i < 5; ++i) {
		m_svg_corner[i] = new QSvgRenderer;
	}
	m_svg_wall = new QSvgRenderer;

	// Load theme locations
#if defined(Q_OS_MAC)
	m_locations.append(QDir::homePath() + "/Library/Application Support/GottCode/CuteMaze");
	m_locations.append("/Library/Application Support/GottCode/CuteMaze");
	m_locations.append(":/games/cutemaze");
#elif defined(Q_OS_UNIX)
	QString xdg;
	xdg = getenv("$XDG_DATA_HOME");
	if (xdg.isEmpty()) {
		xdg = QDir::homePath() + "/.local/share/";
	}
	m_locations = xdg.split(":");
	xdg = getenv("$XDG_DATA_DIRS");
	if (xdg.isEmpty()) {
		xdg = "/usr/local/share/:/usr/share/";
	}
	m_locations += xdg.split(":");
	m_locations += ":";
	for (int i = 0; i < m_locations.size(); ++i) {
		m_locations[i] += "/games/cutemaze";
	}
#elif defined(Q_OS_WIN32)
	m_locations.append(QDir::homePath() + "/Application Data/GottCode/CuteMaze");
	m_locations.append(QCoreApplication::applicationDirPath() + "/Themes");
	m_locations.append(":/games/cutemaze");
#endif
}

// ============================================================================

Theme::~Theme()
{
	for (int i = 0; i < TotalElements; ++i) {
		delete m_svg[i];
	}
	for (int i = 0; i < TotalRotatedElements; ++i) {
		delete m_svg_rotated[i];
	}
	for (int i = 0; i < 5; ++i) {
		delete m_svg_corner[i];
	}
	delete m_svg_wall;
}

// ============================================================================

QStringList Theme::available() const
{
	QSet<QString> dirs;
	dirs.insert("Mouse");
	dirs.insert("Penguin");

	QDir dir;
	foreach (const QString& path, m_locations) {
		if (dir.cd(path)) {
			dirs += dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).toSet();
		}
	}

	QStringList list = QStringList::fromSet(dirs);
	list.sort();
	return list;
}

// ============================================================================

void Theme::load(const QString& name)
{
	QIcon icon(findFile(name, "player.svg"));
	qApp->setWindowIcon(icon);
	foreach (QWidget* window, qApp->topLevelWidgets()) {
		if (qobject_cast<QMainWindow*>(window)) {
			window->setWindowIcon(icon);
		}
	}

	m_svg[Background]->load(findFile(name, "background.svg"));
	m_svg[Flag]->load(findFile(name, "flag.svg"));
	m_svg[Start]->load(findFile(name, "start.svg"));
	m_svg[Target]->load(findFile(name, "target.svg"));
	m_svg_rotated[Marker]->load(findFile(name, "marker.svg"));
	m_svg_rotated[Player]->load(findFile(name, "player.svg"));
	for (int i = 0; i < 5; ++i) {
		m_svg_corner[i]->load(findFile(name, QString("corner%1.svg").arg(i)));
	}
	m_svg_wall->load(findFile(name, "wall.svg"));

	scale(m_unit);
}

// ============================================================================

void Theme::scale(int unit)
{
	Q_ASSERT(unit > 31);
	m_unit = unit;

	QRect bounds(0, 0, unit * 2, unit * 2);
	cache(m_svg[Flag], m_pixmap[Flag], bounds);
	cache(m_svg[Start], m_pixmap[Start], bounds);
	cache(m_svg[Target], m_pixmap[Target], bounds);
	for (int i = 0; i < 4; ++i) {
		cache(m_svg_rotated[Marker], m_pixmap_rotated[Marker][i], bounds, i * 90);
		cache(m_svg_rotated[Player], m_pixmap_rotated[Player][i], bounds, i * 90);
	}

	bounds.setSize(QSize(unit, unit));
	for (int i = 0; i < 15; ++i) {
		const CornerType& corner = corners[i];
		cache(m_svg_corner[corner.renderer], m_pixmap_corner[i], bounds, corner.transform * 90);
	}

	bounds.setSize(QSize(unit * 2, unit));
	cache(m_svg_wall, m_pixmap_wall[0], bounds);
	cache(m_svg_wall, m_pixmap_wall[1], bounds, 90);

	bounds.setSize(QSize(unit * 3, unit * 3));
	cache(m_svg[Background], m_pixmap[Background], bounds);
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
	if (angle == 4)
		angle = 0;
 	painter.drawPixmap(column * 3 * m_unit, row * 3 * m_unit, m_pixmap_rotated[element][angle]);
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

void Theme::cache(QSvgRenderer* svg, QPixmap& pixmap, const QRect& bounds, int angle) const
{
	pixmap = QPixmap(bounds.size());
	pixmap.fill(QColor(255, 255, 255, 0));
	QPainter painter(&pixmap);
	svg->render(&painter, bounds);

	// Handle rotated images
	if (angle) {
		Q_ASSERT(angle == 90 || angle == 180 || angle == 270);
		painter.end();
		pixmap = pixmap.transformed(QTransform().rotate(angle), Qt::SmoothTransformation);
	}
}

// ============================================================================

QString Theme::findFile(const QString& theme, const QString& file) const
{
	QFileInfo info;
	foreach (const QString& location, m_locations) {
		info.setFile(location + "/" + theme, file);
		if (info.exists()) {
			return info.canonicalFilePath();
		}
	}
	return file;
}

// ============================================================================
