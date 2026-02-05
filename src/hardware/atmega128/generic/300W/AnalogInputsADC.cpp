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

#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "atomic.h"
#include "Hardware.h"
#include "SMPS_PID.h"
#include "Utils.h"
#include "memory.h"
#include "Settings.h"
#include "Timer0.h"
#include "AnalogInputsPrivate.h"
#include "IO.h"
#include "SMPS.h"
#include "Discharger.h"

// we need ProgramData::battery.enable_externT
#include "ProgramData.h"

// #define ENABLE_DEBUG
#include "debug.h"
#ifdef ENABLE_DEBUG
#define MAX_DEBUG_DATA 160
uint16_t adcDebugData[MAX_DEBUG_DATA];
uint8_t adcDebugCount = 0;
uint16_t adcDebugTime = 0;
bool adcDebugStart = false;

void LogDebug_run()
{
    if (adcDebugCount >= MAX_DEBUG_DATA && Time::diffU16(adcDebugTime, Time::getMilisecondsU16()) > 1000)
    {
        for (int i = 0; i < MAX_DEBUG_DATA; i++)
        {
            LogDebug(i, ':', adcDebugData[i]);
        }
        LogDebug('-');
        adcDebugStart = false;
        adcDebugCount = 0;
        adcDebugTime = Time::getMilisecondsU16();
    }
}
#endif

/* ADC - measurement:
 * program flow: see conversionDone()
 */

#define ADC_I_SMPS_PER_ROUND 4

#ifdef ENABLE_SIMPLIFIED_VB0_VB2_CIRCUIT
#define ENABLE_ADC_MUX_CAPACITOR_DISCHARGE
// discharge mux ADC capacitor on Vb6
#define ADC_CAPACITOR_DISCHARGE_ADDRESS MADDR_V_BALANSER6
#endif

namespace AnalogInputsADC
{
    void setupNextInput();

    void initialize()
    {
        IO::resetIO(MUX_PORT, MUX_ADR0_PORT | MUX_ADR1_PORT | MUX_ADR2_PORT);
        IO::setIO(MUX_DDR, MUX_ADR0_PORT | MUX_ADR1_PORT | MUX_ADR2_PORT);

        // ADC Auto Trigger Source - Free Running mode
        // ADEN: ADC Enable
        // ADFR: ADC Free Running Select
        // ADIE: ADC Interrupt Enable
        // ADPS2:0: ADC Prescaler Select Bits = 20MHz/ 64 = 312kHz (TODO: above the recommended value)
        /* atmega datasheet:
        By default, the successive approximation circuitry requires an input clock frequency between
        50kHz and 200kHz to get maximum resolution. If a lower resolution than 10 bits is needed, the
        input clock frequency to the ADC can be higher than 200kHz to get a higher sample rate. */
        ADCSRA = _BV(ADEN) | _BV(ADFR) | _BV(ADIF) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1);

        // start conversion
        ADCSRA |= _BV(ADSC);
    }

    struct adc_correlation
    {
        int8_t mux;
        uint8_t adc;
        AnalogInputs::Name ai_name;
        uint8_t noise;
    };

