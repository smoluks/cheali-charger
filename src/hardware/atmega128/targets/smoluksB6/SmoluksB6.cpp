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

#include "SmoluksB6.h"
#include "AnalogInputsADC.h"
#include "IO.h"
#include "Timer0.h"
#include "LiquidCrystal.h"
#include "atomic.h"

#ifndef PINS_H_
#error pins not defined (include *pins.h header in your HardwareConfig.h)
#endif
#include <SMPS_PID.h>
#include <AnalogInputs.h>

namespace hardware
{
    bool isBacklightEnabled = false;
    uint16_t backlightCurrentTime = 0;
    uint16_t backlightTime = 255;

    void doSlowInterrupt()
    {
        // Backlight timeout
        if (isBacklightEnabled)
        {
            if (backlightCurrentTime >= backlightTime)
            {
                disableBacklight();
            }
            else
                backlightCurrentTime++;
        }
    }

    uint8_t getKeyPressed()
    {
        return (IO::readIO(BUTTON_STOP_PIN) ? 0 : BUTTON_STOP) |
               (IO::readIO(BUTTON_DEC_PIN) ? 0 : BUTTON_DEC) |
               (IO::readIO(BUTTON_INC_PIN) ? 0 : BUTTON_INC) |
               (IO::readIO(BUTTON_START_PIN) ? 0 : BUTTON_START);
    }

    // before enabling interrupts
    void initializePins()
    {
        // power
        IO::setIO(BATTERY_DISABLE_PORT);
        IO::setIO(BATTERY_DISABLE_DDR);

        IO::setIO(DISCHARGER_DISABLE_PORT);
        IO::setIO(DISCHARGER_DISABLE_DDR);

        IO::setIO(SMPS_DISABLE_PORT);
        IO::setIO(SMPS_DISABLE_DDR);

        IO::resetIO(SMPS_UP_PORT);
        IO::setIO(SMPS_UP_DDR);

        IO::resetIO(SMPS_DOWN_PORT);
        IO::setIO(SMPS_DOWN_DDR);

        // buttons
        IO::resetIO(BUTTON_STOP_DDR);
        IO::setIO(BUTTON_STOP_PORT);
        IO::resetIO(BUTTON_DEC_DDR);
        IO::setIO(BUTTON_DEC_PORT);
        IO::resetIO(BUTTON_INC_DDR);
        IO::setIO(BUTTON_INC_PORT);
        IO::resetIO(BUTTON_START_DDR);
        IO::setIO(BUTTON_START_PORT);

        // balancer
        IO::resetIO(BALANCER_CELL1_PORT);
        IO::setIO(BALANCER_CELL1_DDR);
        IO::resetIO(BALANCER_CELL2_PORT);
        IO::setIO(BALANCER_CELL2_DDR);
        IO::resetIO(BALANCER_CELL3_PORT);
        IO::setIO(BALANCER_CELL3_DDR);
        IO::resetIO(BALANCER_CELL4_PORT);
        IO::setIO(BALANCER_CELL4_DDR);
        IO::resetIO(BALANCER_CELL5_PORT);
        IO::setIO(BALANCER_CELL5_DDR);
        IO::resetIO(BALANCER_CELL6_PORT);
        IO::setIO(BALANCER_CELL6_DDR);

#if MAX_BALANCE_CELLS > 6
        IO::pinMode(BALANCER7_LOAD_PIN, OUTPUT);
        IO::pinMode(BALANCER8_LOAD_PIN, OUTPUT);
#endif
#ifdef ENABLE_BALANCER_PWR
        IO::pinMode(BALANCER_PWR_ENABLE_PIN, OUTPUT);
#endif

        //
        IO::resetIO(FAN_PORT);
        IO::setIO(FAN_DDR);

        IO::resetIO(BUZZER_PORT);
        IO::setIO(BUZZER_DDR);

        IO::setIO(BACKLIGHT_DDR);
        enableBacklight();
    }

    // after enabling interrupts
    void initialize()
    {
        LiquidCrystal::begin(LCD_COLUMNS, LCD_LINES);
        Timer0::initialize();
        Timer1::initialize();
        AnalogInputsADC::initialize();
        SMPS_PID::setVoutCutoff(MAX_CHARGE_V);

        //не коммитить
        enableChargerOutput();
    }

    //-----called from Monitor-----
    inline void enableFan()
    {
        IO::setIO(FAN_PORT);
    }

