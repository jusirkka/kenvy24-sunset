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
#include "envystructs.h"

#include <qslider.h>
#include <qcheckbox.h>
#include <qdial.h>
#include <kconfig.h>
#include <kdebug.h>

MixerInput::MixerInput(QWidget* parent) : QWidget(parent), mUI(new Ui::MixerInput) {
    mUI->setupUi(this);
    mVDelta = 0;
    mSDelta = 0;
}

MixerInput::~MixerInput() {
    delete mUI;
}

void MixerInput::setup(int index) {
    mIndex = index;
}

void MixerInput::setTitle(const QString& title) {
    mUI->inputGroup->setTitle(title);
}


void MixerInput::connectToCard(EnvyCard* envyCard, const QString& inout) {
    if (inout == "capture") {
        connect(this, SIGNAL(muted(int, LeftRight, bool)),
               envyCard, SLOT(mixerMuteCaptureChannel(int, LeftRight, bool)));
        connect(this, SIGNAL(adjusted(int, LeftRight, int, int)),
               envyCard, SLOT(setMixerCaptureVolume(int,LeftRight,int, int)));
        return;
    }

    if (inout == "spdif") {
        connect(this, SIGNAL(muted(int, LeftRight, bool)),
               envyCard, SLOT(mixerMuteSPDIFChannel(int, LeftRight, bool)));
        connect(this, SIGNAL(adjusted(int, LeftRight, int, int)),
               envyCard, SLOT(setMixerSPDIFVolume(int,LeftRight,int,int)));
        return;
    }

    connect(this, SIGNAL(muted(int, LeftRight, bool)),
           envyCard, SLOT(mixerMutePlaybackChannel(int, LeftRight, bool)));
    connect(this, SIGNAL(adjusted(int, LeftRight, int, int)),
           envyCard, SLOT(setMixerPlaybackVolume(int, LeftRight, int, int)));
}

void MixerInput::connectFromCard(EnvyCard* envyCard, const QString& inout) {

    if (inout == "capture") {
        connect(envyCard,SIGNAL(mixerUpdateAnalogInMuteSwitch(int,LeftRight,bool)),
                            SLOT(mixerUpdateMuteSwitch(int,LeftRight,bool)));
        connect(envyCard,SIGNAL(mixerUpdateAnalogInVolume(int,LeftRight,MixerAdjustement)),
                            SLOT(mixerUpdatePlaybackVolume(int,LeftRight,MixerAdjustement)));
        return;
    }

    if (inout == "spdif") {
        connect(envyCard, SIGNAL(mixerUpdateSPDIFInMuteSwitch(int, LeftRight, bool)),
                            SLOT(mixerUpdateMuteSwitch(int,  LeftRight, bool)));
        connect(envyCard, SIGNAL(mixerUpdateSPDIFVolume(int, LeftRight, MixerAdjustement)),
                             SLOT(mixerUpdatePlaybackVolume(int, LeftRight, MixerAdjustement)));
        return;
    }

    connect(envyCard, SIGNAL(mixerUpdatePCMMuteSwitch(int, LeftRight, bool)),
                         SLOT(mixerUpdateMuteSwitch(int, LeftRight, bool)));
    connect(envyCard, SIGNAL(mixerUpdatePlaybackVolume(int, LeftRight, MixerAdjustement)),
                        SLOT(mixerUpdatePlaybackVolume(int, LeftRight, MixerAdjustement)));
}


void MixerInput::updatePeaks(StereoLevels level) {
    mUI->leftMeter->updatePeak(level.left);
    mUI->rightMeter->updatePeak(level.right);
}

void MixerInput::saveToConfig(KSharedConfigPtr config) {
    KConfigGroup volGroup(config, QString("%1-%2").arg(objectName()).arg(mIndex));
    volGroup.writeEntry("locked", mUI->checkLock->isChecked());

    volGroup.writeEntry("left-volume", mUI->leftVolume->value());
    volGroup.writeEntry("left-stereo", mUI->leftStereo->value());
    volGroup.writeEntry("left-mute", mUI->checkMuteLeft->isChecked());

    volGroup.writeEntry("right-volume", mUI->rightVolume->value());
    volGroup.writeEntry("right-stereo", mUI->rightStereo->value());
    volGroup.writeEntry("right-mute", mUI->checkMuteRight->isChecked());

}

void MixerInput::loadFromConfig(KSharedConfigPtr config) {
    kDebug() << k_funcinfo << "entering ";

    KConfigGroup volGroup(config, QString("%1-%2").arg(objectName()).arg(mIndex));

    int val = volGroup.readEntry("left-volume").toInt();
    mUI->leftVolume->setValue(val);
    on_leftVolume_valueChanged(val);

    val = volGroup.readEntry("left-stereo").toInt();
    mUI->leftStereo->setValue(val);
    on_leftStereo_valueChanged(val);

    val = volGroup.readEntry("right-volume").toInt();
    mUI->rightVolume->setValue(val);
    on_rightVolume_valueChanged(val);

    val = volGroup.readEntry("right-stereo").toInt();
    mUI->rightStereo->setValue(val);
    on_rightStereo_valueChanged(val);


    bool muted = volGroup.readEntry("left-mute").toInt();
    mUI->checkMuteLeft->setChecked(muted);

    muted = volGroup.readEntry("right-mute").toInt();
    mUI->checkMuteRight->setChecked(muted);


    bool locked = volGroup.readEntry("locked").toInt();

    if (locked) {
        mVDelta = mUI->leftVolume->value() - mUI->rightVolume->value();
        mSDelta = mUI->leftStereo->value() - mUI->rightStereo->value();
    } else {
        mVDelta = 0;
        mSDelta = 0;
    }

    mUI->checkLock->setChecked(locked);

    kDebug() << k_funcinfo << "leaving ";
}


