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

#include "settings.h"

#include "theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#if !defined(QTOPIA_PHONE)
#include <QFileDialog>
#endif
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {
// ============================================================================

QString homeDataPath()
{
#if defined(Q_OS_MAC)
	QString path = QDir::homePath() + "/Library/Application Support/GottCode/CuteMaze";
#elif defined(Q_OS_UNIX)
	QString path = getenv("XDG_DATA_HOME");
	if (path.isEmpty()) {
		path = QDir::homePath() + "/.local/share";
	}
	path += "/games/cutemaze";
#elif defined(Q_OS_WIN32)
	QString path = QDir::homePath() + "/Application Data/GottCode/CuteMaze";
#endif
	return path;
}

// ============================================================================

QString supportedArchiveFormats()
{
	static QString filters;

	if (filters.isEmpty()) {
		// Create list of commands
		QMap<QString, bool> commands;
		commands.insert("tar", false);
		commands.insert("gunzip", false);
		commands.insert("bunzip2", false);
		commands.insert("uncompress", false);
		commands.insert("unzip", false);

		// Search PATH for commands
		QString path = getenv("PATH");
#if defined(Q_OS_UNIX)
		QStringList dirs = path.split(":");
#elif defined(Q_OS_WIN32)
		QStringList dirs = path.split(";");
#endif
		QDir dir;
		foreach (QString string, dirs) {
			dir.setPath(string);
			QMutableMapIterator<QString, bool> i(commands);
			while (i.hasNext()) {
				i.next();
				if (i.value() == false) {
#if defined(Q_OS_UNIX)
					if (dir.exists(i.key()))
						i.setValue(true);
#elif defined(Q_OS_WIN32)
					if (dir.exists(i.key() + ".exe"))
						i.setValue(true);
#endif
				}
			}
		}

		// Detect archive formats
		QStringList formats;
		if (commands["tar"]) {
			if (commands["gunzip"]) {
				formats += "*.tgz";
				formats += "*.tar.gz";
			}
			if (commands["bunzip2"]) {
				formats += "*.tar.bz2";
			}
			if (commands["uncompress"]) {
				formats += "*.tar.Z";
			}
			formats += "*.tar";
		}
		if (commands["unzip"]) {
			formats += "*.zip";
		}
		filters = formats.join(" ");
	}

	return filters;
}

// ============================================================================

// ControlButton class
// ============================================================================

class ControlButton : public QPushButton
{
public:
	ControlButton(const QString& type, Qt::Key default_key_value, QWidget* parent = 0);

	unsigned int key;
	unsigned int default_key;

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
};

QList<ControlButton*> controls;
ControlButton* active_control = 0;

// ============================================================================

ControlButton::ControlButton(const QString& type, Qt::Key default_key_value, QWidget* parent)
:	QPushButton(parent),
	key(default_key_value),
	default_key(default_key_value)
{
	setObjectName("control_" + type);
	setText(QKeySequence(key).toString());
	setCheckable(true);
	setAutoDefault(false);
	setFocusPolicy(Qt::StrongFocus);
	installEventFilter(this);
}

// ============================================================================

void ControlButton::hideEvent(QHideEvent* event)
{
	if (this == active_control) {
		active_control = 0;
		setChecked(false);
	}
	QPushButton::hideEvent(event);
}

// ============================================================================

void ControlButton::keyPressEvent(QKeyEvent* event)
{
	if ( (this == active_control) && (event->key() != Qt::Key_Escape) && (event->count() == 1) ) {
		key = event->key();
		QString control;
		switch (key) {
		case Qt::Key_Shift:
			control = tr("Shift");
			break;
#ifndef Q_OS_MAC
		case Qt::Key_Control:
			control = tr("Ctrl");
			break;
		case Qt::Key_Meta:
			control = tr("Meta");
			break;
		case Qt::Key_Alt:
			control = tr("Alt");
			break;
		case Qt::Key_Super_L:
		case Qt::Key_Super_R:
#ifndef Q_OS_WIN32
			control = tr("Super");
#else
			control = tr("Windows");
#endif
			break;
#else
		case Qt::Key_Control:
			control = tr("Command");
			break;
		case Qt::Key_Meta:
			control = tr("Control");
			break;
		case Qt::Key_Alt:
			control = tr("Option");
			break;
#endif
		default:
			control = QKeySequence(key).toString(QKeySequence::NativeText);
			break;
		};
		foreach (ControlButton* button, controls) {
			if (button->text() == control) {
				button->setText("");
				button->key = 0;
			}
		}
		setText(control);
		setChecked(false);
		active_control = 0;
	} else {
		QPushButton::keyPressEvent(event);
	}
}

// ============================================================================

void ControlButton::mousePressEvent(QMouseEvent* event)
{
	if (this != active_control) {
		if (active_control) {
			active_control->setChecked(false);
		}
		active_control = this;
	} else {
		active_control = 0;
	}
	QPushButton::mousePressEvent(event);
}

// ============================================================================
}

