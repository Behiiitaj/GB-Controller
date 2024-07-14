#include "YX3000Spindle.h"


namespace Spindles {
    YX3000::YX3000() : VFD() {
        _baudrate = 9600;
        _parity   = Uart::Parity::None;
    }

    void YX3000::direction_command(SpindleState mode, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 5;
        data.rx_length = 5;

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x20;  // Command ID 0x2000
        data.msg[3] = 0x00;
        data.msg[4] = (mode == SpindleState::Ccw) ? 0x02 : (mode == SpindleState::Cw ? 0x01 : 0x08);
    }

    void YX3000::set_speed_command(uint32_t rpm, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // We have to know the max RPM before we can set the current RPM:
        auto max_rpm = this->_max_rpm;

        // Speed is in [0..10'000] where 10'000 = 100%.
        // We have to use a 32-bit integer here; typical values are 10k/24k rpm.
        // I've never seen a 400K RPM spindle in my life, and they aren't supported
        // by this modbus protocol anyways... So I guess this is OK.
        uint16_t speed = (uint32_t(rpm) * 10000L) / uint32_t(max_rpm);
        if (speed < 0) {
            speed = 0;
        }
        if (speed > 10000) {
            speed = 10000;
        }

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x20;  // Command ID 0x1000
        data.msg[3] = 0x01;
        data.msg[4] = uint8_t(speed >> 8);  // RPM
        data.msg[5] = uint8_t(speed & 0xFF);
    }

    VFD::response_parser YX3000::get_current_rpm(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 4;
        data.rx_length = 5;

        data.msg[1] = 0x03;  
        data.msg[2] = 0x21;  
        data.msg[3] = 0x03;
        // data.msg[4] = 0x00;  
        // data.msg[5] = 0x01;

        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t rpm = (uint16_t(response[3]) << 8) | uint16_t(response[4]);

            vfd->_sync_rpm = rpm;
            return true;
        };
    }

    VFD::response_parser YX3000::get_current_direction(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 4;
        data.rx_length = 4;

        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x21;  // Command group ID
        data.msg[3] = 0x0C;

        // TODO: What are we going to do with this? Update sys.spindle_speed? Update vfd state?
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool { 
		    uint8_t uData = response[3];
            if((uData & (0x01<<1)) == 0)vfd->_current_state = SpindleState::Disable;
            if((uData & (0x01<<2)) != 0)vfd->_current_state = SpindleState::Cw;
            else vfd->_current_state = SpindleState::Ccw;
            return true; 
        };
    }
}
