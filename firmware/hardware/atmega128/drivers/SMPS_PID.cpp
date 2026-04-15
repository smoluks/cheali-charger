#include "SMPS_PID.h"
#include "IO.h"
#include "AnalogInputs.h"
#include "atomic.h"
#include "Monitor.h"
#include "Utils.h"

// #define ENABLE_DEBUG
#include "debug.h"

STATIC_ASSERT(MAX_PID_MV_FACTOR < 1.61);

#define A 4
extern bool adcDebugStop;

namespace SMPS_PID
{
    volatile uint16_t i_PID_setpoint;
    // we have to use i_PID_CutOffVoltage, on some chargers (M0516) ADC can read up to 60V
    volatile uint16_t i_PID_CutOffVoltage;
    volatile long i_PID_MV;
    volatile bool i_PID_enable;

    void init(uint16_t Vin, uint16_t Vout)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            i_PID_setpoint = 0;
            if (Vout > Vin)
            {
                i_PID_MV = TIMER1_PRECISION_PERIOD;
            }
            else
            {
                i_PID_MV = 0;
            }

            i_PID_MV <<= PID_MV_PRECISION;
            i_PID_enable = true;
        }
    }

    void update()
    {
        if (!i_PID_enable)
            return;

        // if Vout is too high disable PID
        if (AnalogInputs::getADCValue(AnalogInputs::Vout_plus_pin) >= i_PID_CutOffVoltage)
        {
            hardware::disableChargerOutput();
            i_PID_enable = false;
            runDebug(adcDebugStop = true);
            LogDebug(AnalogInputs::getADCValue(AnalogInputs::Vout_plus_pin), ">=", i_PID_CutOffVoltage);
            Monitor::i_externalError = MONITOR_EXTERNAL_ERROR_BATTERY_DISCONNECTED;
            return;
        }

        // TODO: rewrite PID
        // this is the PID - actually it is an I (Integral part) - should be rewritten
        uint16_t PV = AnalogInputs::getADCValue(AnalogInputs::Ismps);
        long error = i_PID_setpoint;
        error -= PV;
        i_PID_MV += error * A;

        if (i_PID_MV < 0)
            i_PID_MV = 0;
        if (i_PID_MV > MAX_PID_MV_PRECISION)
        {
            i_PID_MV = MAX_PID_MV_PRECISION;
        }
        SMPS_PID::setPID_MV(i_PID_MV >> PID_MV_PRECISION);
    }

    uint16_t getPIDValue()
    {
        //    return PID_setpoint;
        uint16_t v;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            v = i_PID_MV >> PID_MV_PRECISION;
        }
        return v;
    }

    void setPID_MV(uint16_t value)
    {
        if (value > MAX_PID_MV)
            value = MAX_PID_MV;

        Timer1::setChargerValue(value);
    }

    //Set emergency disable voltage
    void setVoutCutoff(AnalogInputs::ValueType v)
    {
        if (v > MAX_CHARGE_V)
        {
            v = MAX_CHARGE_V;
        }
        AnalogInputs::ValueType cutOff = AnalogInputs::reverseCalibrateValue(AnalogInputs::Vout_plus_pin, v);
        if (cutOff > ANALOG_INPUTS_MAX_ADC_Vout_plus_pin)
        {
            // extra limit if calibration is wrong
            cutOff = ANALOG_INPUTS_MAX_ADC_Vout_plus_pin;
        }

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            i_PID_CutOffVoltage = cutOff;
        }
    }

    void setChargerValue(uint16_t value)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            i_PID_setpoint = value;
        }
    }

    void disablePID()
    {
        i_PID_enable = false;
    }
}