// Settings class
// ============================================================================

Settings::Settings(QWidget* parent)
:	QDialog(parent)
{
	setWindowTitle(tr("Settings"));
	m_theme = new Theme;

	QTabWidget* tabs = new QTabWidget(this);
#if !defined(QTOPIA_PHONE)
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
#endif

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(10);
	layout->setSpacing(18);
	layout->addWidget(tabs);
#if !defined(QTOPIA_PHONE)
	layout->addWidget(buttons);
#endif


	// Create Gameplay tab
	QWidget* gameplay_tab = new QWidget;
	tabs->addTab(gameplay_tab, tr("Game"));

	m_gameplay_path = new QCheckBox(tr("Show where you've been"), gameplay_tab);
	m_gameplay_steps = new QCheckBox(tr("Show number of steps taken"), gameplay_tab);
	m_gameplay_time = new QCheckBox(tr("Show elapsed time"), gameplay_tab);
	m_gameplay_smooth = new QCheckBox(tr("Smooth movement"), gameplay_tab);
#if defined(QTOPIA_PHONE)
	m_gameplay_steps->hide();
	m_gameplay_time->hide();
#endif

	QGridLayout* gameplay_layout = new QGridLayout(gameplay_tab);
	gameplay_layout->setSpacing(6);
	gameplay_layout->setRowStretch(0, 1);
	gameplay_layout->setRowStretch(5, 1);
	gameplay_layout->setColumnStretch(0, 1);
	gameplay_layout->setColumnStretch(2, 1);
	gameplay_layout->addWidget(m_gameplay_path, 1, 1);
	gameplay_layout->addWidget(m_gameplay_steps, 2, 1);
	gameplay_layout->addWidget(m_gameplay_time, 3, 1);
	gameplay_layout->addWidget(m_gameplay_smooth, 4, 1);


	// Create Mazes tab
	QWidget* mazes_tab = new QWidget;
	tabs->addTab(mazes_tab, tr("Mazes"));

	m_mazes_algorithm = new QComboBox(mazes_tab);
	m_mazes_algorithm->setInsertPolicy(QComboBox::InsertAlphabetically);
	m_mazes_algorithm->addItem(tr("Hunt and Kill"), 0);
	m_mazes_algorithm->addItem(tr("Kruskal"), 1);
	m_mazes_algorithm->addItem(tr("Prim"), 2);
	m_mazes_algorithm->addItem(tr("Recursive Backtracker"), 3);
	m_mazes_algorithm->addItem(tr("Stack"), 4);
	m_mazes_algorithm->addItem(tr("Stack 2"), 5);
	m_mazes_algorithm->addItem(tr("Stack 3"), 6);
	m_mazes_algorithm->addItem(tr("Stack 4"), 7);
	m_mazes_algorithm->addItem(tr("Stack 5"), 8);
	connect(m_mazes_algorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(algorithmSelected(int)));

	m_mazes_preview = new QLabel(mazes_tab);

	m_mazes_targets = new QSpinBox(mazes_tab);
	m_mazes_targets->setRange(1, 99);

	m_mazes_size = new QSpinBox(mazes_tab);
	m_mazes_size->setRange(10, 99);

	QGridLayout* mazes_layout = new QGridLayout(mazes_tab);
	mazes_layout->setSpacing(6);
	mazes_layout->setRowStretch(0, 1);
	mazes_layout->setRowStretch(5, 1);
	mazes_layout->setColumnStretch(0, 1);
	mazes_layout->setColumnStretch(3, 1);
	mazes_layout->addWidget(m_mazes_preview, 1, 2);
	mazes_layout->addWidget(new QLabel(tr("Algorithm"), mazes_tab), 2, 1, Qt::AlignRight | Qt::AlignVCenter);
	mazes_layout->addWidget(m_mazes_algorithm, 2, 2);
	mazes_layout->addWidget(new QLabel(tr("Targets"), mazes_tab), 3, 1, Qt::AlignRight | Qt::AlignVCenter);
	mazes_layout->addWidget(m_mazes_targets, 3, 2);
	mazes_layout->addWidget(new QLabel(tr("Size"), mazes_tab), 4, 1, Qt::AlignRight | Qt::AlignVCenter);
	mazes_layout->addWidget(m_mazes_size, 4, 2);


	// Create Controls tab
	QWidget* controls_tab = new QWidget;
	tabs->addTab(controls_tab, tr("Controls"));

	controls.append(new ControlButton("Up", Qt::Key_Up, this));
	controls.append(new ControlButton("Down", Qt::Key_Down, this));
	controls.append(new ControlButton("Left", Qt::Key_Left, this));
	controls.append(new ControlButton("Right", Qt::Key_Right, this));
	controls.append(new ControlButton("Flag", Qt::Key_Space, this));

	QGridLayout * controls_layout = new QGridLayout(controls_tab);
	controls_layout->setSpacing(6);
	controls_layout->setRowStretch(0, 1);
	controls_layout->setRowStretch(6, 1);
	controls_layout->setColumnStretch(0, 1);
	controls_layout->setColumnStretch(3, 1);
	controls_layout->addWidget(new QLabel(tr("Move Up"), controls_tab), 1, 1, Qt::AlignRight | Qt::AlignVCenter);
	controls_layout->addWidget(controls[0], 1, 2);
	controls_layout->addWidget(new QLabel(tr("Move Down"), controls_tab), 2, 1, Qt::AlignRight | Qt::AlignVCenter);
	controls_layout->addWidget(controls[1], 2, 2);
	controls_layout->addWidget(new QLabel(tr("Move Left"), controls_tab), 3, 1, Qt::AlignRight | Qt::AlignVCenter);
	controls_layout->addWidget(controls[2], 3, 2);
	controls_layout->addWidget(new QLabel(tr("Move Right"), controls_tab), 4, 1, Qt::AlignRight | Qt::AlignVCenter);
	controls_layout->addWidget(controls[3], 4, 2);
	controls_layout->addWidget(new QLabel(tr("Toggle Flag"), controls_tab), 5, 1, Qt::AlignRight | Qt::AlignVCenter);
	controls_layout->addWidget(controls[4], 5, 2);


	// Create Themes tab
	QWidget* themes_tab = new QWidget;
	tabs->addTab(themes_tab, tr("Themes"));

	m_themes_selector = new QListWidget(themes_tab);
	connect(m_themes_selector, SIGNAL(currentTextChanged(const QString&)), this, SLOT(themeSelected(const QString&)));

	m_themes_preview = new QLabel(themes_tab);

#if !defined(QTOPIA_PHONE)
	QPushButton* add_button = new QPushButton(tr("Add Theme"), themes_tab);
	add_button->setEnabled(!supportedArchiveFormats().isEmpty());
	connect(add_button, SIGNAL(clicked()), this, SLOT(addTheme()));
	m_themes_remove_button = new QPushButton(tr("Remove Theme"), themes_tab);
	connect(m_themes_remove_button, SIGNAL(clicked()), this, SLOT(removeTheme()));

	QHBoxLayout* themes_preview_layout = new QHBoxLayout;
	themes_preview_layout->setMargin(0);
	themes_preview_layout->setSpacing(6);
	themes_preview_layout->addWidget(m_themes_selector);
	themes_preview_layout->addWidget(m_themes_preview);

	QHBoxLayout* themes_button_layout = new QHBoxLayout;
	themes_button_layout->setMargin(0);
	themes_button_layout->addWidget(add_button);
	themes_button_layout->addWidget(m_themes_remove_button);

	QVBoxLayout* themes_layout = new QVBoxLayout(themes_tab);
	themes_layout->addLayout(themes_preview_layout);
	themes_layout->addLayout(themes_button_layout);
#else
	QHBoxLayout* themes_layout = new QHBoxLayout(themes_tab);
	themes_layout->addWidget(m_themes_selector);
	themes_layout->addWidget(m_themes_preview);
#endif


	// Set dialog's size
#if !defined(QTOPIA_PHONE)
	adjustSize();
	setMinimumSize(size());
#endif

	// Load current settings
	loadSettings();
}

