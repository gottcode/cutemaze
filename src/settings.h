/*
	SPDX-FileCopyrightText: 2007-2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_SETTINGS_H
#define CUTEMAZE_SETTINGS_H

#include <QDialog>
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
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

private slots:
	void themeSelected(const QString& theme);
	void addTheme();
	void removeTheme();

private:
	void loadSettings();
	void generatePreview();

private:
	QCheckBox* m_gameplay_path;
	QCheckBox* m_gameplay_steps;
	QCheckBox* m_gameplay_time;
	QCheckBox* m_gameplay_smooth;

	QComboBox* m_themes_selector;
	QLabel* m_themes_preview;
	QPushButton* m_themes_remove_button;
	Theme* m_theme;
};

#endif // CUTEMAZE_SETTINGS_H
