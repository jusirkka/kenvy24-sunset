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
#include "renamedialog.h"
#include "dbusiface.h"
#include "envycard.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <kdebug.h>
#include <kstatusnotifieritem.h>
#include <kapplication.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kactioncollection.h>
#include <QTimer>
#include <kmessagebox.h>


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

#define RC QString("%1rc").arg(kapp->applicationName())

MainWindow::MainWindow(DBusIface* dbus, bool docked):
    KMainWindow(),
    inSlotFlag(false),
    inEventFlag(false),
    mLevelIndices(4),
    mUI(new Ui::MainWindow),
    m_startDocked(false),
    m_shuttingDown(false),
    mUpdateInterval(20)
{

    try {
        mCard = &EnvyCard::Instance();
    } catch (CardNotFound) {
        KMessageBox::error(this, "Cannot find an Envy24 chip based sound card in your system. Will now quit!");
        QTimer::singleShot(10, kapp, SLOT(quit()));
        return;
    }


    mUI->setupUi(this);

    mUI->actionNewProfile->setEnabled(true);

    mUI->profiles->addAction(mUI->actionNewProfile);
    mUI->profiles->addAction(mUI->actionSaveProfile);
    mUI->profiles->addAction(mUI->actionRenameProfile);
    mUI->profiles->addAction(mUI->actionDeleteProfile);



    mUI->mixerPCM4->setup(mCard->PCMAddress(4), "mixer-iec958-in", "IEC958 Playback", mRouting);
    mUI->mixerPCM1->setup(mCard->PCMAddress(0), "mixer-pcm-in", "PCM Playback", mRouting);
    mUI->mixerAnalogIn->setup(mCard->AnalogInAddress(), "mixer-analog-in", "Analog In", mRouting);
    mUI->mixerDigitalIn->setup(mCard->DigitalInAddress(), "mixer-digital-in", "Digital In", mRouting);

    mUI->analogOut->setup(mCard->AnalogOutAddress(), "router-analog-out", "Analog Out", mRouting);
    mUI->digitalOut->setup(mCard->DigitalOutAddress(), "router-digital-out", "Digital Out", mRouting);

    mRouting[mCard->DACAddress()] = mUI->masterVolume;

    connect(dbus, SIGNAL(signalPCMVolumeDown()), mUI->mixerPCM1, SLOT(dbus_VolumeDown()));
    connect(dbus, SIGNAL(signalPCMVolumeUp()), mUI->mixerPCM1, SLOT(dbus_VolumeUp()));
    connect(dbus, SIGNAL(signalPCMVolumeMute()), mUI->mixerPCM1, SLOT(dbus_VolumeMute()));

    connect(dbus, SIGNAL(signalAnalogVolumeDown()), mUI->mixerAnalogIn, SLOT(dbus_VolumeDown()));
    connect(dbus, SIGNAL(signalAnalogVolumeUp()), mUI->mixerAnalogIn, SLOT(dbus_VolumeUp()));
    connect(dbus, SIGNAL(signalAnalogVolumeMute()), mUI->mixerAnalogIn, SLOT(dbus_VolumeMute()));

    connect(dbus, SIGNAL(signalDigitalVolumeDown()), mUI->mixerDigitalIn, SLOT(dbus_VolumeDown()));
    connect(dbus, SIGNAL(signalDigitalVolumeUp()), mUI->mixerDigitalIn, SLOT(dbus_VolumeUp()));
    connect(dbus, SIGNAL(signalDigitalVolumeMute()), mUI->mixerDigitalIn, SLOT(dbus_VolumeMute()));

    setupHWTab();

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(updateMeters()));

    readState();
    readProfiles();


    connectToCard();
    connectFromCard();

    if (!mUI->profiles->currentItem()) {
        // if no profiles, create a default
        createDefaultProfile();
    } else {
        loadProfile();
    }


    mTray = new KStatusNotifierItem(this);
    mTray->setObjectName("kenvy24 tray");
    mTray->setCategory(KStatusNotifierItem::Hardware);
    mTray->setStatus(KStatusNotifierItem::Active);
    mTray->setIconByName(QLatin1String("folder-sound"));
    mTray->setToolTip(QLatin1String("folder-sound"), i18n("Kenvy24"), i18n("Sound Mixer"));


    QAction* file_quit = mTray->actionCollection()->action("file_quit");
    file_quit->disconnect();
    connect(file_quit, SIGNAL(triggered()), this, SLOT(on_actionQuit_triggered()));

    if (docked || m_startDocked) {
        hide();
    } else {
        show();
    }

    if (mUI->actionEnableLeds->isChecked()) mTimer->start(mUpdateInterval);
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
    kDebug() << "entering";

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

    kDebug() << "leaving";
}

