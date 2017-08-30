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
#include "envycard.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QDebug>
#include <QCloseEvent>

#include "settings.h"
#include "trayitem.h"

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

#define HW_ENUM_SWITCH_SOURCE "Switch digital/internal clock source"
#define ENUM_INTERNAL "Internal clock"


MainWindow::MainWindow(bool docked):
    QMainWindow(),
    inSlotFlag(false),
    inEventFlag(false),
    mUI(new Ui::MainWindow),
    m_PeakSampleFreq(50),
    m_MaxPeakSize(10*m_PeakSampleFreq), // 10 seconds
    m_CurrentSlot(0),
    m_MaxPeakSlot(0),
    m_MaxPeakValue(0),
    m_Color(0),
    m_MaxPeaks(m_MaxPeakSize),
    mLastPos(),
    mTrayItem(new TrayItem(this))
{

    mCard = &EnvyCard::Instance();


    mUI->setupUi(this);


    mUI->mixerPCM4->setup(mCard->PCMAddress(4), "mixer-iec958-in", "IEC958 Playback", mRouting);
    mUI->mixerPCM1->setup(mCard->PCMAddress(0), "mixer-pcm-in", "PCM Playback", mRouting);
    mUI->mixerAnalogIn->setup(mCard->AnalogInAddress(), "mixer-analog-in", "Analog In", mRouting);
    mUI->mixerDigitalIn->setup(mCard->DigitalInAddress(), "mixer-digital-in", "Digital In", mRouting);

    mRouting[mCard->DACAddress()] = mUI->masterVolume;

    mCard->configAddresses(mRouting.keys());

    mUI->analogOut->setup(mCard->AnalogOutAddress(), "router-analog-out", "Analog Out", mRouting);
    mUI->digitalOut->setup(mCard->DigitalOutAddress(), "router-digital-out", "Digital Out", mRouting);

    setupHWTab();

    readGUIState();


    connectToCard();
    connectFromCard();

    readMixerState();

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(updateMeters()));
    mTimer->start(1000/m_PeakSampleFreq);

    if (docked || mStartDocked) {
        minimizeWindow();
    } else {
        restoreWindow();
    }
}

void MainWindow::setupHWTab() {

    // Internal clock rate
    mHWStrings[HW_ENUM_INTERNAL_CLOCK] = (QStringList() <<
        ENUM_RATE320 << ENUM_RATE441 << ENUM_RATE480 << ENUM_RATE882 << ENUM_RATE960);

    QButtonGroup* butg = new QButtonGroup(this);
    butg->addButton(mUI->xtal320, 0);
    butg->addButton(mUI->xtal441, 1);
    butg->addButton(mUI->xtal480, 2);
    butg->addButton(mUI->xtal882, 3);
    butg->addButton(mUI->xtal960, 4);
    connect(butg, SIGNAL(buttonClicked(int)), this, SLOT(internalClockRateChanged(int)));
    mHWGroups[HW_ENUM_INTERNAL_CLOCK] = butg;

    // IEC958 clock rate
    mHWStrings[HW_ENUM_CLOCK_DEFAULT] = (QStringList() <<
        ENUM_RATE320 << ENUM_RATE441 << ENUM_RATE480 << ENUM_RATE882 << ENUM_RATE960);

    butg = new QButtonGroup(this);
    butg->addButton(mUI->iec320, 0);
    butg->addButton(mUI->iec441, 1);
    butg->addButton(mUI->iec480, 2);
    butg->addButton(mUI->iec882, 3);
    butg->addButton(mUI->iec960, 4);
    connect(butg, SIGNAL(buttonClicked(int)), this, SLOT(digitalClockRateChanged(int)));
    mHWGroups[HW_ENUM_CLOCK_DEFAULT] = butg;

    // De-emphasis
    mHWStrings[HW_ENUM_DEEMPHASIS] = (QStringList() <<
        ENUM_DOFF << ENUM_D320 << ENUM_D441 << ENUM_D480);

    butg = new QButtonGroup(this);
    butg->addButton(mUI->dOff, 0);
    butg->addButton(mUI->d320, 1);
    butg->addButton(mUI->d441, 2);
    butg->addButton(mUI->d480, 3);
    connect(butg, SIGNAL(buttonClicked(int)), this, SLOT(deemphasisChanged(int)));
    mHWGroups[HW_ENUM_DEEMPHASIS] = butg;


    // clock source
    mHWStrings[HW_ENUM_SWITCH_SOURCE] = (QStringList() <<
        ENUM_IEC958 << ENUM_INTERNAL);
    butg = new QButtonGroup(this);
    butg->addButton(mUI->syncDigital, 0);
    butg->addButton(mUI->syncInternal, 1);
    connect(butg, SIGNAL(buttonClicked(int)), this, SLOT(masterClockChanged(int)));
    mHWGroups[HW_ENUM_SWITCH_SOURCE] = butg;


    // Flags
    mHWFlags[HW_BOOL_RATE_RESET] = mUI->checkReset;
    mHWFlags[HW_BOOL_RATE_LOCKING] = mUI->checkLock;

}

