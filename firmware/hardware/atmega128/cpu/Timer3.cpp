// Timer3 - systick timer

#include "Time.h"
#include "Timer3.h"
#include "Hardware.h"
#include "atomic.h"
#include <AnalogInputsPrivate.h>
#include <Monitor.h>

namespace Timer3
{
    void init()
    {
        TCCR3A = 0;
        TCCR3B = (1 << WGM32) | (1 << CS31); // Compare Match (CTC) Mode, clk/8 (From prescaler) - 2500000
        TCNT3 = 0;
        OCR3A = (F_CPU / 256 / 1000) - 1;

        ETIMSK |= (1 << OCIE3A); // OCIE3A: Timer/Counter2 Output Compare Match Interrupt Enable
    }

    volatile uint32_t _systimeInMS = 0;
    volatile uint8_t slowInterval = 100;
    ISR(TIMER3_COMP_vect)
    {
        _systimeInMS++;
        if (--slowInterval == 0)
        {
            Time::doTimerSlowInterrupts();
            slowInterval = 100;
        }
    }

    // return milliseconds elapsed from program start
    uint32_t getMiliseconds()
    {
        uint32_t v;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) // TODO: disable timer3 int only?
        {
            v = _systimeInMS;
        }

        return v;
    }
}