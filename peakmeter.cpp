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

#include <QVBoxLayout>

#include "peakmeter.h"
#include "led.h"

PeakMeter::PeakMeter(QWidget* parent)
    : QWidget(parent),
      mLevel(0),
      mDischargeRate(5),
      mDischargeStep(5)
{
    QVBoxLayout* leds = new QVBoxLayout;
    leds->setDirection(QBoxLayout::BottomToTop);
    leds->setSpacing(0);
    leds->setContentsMargins(0, 0, 0, 0);
    for (int i = 0; i < 14; i++) {
        Led* green = new Led(Qt::green, Led::Off);
        mLeds.append(green);
        leds->addWidget(green);
    }
    for (int i = 0; i < 5; i++) {
        Led* yellow = new Led(Qt::yellow, Led::Off);
        mLeds.append(yellow);
        leds->addWidget(yellow);
    }
    for (int i = 0; i < 2; i++) {
        Led* red = new Led(Qt::red, Led::Off);
        mLeds.append(red);
        leds->addWidget(red);
    }

    setLayout(leds);

}


PeakMeter::~PeakMeter() {
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

