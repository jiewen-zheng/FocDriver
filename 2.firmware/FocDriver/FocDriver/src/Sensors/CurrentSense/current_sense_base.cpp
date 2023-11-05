//
// Created by Monster on 2023/6/5.
//

#include "current_sense_base.h"


/**
 * Function reading the magnitude of the current set to the motor
 *  It returns the abosolute or signed magnitude if possible
 *  It can receive the motor electrical angle to help with calculation
 * @param electrical_angle - electrical angle of the motor (optional)
 * @return
 */
float CurrentSenseBase::getDCCurrent(float electrical_angle) {
    return 0;
}

DQCurrent_t CurrentSenseBase::getFOCCurrents(float electrical_angle) {
    return DQCurrent_t();
}
