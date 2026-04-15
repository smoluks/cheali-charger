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
#define __STDC_LIMIT_MACROS
#include "Hardware.h"
#include "AnalogInputsPrivate.h"
#include "memory.h"
#include "LcdPrint.h"
#include "SerialLog.h"
#include "eeprom.h"
#include "atomic.h"
#include "Balancer.h"

#define ANALOG_INPUTS_E_OUT_dt_FACTOR 50
#define ANALOG_INPUTS_E_OUT_DIVIDER 100

namespace AnalogInputs
{
    uint32_t i_charge_;
    uint32_t i_Eout_;
    uint8_t i_Eout_dt_;

    void init()
    {
        resetAccumulatedMeasurements();
    }

    void resetAccumulatedMeasurements()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            i_charge_ = 0;
            i_Eout_ = 0;
            i_Eout_dt_ = ANALOG_INPUTS_E_OUT_dt_FACTOR;
        }
        
        setReal(deltaVoutMax, getVout());
        resetMeasurement();
        DeltaAnalogInputs::_resetDeltaAvr();

        setReal(Cout, 0);
        setReal(deltaVout, 0);
        setReal(deltaTextern, 0);
    }

    void loop()
    {
        i_charge_ += getIout();

        if (--i_Eout_dt_ == 0)
        {
            i_Eout_dt_ = ANALOG_INPUTS_E_OUT_dt_FACTOR;

            uint32_t P = getIout();
            P *= getVout();
            uint32_t E_since_previous_measurement = P / ANALOG_INPUTS_E_OUT_DIVIDER;
            i_Eout_ += E_since_previous_measurement;
        }
    }

    void setReal(Name name, ValueType real)
    {
        if (absDiff(real_[name], real) > STABLE_VALUE_ERROR)
            stableCount_[name] = 0;
        else
            stableCount_[name]++;

        real_[name] = real;
    }

    void finalizeFullVirtualMeasurement()
    {
        //-----out_p - out_m-----
        ValueType balancer = 0;
        ValueType out_p = real_[Vout_plus_pin];
        ValueType out_m = real_[Vout_minus_pin];
        ValueType out = 0;
        if (out_m < out_p)
            out = out_p - out_m; // TODO: can we do it in raw?
        setReal(Vout, out);

        //-----balanser-----
        for (uint8_t i = 0; i < MAX_BALANCE_CELLS; i++)
        {
            setReal(Name(Vb1 + i), getRealValue(Name(Vb1_pin + i)));
        }

        if (!balancePortStateSaved_)
        {
            connectedBalancePortCells = getConnectedBalancePortCells();
        }

        uint16_t connectedCells = connectedBalancePortCells;

        for (uint8_t i = 0; i < MAX_BALANCE_CELLS; i++)
        {
            if (connectedCells & (1 << i))
                balancer += getRealValue(Name(Vb1 + i));
        }

        setReal(Vbalancer, balancer);
        Name obInfo;
        if (balancer == 0 || absDiff(out, balancer) > ANALOG_VOLT(3.000))
        {
            // balancer not connected or big error in calibration
            obInfo = Vout;
            connectedCells = 0;
        }
        else
        {
            out = balancer;
            obInfo = Vbalancer;
        }
        setReal(VoutBalancer, out);
        setReal(VbalanceInfo, connectedCells);
        setReal(VobInfo, obInfo);

        //-----Iout-----
        ValueType IoutValue = 0;
        if (Discharger::isPowerOn())
        {
            IoutValue = getRealValue(Idischarge);
        }
        else if (SMPS::isPowerOn())
        {
            IoutValue = getRealValue(Ismps);
        }

        uint32_t P = IoutValue;
        P *= out;
        P /= 10000;
        setReal(Pout, P);

        setReal(Iout, IoutValue);
        setReal(Cout, getCharge());
        setReal(Eout, getEout());
    }

    ValueType getCharge()
    {
        // check units
        STATIC_ASSERT(ANALOG_AMP(1.0) == ANALOG_CHARGE(1.0));

        uint32_t retu;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            retu = i_charge_;
        }
        return toHoursBasis(retu);
    }

    ValueType getEout()
    {
        uint32_t retu;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            retu = i_Eout_;
        }

        // check units
        STATIC_ASSERT(uint32_t(ANALOG_AMP(1.0)) * ANALOG_VOLT(1.0) / (ANALOG_INPUTS_E_OUT_DIVIDER * ANALOG_INPUTS_E_OUT_dt_FACTOR) == 2 * ANALOG_WATTH(1.0));

        retu /= 2;

        return toHoursBasis(retu);
    }

    static inline uint32_t toHoursBasis(uint32_t accumulator)
    {
        uint64_t retu = accumulator;                                                                    // sum of all measurements during slow interrupt
        retu = retu * TIMER_INTERRUPT_PERIOD_MICROSECONDS * TIMER_SLOW_INTERRUPT_INTERVAL / 3600000000; // value * us per value / us in hours
        return retu;
    }

    ValueType getVbattery()
    {
        return getRealValue(VoutBalancer);
    }

    ValueType getVout()
    {
        return getRealValue(Vout);
    }

    ValueType getIout()
    {
        return getRealValue(Iout);
    }

    bool isConnected(Name name)
    {
        if (name == Vbalancer)
        {
            return getRealValue(VobInfo) == Vbalancer;
        }
        if (getType(name) == Voltage)
        {
            return getRealValue(name) > CONNECTED_MIN_VOLTAGE;
        }
        return true;
    }

    uint16_t stableCount_[ALL_INPUTS];
    bool isOutStable()
    {
        return isStable(VoutBalancer) && isStable(Iout) && Balancer::isStable();
    }

    void resetStable()
    {
        ANALOG_INPUTS_FOR_ALL(name)
        {
            stableCount_[name] = 0;
        }
    }

    bool isBalancePortConnected()
    {
        return isConnected(Vbalancer);
    }

    uint8_t getConnectedBalancePortCells()
    {
        uint8_t cellFlags = 0;
        for (uint8_t i = 0; i < MAX_BALANCE_CELLS; i++)
        {
            if (isConnected(Name(Vb1 + i)))
            {
                cellFlags |= 1;
            }

            cellFlags <<= 1;
        }
        return cellFlags;
    }

    uint8_t getConnectedBalancePortCellsCount()
    {
        uint8_t count = 0;
        for (uint8_t i = 0; i < MAX_BALANCE_CELLS; i++)
        {
            // TODO: compare raw values
            if (getRealValue(Name(Vb1 + i)) > CONNECTED_MIN_VOLTAGE)
            {
                count++;
            }
        }
        return count;
    }

} // namespace AnalogInputs

void resetMeasurement()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i_avrCount_ = 1; // TODO:??
        ignoreLastResult_ = true;
        resetStable();
    }
}

bool isReversePolarity()
{
#ifdef OUTPUT_VOLTAGE_MINUS_PIN
    ValueType vm = getADCValue(Vout_minus_pin);
    ValueType vp = getADCValue(Vout_plus_pin);
    if (vm > vp)
        vm -= vp;
    else
        vm = 0;

    return vm > REVERSE_POLARITY_MIN_VOLTAGE;
#else
    return hardware::isReversePolarity();
#endif
}

void initialize()
{
    reset();
}

void printRealValue(Name name, uint8_t dig)
{
    ValueType x = getRealValue(name);
    Type t = getType(name);
    lcdPrintAnalog(x, dig, t);
}
