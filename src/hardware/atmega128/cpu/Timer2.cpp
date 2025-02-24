/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013  Paweł Stawicki. All right reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Time.h"
#include "Hardware.h"
#include "atomic.h"

// time measurement - It uses atmega32/Timer2 to measure TIMER_INTERRUPT_PERIOD_MICROSECONDS

ISR(TIMER2_COMP_vect)
{
    Time::callback();
}

void Time::initialize()
{
    TCCR2 = (1 << WGM21); // Clear Timer on Compare Match (CTC) Mode
    TCNT2 = 0;

#if F_CPU == 16000000 || F_CPU == 20000000
#if F_CPU % (256 * TIMER_INTERRUPT_PERIOD_MICROSECONDS) != 0
#error "F_CPU not divisible to prescaler values"
#endif
#if F_CPU / (256 * TIMER_INTERRUPT_PERIOD_MICROSECONDS) < 1
#error "prescaler values too high"
#endif
#if F_CPU / (256 * TIMER_INTERRUPT_PERIOD_MICROSECONDS) > 255
#error "prescaler values too low"
#endif

    TCCR2 |= (1 << CS22); // clk/256 (From prescaler)
    OCR2 = (F_CPU / 256 / TIMER_INTERRUPT_PERIOD_MICROSECONDS) - 1;

#else
#error "F_CPU not supported"
#endif

    TIMSK |= (1 << OCIE2); // OCIE2: Timer/Counter2 Output Compare Match Interrupt Enable
}
