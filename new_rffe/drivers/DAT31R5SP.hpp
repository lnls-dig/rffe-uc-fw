/**
* @file DAT31R5SP.hpp
* @brief Driver for controlling up to four DAT-31R5-SP+ digital attenuators using bit-banging.
*
* This class provides an interface to control the DAT-31R5-SP+ digital attenuator from Mini-Circuits
* via software-driven serial communication (bit-banging), compatible with Mbed OS.
* The device operates with a 6-bit serial control word using Data, Clock, and Latch Enable signals.
*
* This implementation supports up to four data lines (dataA, dataB, dataC, dataD),
* allowing simultaneous programming of up to four attenuators.
*
* All GPIO pins used (LE, CLK, DATAx) are generic and do not require dedicated SPI hardware,
* since the driver performs manual bit-level control using bit-banging. As a result, the same
* attenuation level is applied to all connected data lines simultaneously, enabling consistent
* configuration across multiple attenuators.
*
* Device reference: https://www.minicircuits.com/pdfs/DAT-31R5-SP+.pdf
*/

    #ifndef DAT31R5SP_H_
    #define DAT31R5SP_H_

#include "mbed.h"
#include <cstdint>
#include <iostream>

/**
 * @class DAT31R5SP
 * @brief Class for serial communication with DAT-31R5-SP+ attenuators using GPIO-based bit-banging.
 */
class DAT31R5SP {
public:
    /**
     * @brief Constructor.
     * @param le     Pointer to the shared Latch Enable (LE) control pin.
     * @param clk    Pointer to the shared Clock pin.
     * @param dataA  Pointer to the data pin for attenuator channel A.
     * @param dataB  Pointer to the data pin for attenuator channel B.
     * @param dataC  Pointer to the data pin for attenuator channel C.
     * @param dataD  Pointer to the data pin for attenuator channel D.
     */
    DAT31R5SP(mbed::DigitalOut* le, mbed::DigitalOut* clk,
            mbed::DigitalOut* dataA, mbed::DigitalOut* dataB,
            mbed::DigitalOut* dataC, mbed::DigitalOut* dataD);

    /**
    * @brief Sets the attenuation level in dB (range: 0.5 to 31.5 in 0.5 dB steps).
    * @param dB Desired attenuation level.
    * @return true if the value is valid and the operation succeeds; false otherwise.
    */
    bool set_attenuation(float dB);

    /**
     * @brief Returns the last attenuation value applied to the device.
     * @return Attenuation in dB.
     */
    float get_attenuation();

    bool is_valid() const;
private:
    /**
    * @brief Sends a 6-bit attenuation control word to all active data channels via bit-banging.
    * @param value Encoded attenuation value (0–63), where each bit represents a step (e.g., 0.5 dB).
    */
    void _bit_banging(uint8_t value);

    mbed::DigitalOut* _le;
    mbed::DigitalOut* _clk;
    mbed::DigitalOut* _dataA;
    mbed::DigitalOut* _dataB;
    mbed::DigitalOut* _dataC;
    mbed::DigitalOut* _dataD;
    uint8_t _att_stored = 0; ///< Stores the last applied attenuation value (0 to 63).
};

#endif // DAT31R5SP_H_
