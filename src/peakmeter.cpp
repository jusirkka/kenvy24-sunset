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

#include "ui_peakmeter.h"
#include "peakmeter.h"

#define L(x, y, z) mLeds.insert(x, mUI->led_##x); mLeds.insert(y, mUI->led_##y); mLeds.insert(z, mUI->led_##z)

PeakMeter::PeakMeter(QWidget* parent) :
        QWidget(parent),
        mUI(new Ui::PeakMeter),
        mLevel(0),
        mDischargeRate(5),
        mDischargeStep(5) {

    mUI->setupUi(this);

    L(0, 1, 2);
    L(3, 4, 5);
    L(6, 7, 8);
    L(9, 10, 11);
    L(12, 13, 14);
    L(15, 16, 17);
    L(18, 19, 20);
}


PeakMeter::~PeakMeter() {
    delete mUI;
}


void PeakMeter::updatePeak(int peak) {
    int upd = (peak * mLeds.size() + 128) / 255;

    if (upd > mLevel) {
        mDischargeStep = mDischargeRate;
        for (int i = mLevel; i < upd; i++) mLeds[i]->on();
        mLevel = upd;
    }

    if (mLevel > 0) {
        if (mDischargeStep == 0) {
            mLeds[mLevel - 1]->off();
            mLevel--;
            mDischargeStep = mDischargeRate;
        } else {
            mDischargeStep--;
        }
    }
}


#include "peakmeter.moc"
