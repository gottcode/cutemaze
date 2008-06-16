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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QList>
class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QSpinBox;
class Theme;

class Settings : public QDialog
{
	Q_OBJECT
public:
	Settings(QWidget* parent = 0);
	~Settings();

signals:
	void settingsChanged();

public slots:
	virtual void accept();
	virtual void reject();
	void algorithmSelected(int index);
	void themeSelected(const QString& theme);
	void addTheme();
	void removeTheme();

private:
	void loadSettings();
	void generatePreview();

	QCheckBox* m_gameplay_path;
	QCheckBox* m_gameplay_steps;
	QCheckBox* m_gameplay_time;
	QCheckBox* m_gameplay_smooth;

	QLabel* m_mazes_preview;
	QComboBox* m_mazes_algorithm;
	QSpinBox* m_mazes_targets;
	QSpinBox* m_mazes_size;

	QListWidget* m_themes_selector;
	QLabel* m_themes_preview;
	QPushButton* m_themes_remove_button;
	Theme* m_theme;
};

#endif // SETTINGS_H