void MainWindow::connectFromCard() {
    qDebug() << "entering";

    mUI->mixerAnalogIn->connectFromCard(mCard);
    mUI->mixerDigitalIn->connectFromCard(mCard);
    mUI->mixerPCM1->connectFromCard(mCard);
    mUI->mixerPCM4->connectFromCard(mCard);
    mUI->masterVolume->connectFromCard(mCard);


    mUI->analogOut->connectFromCard(mCard);
    mUI->digitalOut->connectFromCard(mCard);

    // Hardware settings tab
    connect(mCard, SIGNAL(boolConfigChanged(QString,bool)), SLOT(updateBoolConfig(const QString&, bool)));
    connect(mCard, SIGNAL(enumConfigChanged(const QString&, const QString&)), SLOT(updateEnumConfig(const QString&, const QString&)));

    qDebug() << "leaving";
}

void MainWindow::connectToCard() {
    qDebug() << "entering";

    mUI->mixerAnalogIn->connectToCard(mCard);
    mUI->mixerDigitalIn->connectToCard(mCard);
    mUI->mixerPCM1->connectToCard(mCard);
    mUI->mixerPCM4->connectToCard(mCard);
    mUI->masterVolume->connectToCard(mCard);

    mUI->analogOut->connectToCard(mCard);
    mUI->digitalOut->connectToCard(mCard);

    // Hardware settings tab
    connect(this, SIGNAL(boolConfigChanged(const QString&, bool)), mCard, SLOT(on_slot_boolConfigChanged(QString,bool)));
    connect(this, SIGNAL(enumConfigChanged(const QString&, const QString&)), mCard, SLOT(on_slot_enumConfigChanged(QString,QString)));

    qDebug() << "leaving";
}


int MainWindow::Color(int mx) {
    if (mx > 232) return 3; // red
    if (mx > 174) return 2; // yellow
    if (mx > 0) return 1; // green
    return 0; // black
}

void MainWindow::updateMeters() {

    EnvyCard::PeakMap levels = mCard->peaks();

    if (isVisible()) {
        foreach (int address, levels.keys()) {
            MixerInput* mix = qobject_cast<MixerInput*>(mRouting[address]);
            if (mix) {
                mix->updatePeaks(levels[address]);
                continue;
            }
            MasterVolume* master = qobject_cast<MasterVolume*>(mRouting[address]);
            if (master) {
                master->updatePeaks(levels[address]);
                continue;
            }
        }
    }

    int color = m_Color;

    m_CurrentSlot = (m_CurrentSlot + 1) % m_MaxPeakSize;
    // qDebug() << "Current slot = " << m_CurrentSlot;
    if (m_CurrentSlot == m_MaxPeakSlot) {
        // recalculate max slot and value
        m_MaxPeakValue = 0;
        for (int idx = 0; idx < m_MaxPeakSize - 1; idx++) {
            int slot = (m_CurrentSlot + 1 + idx) % m_MaxPeakSize;
            int value = m_MaxPeaks[slot];
            if (value >= m_MaxPeakValue) {
                m_MaxPeakValue = value;
                m_MaxPeakSlot = slot;
            }
        }
        color = Color(m_MaxPeakValue);
        // qDebug() << "Recalculated max value, slot = " << m_MaxPeakValue << m_MaxPeakSlot;
    }



    int slotmax = 0;
    foreach (int address, levels.keys()) {
        StereoLevels p = levels[address];
        int mx = p.left > p.right ? p.left : p.right;
        if (mx >= slotmax) slotmax = mx;
    }


    m_MaxPeaks[m_CurrentSlot] = slotmax;
    if (slotmax >= m_MaxPeakValue) {
        m_MaxPeakValue = slotmax;
        m_MaxPeakSlot = m_CurrentSlot;
        color = Color(m_MaxPeakValue);
        // qDebug() << "Updated max value, slot = " << m_MaxPeakValue << m_MaxPeakSlot;
    }


    if (color != m_Color) {
        // qDebug() << "Level color changed: " << m_Color << "=>" << color;
        m_Color = color;
        emit peakColorChanged(m_Color);
    }

    if (m_MaxPeakValue == 0 && mTimer->interval() != 1000) {
        mTimer->setInterval(1000);
        return;
    }

    if (mTimer->interval() == 1000 && m_MaxPeakValue != 0) {
        mTimer->setInterval(1000/m_PeakSampleFreq);
    }
}


