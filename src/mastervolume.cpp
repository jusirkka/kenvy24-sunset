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
#include "peakmeter.h"
#include "envycard.h"

MasterVolume::MasterVolume(QWidget* parent, const char * name) : MasterVolume(parent, name) {}

void MasterVolume::connectToCard(EnvyCard* envyCard) {
    connect(this, SIGNAL(adjusted(LeftRight, int)), envyCard, SLOT(setDACVolume(LeftRight, int)));
}

void MasterVolume::connectFromCard(EnvyCard* envyCard) {
    connect(envyCard, SIGNAL(analogUpdateDACVolume(LeftRight, int)), this, SLOT(analogUpdateDACVolume(LeftRight, int)));
}

void MasterVolume::updatePeaks(StereoLevels level) {
    leftMeter->updatePeak(level.left);
    rightMeter->updatePeak(level.right);
}



void MasterVolume::saveToConfig(KConfig* config) {
    QString keyBase = name();
    config->writeEntry(QString("%1-locked").arg(keyBase), checkLock->isOn());

    keyBase = QString("%1-left").arg(name());
    config->writeEntry(QString("%1-vol").arg(keyBase), leftSlider->value());

    keyBase = QString("%1-right").arg(name());
    config->writeEntry(QString("%1-vol").arg(keyBase), rightSlider->value());

}

void MasterVolume::loadFromConfig(KConfig* config) {
    kdDebug() << k_funcinfo << "entering " << endl;

    QString keyBase = QString("%1-left").arg(name());
    int val = config->readNumEntry(QString("%1-vol").arg(keyBase));
    leftSlider->setValue(val);
    leftVolumeChanged(val);

    keyBase = QString("%1-right").arg(name());
    val = config->readNumEntry(QString("%1-vol").arg(keyBase));
    rightSlider->setValue(val);
    rightVolumeChanged(val);

    keyBase = name();
    checkLock->setChecked(config->readBoolEntry(QString("%1-locked").arg(keyBase)));

    kdDebug() << k_funcinfo << "leaving" << endl;
}


void MasterVolume::analogUpdateDACVolume(LeftRight channel, int value) {
    kdDebug() << k_funcinfo << "entering " << endl;
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        int dispVal = 127 - value;
        if (channel == LEFT) {
            kdDebug() << k_funcinfo << "set left" << endl;
            leftSlider->setValue(dispVal);
        } else {
            kdDebug() << k_funcinfo << "set right" << endl;
            rightSlider->setValue(dispVal);
        }
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MasterVolume::leftVolumeChanged(int dispVal) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        int value = 127 - dispVal;
        kdDebug() << k_funcinfo << "notify card" << endl;
        emit adjusted(LEFT, value);
        kdDebug() << k_funcinfo << "notify right" << endl;
        emit notifyRightVolume(dispVal);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MasterVolume::rightVolumeChanged(int dispVal) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        int value  = 127 - dispVal;
        kdDebug() << k_funcinfo << "notify card" << endl;
        emit adjusted(RIGHT, value);
        kdDebug() << k_funcinfo << "notify left" << endl;
        emit notifyLeftVolume(dispVal);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MasterVolume::lockToggled(bool locked) {
    kdDebug() << k_funcinfo << "entering " << endl;
    if (locked) {
        connect(this, SIGNAL(notifyLeftVolume(int)), leftSlider, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyRightVolume(int)), rightSlider, SLOT(setValue(int)));
    } else {
        disconnect(this, SIGNAL(notifyLeftVolume(int)), leftSlider, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyRightVolume(int)), rightSlider, SLOT(setValue(int)));
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

#include "mastervolume.moc"
