
#include <interrupt.h>

namespace Timer3
{
    void init();
    ISR(TIMER3_COMP_vect);
    uint32_t getMiliseconds();
}