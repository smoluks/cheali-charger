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
#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "Keyboard.h"
#include "SMPS.h"
#include "Discharger.h"
#include "Timer1.h"
#include "Time.h"
#include "Buzzer.h"

#include STRINGS_HEADER

namespace hardware
{
    void do100msInterrupt();
    uint8_t getKeyPressed();
    void preInitialize();
    void initialize();

    void enableFan();
    void disableFan();

    void disableBacklight();
    bool enableBacklight();
    void setBacklightParams(uint8_t val, int8_t time);

    void enableChargerOutput();
    void disableChargerOutput();
    
    void enableDischargerOutput();
    void disableDischargerOutput();

    void setBatteryOutput(bool enable);

    void enableBalancerOutput();
    void disableBalancerOutput();
    void setBalancer(uint8_t bitmask);
}

#endif /* HARDWARE_H_ */