void MainWindow::internalClockRateChanged(int id) {
    qDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QString enumVal = mHWStrings[HW_ENUM_INTERNAL_CLOCK][id];
    qDebug() << "notify card " << enumVal;
    emit enumConfigChanged(HW_ENUM_INTERNAL_CLOCK, enumVal);
    qDebug() << "leaving";
}

void MainWindow::digitalClockRateChanged(int id) {
    qDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QString enumVal = mHWStrings[HW_ENUM_CLOCK_DEFAULT][id];
    qDebug() << "notify card " << enumVal;
    emit enumConfigChanged(HW_ENUM_CLOCK_DEFAULT, enumVal);
    qDebug() << "leaving";
}

void MainWindow::deemphasisChanged(int id) {
    qDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QString enumVal = mHWStrings[HW_ENUM_DEEMPHASIS][id];
    qDebug() << "notify card " << enumVal;
    emit enumConfigChanged(HW_ENUM_DEEMPHASIS, enumVal);
    qDebug() << "leaving";
}

void MainWindow::masterClockChanged(int id) {
    qDebug() << "entering" << id;
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QString enumVal = mHWStrings[HW_ENUM_SWITCH_SOURCE][id];

    if (enumVal == ENUM_IEC958) {
        mUI->digitalGroup->setEnabled(true);
        mUI->internalRate->setEnabled(false);
        qDebug() << "notify card " << ENUM_IEC958;
        emit enumConfigChanged(HW_ENUM_INTERNAL_CLOCK, ENUM_IEC958);
    } else {
        mUI->digitalGroup->setEnabled(false);
        mUI->internalRate->setEnabled(true);
        int cid = mHWGroups[HW_ENUM_INTERNAL_CLOCK]->checkedId();
        enumVal = mHWStrings[HW_ENUM_INTERNAL_CLOCK][cid];
        qDebug() << "notify card " << enumVal;
        emit enumConfigChanged(HW_ENUM_INTERNAL_CLOCK, enumVal);
    }
    qDebug() << "leaving";
}


void MainWindow::on_checkLock_toggled(bool value) {
    qDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    qDebug() << "notify card " << value;
    emit boolConfigChanged(HW_BOOL_RATE_LOCKING, value);
    qDebug() << "leaving";
}

void MainWindow::on_checkReset_toggled(bool value) {
    qDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    qDebug() << "notify card " << value;
    emit boolConfigChanged(HW_BOOL_RATE_RESET, value);
    qDebug() << "leaving";
}


void MainWindow::updateBoolConfig(const QString& key, bool value) {
    qDebug() << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (inEventFlag) return;
    qDebug() << "setting " << key << " = " << value;
    QCheckBox* checkBox = mHWFlags[key];
    checkBox->setChecked(value);
    qDebug() << "leaving ";
}

