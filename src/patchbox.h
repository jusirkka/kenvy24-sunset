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
#ifndef _PATCHBOX_H_INCLUDED_
#define _PATCHBOX_H_INCLUDED_

#include "envycard.h"
#include <QWidget>
#include <QRadioButton>
#include <ksharedconfig.h>

class KConfig;
class QButtonGroup;

namespace Ui {
class PatchBox;
}

class PatchBox : public QWidget {
    Q_OBJECT

public:

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

    Ui::PatchBox* mUI;
    int mAddress;
    QHash<QString, int> mLSources;
    QHash<QString, int> mRSources;
    QButtonGroup* mLGroup;
    QButtonGroup* mRGroup;

public:
    PatchBox(QWidget* parent);
    ~PatchBox();

    typedef QMap<int, QWidget*> Routing;

    void setup(int address, const QString& name, const QString& title, Routing& routing);
    void connectToCard(EnvyCard* envyCard);
    void connectFromCard(EnvyCard* envyCard);

    void saveToConfig(KConfigBase*);
    void loadFromConfig(KConfigBase*);

public slots:

    void updateRoute(int index, LeftRight channel, const QString& soundSource);

private slots:

    void leftNotified(int);
    void rightNotified(int);
    void leftPressed(int);
    void rightPressed(int);
    void lockToggled(bool);

signals:

    void routeChanged(int index, LeftRight channel, const QString& soundSource);

};

#endif // _PATCHBOX_H_INCLUDED_
