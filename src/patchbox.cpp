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

#include <QButtonGroup>
#include <kconfig.h>
#include <kdebug.h>

PatchBox::PatchBox(QWidget* parent):
    QWidget(parent),
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


void PatchBox::setup(int index, const QString& name, const QString& title) {
    mIndex = index;
    mUI->patchGroup->setTitle(title);
    setObjectName(name);
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
        
void PatchBox::saveToConfig(KConfigBase* config) {
    KConfigGroup routing(config, QString("%1-%2").arg(objectName()).arg(mIndex));
    routing.writeEntry("locked", mUI->checkLock->isChecked());
    routing.writeEntry(QString("left-route"), mLGroup->checkedId());
    routing.writeEntry(QString("right-route"), mRGroup->checkedId());
}

void PatchBox::loadFromConfig(KConfigBase* config) {
    kDebug() << k_funcinfo << "entering";

    KConfigGroup routing(config, QString("%1-%2").arg(objectName()).arg(mIndex));

    int id = routing.readEntry("left-route", mLSources[R_SRC_MIXER]);
    mLGroup->button(id)->setChecked(true);
    leftPressed(id);

    id = routing.readEntry("right-route", mRSources[R_SRC_MIXER]);
    mRGroup->button(id)->setChecked(true);
    rightPressed(id);

    bool locked = routing.readEntry("locked", true);
    mUI->checkLock->setChecked(locked);

    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::updateRoute(int index, LeftRight channel, const QString& soundSource) {
    if (index != mIndex) return;

    kDebug() << k_funcinfo << "entering ";
    ExclusiveFlag inSlot(inSlotFlag);
    if (!inEventFlag) {
        if (channel == LEFT) {
            kDebug() << k_funcinfo << "set left " << soundSource;
            mLGroup->button(mLSources[soundSource])->setChecked(true);
        } else {
            kDebug() << k_funcinfo << "set right " << soundSource;
            mRGroup->button(mRSources[soundSource])->setChecked(true);
        }
    }
    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::leftPressed(int btn) {
    kDebug() << k_funcinfo << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        const QString& soundSource = mLSources.key(btn);
        kDebug() << k_funcinfo << "notify card " << soundSource;
        emit routeChanged(mIndex, LEFT, soundSource);
    }
    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::rightPressed(int btn) {
    kDebug() << k_funcinfo << "entering";
    ExclusiveFlag inEvent(inEventFlag);
    if (!inSlotFlag) {
        const QString& soundSource = mRSources.key(btn);
        kDebug() << k_funcinfo << "notify card " << soundSource;
        emit routeChanged(mIndex, RIGHT, soundSource);
    }
    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::leftNotified(int btn) {
    kDebug() << k_funcinfo << "entering";
    QString searchTerm = mRSources.key(btn);
    if (searchTerm == QString(R_SRC_DIGITAL_L)) {
        searchTerm = QString(R_SRC_DIGITAL_R);
    } else if (searchTerm == QString(R_SRC_DIGITAL_R)) {
        searchTerm = QString(R_SRC_DIGITAL_L);
    }
    kDebug() << k_funcinfo << "set selection " << searchTerm;
    mLGroup->button(mLSources[searchTerm])->setChecked(true);

    ExclusiveFlag inEvent(inEventFlag);
    kDebug() << k_funcinfo << "notify card " <<  searchTerm;
    emit routeChanged(mIndex, LEFT, searchTerm);

    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::rightNotified(int btn) {
    kDebug() << k_funcinfo << "entering";
    QString searchTerm = mLSources.key(btn);
    if (searchTerm == QString(R_SRC_DIGITAL_L)) {
        searchTerm = QString(R_SRC_DIGITAL_R);
    } else if (searchTerm == QString(R_SRC_DIGITAL_R)) {
        searchTerm = QString(R_SRC_DIGITAL_L);
    }
    kDebug() << k_funcinfo << "set selection " << searchTerm;
    mRGroup->button(mRSources[searchTerm])->setChecked(true);

    ExclusiveFlag inEvent(inEventFlag);
    kDebug() << k_funcinfo << "notify card " <<  searchTerm;
    emit routeChanged(mIndex, RIGHT, searchTerm);

    kDebug() << k_funcinfo << "leaving";
}

void PatchBox::lockToggled(bool locked) {
    kDebug() << k_funcinfo << "entering";
    if (locked) {
        connect(mLGroup, SIGNAL(buttonClicked(int)), this, SLOT(rightNotified(int)));
        connect(mRGroup, SIGNAL(buttonClicked(int)), this, SLOT(leftNotified(int)));
    } else {
        disconnect(mLGroup, SIGNAL(buttonClicked(int)), this, SLOT(rightNotified(int)));
        disconnect(mRGroup, SIGNAL(buttonClicked(int)), this, SLOT(leftNotified(int)));
    }
    kDebug() << k_funcinfo << "leaving";
}





#include "patchbox.moc"
