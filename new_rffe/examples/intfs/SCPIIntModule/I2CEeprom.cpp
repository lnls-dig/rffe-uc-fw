/* Simple access class for I2C EEPROM chips like Microchip 24LC
 * Copyright (c) 2015 Robin Hourahane
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "I2CEeprom.hpp"

I2CEeprom::I2CEeprom(PinName sda, PinName scl, int address, size_t pageSize, size_t chipSize, int busSpeed):
    m_i2c(sda, scl),
    m_i2cAddress(address),
    m_chipSize(chipSize),
    m_pageSize(pageSize)
{
    m_i2c.frequency(busSpeed);
}

size_t I2CEeprom::read(size_t address, char &value) {
    // Check the address and size fit onto the chip.
    if (!checkSpace(address, 1))
        return 0;

    char values[] = { (char)(address >> 8), (char)(address & 0xFF) };
    if (m_i2c.write(m_i2cAddress, values, 2) == 0) {
        if (m_i2c.read(m_i2cAddress, &value, 1) == 0) {
            return 1;
        }
    }

    return 0;
}


size_t I2CEeprom::read(size_t address, char *buffer, size_t size) {
    if (!checkSpace(address, size))
        return 0;

    // page_addr = 10:8 bits
    uint8_t page_addr = (address >> 8) & 0x07;
    // slave_addr = 3:1 bits
    uint8_t slave_addr = 0xA0 | (page_addr << 1); // (bits 3-1 do slave address)
    uint8_t word_addr = address & 0xFF;

    if (m_i2c.write(slave_addr, (char*)&word_addr, 1) == 0) {
        if (m_i2c.read(slave_addr, buffer, size) == 0) {
            return size;
        }
    }

    return 0;
}


size_t I2CEeprom::write(size_t address, char value) {
    // Check the address and size fit onto the chip.
    if (!checkSpace(address, 1))
        return 0;

    char values[] = { (char)(address >> 8), (char)(address & 0xFF), value };
    if (m_i2c.write(m_i2cAddress, values, 3) != 0) {
        return 0;
    }

    waitForWrite();

    return 1;
}

static inline uint8_t make_dev8(uint8_t base8, size_t addr11) {
    const uint8_t page = static_cast<uint8_t>((addr11 >> 8) & 0x07);
    uint8_t dev8 = static_cast<uint8_t>((base8 & 0xF0) | (page << 1));
    dev8 &= 0xFE;
    return dev8;
}

size_t I2CEeprom::write(size_t address, const char* buffer, size_t size) {
    if (!buffer || size == 0) return 0;
    if (!checkSpace(address, size)) return 0;

    size_t written = 0;

    while (written < size) {
        const size_t cur   = address + written;
        const uint8_t dev8 = make_dev8(static_cast<uint8_t>(m_i2cAddress), cur);

        const uint8_t word = static_cast<uint8_t>(cur & 0xFF);
        const size_t room  = 256u - static_cast<size_t>(word);
        size_t chunk       = size - written;
        if (chunk > room) chunk = room;

        while (chunk) {
            const size_t step = (chunk > 32) ? 32 : chunk;

            char tx[1 + 32];
            tx[0] = static_cast<char>( (written == 0 && step == chunk) ? word : static_cast<uint8_t>((address + written) & 0xFF) );
            memcpy(&tx[1], buffer + written, step);

            const int rc = m_i2c.write(dev8, tx, static_cast<int>(1 + step), false);
            if (rc != 0) {
                return written;
            }

            written += step;
            chunk   -= step;
        }
    }
    return written;
}


size_t I2CEeprom::fill(size_t address, char value, size_t size) {
    // Check the address and size fit onto the chip.
    if (!checkSpace(address, size))
        return 0;

    size_t left = size;

    while (left != 0) {
        size_t toWrite;
        if ((address % m_pageSize) != 0) {
            toWrite = (((address / m_pageSize) + 1) * 64) - address;
            if (toWrite > size) {
                toWrite = size;
            }
        } else {
            if (left <= m_pageSize) {
                toWrite = left;
            } else {
                toWrite = m_pageSize;
            }
        }

        char values[] = { (char)(address >> 8), (char)(address & 0xFF) };

        m_i2c.start();
        if (m_i2c.write(m_i2cAddress) == 0) {
            return size - left;
        }
        if (m_i2c.write(values[0]) == 0) {
            return size - left;
        }
        if (m_i2c.write(values[1]) == 0) {
            return size - left;
        }

        for (int count = 0; count != toWrite; ++count) {
            if (m_i2c.write(value) == 0)
                return size - left;
        }

        m_i2c.stop();

        waitForWrite();

        left -= toWrite;
        address += toWrite;
    }

    return size;
}

void I2CEeprom::waitForWrite() {
    // The chip doesn't ACK while writing to the actual EEPROM
    // so loop trying to do a zero byte write until it is ACKed
    // by the chip.
    while (m_i2c.write(m_i2cAddress, 0, 0) != 0) {
        // Wait for ack.
        // wait_ms(1);
        thread_sleep_for(1);
    }
}

bool I2CEeprom::checkSpace(size_t address, size_t size) {
    // Only check if chip size is non-zero.
    if (m_chipSize != 0) {
        // Check that the address start in the chip and doesn't
        // extend past.
        if ((address >= m_chipSize) || ((address + size) >= m_chipSize))
            return false;
        else
            return true;
    }

    return true;
}

