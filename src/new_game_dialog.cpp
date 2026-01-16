/*
	SPDX-FileCopyrightText: 2007 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "new_game_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

NewGameDialog::NewGameDialog(QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(tr("New Game"));

	// Create widgets
	m_mazes_preview = new QLabel(this);

	m_mazes_algorithm = new QComboBox(this);
	m_mazes_algorithm->setInsertPolicy(QComboBox::InsertAlphabetically);
	connect(m_mazes_algorithm, qOverload<int>(&QComboBox::currentIndexChanged), this, &NewGameDialog::algorithmSelected);
	for (int i = 0; i < 9; ++i) {
		m_mazes_algorithm->addItem(algorithmString(i), i);
	}

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

//-----------------------------------------------------------------------------

QString NewGameDialog::algorithmString(int algorithm)
{
	static const QStringList algorithms = QStringList()
			<< tr("Hunt and Kill")
			<< tr("Kruskal")
			<< tr("Prim")
			<< tr("Recursive Backtracker")
			<< tr("Stack")
			<< tr("Stack 2")
			<< tr("Stack 3")
			<< tr("Stack 4")
			<< tr("Stack 5");
	return algorithms.value(algorithm);
}

//-----------------------------------------------------------------------------

void NewGameDialog::accept()
{
	QSettings settings;

	settings.setValue("New/Algorithm", m_mazes_algorithm->itemData(m_mazes_algorithm->currentIndex()));
	settings.setValue("New/Targets", m_mazes_targets->value());
	settings.setValue("New/Size", m_mazes_size->value());

	QDialog::accept();
}

//-----------------------------------------------------------------------------

void NewGameDialog::algorithmSelected(int index)
{
	if (index != -1) {
		m_mazes_preview->setPixmap( QString(":/preview%1.png").arg( m_mazes_algorithm->itemData(index).toInt()) );
	}
}

//-----------------------------------------------------------------------------
