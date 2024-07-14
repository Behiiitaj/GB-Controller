#include "HD09Spindle.h"

/*
    HD09Spindle.cpp

    This is for the new HD09 HD09 VFD based spindle via RS485 Modbus.

    Part of Grbl_ESP32
    2020 -  Stefan de Bruijn

    Grbl is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

                         WARNING!!!!
    VFDs are very dangerous. They have high voltages and are very powerful
    Remove power before changing bits.

    The documentation is okay once you get how it works, but unfortunately
    incomplete... See HD09Spindle.md for the remainder of the docs that I
    managed to piece together.
*/

//1-->    02 06 32 00 10 03 ca 80   ==>  Run/backward
//2-->    02 06 00 0a 00 02 28 3a   ==>  FreqOrder
//3-->    02 06 32 01 61 a8 fe af   ==>  SetSpeed
//4-->    02 03 33 10 00 01 8a b8   ==>  Get Freq

namespace Spindles {
    HD09::HD09() : VFD() {
        _baudrate = 9600;
        _parity   = Uart::Parity::None;
    }

    void HD09::direction_command(SpindleState mode, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x32;  // Command ID 0x2000
        data.msg[3] = 0x00;
        data.msg[4] = 0x10;
        data.msg[5] = (mode == SpindleState::Ccw) ? 0x02 : (mode == SpindleState::Cw ? 0x01 : 0x10);
    }

    void HD09::set_speed_command(uint32_t rpm, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // We have to know the max RPM before we can set the current RPM:
        auto max_rpm = this->_max_rpm;

        // Speed is in [0..10'000] where 10'000 = 100%.
        // We have to use a 32-bit integer here; typical values are 10k/24k rpm.
        // I've never seen a 400K RPM spindle in my life, and they aren't supported
        // by this modbus protocol anyways... So I guess this is OK.
        uint16_t speed = rpm * 100;
        if (speed < 0) {
            speed = 0;
        }

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x32;  // Command ID 0x1000
        data.msg[3] = 0x01;
        data.msg[4] = uint8_t(speed >> 8);  // RPM
        data.msg[5] = uint8_t(speed & 0xFF);
    }

    // VFD::response_parser HD09::initialization_sequence(int index, ModbusCommand& data) {
    //     if (index == -1) {
    //         // NOTE: data length is excluding the CRC16 checksum.
    //         data.tx_length = 6;
    //         data.rx_length = 6;

    //         // Send: 01 03 B005 0002
    //         data.msg[1] = 0x06;  // READ
    //         data.msg[2] = 0x00;  // B0.05 = Get RPM
    //         data.msg[3] = 0x0A;
    //         data.msg[4] = 0x00;  // Read 2 values
    //         data.msg[5] = 0x02;

    //         //  Recv: 01 03 00 04 5D C0 03 F6
    //         //                    -- -- = 24000 (val #1)
    //         return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
    //             // Set current RPM value? Somewhere?
    //             vfd->_max_rpm = 250;
    //             return true;
    //         };
    //     }
    //     else if (index == 0) {
    //         // NOTE: data length is excluding the CRC16 checksum.
    //         data.tx_length = 6;
    //         data.rx_length = 6;

    //         // Send: 01 03 B005 0002
    //         data.msg[1] = 0x06;  // READ
    //         data.msg[2] = 0x00;  // B0.05 = Get RPM
    //         data.msg[3] = 0x0B;
    //         data.msg[4] = 0x00;  // Read 2 values
    //         data.msg[5] = 0x02;

    //         return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
    //             return true;
    //         };
    //     }
    //     else {
    //         return nullptr;
    //     }
    // }

    VFD::response_parser HD09::get_current_rpm(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: 01 03 700C 0002
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x33;  // B0.05 = Get RPM
        data.msg[3] = 0x10;
        data.msg[4] = 0x00;  // Read 2 values
        data.msg[5] = 0x01;

        //  Recv: 01 03 0004 095D 0000
        //                   ---- = 2397 (val #1)
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t rpm = (uint16_t(response[3]) << 8) | uint16_t(response[4]);

            // Set current RPM value? Somewhere?
            vfd->_sync_rpm = rpm * 0.01001;
            return true;
        };
    }

    VFD::response_parser HD09::get_current_direction(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // Send: 01 03 30 00 00 01
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x33;  // Command group ID
        data.msg[3] = 0x0A;
        data.msg[4] = 0x00;  // Message ID
        data.msg[5] = 0x01;

        // Receive: 01 03 00 02 00 02
        //                      ----- status

        // TODO: What are we going to do with this? Update sys.spindle_speed? Update vfd state?
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint8_t uData = response[3];
            if ((uData & (0x01 << 1)) == 0)
                vfd->_current_state = SpindleState::Disable;
            if ((uData & (0x01 << 2)) != 0)
                vfd->_current_state = SpindleState::Cw;
            else
                vfd->_current_state = SpindleState::Ccw;
            return true;
        };
    }
}
