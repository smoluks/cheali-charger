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
#ifndef HARDWARE_CONFIG_H_
#define HARDWARE_CONFIG_H_

#include "GlobalConfig.h"
#include "SmoluksB6-pins.h"

#define F_CPU 20000000UL

#define ENABLE_HELPER
#define ENABLE_HELPER_ANALOG_INPUTS_ANALYZER
//#define ENABLE_HELPER_BALANCE_PORT_ANALYZER
//#define ENABLE_HELPER_LCD_ANALYZER
//#define ENABLE_HELPER_ADC_KEYBOARD_ANALYZER

#define LCD_BACKLIGHT_MIN       100
#define LCD_BACKLIGHT_MAX       32000

#define MAX_BALANCE_CELLS       6
#define TIMER1_PERIOD           512

#define CALIBRATION_CHARGE_POINT0_mA    100
#define CALIBRATION_CHARGE_POINT1_mA    1000
#define CALIBRATION_DISCHARGE_POINT0_mA 100
#define CALIBRATION_DISCHARGE_POINT1_mA 1000

#define ENABLE_LCD_BACKLIGHT
//#define ENABLE_FAN
//#define ENABLE_T_INTERNAL
#define ENABLE_STACK_INFO
#define ENABLE_EXPERT_VOLTAGE_CALIBRATION

#define ANALOG_INPUTS_ADC_RESOLUTION_BITS    10
#define ANALOG_INPUTS_ADC_BURST_COUNT_BITS   4 //count of measurements per channel per round (+3 for stabilization)
#define ANALOG_INPUTS_ADC_PACKAGE_COUNT_BITS 5
#define ANALOG_INPUTS_VIRTUAL_PRECISION_BITS 6
#define ANALOG_INPUTS_ADC_DELTA_SHIFT        1
#define ENABLE_ANALOG_INPUTS_ADC_NOISE

#define ANALOG_INPUTS_MAX_ADC_Vout_plus_pin     ANALOG_INPUTS_MAX_ADC_VALUE

#define CHEALI_CHARGER_ARCHITECTURE_GENERIC             2
#define CHEALI_CHARGER_ARCHITECTURE_GENERIC_STRING      "300W"

#define ENABLE_LCD_BACKLIGHT

#define MAX_CHARGE_V            ANALOG_VOLT(28.000)
#define MAX_CHARGE_I            ANALOG_AMP(10.000)
#define MAX_CHARGE_P            ANALOG_WATT(280.000)

#define MAX_DISCHARGE_P         ANALOG_WATT(1000.000)
#define MAX_DISCHARGE_I         ANALOG_AMP(10.000)

#define SMPS_UPPERBOUND_VALUE               TIMER1_PRECISION_PERIOD
#define DISCHARGER_UPPERBOUND_VALUE         TIMER1_PRECISION_PERIOD

#define OUTPUT_VOLTAGE_MINUS_PIN

//#define ENABLE_BALANCER_PWR

#endif /* HARDWARE_CONFIG_H_ */
