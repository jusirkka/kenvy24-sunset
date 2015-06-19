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

#include "patchbox.h"
#include "envycard.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <kconfig.h>
#include <kdebug.h>

PatchBox::PatchBox(QWidget* parent): QWidget(parent)
{
    mLSources << "1" << "2" << "3" << "4" << "5";
    mRSources << "1" << "2" << "3" << "4" << "5";

    mLSources[leftSelection->id(mixer_l)] = QString(R_SRC_MIXER);
    mLSources[leftSelection->id(analog_l)] = QString(R_SRC_ANALOG);
    mLSources[leftSelection->id(digital_ll)] = QString(R_SRC_DIGITAL_L);
    mLSources[leftSelection->id(digital_lr)] = QString(R_SRC_DIGITAL_R);
    mLSources[leftSelection->id(pcm_l)] = QString(R_SRC_PCM);

    mRSources[rightSelection->id(mixer_r)] = QString(R_SRC_MIXER);
    mRSources[rightSelection->id(analog_r)] = QString(R_SRC_ANALOG);
    mRSources[rightSelection->id(digital_rl)] = QString(R_SRC_DIGITAL_L);
    mRSources[rightSelection->id(digital_rr)] = QString(R_SRC_DIGITAL_R);
    mRSources[rightSelection->id(pcm_r)] = QString(R_SRC_PCM);
}

PatchBox::~PatchBox() {}


void PatchBox::setup(int index) {
    mIndex = index;
}

void PatchBox::connectToCard(EnvyCard* envyCard, const QString& outputType) {
    if (outputType == "digital") {
        connect(this, SIGNAL(routeChanged(int, LeftRight, const QString&)),
               envyCard, SLOT(setDigitalRoute(int, LeftRight, const QString&)));
        return;
    }

    connect(this, SIGNAL(routeChanged(int, LeftRight, const QString&)),
           envyCard, SLOT(setAnalogRoute(int, LeftRight, const QString&)));
}

void PatchBox::connectFromCard(EnvyCard* envyCard, const QString& outputType) {
    if (outputType == "digital") {
        connect(envyCard, SIGNAL(digitalRouteUpdated(int, LeftRight, const QString&)),
                             SLOT(updateRoute(int, LeftRight, const QString&)));
        return;
    }

    connect(envyCard, SIGNAL(analogRouteUpdated(int, LeftRight, const QString&)),
                         SLOT(updateRoute(int, LeftRight, const QString&)));
}
        
void PatchBox::saveToConfig(KConfig* config) {
    QString keyBase = name();
    config->writeEntry(QString("%1-locked").arg(keyBase), checkLock->isChecked());

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(LEFT);
    config->writeEntry(QString("%1-route").arg(keyBase), leftSelection->selectedId());

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(RIGHT);
    config->writeEntry(QString("%1-route").arg(keyBase), rightSelection->selectedId());
}

void PatchBox::loadFromConfig(KConfig* config) {
    kdDebug() << k_funcinfo << "entering" << endl;

    QString keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(LEFT);
    int btn = config->readNumEntry(QString("%1-route").arg(keyBase));
    leftSelection->setButton(btn);
    leftPressed(btn);

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(RIGHT);
    btn = config->readNumEntry(QString("%1-route").arg(keyBase));
    rightSelection->setButton(btn);
    rightPressed(btn);

    keyBase = name();
    checkLock->setChecked(config->readBoolEntry(QString("%1-locked").arg(keyBase)));

    kdDebug() << k_funcinfo << "leaving" << endl;
}

void PatchBox::updateRoute(int index, LeftRight channel, const QString& soundSource) {
    if (index != mIndex) return;

    kdDebug() << k_funcinfo << "entering " << endl;
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        if (channel == LEFT) {
            kdDebug() << k_funcinfo << "set left " << soundSource << endl;
            leftSelection->setButton(mLSources.findIndex(soundSource));
        } else {
            kdDebug() << k_funcinfo << "set right " << soundSource << endl;
            rightSelection->setButton(mRSources.findIndex(soundSource));
        }
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void PatchBox::leftPressed(int btn) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kdDebug() << k_funcinfo << "notify card " << mLSources[btn] << endl;
        emit routeChanged(mIndex, LEFT, mLSources[btn]);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void PatchBox::rightPressed(int btn) {
    kdDebug() << k_funcinfo << "entering" << endl;
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kdDebug() << k_funcinfo << "notify card " << mRSources[btn] << endl;
        emit routeChanged(mIndex, RIGHT, mRSources[btn]);
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}

void PatchBox::leftNotified(int btn) {
    kdDebug() << k_funcinfo << "entering" << endl;
    QString searchTerm = mRSources[btn];
    if (searchTerm == QString(R_SRC_DIGITAL_L)) {
        searchTerm = QString(R_SRC_DIGITAL_R);
    } else if (searchTerm == QString(R_SRC_DIGITAL_R)) {
        searchTerm = QString(R_SRC_DIGITAL_L);
    }
    kdDebug() << k_funcinfo << "set selection " << searchTerm << endl;
    leftSelection->setButton(mLSources.findIndex(searchTerm));

    ExclusiveFlag inEvent(inEventFlag);
    kdDebug() << k_funcinfo << "notify card " <<  searchTerm << endl;
    emit routeChanged(mIndex, LEFT, searchTerm);

    kdDebug() << k_funcinfo << "leaving" << endl;
}

void PatchBox::rightNotified(int btn) {
    kdDebug() << k_funcinfo << "entering" << endl;
    QString searchTerm = mLSources[btn];
    if (searchTerm == QString(R_SRC_DIGITAL_L)) {
        searchTerm = QString(R_SRC_DIGITAL_R);
    } else if (searchTerm == QString(R_SRC_DIGITAL_R)) {
        searchTerm = QString(R_SRC_DIGITAL_L);
    }
    kdDebug() << k_funcinfo << "set selection " << searchTerm << endl;
    rightSelection->setButton(mRSources.findIndex(searchTerm));

    ExclusiveFlag inEvent(inEventFlag);
    kdDebug() << k_funcinfo << "notify card " <<  searchTerm << endl;
    emit routeChanged(mIndex, RIGHT, searchTerm);

    kdDebug() << k_funcinfo << "leaving" << endl;
}

void PatchBox::lockToggled(bool locked) {
    kdDebug() << k_funcinfo << "entering" << endl;
    if (locked) {
        connect(leftSelection, SIGNAL(pressed(int)), this, SLOT(rightNotified(int)));
        connect(rightSelection, SIGNAL(pressed(int)), this, SLOT(leftNotified(int)));
    } else {
        disconnect(leftSelection, SIGNAL(pressed(int)), this, SLOT(rightNotified(int)));
        disconnect(rightSelection, SIGNAL(pressed(int)), this, SLOT(leftNotified(int)));
    }
    kdDebug() << k_funcinfo << "leaving" << endl;
}





#include "patchbox.moc"
