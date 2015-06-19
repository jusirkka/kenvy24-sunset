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
#include "envycard.h"
#include "envystructs.h"

#include <qslider.h>
#include <qcheckbox.h>
#include <qdial.h>
#include <kconfig.h>
#include <kdebug.h>

MixerInput::MixerInput(QWidget* parent) : QWidget(parent) {}

void MixerInput::setup(int index) {
    mIndex = index;
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
    leftMeter->updatePeak(level.left);
    rightMeter->updatePeak(level.right);
}

void MixerInput::saveToConfig(KConfig* config) {
    QString keyBase = name();
    config->writeEntry(QString("%1-locked").arg(keyBase), checkLock->isChecked());

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(LEFT);
    config->writeEntry(QString("%1-vol").arg(keyBase), leftVolume->value());
    config->writeEntry(QString("%1-stereo").arg(keyBase), leftStereo->value());
    config->writeEntry(QString("%1-mute").arg(keyBase), checkMuteLeft->isChecked());

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(RIGHT);
    config->writeEntry(QString("%1-vol").arg(keyBase), rightVolume->value());
    config->writeEntry(QString("%1-stereo").arg(keyBase), rightStereo->value());
    config->writeEntry(QString("%1-mute").arg(keyBase), checkMuteRight->isChecked());
}

void MixerInput::loadFromConfig(KConfig* config) {
    kdDebug() << k_funcinfo << "entering " << endl;

    QString keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(LEFT);
    int val = config->readNumEntry(QString("%1-stereo").arg(keyBase));
    leftStereo->setValue(val);
    leftStereoChanged(val);
    val = config->readNumEntry(QString("%1-vol").arg(keyBase));
    leftVolume->setValue(val);
    leftVolumeChanged(val);

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(RIGHT);
    val = config->readNumEntry(QString("%1-stereo").arg(keyBase));
    rightStereo->setValue(val);
    rightStereoChanged(val);
    val = config->readNumEntry(QString("%1-vol").arg(keyBase));
    rightVolume->setValue(val);
    rightVolumeChanged(val);

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(LEFT);
    checkMuteLeft->setChecked(config->readBoolEntry(QString("%1-mute").arg(keyBase)));

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(RIGHT);
    checkMuteRight->setChecked(config->readBoolEntry(QString("%1-mute").arg(keyBase)));

    keyBase = name();
    checkLock->setChecked(config->readBoolEntry(QString("%1-locked").arg(keyBase)));

    kdDebug() << k_funcinfo << "leaving " << endl;
}


void MixerInput::mixerUpdateMuteSwitch(int index, LeftRight channel, bool muted) {

    if (index != mIndex) return;

    kdDebug() << k_funcinfo << "entering " << endl;
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        if (channel == LEFT) {
            kdDebug() << k_funcinfo << "set left" << endl;
            checkMuteLeft->setChecked(muted);
        } else {
            kdDebug() << k_funcinfo << "set right" << endl;
            checkMuteRight->setChecked(muted);
        }
    }
    kdDebug() << k_funcinfo << "leaving" << endl;

}

void MixerInput::mixerUpdatePlaybackVolume(int index, LeftRight channel, MixerAdjustement adj) {

    if (index != mIndex) return;

    kdDebug() << k_funcinfo << "entering " << endl;
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        int dispVol = 96 - adj.volume;
        int dispStereo =  96 - adj.stereo;
        kdDebug() << k_funcinfo << "(volume, stereo) = (" << dispVol << ", " << dispStereo << ")"<< endl;
        if (channel == LEFT) {
            kdDebug() << k_funcinfo << "set left" << endl;
            leftStereo->setValue(dispStereo);
            leftVolume->setValue(dispVol);
        } else {
            kdDebug() << k_funcinfo << "set right" << endl;
            rightStereo->setValue(dispStereo);
            rightVolume->setValue(dispVol);
        }
    }
    kdDebug() << k_funcinfo << "leaving" << endl;

}

