/***************************************************************************
 *   Copyright (C) 2007 by Valentin Rusu   *
 *   kenvy24@rusu.info   *
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

#include <QSlider>
#include <QCheckBox>
#include <kconfig.h>
#include <kdebug.h>


#include "mastervolume.h"
#include "ui_mastervolume.h"
#include "peakmeter.h"
#include "envycard.h"

MasterVolume::MasterVolume(QWidget* parent) : QWidget(parent), mUI(new Ui::MasterVolume) {
    mUI->setupUi(this);
}

MasterVolume::~MasterVolume() {
    delete mUI;
}

void MasterVolume::connectToCard(EnvyCard* envyCard) {
    connect(this, SIGNAL(adjusted(LeftRight, int)), envyCard, SLOT(setDACVolume(LeftRight, int)));
}

void MasterVolume::connectFromCard(EnvyCard* envyCard) {
    connect(envyCard, SIGNAL(analogUpdateDACVolume(LeftRight, int)), this, SLOT(analogUpdateDACVolume(LeftRight, int)));
}

void MasterVolume::updatePeaks(StereoLevels level) {
    mUI->leftMeter->updatePeak(level.left);
    mUI->rightMeter->updatePeak(level.right);
}



void MasterVolume::saveToConfig(KSharedConfigPtr config) {
    KConfigGroup volGroup(config, objectName());
    volGroup.writeEntry("locked", mUI->checkLock->isChecked());
    volGroup.writeEntry("left-volume", mUI->leftSlider->value());
    volGroup.writeEntry("right-volume", mUI->rightSlider->value());
}

void MasterVolume::loadFromConfig(KSharedConfigPtr config) {
    kDebug() << k_funcinfo << "entering ";
    KConfigGroup volGroup(config, objectName());

    int val = volGroup.readEntry("left-volume").toInt();
    mUI->leftSlider->setValue(val);
    leftVolumeChanged(val);

    val = volGroup.readEntry("right-volume").toInt();
    mUI->rightSlider->setValue(val);
    rightVolumeChanged(val);

    bool locked = volGroup.readEntry("locked").toInt();
    mUI->checkLock->setChecked(locked);

    kDebug() << k_funcinfo << "leaving";
}


void MasterVolume::analogUpdateDACVolume(LeftRight channel, int value) {
    kDebug() << k_funcinfo << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        int dispVal = 127 - value;
        if (channel == LEFT) {
            kDebug() << k_funcinfo << "set left";
            mUI->leftSlider->setValue(dispVal);
        } else {
            kDebug() << k_funcinfo << "set right";
            mUI->rightSlider->setValue(dispVal);
        }
    }
    kDebug() << k_funcinfo << "leaving";
}

void MasterVolume::leftVolumeChanged(int dispVal) {
    kDebug() << k_funcinfo << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        int value = 127 - dispVal;
        kDebug() << k_funcinfo << "notify card";
        emit adjusted(LEFT, value);
        kDebug() << k_funcinfo << "notify right";
        emit notifyRightVolume(dispVal);
    }
    kDebug() << k_funcinfo << "leaving";
}

void MasterVolume::rightVolumeChanged(int dispVal) {
    kDebug() << k_funcinfo << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        int value  = 127 - dispVal;
        kDebug() << k_funcinfo << "notify card";
        emit adjusted(RIGHT, value);
        kDebug() << k_funcinfo << "notify left";
        emit notifyLeftVolume(dispVal);
    }
    kDebug() << k_funcinfo << "leaving";
}

void MasterVolume::lockToggled(bool locked) {
    kDebug() << k_funcinfo << "entering ";
    if (locked) {
        connect(this, SIGNAL(notifyLeftVolume(int)), mUI->leftSlider, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyRightVolume(int)), mUI->rightSlider, SLOT(setValue(int)));
    } else {
        disconnect(this, SIGNAL(notifyLeftVolume(int)), mUI->leftSlider, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyRightVolume(int)), mUI->rightSlider, SLOT(setValue(int)));
    }
    kDebug() << k_funcinfo << "leaving";
}

#include "mastervolume.moc"
