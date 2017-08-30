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

#include "peakmeter.h"
#include "mixerinput.h"
#include "ui_mixerinput.h"
#include "envycard.h"
#include "settings.h"

#include <QSlider>
#include <QCheckBox>
#include <QDebug>

MixerInput::MixerInput(QWidget* parent) :
    QWidget(parent),
    inSlotFlag(false),
    inEventFlag(false),
    mUI(new Ui::MixerInput) {
    mUI->setupUi(this);
    mVDelta = 0;
    mSDelta = 0;
}

MixerInput::~MixerInput() {
    delete mUI;
}

void MixerInput::setup(int address, const QString& name, const QString& title, Routing& routing) {
    mAddress = address;
    routing[mAddress] = this;
    mUI->inputGroup->setTitle(title);
    setObjectName(name);
}


void MixerInput::connectToCard(EnvyCard* envyCard) {
    connect(this, SIGNAL(muted(int, Position, bool)),
            envyCard, SLOT(on_slot_mixerMuteSwitchChanged(int,Position,bool)));
    connect(this, SIGNAL(adjusted(int, Position, const ChannelState&)),
            envyCard, SLOT(on_slot_mixerVolumeChanged(int,Position,const ChannelState&)));
}

void MixerInput::connectFromCard(EnvyCard* envyCard) {
    connect(envyCard,SIGNAL(mixerMuteSwitchChanged(int,Position,bool)),
            SLOT(mixerUpdateMuteSwitch(int,Position,bool)));
    connect(envyCard,SIGNAL(mixerVolumeChanged(int,Position, const ChannelState&)),
            SLOT(mixerUpdatePlaybackVolume(int,Position, const ChannelState&)));
}


void MixerInput::updatePeaks(const StereoLevels& level) {
    mUI->leftMeter->updatePeak(level.left);
    mUI->rightMeter->updatePeak(level.right);
}

void MixerInput::saveToConfig(Settings& s) {
    s.beginGroup(QString("%1-%2").arg(objectName()).arg(mAddress));

    s.setValue("locked", mUI->checkLock->isChecked());

    s.setValue("left-volume", mUI->leftVolume->value());
    s.setValue("left-stereo", mUI->leftStereo->value());
    s.setValue("left-mute", mUI->checkMuteLeft->isChecked());

    s.setValue("right-volume", mUI->rightVolume->value());
    s.setValue("right-stereo", mUI->rightStereo->value());
    s.setValue("right-mute", mUI->checkMuteRight->isChecked());

    s.endGroup();
}

void MixerInput::loadFromConfig(Settings& s) {

    s.beginGroup(QString("%1-%2").arg(objectName()).arg(mAddress));

    int val = s.get("left-volume", 10);
    mUI->leftVolume->setValue(val);
    on_leftVolume_valueChanged(val);

    val = s.get("left-stereo", 96);
    mUI->leftStereo->setValue(val);
    on_leftStereo_valueChanged(val);

    val = s.get("right-volume", 10);
    mUI->rightVolume->setValue(val);
    on_rightVolume_valueChanged(val);

    val = s.get("right-stereo", 96);
    mUI->rightStereo->setValue(val);
    on_rightStereo_valueChanged(val);


    bool muted = s.value("left-mute", false).toBool();
    mUI->checkMuteLeft->setChecked(muted);
    on_checkMuteLeft_toggled(muted);

    muted = s.value("right-mute", false).toBool();
    mUI->checkMuteRight->setChecked(muted);
    on_checkMuteRight_toggled(muted);


    bool locked = s.value("locked", true).toBool();

    if (locked) {
        mVDelta = mUI->leftVolume->value() - mUI->rightVolume->value();
        mSDelta = mUI->leftStereo->value() - mUI->rightStereo->value();
    } else {
        mVDelta = 0;
        mSDelta = 0;
    }

    mUI->checkLock->setChecked(locked);

    s.endGroup();
}


void MixerInput::mixerUpdateMuteSwitch(int address, Position channel, bool muted) {

    if (address != mAddress) return;

    qDebug()  << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        if (channel == Position::Left) {
            qDebug()  << "set left";
            mUI->checkMuteLeft->setChecked(muted);
        } else {
            qDebug()  << "set right";
            mUI->checkMuteRight->setChecked(muted);
        }
    }
    qDebug()  << "leaving";

}

void MixerInput::mixerUpdatePlaybackVolume(int address, Position channel, const ChannelState& adj) {

    if (address != mAddress) return;

    qDebug()  << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        int dispVol = 96 - adj.volume;
        int dispStereo =  96 - adj.stereo;
        qDebug()  << "(volume, stereo) = (" << dispVol << ", " << dispStereo << ")";
        if (channel == Position::Left) {
            qDebug()  << "set left";
            mUI->leftStereo->setValue(dispStereo);
            mUI->leftVolume->setValue(dispVol);
        } else {
            qDebug()  << "set right";
            mUI->rightStereo->setValue(dispStereo);
            mUI->rightVolume->setValue(dispVol);
        }
    }
    qDebug()  << "leaving";

}

void MixerInput::on_checkMuteLeft_toggled(bool m) {
    qDebug()  << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        qDebug()  << "notify card";
        emit muted(mAddress, Position::Left, m);
        qDebug()  << "notify right";
        emit notifyRightMute(m);
    }
    qDebug()  << "leaving";
}

void MixerInput::on_checkMuteRight_toggled(bool m) {
    qDebug()  << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        qDebug()  << "notify card";
        emit muted(mAddress, Position::Right, m);
        qDebug()  << "notify left";
        emit notifyLeftMute(m);
    }
    qDebug()  << "leaving";
}

