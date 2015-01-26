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
#ifndef _TABVIEW_H_INCLUDED_
#define _TABVIEW_H_INCLUDED_

#include "tabview.h"
#include "envystructs.h"

#include <vector>
#include <qmemarray.h>
#include <qptrlist.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

class EnvyCard;
class QTimer;
class QListBoxItem;


class TabViewImpl : public TabView {
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

    EnvyCard*       envyCard;
    QTimer*         mTimer;
    QListBoxItem*   currentProfileItem;
    bool mLevelsEnabled;
    typedef QMemArray<int> IndexList;
    IndexList mLevelIndices;
    int mUpdateInterval;
    QString mConfigRateLocking;
    QString mConfigRateReset;
    QString mConfigInternalClock;
    QString mConfigClockDefault;
    QString mConfigDeemphasis;
    QString mConfigIEC958;
    QStringList mConfigEnumVars;
    QPtrList<QStringList> mConfigEnums;
    QStringList mConfigBoolVars;
    QPtrList<QCheckBox> mConfigCheckers;
    QPtrList<QButtonGroup> mConfigGroups;
    int mIdxOfInternalSamples;

public:
    TabViewImpl(QWidget* parent = 0);
    ~TabViewImpl();

    void setLevelsEnabled(bool enabled);

public slots:
    void updateMeters();
    void enableLevels();
    void updateBoolConfig(const QString&, bool);
    void updateEnumConfig(const QString&, const QString&);

private:

    void setupTimer();
    void setupTabs();
    void connectToCard();
    void connectFromCard();
    void loadProfiles();

    virtual void masterClockChanged(int);
    virtual void deemphasisChanged(int);
    virtual void intRateChanged(int);
    virtual void digRateChanged(int);
    virtual void rateLockToggled(bool);
    virtual void idleResetToggled(bool);
    virtual void loadProfile(QListBoxItem*);
    virtual void updateProfile();
    virtual void deleteProfile();
    virtual void newProfile();

signals:
    void enumConfigChanged(const QString&, const QString&);
    void boolConfigChanged(const QString&, bool);
};

#endif // _TABVIEW_H_INCLUDED_
