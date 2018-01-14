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
#include "settings.h"

#include <QButtonGroup>
#include <QDebug>

PatchBox::PatchBox(QWidget* parent):
    QWidget(parent),
    inSlotFlag(false),
    inEventFlag(false),
    mUI(new Ui::PatchBox),
    mLGroup(new QButtonGroup),
    mRGroup(new QButtonGroup)
{
    mUI->setupUi(this);


    mLSources[R_SRC_MIXER] = 0;
    mLGroup->addButton(mUI->mixer_l, 0);

    mLSources[R_SRC_ANALOG] = 1;
    mLGroup->addButton(mUI->analog_l, 1);

    mLSources[R_SRC_DIGITAL_L] = 2;
    mLGroup->addButton(mUI->digital_ll, 2);

    mLSources[R_SRC_DIGITAL_R] = 3;
    mLGroup->addButton(mUI->digital_lr, 3);

    mLSources[R_SRC_PCM] = 4;
    mLGroup->addButton(mUI->pcm_l, 4);

    mRSources[R_SRC_MIXER] = 0;
    mRGroup->addButton(mUI->mixer_r, 0);

    mRSources[R_SRC_ANALOG] = 1;
    mRGroup->addButton(mUI->analog_r, 1);

    mRSources[R_SRC_DIGITAL_L] = 2;
    mRGroup->addButton(mUI->digital_rl, 2);

    mRSources[R_SRC_DIGITAL_R] = 3;
    mRGroup->addButton(mUI->digital_rr, 3);

    mRSources[R_SRC_PCM] = 4;
    mRGroup->addButton(mUI->pcm_r, 4);

    connect(mLGroup, SIGNAL(buttonClicked(int)), this, SLOT(leftPressed(int)));
    connect(mRGroup, SIGNAL(buttonClicked(int)), this, SLOT(rightPressed(int)));
}


PatchBox::~PatchBox() {
    delete mLGroup;
    delete mRGroup;
    delete mUI;
}


void PatchBox::setup(int address, const QString& name, const QString& title, Routing& routing) {
    mAddress = address;
    routing[mAddress] = this;
    mUI->patchGroup->setTitle(title);
    setObjectName(name);
}

void PatchBox::connectToCard(EnvyCard* envyCard) {
    connect(this, SIGNAL(routeChanged(int, Position, const QString&)),
            envyCard, SLOT(on_slot_mixerRouteChanged(int, Position, const QString&)));
}

void PatchBox::connectFromCard(EnvyCard* envyCard) {
    connect(envyCard, SIGNAL(mixerRouteChanged(int,Position,const QString&)),
            SLOT(updateRoute(int, Position, const QString&)));
}

void PatchBox::saveToConfig(Settings& s) {
    s.beginGroup(QString("%1-%2").arg(objectName()).arg(mAddress));
    s.setValue("locked", mUI->checkLock->isChecked());
    s.setValue("left-route", mLGroup->checkedId());
    s.setValue("right-route", mRGroup->checkedId());
    s.endGroup();
}

void PatchBox::loadFromConfig(Settings& s) {

    s.beginGroup(QString("%1-%2").arg(objectName()).arg(mAddress));

    int id = s.get("left-route", mLSources[R_SRC_MIXER]);
    mLGroup->button(id)->setChecked(true);
    leftPressed(id);

    id = s.get("right-route", mRSources[R_SRC_MIXER]);
    mRGroup->button(id)->setChecked(true);
    rightPressed(id);

    bool locked = s.value("locked", true).toBool();
    mUI->checkLock->setChecked(locked);

    s.endGroup();
}

void PatchBox::updateRoute(int address, Position channel, const QString& soundSource) {
    if (address != mAddress) return;

    qDebug()  << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        if (channel == Position::Left) {
            qDebug()  << "set left " << soundSource;
            mLGroup->button(mLSources[soundSource])->setChecked(true);
        } else {
            qDebug()  << "set right " << soundSource;
            mRGroup->button(mRSources[soundSource])->setChecked(true);
        }
    }
    qDebug()  << "leaving";
}

void PatchBox::leftPressed(int btn) {
    qDebug()  << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        const QString& soundSource = mLSources.key(btn);
        qDebug()  << "notify card " << soundSource;
        emit routeChanged(mAddress, Position::Left, soundSource);
    }
    qDebug()  << "leaving";
}

void PatchBox::rightPressed(int btn) {
    qDebug()  << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        const QString& soundSource = mRSources.key(btn);
        qDebug()  << "notify card " << soundSource;
        emit routeChanged(mAddress, Position::Right, soundSource);
    }
    qDebug()  << "leaving";
}

void PatchBox::leftNotified(int btn) {
    qDebug()  << "entering";
    QString searchTerm = mRSources.key(btn);
    if (searchTerm == QString(R_SRC_DIGITAL_L)) {
        searchTerm = QString(R_SRC_DIGITAL_R);
    } else if (searchTerm == QString(R_SRC_DIGITAL_R)) {
        searchTerm = QString(R_SRC_DIGITAL_L);
    }
    qDebug()  << "set selection " << searchTerm;
    mLGroup->button(mLSources[searchTerm])->setChecked(true);

    ExclusiveFlag inEvent(inEventFlag);
    qDebug()  << "notify card " <<  searchTerm;
    emit routeChanged(mAddress, Position::Left, searchTerm);

    qDebug()  << "leaving";
}

void PatchBox::rightNotified(int btn) {
    qDebug()  << "entering";
    QString searchTerm = mLSources.key(btn);
    if (searchTerm == QString(R_SRC_DIGITAL_L)) {
        searchTerm = QString(R_SRC_DIGITAL_R);
    } else if (searchTerm == QString(R_SRC_DIGITAL_R)) {
        searchTerm = QString(R_SRC_DIGITAL_L);
    }
    qDebug()  << "set selection " << searchTerm;
    mRGroup->button(mRSources[searchTerm])->setChecked(true);

    ExclusiveFlag inEvent(inEventFlag);
    qDebug()  << "notify card " <<  searchTerm;
    emit routeChanged(mAddress, Position::Right, searchTerm);

    qDebug()  << "leaving";
}

void PatchBox::lockToggled(bool locked) {
    qDebug()  << "entering";
    if (locked) {
        connect(mLGroup, SIGNAL(buttonClicked(int)), this, SLOT(rightNotified(int)));
        connect(mRGroup, SIGNAL(buttonClicked(int)), this, SLOT(leftNotified(int)));
    } else {
        disconnect(mLGroup, SIGNAL(buttonClicked(int)), this, SLOT(rightNotified(int)));
        disconnect(mRGroup, SIGNAL(buttonClicked(int)), this, SLOT(leftNotified(int)));
    }
    qDebug()  << "leaving";
}