void MixerInput::on_leftVolume_valueChanged(int left) {
    qDebug()  << "entering";

    int right = left - mVDelta;
    if (right < 0) {
        mUI->leftVolume->setValue(mVDelta);
        return;
    }

    if (right > 96) {
        mUI->leftVolume->setValue(96 + mVDelta);
        return;
    }


    ExclusiveFlag inEvent(inEventFlag);
    qDebug()  << "(volume, stereo) = (" << left << ", " << mUI->leftStereo->value() << ")";
    if (!inSlotFlag) {
        ChannelState vols(96 - left, 96 - mUI->leftStereo->value());
        qDebug()  << "notify card";
        emit adjusted(mAddress, Position::Left, vols);
        qDebug()  << "notify right";
        emit notifyRightVolume(right);
    }
    qDebug()  << "leaving";
}

void MixerInput::on_rightVolume_valueChanged(int right) {
    qDebug()  << "entering";

    int left = right + mVDelta;
    if (left < 0) {
        mUI->rightVolume->setValue(-mVDelta);
        return;
    }

    if (left > 96) {
        mUI->rightVolume->setValue(96 - mVDelta);
        return;
    }

    ExclusiveFlag inEvent(inEventFlag);
    qDebug()  << "(volume, stereo) = (" << right << ", " << mUI->rightStereo->value() << ")";
    if (!inSlotFlag) {
        ChannelState vols(96 - right, 96 - mUI->rightStereo->value());
        qDebug()  << "notify card";
        emit adjusted(mAddress, Position::Right, vols);
        qDebug()  << "notify left";
        emit notifyLeftVolume(left);
    }
    qDebug()  << "leaving";
}

void MixerInput::on_leftStereo_valueChanged(int left) {
    qDebug()  << "entering";

    int right = left - mSDelta;
    if (right < 0) {
        mUI->leftStereo->setValue(mSDelta);
        return;
    }

    if (right > 96) {
        mUI->leftStereo->setValue(96 + mSDelta);
        return;
    }


    ExclusiveFlag inEvent(inEventFlag);
    qDebug()  << "(volume, stereo) = (" << mUI->leftVolume->value() << ", " << left << ")";
    if (!inSlotFlag) {
        ChannelState vols(96 - mUI->leftVolume->value(), 96 - left);
        qDebug()  << "notify card";
        emit adjusted(mAddress, Position::Left, vols);
        qDebug()  << "notify right";
        emit notifyRightStereo(right);
    }
    qDebug()  << "leaving";
}

void MixerInput::on_rightStereo_valueChanged(int right) {
    qDebug()  << "entering";

    int left = right + mSDelta;
    if (left < 0) {
        mUI->rightStereo->setValue(-mSDelta);
        return;
    }

    if (left > 96) {
        mUI->rightStereo->setValue(96 - mSDelta);
        return;
    }

    ExclusiveFlag inEvent(inEventFlag);
    qDebug()  << "(volume, stereo) = (" << mUI->rightVolume->value() << ", " << right << ")";
    if (!inSlotFlag) {
        ChannelState vols(96 - mUI->rightVolume->value(), 96 - right);
        qDebug()  << "notify card";
        emit adjusted(mAddress, Position::Right, vols);
        qDebug()  << "notify left";
        emit notifyLeftStereo(left);
    }
    qDebug()  << "leaving";
}

void MixerInput::on_checkLock_toggled(bool locked) {
    qDebug()  << "entering" << locked;
    if (locked) {
        mVDelta = mUI->leftVolume->value() - mUI->rightVolume->value();
        mSDelta = mUI->leftStereo->value() - mUI->rightStereo->value();
        connect(this, SIGNAL(notifyLeftVolume(int)), mUI->leftVolume, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyRightVolume(int)), mUI->rightVolume, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyLeftStereo(int)), mUI->leftStereo, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyRightStereo(int)), mUI->rightStereo, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyLeftMute(bool)), mUI->checkMuteLeft, SLOT(setChecked(bool)));
        connect(this, SIGNAL(notifyRightMute(bool)), mUI->checkMuteRight, SLOT(setChecked(bool)));
    } else {
        mVDelta = 0;
        mSDelta = 0;
        disconnect(this, SIGNAL(notifyLeftVolume(int)), mUI->leftVolume, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyRightVolume(int)), mUI->rightVolume, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyLeftStereo(int)), mUI->leftStereo, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyRightStereo(int)), mUI->rightStereo, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyLeftMute(bool)), mUI->checkMuteLeft, SLOT(setChecked(bool)));
        disconnect(this, SIGNAL(notifyRightMute(bool)), mUI->checkMuteRight, SLOT(setChecked(bool)));
    }
    qDebug()  << "leaving ";
}


void MixerInput::dbus_volumeIncrement(int incr) {
    qDebug() << "entering";
    // negative values
    int lval = mUI->leftVolume->value() - incr;
    int rval = mUI->rightVolume->value() - incr;

    if (lval < 0 || rval < 0 || lval > 96 || rval > 96) {
        return;
    }

    mUI->leftVolume->setValue(lval);

    if (!mUI->checkLock->isChecked()) {
        mUI->rightVolume->setValue(rval);
    }
    qDebug() << "leaving";
}


void MixerInput::dbus_volumeMute() {

    qDebug() << "entering";
    mUI->checkMuteLeft->toggle();
    if (!mUI->checkLock->isChecked()) {
        mUI->checkMuteRight->toggle();
    }
    qDebug() << "leaving";
}

