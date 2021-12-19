// esp8266.c
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//
/*
  Author: Steven Prickett (steven.prickett@gmail.com)

  THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
  OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
  VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
  OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

*/

// NOTE: ESP8266 resources below:
// General info and AT commands: http://nurdspace.nl/ESP8266
// General info and AT commands: http://www.electrodragon.com/w/Wi07c
// Community forum: http://www.esp8266.com/
// Offical forum: http://bbs.espressif.com/
// example http://zeflo.com/2014/esp8266-weather-display/

/* Modified by Jonathan Valvano, Daniel Valvano
 October 12, 2015
 Hardware connections
 Vcc is a separate regulated 3.3V supply with at least 215mA
 /------------------------------\
 |              chip      1   8 |
 | Ant                    2   7 |
 | enna       processor   3   6 |
 |                        4   5 |
 \------------------------------/
ESP8266    MSP432
  1 URxD    P3.3  UART out of MSP432, 115200 baud
  2 GPIO0         +3.3V for normal operation (ground to flash)
  3 GPIO2         +3.3V
  4 GND     Gnd   GND (70mA)
  5 UTxD    P3.2  UART out of ESP8266, 115200 baud
  6 Ch_PD         chip select, 10k resistor to 3.3V
  7 Reset   P6.0  MSP432 can issue output low to cause hardware reset
  8 Vcc           regulated 3.3V supply with at least 70mA
 */

/*
===========================================================
==========          CONSTANTS                    ==========
===========================================================
*/
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "..\inc\msp432p401r.h"
#include "esp8266.h"
#include "UART.h"
// Access point parameters
#define SSID_NAME  "ValvanoAP"
#define PASSKEY    "12345678"
//#define SEC_TYPE   ESP8266_ENCRYPT_MODE_WPA2_PSK
#define BUFFER_SIZE 1024
#define MAXTRY 10
// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
// prototypes for helper functions
void DelayMs(uint32_t delay);
void ESP8266ProcessInput(const char* buffer);
void ESP8266HandleInputCommands(const char* input);
void ESP8266ParseSettingPointers(char* timePtr, char* shotsPtr, char* distPtr);
void ESP8266InitUART(void);
void ESP8266PrintChar(char iput);
void ESP8266EnableRXInterrupt(void);
void ESP8266DisableRXInterrupt(void);
void ESP8266SendCommand(const char* inputString);
void ESP8266FIFOtoBuffer(void);


/*
=============================================================
==========            GLOBAL VARIABLES             ==========
=============================================================
*/

uint32_t RXBufferIndex = 0;
uint32_t LastReturnIndex = 0;
uint32_t CurrentReturnIndex = 0;
char RXBuffer[BUFFER_SIZE];
char TXBuffer[BUFFER_SIZE];
#define SERVER_RESPONSE_SIZE 1024
char ServerResponseBuffer[SERVER_RESPONSE_SIZE]; // characters after +IPD,
uint32_t ServerResponseIndex = 0;

volatile bool ESP8266_ProcessBuffer = false;
volatile bool ESP8266_EchoResponse = true;
volatile bool ESP8266_ResponseComplete = false;
volatile bool ESP8266_APEnabled = false;
volatile bool ESP8266_ClientEnabled = false;
volatile bool ESP8266_ServerEnabled = false;
volatile bool ESP8266_InputProcessingEnabled = false;
volatile bool ESP8266_PageRequested = false;

/*
=======================================================================
==========              search FUNCTIONS                     ==========
=======================================================================
*/
char SearchString[32];
volatile bool SearchLooking = false;
volatile bool SearchFound = false;
volatile uint32_t SearchIndex = 0;
//-------------------SearchStart -------------------
// - start looking for string in received data stream
// Inputs: none
// Outputs: none
void SearchStart(char *pt){
  strcpy(SearchString,pt);
  SearchIndex = 0;
  SearchFound = false;
  SearchLooking = true;
}
//-------------------SearchCheck -------------------
// - start looking for string in received data stream
// Inputs: none
// Outputs: none
void SearchCheck(char letter){
  if(SearchLooking){
    if(SearchString[SearchIndex] == letter){ // match letter?
      SearchIndex++;
      if(SearchString[SearchIndex] == 0){ // match string?
        SearchFound = true;
        SearchLooking = false;
      }
    }else{
      SearchIndex = 0; // start over
    }
  }
}

