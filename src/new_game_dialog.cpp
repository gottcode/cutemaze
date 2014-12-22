/***********************************************************************
 *
 * Copyright (C) 2007, 2008, 2009, 2012, 2014 Graeme Gott <graeme@gottcode.org>
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

#include "new_game_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

// ============================================================================

NewGameDialog::NewGameDialog(QWidget* parent)
:	QDialog(parent)
{
	setWindowTitle(tr("New Game"));

	// Create widgets
	m_mazes_preview = new QLabel(this);

	m_mazes_algorithm = new QComboBox(this);
	m_mazes_algorithm->setInsertPolicy(QComboBox::InsertAlphabetically);
	connect(m_mazes_algorithm, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &NewGameDialog::algorithmSelected);
	m_mazes_algorithm->addItem(tr("Hunt and Kill"), 0);
	m_mazes_algorithm->addItem(tr("Kruskal"), 1);
	m_mazes_algorithm->addItem(tr("Prim"), 2);
	m_mazes_algorithm->addItem(tr("Recursive Backtracker"), 3);
	m_mazes_algorithm->addItem(tr("Stack"), 4);
	m_mazes_algorithm->addItem(tr("Stack 2"), 5);
	m_mazes_algorithm->addItem(tr("Stack 3"), 6);
	m_mazes_algorithm->addItem(tr("Stack 4"), 7);
	m_mazes_algorithm->addItem(tr("Stack 5"), 8);

	m_mazes_targets = new QSpinBox(this);
	m_mazes_targets->setRange(1, 99);

	m_mazes_size = new QSpinBox(this);
	m_mazes_size->setRange(10, 99);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &NewGameDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &NewGameDialog::reject);

	// Load settings
	QSettings settings;
	int algorithm = settings.value("New/Algorithm", 4).toInt();
	m_mazes_algorithm->setCurrentIndex(m_mazes_algorithm->findData(algorithm));
	m_mazes_targets->setValue(settings.value("New/Targets", 3).toInt());
	m_mazes_size->setValue(settings.value("New/Size", 50).toInt());

	// Lay out dialog
	QFormLayout* contents_layout = new QFormLayout;
	contents_layout->addRow("", m_mazes_preview);
	contents_layout->addRow(tr("Algorithm:"), m_mazes_algorithm);
	contents_layout->addRow(tr("Targets:"), m_mazes_targets);
	contents_layout->addRow(tr("Size:"), m_mazes_size);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addLayout(contents_layout);
	layout->addWidget(buttons);
}

// ============================================================================

void NewGameDialog::accept()
{
	QSettings settings;

	settings.setValue("New/Algorithm", m_mazes_algorithm->itemData(m_mazes_algorithm->currentIndex()));
	settings.setValue("New/Targets", m_mazes_targets->value());
	settings.setValue("New/Size", m_mazes_size->value());

	QDialog::accept();
}

// ============================================================================

void NewGameDialog::algorithmSelected(int index)
{
	if (index != -1) {
		m_mazes_preview->setPixmap( QString(":/preview%1.png").arg( m_mazes_algorithm->itemData(index).toInt()) );
	}
}

// ============================================================================
