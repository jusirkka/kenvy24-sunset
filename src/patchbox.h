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

#include "envystructs.h"
#include <QWidget>

class EnvyCard;
class KConfig;

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

    int mIndex;
    QStringList mLSources;
    QStringList mRSources;

public:
    PatchBox(QWidget* parent);
    ~PatchBox();

    void setup(int index);
    void connectToCard(EnvyCard* envyCard, const QString& outputType = QString("analog"));
    void connectFromCard(EnvyCard* envyCard, const QString& outputType = QString("analog"));
        
    void saveToConfig(KConfig*);
    void loadFromConfig(KConfig*);

public slots:

    void updateRoute(int index, LeftRight channel, const QString& soundSource);

protected:

    virtual void leftPressed(int);
    virtual void rightPressed(int);
    virtual void lockToggled(bool);

private slots:

    void leftNotified(int);
    void rightNotified(int);

signals:

    void routeChanged(int index, LeftRight channel, const QString& soundSource);

};

#endif // _PATCHBOX_H_INCLUDED_