char ServerResponseSearchString[16]="+IPD,";
volatile uint32_t ServerResponseSearchFinished = false;
volatile uint32_t ServerResponseSearchIndex = 0;
volatile uint32_t ServerResponseSearchLooking = 0;

//-------------------ServerResponseSearchStart -------------------
// - start looking for server response string in received data stream
// Inputs: none
// Outputs: none
void ServerResponseSearchStart(void){
  strcpy(ServerResponseSearchString,"+IPD,");
  ServerResponseSearchIndex = 0;
  ServerResponseSearchLooking = 1; // means look for "+IPD"
  ServerResponseSearchFinished = 0;
  ServerResponseIndex = 0;
}

//-------------------ServerResponseSearchCheck -------------------
// - start looking for string in received data stream
// Inputs: none
// Outputs: none
void ServerResponseSearchCheck(char letter){
  if(ServerResponseSearchLooking==1){
    if(ServerResponseSearchString[ServerResponseSearchIndex] == letter){ // match letter?
      ServerResponseSearchIndex++;
      if(ServerResponseSearchString[ServerResponseSearchIndex] == 0){ // match string?
        ServerResponseSearchLooking = 2;
        strcpy(ServerResponseSearchString,"\r\nOK\r\n");
        ServerResponseSearchIndex = 0;
      }
    }else{
      ServerResponseSearchIndex = 0; // start over
    }
  }else if(ServerResponseSearchLooking==2){
    if(ServerResponseIndex < SERVER_RESPONSE_SIZE){
      ServerResponseBuffer[ServerResponseIndex] = letter; // copy data from "+IPD," to "OK"
      ServerResponseIndex++;
    }
    if(ServerResponseSearchString[ServerResponseSearchIndex] == letter){ // match letter?
      ServerResponseSearchIndex++;
      if(ServerResponseSearchString[ServerResponseSearchIndex] == 0){   // match OK string?
        ServerResponseSearchFinished = 1;
        ServerResponseSearchLooking = 0;
      }
    }else{
      ServerResponseSearchIndex = 0; // start over
    }
  }
}
/*
=======================================================================
==========       eUSCIA2 and private FUNCTIONS               ==========
=======================================================================
*/

//------------------- ESP8266InitUART-------------------
// - intializes uart and gpio needed to communicate with esp8266
// Configure eUSCIA2 for 115200bps operation
// Inputs: none
// Outputs: none
// Assumes: 12,000,000 Hz SMCLK (such as after Clock_Init48MHz() called)
#define RESET     (*((volatile uint8_t *)(0x42000000+32*0x4C43+4*0)))  /* Port 6.0 Output */
void ESP8266InitUART(void){ volatile int delay;
  UCA2CTLW0 = 0x0001;                   // hold the USCI module in reset mode
  // bit15=0,      no parity bits
  // bit14=x,      not used when parity is disabled
  // bit13=0,      LSB first
  // bit12=0,      8-bit data length
  // bit11=0,      1 stop bit
  // bits10-8=000, asynchronous UART mode
  // bits7-6=11,   clock source to SMCLK
  // bit5=0,       reject erroneous characters and do not set flag
  // bit4=0,       do not set flag for break characters
  // bit3=0,       not dormant
  // bit2=0,       transmit data, not address (not used here)
  // bit1=0,       do not transmit break (not used here)
  // bit0=1,       hold logic in reset state while configuring
  UCA2CTLW0 = 0x00C1;
                                        // set the baud rate
                                        // N = clock/baud rate = 12,000,000/115,200 = 104.167
  UCA2BRW = 104;                        // UCBR = baud rate = int(N) = 104
  UCA2MCTLW &= ~0xFFF1;                 // clear first and second modulation stage bit fields
                                        // configure second modulation stage select (from Table 22-4 on p731 of datasheet)
//  UCA2MCTLW |= (0x11<<8);               // UCBRS = N - int(N) = 0.167; plug this in Table 22-4
                                        // configure first modulation stage select (ignored when oversampling disabled)
//  UCA2MCTLW |= (8<<4);                  // UCBRF = int(((N/16) - int(N/16))*16) = 0
//  UCA2MCTLW |= 0x0001;                  // enable oversampling mode
  P3SEL0 |= 0x0C;
  P3SEL1 &= ~0x0C;                      // configure P3.3 and P3.2 as primary module function
  P6SEL0 &= ~0x01;
  P6SEL1 &= ~0x01;                      // configure P6.0 as GPIO
  P6DIR |= 0x01;                        // make P6.0 out
  RESET = 0x01;                         // reset high
  NVIC_IPR4 = (NVIC_IPR4&0xFF00FFFF)|0x00400000; // priority 2
  NVIC_ISER0 = 0x00040000;              // enable interrupt 18 in NVIC
  UCA2CTLW0 &= ~0x0001;                 // enable the USCI module
                                        // enable interrupts on receive full
  UCA2IE = 0x0001;                      // disable interrupts on transmit empty, start, complete
}
// -----------EUSCIA2_IRQHandler-----------
// called when UCA2RXBUF has received a complete character
void EUSCIA2_IRQHandler(void){
  ESP8266FIFOtoBuffer();                // reading from UCAxRXBUF acknowledges interrupt
}
//--------ESP8266EnableRXInterrupt--------
// - enables uart rx interrupt
// Inputs: none
// Outputs: none
// Assumes: eUSCIA2 is active (bit 0 of UCA2CTLW0 == 0)
void ESP8266EnableRXInterrupt(void){
                                        // enable interrupts on receive full
  UCA2IE = 0x0001;                      // disable interrupts on transmit empty, start, complete
  NVIC_ISER0 = 1<<18;                   // interrupt 18
}