void MainWindow::updateEnumConfig(const QString& key, const QString& value) {
    qDebug() << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (inEventFlag) return;
    qDebug() << "setting " << key << " = " << value;
    if (key == HW_ENUM_INTERNAL_CLOCK) {
        if (value == ENUM_IEC958) {
            mUI->digitalGroup->setEnabled(true);
            mUI->internalRate->setEnabled(false);
            int id = mHWStrings[HW_ENUM_SWITCH_SOURCE].indexOf(ENUM_IEC958);
            mHWGroups[HW_ENUM_SWITCH_SOURCE]->button(id)->setChecked(true);
            return;
        }
        mUI->digitalGroup->setEnabled(false);
        mUI->internalRate->setEnabled(true);
        int id = mHWStrings[HW_ENUM_SWITCH_SOURCE].indexOf(ENUM_INTERNAL);
        mHWGroups[HW_ENUM_SWITCH_SOURCE]->button(id)->setChecked(true);
    }
    int id = mHWStrings[key].indexOf(value);
    mHWGroups[key]->button(id)->setChecked(true);
    qDebug() << "leaving ";
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (mTrayItem->isActive()) {
        minimizeWindow();
        if (event->spontaneous()) {
            event->ignore();
        }
    } else {
        QMainWindow::closeEvent(event);
    }
}



void MainWindow::on_actionQuit_triggered() {
    qDebug() << "entering ";
    writeGUIState();
    qApp->quit();
    qDebug() << "leaving ";
}

void MainWindow::on_actionClose_triggered() {
    qDebug() << "entering ";
    close();
    qDebug() << "leaving ";
}

void MainWindow::on_actionSave_triggered() {
    writeMixerState();
}


void MainWindow::writeGUIState() {
    qDebug() << "entering ";
    Settings s;
    s.beginGroup("GUIState");
    s.setValue("docked", isHidden());
    s.setValue("show-statusbar", mUI->statusbar->isVisible());
    s.setValue("show-menubar", mUI->menubar->isVisible());
    s.setValue("geometry", geometry());
    s.endGroup();
    qDebug() << "leaving";
}

void MainWindow::writeMixerState() {
    qDebug() << "entering ";

    Settings s;
    s.beginGroup("MixerState");


    mUI->mixerPCM1->saveToConfig(s);
    mUI->mixerPCM4->saveToConfig(s);
    mUI->mixerAnalogIn->saveToConfig(s);
    mUI->mixerDigitalIn->saveToConfig(s);

    mUI->masterVolume->saveToConfig(s);

    mUI->analogOut->saveToConfig(s);
    mUI->digitalOut->saveToConfig(s);

    saveHWToConfig(s);

    s.endGroup();

    qDebug() << "leaving";
}


void MainWindow::readGUIState() {
    qDebug() << "entering ";
    Settings s;
    s.beginGroup("GUIState");
    mStartDocked = s.value("docked", false).toBool();
    mUI->actionMenubar->setChecked(s.value("show-menubar", true).toBool());
    mUI->actionStatusbar->setChecked(s.value("show-statusbar", true).toBool());
    QRect default_geom(30, 30, 600, 400);
    setGeometry(s.value("geometry", default_geom).toRect());
    s.endGroup();
    qDebug() << "leaving";
}


void MainWindow::readMixerState() {
    qDebug() << "entering ";
    Settings s;
    s.beginGroup("MixerState");


    mUI->mixerPCM1->loadFromConfig(s);
    mUI->mixerPCM4->loadFromConfig(s);
    mUI->mixerAnalogIn->loadFromConfig(s);
    mUI->mixerDigitalIn->loadFromConfig(s);

    mUI->masterVolume->loadFromConfig(s);

    mUI->analogOut->loadFromConfig(s);
    mUI->digitalOut->loadFromConfig(s);

    loadHWFromConfig(s);

    s.endGroup();

    qDebug() << "leaving";
}


void MainWindow::saveHWToConfig(Settings& s) {
    qDebug() << "entering ";

    s.beginGroup("HWState");

    s.setValue("clock-source", mHWGroups[HW_ENUM_SWITCH_SOURCE]->checkedId());
    s.setValue("internal-clock-rate",  mHWGroups[HW_ENUM_INTERNAL_CLOCK]->checkedId());
    s.setValue("digital-clock-rate", mHWGroups[HW_ENUM_CLOCK_DEFAULT]->checkedId());
    s.setValue("de-emphasis", mHWGroups[HW_ENUM_DEEMPHASIS]->checkedId());
    s.setValue("rate-reset", mHWFlags[HW_BOOL_RATE_RESET]->isChecked());
    s.setValue("rate-locked", mHWFlags[HW_BOOL_RATE_LOCKING]->isChecked());

    s.endGroup();
    qDebug() << "leaving";
}

