/*
	SPDX-FileCopyrightText: 2007-2008 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_SETTINGS_H
#define CUTEMAZE_SETTINGS_H

class Theme;

#include <QDialog>
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;

class Settings : public QDialog
{
	Q_OBJECT

public:
	explicit Settings(QWidget* parent = nullptr);
	~Settings();

signals:
	void settingsChanged();

public slots:
	void accept() override;

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
