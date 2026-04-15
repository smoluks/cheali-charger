#include "AnalogInputs.h"
#include "Time.h"

#define ANALOG_INPUTS_ADC_MEASUREMENTS_COUNT (ANALOG_INPUTS_ADC_PACKAGE_COUNT * ANALOG_INPUTS_ADC_BURST_COUNT)

#if (1 << ANALOG_INPUTS_RESOLUTION) * ANALOG_INPUTS_ADC_MEASUREMENTS_COUNT > UINT32_MAX
#error "avr sum don't fit into uint32_t"
#endif

// TODO: 120?? we take not more then 60 measurements into account
#if ((1 << ANALOG_INPUTS_RESOLUTION) >> ANALOG_INPUTS_ADC_DELTA_SHIFT) * ANALOG_INPUTS_ADC_MEASUREMENTS_COUNT * 120 > UINT32_MAX
#error "delta avr sum don't fit into uint32_t"
#endif

namespace DeltaAnalogInputs
{
    AnalogInputs::ValueType getDeltaLastT() { return deltaLastT_; }
    AnalogInputs::ValueType getDeltaCount() { return deltaCount_; }
    void enableDeltaVoutMax(bool enable) { enable_deltaVoutMax_ = enable; }

    uint16_t i_deltaAvrCount_;
    uint32_t i_deltaAvrSumVoutPlus_;
    uint32_t i_deltaAvrSumVoutMinus_;
    uint32_t i_deltaAvrSumTextern_;

    uint16_t deltaCount_;
    AnalogInputs::ValueType deltaLastT_;
    uint16_t deltaStartTimeU16_;
    bool enable_deltaVoutMax_;

    void finalizeDeltaMeasurement()
    {
        if (Time::diffU16(deltaStartTimeU16_, Time::getMilisecondsU16()) > ANALOG_INPUTS_DELTA_TIME_MILISECONDS)
        {
            uint32_t deltaAvrCount;
            uint32_t deltaAvrSumVoutPlus;
            uint32_t deltaAvrSumVoutMinus;
            uint32_t deltaAvrSumTextern;

            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                deltaAvrCount = i_deltaAvrCount_;
                deltaAvrCount *= ANALOG_INPUTS_ADC_MEASUREMENTS_COUNT;
                deltaAvrCount >>= ANALOG_INPUTS_ADC_DELTA_SHIFT;
                deltaAvrSumVoutPlus = i_deltaAvrSumVoutPlus_;
                deltaAvrSumVoutMinus = i_deltaAvrSumVoutMinus_;
                deltaAvrSumTextern = i_deltaAvrSumTextern_;
            }
            _resetDeltaAvr();
            deltaCount_++;

            uint16_t x;
            ValueType real, old, VoutPlus, VoutMinus;

            // calculate deltaVout
            deltaAvrSumVoutPlus /= deltaAvrCount;
            deltaAvrSumVoutMinus /= deltaAvrCount;

            VoutPlus = calibrateValue(Vout_plus_pin, deltaAvrSumVoutPlus);
            VoutMinus = calibrateValue(Vout_minus_pin, deltaAvrSumVoutMinus);
            real = 0;
            if (VoutPlus > VoutMinus)
                real = VoutPlus - VoutMinus;

            old = getRealValue(deltaVoutMax);
            if (real >= old || (!enable_deltaVoutMax_))
            {
                setReal(deltaVoutMax, real);
            }
            setReal(deltaVout, real - old);

            // calculate deltaTextern
            uint16_t dc = 2;
#if ANALOG_INPUTS_DELTA_TIME_MILISECONDS != 30000
#error "ANALOG_INPUTS_DELTA_TIME_MILISECONDS != 30000"
#endif
            deltaAvrSumTextern /= deltaAvrCount;
            x = deltaAvrSumTextern;
            real = calibrateValue(Textern, x);
            old = deltaLastT_;
            deltaLastT_ = real;
            real -= old;
            real *= dc;

            setReal(deltaTextern, real);
            setReal(deltaLastCount, deltaAvrCount);
        }
    }

    void _resetDeltaAvr()
    {
        i_deltaAvrCount_ = 0;
        i_deltaAvrSumVoutPlus_ = 0;
        i_deltaAvrSumVoutMinus_ = 0;
        i_deltaAvrSumTextern_ = 0;
        deltaStartTimeU16_ = Time::getMilisecondsU16();
    }
}