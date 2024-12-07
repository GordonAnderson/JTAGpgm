//
// JTAGpgm
//
// Adafruit Trinket M0
//
// Implements a jtag uploader.
//
// Gordon Anderson
// January 1, 2021
//
// Version 1.0, January 1, 2021
//  - First release
//
#include <Arduino.h>
#include <variant.h>
#include <wiring_private.h>
#include "SERCOM.h"
#include <Thread.h>
#include <ThreadController.h>
#include <Adafruit_DotStar.h>
#include <PlayXSVFJTAGArduino.h>

#include <Wire.h>
#include <SPI.h>
#include "Hardware.h"
#include "Errors.h"
#include "Serial.h"

const char   Version[] PROGMEM = "JTAGpgm version 1.0, January 1, 2021";

Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

int TimeCounter = 0;

// ThreadController that will control all threads
ThreadController control = ThreadController();
//Threads
Thread SystemThread = Thread();

extern void (*mySysTickHook)(void);
void msTimerIntercept(void)
{

}
void (*mySysTickHook)(void) = msTimerIntercept;

void setup() 
{    
  // Turn off the Pixel
  strip.begin();
  strip.setPixelColor(0, 0, 0, 0);
  strip.show();
  // Init serial communications
  SerialInit();
  // Configure Threads
  SystemThread.setName("Update");
  SystemThread.onRun(Update);
  SystemThread.setInterval(25);
  // Add threads to the controller
  control.add(&SystemThread);
  // Print the signon version message
  serial->println(Version);
}

// This function is called at 40 Hz
void Update(void)
{
  static int i = 40;

  if(i-- == 0) // Flash the led to let everyone know we are alive!
  {
    i = 40;
    if(digitalRead(13) == LOW) digitalWrite(13,HIGH);
    else digitalWrite(13,LOW);
  }
  if(TimeCounter > 0) 
  {
    TimeCounter--;
    if(TimeCounter == 0) RB_Init(&RB);
  }
}

// This function process all the serial IO and commands
void ProcessSerial(bool scan = true)
{
  // Put serial received characters in the input ring buffer
  if (Serial.available() > 0)
  {
    PutCh(Serial.read());
  }
  if (!scan) return;
  // If there is a command in the input ring buffer, process it!
  if (RB_Commands(&RB) > 0) while (ProcessCommand() == 0); // Process until flag that there is nothing to do
}

bool PlayFlag = false;

void loop() 
{
  ProcessSerial();
  control.run();
}

//
// Host command functions
//
void JTAGplay(void)
{
#ifndef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 64
#endif /* SERIAL_RX_BUFFER_SIZE */

  Serial.flush();
  PlayXSVFJTAGArduino p(Serial, SERIAL_RX_BUFFER_SIZE, TMS, TDI, TDO, TCK, VREF);
  p.play();
  Serial.flush();
  TimeCounter = 10;
}

void Software_Reset(void)
{
  NVIC_SystemReset();  
}

void Debug(int i)
{
}
