#ifndef TIME_H_
#define TIME_H_

#include "Hardware.h"

#define SLOW_INTERRUPT_PERIOD_MILISECONDS ((long)TIMER_INTERRUPT_PERIOD_MICROSECONDS*TIMER_SLOW_INTERRUPT_INTERVAL/1000)

namespace Time {

    void doTimerSlowInterrupts();

    uint16_t getMilisecondsU16();

    uint32_t getMiliseconds();
    uint16_t getInterruptsU16();
    uint16_t getMilisecondsU16();
    uint16_t getSecondsU16();
    uint32_t getSeconds();
    uint16_t getMinutesU16();
    void delay(uint16_t ms);

    //warning: this method runs stuff in background,
    //delay may take significantly longer than "ms"
    void delayDoIdle(uint16_t ms);

    inline uint16_t diffU16(uint16_t start, uint16_t end) {
        return end - start;
    }

    //private
    void systickHandler();
};


#endif /* TIME_H_ */