    inline void disableFan()
    {
        IO::resetIO(FAN_PORT);
    }

    //-----moved to Timer0-----
    /* void setBuzzer(bool enable)
    {
        IO::digitalWrite(FAN_PIN, enable);
    } */

    void disableBacklight()
    {
        // IO::resetIO(BACKLIGHT_PORT);
        isBacklightEnabled = false;
    }

    //-----called from here and Keyboard-----
    bool enableBacklight()
    {
        if (isBacklightEnabled)
        {
            backlightCurrentTime = 0;
            return false;
        }

        IO::setIO(BACKLIGHT_PORT);
        isBacklightEnabled = true;
        backlightCurrentTime = 0;
        return true;
    }

    //-----called from Settings-----
    void setBacklightParams(uint8_t value, int8_t time_in_seconds)
    {
        backlightTime = time_in_seconds * (1000000 / TIMER_INTERRUPT_PERIOD_MICROSECONDS / TIMER_SLOW_INTERRUPT_INTERVAL); // time_in_seconds * interrupt freq

        // if(val)
        // IO::setIO(BACKLIGHT_PORT);
        // else
        //     IO::resetIO(BACKLIGHT_PORT);

        // uint32_t v1,v2;
        // v1  = LCD_BACKLIGHT_MAX;
        // v1 *= val;
        // v2  = LCD_BACKLIGHT_MIN;
        // v2 *= 100 - val;
        // 1+=v2;
        // v1/=100;
        // Timer1::setPWM(BACKLIGHT_PIN, v1);
    }

    //-----called from SMPS-----
    inline void enableChargerOutput()
    {
        IO::setIO(DISCHARGER_DISABLE_PORT);

        SMPS_PID::disablePID();
        SMPS_PID::init(AnalogInputs::getRealValue(AnalogInputs::Vin), AnalogInputs::getRealValue(AnalogInputs::Vout_plus_pin));

        IO::resetIO(SMPS_DISABLE_PORT);
    }

    void disableChargerOutput()
    {
        IO::setIO(DISCHARGER_DISABLE_PORT);
        IO::setIO(SMPS_DISABLE_PORT);

        SMPS_PID::disablePID();
    }
    
    //-----called from Discharger-----
    void enableDischargerOutput()
    {   
        IO::setIO(SMPS_DISABLE_PORT);
        IO::resetIO(DISCHARGER_DISABLE_PORT);
    }

    void disableDischargerOutput()
    {
        IO::setIO(DISCHARGER_DISABLE_PORT);
    }

     //-----called from AnalogInputs-----
    void setBatteryOutput(bool enable)
    {
        if (enable)
            IO::resetIO(BATTERY_DISABLE_PORT);
        else
        {
            IO::setIO(BATTERY_DISABLE_PORT);
            IO::setIO(DISCHARGER_DISABLE_PORT);
            IO::setIO(SMPS_DISABLE_PORT);
        }

#ifdef ENABLE_BALANCER_PWR
        IO::digitalWrite(BALANCER_PWR_ENABLE_PIN, enable);
#endif
    }

    //-----called from Balancer-----
    void enableBalancerOutput()
    {
    }

    void disableBalancerOutput()
    {
    }

    void setBalancer(uint8_t bitmask)
    {
        (bitmask & 1) ? IO::setIO(BALANCER_CELL1_PORT) : IO::resetIO(BALANCER_CELL1_PORT);
        (bitmask & 2) ? IO::setIO(BALANCER_CELL2_PORT) : IO::resetIO(BALANCER_CELL2_PORT);
        (bitmask & 4) ? IO::setIO(BALANCER_CELL3_PORT) : IO::resetIO(BALANCER_CELL3_PORT);
        (bitmask & 8) ? IO::setIO(BALANCER_CELL4_PORT) : IO::resetIO(BALANCER_CELL4_PORT);
        (bitmask & 16) ? IO::setIO(BALANCER_CELL5_PORT) : IO::resetIO(BALANCER_CELL5_PORT);
        (bitmask & 32) ? IO::setIO(BALANCER_CELL6_PORT) : IO::resetIO(BALANCER_CELL6_PORT);

#if MAX_BALANCE_CELLS > 6
        IO::digitalWrite(BALANCER7_LOAD_PIN, bitmask & 64);
        IO::digitalWrite(BALANCER8_LOAD_PIN, bitmask & 128);
#endif
    }
}