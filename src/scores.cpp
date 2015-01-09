/***********************************************************************
 *
 * Copyright (C) 2008, 2009, 2012, 2014, 2015 Graeme Gott <graeme@gottcode.org>
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

#include "scores.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QScrollBar>
#include <QSettings>
#include <QTime>
#include <QTreeWidget>
#include <QVBoxLayout>

#if defined(Q_OS_UNIX)
#include <pwd.h>
#include <unistd.h>

#elif defined(Q_OS_WIN)
#include <lmcons.h>
#include <windows.h>

#endif

namespace {
// ============================================================================

class Score : public QTreeWidgetItem
{
public:
	Score(int seconds, int steps, int algorithm, int size);

	bool operator<(const QTreeWidgetItem& other) const
	{
		return data(1, Qt::UserRole).toInt() < other.data(1, Qt::UserRole).toInt();
	}
};

// ============================================================================

Score::Score(int seconds, int steps, int algorithm, int size)
:	QTreeWidgetItem(QTreeWidgetItem::UserType + 1)
{
	int score = seconds ? ((steps * size) / seconds) : (steps * size);
	setText(1, QString::number(score));
	setData(1, Qt::UserRole, score);
	setText(2, QTime(0, 0, 0).addSecs(seconds).toString("hh:mm:ss"));
	setData(2, Qt::UserRole, QString::number(seconds));
	setText(3, QString::number(steps));
	setData(4, Qt::UserRole, QString::number(algorithm));
	setText(5, QString::number(size));

	setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
	setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
	setTextAlignment(3, Qt::AlignRight | Qt::AlignVCenter);
	setTextAlignment(4, Qt::AlignCenter | Qt::AlignVCenter);
	setTextAlignment(5, Qt::AlignRight | Qt::AlignVCenter);

	static QStringList algorithms = QStringList()
		<< QLabel::tr("Hunt and Kill")
		<< QLabel::tr("Kruskal")
		<< QLabel::tr("Prim")
		<< QLabel::tr("Recursive Backtracker")
		<< QLabel::tr("Stack")
		<< QLabel::tr("Stack 2")
		<< QLabel::tr("Stack 3")
		<< QLabel::tr("Stack 4")
		<< QLabel::tr("Stack 5");
	Q_ASSERT(algorithm > -1);
	Q_ASSERT(algorithm < algorithms.size());
	setText(4, algorithms.at(algorithm));
}

// ============================================================================
}

// ============================================================================

Scores::Scores(QWidget* parent)
:	QDialog(parent)
{
	setWindowTitle(tr("CuteMaze Scores"));

	QVBoxLayout* layout = new QVBoxLayout(this);

	m_board = new QTreeWidget(this);
	m_board->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_board->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_board->setRootIsDecorated(false);
	m_board->setColumnCount(5);
	m_board->setHeaderLabels(QStringList() << tr("Name") << tr("Score") << tr("Time") << tr("Steps") << tr("Algorithm") << tr("Size"));
	m_board->header()->setStretchLastSection(false);
	m_board->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	layout->addWidget(m_board);

	layout->addSpacing(12);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::rejected, this, &Scores::reject);
	layout->addWidget(buttons);

	read();
}

// ============================================================================

void Scores::addScore(int steps, int seconds, int algorithm, int size)
{
	// Check if it is a high score
	int value = seconds ? ((steps * size) / seconds) : (steps * size);
	int count = m_board->topLevelItemCount();
	Q_ASSERT(count < 11);
	if (count == 10 && value < m_board->topLevelItem(count - 1)->text(1).toInt()) {
		return;
	}

	// Get player's name
	QString name;
#if defined(Q_OS_UNIX)
	{
		passwd* pws = getpwuid(geteuid());
		if (pws) {
			name = pws->pw_gecos;
			if (name.isEmpty()) {
				name = pws->pw_name;
			}
		}
	}
#elif defined(Q_OS_WIN)
	{
		WCHAR buffer[UNLEN + 1];
		DWORD count = sizeof(buffer);
		if (GetUserName(buffer, &count)) {
			name = QString::fromStdWString(buffer);
		}
	}
#endif
	bool ok = true;
	name = QInputDialog::getText(parentWidget(), tr("Congratulations!"), tr("Your score has made the top ten.\nPlease enter your name:"), QLineEdit::Normal, name, &ok);
	if (!ok || name.isEmpty()) {
		return;
	}

	// Create score
	QTreeWidgetItem* score = new Score(seconds, steps, algorithm, size);
	score->setText(0, name);
	m_board->addTopLevelItem(score);
	m_board->clearSelection();
	score->setSelected(true);
	updateItems();

	// Save items
	QStringList values;
	QTreeWidgetItem* item;
	count = m_board->topLevelItemCount();
	for (int i = 0; i < count; ++i) {
		item = m_board->topLevelItem(i);
		values += QString("%1:%2:%3:%4:%5") .arg(item->text(0)) .arg(item->data(2, Qt::UserRole).toInt()) .arg(item->text(3)) .arg(item->data(4, Qt::UserRole).toInt()) .arg(item->text(5).toInt());
	}
	QSettings().setValue("Scores", values);

	show();
}

// ============================================================================

void Scores::read()
{
	QStringList data = QSettings().value("Scores").toStringList();
	for (const QString& s : data) {
		QStringList values = s.split(':');
		if (values.size() != 5) {
			continue;
		}
		Score* score = new Score(values[1].toInt(), values[2].toInt(), values[3].toInt(), values[4].toInt());
		score->setText(0, values[0]);
		m_board->addTopLevelItem(score);
	}
	updateItems();
}

// ============================================================================

void Scores::updateItems()
{
	// Sort items
	m_board->sortItems(1, Qt::DescendingOrder);
	while (m_board->topLevelItemCount() > 10) {
		m_board->takeTopLevelItem(10);
	}

	// Find minimum size
	int frame = m_board->frameWidth() * 2;
	int width = frame;
	for (int i = 0; i < 6; ++i) {
		width += m_board->columnWidth(i);
	}
	m_board->setMinimumSize(width, frame + m_board->header()->height() + m_board->sizeHintForRow(0) * 10);
}

// ============================================================================
