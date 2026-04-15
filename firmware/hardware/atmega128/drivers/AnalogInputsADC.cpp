//
// Save data from internal ADC to AnalogInputs, calculate virtual inputs, do some processing on raw values (like noise)
//

#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "atomic.h"
#include "memory.h"
#include "string.h"
#include "SMPS_PID.h"
#include "Utils.h"
#include "Settings.h"
#include "AnalogInputsPrivate.h"
#include "IO.h"
#include "SMPS.h"
#include "Discharger.h"
#include "AnalogInputs.h"
#include "Calibration.h"

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

// #define ADC_I_SMPS_PER_ROUND 4
/*#ifdef ENABLE_SIMPLIFIED_VB0_VB2_CIRCUIT
#define ENABLE_ADC_MUX_CAPACITOR_DISCHARGE
// discharge mux ADC capacitor on Vb6
#define ADC_CAPACITOR_DISCHARGE_ADDRESS MADDR_V_BALANSER6
#endif*/

namespace AnalogInputsADC
{
    struct adc_channel
    {
        int8_t mux;
        uint8_t adc;
        AnalogInputs::Name ai_name;
        uint8_t noise;
    };

#define NO_NOISE 0
    const adc_channel analog_inputs_order[] = {
        // commutator port output, adc pin, enum name, noise
        {-1, ADC_BAT_PLUS_CH, AnalogInputs::Vout_plus_pin, 10},
        {-1, ADC_BAT_MINUS_CH, AnalogInputs::Vout_minus_pin, 10},
        {-1, ADC_CHRG_I_CH, AnalogInputs::Ismps, NO_NOISE},
        {-1, ADC_DISC_I_CH, AnalogInputs::Idischarge, NO_NOISE},
        {MUX_V1_CH, ADC_4051_CH, AnalogInputs::Vb1_pin, NO_NOISE},
        {MUX_V2_CH, ADC_4051_CH, AnalogInputs::Vb2_pin, NO_NOISE},
        {MUX_V3_CH, ADC_4051_CH, AnalogInputs::Vb3_pin, NO_NOISE},
        {MUX_V4_CH, ADC_4051_CH, AnalogInputs::Vb4_pin, NO_NOISE},
        {MUX_V5_CH, ADC_4051_CH, AnalogInputs::Vb5_pin, NO_NOISE},
        {MUX_V6_CH, ADC_4051_CH, AnalogInputs::Vb6_pin, NO_NOISE},
        {-1, ADC_VIN_CH, AnalogInputs::Vin, NO_NOISE},
        {-1, ADC_TEMP_EXT_CH, AnalogInputs::Textern, NO_NOISE},
    };

    inline void _setupNextInput();
    inline bool _setNextInput();
    inline void _setADCChannel(uint8_t pin);
    inline void _setCommutator(uint8_t mux);
    void _processConversion(uint16_t adc_raw_value);
    void _finalizeMeasurement();
    void _addAdcNoise();

    volatile uint16_t adc_raw_values[PHYSICAL_INPUTS_COUNT];
    volatile uint32_t adc_sum_values[PHYSICAL_INPUTS_COUNT]; // sum of raw values
    volatile uint32_t adc_sum_values_copy[PHYSICAL_INPUTS_COUNT];
    volatile bool _isDataReady = false;
    
    void init()
    {
        IO::resetIO(MUX_PORT, MUX_ADR0_PORT | MUX_ADR1_PORT | MUX_ADR2_PORT);
        IO::setIO(MUX_DDR, MUX_ADR0_PORT | MUX_ADR1_PORT | MUX_ADR2_PORT);
        _setupNextInput();

        // ADC Auto Trigger Source - Free Running mode
        // ADEN: ADC Enable
        // ADFR: ADC Free Running Select
        // ADIE: ADC Interrupt Enable
        // ADPS2:0: ADC Prescaler Select Bits = 20MHz/ 64 = 312kHz (TODO: above the recommended value)
        // 24 ksps - on 16 samples 1.5ksps per channel - 125 sps package
        /* atmega datasheet:
        By default, the successive approximation circuitry requires an input clock frequency between
        50kHz and 200kHz to get maximum resolution. If a lower resolution than 10 bits is needed, the
        input clock frequency to the ADC can be higher than 200kHz to get a higher sample rate. */
        ADCSRA = _BV(ADEN) | _BV(ADFR) | _BV(ADIF) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1);

        // start conversion
        ADCSRA |= _BV(ADSC);
    }

    static volatile uint8_t _currentPackageNumber = ANALOG_INPUTS_ADC_PACKAGE_COUNT;
    void loop()
    {
        if (_currentPackageNumber)
            return;

        // calculationCount_++;

        // i_deltaAvrSumVoutPlus_ += adc_sum_values[Vout_plus_pin] >> ANALOG_INPUTS_ADC_DELTA_SHIFT;
        // i_deltaAvrSumVoutMinus_ += adc_sum_values[Vout_minus_pin] >> ANALOG_INPUTS_ADC_DELTA_SHIFT;
        // i_deltaAvrSumTextern_ += adc_sum_values[Textern] >> ANALOG_INPUTS_ADC_DELTA_SHIFT;
        // i_deltaAvrCount_++;
        // finalizeDeltaMeasurement();

        for (int i = 0; i < PHYSICAL_INPUTS_COUNT; i++)
        {
            AnalogInputs::Name name = AnalogInputs::Name(i);
            AnalogInputs::ValueType real = Calibration::calibrateValue(name, adc_sum_values_copy[i] >> (ANALOG_INPUTS_ADC_BURST_COUNT_BITS +
                                                                                                   ANALOG_INPUTS_ADC_PACKAGE_COUNT_BITS -
                                                                                                   ANALOG_INPUTS_VIRTUAL_PRECISION_BITS));
            AnalogInputs::setReal(name, real);

            adc_sum_values[i] = 0;
        }

        AnalogInputs::finalizeFullVirtualMeasurement();

        // TODO: make fan logic here
        // if (onTintern_)
        //{
        //    setRealBasedOnAvr(AnalogInputs::Tintern);
        //}

        _currentPackageNumber = 1 << ANALOG_INPUTS_ADC_PACKAGE_COUNT_BITS;
    }

