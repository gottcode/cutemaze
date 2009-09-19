/***********************************************************************
 *
 * Copyright (C) 2007-2009 Graeme Gott <graeme@gottcode.org>
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

#ifndef NEW_GAME_DIALOG_H
#define NEW_GAME_DIALOG_H

#include <QDialog>
class QComboBox;
class QLabel;
class QSpinBox;

class NewGameDialog : public QDialog
{
	Q_OBJECT
public:
	NewGameDialog(QWidget* parent = 0);

public slots:
	virtual void accept();

private slots:
	void algorithmSelected(int index);

private:
	QLabel* m_mazes_preview;
	QComboBox* m_mazes_algorithm;
	QSpinBox* m_mazes_targets;
	QSpinBox* m_mazes_size;
};

#endif // NEW_GAME_DIALOG_H
