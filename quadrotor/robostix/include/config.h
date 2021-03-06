/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   config.h    Definition of global configuration settings.                */
/*                                                                           */
/*   Copyright (c) 2006-2013 Rainer Trummer <rainer.trummer@gmail.com>       */
/*                                                                           */
/*   This program is free software; you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by    */
/*   the Free Software Foundation; either version 2 of the License, or       */
/*   (at your option) any later version.                                     */
/*                                                                           */
/*   This program is distributed in the hope that it will be useful,         */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*   GNU General Public License for more details.                            */
/*                                                                           */
/*   You should have received a copy of the GNU General Public License       */
/*   along with this program; if not, write to the Free Software Foundation, */
/*   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.      */
/*                                                                           */
/*****************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>


/*****************************************************************************/
/*                                                                           */
/*   Configuration Settings                                                  */
/*                                                                           */
/*****************************************************************************/

/* Max number of timer events that can be added to the timer event list
*/
#define MAX_TIMER_EVENTS    1       /* should be kept to the minimum required */

/* Periods to wait before interacting when not receiving motor signals anymore
*/
#define PERIODS_TO_WAIT     10      /* should not be less than 10 periods */

/* Period for setting a notify flag (watchdog timer counts internally in 10ms)
*/
#define NOTIFY_PERIOD       100     /* [ms] should be a multiple of 10 */

/* Value for decrementing the motor signals when a loss of connectivity occurs
*/
#define MOTOR_DEC           5       /* should be adapted to the PWM mode used */

/* Selection of UART channel used for communication with the Gumstix
*/
#define UART_SERIAL         0       /* must be either 0 for UART0 or 1 for UART1 */

/* Selection of UART channel used for the 3DM-GX1 inertial measurement unit
*/
#define UART_DM3GX1         1       /* must be either 0 for UART0 or 1 for UART1 */

/* Definitions for establishing a parallel port between Robostix 1 and Robostix 2
*/
#define PP_DATA_PORT        PORTC   /* port exploited for parallel communication */
#define PP_DATA_DDR         DDRC    /* data direction register associated with port */
#define PP_DATA_REG         PINC    /* data register associated with parallel port */
#define PP_CTRL_PORT        PORTE   /* port exploited for parallel port interrupts */
#define PP_CTRL_DDR         DDRE    /* data direction register associated with port */
#define PP_CMD_RDY          INT6    /* port pin where the RDY command connects to */
#define PP_INT_RDY          INT7    /* port pin where the RDY interrupt connects to */
#define PP_ISC_RDY          ISC70   /* EICRB configuration setting for RDY interrupt */
#define PP_SIG_RDY          SIG_INTERRUPT7  /* interrupt signal associated with RDY */

/* Analog input channels assigned to the Analog-to-Digital Converter
*/
#define ADC_CH_SONAR        PF2//7     /* pin where the sonar signal connects to */
#define ADC_CH_BATT         PF3//6     /* pin where the battery signal connects to */

/* Port, data direction register, and pins used for the BMU 09-A sensor
*/
#define BMU09A_PORT         PORTA   /* port exploited for BMU chip-select signals */
#define BMU09A_DDR          DDRA    /* data direction register associated with port */
#define BMU09A_REG          PINA    /* control register associated with port pins */
#define BMU09A_BSY          PA0     /* pin where the BMU's BUSY signal connects to */
#define BMU09A_PCS          PA1     /* pin where the pressure CS signal connects to */
#define BMU09A_TCS          PA2     /* pin where the temperature CS signal connects to */
#define BMU09A_BCS          PA3     /* pin where the battery CS signal connects to */
#define BMU09A_110          0       /* enables a sampling rate of 110Hz */
#define BMU09A_55           0       /* enables a sampling rate of 55Hz */
#define BMU09A_27           1       /* enables a sampling rate of 27.5Hz */
#define BMU09A_13           0       /* enables a sampling rate of 13.8Hz */
#define BMU09A_6            0       /* enables a sampling rate of 6.9Hz */

/* Port, data direction register, and pin used for the Mini-A sonar sensor
*/
#define MINIA_PORT          PORTC//A   /* port where the trigger signal is provided */
#define MINIA_DDR           DDRC//A    /* data direction register associated with port */
#define MINIA_REG           PINC//A    /* control register associated with port pins */
#define MINIA_TRIGGER       PC0//A7     /* pin where the trigger signal connects to */
#define MINIA_PWM_CLK       PC1//A7     /* pin where the PWM clock signal connects to */

/* Default, minimum, and maximum address settings for the SRF10 sonar sensor
*/
#define SRF10_DEF_ADDR      0xE0    /* default SRF10 sensor address */
#define SRF10_MIN_ADDR      0xE0    /* lowest addmissible address */
#define SRF10_MAX_ADDR      0xFE    /* highest addmissible address */

/* Enables/disables the Fast PWM Mode (not suitable for standard R/C servos)
*/
#define FAST_PWM_MODE       1       /* must be either 0 to disable or 1 to enable */

/* Selection of the Robostix Expension Board version
*/
#define ROBOSTIX_R341       0       /* must be either 0 to disable or 1 to enable */
#define ROBOSTIX_R790       0       /* must be either 0 to disable or 1 to enable */
#define ROBOSTIX_R1131      1       /* must be either 0 to disable or 1 to enable */

/* Robostix PWM Connecting Positions

    +-----------+           +-----------+
    |         3A| -----     |         3A| front
    |         3B| front     |         3B| right
    |         3C| right     |         3C| -----
    |         1A| -----     |         1A| -----
    |         1B| rear      |         1B| rear
    |         1C| left      |         1C| left
    |    TOP    |           |    TOP    |
    |           |           |           | 
    |           |           |           | 
    |           |           |           | 
    |   R341    |           |   R1131   | 
    |   R790    |           |           | 
    |           |           |           |
    +-----------+           +-----------+
*/

/* BMU 09-A Interface

    Pin     I/O     Description                 Color
    --------------------------------------------------
    VCC     Pwr     Positive supply voltage     red
    VSS     Pwr     Negative supply voltage     black
    GND     Pwr     Ground for interface        brown
    SCK     In      Serial Clock (SCK)          white
    SDO     Out     Serial Data Out (MISO)      orange
    SDI     In      Serial Data In (MOSI)       white
    BSY     Out     Busy (from Pressure ADC)    orange
    PCS     In      Pressure Chip Select        white
    TCS     In      Temperature Chip Select     white
    BCS     In      Battery Chip Select         white
*/

/* BMU 09-A Connector (plug-in side)

     SCK SDO SDI GND VSS
    +---+---+---+---+---+
    | o | o | o | o | o |
    +---+---+---+---+---+
    | o | o | o | o | o |
    +---+---+---+---+---+
     BSY PCS TCS BCS VCC
*/

/* Robostix BMU Interface (top side)

           S S S B P T B
           C D D S C C C
           K O I Y S S S
           _____________________
     _____|o_o_o_o_o_o_o_o_o_o_o|____
    |o o o|o o o|o o o o o o o o|o o
    |o o o|o o o|o o o o o o o o|o o
    |o o o|o o o|o o o o o o o o|o o
    |--------------------------------
    |PWM_3 PWM_1 0  A/D PORT   7
    |
*/

#endif /* !CONFIG_H */

/* End of file */
