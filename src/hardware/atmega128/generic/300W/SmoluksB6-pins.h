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
#ifndef PINS_H_
#define PINS_H_

#define PIN_0                    (1 << 0)
#define PIN_1                    (1 << 1)
#define PIN_2                    (1 << 2)
#define PIN_3                    (1 << 3)
#define PIN_4                    (1 << 4)
#define PIN_5                    (1 << 5)
#define PIN_6                    (1 << 6)
#define PIN_7                    (1 << 7)
#define PIN_PASTHALF             0xF0
#define PIN_ALL                  0xFF

// pin configuration
#define MUX_ADR2_PIN            1
#define MUX_ADR1_PIN            2
#define MUX_ADR0_PIN            3

//pin 4 - 8 - reset, Vcc, GND, XTAL2, XTAL1

//pin 9 - rs232 RX
#define FAN_PIN                 9
//pin 10 - rs232 TX

#define DISCHARGE_DISABLE_PIN   11
#define OUTPUT_DISABLE_PIN      12
#define BACKLIGHT_PIN           13
// pin 14 - the same pin for charge and discharge
#define DISCHARGE_VALUE_PIN     14
#define SMPS_VALUE_PIN          14
#define SMPS_DISABLE_PIN        15
#define BUZZER_PIN              16

// ----- LCD -----
#define LCD_ENABLE_8BITMODE

#define LCD_DATA_PORT          &PORTC
#define LCD_DATA_DDR           &DDRC

#define LCD_RS_PORT            &PORTD, 7
#define LCD_RS_DDR             &DDRD, 7

#define LCD_RW_PORT            &PORTG, 1
#define LCD_RW_DDR             &DDRG, 1

#define LCD_E_PORT             &PORTG, 0
#define LCD_E_DDR              &DDRG, 0

// ----- Buttons -----
#define BUTTON_STOP_PORT       &PORTB, 3
#define BUTTON_STOP_DDR        &DDRB, 3
#define BUTTON_STOP_PIN        &PINB, 3

#define BUTTON_DEC_PORT       &PORTB, 2
#define BUTTON_DEC_DDR        &DDRB, 2
#define BUTTON_DEC_PIN        &PINB, 2

#define BUTTON_INC_PORT       &PORTE, 6
#define BUTTON_INC_DDR        &DDRE, 6
#define BUTTON_INC_PIN        &PINE, 6

#define BUTTON_START_PORT       &PORTE, 7
#define BUTTON_START_DDR        &DDRE, 7
#define BUTTON_START_PIN        &PINE, 7


#define BALANCER7_LOAD_PIN      25
#define BALANCER8_LOAD_PIN      26

// pin 27 - 29 - AVcc, GND, AREF

#define BALANCER6_LOAD_PIN      30
#define BALANCER5_LOAD_PIN      31
//pin 32
#define MUX0_Z_A_PIN            A5
#define MUX0_Z_D_PIN            32
//pin 33
#define MUX1_Z_A_PIN            A4
#define MUX1_Z_D_PIN            33
//pin 34
#define DISCHARGE_CURRENT_PIN   A3
//pin 35 - ??
#define SMPS_CURRENT_PIN        A2
//pin 36
#define OUTPUT_VOLTAGE_MINUS_PIN    A1
//pin 37
#define OUTPUT_VOLTAGE_PLUS_PIN     A0

// pin 38,39 - Vcc, GND

#define BALANCER1_LOAD_PIN      40
#define BALANCER2_LOAD_PIN      41
#define BALANCER3_LOAD_PIN      42
#define BALANCER4_LOAD_PIN      43

#define BALANCER_PWR_ENABLE_PIN        44



//Multiplexer addresses
#define MUXINPUT0     0
#define MUXINPUT1     0

#define MADDR_V_OUTMUX          (0 + MUXINPUT0)
#define MADDR_T_INTERN          (1 + MUXINPUT0)
#define MADDR_V_IN              (2 + MUXINPUT0)
#define MADDR_T_EXTERN          (3 + MUXINPUT0)

#define MADDR_V_BALANSER1       (0 + MUXINPUT1)
#define MADDR_V_BALANSER2       (1 + MUXINPUT1)
#define MADDR_V_BALANSER3       (2 + MUXINPUT1)
#define MADDR_V_BALANSER4       (3 + MUXINPUT1)
#define MADDR_V_BALANSER5       (4 + MUXINPUT1)
#define MADDR_V_BALANSER6       (5 + MUXINPUT1)
#define MADDR_V_BALANSER7       (6 + MUXINPUT1)
#define MADDR_V_BALANSER8       (7 + MUXINPUT1)


#endif /* PINS_H_ */
