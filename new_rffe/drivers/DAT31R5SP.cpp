#include "DAT31R5SP.hpp"

DAT31R5SP::DAT31R5SP(mbed::DigitalOut * le, mbed::DigitalOut * clk,
                     mbed::DigitalOut * dataA, mbed::DigitalOut * dataB,
                     mbed::DigitalOut * dataC, mbed::DigitalOut * dataD)
    :  _le(le), _clk(clk), _dataA(dataA), _dataB(dataB), _dataC(dataC), _dataD(dataD)
{
    *_le = 0;
    *_clk = 0;
    *_dataA = 0;
    *_dataB = 0;
    *_dataC = 0;
    *_dataD = 0;
}

bool DAT31R5SP::set_attenuation(float dB)
{
    if(dB < 0 || dB > 31.5f) return false;
    // Convert dB value to 6-bit control word (0.5 dB per bit)
    uint8_t value = static_cast<uint8_t>(dB /0.5);
    // Send the value to all data lines simultaneously
    _bit_banging(value);
    // Store the last written value for retrieval
    _att_stored = value;
    return true;
}

float DAT31R5SP::get_attenuation()
{
    // Convert stored 6-bit value back to dB
    return  _att_stored * 0.5;
}

bool DAT31R5SP::is_valid() const
{
    return _le && _clk && _dataA && _dataB && _dataC && _dataD;
}

void DAT31R5SP::_bit_banging(uint8_t value)
{
    *_le = 0;

    // Send 6-bit value (MSB first) over all data lines
    for(int i = 5; i >= 0; i--)
    {
        *_clk = 0;

        // Set all data lines to the current bit (broadcast same attenuation to all channels)
        uint8_t bit = (value >> i) & 0x01;
        *_dataA = bit;
        *_dataB = bit;
        *_dataC = bit;
        *_dataD = bit;

        ThisThread::sleep_for(1ms);
        *_clk = 1;
        ThisThread::sleep_for(1ms);
    }
    *_clk = 0;
    ThisThread::sleep_for(1ms);

    *_le = 1;
    ThisThread::sleep_for(1ms);
    *_le = 0;
}
