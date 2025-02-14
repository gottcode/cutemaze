/*
	SPDX-FileCopyrightText: 2009-2025 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CUTEMAZE_SCORES_DIALOG_H
#define CUTEMAZE_SCORES_DIALOG_H

#include <QDialog>
class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QSettings;

/**
 * @brief The ScoresDialog class displays the list of high scores.
 */
class ScoresDialog : public QDialog
{
	Q_OBJECT

	/** The columns of scores. */
	enum Columns
	{
		RankColumn,
		NameColumn,
		ScoreColumn,
		TimeColumn,
		StepsColumn,
		AlgorithmColumn,
		SizeColumn,
		TotalColumns
	};

	/**
	 * @brief The ScoresDialog::Score struct descibres a high score.
	 */
	struct Score
	{
		QString name; /**< the player's name */
		int score; /**< the value of the score */
		int seconds; /**< how long it took to play the game */
		int steps; /**< how many steps the player took */
		int algorithm; /**< which algorithm was used to generate the maze */
		int size; /**< how big was the board */

		/**
		 * Constructs a score.
		 * @param n the player's name
		 * @param s the value of the score
		 * @param sc how long it took to play the game
		 * @param stp how many steps the player took
		 * @param alg which algorithm was used to generate the maze
		 * @param sz the the size of the board
		 */
		Score(const QString& n = QString(), int s = 0, int sc = 0, int stp = 0, int alg = 0, int sz = 0)
			: score(s)
			, seconds(sc)
			, steps(stp)
			, algorithm(alg)
			, size(sz)
		{
			setName(n);
		}

		/**
		 * Sets the player name for the score.
		 * @param text the player name
		 */
		void setName(const QString& text)
		{
			name = text.simplified();
			name.remove('\0');
		}
	};

public:
	/**
	 * Constructs a scores dialog.
	 * @param parent the QWidget that manages the dialog
	 */
	explicit ScoresDialog(QWidget* parent = nullptr);

	/**
	 * Attempts to add a score.
	 * @param seconds how long it took to play the game
	 * @param steps how many steps the player took
	 * @param algorithm which algorithm was used to generate the maze
	 * @param size how big was the board
	 * @return whether the score was added
	 */
	bool addScore(int seconds, int steps, int algorithm, int size);

	/**
	 * Converts the stored scores to the new format.
	 */
	static void migrate();

protected:
	/**
	 * Override hideEvent to add score if the player has not already pressed enter.
	 * @param event details of the hide event
	 */
	void hideEvent(QHideEvent* event) override;

	/**
	 * Override keyPressEvent to ignore return key. Keeps dialog from closing when the player
	 * presses return after entering their name.
	 * @param event details of the key press event
	 */
	void keyPressEvent(QKeyEvent* event) override;

private Q_SLOTS:
	/**
	 * Enters the score and saves list of scores once the player has finished entering their name.
	 */
	void editingFinished();

private:
	/**
	 * Adds a score to the high score board.
	 * @param name the player's name
	 * @param seconds how long it took to play the game
	 * @param steps how many steps the player took
	 * @param algorithm which algorithm was used to generate the maze
	 * @param size how big was the board
	 * @return @c true if the score was added
	 */
	bool addScore(const QString& name, int seconds, int steps, int algorithm, int size);

	/**
	 * Loads the scores from the settings.
	 * @param settings where to load the scores from
	 */
	void load(QSettings& settings);

	/**
	 * Sets the text of the high scores. Adds the dashed lines for empty scores.
	 */
	void updateItems();

private:
	QDialogButtonBox* m_buttons; /**< buttons to control dialog */
	QLineEdit* m_username; /**< widget for the player to enter their name */

	QList<Score> m_scores; /**< the high score data */
	QLabel* m_score_labels[10][TotalColumns]; /**< the grid[row][column] of labels to display the scores */
	QGridLayout* m_scores_layout; /**< the layout for the dialog */
	int m_row; /**< location of most recently added score */

	QString m_default_name; /**< the default name */
};

#endif // CUTEMAZE_SCORES_DIALOG_H
