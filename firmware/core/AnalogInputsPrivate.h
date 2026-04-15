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
#ifndef ANALOGINPUTSPRIVATE_H_
#define ANALOGINPUTSPRIVATE_H_

#include "AnalogInputs.h"
#include "Calibration.h"

namespace AnalogInputs {

    extern const Calibration::DefaultValues inputsP_[];//AnalogInputs::PHYSICAL_INPUTS];
    extern ValueType real_[ALL_INPUTS];
    extern ValueType avrAdc_[PHYSICAL_INPUTS_COUNT];
    extern volatile uint16_t  i_avrCount_;
    extern volatile uint32_t  adc_sum_values[PHYSICAL_INPUTS_COUNT];

    extern volatile bool on_;
    extern volatile bool onTintern_;

    void intterruptFinalizeMeasurement();
    void resetStable();

    void loop();
    void do100msInterrupt();
};


#endif /* ANALOGINPUTSPRIVATE_H_ */
