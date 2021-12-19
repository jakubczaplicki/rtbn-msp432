//***********************  main.c  ***********************
// Program written by:
// - Steven Prickett  steven.prickett@gmail.com
//
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//
//*********************************************************
/* Modified by Jonathan Valvano, Daniel Valvano
   October 12, 2015

 */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "..\inc\msp432p401r.h"

#include "ClockSystem.h"
#include "UART.h"
#include "esp8266.h"
#include "LED.h"

// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

char Fetch[] = "GET /data/2.5/weather?q=Austin%20Texas&APPID=1234567890abcdef1234567890abcdef HTTP/1.1\r\nHost:api.openweathermap.org\r\n\r\n";
// 1) go to http://openweathermap.org/appid#use
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID
int main(void){
  DisableInterrupts(); // interrupts enabled in ESP8266_Init()
  Clock_Init48MHz();
  LED_Init();
  Output_Init();       // eUSCIA0 only used for debugging
  printf("\n\r-----------\n\rSystem starting...\n\r");
  ESP8266_Init();      // connect to access point, set up as client
  ESP8266_GetVersionNumber();
  while(1){
    ESP8266_GetStatus();
    if(ESP8266_MakeTCPConnection("openweathermap.org")){ // open socket in server
      LED_GreenOn();
      ESP8266_SendTCP(Fetch); // result in ServerResponseBuffer
    }
    ESP8266_CloseTCPConnection();
    while(Board_Input()==0){// wait for touch
    };
    LED_GreenOff();
    LED_RedToggle();
  }
}
