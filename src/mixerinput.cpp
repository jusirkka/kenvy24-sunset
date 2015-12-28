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

#include <qslider.h>
#include <qcheckbox.h>
#include <qdial.h>
#include <kconfig.h>
#include <kdebug.h>

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
    connect(this, SIGNAL(muted(int, LeftRight, bool)),
            envyCard, SLOT(on_slot_mixerMuteSwitchChanged(int,LeftRight,bool)));
    connect(this, SIGNAL(adjusted(int, LeftRight, const ChannelState&)),
            envyCard, SLOT(on_slot_mixerVolumeChanged(int,LeftRight,const ChannelState&)));
}

void MixerInput::connectFromCard(EnvyCard* envyCard) {
    connect(envyCard,SIGNAL(mixerMuteSwitchChanged(int,LeftRight,bool)),
            SLOT(mixerUpdateMuteSwitch(int,LeftRight,bool)));
    connect(envyCard,SIGNAL(mixerVolumeChanged(int,LeftRight, const ChannelState&)),
            SLOT(mixerUpdatePlaybackVolume(int,LeftRight, const ChannelState&)));
}


void MixerInput::updatePeaks(StereoLevels level) {
    mUI->leftMeter->updatePeak(level.left);
    mUI->rightMeter->updatePeak(level.right);
}

void MixerInput::saveToConfig(KConfigBase* config) {
    KConfigGroup volGroup(config, QString("%1-%2").arg(objectName()).arg(mAddress));
    volGroup.writeEntry("locked", mUI->checkLock->isChecked());

    volGroup.writeEntry("left-volume", mUI->leftVolume->value());
    volGroup.writeEntry("left-stereo", mUI->leftStereo->value());
    volGroup.writeEntry("left-mute", mUI->checkMuteLeft->isChecked());

    volGroup.writeEntry("right-volume", mUI->rightVolume->value());
    volGroup.writeEntry("right-stereo", mUI->rightStereo->value());
    volGroup.writeEntry("right-mute", mUI->checkMuteRight->isChecked());

}

void MixerInput::loadFromConfig(KConfigBase* config) {
    kDebug()  << "entering ";

    KConfigGroup volGroup(config, QString("%1-%2").arg(objectName()).arg(mAddress));

    int val = volGroup.readEntry("left-volume", 10);
    mUI->leftVolume->setValue(val);
    on_leftVolume_valueChanged(val);

    val = volGroup.readEntry("left-stereo", 96);
    mUI->leftStereo->setValue(val);
    on_leftStereo_valueChanged(val);

    val = volGroup.readEntry("right-volume", 10);
    mUI->rightVolume->setValue(val);
    on_rightVolume_valueChanged(val);

    val = volGroup.readEntry("right-stereo", 96);
    mUI->rightStereo->setValue(val);
    on_rightStereo_valueChanged(val);


    bool muted = volGroup.readEntry("left-mute", false);
    mUI->checkMuteLeft->setChecked(muted);

    muted = volGroup.readEntry("right-mute", false);
    mUI->checkMuteRight->setChecked(muted);


    bool locked = volGroup.readEntry("locked", true);

    if (locked) {
        mVDelta = mUI->leftVolume->value() - mUI->rightVolume->value();
        mSDelta = mUI->leftStereo->value() - mUI->rightStereo->value();
    } else {
        mVDelta = 0;
        mSDelta = 0;
    }

    mUI->checkLock->setChecked(locked);

    kDebug()  << "leaving ";
}


void MixerInput::mixerUpdateMuteSwitch(int address, LeftRight channel, bool muted) {

    if (address != mAddress) return;

    kDebug()  << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        if (channel == LEFT) {
            kDebug()  << "set left";
            mUI->checkMuteLeft->setChecked(muted);
        } else {
            kDebug()  << "set right";
            mUI->checkMuteRight->setChecked(muted);
        }
    }
    kDebug()  << "leaving";

}

void MixerInput::mixerUpdatePlaybackVolume(int address, LeftRight channel, const ChannelState& adj) {

    if (address != mAddress) return;

    kDebug()  << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        int dispVol = 96 - adj.volume;
        int dispStereo =  96 - adj.stereo;
        kDebug()  << "(volume, stereo) = (" << dispVol << ", " << dispStereo << ")";
        if (channel == LEFT) {
            kDebug()  << "set left";
            mUI->leftStereo->setValue(dispStereo);
            mUI->leftVolume->setValue(dispVol);
        } else {
            kDebug()  << "set right";
            mUI->rightStereo->setValue(dispStereo);
            mUI->rightVolume->setValue(dispVol);
        }
    }
    kDebug()  << "leaving";

}

