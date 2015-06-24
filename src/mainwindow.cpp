// ------------------------------------------------------------------------------
//   Copyright (C) 2007 by Jukka Sirkka
//   jukka.sirkka@iki.fi
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the
//   Free Software Foundation, Inc.,
//   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ------------------------------------------------------------------------------

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QButtonGroup>

#define L_MASTER 0
#define L_PCM 1
#define L_ANALOG_IN 2
#define L_DIGITAL_IN 3

#define ENUM_RATE320 "32000"
#define ENUM_RATE441 "44100"
#define ENUM_RATE480 "48000"
#define ENUM_RATE882 "88200"
#define ENUM_RATE960 "96000"
#define ENUM_IEC958 "IEC958 Input"
#define ENUM_DOFF "Off"
#define ENUM_D320 "32kHz"
#define ENUM_D441 "44.1kHz"
#define ENUM_D480 "48kHz"


MainWindow::MainWindow():
    KMainWindow(),
    mUI(new Ui::MainWindow)
{

    mUI->setupUi(this);

    mUI->profiles->addAction(mUI->actionNewProfile);
    mUI->profiles->addAction(mUI->actionSaveProfile);
    mUI->profiles->addAction(mUI->actionRenameProfile);
    mUI->profiles->addAction(mUI->actionDeleteProfile);

    mUI->mixerPCM1->setTitle("PCM Playback");
    mUI->mixerPCM1->setup(0);
    mUI->mixerAnalogIn->setTitle("Analog In");
    mUI->mixerAnalogIn->setup(0);
    mUI->mixerDigitalIn->setTitle("Digital In");
    mUI->mixerDigitalIn->setup(0);

    mUI->analogOut->setTitle("Analog Out");
    mUI->analogOut->setup(0);
    mUI->digitalOut->setTitle("Digital Out");
    mUI->digitalOut->setup(0);

    mLevelIndices[L_MASTER] = 20;
    mLevelIndices[L_PCM] = 0;
    mLevelIndices[L_ANALOG_IN] = 10;
    mLevelIndices[L_DIGITAL_IN] = 12;

    setupHWTab();


    readSettings();
}

void MainWindow::setupHWTab() {

    mHWInternalG = new QButtonGroup;

    mHWInternal[ENUM_RATE320] = 0;
    mHWInternalG->addButton(mUI->xtal320, 0);

    mHWInternal[ENUM_RATE441] = 1;
    mHWInternalG->addButton(mUI->xtal441, 1);

    mHWInternal[ENUM_RATE480] = 2;
    mHWInternalG->addButton(mUI->xtal480, 2);

    mHWInternal[ENUM_RATE882] = 3;
    mHWInternalG->addButton(mUI->xtal882, 3);

    mHWInternal[ENUM_RATE960] = 4;
    mHWInternalG->addButton(mUI->xtal960, 4);

    connect(mHWInternalG, SIGNAL(buttonClicked(int)), this, SLOT(leftPressed(int)));

}

void MainWindow::closeEvent(QCloseEvent*) {}


void MainWindow::on_actionNewProfile_triggered() {}
void MainWindow::on_actionSaveProfile_triggered() {}
void MainWindow::on_actionDeleteProfile_triggered() {}
void MainWindow::on_actionRenameProfile_triggered() {}

void MainWindow::on_actionEnableLeds_toggled(bool) {}

void MainWindow::on_actionQuit_triggered() {}
void MainWindow::on_actionClose_triggered() {}


void MainWindow::readSettings() {}

void MainWindow::writeSettings() {}

void MainWindow::dbus_PCMVolumeUp() {}
void MainWindow::dbus_PCMVolumeDown() {}
void MainWindow::dbus_PCMVolumeMute() {}


MainWindow::~MainWindow() {
    delete mUI;
}

#include "mainwindow.moc"