// ============================================================================

Settings::~Settings()
{
	delete m_theme;
}

// ============================================================================

void Settings::accept()
{
	QSettings settings;

	// Write gameplay settings to disk
	settings.setValue("Show Path", m_gameplay_path->isChecked());
	settings.setValue("Show Steps", m_gameplay_steps->isChecked());
	settings.setValue("Show Time", m_gameplay_time->isChecked());
	settings.setValue("Smooth Movement", m_gameplay_smooth->isChecked());

	// Write new maze settings to disk
	settings.setValue("New/Algorithm", m_mazes_algorithm->itemData(m_mazes_algorithm->currentIndex()));
	settings.setValue("New/Targets", m_mazes_targets->value());
	settings.setValue("New/Size", m_mazes_size->value());

	// Write control button settings to disk
	foreach (ControlButton* button, controls) {
		settings.setValue("Controls/" + button->objectName().mid(8), button->key);
	}

	// Write theme to disk
	settings.setValue("Theme", m_themes_selector->currentItem()->text());

	emit settingsChanged();

	QDialog::accept();
}

// ============================================================================

void Settings::reject()
{
	loadSettings();
	QDialog::reject();
}

// ============================================================================

void Settings::algorithmSelected(int index)
{
	if (index != -1) {
		m_mazes_preview->setPixmap( QString(":/preview%1.png").arg( m_mazes_algorithm->itemData(index).toInt()) );
	}
}

