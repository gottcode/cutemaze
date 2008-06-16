/***********************************************************************
 *
 * Copyright (C) 2008 Graeme Gott <graeme@gottcode.org>
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

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QScrollBar>
#include <QSettings>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#if defined(Q_OS_UNIX)
#include <pwd.h>
#include <unistd.h>

#elif defined(Q_OS_WIN32)
#include <lmcons.h>
#include <windows.h>

#endif

namespace {
// ============================================================================

class Score : public QTreeWidgetItem
{
public:
	Score(int seconds, int steps, int algorithm);

	virtual bool operator<(const QTreeWidgetItem& other) const;
};

// ============================================================================

Score::Score(int seconds, int steps, int algorithm)
:	QTreeWidgetItem(1001)
{
	setData(1, Qt::UserRole, QString::number(seconds));
	setText(2, QString::number(steps));
	setData(3, Qt::UserRole, QString::number(algorithm));

	setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
	setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
	setTextAlignment(3, Qt::AlignCenter | Qt::AlignVCenter);

	int minutes = seconds / 60;
	seconds -= (minutes * 60);
	int hours = seconds / 3600;
	seconds -= (hours * 3600);
	setText(1, QString("%1:%2:%3") .arg(hours, 2, 10, QLatin1Char('0')) .arg(minutes, 2, 10, QLatin1Char('0')) .arg(seconds, 2, 10, QLatin1Char('0')) );

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
	setText(3, algorithms.at(algorithm));
}

// ============================================================================

bool Score::operator<(const QTreeWidgetItem& other) const
{
	if (other.type() == type()) {
		int seconds = data(1, Qt::UserRole).toInt();
		int other_seconds = other.data(1, Qt::UserRole).toInt();
		if (seconds == other_seconds) {
			return text(2).toInt() < other.text(2).toInt();
		} else {
			return seconds < other_seconds;
		}
	} else {
		return QTreeWidgetItem::operator<(other);
	}
}

// ============================================================================

class ScoreBoard : public QTreeWidget
{
public:
	ScoreBoard(QWidget* parent = 0);

	void updateItems();
};

// ============================================================================

ScoreBoard::ScoreBoard(QWidget* parent)
:	QTreeWidget(parent)
{
	setRootIsDecorated(false);
	setColumnCount(4);
	setHeaderLabels(QStringList() << tr("Name") << tr("Time") << tr("Steps") << tr("Algorithm"));
	header()->setStretchLastSection(false);
	header()->setResizeMode(QHeaderView::ResizeToContents);
}

// ============================================================================

void ScoreBoard::updateItems()
{
	// Sort items
	sortItems(1, Qt::AscendingOrder);
	while (topLevelItemCount() > 10) {
		takeTopLevelItem(10);
	}

	// Update minimum width
	int w = (frameWidth() * 2) + verticalScrollBar()->sizeHint().width();
	for (int i = 0; i < 4; ++i) {
		w += columnWidth(i);
	}
	setMinimumSize(w, header()->height() + sizeHintForRow(0) * 10 + frameWidth() * 2);
}

// ============================================================================
}

// ============================================================================

Scores::Scores(QWidget* parent)
:	QDialog(parent)
{
	setWindowTitle(tr("CuteMaze Scores"));

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	m_sizes = new QComboBox(this);

	QHBoxLayout* section_layout = new QHBoxLayout;
	section_layout->addWidget(new QLabel(tr("<b>Size:</b>"), this));
	section_layout->addWidget(m_sizes, 1);

	m_lists = new QStackedWidget(this);
	connect(m_sizes, SIGNAL(activated(int)), m_lists, SLOT(setCurrentIndex(int)));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addLayout(section_layout);
	layout->addWidget(m_lists);
	layout->addSpacing(12);
	layout->addWidget(buttons);

	read();
}

// ============================================================================

void Scores::addScore(int steps, int seconds, int algorithm, int size)
{
	// Find high score board
	ScoreBoard* board;
	int pos = m_sizes->findText(QString::number(size));
	if (pos != -1) {
		// Access already loaded one
		board = dynamic_cast<ScoreBoard*>(m_lists->widget(pos));
		if (board == 0) {
			return;
		}
	} else {
		// Create new one
		board = new ScoreBoard(this);
		int count = m_sizes->count();
		for (pos = 0; pos < count; ++pos) {
			if (size < m_sizes->itemText(pos).toInt()) {
				break;
			}
		}
		m_sizes->insertItem(pos, QString::number(size));
		m_lists->insertWidget(pos, board);
	}

	// Check if it is a high score
	int count = board->topLevelItemCount();
	Q_ASSERT(count < 11);
	if (count == 10) {
		bool success = false;
		QTreeWidgetItem* item;
		int item_seconds;
		for (int i = 0; i < count; ++i) {
			item = board->topLevelItem(i);
			item_seconds = item->data(1, Qt::UserRole).toInt();
			if ( seconds < item_seconds ||
				(seconds == item_seconds && steps < item->text(2).toInt()) ) {
				success = true;
				break;
			}
		}
		if (!success) {
			return;
		}
	}

	// Switch to appropriate high score board
	m_sizes->setCurrentIndex(pos);
	m_lists->setCurrentWidget(board);

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
#elif defined(Q_OS_WIN32)
	{
		WCHAR buffer[UNLEN + 1];
		DWORD count = sizeof(buffer);
		if (GetUserName(buffer, &count)) {
			name = QString::fromStdWString(buffer);
		}
	}
#endif
	bool ok;
	name = QInputDialog::getText(parentWidget(), tr("Congratulations!"), tr("Your score has made the top ten.\nPlease enter your name:"), QLineEdit::Normal, name, &ok);
	if (!ok || name.isEmpty()) {
		return;
	}

	// Create score
	QTreeWidgetItem* score = new Score(seconds, steps, algorithm);
	score->setText(0, name);
	board->addTopLevelItem(score);
	board->clearSelection();
	score->setSelected(true);
	board->updateItems();

	// Save items
	QStringList values;
	QTreeWidgetItem* item;
	count = board->topLevelItemCount();
	for (int i = 0; i < count; ++i) {
		item = board->topLevelItem(i);
		values += QString("%1:%2:%3:%4") .arg(item->text(0)) .arg(item->data(1, Qt::UserRole).toInt()) .arg(item->text(2)) .arg(item->data(3, Qt::UserRole).toInt());
	}
	QSettings().setValue("Scores/" + QString::number(size), values);

	show();
}

// ============================================================================

void Scores::read()
{
	QStringList values;
	ScoreBoard* board;
	Score* score;

	QSettings settings;
	settings.beginGroup("Scores");
	foreach (QString key, settings.childKeys()) {
		if (key.toInt() == 0) {
			continue;
		}
		m_sizes->addItem(key);

		board = new ScoreBoard(this);
		m_lists->addWidget(board);

		QStringList data = settings.value(key).toStringList();
		foreach (QString s, data) {
			values = s.split(':');
			if (values.size() != 4) {
				continue;
			}
			score = new Score(values[1].toInt(), values[2].toInt(), values[3].toInt());
			score->setText(0, values[0]);
			board->addTopLevelItem(score);
		}
		board->updateItems();
	}
}

// ============================================================================