#define ADC_STANDARD_PER_ROUND 2
#define NO_NOISE 0

    const adc_correlation order_analogInputs_on[] PROGMEM = {
        // commutator port output, adc pin, enum name, keyboard?, noise

        {MUX_V1_CH, ADC_4051_CH, AnalogInputs::Vb1_pin, NO_NOISE},
        {MUX_V2_CH, ADC_4051_CH, AnalogInputs::Vb2_pin, NO_NOISE},
        {MUX_V3_CH, ADC_4051_CH, AnalogInputs::Vb3_pin, NO_NOISE},
        {MUX_V4_CH, ADC_4051_CH, AnalogInputs::Vb4_pin, NO_NOISE},
        {MUX_V5_CH, ADC_4051_CH, AnalogInputs::Vb5_pin, NO_NOISE},
        {MUX_V6_CH, ADC_4051_CH, AnalogInputs::Vb6_pin, NO_NOISE},
        {-1, ADC_VIN_CH, AnalogInputs::Vin, NO_NOISE},
        {-1, ADC_TEMP_EXT_CH, AnalogInputs::Textern, NO_NOISE},
        {-1, ADC_BAT_PLUS_CH, AnalogInputs::Vout_plus_pin, 10},
        {-1, ADC_BAT_MINUS_CH, AnalogInputs::Vout_minus_pin, 10},
        {-1, ADC_CHRG_I_CH, AnalogInputs::Ismps, NO_NOISE},
        {-1, ADC_DISC_I_CH, AnalogInputs::Idischarge, NO_NOISE},
    };

    inline uint8_t nextInput(uint8_t i)
    {
        if (++i >= sizeOfArray(order_analogInputs_on))
            i = 0;
        return i;
    }

    adc_correlation adc_input;
    adc_correlation adc_input_next;
    static volatile uint8_t g_addSumToInput = 0;

    // static uint8_t adc_keyboard_;

    inline void setADCChannel(uint8_t pin)
    {
        // TODO: use differential input for Vbat?
        //  ADLAR - ADC Left Adjust Result
        //  REFS1:0 = 00 - AREF, Internal Vref turned off
        ADMUX = _BV(ADLAR) | pin;
    }

    inline void setCommutator(uint8_t mux)
    {
        IO::resetIO(MUX_PORT, MUX_ADR0_PORT | MUX_ADR1_PORT | MUX_ADR2_PORT);
        IO::setIO(MUX_PORT, mux);
    }

    void processConversion(uint16_t adc_raw_value)
    {
        AnalogInputs::Name name = adc_input.ai_name;
        // if (name != AnalogInputs::VirtualInputs)
        //{
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            AnalogInputs::i_adc_[name] = adc_raw_value;
        }
        if (g_addSumToInput)
            AnalogInputs::i_avrSum_[name] += adc_raw_value;
        //}
        /*else
        {
            uint8_t key = adc_input.key;
            uint8_t high = adc_raw_value >> 8;

            if (high < ADC_KEY_BORDER)
            {
                adc_keyboard_ |= key;
            }
            else
            {
                adc_keyboard_ &= ~key;
            }
        }*/
    }

    void finalizeMeasurement()
    {
        AnalogInputs::i_adc_[AnalogInputs::IsmpsSet] = SMPS::getValue();
        AnalogInputs::i_adc_[AnalogInputs::IdischargeSet] = Discharger::getValue();
        if (g_addSumToInput)
        {
            AnalogInputs::i_avrSum_[AnalogInputs::IsmpsSet] += SMPS::getValue();
            AnalogInputs::i_avrSum_[AnalogInputs::IdischargeSet] += Discharger::getValue();
            if (AnalogInputs::i_avrCount_ == 1)
            {
                AnalogInputs::i_avrSum_[AnalogInputs::Ismps] /= ADC_STANDARD_PER_ROUND;
                AnalogInputs::i_avrSum_[AnalogInputs::Vout_plus_pin] /= ADC_STANDARD_PER_ROUND;
                AnalogInputs::i_avrSum_[AnalogInputs::Vout_minus_pin] /= ADC_STANDARD_PER_ROUND;
                AnalogInputs::i_avrSum_[AnalogInputs::Idischarge] /= ADC_STANDARD_PER_ROUND;
            }
            // TODO: maybe intterruptFinalizeMeasurement should be removed
            AnalogInputs::intterruptFinalizeMeasurement();
        }
    }

    // Enable pull-up resistor as noise for time time(i dont understand this)
    void addAdcNoise()
    {
        if (!settings.adcNoise)
            return;
        if (adc_input_next.noise == NO_NOISE)
            return;

        uint8_t adcbit = 1 << adc_input_next.adc;
        uint8_t time = AnalogInputs::i_avrCount_ & 0x0F;
        time += adc_input_next.noise;

        // we only add positive noise

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // PORTA up (charge, pin: Pull-up resistor)
            uint8_t old_porta = PORTA;
            PORTA |= adcbit;
            _delay_loop_1(time);
            PORTA = old_porta;
        }
    }

#if ANALOG_INPUTS_ADC_BURST_COUNT < 2
#error "ANALOG_INPUTS_ADC_BURST_COUNT < 2"
#endif

    static volatile uint8_t _currentBurstNumber = 0; // state machine variable

    void conversionDone()
    {
        uint16_t v;
        uint8_t low, high;
        low = ADCL;
        high = ADCH;
        v = (high << 8) | low;

        // ignore first 3 measurements, ADC channel needs to stabilize
        if (_currentBurstNumber > 2)
        {
            processConversion(v);
        }

#ifdef ENABLE_DEBUG
        if (adcDebugCount < MAX_DEBUG_DATA)
        {
            if (g_adcBurstCount_ == 0 && adc_input.ai_name == AnalogInputs::Vb4_pin)
            {
                adcDebugStart = true;
            }
            if (adcDebugStart)
            {
                if (g_adcBurstCount_ == 0)
                {
                    adcDebugData[adcDebugCount++] = 7777;
                    adcDebugData[adcDebugCount++] = adc_input.ai_name;
                }
                adcDebugData[adcDebugCount++] = g_adcBurstCount_;
                adcDebugData[adcDebugCount++] = v;
            }
        }
#endif

        switch (_currentBurstNumber++)
        {
        case 0:
            /* set new mux address */
            if (adc_input_next.mux >= 0)
            {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
                {
                    // switch analog commutator
                    setCommutator(adc_input_next.mux);
                }
            }
            break;

        case ANALOG_INPUTS_ADC_BURST_COUNT - 3:
            /* update PID if necessary */
            if (adc_input.ai_name == AnalogInputs::Ismps)
                SMPS_PID::update();
            break;

#ifdef ENABLE_ANALOG_INPUTS_ADC_NOISE
        case ANALOG_INPUTS_ADC_BURST_COUNT - 1:
            addAdcNoise();
            break;
#endif

        case ANALOG_INPUTS_ADC_BURST_COUNT + 2:
            /* set next adc input */
            setADCChannel(adc_input_next.adc);
            /* switch to new input */
            _currentBurstNumber = 0;
            setupNextInput();
        }
    }

    static volatile uint8_t _currentInput = 0;
    void setupNextInput()
    {
        _currentInput = nextInput(_currentInput);
        adc_input = adc_input_next;
        pgm::read(adc_input_next, &order_analogInputs_on[nextInput(_currentInput)]);

        /*if ((!ProgramData::battery.enable_externT) && adc_input_next.mux == MADDR_T_EXTERN)
        {
            adc_input_next.mux = MADDR_V_BALANSER6;
        }*/

        if (_currentInput == 0)
        {
            finalizeMeasurement();
            g_addSumToInput = AnalogInputs::i_avrCount_ > 0;
        }
    }

} // namespace AnalogInputsADC

ISR(ADC_vect)
{
    AnalogInputsADC::conversionDone();
}