// ============================================================================

void Settings::themeSelected(const QString& theme)
{
	if (!theme.isEmpty()) {
		m_theme->load(theme);
		generatePreview();
#if !defined(QTOPIA_PHONE)
		m_themes_remove_button->setEnabled(QFileInfo(homeDataPath() + "/" + theme).exists());
#endif
	}
}

// ============================================================================

void Settings::addTheme()
{
	// Select theme archive
#if !defined(QTOPIA_PHONE)
	QString filters = tr("Archives (%1)").arg(supportedArchiveFormats());
	QString file = QFileDialog::getOpenFileName(this, tr("Select Theme Archive"), QDir::homePath(), filters);
#else
	QString file;
#endif
	if (file.isEmpty()) {
		return;
	}

	// Create data folder if necessary
	QString path = homeDataPath();
	if (!QFileInfo(path).exists()) {
		QDir dir = QDir::home();
		if (!dir.mkpath(path)) {
			QMessageBox::warning(this, tr("Error"), tr("Unable to install theme.\n\nUnable to create data folder."), QMessageBox::Ok);
			return;
		}
	}

	// Extract files
	QProcess extract;
	extract.setWorkingDirectory(path);
	QString cmd;
	if (file.endsWith(".tar.gz", Qt::CaseInsensitive) || file.endsWith(".tgz", Qt::CaseInsensitive)) {
		cmd = "tar -xzf %1";
	} else if (file.endsWith(".tar.bz2", Qt::CaseInsensitive)) {
		cmd = "tar -xjf %1";
	} else if (file.endsWith(".tar.Z", Qt::CaseInsensitive)) {
		cmd = "tar -xZf %1";
	} else if (file.endsWith(".tar", Qt::CaseInsensitive)) {
		cmd = "tar -xf %1";
	} else if (file.endsWith(".zip", Qt::CaseInsensitive)) {
		cmd = "unzip %1";
	}
	extract.start(QString(cmd).arg(file));
	if (!extract.waitForFinished(-1)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to install theme.\n\nError while extracting archive."), QMessageBox::Ok);
		return;
	}

	// Add theme to list
	QStringList themes = m_theme->available();
	int theme = themes.indexOf(m_themes_selector->currentItem()->text());
	if (theme == -1) {
		theme = themes.indexOf("Mouse");
	}
	m_themes_selector->clear();
	m_themes_selector->addItems(themes);
	m_themes_selector->setCurrentRow(theme);
}

// ============================================================================