void MainWindow::connectToCard() {
    kDebug() << "entering";

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

    kDebug() << "leaving";
}


void MainWindow::updateMeters() {
    EnvyCard::PeakMap levels = mCard->peaks();

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


void MainWindow::internalClockRateChanged(int id) {
    kDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QString enumVal = mHWStrings[HW_ENUM_INTERNAL_CLOCK][id];
    kDebug() << "notify card " << enumVal;
    emit enumConfigChanged(HW_ENUM_INTERNAL_CLOCK, enumVal);
    kDebug() << "leaving";
}

void MainWindow::digitalClockRateChanged(int id) {
    kDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QString enumVal = mHWStrings[HW_ENUM_CLOCK_DEFAULT][id];
    kDebug() << "notify card " << enumVal;
    emit enumConfigChanged(HW_ENUM_CLOCK_DEFAULT, enumVal);
    kDebug() << "leaving";
}

void MainWindow::deemphasisChanged(int id) {
    kDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QString enumVal = mHWStrings[HW_ENUM_DEEMPHASIS][id];
    kDebug() << "notify card " << enumVal;
    emit enumConfigChanged(HW_ENUM_DEEMPHASIS, enumVal);
    kDebug() << "leaving";
}

void MainWindow::masterClockChanged(int id) {
    kDebug() << "entering" << id;
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QString enumVal = mHWStrings[HW_ENUM_SWITCH_SOURCE][id];

    if (enumVal == ENUM_IEC958) {
        mUI->digitalGroup->setEnabled(true);
        mUI->internalRate->setEnabled(false);
        kDebug() << "notify card " << ENUM_IEC958;
        emit enumConfigChanged(HW_ENUM_INTERNAL_CLOCK, ENUM_IEC958);
    } else {
        mUI->digitalGroup->setEnabled(false);
        mUI->internalRate->setEnabled(true);
        int cid = mHWGroups[HW_ENUM_INTERNAL_CLOCK]->checkedId();
        enumVal = mHWStrings[HW_ENUM_INTERNAL_CLOCK][cid];
        kDebug() << "notify card " << enumVal;
        emit enumConfigChanged(HW_ENUM_INTERNAL_CLOCK, enumVal);
    }
    kDebug() << "leaving";
}


void MainWindow::on_checkLock_toggled(bool value) {
    kDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    kDebug() << "notify card " << value;
    emit boolConfigChanged(HW_BOOL_RATE_LOCKING, value);
    kDebug() << "leaving";
}

void MainWindow::on_checkReset_toggled(bool value) {
    kDebug() << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    kDebug() << "notify card " << value;
    emit boolConfigChanged(HW_BOOL_RATE_RESET, value);
    kDebug() << "leaving";
}


void MainWindow::updateBoolConfig(const QString& key, bool value) {
    kDebug() << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (inEventFlag) return;
    kDebug() << "setting " << key << " = " << value;
    QCheckBox* checkBox = mHWFlags[key];
    checkBox->setChecked(value);
    kDebug() << "leaving ";
}

void MainWindow::updateEnumConfig(const QString& key, const QString& value) {
    kDebug() << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (inEventFlag) return;
    kDebug() << "setting " << key << " = " << value;
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
    kDebug() << "leaving ";
}



bool MainWindow::queryClose() {
    kDebug() << "entering ";
    if (!m_shuttingDown &&!kapp->sessionSaving() && mTray) {
         hide();
         kDebug() << "leaving: false ";
         return false;
     } else {
          m_startDocked = !isVisible();
          writeState();
          kDebug() << "leaving: true";
          return true;
    }
}

void MainWindow::on_profiles_currentItemChanged(QListWidgetItem* curr, QListWidgetItem*) {
    kDebug() << "entering ";
    if (!curr) {
        mUI->actionRenameProfile->setEnabled(false);
        mUI->actionSaveProfile->setEnabled(false);
        mUI->actionDeleteProfile->setEnabled(false);
    } else {
        loadProfile();
    }
    kDebug() << "leaving";
}

QString MainWindow::uniqueName(const QString& name) {
    QStringList names;
    QString uniq(name);
    for (int i = 0; i < mUI->profiles->count(); i++) {
        names << mUI->profiles->item(i)->text();
    }

    while (names.contains(uniq)) {
        uniq = QString("%1'").arg(uniq);
    }

    return uniq;
}

void MainWindow::on_actionNewProfile_triggered() {
    QListWidgetItem* p = new QListWidgetItem(uniqueName("new profile"), mUI->profiles);
    mUI->profiles->setCurrentItem(p);
    writeProfile();
}

void MainWindow::on_actionSaveProfile_triggered() {
    writeProfile();
}

void MainWindow::on_actionDeleteProfile_triggered() {
    kDebug() << "entering";
    KConfig base(RC);
    KConfigGroup profiles(&base, "Profiles");
    KConfigGroup profile(&profiles, mUI->profiles->currentItem()->text());
    profile.deleteGroup();
    int currentRow = mUI->profiles->row(mUI->profiles->currentItem());
    QListWidgetItem* popped = mUI->profiles->takeItem(currentRow);
    mUI->profiles->setCurrentItem(0);
    delete popped;
    loadProfile();
    kDebug() << "leaving";
}

void MainWindow::on_actionRenameProfile_triggered() {
    KConfig base(RC);
    KConfigGroup profiles(&base, "Profiles");
    KConfigGroup from_profile(&profiles, mUI->profiles->currentItem()->text());
    RenameDialog dialog(this, mUI->profiles->currentItem()->text());
    if (dialog.exec() == QDialog::Accepted) {
        mUI->profiles->currentItem()->setText(uniqueName(dialog.name()));
        KConfigGroup to_profile(&profiles, mUI->profiles->currentItem()->text());
        from_profile.copyTo(&to_profile);
        from_profile.deleteGroup();
    }
}

void MainWindow::on_actionEnableLeds_toggled(bool on) {
    if (on) {
        mTimer->start(mUpdateInterval);
    } else {
        mTimer->stop();
    }
}

void MainWindow::on_actionQuit_triggered() {
    kDebug() << "entering ";
    m_shuttingDown = true;
    m_startDocked = !isVisible();
    writeState();
    kapp->quit();
    kDebug() << "leaving ";
}

void MainWindow::on_actionClose_triggered() {
    kDebug() << "entering ";
    close();
    kDebug() << "leaving ";
}



void MainWindow::writeState() {
    kDebug() << "entering ";
    KConfig base(RC);
    KConfigGroup state(&base, "State");
    state.writeEntry("docked", m_startDocked);
    state.writeEntry("show-toolbar", mUI->toolBar->isVisible());
    state.writeEntry("show-statusbar", mUI->statusbar->isVisible());
    state.writeEntry("show-profiles", mUI->profileDock->isVisible());
    state.writeEntry("enable-leds", mUI->actionEnableLeds->isChecked());
    if (mUI->profiles->currentItem()) {
        state.writeEntry("selected-profile", mUI->profiles->currentItem()->text());
    }
    state.writeEntry("geometry", geometry());
    state.writeEntry("profiles-dock-area", (int) dockWidgetArea(mUI->profileDock));
    kDebug() << "leaving";
}

void MainWindow::writeProfile() {
    kDebug() << "entering ";
    if (!mUI->profiles->currentItem()) {
        return;
    }
    KConfig base(RC);
    KConfigGroup profiles(&base, "Profiles");
    KConfigGroup current(&profiles, mUI->profiles->currentItem()->text());


    mUI->mixerPCM1->saveToConfig(&current);
    mUI->mixerAnalogIn->saveToConfig(&current);
    mUI->mixerDigitalIn->saveToConfig(&current);

    mUI->masterVolume->saveToConfig(&current);

    mUI->analogOut->saveToConfig(&current);
    mUI->digitalOut->saveToConfig(&current);

    saveHWToConfig(&current);
    kDebug() << "leaving";
}

void MainWindow::createDefaultProfile() {
    kDebug() << "entering";
    mCard->pulse();
    KConfig base(RC);
    KConfigGroup profiles(&base, "Profiles");
    KConfigGroup current(&profiles, "default");
    mUI->mixerPCM1->saveToConfig(&current);
    mUI->mixerPCM4->saveToConfig(&current);
    mUI->mixerAnalogIn->saveToConfig(&current);
    mUI->mixerDigitalIn->saveToConfig(&current);
    mUI->masterVolume->saveToConfig(&current);
    mUI->analogOut->saveToConfig(&current);
    mUI->digitalOut->saveToConfig(&current);
    QListWidgetItem* def = new QListWidgetItem("default", mUI->profiles);
    mUI->profiles->setCurrentItem(def);
    kDebug() << "leaving";
}

void MainWindow::readState() {
    kDebug() << "entering ";
    KConfig base(RC);
    KConfigGroup state(&base, "State");
    m_startDocked = state.readEntry("docked", false);
    mUI->actionToolbar->setChecked(state.readEntry("show-toolbar", true));
    mUI->actionStatusbar->setChecked(state.readEntry("show-statusbar", true));
    mUI->actionProfileDock->setChecked(state.readEntry("show-profiles", true));
    mUI->actionEnableLeds->setChecked(state.readEntry("enable-leds", true));
    QRect default_geom(30, 30, 600, 400);
    setGeometry(state.readEntry("geometry", default_geom));
    addDockWidget((Qt::DockWidgetArea) state.readEntry("profiles-dock-area", (int) Qt::LeftDockWidgetArea), mUI->profileDock);
    kDebug() << "leaving";
}

void MainWindow::readProfiles() {
    kDebug() << "entering ";
    KConfig base(RC);
    KConfigGroup state(&base, "State");
    QString selected = state.readEntry("selected-profile", "no-selection");
    KConfigGroup profiles(&base, "Profiles");
    foreach (const QString& profile, profiles.groupList()) {
        QListWidgetItem* item = new QListWidgetItem(profile, mUI->profiles);
        if (profile == selected) {
            mUI->profiles->setCurrentItem(item);
        }
    }
    if (!mUI->profiles->currentItem() && mUI->profiles->count()) {
        mUI->profiles->setCurrentItem(mUI->profiles->item(0));
    }
    kDebug() << "leaving";
}

void MainWindow::loadProfile() {
    kDebug() << "entering ";
    if (!mUI->profiles->currentItem()) {
        mUI->actionRenameProfile->setEnabled(false);
        mUI->actionSaveProfile->setEnabled(false);
        mUI->actionDeleteProfile->setEnabled(false);
        return;
    }
    mUI->actionRenameProfile->setEnabled(true);
    mUI->actionSaveProfile->setEnabled(true);
    mUI->actionDeleteProfile->setEnabled(true);

    KConfig base(RC);
    KConfigGroup profiles(&base, "Profiles");
    KConfigGroup current(&profiles, mUI->profiles->currentItem()->text());


    mUI->mixerPCM1->loadFromConfig(&current);
    mUI->mixerPCM4->loadFromConfig(&current);
    mUI->mixerAnalogIn->loadFromConfig(&current);
    mUI->mixerDigitalIn->loadFromConfig(&current);

    mUI->masterVolume->loadFromConfig(&current);

    mUI->analogOut->loadFromConfig(&current);
    mUI->digitalOut->loadFromConfig(&current);

    loadHWFromConfig(&current);
    kDebug() << "leaving";
}


void MainWindow::saveHWToConfig(KConfigBase* profile) {
    kDebug() << "entering ";
    KConfigGroup hw(profile, "hw-settings");

    hw.writeEntry("clock-source", mHWGroups[HW_ENUM_SWITCH_SOURCE]->checkedId());
    hw.writeEntry("internal-clock-rate",  mHWGroups[HW_ENUM_INTERNAL_CLOCK]->checkedId());
    hw.writeEntry("digital-clock-rate", mHWGroups[HW_ENUM_CLOCK_DEFAULT]->checkedId());
    hw.writeEntry("de-emphasis", mHWGroups[HW_ENUM_DEEMPHASIS]->checkedId());
    hw.writeEntry("rate-reset", mHWFlags[HW_BOOL_RATE_RESET]->isChecked());
    hw.writeEntry("rate-locked", mHWFlags[HW_BOOL_RATE_LOCKING]->isChecked());
    kDebug() << "leaving";
}

void MainWindow::loadHWFromConfig(KConfigBase* profile) {
    kDebug()  << "entering ";
    KConfigGroup hw(profile, "hw-settings");

    // Internal clock rate
    int index = hw.readEntry("internal-clock-rate", mHWStrings[HW_ENUM_INTERNAL_CLOCK].indexOf(ENUM_RATE441));
    mHWGroups[HW_ENUM_INTERNAL_CLOCK]->button(index)->setChecked(true);

    // clock source
    index = hw.readEntry("clock-source", mHWStrings[HW_ENUM_SWITCH_SOURCE].indexOf(ENUM_INTERNAL));
    kDebug() << index;
    mHWGroups[HW_ENUM_SWITCH_SOURCE]->button(index)->setChecked(true);
    masterClockChanged(index);



    // IEC958 clock rate
    index = hw.readEntry("digital-clock-rate", mHWStrings[HW_ENUM_CLOCK_DEFAULT].indexOf(ENUM_RATE441));
    kDebug() << index;
    mHWGroups[HW_ENUM_CLOCK_DEFAULT]->button(index)->setChecked(true);
    if (mUI->digitalGroup->isEnabled()) {
        digitalClockRateChanged(index);
    }

    // De-emphasis
    index = hw.readEntry("de-emphasis", mHWStrings[HW_ENUM_DEEMPHASIS].indexOf(ENUM_DOFF));
    kDebug() << "deemphasis" << index;
    mHWGroups[HW_ENUM_DEEMPHASIS]->button(index)->setChecked(true);
    deemphasisChanged(index);

    // Reset flag
    bool checked = hw.readEntry("rate-reset", false);
    mHWFlags[HW_BOOL_RATE_RESET]->setChecked(checked);
    if (mUI->internalRate->isEnabled()) {
        on_checkReset_toggled(checked);
    }

    // Rate lock flag
    checked = hw.readEntry("rate-locked", false);
    mHWFlags[HW_BOOL_RATE_LOCKING]->setChecked(checked);
    if (mUI->internalRate->isEnabled()) {
        on_checkLock_toggled(checked);
    }

    kDebug() << "leaving";
}


MainWindow::~MainWindow() {
    delete mUI;
}

#include "mainwindow.moc"
