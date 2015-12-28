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

MasterVolume::MasterVolume(QWidget* parent) :
    QWidget(parent),
    inSlotFlag(false),
    inEventFlag(false),
    mUI(new Ui::MasterVolume)
{
    mUI->setupUi(this);
    mLRDiff = 0;
}

MasterVolume::~MasterVolume() {
    delete mUI;
}

void MasterVolume::connectToCard(EnvyCard* envyCard) {
    connect(this, SIGNAL(adjusted(LeftRight, int)), envyCard, SLOT(on_slot_masterVolumeChanged(LeftRight,int)));
}

void MasterVolume::connectFromCard(EnvyCard* envyCard) {
    connect(envyCard, SIGNAL(masterVolumeChanged(LeftRight,int)), this, SLOT(analogUpdateDACVolume(LeftRight, int)));
}

void MasterVolume::updatePeaks(StereoLevels level) {
    mUI->leftMeter->updatePeak(level.left);
    mUI->rightMeter->updatePeak(level.right);
}



void MasterVolume::saveToConfig(KConfigBase* config) {
    KConfigGroup volGroup(config, objectName());
    volGroup.writeEntry("locked", mUI->checkLock->isChecked());
    volGroup.writeEntry("left-volume", mUI->leftSlider->value());
    volGroup.writeEntry("right-volume", mUI->rightSlider->value());
}

void MasterVolume::loadFromConfig(KConfigBase* config) {
    kDebug()  << "entering ";
    KConfigGroup volGroup(config, objectName());

    int val = volGroup.readEntry("left-volume", 20);
    mUI->leftSlider->setValue(val);
    on_leftSlider_valueChanged(val);

    val = volGroup.readEntry("right-volume", 20);
    mUI->rightSlider->setValue(val);
    on_rightSlider_valueChanged(val);


    bool locked = volGroup.readEntry("locked", true);
    if (locked) {
        mLRDiff = mUI->leftSlider->value() - mUI->rightSlider->value();
    } else {
        mLRDiff = 0;
    }
    mUI->checkLock->setChecked(locked);

    kDebug()  << "leaving";
}


void MasterVolume::analogUpdateDACVolume(LeftRight channel, int value) {
    kDebug()  << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        int dispVal = 127 - value;
        if (channel == LEFT) {
            kDebug()  << "set left";
            mUI->leftSlider->setValue(dispVal);
        } else {
            kDebug()  << "set right";
            mUI->rightSlider->setValue(dispVal);
        }
    }
    kDebug()  << "leaving";
}

void MasterVolume::on_leftSlider_valueChanged(int left) {
    kDebug()  << "entering";
    int right = left - mLRDiff;
    if (right < 0) {
        mUI->leftSlider->setValue(mLRDiff);
        return;
    }

    if (right > 127) {
        mUI->leftSlider->setValue(127 + mLRDiff);
        return;
    }

    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        int value = 127 - left;
        kDebug()  << "notify card";
        emit adjusted(LEFT, value);
        kDebug()  << "notify right";
        emit notifyRightVolume(right);
    }
    kDebug()  << "leaving";
}

void MasterVolume::on_rightSlider_valueChanged(int right) {
    kDebug()  << "entering";
    ExclusiveFlag inEvent(inEventFlag);

    int left = right + mLRDiff;
    if (left < 0) {
        mUI->rightSlider->setValue(-mLRDiff);
        return;
    }

    if (left > 127) {
        mUI->rightSlider->setValue(127 - mLRDiff);
        return;
    }

    if (!inSlotFlag) {
        int value  = 127 - right;
        kDebug()  << "notify card";
        emit adjusted(RIGHT, value);
        kDebug()  << "notify left";
        emit notifyLeftVolume(left);
    }
    kDebug()  << "leaving";
}

void MasterVolume::on_checkLock_toggled(bool locked) {
    kDebug()  << "entering ";
    if (locked) {
        mLRDiff = mUI->leftSlider->value() - mUI->rightSlider->value();
        connect(this, SIGNAL(notifyLeftVolume(int)), mUI->leftSlider, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyRightVolume(int)), mUI->rightSlider, SLOT(setValue(int)));
    } else {
        mLRDiff = 0;
        disconnect(this, SIGNAL(notifyLeftVolume(int)), mUI->leftSlider, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyRightVolume(int)), mUI->rightSlider, SLOT(setValue(int)));
    }
    kDebug()  << "leaving";
}

#include "mastervolume.moc"
