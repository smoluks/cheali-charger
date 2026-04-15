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

#define __nop() asm("nop")

//  ----- MUX ----- 
#define MUX_PORT                 &PORTE
#define MUX_DDR                  &DDRE

#define MUX_ADR0_PORT            PIN_4
#define MUX_ADR1_PORT            PIN_3
#define MUX_ADR2_PORT            PIN_2

#define MUX_V1_CH                MUX_ADR2_PORT | MUX_ADR1_PORT
#define MUX_V2_CH                MUX_ADR1_PORT
#define MUX_V3_CH                MUX_ADR0_PORT
#define MUX_V4_CH                0 
#define MUX_V5_CH                MUX_ADR2_PORT | MUX_ADR1_PORT | MUX_ADR0_PORT
#define MUX_V6_CH                MUX_ADR2_PORT | MUX_ADR0_PORT

// ----- ADC ----- 
#define ADC_TEMP_EXT_CH          0
#define ADC_BAT_MINUS_CH         1
#define ADC_BAT_PLUS_CH          2
#define ADC_VIN_CH               3
#define ADC_DISC_I_CH            4
#define ADC_CHRG_I_CH            5
#define ADC_4051_CH              6
#define ADC_TEMP_INT_CH          7

// ----- PWM -----
#define SMPS_UP_PORT             &PORTB, PIN_5
#define SMPS_UP_DDR              &DDRB, PIN_5

#define SMPS_DOWN_PORT           &PORTB, PIN_6
#define SMPS_DOWN_DDR            &DDRB, PIN_6

#define SMPS_DISABLE_PORT        &PORTB, PIN_7
#define SMPS_DISABLE_DDR         &DDRB, PIN_7

#define DISCHARGER_DISABLE_PORT  &PORTE, PIN_5
#define DISCHARGER_DISABLE_DDR   &DDRE, PIN_5

#define BATTERY_DISABLE_PORT     &PORTG, PIN_3
#define BATTERY_DISABLE_DDR      &DDRG, PIN_3

// ----- LCD -----
#define LCD_ENABLE_8BITMODE

#define LCD_DATA_PORT            &PORTC
#define LCD_DATA_DDR             &DDRC
#define LCD_DATA_PIN             &PINC

#define LCD_RS_PORT              &PORTD, PIN_7
#define LCD_RS_DDR               &DDRD, PIN_7

#define LCD_RW_PORT              &PORTG, PIN_1
#define LCD_RW_DDR               &DDRG, PIN_1

#define LCD_E_PORT               &PORTG, PIN_0
#define LCD_E_DDR                &DDRG, PIN_0

// ----- Buttons -----
#define BUTTON_STOP_PORT         &PORTB, PIN_3
#define BUTTON_STOP_DDR          &DDRB, PIN_3
#define BUTTON_STOP_PIN          &PINB, PIN_3

#define BUTTON_DEC_PORT          &PORTB, PIN_2
#define BUTTON_DEC_DDR           &DDRB, PIN_2
#define BUTTON_DEC_PIN           &PINB, PIN_2

#define BUTTON_INC_PORT          &PORTE, PIN_6
#define BUTTON_INC_DDR           &DDRE, PIN_6
#define BUTTON_INC_PIN           &PINE, PIN_6

#define BUTTON_START_PORT        &PORTE, PIN_7
#define BUTTON_START_DDR         &DDRE, PIN_7
#define BUTTON_START_PIN         &PINE, PIN_7

// ----- Balancer -----
#define BALANCER_CELL1_PORT   &PORTA, PIN_3
#define BALANCER_CELL1_DDR    &DDRA, PIN_3

#define BALANCER_CELL2_PORT   &PORTA, PIN_5
#define BALANCER_CELL2_DDR    &DDRA, PIN_5

#define BALANCER_CELL3_PORT   &PORTA, PIN_4
#define BALANCER_CELL3_DDR    &DDRA, PIN_4

#define BALANCER_CELL4_PORT   &PORTA, PIN_6
#define BALANCER_CELL4_DDR    &DDRA, PIN_6

#define BALANCER_CELL5_PORT   &PORTA, PIN_7
#define BALANCER_CELL5_DDR    &DDRA, PIN_7

#define BALANCER_CELL6_PORT   &PORTA, PIN_3
#define BALANCER_CELL6_DDR    &DDRA, PIN_3

//#define BALANCER_PWR_ENABLE_PIN        44

// ----- common gpio ----- 
#define BUZZER_PORT             &PORTB, PIN_0
#define BUZZER_DDR              &DDRB, PIN_0
#define BUZZER_PIN              &PINB, PIN_0

#define BACKLIGHT_PORT          &PORTB, PIN_4
#define BACKLIGHT_DDR           &DDRB, PIN_4

#define FAN_PORT                &PORTD, PIN_6
#define FAN_DDR                 &DDRD, PIN_6


#endif /* PINS_H_ */
