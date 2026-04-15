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
#include "Monitor.h"
#include "Buzzer.h"
#include "Screen.h"
#include "SerialLog.h"
#include "AnalogInputsPrivate.h"
#include "atomic.h"
#include "Timer3.h"

// #define ENABLE_DEBUG
#include "debug.h"


// time measurement
namespace Time
{
    // do 100ms interrupts
    void doTimerSlowInterrupts()
    {
        AnalogInputs::do100msInterrupt();
        Monitor::do100msInterrupt();
        hardware::do100msInterrupt();
    }

    uint16_t getMilisecondsU16()
    {
        return Timer3::getMiliseconds();
    }

    uint16_t Time::getInterruptsU16()
    {
        return Timer3::getMiliseconds();
    }

    uint32_t Time::getSeconds()
    {
        return getMiliseconds() / 1000;
    }

    uint16_t Time::getSecondsU16()
    {
        return getSeconds();
    }

    uint16_t Time::getMinutesU16()
    {
        return getMiliseconds() / 60000;
    }

    void Time::delay(uint16_t ms)
    {
        uint16_t start = getMilisecondsU16();

        while (diffU16(start, getMilisecondsU16()) < ms)
        {
        };
    }

    // warning: this method runs stuff in background,
    // delay may take significantly longer than "ms"
    void Time::delayDoIdle(uint16_t ms)
    {
        uint16_t start = getMilisecondsU16();
        uint16_t delay;
        do
        {
            doIdle();
            delay = diffU16(start, getMilisecondsU16());
        } while (delay < ms);

        LogDebug("delayDoIdle ms:", ms, " delay:", delay);
    }

    void doIdle()
    {
        Monitor::doIdle();
        SerialLog::doIdle();
        Buzzer::doIdle();
        AnalogInputs::loop();
    }
}