#define ANALOG_INPUTS_ADC_BURST_COUNT (1 << ANALOG_INPUTS_ADC_BURST_COUNT_BITS)
#if ANALOG_INPUTS_ADC_BURST_COUNT < 2
#error "ANALOG_INPUTS_ADC_BURST_COUNT < 2"
#endif

    adc_channel adc_input;
    static volatile uint8_t _currentBurstNumber = 0; // state machine variable
    void onInterrupt()
    {
        // ignore first 3 measurements, ADC channel needs to stabilize
        if (_currentBurstNumber > 2)
        {
            _processConversion(ADC);
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

        switch (_currentBurstNumber)
        {
        case ANALOG_INPUTS_ADC_BURST_COUNT - 4:
            /* update PID if necessary */
            if (adc_input.ai_name == AnalogInputs::Ismps)
                SMPS_PID::update();
            break;

#ifdef ENABLE_ANALOG_INPUTS_ADC_NOISE
        case ANALOG_INPUTS_ADC_BURST_COUNT - 2:
            _addAdcNoise();
            break;
#endif

        case ANALOG_INPUTS_ADC_BURST_COUNT + 2:
            /* set next adc input */
            _setupNextInput();

            _currentBurstNumber = 0;
        }

        _currentBurstNumber++;
    }

    void _processConversion(uint16_t adc_raw_value)
    {
        AnalogInputs::Name name = adc_input.ai_name;

        adc_raw_values[name] = adc_raw_value;
        adc_sum_values[name] += adc_raw_value;
    }

    inline void _setADCChannel(uint8_t pin)
    {
        // TODO: use differential input for Vbat?
        //  ADLAR - ADC Left Adjust Result
        //  REFS1:0 = 00 - AREF, Internal Vref turned off
        ADMUX = _BV(ADLAR) | pin;
    }

    inline void _setCommutator(uint8_t mux)
    {
        IO::resetIO(MUX_PORT, MUX_ADR0_PORT | MUX_ADR1_PORT | MUX_ADR2_PORT);
        IO::setIO(MUX_PORT, mux);
    }

    // Enable pull-up resistor as noise for time time(i dont understand this)
    void _addAdcNoise()
    {
        if (!settings.adcNoise)
            return;
        if (adc_input.noise == NO_NOISE)
            return;

        uint8_t adcbit = 1 << adc_input.adc;
        uint8_t time = AnalogInputs::i_avrCount_ & 0x0F;
        time += adc_input.noise;

        // we only add positive noise
        // PORTA up (charge, pin: Pull-up resistor)
        uint8_t old_portf = PORTF;
        PORTF |= adcbit;
        _delay_loop_1(time);
        PORTF = old_portf;
    }

    static volatile uint8_t _currentInput = 0;
    void _setupNextInput()
    {
        bool packageComplete = _setNextInput();

        _setADCChannel(adc_input.adc);
        if (adc_input.mux >= 0)
        {
            // switch analog commutator
            _setCommutator(adc_input.mux);
        }

        if (packageComplete)
        {
            _finalizeMeasurement();
        }
    }

    inline bool _setNextInput()
    {
        bool packageComplete = false;
        do
        {
            if (++_currentInput >= sizeOfArray(analog_inputs_order))
            {
                packageComplete = true;
                _currentInput = 0;
            }

            adc_input = analog_inputs_order[_currentInput];

            // TODO: some logic to skip
            /*if (adc_input.ai_name == AnalogInputs::Textern)
            {
                //if (settings.TempOutput)
                //{
                    break;
                //}
            }*/

            break;

        } while (true);

        return packageComplete;
    }

    volatile uint16_t i_avrCount_ = 0;
    void _finalizeMeasurement()
    {
        adc_raw_values[AnalogInputs::IsmpsSet] = SMPS::getValue(); //TODO: set in Ismps
        adc_raw_values[AnalogInputs::IdischargeSet] = Discharger::getValue();

        _currentPackageNumber--;
        if (!_currentPackageNumber)
        {
            if(_isDataReady)
                return; // if previous package is not processed, skip this one

            memcpy((void*)adc_sum_values_copy, (void*)adc_sum_values, sizeof(adc_sum_values));
            _isDataReady = true;
            _currentPackageNumber = 1 << ANALOG_INPUTS_ADC_PACKAGE_COUNT_BITS;
        }
    }
} // namespace AnalogInputsADC

ISR(ADC_vect)
{
    AnalogInputsADC::onInterrupt();
}
