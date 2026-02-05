#include <avr/interrupt.h>

#include "Timer1.h"
#include "Hardware.h"
#include "atomic.h"
#include "IO.h"

ISR(TIMER1_OVF_vect)
{
    Timer1::setOCR(); // modulate the PWM
}

namespace Timer1
{
    volatile uint16_t Timer1_value = 0;
    volatile uint16_t Timer1_sum = 0;
    volatile uint16_t Timer1_ch = 0;

    void setOCR()
    {
        // modulate the PWM - we modulate the PWM signal to get more precision.
        //(the PWM frequency stays at about 31kHz)
        Timer1_sum += Timer1_value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (Timer1_ch == 1)
                OCR1A = (Timer1_sum >> TIMER1_PRECISION);
            else if (Timer1_ch == 2)
                OCR1B = (Timer1_sum >> TIMER1_PRECISION);
        }
        Timer1_sum &= (1 << TIMER1_PRECISION) - 1;
    }

    void initialize()
    {
        // top value for PWM
        ICR1 = TIMER1_PERIOD;       
    }

    void setChargerValue(uint16_t value)
    {
        if (value <= TIMER1_PRECISION_PERIOD)
        {
            Timer1_value = value;
            Timer1_ch = 1;
            OCR1A = 0;
            OCR1B = 500;
        }
        else
        {
            Timer1_value = value - TIMER1_PRECISION_PERIOD;
            Timer1_ch = 2;
            OCR1B = 0;
            OCR1A = 500;
        }

        // Clear OCnA/OCnB/OCnC on compare match, set OCnA/OCnB/OCnC at BOTTOM
        TCCR1A = _BV(WGM11) | _BV(COM1A1) | _BV(COM1B1);
        // Set mode 14: Fast PWM, no prescaling
        TCCR1B = _BV(WGM13) | _BV(WGM12)| _BV(CS10);
        //
        TIMSK |= _BV(TOIE1);
    }

    void setDischargerValue(uint16_t value)
    {
        Timer1_value = value;
        Timer1_ch = 2;
        OCR1A = 0;
        TCCR1A = _BV(WGM11) | _BV(COM1B1);
        TCCR1B = _BV(WGM13) | _BV(WGM12)| _BV(CS10);
        TIMSK |= _BV(TOIE1);
    }

    void disablePWM()
    {
        TIMSK &= ~_BV(TOIE1);
        TCCR1B = 0;
        TCCR1A = 0;
    }
}
