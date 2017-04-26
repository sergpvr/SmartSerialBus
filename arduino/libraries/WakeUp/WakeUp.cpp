/*
  WakeUp.cpp - Library for communication by WakeUp485 protocol.
  Created by Serhiy Povoroznyuk, November 6, 2016.
  Released into the public domain.
*/

#include "Arduino.h"
#include "WakeUp.h"

#define BAUDRATE 115200

// Actual interrupt handler
void WakeUp::_tx_complete_irq(void)
{
  digitalWrite(_tx_pin, LOW);
}

ISR(USART_TX_vect)
{
  WakeUp._tx_complete_irq();
}

WakeUp::WakeUp(int txpin)
{
  _tx_pin = txpin;
  pinMode(_tx_pin, OUTPUT);
  //set listening mode
  digitalWrite(_tx_pin, LOW);
  
  // initialize serial:
  Serial.begin(BAUDRATE);
  //
}

~WakeUp()
{
  Serial.end();
}