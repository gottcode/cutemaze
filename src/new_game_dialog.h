/*
	SPDX-FileCopyrightText: 2007 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_NEW_GAME_DIALOG_H
#define CUTEMAZE_NEW_GAME_DIALOG_H

#include <QDialog>
class QComboBox;
class QLabel;
class QSpinBox;
class QString;

class NewGameDialog : public QDialog
{
	Q_OBJECT

public:
	explicit NewGameDialog(QWidget* parent = nullptr);

	static QString algorithmString(int algorithm);

public Q_SLOTS:
	void accept() override;

private Q_SLOTS:
	void algorithmSelected(int index);

private:
	QLabel* m_mazes_preview;
	QComboBox* m_mazes_algorithm;
	QSpinBox* m_mazes_targets;
	QSpinBox* m_mazes_size;
};

#endif // CUTEMAZE_NEW_GAME_DIALOG_H