//--------ESP8266DisableRXInterrupt--------
// - disables uart rx interrupt
// Inputs: none
// Outputs: none
// Assumes: eUSCIA2 is active (bit 0 of UCA2CTLW0 == 0)
void ESP8266DisableRXInterrupt(void){
  UCA2IE = 0x0000;                      // disable interrupts on transmit empty, start, complete, and receive full
  NVIC_ICER0 = 1<<18;                   // interrupt 18
}

//--------ESP8266PrintChar--------
// - prints a character to the esp8226 via uart
// Inputs: character to transmit
// Outputs: none
void ESP8266PrintChar(char input){
  while((UCA2IFG&0x0002) == 0) {};
  UCA2TXBUF = input;
//  UCA0TXBUF = input; // non-blocking echo debugging
}
//----------ESP8266FIFOtoBuffer----------
// - copies uart fifo to RXBuffer, using a circular MACQ to store the last data
// - NOTE: would probably be better to use a software defined FIFO here
// - LastReturnIndex is index to previous \n
// Inputs: none
// Outputs:none
void ESP8266FIFOtoBuffer(void){
  char letter;
  if((UCA2IFG&0x0001) == 0x0001){
    letter = UCA2RXBUF;        // retrieve char from receive buffer
    if(ESP8266_EchoResponse){
      UCA0TXBUF = letter; // non-blocking echo
    }
    if(RXBufferIndex >= BUFFER_SIZE){
      RXBufferIndex = 0; // store last BUFFER_SIZE received
    }
    RXBuffer[RXBufferIndex] = letter; // put char into buffer
    RXBufferIndex++; // increment buffer index
    SearchCheck(letter);               // check for end of command
    ServerResponseSearchCheck(letter); // check for server response
    if(letter == '\n'){
      LastReturnIndex = CurrentReturnIndex;
      CurrentReturnIndex = RXBufferIndex;
    }
  }
}

//---------ESP8266SendCommand-----
// - sends a string to the esp8266 module
// uses busy-wait
// however, hardware has 1 character hardware buffer
// Inputs: string to send (null-terminated)
// Outputs: none
void ESP8266SendCommand(const char* inputString){
  int index = 0;
  while(inputString[index] != 0){
    ESP8266PrintChar(inputString[index++]);
  }
}

// DelayMs
//  - busy wait n milliseconds
// Input: time to wait in msec
// Outputs: none
void DelayMs(uint32_t n){
  volatile uint32_t time;
  while(n){
    time = 2997;  // 1msec, tuned at 48 MHz
    while(time){
      time--;
    }
    n--;
  }
}

void DelayMsSearching(uint32_t n){
  volatile uint32_t time;
  while(n){
    time = 1410;  // 1msec, tuned at 48 MHz
    while(time){
      time--;
      if(SearchFound) return;
    }
    n--;
  }
}

