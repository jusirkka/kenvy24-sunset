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
#include "settings.h"

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
    connect(this, SIGNAL(adjusted(Position, int)), envyCard, SLOT(on_slot_masterVolumeChanged(Position,int)));
}

void MasterVolume::connectFromCard(EnvyCard* envyCard) {
    connect(envyCard, SIGNAL(masterVolumeChanged(Position,int)), this, SLOT(analogUpdateDACVolume(Position, int)));
}

void MasterVolume::updatePeaks(const StereoLevels& level) {
    mUI->leftMeter->updatePeak(level.left);
    mUI->rightMeter->updatePeak(level.right);
}



void MasterVolume::saveToConfig(Settings& s) {
    s.beginGroup(objectName());
    s.setValue("locked", mUI->checkLock->isChecked());
    s.setValue("left-volume", mUI->leftSlider->value());
    s.setValue("right-volume", mUI->rightSlider->value());
    s.endGroup();
}

void MasterVolume::loadFromConfig(Settings& s) {
    s.beginGroup(objectName());

    int val = s.get("left-volume", 20);
    mUI->leftSlider->setValue(val);
    on_leftSlider_valueChanged(val);

    val = s.get("right-volume", 20);
    mUI->rightSlider->setValue(val);
    on_rightSlider_valueChanged(val);


    bool locked = s.value("locked", true).toBool();
    if (locked) {
        mLRDiff = mUI->leftSlider->value() - mUI->rightSlider->value();
    } else {
        mLRDiff = 0;
    }
    mUI->checkLock->setChecked(locked);

    s.endGroup();

}


void MasterVolume::analogUpdateDACVolume(Position channel, int value) {
    qDebug()  << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        int dispVal = 127 - value;
        if (channel == Position::Left) {
            qDebug()  << "set left";
            mUI->leftSlider->setValue(dispVal);
        } else {
            qDebug()  << "set right";
            mUI->rightSlider->setValue(dispVal);
        }
    }
    qDebug()  << "leaving";
}

void MasterVolume::on_leftSlider_valueChanged(int left) {
    qDebug()  << "entering";
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
        qDebug()  << "notify card";
        emit adjusted(Position::Left, value);
        qDebug()  << "notify right";
        emit notifyRightVolume(right);
    }
    qDebug()  << "leaving";
}

void MasterVolume::on_rightSlider_valueChanged(int right) {
    qDebug()  << "entering";
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
        qDebug()  << "notify card";
        emit adjusted(Position::Right, value);
        qDebug()  << "notify left";
        emit notifyLeftVolume(left);
    }
    qDebug()  << "leaving";
}

void MasterVolume::on_checkLock_toggled(bool locked) {
    qDebug()  << "entering ";
    if (locked) {
        mLRDiff = mUI->leftSlider->value() - mUI->rightSlider->value();
        connect(this, SIGNAL(notifyLeftVolume(int)), mUI->leftSlider, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyRightVolume(int)), mUI->rightSlider, SLOT(setValue(int)));
    } else {
        mLRDiff = 0;
        disconnect(this, SIGNAL(notifyLeftVolume(int)), mUI->leftSlider, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyRightVolume(int)), mUI->rightSlider, SLOT(setValue(int)));
    }
    qDebug()  << "leaving";
}

void MasterVolume::dbus_volumeIncrement(int incr) {

    qDebug() << "entering";
    // negative values
    int lval = mUI->leftSlider->value() - incr;
    int rval = mUI->rightSlider->value() - incr;

    if (lval < 0 || rval < 0 || lval > 127 || rval > 127) {
        return;
    }

    mUI->leftSlider->setValue(lval);

    if (!mUI->checkLock->isChecked()) {
        mUI->rightSlider->setValue(rval);
    }
    qDebug() << "leaving";
}