void MainWindow::loadHWFromConfig(Settings& s) {
    qDebug()  << "entering ";

    s.beginGroup("HWState");

    // Internal clock rate
    int index = s.get("internal-clock-rate", mHWStrings[HW_ENUM_INTERNAL_CLOCK].indexOf(ENUM_RATE441));
    mHWGroups[HW_ENUM_INTERNAL_CLOCK]->button(index)->setChecked(true);

    // clock source
    index = s.get("clock-source", mHWStrings[HW_ENUM_SWITCH_SOURCE].indexOf(ENUM_INTERNAL));
    mHWGroups[HW_ENUM_SWITCH_SOURCE]->button(index)->setChecked(true);
    masterClockChanged(index);


    // IEC958 clock rate
    index = s.get("digital-clock-rate", mHWStrings[HW_ENUM_CLOCK_DEFAULT].indexOf(ENUM_RATE441));
    mHWGroups[HW_ENUM_CLOCK_DEFAULT]->button(index)->setChecked(true);
    if (mUI->digitalGroup->isEnabled()) {
        digitalClockRateChanged(index);
    }

    // De-emphasis
    index = s.get("de-emphasis", mHWStrings[HW_ENUM_DEEMPHASIS].indexOf(ENUM_DOFF));
    mHWGroups[HW_ENUM_DEEMPHASIS]->button(index)->setChecked(true);
    deemphasisChanged(index);

    // Reset flag
    bool checked = s.value("rate-reset", false).toBool();
    mHWFlags[HW_BOOL_RATE_RESET]->setChecked(checked);
    if (mUI->internalRate->isEnabled()) {
        on_checkReset_toggled(checked);
    }

    // Rate lock flag
    checked = s.value("rate-locked", false).toBool();
    mHWFlags[HW_BOOL_RATE_LOCKING]->setChecked(checked);
    if (mUI->internalRate->isEnabled()) {
        on_checkLock_toggled(checked);
    }

    qDebug() << "leaving";
}


MainWindow::~MainWindow() {
    delete mUI;
}


// dbus interface

void MainWindow::pcmPlaybackVolumeUp() {
    mUI->mixerPCM1->dbus_volumeIncrement(1);
}

void MainWindow::pcmPlaybackVolumeDown() {
    mUI->mixerPCM1->dbus_volumeIncrement(-1);
}

void MainWindow::pcmPlaybackMute() {
    mUI->mixerPCM1->dbus_volumeMute();
}

void MainWindow::iec958PlaybackVolumeUp() {
    mUI->mixerPCM4->dbus_volumeIncrement(1);
}
void MainWindow::iec958PlaybackVolumeDown() {
    mUI->mixerPCM4->dbus_volumeIncrement(-1);
}

void MainWindow::iec958PlaybackMute() {
    mUI->mixerPCM4->dbus_volumeMute();
}

void MainWindow::analogInVolumeUp() {
    mUI->mixerAnalogIn->dbus_volumeIncrement(1);
}
void MainWindow::analogInVolumeDown() {
    mUI->mixerAnalogIn->dbus_volumeIncrement(-1);
}
void MainWindow::analogInMute() {
    mUI->mixerAnalogIn->dbus_volumeMute();
}

void MainWindow::digitalInVolumeUp() {
    mUI->mixerDigitalIn->dbus_volumeIncrement(1);
}
void MainWindow::digitalInVolumeDown() {
    mUI->mixerDigitalIn->dbus_volumeIncrement(-1);
}
void MainWindow::digitalInMute() {
    mUI->mixerDigitalIn->dbus_volumeMute();
}

void MainWindow::dacVolumeUp() {
    mUI->masterVolume->dbus_volumeIncrement(1);
}
void MainWindow::dacVolumeDown() {
    mUI->masterVolume->dbus_volumeIncrement(-1);
}

int MainWindow::peakColor() const {
    return m_Color;
}


void MainWindow::restoreWindow() {
    bool wasHidden = isHidden();
    raise();
    showNormal();
    activateWindow();
    if (wasHidden && !mLastPos.isNull()) {
        move(mLastPos);
    }
}

void MainWindow::minimizeWindow() {
    mLastPos = pos();
    hide();
}



