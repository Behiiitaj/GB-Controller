#pragma once
// clang-format off

/*
    4axis_external_driver.h
    Part of Grbl_ESP32

    Pin assignments for the buildlog.net 4-axis external driver board
    https://github.com/bdring/4_Axis_SPI_CNC

    2018    - Bart Dring
    2020    - Mitch Bradley

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#define MACHINE_NAME            "External 4 Axis Driver Board V2"

#ifdef N_AXIS
        #undef N_AXIS
#endif


// I2S (steppers & other output-only pins)
// #define USE_I2S_OUT
//#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC

// #define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

// #define I2S_OUT_BCK      GPIO_NUM_27
// #define I2S_OUT_WS       GPIO_NUM_14
// #define I2S_OUT_DATA     GPIO_NUM_12

// You can override these by defining them in a board file.
// To override, you must set all of them
//-1 means use the default board pin
#    define GRBL_SPI_SS 5
#    define GRBL_SPI_MOSI 23
#    define GRBL_SPI_MISO 19
#    define GRBL_SPI_SCK 18
#    define GRBL_SPI_FREQ 4000000


#define N_AXIS 4

#define X_STEP_PIN              GPIO_NUM_32
#define X_DIRECTION_PIN         GPIO_NUM_33
#define Y_STEP_PIN              GPIO_NUM_25
#define Y_DIRECTION_PIN         GPIO_NUM_26
#define Z_STEP_PIN              GPIO_NUM_27
#define Z_DIRECTION_PIN         GPIO_NUM_14
#define A_STEP_PIN              GPIO_NUM_13
#define A_DIRECTION_PIN         GPIO_NUM_12
//#define STEPPERS_DISABLE_PIN    GPIO_NUM_13



//#define SPINDLE_TYPE            SpindleType::H2A // only one spindle at a time
//#define VFD_RS485_TXD_PIN		GPIO_NUM_17
//#define VFD_RS485_RXD_PIN		GPIO_NUM_16     
//#define VFD_RS485_RTS_PIN		GPIO_NUM_4
//#define VFD_RS485_ADDR            0x02
//#define VFD_RS485_BAUD_RATE       9600
#define COOLANT_MIST_PIN        GPIO_NUM_21  // labeled Mist
// #define COOLANT_FLOOD_PIN       GPIO_NUM_27

// #define USER_DIGITAL_PIN_0      I2SO(5)


#define X_LIMIT_PIN             GPIO_NUM_36
#define Y_LIMIT_PIN             GPIO_NUM_39
#define Z_LIMIT_PIN             GPIO_NUM_34
//#define MACRO_BUTTON_0_PIN      GPIO_NUM_22


// #define PROBE_PIN               GPIO_NUM_15
// #define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_22



#if (N_AXIS != 3)
        #define A_LIMIT_PIN     GPIO_NUM_35
#endif

