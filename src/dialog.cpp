/**     @file dialog.cpp
 *
 *     EEEE2076 - Software Engineering & VR Project
 *
 */

#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>

Dialog::Dialog(QWidget *parent)
	: QDialog(parent), ui(new Ui::Dialog)
{
	ui->setupUi(this);

	/* Set up the LCDs */
	ui->RedLCD->setSegmentStyle(QLCDNumber::Flat);
	ui->RedLCD->setStyleSheet("QLCDNumber { color: red; background-color: black; }");
	ui->GreenLCD->setSegmentStyle(QLCDNumber::Flat);
	ui->GreenLCD->setStyleSheet("QLCDNumber { color: green; background-color: black; }");
	ui->BlueLCD->setSegmentStyle(QLCDNumber::Flat);
	ui->BlueLCD->setStyleSheet("QLCDNumber { color: blue; background-color: black; }");

	/* Set up the sliders */
	ui->RedSlider->setStyleSheet("QSlider::groove:horizontal{background: red;position: absolute;up: 4px; down: 4px;}QSlider::handle:horizontal{width: 10px;background: black;margin: -4px 0;}QSlider::add-page:horizontal{background: white;}QSlider::sub-page:horizontal{background: #ff0000;}");
	ui->GreenSlider->setStyleSheet("QSlider::groove:horizontal{background: red;position: absolute;up: 4px; down: 4px;}QSlider::handle:horizontal{width: 10px;background: black;margin: -4px 0;}QSlider::add-page:horizontal{background: white;}QSlider::sub-page:horizontal{background: #00ff00;}");
	ui->BlueSlider->setStyleSheet("QSlider::groove:horizontal{background: red;position: absolute;up: 4px; down: 4px;}QSlider::handle:horizontal{width: 10px;background: black;margin: -4px 0;}QSlider::add-page:horizontal{background: white;}QSlider::sub-page:horizontal{background: #0000ff;}");

	/* Set up the connections */
	connect(ui->RedSlider, &QSlider::valueChanged, this, &Dialog::handleRedSlider);
	connect(ui->GreenSlider, &QSlider::valueChanged, this, &Dialog::handleGreenSlider);
	connect(ui->BlueSlider, &QSlider::valueChanged, this, &Dialog::handleBlueSlider);

	connect(ui->ColourValue, &QLineEdit::textChanged, this, &Dialog::handleColourEntryBox);

	updateColour();
}

Dialog::~Dialog()
{
	delete ui;
}

void Dialog::updateColour()
{
	/* Update the LCDs and sliders */
	ui->RedSlider->setValue(colour.red());
	ui->RedLCD->display(colour.red());
	ui->GreenLCD->display(colour.green());
	ui->GreenSlider->setValue(colour.green());
	ui->BlueLCD->display(colour.blue());
	ui->BlueSlider->setValue(colour.blue());

	ui->ColourValue->setText(colour.name());
	ui->ColourDisplay->setStyleSheet(QString("QWidget{ background-color: %1; }").arg(colour.name()));
}

 //3 functions to display slider values on lcd screens 
void Dialog::handleRedSlider()
{
	colour.setRed(ui->RedSlider->value());
	updateColour();
}

void Dialog::handleGreenSlider()
{
	colour.setGreen(ui->GreenSlider->value());
	updateColour();
}

void Dialog::handleBlueSlider()
{
	colour.setBlue(ui->BlueSlider->value());
	updateColour();
}

/* Handles the hex code entry box */
void Dialog::handleColourEntryBox()
{
	QColor newColor(ui->ColourValue->text());
	if (newColor.isValid())
	{
		colour = newColor;
		updateColour();
	}
}

 //This runs after accepting the dialog box 
void Dialog::accept()
{
	/* Get the colour, name and bool values from the ui elements */
	QColor colour(ui->RedSlider->value(), ui->GreenSlider->value(), ui->BlueSlider->value());
	QString name(ui->lineEdit->displayText());
	bool visible(ui->checkBox->isChecked());

	/* send the data back to the main window */
	emit(sendingData(name, visible, colour));

	/* Close the dialog */
	QDialog::accept();
}

 //Set the initial values of the dialog 
void Dialog::setInitialValues(const QString &_name, const bool &_visible, const QColor &_colour)
{
	/* Set local variables */
	name = _name;
	visible = _visible;
	colour = _colour;

	/* Set the initial values */
	ui->checkBox->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
	ui->lineEdit->setText(name);
	updateColour();
}