/*
  WakeUp.h - Library for communication by WakeUp485 protocol.
  Created by Serhiy Povoroznyuk, November 6, 2016.
  Released into the public domain.
*/
#ifndef WakeUp_h
#define WakeUp_h

#include "Arduino.h"

class WakeUp
{
  public:
    WakeUp(int txpin);
    ~WakeUp();
	// Interrupt handlers - Not intended to be called externally
    void _tx_complete_irq(void);
  private:
    int _tx_pin;
};

#endif