void Settings::removeTheme()
{
	// Find theme
	if (!m_themes_selector->currentItem()) {
		return;
	}
	QString path = homeDataPath() + "/" + m_themes_selector->currentItem()->text();
	if (!QFileInfo(path).exists()) {
		return;
	}

	if (QMessageBox::warning(this, tr("Remove Theme"), tr("Are you sure you want to remove the selected theme?\n\nThis will delete the files installed by this theme."), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		// Delete theme files
		QDir dir(path);
		QStringList files = dir.entryList(QDir::Files);
		foreach (QString file, files) {
			if (dir.remove(file) == false) {
				QMessageBox::warning(this, tr("Error"), tr("Unable to remove the theme.\n\nUnable to delete %1").arg(file), QMessageBox::Ok);
				return;
			}
		}

		// Delete theme folder
		if (QDir::home().rmdir(path) == false) {
			QMessageBox::warning(this, tr("Error"), tr("Unable to remove the theme.\n\nUnable to delete the theme folder."), QMessageBox::Ok);
			return;
		}

		// Delete theme from list
		delete m_themes_selector->currentItem();

		// Force change to next theme in lest
		QSettings().setValue("Theme", m_themes_selector->currentItem()->text());
		emit settingsChanged();
	}
}

// ============================================================================

void Settings::loadSettings()
{
	QSettings settings;

	// Read gameplay settings from disk
	m_gameplay_path->setChecked(settings.value("Show Path", true).toBool());
	m_gameplay_steps->setChecked(settings.value("Show Steps", true).toBool());
	m_gameplay_time->setChecked(settings.value("Show Time", true).toBool());
	m_gameplay_smooth->setChecked(settings.value("Smooth Movement", true).toBool());

	// Read new maze settings from disk
	int algorithm = settings.value("New/Algorithm", 4).toInt();
	m_mazes_algorithm->setCurrentIndex(m_mazes_algorithm->findData(algorithm));
	m_mazes_targets->setValue(settings.value("New/Targets", 3).toInt());
	m_mazes_size->setValue(settings.value("New/Size", 50).toInt());

	// Read control button settings from disk
	foreach (ControlButton* button, controls) {
		button->key = settings.value("Controls/" + button->objectName().mid(8), button->default_key).toUInt();
		button->setText(QKeySequence(button->key).toString());
	}

	// Read theme from disk
	QStringList themes = m_theme->available();
	int theme = themes.indexOf(settings.value("Theme", "Mouse").toString());
	if (theme == -1) {
		theme = themes.indexOf("Mouse");
	}
	m_themes_selector->clear();
	m_themes_selector->addItems(themes);
	m_themes_selector->setCurrentRow(theme);
}

// ============================================================================

void Settings::generatePreview()
{
	QPixmap pixmap(192, 192);
	pixmap.fill(Qt::white);
	{
		QPainter painter(&pixmap);
		painter.translate(-80, -80);
		for (int c = 0; c < 4; ++c) {
			for (int r = 0; r < 4; ++r) {
				m_theme->draw(painter, c, r, Theme::Background);
			}
		}

		m_theme->drawWall(painter, 1, 1);
		m_theme->drawWall(painter, 2, 1);
		m_theme->drawWall(painter, 2, 2);
		m_theme->drawWall(painter, 1, 3);
		m_theme->drawWall(painter, 2, 1, true);
		m_theme->drawWall(painter, 1, 2, true);
		m_theme->drawWall(painter, 3, 2, true);

		m_theme->drawCorner(painter, 1, 1, 10);
		m_theme->drawCorner(painter, 2, 1, 14);
		m_theme->drawCorner(painter, 3, 1, 10);
		m_theme->drawCorner(painter, 1, 2, 4);
		m_theme->drawCorner(painter, 2, 2, 3);
		m_theme->drawCorner(painter, 3, 2, 12);
		m_theme->drawCorner(painter, 1, 3, 3);
		m_theme->drawCorner(painter, 2, 3, 8);
		m_theme->drawCorner(painter, 3, 3, 5);

		m_theme->draw(painter, 1, 1, Theme::Start);
		m_theme->draw(painter, 1, 1, Theme::Marker, 180);
		m_theme->draw(painter, 1, 2, Theme::Marker, 90);
		m_theme->draw(painter, 1, 2, Theme::Flag);
		m_theme->draw(painter, 2, 1, Theme::Target);
		m_theme->draw(painter, 2, 2, Theme::Player, 360);
	}

	m_themes_preview->setPixmap(pixmap);
}

// ============================================================================