void MixerInput::mixerUpdateMuteSwitch(int index, LeftRight channel, bool muted) {

    if (index != mIndex) return;

    kDebug() << k_funcinfo << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        if (channel == LEFT) {
            kDebug() << k_funcinfo << "set left";
            mUI->checkMuteLeft->setChecked(muted);
        } else {
            kDebug() << k_funcinfo << "set right";
            mUI->checkMuteRight->setChecked(muted);
        }
    }
    kDebug() << k_funcinfo << "leaving";

}

void MixerInput::mixerUpdatePlaybackVolume(int index, LeftRight channel, MixerAdjustement adj) {

    if (index != mIndex) return;

    kDebug() << k_funcinfo << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        int dispVol = 96 - adj.volume;
        int dispStereo =  96 - adj.stereo;
        kDebug() << k_funcinfo << "(volume, stereo) = (" << dispVol << ", " << dispStereo << ")"<< endl;
        if (channel == LEFT) {
            kDebug() << k_funcinfo << "set left";
            mUI->leftStereo->setValue(dispStereo);
            mUI->leftVolume->setValue(dispVol);
        } else {
            kDebug() << k_funcinfo << "set right";
            mUI->rightStereo->setValue(dispStereo);
            mUI->rightVolume->setValue(dispVol);
        }
    }
    kDebug() << k_funcinfo << "leaving";

}

void MixerInput::on_checkMuteLeft_toggled(bool m) {
    kDebug() << k_funcinfo << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kDebug() << k_funcinfo << "notify card";
        emit muted(mIndex, LEFT, m);
        kDebug() << k_funcinfo << "notify right";
        emit notifyRightMute(m);
    }
    kDebug() << k_funcinfo << "leaving";
}

void MixerInput::on_checkMuteRight_toggled(bool m) {
    kDebug() << k_funcinfo << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kDebug() << k_funcinfo << "notify card";
        emit muted(mIndex, RIGHT, m);
        kDebug() << k_funcinfo << "notify left";
        emit notifyLeftMute(m);
    }
    kDebug() << k_funcinfo << "leaving";
}

void MixerInput::on_leftVolume_valueChanged(int left) {
    kDebug() << k_funcinfo << "entering";

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
    kDebug() << k_funcinfo << "(volume, stereo) = (" << left << ", " << mUI->leftStereo->value() << ")"<< endl;
    if (!inSlotFlag) {
        int vol = 96 - left;
        int stereo = 96 - mUI->leftStereo->value();
        kDebug() << k_funcinfo << "notify card";
        emit adjusted(mIndex, LEFT, vol, stereo);
        kDebug() << k_funcinfo << "notify right";
        emit notifyRightVolume(right);
    }
    kDebug() << k_funcinfo << "leaving";
}

void MixerInput::on_rightVolume_valueChanged(int right) {
    kDebug() << k_funcinfo << "entering";

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
    kDebug() << k_funcinfo << "(volume, stereo) = (" << right << ", " << mUI->rightStereo->value() << ")"<< endl;
    if (!inSlotFlag) {
        int vol = 96 - right;
        int stereo = 96 - mUI->rightStereo->value();
        kDebug() << k_funcinfo << "notify card";
        emit adjusted(mIndex, RIGHT, vol, stereo);
        kDebug() << k_funcinfo << "notify left";
        emit notifyLeftVolume(left);
    }
    kDebug() << k_funcinfo << "leaving";
}

void MixerInput::on_leftStereo_valueChanged(int left) {
    kDebug() << k_funcinfo << "entering";

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
    kDebug() << k_funcinfo << "(volume, stereo) = (" << mUI->leftVolume->value() << ", " << left << ")"<< endl;
    if (!inSlotFlag) {
        int vol = 96 - mUI->leftVolume->value();
        int stereo = 96 - left;
        kDebug() << k_funcinfo << "notify card";
        emit adjusted(mIndex, LEFT, vol, stereo);
        kDebug() << k_funcinfo << "notify right";
        emit notifyRightStereo(right);
    }
    kDebug() << k_funcinfo << "leaving";
}

void MixerInput::on_rightStereo_valueChanged(int right) {
    kDebug() << k_funcinfo << "entering";

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
    kDebug() << k_funcinfo << "(volume, stereo) = (" << mUI->rightVolume->value() << ", " << right << ")"<< endl;
    if (!inSlotFlag) {
        int vol = 96 - mUI->rightVolume->value();
        int stereo = 96 - right;
        kDebug() << k_funcinfo << "notify card";
        emit adjusted(mIndex, RIGHT, vol, stereo);
        kDebug() << k_funcinfo << "notify left";
        emit notifyLeftStereo(left);
    }
    kDebug() << k_funcinfo << "leaving";
}

void MixerInput::on_checkLock_toggled(bool locked) {
    kDebug() << k_funcinfo << "entering";
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
    kDebug() << k_funcinfo << "leaving ";
}

#include "mixerinput.moc"