void MixerInput::leftMuteToggled(bool m) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kdDebug() << k_funcinfo << "notify card" << endl;
        emit muted(mIndex, LEFT, m);
        kdDebug() << k_funcinfo << "notify right" << endl;
        emit notifyRightMute(m);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MixerInput::rightMuteToggled(bool m) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kdDebug() << k_funcinfo << "notify card" << endl;
        emit muted(mIndex, RIGHT, m);
        kdDebug() << k_funcinfo << "notify left" << endl;
        emit notifyLeftMute(m);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MixerInput::leftVolumeChanged(int dispVal) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    kdDebug() << k_funcinfo << "(volume, stereo) = (" << dispVal << ", " << leftStereo->value() << ")"<< endl;
    if (!inSlotFlag) {
        int vol = 96 - dispVal;
        int stereo = 96 - leftStereo->value();
        kdDebug() << k_funcinfo << "notify card" << endl;
        emit adjusted(mIndex, LEFT, vol, stereo);
        kdDebug() << k_funcinfo << "notify right" << endl;
        emit notifyRightVolume(dispVal);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MixerInput::rightVolumeChanged(int dispVal) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    kdDebug() << k_funcinfo << "(volume, stereo) = (" << dispVal << ", " << rightStereo->value() << ")"<< endl;
    if (!inSlotFlag) {
        int vol = 96 - dispVal;
        int stereo = 96 - rightStereo->value();
        kdDebug() << k_funcinfo << "notify card" << endl;
        emit adjusted(mIndex, RIGHT, vol, stereo);
        kdDebug() << k_funcinfo << "notify left" << endl;
        emit notifyLeftVolume(dispVal);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MixerInput::leftStereoChanged(int dispStereo) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    kdDebug() << k_funcinfo << "(volume, stereo) = (" << leftVolume->value() << ", " << dispStereo << ")"<< endl;
    if (!inSlotFlag) {
        int vol = 96 - leftVolume->value();
        int stereo = 96 - dispStereo;
        kdDebug() << k_funcinfo << "notify card" << endl;
        emit adjusted(mIndex, LEFT, vol, stereo);
        kdDebug() << k_funcinfo << "notify right" << endl;
        emit notifyRightStereo(dispStereo);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MixerInput::rightStereoChanged(int dispStereo) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    kdDebug() << k_funcinfo << "(volume, stereo) = (" << rightVolume->value() << ", " << dispStereo << ")"<< endl;
    if (!inSlotFlag) {
        int vol = 96 - rightVolume->value();
        int stereo = 96 - dispStereo;
        kdDebug() << k_funcinfo << "notify card" << endl;
        emit adjusted(mIndex, RIGHT, vol, stereo);
        kdDebug() << k_funcinfo << "notify left" << endl;
        emit notifyLeftStereo(dispStereo);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void MixerInput::lockToggled(bool locked) {
    kdDebug() << k_funcinfo << "entering" << endl;
    if (locked) {
        connect(this, SIGNAL(notifyLeftVolume(int)), leftVolume, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyRightVolume(int)), rightVolume, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyLeftStereo(int)), leftStereo, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyRightStereo(int)), rightStereo, SLOT(setValue(int)));
        connect(this, SIGNAL(notifyLeftMute(bool)), checkMuteLeft, SLOT(setChecked(bool)));
        connect(this, SIGNAL(notifyRightMute(bool)), checkMuteRight, SLOT(setChecked(bool)));
    } else {
        disconnect(this, SIGNAL(notifyLeftVolume(int)), leftVolume, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyRightVolume(int)), rightVolume, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyLeftStereo(int)), leftStereo, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyRightStereo(int)), rightStereo, SLOT(setValue(int)));
        disconnect(this, SIGNAL(notifyLeftMute(bool)), checkMuteLeft, SLOT(setChecked(bool)));
        disconnect(this, SIGNAL(notifyRightMute(bool)), checkMuteRight, SLOT(setChecked(bool)));
    }
    kdDebug() << k_funcinfo << "leaving " << endl;
}

#include "mixerinput.moc"
