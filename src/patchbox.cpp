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

#include "ui_patchbox.h"
#include "patchbox.h"
#include "envycard.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <kconfig.h>
#include <kdebug.h>

PatchBox::PatchBox(QWidget* parent): QWidget(parent), mUI(new Ui::PatchBox)
{
    mUI->setupUi(this);


    mLSources[R_SRC_MIXER] = mUI->mixer_l;
    mLSources[QString(R_SRC_ANALOG)] = mUI->analog_l;
    mLSources[R_SRC_DIGITAL_L] = mUI->digital_ll;
    mLSources[R_SRC_DIGITAL_R] = mUI->digital_lr;
    mLSources[R_SRC_PCM] = mUI->pcm_l;

    mRSources[R_SRC_MIXER] = mUI->mixer_r;
    mRSources[R_SRC_ANALOG] = mUI->analog_r;
    mRSources[R_SRC_DIGITAL_L] = mUI->digital_rl;
    mRSources[R_SRC_DIGITAL_R] = mUI->digital_rr;
    mRSources[R_SRC_PCM] = mUI->pcm_r;
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
        
void PatchBox::saveToConfig(KSharedConfigPtr config) {
    QString keyBase = name();
    config->writeEntry(QString("%1-locked").arg(keyBase), checkLock->isChecked());

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(LEFT);
    config->writeEntry(QString("%1-route").arg(keyBase), leftSelection->selectedId());

    keyBase = QString("%1-%2-%3").arg(name()).arg(mIndex).arg(RIGHT);
    config->writeEntry(QString("%1-route").arg(keyBase), rightSelection->selectedId());
}

void PatchBox::loadFromConfig(KSharedConfigPtr* config) {
    kDebug() << k_funcinfo << "entering";

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

    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::updateRoute(int index, LeftRight channel, const QString& soundSource) {
    if (index != mIndex) return;

    kDebug() << k_funcinfo << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        if (channel == LEFT) {
            kDebug() << k_funcinfo << "set left " << soundSource;
            mLSources[soundSource]->setChecked(true);
        } else {
            kDebug() << k_funcinfo << "set right " << soundSource;
            mRSources[soundSource]->setChecked(true);
        }
    }
    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::leftPressed(int btn) {
    kDebug() << k_funcinfo << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kDebug() << k_funcinfo << "notify card " << mLSources[btn];
        emit routeChanged(mIndex, LEFT, mLSources[btn]);
    }
    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::rightPressed(int btn) {
    kDebug() << k_funcinfo << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        kDebug() << k_funcinfo << "notify card " << mRSources[btn];
        emit routeChanged(mIndex, RIGHT, mRSources[btn]);
    }
    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::leftNotified(int btn) {
    kDebug() << k_funcinfo << "entering";
    QString searchTerm = mRSources[btn];
    if (searchTerm == QString(R_SRC_DIGITAL_L)) {
        searchTerm = QString(R_SRC_DIGITAL_R);
    } else if (searchTerm == QString(R_SRC_DIGITAL_R)) {
        searchTerm = QString(R_SRC_DIGITAL_L);
    }
    kDebug() << k_funcinfo << "set selection " << searchTerm;
    leftSelection->setButton(mLSources.findIndex(searchTerm));

    ExclusiveFlag inEvent(inEventFlag);
    kDebug() << k_funcinfo << "notify card " <<  searchTerm;
    emit routeChanged(mIndex, LEFT, searchTerm);

    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::rightNotified(int btn) {
    kDebug() << k_funcinfo << "entering";
    QString searchTerm = mLSources[btn];
    if (searchTerm == QString(R_SRC_DIGITAL_L)) {
        searchTerm = QString(R_SRC_DIGITAL_R);
    } else if (searchTerm == QString(R_SRC_DIGITAL_R)) {
        searchTerm = QString(R_SRC_DIGITAL_L);
    }
    kDebug() << k_funcinfo << "set selection " << searchTerm;
    rightSelection->setButton(mRSources.findIndex(searchTerm));

    ExclusiveFlag inEvent(inEventFlag);
    kDebug() << k_funcinfo << "notify card " <<  searchTerm;
    emit routeChanged(mIndex, RIGHT, searchTerm);

    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::lockToggled(bool locked) {
    kDebug() << k_funcinfo << "entering";
    if (locked) {
        connect(leftSelection, SIGNAL(pressed(int)), this, SLOT(rightNotified(int)));
        connect(rightSelection, SIGNAL(pressed(int)), this, SLOT(leftNotified(int)));
    } else {
        disconnect(leftSelection, SIGNAL(pressed(int)), this, SLOT(rightNotified(int)));
        disconnect(rightSelection, SIGNAL(pressed(int)), this, SLOT(leftNotified(int)));
    }
    kDebug() << k_funcinfo << "leaving";
}





#include "patchbox.moc"
