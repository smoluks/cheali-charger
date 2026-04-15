#ifndef TIMER_1_H_
#define TIMER_1_H_

#include "HardwareConfig.h"

#define TIMER1_PRECISION 5
#define TIMER1_PRECISION_PERIOD (TIMER1_PERIOD<<TIMER1_PRECISION)

namespace Timer1
{
    void setOCR();    
    void initialize();
    void setChargerValue(uint16_t value);
    void setDischargerValue(uint16_t value);
    void disablePWM();
};

#endif //TIMER_1_H_
