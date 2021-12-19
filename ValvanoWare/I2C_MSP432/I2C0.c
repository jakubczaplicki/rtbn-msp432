// I2C0.c
// Runs on MSP432
// Provide a function that initializes, sends, and receives
// the eUSCI0B module interfaced with an HMC6352 compass or
// TMP102 thermometer.
// Daniel Valvano
// October 12, 2015

/* This example accompanies the book
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2015
   Section 8.6.4 Programs 8.5, 8.6 and 8.7

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// UCB0SCL connected to P1.7 and to pin 4 of HMC6352 compass or pin 3 of TMP102 thermometer
// UCB0SDA connected to P1.6 and to pin 3 of HMC6352 compass or pin 2 of TMP102 thermometer
// SCL and SDA lines pulled to +3.3 V with 10 k resistors (part of breakout module)
// ADD0 pin of TMP102 thermometer connected to GND

#include <stdint.h>
#include "..\inc\msp432p401r.h"

void I2C_Init(void){
  // initialize eUSCI
  UCB0CTLW0 = 0x0001;                // hold the eUSCI module in reset mode
  // configure UCB0CTLW0 for:
  // bit15      UCA10 = 0; own address is 7-bit address
  // bit14      UCSLA10 = 0; address slave with 7-bit address
  // bit13      UCMM = 0; single master environment
  // bit12      reserved
  // bit11      UCMST = 1; master mode
  // bits10-9   UCMODEx = 3; I2C mode
  // bit8       UCSYNC = 1; synchronous mode
  // bits7-6    UCSSELx = 2; eUSCI clock SMCLK
  // bit5       UCTXACK = X; transmit ACK condition in slave mode
  // bit4       UCTR = X; transmitter/receiver
  // bit3       UCTXNACK = X; transmit negative acknowledge in slave mode
  // bit2       UCTXSTP = X; transmit stop condition in master mode
  // bit1       UCTXSTT = X; transmit start condition in master mode
  // bit0       UCSWRST = 1; reset enabled
  UCB0CTLW0 = 0x0F81;
  // configure UCB0CTLW1 for:
  // bits15-9   reserved
  // bit8       UCETXINT = X; early UCTXIFG0 in slave mode
  // bits7-6    UCCLTO = 3; timeout clock low after 165,000 SYSCLK cycles
  // bit5       UCSTPNACK = 0; send negative acknowledge before stop condition in master receiver mode
  // bit4       UCSWACK = 0; slave address acknowledge controlled by hardware
  // bits3-2    UCASTPx = 2; generate stop condition automatically after UCB0TBCNT bytes
  // bits1-0    UCGLITx = 0 deglitch time of 50 ns
  UCB0CTLW1 = 0x00C8;
  UCB0TBCNT = 2;                     // generate stop condition after this many bytes
  // set the baud rate for the eUSCI which gets its clock from SMCLK
  // Clock_Init48MHz() from ClockSystem.c sets SMCLK = HFXTCLK/4 = 12 MHz
  // if the SMCLK is set to 12 MHz, divide by 120 for 100 kHz baud clock
  UCB0BRW = 120;
  P1SEL0 |= 0xC0;
  P1SEL1 &= ~0xC0;                   // configure P1.7 and P1.6 as primary module function
  UCB0CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB0IE = 0x0000;                   // disable interrupts
}

// receives one byte from specified slave
// Note for HMC6352 compass only:
// Used with 'r' and 'g' commands
// Note for TMP102 thermometer only:
// Used to read the top byte of the contents of the pointer register
//  This will work but is probably not what you want to do.
uint8_t I2C_Recv(int8_t slave){
  int8_t data1;
  while(UCB0STATW&0x0010){};         // wait for I2C ready
  UCB0CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB0TBCNT = 1;                     // generate stop condition after this many bytes
  UCB0CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB0I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB0CTLW0 = ((UCB0CTLW0&~0x0014)   // clear bit4 (UCTR) for receive mode
                                     // clear bit2 (UCTXSTP) for no transmit stop condition
                | 0x0002);           // set bit1 (UCTXSTT) for transmit start condition
  while((UCB0IFG&0x0001) == 0){      // wait for complete character received
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      I2C_Init();                    // reset to known state
      return 0xFF;
    }
  }
  data1 = UCB0RXBUF&0xFF;            // get the reply
  return data1;
}

// receives two bytes from specified slave
// Note for HMC6352 compass only:
// Used with 'A' commands
// Note for TMP102 thermometer only:
// Used to read the contents of the pointer register
uint16_t I2C_Recv2(int8_t slave){
  uint8_t data1, data2;
  while(UCB0STATW&0x0010){};         // wait for I2C ready
  UCB0CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB0TBCNT = 2;                     // generate stop condition after this many bytes
  UCB0CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB0I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB0CTLW0 = ((UCB0CTLW0&~0x0014)   // clear bit4 (UCTR) for receive mode
                                     // clear bit2 (UCTXSTP) for no transmit stop condition
                | 0x0002);           // set bit1 (UCTXSTT) for transmit start condition
  while((UCB0IFG&0x0001) == 0){      // wait for complete character received
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      I2C_Init();                    // reset to known state
      return 0xFFFF;
    }
  }
  data1 = UCB0RXBUF&0xFF;            // get the reply
  while((UCB0IFG&0x0001) == 0){      // wait for complete character received
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      I2C_Init();                    // reset to known state
      return 0xFFFF;
    }
  }
  data2 = UCB0RXBUF&0xFF;            // get the reply
  return (data1<<8)+data2;
}

// sends one byte to specified slave
// Note for HMC6352 compass only:
// Used with 'S', 'W', 'O', 'C', 'E', 'L', and 'A' commands
//  For 'A' commands, I2C_Recv2() should also be called
// Note for TMP102 thermometer only:
// Used to change the pointer register
// Returns 0 if successful, nonzero if error
uint16_t I2C_Send1(int8_t slave, uint8_t data1){
  uint16_t status;                   // save status register here in case of error
  while(UCB0STATW&0x0010){};         // wait for I2C ready
  UCB0CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB0TBCNT = 1;                     // generate stop condition after this many bytes
  UCB0CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB0I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB0CTLW0 = ((UCB0CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while(UCB0CTLW0&0x0002){};         // wait for slave address sent
  UCB0TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while(UCB0STATW&0x0010){           // wait for I2C idle
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      status = UCB0IFG;              // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return status;
    }
  }
  return 0;
}

// sends two bytes to specified slave
// Note for HMC6352 compass only:
// Used with 'r' and 'g' commands
//  For 'r' and 'g' commands, I2C_Recv() should also be called
// Note for TMP102 thermometer only:
// Used to change the top byte of the contents of the pointer register
//  This will work but is probably not what you want to do.
// Returns 0 if successful, nonzero if error
uint16_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2){
  uint16_t status;                   // save status register here in case of error
  while(UCB0STATW&0x0010){};         // wait for I2C ready
  UCB0CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB0TBCNT = 2;                     // generate stop condition after this many bytes
  UCB0CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB0I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB0CTLW0 = ((UCB0CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while(UCB0CTLW0&0x0002){};         // wait for slave address sent
  UCB0TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while((UCB0IFG&0x0002) == 0){      // wait for first data sent
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      status = UCB0IFG;              // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return status;
    }
  }
  UCB0TXBUF = data2&0xFF;            // TXBUF[7:0] is data
  while(UCB0STATW&0x0010){           // wait for I2C idle
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      status = UCB0IFG;              // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return status;
    }
  }
  return 0;
}

// sends three bytes to specified slave
// Note for HMC6352 compass only:
// Used with 'w' and 'G' commands
// Note for TMP102 thermometer only:
// Used to change the contents of the pointer register
// Returns 0 if successful, nonzero if error
uint16_t I2C_Send3(int8_t slave, uint8_t data1, uint8_t data2, uint8_t data3){
  uint16_t status;                   // save status register here in case of error
  while(UCB0STATW&0x0010){};         // wait for I2C ready
  UCB0CTLW0 |= 0x0001;               // hold the eUSCI module in reset mode
  UCB0TBCNT = 3;                     // generate stop condition after this many bytes
  UCB0CTLW0 &= ~0x0001;              // enable eUSCI module
  UCB0I2CSA = slave;                 // I2CCSA[6:0] is slave address
  UCB0CTLW0 = ((UCB0CTLW0&~0x0004)   // clear bit2 (UCTXSTP) for no transmit stop condition
                                     // set bit1 (UCTXSTT) for transmit start condition
                | 0x0012);           // set bit4 (UCTR) for transmit mode
  while((UCB0IFG&0x0002) == 0){};    // wait for slave address sent
  UCB0TXBUF = data1&0xFF;            // TXBUF[7:0] is data
  while((UCB0IFG&0x0002) == 0){      // wait for first data sent
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      status = UCB0IFG;              // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return status;
    }
  }
  UCB0TXBUF = data2&0xFF;            // TXBUF[7:0] is data
  while((UCB0IFG&0x0002) == 0){      // wait for second data sent
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      status = UCB0IFG;              // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return status;
    }
  }
  UCB0TXBUF = data3&0xFF;            // TXBUF[7:0] is data
  while(UCB0STATW&0x0010){           // wait for I2C idle
    if(UCB0IFG&0x0030){              // bit5 set on not-acknowledge; bit4 set on arbitration lost
      status = UCB0IFG;              // snapshot flag register for calling program
      I2C_Init();                    // reset to known state
      return status;
    }
  }
  return 0;
}
