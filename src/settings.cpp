/***********************************************************************
 *
 * Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015 Graeme Gott <graeme@gottcode.org>
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
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {
// ============================================================================

QString homeDataPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
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
#ifndef Q_OS_WIN
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
		for (ControlButton* button : controls) {
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
	m_theme->setDevicePixelRatio(devicePixelRatio());

	QTabWidget* tabs = new QTabWidget(this);
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &Settings::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &Settings::reject);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(10);
	layout->setSpacing(18);
	layout->addWidget(tabs);
	layout->addWidget(buttons);


	// Create Gameplay tab
	QWidget* gameplay_tab = new QWidget;
	tabs->addTab(gameplay_tab, tr("Game"));

	m_gameplay_path = new QCheckBox(tr("Show where you've been"), gameplay_tab);
	m_gameplay_steps = new QCheckBox(tr("Show number of steps taken"), gameplay_tab);
	m_gameplay_time = new QCheckBox(tr("Show elapsed time"), gameplay_tab);
	m_gameplay_smooth = new QCheckBox(tr("Smooth movement"), gameplay_tab);

	QFormLayout* gameplay_layout = new QFormLayout(gameplay_tab);
	gameplay_layout->addRow(m_gameplay_path);
	gameplay_layout->addRow(m_gameplay_steps);
	gameplay_layout->addRow(m_gameplay_time);
	gameplay_layout->addRow(m_gameplay_smooth);


	// Create Controls tab
	QWidget* controls_tab = new QWidget;
	tabs->addTab(controls_tab, tr("Controls"));

	controls.append(new ControlButton("Up", Qt::Key_Up, this));
	controls.append(new ControlButton("Down", Qt::Key_Down, this));
	controls.append(new ControlButton("Left", Qt::Key_Left, this));
	controls.append(new ControlButton("Right", Qt::Key_Right, this));
	controls.append(new ControlButton("Flag", Qt::Key_Space, this));
	controls.append(new ControlButton("Hint", Qt::Key_H, this));

	QFormLayout * controls_layout = new QFormLayout(controls_tab);
	controls_layout->addRow(tr("Move Up:"), controls[0]);
	controls_layout->addRow(tr("Move Down:"), controls[1]);
	controls_layout->addRow(tr("Move Left:"), controls[2]);
	controls_layout->addRow(tr("Move Right:"), controls[3]);
	controls_layout->addRow(tr("Toggle Flag:"), controls[4]);
	controls_layout->addRow(tr("Show Hint:"), controls[5]);


	// Create Themes tab
	QWidget* themes_tab = new QWidget;
	tabs->addTab(themes_tab, tr("Themes"));

	m_themes_preview = new QLabel(themes_tab);

	m_themes_selector = new QComboBox(themes_tab);
	connect(m_themes_selector, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &Settings::themeSelected);

	QHBoxLayout* themes_selector_layout = new QHBoxLayout;
	themes_selector_layout->setMargin(0);
	themes_selector_layout->addWidget(m_themes_selector, 1);

	QPushButton* add_button = new QPushButton(tr("Add"), themes_tab);
	connect(add_button, &QPushButton::clicked, this, &Settings::addTheme);
	m_themes_remove_button = new QPushButton(tr("Remove"), themes_tab);
	connect(m_themes_remove_button, &QPushButton::clicked, this, &Settings::removeTheme);

	themes_selector_layout->addWidget(add_button, 0);
	themes_selector_layout->addWidget(m_themes_remove_button, 0);

	QVBoxLayout* themes_layout = new QVBoxLayout(themes_tab);
	themes_layout->addWidget(m_themes_preview, 1, Qt::AlignCenter);
	themes_layout->addLayout(themes_selector_layout);


	// Load current settings
	loadSettings();
}

// ============================================================================

Settings::~Settings()
{
	controls.clear();
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

	// Write control button settings to disk
	for (ControlButton* button : controls) {
		settings.setValue("Controls/" + button->objectName().mid(8), button->key);
	}

	// Write theme to disk
	settings.setValue("Theme", m_themes_selector->currentText());

	emit settingsChanged();

	QDialog::accept();
}

// ============================================================================

void Settings::themeSelected(const QString& theme)
{
	if (!theme.isEmpty()) {
		m_theme->load(theme);
		generatePreview();
		m_themes_remove_button->setEnabled(QFileInfo(homeDataPath() + '/' + theme + ".svg").exists());
	}
}

// ============================================================================

void Settings::addTheme()
{
	// Select theme file
	QString path = QFileDialog::getOpenFileName(this, tr("Select Theme File"), QDir::homePath(), QString("*.svg"));
	if (path.isEmpty()) {
		return;
	}

	// Create data folder if necessary
	QString dirpath = homeDataPath();
	if (!QFileInfo(dirpath).exists()) {
		QDir dir = QDir::home();
		if (!dir.mkpath(dirpath)) {
			QMessageBox::warning(this, tr("Sorry"), tr("Unable to create data folder."));
			return;
		}
	}

	// Copy theme file
	QFileInfo file(path);
	if (!QFile::copy(path, dirpath + '/' + file.fileName())) {
		QMessageBox::warning(this, tr("Sorry"), tr("Unable to copy theme file."));
		return;
	}

	// Add theme to list
	QStringList themes = m_theme->available();
	int theme = themes.indexOf(m_themes_selector->currentText());
	if (theme == -1) {
		theme = themes.indexOf("Mouse");
	}
	m_themes_selector->clear();
	m_themes_selector->addItems(themes);
	m_themes_selector->setCurrentIndex(theme);
}

// ============================================================================

void Settings::removeTheme()
{
	// Find theme
	if (m_themes_selector->currentIndex() == -1) {
		return;
	}
	QString dirpath = homeDataPath();
	QString file = m_themes_selector->currentText() + ".svg";
	if (!QFileInfo(dirpath + '/' + file).exists()) {
		return;
	}

	if (QMessageBox::question(this, tr("Question"), tr("Remove the selected theme?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		// Delete theme file
		QDir dir(dirpath);
		if (dir.remove(file) == false) {
			QMessageBox::warning(this, tr("Sorry"), tr("Unable to remove the selected theme."));
			return;
		}

		// Delete theme from list
		if (!m_theme->available().contains(m_themes_selector->currentText())) {
			m_themes_selector->removeItem(m_themes_selector->currentIndex());
		}

		// Force change to next theme in list
		themeSelected(m_themes_selector->currentText());
		QSettings().setValue("Theme", m_themes_selector->currentText());
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

	// Read control button settings from disk
	for (ControlButton* button : controls) {
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
	m_themes_selector->setCurrentIndex(theme);
}

// ============================================================================

void Settings::generatePreview()
{
	int ratio = devicePixelRatio();
	QPixmap pixmap(QSize(192, 192) * ratio);
	pixmap.setDevicePixelRatio(ratio);
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