void MixerInput::on_checkMuteLeft_toggled(bool m) {
    kDebug()  << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kDebug()  << "notify card";
        emit muted(mAddress, LEFT, m);
        kDebug()  << "notify right";
        emit notifyRightMute(m);
    }
    kDebug()  << "leaving";
}

void MixerInput::on_checkMuteRight_toggled(bool m) {
    kDebug()  << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kDebug()  << "notify card";
        emit muted(mAddress, RIGHT, m);
        kDebug()  << "notify left";
        emit notifyLeftMute(m);
    }
    kDebug()  << "leaving";
}

void MixerInput::on_leftVolume_valueChanged(int left) {
    kDebug()  << "entering";

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
    kDebug()  << "(volume, stereo) = (" << left << ", " << mUI->leftStereo->value() << ")";
    if (!inSlotFlag) {
        ChannelState vols(96 - left, 96 - mUI->leftStereo->value());
        kDebug()  << "notify card";
        emit adjusted(mAddress, LEFT, vols);
        kDebug()  << "notify right";
        emit notifyRightVolume(right);
    }
    kDebug()  << "leaving";
}

void MixerInput::on_rightVolume_valueChanged(int right) {
    kDebug()  << "entering";

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
    kDebug()  << "(volume, stereo) = (" << right << ", " << mUI->rightStereo->value() << ")";
    if (!inSlotFlag) {
        ChannelState vols(96 - right, 96 - mUI->rightStereo->value());
        kDebug()  << "notify card";
        emit adjusted(mAddress, RIGHT, vols);
        kDebug()  << "notify left";
        emit notifyLeftVolume(left);
    }
    kDebug()  << "leaving";
}

void MixerInput::on_leftStereo_valueChanged(int left) {
    kDebug()  << "entering";

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
    kDebug()  << "(volume, stereo) = (" << mUI->leftVolume->value() << ", " << left << ")";
    if (!inSlotFlag) {
        ChannelState vols(96 - mUI->leftVolume->value(), 96 - left);
        kDebug()  << "notify card";
        emit adjusted(mAddress, LEFT, vols);
        kDebug()  << "notify right";
        emit notifyRightStereo(right);
    }
    kDebug()  << "leaving";
}

void MixerInput::on_rightStereo_valueChanged(int right) {
    kDebug()  << "entering";

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
    kDebug()  << "(volume, stereo) = (" << mUI->rightVolume->value() << ", " << right << ")";
    if (!inSlotFlag) {
        ChannelState vols(96 - mUI->rightVolume->value(), 96 - right);
        kDebug()  << "notify card";
        emit adjusted(mAddress, RIGHT, vols);
        kDebug()  << "notify left";
        emit notifyLeftStereo(left);
    }
    kDebug()  << "leaving";
}

void MixerInput::on_checkLock_toggled(bool locked) {
    kDebug()  << "entering" << locked;
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
    kDebug()  << "leaving ";
}


void MixerInput::dbus_VolumeUp() {
    kDebug() << "entering";
    // negative values
    int lval = mUI->leftVolume->value();
    int rval = mUI->rightVolume->value();

    if (lval <= 0 || rval <= 0) {
        return;
    }

    mUI->leftVolume->setValue(lval - 1);

    if (!mUI->checkLock->isChecked()) {
        mUI->rightVolume->setValue(rval - 1);
    }
    kDebug() << "leaving";
}

void MixerInput::dbus_VolumeDown() {
    kDebug() << "entering";
    // negative values
    int lval = mUI->leftVolume->value();
    int rval = mUI->rightVolume->value();

    if (lval >= 96 || rval >= 96) {
        return;
    }

    mUI->leftVolume->setValue(lval + 1);

    if (!mUI->checkLock->isChecked()) {
        mUI->rightVolume->setValue(rval + 1);
    }
    kDebug() << "leaving";
}

void MixerInput::dbus_VolumeMute() {
    kDebug() << "entering";
    mUI->checkMuteLeft->toggle();
    if (!mUI->checkLock->isChecked()) {
        mUI->checkMuteRight->toggle();
    }
    kDebug() << "leaving";
}

#include "mixerinput.moc"
