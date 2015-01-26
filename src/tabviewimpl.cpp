/***************************************************************************
 *   Copyright (C) 2007 by Valentin Rusu                                   *
 *   kenvy24@rusu.info                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qtimer.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kactioncollection.h>
#include <kmainwindow.h>
#include <kdebug.h>
#include <kpushbutton.h>


#include "envycard.h"
#include "tabviewimpl.h"
#include "mixerinputimpl.h"
#include "mastervolumeimpl.h"
#include "patchboximpl.h"

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

TabViewImpl::TabViewImpl(QWidget* parent) :
        TabView(parent), envyCard(0),
        currentProfileItem(0),
        mLevelsEnabled(true),
        mLevelIndices(4),
        mUpdateInterval(20),
        mConfigRateLocking(HW_BOOL_RATE_LOCKING),
        mConfigRateReset(HW_BOOL_RATE_RESET),
        mConfigInternalClock(HW_ENUM_INTERNAL_CLOCK),
        mConfigClockDefault(HW_ENUM_CLOCK_DEFAULT),
        mConfigDeemphasis(HW_ENUM_DEEMPHASIS),
        mConfigIEC958(ENUM_IEC958)
{

    mLevelIndices[L_MASTER] = 20;
    mLevelIndices[L_PCM] = 0;
    mLevelIndices[L_ANALOG_IN] = 10;
    mLevelIndices[L_DIGITAL_IN] = 12;

    // hardware settings
    mConfigBoolVars << mConfigRateLocking << mConfigRateReset;
    mConfigCheckers.append(checkLock);
    mConfigCheckers.append(checkReset);

    mConfigEnumVars << mConfigInternalClock << mConfigClockDefault << mConfigDeemphasis;
    mConfigGroups.append(internalGroup);
    mConfigGroups.append(digitalGroup);
    mConfigGroups.append(deemphasisGroup);

    QStringList* enums = new QStringList;
    *enums << "1" << "2" << "3" << "4" << "5";
    (*enums)[internalGroup->id(xtal320)] = QString(ENUM_RATE320);
    (*enums)[internalGroup->id(xtal441)] = QString(ENUM_RATE441);
    (*enums)[internalGroup->id(xtal480)] = QString(ENUM_RATE480);
    (*enums)[internalGroup->id(xtal882)] = QString(ENUM_RATE882);
    (*enums)[internalGroup->id(xtal960)] = QString(ENUM_RATE960);
    mConfigEnums.append(enums);

    enums = new QStringList;
    *enums << "1" << "2" << "3" << "4" << "5";
    (*enums)[digitalGroup->id(iec320)] = QString(ENUM_RATE320);
    (*enums)[digitalGroup->id(iec441)] = QString(ENUM_RATE441);
    (*enums)[digitalGroup->id(iec480)] = QString(ENUM_RATE480);
    (*enums)[digitalGroup->id(iec882)] = QString(ENUM_RATE882);
    (*enums)[digitalGroup->id(iec960)] = QString(ENUM_RATE960);
    mConfigEnums.append(enums);

    enums = new QStringList;
    *enums << "1" << "2" << "3" << "4";
    (*enums)[deemphasisGroup->id(dOff)] = QString(ENUM_DOFF);
    (*enums)[deemphasisGroup->id(d320)] = QString(ENUM_D320);
    (*enums)[deemphasisGroup->id(d441)] = QString(ENUM_D441);
    (*enums)[deemphasisGroup->id(d480)] = QString(ENUM_D480);
    mConfigEnums.append(enums);

    // special value
    mIdxOfInternalSamples  = masterGroup->id(syncInternal);

    envyCard = new EnvyCard();
    if (! envyCard->foundEnvyCard()) {
        KMessageBox::error(this, "Cannot find an Envy24 chip based sound card in your system. Will now quit!");
        QTimer::singleShot(10, KApplication::kApplication(), SLOT(quit()));
        return;
    }

    setupTabs();
    connectToCard();
    connectFromCard();
    envyCard->pulse();
    loadProfiles();
    setupTimer();
}

TabViewImpl::~TabViewImpl() {
    delete envyCard;
}

void TabViewImpl::setupTabs() {

    mixerAnalogIn->inputGroup->setTitle("Analog In");
    mixerAnalogIn->setup(0);
    mixerDigitalIn->inputGroup->setTitle("Digital In");
    mixerDigitalIn->setup(0);
    mixerPCM1->inputGroup->setTitle("PCM Playback");
    mixerPCM1->setup(0);

    analogOut->patchGroup->setTitle("Analog Out");
    analogOut->setup(0);
    digitalOut->patchGroup->setTitle("Digital Out");
    digitalOut->setup(0);
}



void TabViewImpl::connectToCard() {
    QString capture("capture");
    QString spdif("spdif");

    mixerAnalogIn->connectToCard(envyCard, capture);
    mixerDigitalIn->connectToCard(envyCard, spdif);
    mixerPCM1->connectToCard(envyCard);
    masterVolume->connectToCard(envyCard);

    QString digital("digital");

    analogOut->connectToCard(envyCard);
    digitalOut->connectToCard(envyCard, digital);

    // Hardware settings tab
    connect(this, SIGNAL(boolConfigChanged(const QString&, bool)),
           envyCard, SLOT(setBoolConfig(const QString&, bool)));
    connect(this, SIGNAL(enumConfigChanged(const QString&, const QString&)),
           envyCard, SLOT(setEnumConfig(const QString&, const QString&)));

}


void TabViewImpl::connectFromCard() {
    QString capture("capture");
    QString spdif("spdif");

    mixerAnalogIn->connectFromCard(envyCard, capture);
    mixerDigitalIn->connectFromCard(envyCard, spdif);
    mixerPCM1->connectFromCard(envyCard);
    masterVolume->connectFromCard(envyCard);

    QString digital("digital");

    analogOut->connectFromCard(envyCard);
    digitalOut->connectFromCard(envyCard, digital);

    // Hardware settings tab
    connect(envyCard, SIGNAL(boolConfigUpdated(const QString&, bool)),
                         SLOT(updateBoolConfig(const QString&, bool)));
    connect(envyCard, SIGNAL(enumConfigUpdated(const QString&, const QString&)),
                         SLOT(updateEnumConfig(const QString&, const QString&)));
}



void TabViewImpl::updateMeters() {
    EnvyCard::PeakList peaks = envyCard->getPeaks(mLevelIndices);

    mixerAnalogIn->updatePeaks(peaks[L_ANALOG_IN]);
    mixerDigitalIn->updatePeaks(peaks[L_DIGITAL_IN]);
    mixerPCM1->updatePeaks(peaks[L_PCM]);
    masterVolume->updatePeaks(peaks[L_MASTER]);
}

void TabViewImpl::setupTimer() {
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), SLOT(updateMeters()));
    if (mLevelsEnabled) mTimer->start(mUpdateInterval, FALSE);
}

void TabViewImpl::enableLevels() {
    mLevelsEnabled = !mLevelsEnabled;
    KActionCollection* collection = ((KMainWindow*) parent())->actionCollection();
    if (mLevelsEnabled) {
        mTimer->start(mUpdateInterval, FALSE);
        collection->action("enable_levels")->setText(QString("Disable levels"));
    } else {
        mTimer->stop();
        collection->action("enable_levels")->setText(QString("Enable levels"));
    }
}


void TabViewImpl::setLevelsEnabled(bool enabled) {
    mLevelsEnabled = enabled;
    KActionCollection* collection = ((KMainWindow*) parent())->actionCollection();
    if (mLevelsEnabled) {
        mTimer->start(mUpdateInterval, FALSE);
        collection->action("enable_levels")->setText(QString("Disable levels"));
    } else {
        mTimer->stop();
        collection->action("enable_levels")->setText(QString("Enable levels"));
    }
}

void TabViewImpl::masterClockChanged(int id) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    if (id == mIdxOfInternalSamples) {
        digitalGroup->setEnabled(false);
        internalRate->setEnabled(true);
        QStringList* enums = mConfigEnums.at(mConfigEnumVars.findIndex(mConfigInternalClock));
        kdDebug() << k_funcinfo << "notify card " << (*enums)[internalGroup->selectedId()] << endl;
        emit enumConfigChanged(mConfigInternalClock, (*enums)[internalGroup->selectedId()]);
    } else {
        digitalGroup->setEnabled(true);
        internalRate->setEnabled(false);
        kdDebug() << k_funcinfo << "notify card " << mConfigIEC958 << endl;
        emit enumConfigChanged(mConfigInternalClock, mConfigIEC958);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void TabViewImpl::deemphasisChanged(int id) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QStringList* enums = mConfigEnums.at(mConfigEnumVars.findIndex(mConfigDeemphasis));
    kdDebug() << k_funcinfo << "notify card " << (*enums)[id] << endl;
    emit enumConfigChanged(mConfigDeemphasis, (*enums)[id]);
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void TabViewImpl::intRateChanged(int id) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QStringList* enums = mConfigEnums.at(mConfigEnumVars.findIndex(mConfigInternalClock));
    kdDebug() << k_funcinfo << "notify card " << (*enums)[id] << endl;
    emit enumConfigChanged(mConfigInternalClock, (*enums)[id]);
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void TabViewImpl::digRateChanged(int id) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    QStringList* enums = mConfigEnums.at(mConfigEnumVars.findIndex(mConfigClockDefault));
    kdDebug() << k_funcinfo << "notify card " << (*enums)[id] << endl;
    emit enumConfigChanged(mConfigClockDefault, (*enums)[id]);
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void TabViewImpl::rateLockToggled(bool toggle) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    kdDebug() << k_funcinfo << "notify card " << toggle << endl;
    emit boolConfigChanged(mConfigRateLocking, toggle);
    if (!(checkLock->isChecked() || checkReset->isChecked())) {
        internalGroup->setEnabled(false);
    } else {
        internalGroup->setEnabled(true);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void TabViewImpl::idleResetToggled(bool toggle) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (inSlotFlag) return;
    kdDebug() << k_funcinfo << "notify card " << toggle << endl;
    emit boolConfigChanged(mConfigRateReset, toggle);
    if (!(checkLock->isChecked() || checkReset->isChecked())) {
        internalGroup->setEnabled(false);
    } else {
        internalGroup->setEnabled(true);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}


void TabViewImpl::updateBoolConfig(const QString& var, bool value) {
    kdDebug() << k_funcinfo << "entering " << endl;
    ExclusiveFlag inSlot(inSlotFlag);
    if (inEventFlag) return;
    QCheckBox* checker = mConfigCheckers.at(mConfigBoolVars.findIndex(var));
    kdDebug() << k_funcinfo << "setting " << var << " = " << value << endl;
    checker->setChecked(value);
    kdDebug() << k_funcinfo << "leaving " << endl;
    if (!(checkLock->isChecked() || checkReset->isChecked())) {
        internalGroup->setEnabled(false);
    } else {
        internalGroup->setEnabled(true);
    }
}

void TabViewImpl::updateEnumConfig(const QString& var, const QString& value) {
    kdDebug() << k_funcinfo << "entering " << endl;
    ExclusiveFlag inSlot(inSlotFlag);
    if (inEventFlag) return;
    kdDebug() << k_funcinfo << "setting " << var << " = " << value << endl;
    if (var == mConfigInternalClock) {
        if (value == mConfigIEC958) {
            digitalGroup->setEnabled(true);
            internalRate->setEnabled(false);
            masterGroup->setButton((mIdxOfInternalSamples + 1) % 2);
            return;
        }
        digitalGroup->setEnabled(false);
        internalRate->setEnabled(true);
        masterGroup->setButton(mIdxOfInternalSamples);
    }
    int varId = mConfigEnumVars.findIndex(var);
    QButtonGroup* group = mConfigGroups.at(varId);
    QStringList* enums = mConfigEnums.at(varId);
    int id = enums->findIndex(value);
    group->setButton(id);
    kdDebug() << k_funcinfo << "leaving " << endl;
}

// ---------------------------------
// Profiles stuff
// ---------------------------------

void TabViewImpl::loadProfiles() {
    KConfig* config = kapp->config();
    QStringList groupList = config->groupList();
    for (QStringList::iterator it = groupList.begin(); it != groupList.end(); it++) {
        QString group = *it;
        if (group.startsWith("profile-")) {
            // this is a saved profile, so read it
            profileList->insertItem(group.remove("profile-"));
        }
    }
    config->setGroup(0); // general group
    QString lastProfile = config->readEntry("lastProfile");
    if (!lastProfile.isEmpty()) {
        profileList->setSelected(profileList->findItem(lastProfile), true);
    }
}


void TabViewImpl::newProfile() {
    QString profileName;

    for (;;) {
        profileName = KInputDialog::getText("New profile", "Enter profile name");
        QListBoxItem* existingItem = profileList->findItem(profileName);
        if (existingItem != 0) {
            QString message = QString("A profile named '%1' already exists. Please enter another name or cancel").arg(profileName);
            profileName.truncate(0);
            int action = KMessageBox::warningContinueCancel(this, message);
            if (action == KMessageBox::Continue)
                continue;
        }
        break;
    }

    if (profileName.length() >0) {
        profileList->insertItem(profileName);
        profileList->setSelected(profileList->findItem(profileName), true);
    }
}

void TabViewImpl::loadProfile(QListBoxItem* item) {
    if (item) {
        if (currentProfileItem != 0) {
            // TODO: prompt to save previously selected profile, if changes are detected
        }
        currentProfileItem = item;
        deleteButton->setEnabled(true);
        saveButton->setEnabled(true);

        // TODO; extract a method containing this loading stuff
        QString profileGroup = QString("profile-%1").arg(currentProfileItem->text());
        KConfig* config = kapp->config();
        if (config->hasGroup(profileGroup)) {
            config->setGroup(profileGroup);

            mixerPCM1->loadFromConfig(config);
            mixerAnalogIn->loadFromConfig(config);
            mixerDigitalIn->loadFromConfig(config);

            masterVolume->loadFromConfig(config);

            analogOut->loadFromConfig(config);
            digitalOut->loadFromConfig(config);

            // hardware settings
            int nval = config->readNumEntry(QString("digitalGroup-selected"));
            digitalGroup->setButton(nval);
            digRateChanged(nval);

            nval = config->readNumEntry(QString("internalGroup-selected"));
            internalGroup->setButton(nval);
            intRateChanged(nval);

            nval = config->readNumEntry(QString("masterGroup-selected"));
            masterGroup->setButton(nval);
            masterClockChanged(nval);

            nval = config->readNumEntry(QString("deemphasisGroup-selected"));
            deemphasisGroup->setButton(nval);
            deemphasisChanged(nval);

            bool bval = config->readBoolEntry(QString("checkLock-checked"));
            checkLock->setChecked(bval);
            rateLockToggled(bval);

            bval = config->readBoolEntry(QString("checkReset-checked"));
            checkReset->setChecked(bval);
            idleResetToggled(bval);

        }
        config->setGroup(0);
        config->writeEntry("lastProfile", currentProfileItem->text());
        config->sync();
    } else {
        deleteButton->setEnabled(false);
        saveButton->setEnabled(false);
    }
}

void TabViewImpl::updateProfile() {
    QString profileGroup = QString("profile-%1").arg(currentProfileItem->text());
    KConfig* config = kapp->config();
    config->setGroup(profileGroup);

    mixerPCM1->saveToConfig(config);
    mixerAnalogIn->saveToConfig(config);
    mixerDigitalIn->saveToConfig(config);

    masterVolume->saveToConfig(config);

    analogOut->saveToConfig(config);
    digitalOut->saveToConfig(config);

    // hardware settings
    config->writeEntry(QString("checkLock-checked"), checkLock->isChecked());
    config->writeEntry(QString("checkReset-checked"), checkReset->isChecked());
    config->writeEntry(QString("digitalGroup-selected"), digitalGroup->selectedId());
    config->writeEntry(QString("internalGroup-selected"), internalGroup->selectedId());
    config->writeEntry(QString("masterGroup-selected"), masterGroup->selectedId());
    config->writeEntry(QString("deemphasisGroup-selected"), deemphasisGroup->selectedId());

    // finally, save the profile
    config->sync();
}

void TabViewImpl::deleteProfile() {
    if (KMessageBox::questionYesNo(this, "Delete selected profile?") == KMessageBox::Yes) {
        if (currentProfileItem != 0) {
            KConfig* config = kapp->config();
            QString configGroup = QString("profile-%1").arg(currentProfileItem->text());
            if (!config->deleteGroup(configGroup)) {
                qWarning("Cannot delete profile group");
            }
            delete currentProfileItem;
        }
    }
}



#include "tabviewimpl.moc"
