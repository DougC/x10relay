/* Arduino Interface to the PSC05 X10 Receiver.                       BroHogan 3/24/09
 * SETUP: X10 PSC05/TW523 RJ11 to Arduino (timing for 60Hz)
 * - RJ11 pin 1 (BLK) -> Pin 2 (Interrupt 0) = Zero Crossing
 * - RJ11 pin 2 (RED) -> GND
 * - RJ11 pin 3 (GRN) -> Pin 4 = Arduino receive
 * - RJ11 pin 4 (YEL) -> Pin 5 = Arduino transmit (via X10 Lib)
 * NOTES:
 * - Must detach interrup when transmitting with X10 Lib 
 */

const unsigned int RELAY_PIN = 6;
const unsigned int BAUD_RATE = 9600;

#include "Arduino.h"                  // this is needed to compile with Rel. 0013
#include <x10.h>                       // X10 lib is used for transmitting X10
#include <x10constants.h>              // X10 Lib constants
#define RPT_SEND 1 

#define ZCROSS_PIN     2               // BLK pin 1 of PSC05
#define RCVE_PIN       4               // GRN pin 3 of PSC05+++
#define TRANS_PIN      5               // YEL pin 4 of PSC05
#define LED_PIN        13              // for testing 

x10 SX10= x10(ZCROSS_PIN,TRANS_PIN,RCVE_PIN);// set up a x10 library instance:


// This is the time at which the pump was last switched on
long lastPumpOnTime = 0;

// The time at which we last switched the pump off
long lastPumpOffTime = 0;

unsigned int isPumpOn = 0;

// Length of time to run pump, in ms
const unsigned int ON_PERIOD = secondsInMillis(60);

// Period before circuit is active again, in ms
const unsigned int INACTIVE_PERIOD = minutesInMillis(5);

void setup () {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Serial.begin(BAUD_RATE);
}

// Check if we have received the appropriate X10 command to signal
// a request for hot water
int checkForSignal() {
  int signal = 0;

  if (SX10.received()) {      // received a new command
    SX10.debug();
    SX10.reset();
    byte cmndCode = SX10.cmndCode();

    if (SX10.unitCode() == 2 && cmndCode == ON) {
      signal = 1;
    }      
    SX10.reset();
  }  
  return signal;
}

void loop() {
  int signal = checkForSignal();

  if (signal) {
    Serial.println("X10 Signal received");
  }

  if (isPumpOn) {
    // If the pump has been running long enough, turn it off.
    if (millis() - lastPumpOnTime > ON_PERIOD) {
      turnPumpOff();
    }
  } else {
    // If there's been a signal, and enough time has passed since we last ran the pump, then
    // turn it on again.
    if (signal && (lastPumpOffTime == 0 || millis() - lastPumpOffTime > INACTIVE_PERIOD)) {
      turnPumpOn();
    }
  }
}

void turnPumpOff() {
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  isPumpOn = 0;
  lastPumpOffTime = millis();
  Serial.println("Pump off");
}

void turnPumpOn() {
  isPumpOn = 1;
  lastPumpOnTime = millis();
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Pump on");
}

long minutesInMillis(long mins) {
  return mins * 60 * 1000;
}

long secondsInMillis(long seconds) {
  return seconds * 1000;
}