/*
=======================================================================
==========          ESP8266 PUBLIC FUNCTIONS                 ==========
=======================================================================
*/
//-------------------ESP8266_Init --------------
// initializes the module as a client
// Inputs: none
// Outputs: none
void ESP8266_Init(void){
  ESP8266InitUART();
  ESP8266EnableRXInterrupt();
  SearchLooking = false;
  SearchFound = false;
  ServerResponseSearchLooking = 0; // not looking for "+IPD"
  ServerResponseSearchFinished = 0;
  EnableInterrupts();
// step 1: AT+RST reset module
  printf("ESP8266 Initialization:\n\r");
  if(ESP8266_Reset()==0){
    printf("Reset failure, could not reset\n\r"); while(1){};
  }
// step 2: AT+CWMODE=1 set wifi mode to client (not an access point)
  if(ESP8266_SetWifiMode(ESP8266_WIFI_MODE_CLIENT)==0){
    printf("SetWifiMode, could not set mode\n\r"); while(1){};
  }
// step 3: AT+CWJAP="ValvanoAP","12345678"  connect to access point
  if(ESP8266_JoinAccessPoint(SSID_NAME,PASSKEY)==0){
    printf("JoinAccessPoint error, could not join AP\n\r"); while(1){};
  }
// optional step: AT+CIFSR check to see our IP address
  if(ESP8266_GetIPAddress()==0){ // data streamed to UART0, OK
    printf("GetIPAddress error, could not get IP address\n\r"); while(1){};
  }
//// optional step: AT+CIPMUX==0 set mode to single socket
//  if(ESP8266_SetConnectionMux(0)==0){ // single socket
//    printf("SetConnectionMux error, could not set connection mux\n\r"); while(1){};
//  }
// optional step: AT+CWLAP check to see other AP in area
  if(ESP8266_ListAccessPoints()==0){
    printf("ListAccessPoints, could not list access points\n\r"); while(1){};
  }
// step 4: AT+CIPMODE=0 set mode to not data mode
  if(ESP8266_SetDataTransmissionMode(0)==0){
    printf("SetDataTransmissionMode, could not make connection\n\r"); while(1){};
  }
  ESP8266_InputProcessingEnabled = false; // not a server
}

