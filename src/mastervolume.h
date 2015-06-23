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
#ifndef _MASTERVOLUME_H_INCLUDED_
#define _MASTERVOLUME_H_INCLUDED_

#include "envystructs.h"
#include <QWidget>
#include <ksharedconfig.h>

class EnvyCard;

namespace Ui {
class MasterVolume;
}

class MasterVolume : public QWidget {
    Q_OBJECT

private:
    bool inSlotFlag;
    bool inEventFlag;

    struct ExclusiveFlag {
        bool& slotFlag;
        ExclusiveFlag(bool& flag) : slotFlag(flag) {
            flag = true;
        }
        ~ExclusiveFlag() {
            slotFlag = false;
        }
    };


    Ui::MasterVolume *mUI;

public:


    MasterVolume(QWidget* parent);
    ~MasterVolume();

    void connectToCard(EnvyCard* envyCard);
    void connectFromCard(EnvyCard* envyCard);

    void saveToConfig(KSharedConfigPtr);
    void loadFromConfig(KSharedConfigPtr);

    void updatePeaks(StereoLevels level);

public slots:

    void analogUpdateDACVolume(LeftRight, int);

    void lockToggled(bool);
    void leftVolumeChanged(int);
    void rightVolumeChanged(int);

signals:

    void adjusted(LeftRight channel, int volume);
    void notifyRightVolume(int);
    void notifyLeftVolume(int);
};


#endif // _MASTERVOLUME_H_INCLUDED_
