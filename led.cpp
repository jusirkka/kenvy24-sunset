/* This file is part of the KDE libraries
    Copyright (C) 1998 Jörg Habenicht (j.habenicht@europemail.com)
    Copyright (C) 2010 Christoph Feck <cfeck@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    Nicked from kwidgetsaddons & simplified.

*/

#include "led.h"

#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QStyle>
#include <QStyleOption>

Led::Led(QWidget *parent)
    : QWidget(parent),
      m_darkFactor(150),
      m_state(On)
{
    setColor(Qt::green);
}

Led::Led(const QColor &color, QWidget *parent)
    : QWidget(parent),
      m_darkFactor(150),
      m_state(On)
{
    setColor(color);
}

Led::Led(const QColor &color, State state, QWidget *parent)
    : QWidget(parent),
      m_darkFactor(150),
      m_state(state)
{
    setColor(color);
}

Led::~Led()
{
}

Led::State Led::state() const
{
    return m_state;
}


QColor Led::color() const
{
    return m_color;
}

void Led::setState(State state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
    updateCachedPixmap();
}

void Led::setColor(const QColor &color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    updateCachedPixmap();
}

void Led::setDarkFactor(int darkFactor)
{
    if (m_darkFactor == darkFactor) {
        return;
    }

    m_darkFactor = darkFactor;
    updateCachedPixmap();
}

int Led::darkFactor() const
{
    return m_darkFactor;
}


void Led::toggle()
{
    m_state = (m_state == On ? Off : On);
    updateCachedPixmap();
}

void Led::on()
{
    setState(On);
}

void Led::off()
{
    setState(Off);
}

void Led::resizeEvent(QResizeEvent *)
{
    updateCachedPixmap();
}

QSize Led::sizeHint() const
{
    QStyleOption option;
    option.initFrom(this);
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, &option, this);
    return QSize(iconSize,  iconSize);
}

QSize Led::minimumSizeHint() const
{
    return QSize(16, 16);
}

void Led::updateCachedPixmap()
{
    m_cachedPixmap[Off] = QPixmap();
    m_cachedPixmap[On] = QPixmap();
    update();
}

void Led::paintEvent(QPaintEvent *)
{
    if (!m_cachedPixmap[m_state].isNull()) {
        QPainter painter(this);
        painter.drawPixmap(1, 1, m_cachedPixmap[m_state]);
        return;
    }

    QSize size(width() - 2, height() - 2);
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    const int smallestSize = qMin(size.width(), size.height());
    QPainter painter;

    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);

    const QColor fillColor = m_state != Off ? m_color : m_color.dark(m_darkFactor);
    QColor borderColor = palette().color(QPalette::Dark);

    if (m_state == On) {
        QColor glowOverlay = fillColor;
        glowOverlay.setAlpha(80);

        // This isn't the fastest way, but should be "fast enough".
        // It's also the only safe way to use QPainter::CompositionMode
        QImage img(1, 1, QImage::Format_ARGB32_Premultiplied);
        QPainter p(&img);
        QColor start = borderColor;
        start.setAlpha(255); // opaque
        p.fillRect(0, 0, 1, 1, start);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.fillRect(0, 0, 1, 1, glowOverlay);
        p.end();

        borderColor = img.pixel(0, 0);
    }

    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(fillColor));
    const QBrush penBrush = QBrush(borderColor);
    const qreal penWidth = smallestSize / 8.0;
    painter.setPen(QPen(penBrush, penWidth));
    QRectF r(penWidth / 2.0, penWidth / 2.0, size.width() - penWidth, size.height() - penWidth);
    painter.drawRect(r);
    painter.end();

    m_cachedPixmap[m_state] = QPixmap::fromImage(image);
    painter.begin(this);
    painter.drawPixmap(1, 1, m_cachedPixmap[m_state]);
    painter.end();
}

