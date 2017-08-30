/* This file is part of the KDE libraries
    Copyright (C) 1998 Jörg Habenicht (j.habenicht@europemail.com)

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

#ifndef LED_H
#define LED_H

#include <QWidget>

class QColor;

/**
 * @class KLed kled.h KLed
 *
 * @short An LED widget.
 *
 * Displays a round or rectangular light emitting diode.
 *
 * It is configurable to arbitrary colors, the two on/off states and
 *
 * It displays itself in a performant flat view.
 *
 *
 * @author Joerg Habenicht, Richard J. Moore (rich@kde.org) 1998, 1999
 */
class Led : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(int darkFactor READ darkFactor WRITE setDarkFactor)

public:

    /**
     * Status of the light is on/off.
     * @short LED on/off.
     */
    enum State { Off, On };
    Q_ENUM(State)


    /**
     * Constructs a green, round LED widget which will initially
     * be turned on.
     *
     * @param parent The parent widget.
     */
    explicit Led(QWidget *parent = nullptr);

    /**
     * Constructs a round LED widget with the supplied color which will
     * initially be turned on.
     *
     * @param color Initial color of the LED.
     * @param parent The parent widget.
     * @short Constructor
     */
    explicit Led(const QColor &color, QWidget *parent = nullptr);

    explicit Led(const QColor &color, State state, QWidget *parent = nullptr);

    /**
     * Destroys the LED widget.
     * @short Destructor
     */
    ~Led();

    /**
     * Returns the current color of the widget.
     *
     * @see Color
     * @short Returns LED color.
     */
    QColor color() const;

    /**
     * Returns the current state of the widget (on/off).
     *
     * @see State
     * @short Returns LED state.
     */
    State state() const;


    /**
     * Returns the factor to darken the LED.
     *
     * @see setDarkFactor()
     * @short Returns dark factor.
     */
    int darkFactor() const;

    /**
     * Set the color of the widget.
     *
     * The LED is shown with Color when in the KLed::On state
     * or with the darken Color (@see setDarkFactor) in KLed::Off
     * state.
     *
     * The widget calls the update() method, so it will
     * be updated when entering the main event loop.
     *
     * @see Color
     *
     * @param color New color of the LED.
     * @short Sets the LED color.
     */
    void setColor(const QColor &color);

    /**
     * Sets the state of the widget to On or Off.
     *
     * @see on() off() toggle()
     *
     * @param state The LED state: on or off.
     * @short Set LED state.
     */
    void setState(State state);


    /**
     * Sets the factor to darken the LED in KLed::Off state.
     *
     * The @p darkFactor should be greater than 100, otherwise the LED
     * becomes lighter in KLed::Off state.
     *
     * Defaults to 150.
     *
     * @see QColor
     *
     * @param darkFactor Sets the factor to darken the LED.
     * @short Sets the factor to darken the LED.
     */
    void setDarkFactor(int darkFactor);

    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

public Q_SLOTS:

    /**
     * Toggles the state of the led from Off to On or vice versa.
     */
    void toggle();

    /**
     * Sets the state of the widget to On.
     *
     * @see off() toggle()  setState()
     */
    void on();

    /**
     * Sets the state of the widget to Off.
     *
     * @see on() toggle()  setState()
     */
    void off();

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

    /**
     * @internal
     * invalidates caches after property changes and calls update()
     */
    void updateCachedPixmap();

private:

    int m_darkFactor;
    QColor m_color;
    State m_state;

    QPixmap m_cachedPixmap[2]; // for both states


};

#endif