//----------ESP8266_Reset------------
// resets the esp8266 module
// input:  none
// output: 1 if success, 0 if fail
int ESP8266_Reset(){int try=MAXTRY;
  SearchStart("System Ready");
  while(try){
    RESET = 0x00;                  // reset low
    DelayMs(10);
    RESET = 0x01;                  // reset high
    ESP8266SendCommand("AT+RST\r\n");
    DelayMsSearching(500);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//---------ESP8266_SetWifiMode----------
// configures the esp8266 to operate as a wifi client, access point, or both
// since it searches for "no change" it will execute twice when changing modes
// Input: mode accepts ESP8266_WIFI_MODE constants
// output: 1 if success, 0 if fail
int ESP8266_SetWifiMode(uint8_t mode){
  int try=MAXTRY;
  if(mode > ESP8266_WIFI_MODE_AP_AND_CLIENT)return 0; // fail
  SearchStart("no change");
  while(try){
    sprintf((char*)TXBuffer, "AT+CWMODE=%d\r\n", mode);
    ESP8266SendCommand((const char*)TXBuffer);
    DelayMsSearching(5000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//---------ESP8266_SetConnectionMux----------
// enables the esp8266 connection mux, required for starting tcp server
// Input: 0 (single) or 1 (multiple)
// output: 1 if success, 0 if fail
int ESP8266_SetConnectionMux(uint8_t enabled){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    sprintf((char*)TXBuffer, "AT+CIPMUX=%d\r\n", enabled);
    ESP8266SendCommand((const char*)TXBuffer);
    DelayMsSearching(5000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//----------ESP8266_JoinAccessPoint------------
// joins a wifi access point using specified ssid and password
// input:  SSID and PASSWORD
// output: 1 if success, 0 if fail
int ESP8266_JoinAccessPoint(const char* ssid, const char* password){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    sprintf((char*)TXBuffer, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    ESP8266SendCommand((const char*)TXBuffer);
    DelayMsSearching(4000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//---------ESP8266_ListAccessPoints----------
// lists available wifi access points
// Input: none
// output: 1 if success, 0 if fail
int ESP8266_ListAccessPoints(void){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    ESP8266SendCommand("AT+CWLAP\r\n");
    DelayMsSearching(8000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

// ----------ESP8266_QuitAccessPoint-------------
// - disconnects from currently connected wifi access point
// Inputs: none
// Outputs: 1 if success, 0 if fail
int ESP8266_QuitAccessPoint(void){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    ESP8266SendCommand("AT+CWQAP\r\n");
    DelayMsSearching(8000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//----------ESP8266_ConfigureAccessPoint------------
// configures esp8266 wifi access point settings
// use this function only when in AP mode (and not in client mode)
// input:  SSID, Password, channel, security
// output: 1 if success, 0 if fail
int ESP8266_ConfigureAccessPoint(const char* ssid, const char* password, uint8_t channel, uint8_t encryptMode){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    sprintf((char*)TXBuffer, "AT+CWSAP=\"%s\",\"%s\",%d,%d\r\n", ssid, password, channel, encryptMode);
    ESP8266SendCommand((const char*)TXBuffer);
    DelayMsSearching(4000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//---------ESP8266_GetIPAddress----------
// Get local IP address
// Input: none
// output: 1 if success, 0 if fail
int ESP8266_GetIPAddress(void){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    ESP8266SendCommand("AT+CIFSR\r\n");
    DelayMsSearching(5000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//---------ESP8266_MakeTCPConnection----------
// Establish TCP connection
// Input: IP address or web page as a string
// output: 1 if success, 0 if fail
int ESP8266_MakeTCPConnection(char *IPaddress){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    sprintf((char*)TXBuffer, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n", IPaddress);
    ESP8266SendCommand(TXBuffer);   // open and connect to a socket
    DelayMsSearching(8000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//---------ESP8266_SendTCP----------
// Send a TCP packet to server
// Input: TCP payload to send
// output: 1 if success, 0 if fail
int ESP8266_SendTCP(char* fetch){
  volatile uint32_t time,n;
  sprintf((char*)TXBuffer, "AT+CIPSEND=%d\r\n", strlen(fetch));
  ESP8266SendCommand(TXBuffer);
  DelayMs(5);
  ESP8266SendCommand(fetch);
  ServerResponseSearchStart();
  n = 8000;
  while(n&&(ServerResponseSearchFinished==0)){
    time = (75825*8)/91;  // 1msec, tuned at 80 MHz
    while(time){
      time--;
    }
    n--;
  }
  if(ServerResponseSearchFinished==0) return 0; // no response
  return 1; // success
}

//---------ESP8266_CloseTCPConnection----------
// Close TCP connection
// Input: none
// output: 1 if success, 0 if fail
int ESP8266_CloseTCPConnection(void){
  int try=1;
  SearchStart("OK");
  while(try){
    ESP8266SendCommand("AT+CIPCLOSE\r\n");
    DelayMsSearching(4000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}
//---------ESP8266_SetDataTransmissionMode----------
// set data transmission mode
// Input: 0 not data mode, 1 data mode; return "Link is builded"
// output: 1 if success, 0 if fail
int ESP8266_SetDataTransmissionMode(uint8_t mode){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    sprintf((char*)TXBuffer, "AT+CIPMODE=%d\r\n", mode);
    ESP8266SendCommand((const char*)TXBuffer);
    DelayMsSearching(5000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//---------ESP8266_GetStatus----------
// get status
// Input: none
// output: 1 if success, 0 if fail
int ESP8266_GetStatus(void){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    ESP8266SendCommand("AT+CIPSTATUS\r\n");
    DelayMsSearching(5000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//---------ESP8266_GetVersionNumber----------
// get status
// Input: none
// output: 1 if success, 0 if fail
int ESP8266_GetVersionNumber(void){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    ESP8266SendCommand("AT+GMR\r\n");
    DelayMsSearching(500);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}
// Program written by:
// - Steven Prickett  steven.prickett@gmail.com
//
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi access
//   point with a http server (IP = ...) serving a webpage
//   that allows the user to enter a message to send to the
//   microcontroller. The message will then be relayed back
//   over the USB UART.
//
//*********************************************************
/*
  This work is copyright Steven Prickett (steven.prickett@gmail.com) and
  licensed under a Creative Commons Attribution 3.0 Unported License,
  available at:

  https://creativecommons.org/licenses/by/3.0/

  THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
  OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
  VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
  OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*/
uint16_t ESP8266_ServerPort = 80;
uint16_t ESP8266_ServerTimeout = 28800;
// defines the form served by the http server

// ----------ESP8266_SetServerTimeout--------------
// - sets connection timeout for tcp server, 0-28800 seconds
// ***Prickett code for server, not used or tested in this client project
// Inputs: timeout parameter
// Outputs: none
void ESP8266_SetServerTimeout(uint16_t timeout){
  ESP8266_ServerTimeout = timeout;
  sprintf((char*)TXBuffer, "AT+CIPSTO=%d\r\n", ESP8266_ServerTimeout);
  ESP8266SendCommand((const char*)TXBuffer);
}

// --------ESP8266_EnableServer------------------
//  - enables tcp server on specified port
// ***Prickett code for server, not used or tested in this client project
// Inputs: port number
// Outputs: none
void ESP8266_EnableServer(uint16_t port){
  ESP8266_ServerPort = port;
  sprintf((char*)TXBuffer, "AT+CIPSERVER=1,%d\r\n", ESP8266_ServerPort);
  ESP8266SendCommand((const char*)TXBuffer);
}


//---------ESP8266_DisableServer----------
// disables tcp server
// ***Prickett code for server, not used or tested in this client project
// Input: none
// output: 1 if success, 0 if fail
int ESP8266_DisableServer(void){
  int try=MAXTRY;
  SearchStart("OK");
  while(try){
    sprintf((char*)TXBuffer, "AT+CIPSERVER=0,%d\r\n", ESP8266_ServerPort);
    ESP8266SendCommand((const char*)TXBuffer);
    DelayMsSearching(4000);
    if(SearchFound) return 1; // success
    try--;
  }
  return 0; // fail
}

//-------------ESP8266ProcessInput--------
// - parses whether we're interested in the data that just came in or not
// ***Prickett code for server, not used or tested in this client project
// Input: buffer to process
// output: none
void ESP8266ProcessInput(const char* buffer){
  char* ptr;
  // "+IPD" indicates data coming in from IP server
  if (buffer[0] == '+' && buffer[1] == 'I' && buffer[2] == 'P' && buffer[3] == 'D' && buffer[8] != '\n'){
    ptr = (char *)buffer + 7;
    while (*ptr != ':'){
      ptr++;
    }
    ptr++;

    // check for HTTP GET
    if (*ptr == 'G' && *(ptr + 1) == 'E' && *(ptr + 2) == 'T') {
      if (*(ptr + 5) == '?'){ // means data to process
        char* messagePtr = strstr(ptr, "message=") + 8;
        printf("Message from ESP8266: %s\n", messagePtr);
      }
      ESP8266_PageRequested = true;
    } else {
      // handle data that may be sent via means other than HTTP GET
    }
  }
}

const char formBody[] =
  "<!DOCTYPE html><html><body><center> \
  <h1>Enter a message to send to your microcontroller!</h1> \
  <form> \
  <input type=\"text\" name=\"message\" value=\"Hello ESP8266!\"> \
  <br><input type=\"submit\" value=\"Go!\"> \
  </form></center></body></html>";


/*
========================================================================================================================
==========                                           HTTP FUNCTIONS                                           ==========
========================================================================================================================
*/

/*
===================================================================================================
  HTTP :: HTTP_ServePage

   - constructs and sends a web page via the ESP8266 server

   - NOTE: this seems to work for sending pages to Firefox (and maybe other PC-based browsers),
           but does not seem to load properly on iPhone based Safari. May need to add some more
           data to the header.
===================================================================================================
*/
void HTTP_ServePage(const char* body){
  char header[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: ";

  char contentLength[16];
  sprintf(contentLength, "%d\r\n\r\n", strlen(body));

  sprintf((char*)TXBuffer, "AT+CIPSEND=%d,%d\r\n", 0, strlen(body) + strlen(contentLength) + strlen(header));
  ESP8266SendCommand((const char*)TXBuffer);

  DelayMs(100);

  ESP8266SendCommand(header);
  ESP8266SendCommand(contentLength);
  ESP8266SendCommand(body);
}
