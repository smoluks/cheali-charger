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
#include "Timer0.h"
#include "Hardware.h"
#include "IO.h"
#include "Settings.h"

//todo: speaker with internal buzzer

#include <avr/interrupt.h>
// Timer0 used to generate sound

#if F_CPU == 16000000 || F_CPU == 20000000
    // clk/256 (From prescaler)
    #define TCCR0_START_VALUE (1 << WGM00) | (1 << WGM01) | (1 << CS02);
#else    
    //| (1<<CS00) | (1<<CS01); clk/64
    #error "F_CPU not implemented"
#endif

namespace
{
    volatile uint8_t sound_TCNT0 = 0;

    void start_timer()
    {
        // Fast PWM        
        TCCR0 = TCCR0_START_VALUE;
    }

    void stop_timer()
    {
        TCCR0 = 0;
        TCNT0 = 0;
    }
}

ISR(TIMER0_COMP_vect)
{
    IO::setIO(BUZZER_PORT);
}

ISR(TIMER0_OVF_vect)
{
    IO::resetIO(BUZZER_PORT);
    TCNT0 = sound_TCNT0;
}

void Timer0::setBuzzer(uint8_t val)
{
    if (val == 0)
    {
        stop_timer();
        IO::resetIO(BUZZER_PORT);
    }
    else
    {
        val >>= 1;
        sound_TCNT0 = 120 + val;
        OCR0 = (120 + val + 256) / 2;
        start_timer();
    }
}

void Timer0::initialize()
{
    IO::setIO(BUZZER_DDR);
    TIMSK |= (1 << OCIE0) | (1 << TOIE0); // enable interrupts
}
