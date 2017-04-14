#include "RemoteClimateDevice.h"
#include <DHT.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)


int deviceAddress = 1;
WakeUpSerial* wakeUpSerial;

void alarm() {
  digitalWrite(8, HIGH);
}

void setup() {
  pinMode(8, OUTPUT);
  wakeUpSerial = new WakeUpSerial(Serial, 9600);
  wakeUpSerial->addRxPacketListener(RemoteClimateDevice::wakeUpRxHandler);
 }

void loop() {
  wakeUpSerial->processing();
  RemoteClimateDevice::getInstance()->updateData();  
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  wakeUpSerial->keepRxOn();
}


