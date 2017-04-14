#include "WakeUpSerial.h"
void alarm();

/*
WakePacket::WakePacket(WakeUpSerial &wakeUpSerial, unsigned char adress,
  unsigned char command, const unsigned char *data, unsigned char dataSize):
  _wakeUpSerial(wakeUpSerial), _address(address), _command(command), _dataSize(dataSize) {
  strncpy((char*)_data, (char*)data, (size_t)dataSize);
}

unsigned char WakePacket::do_crc8(unsigned char b, unsigned char crc) {
  unsigned char i;
  for (i = 0; i < 8; b = b >> 1, i++) {
    if (((b ^ crc) & 1) == 1) {
      crc = ((crc ^ 0x18) >> 1) | 0x80;
    } else {
      crc = (crc >> 1) & 0x7F;
    }
  }
  return crc;
}

unsigned char  WakePacket::performCRCcalculation() {
  unsigned char crc = CRC_INIT;
  crc = do_crc8(FEND, crc);
  if (_rxAddressIsPresent) crc = do_crc8(this->getAddress(), crc);
  crc = do_crc8(this->getCommand(), crc);
  unsigned char i = this->getDataSize();
  crc = do_crc8(i, crc);
  const unsigned char *data = this->getData();
  while (i--) {
    crc = do_crc8(*data, crc);
  }
  return crc;
}

boolean WakePacket::readByteFromSerial() {
  int rxb = _wakeUpSerial.read();
  if (rxb != -1) return false;
  unsigned char rb = (unsigned char)rxb;
  if (rb == FEND)  _state = rxBEGIN; 
  if(rb == FESC && _flagFESC == false) {
    _flagFESC = true;
    return false;
  }
  if (_flagFESC == true) { //byte stuffing
    _flagFESC = false;
    if (rb == TFEND) rb = FEND;
    else if (rb == TFESC) rb = FESC;
  }
  switch(_state) {
    case  rxBEGIN:
      if(rb == FEND) {
        _state = rxSTARTPACKET;
      }
      break;
    case rxSTARTPACKET:
      if(rb & 0x80) {
        _state = rxADDRESS;
        _address = rb & 0x7F;
        _rxAddressIsPresent = true;
      } else {
        _state = rxCOMMAND;
        _address = 0;
        _command = rb;
      }
      break;
    case rxADDRESS:
      _state = rxCOMMAND;
      _command = rb;
      break;
    case rxCOMMAND: // receive CntData
      _dataptr = _data;
      _state = (rb != 0) ? rxDATA : rxCRC;
      _dataSize = rb;
      if(rb > SLIPFRAME) {// err: packet is very long
        _codeErr = ERR_TX;
        return true;
      }
      break;
    case rxDATA:
      *_dataptr++ = rb;
      if((_dataptr - _data) == _dataSize) {
        _state = rxCRC;
      }
      break;
    case rxCRC:
      _codeErr = (rb == performCRCcalculation()) ? ERR_NO : ERR_TX;
      return true;
  }
  
  return false;
}
*/

unsigned char WakePacket::do_crc8(unsigned char b, unsigned char crc) {
  unsigned char i;
  for (i = 0; i < 8; b = b >> 1, i++) {
    if (((b ^ crc) & 1) == 1) {
      crc = ((crc ^ 0x18) >> 1) | 0x80;
      } else {
      crc = (crc >> 1) & 0x7F;
    }
  }
  return crc;
}

boolean WakePacketRx::readByteFromSerial() {
  int rxb = _wakeUpSerial.read();
  if (rxb == -1) return false;
  unsigned char rb = (unsigned char)rxb;
  if (rb == FEND)  _state = rxBEGIN;
  if(rb == FESC && _flagFESC == false) {
    _flagFESC = true;
    return false;
  }
  if (_flagFESC == true) { //byte stuffing
    _flagFESC = false;
    if (rb == TFEND) rb = FEND;
    else if (rb == TFESC) rb = FESC;
  }
  switch(_state) {
    case  rxBEGIN:
    if(rb == FEND) {
      _state = rxSTARTPACKET;
    }
    break;
    case rxSTARTPACKET:
    if(rb & 0x80) {
      _state = rxADDRESS;
      _address = rb & 0x7F;
      _rxAddressIsPresent = true;
      } else {
      _state = rxCOMMAND;
      _address = 0;
      _command = rb;
    }
    break;
    case rxADDRESS:
    _state = rxCOMMAND;
    _command = rb;
    break;
    case rxCOMMAND: // receive CntData
    _dataptr = _data;
    _state = (rb != 0) ? rxDATA : rxCRC;
    _dataSize = rb;
    if(rb > SLIPFRAME) {// err: packet is very long
      _codeErr = ERR_TX;
      return true;
    }
    break;
    case rxDATA:
    *_dataptr++ = rb;
    if((_dataptr - _data) == _dataSize) {
      _state = rxCRC;
    }
    break;
    case rxCRC:
    _codeErr = (rb == this->performCRCcalculation()) ? ERR_NO : ERR_TX;
    return true;
  }
  
  return false;
}

unsigned char  WakePacketRx::performCRCcalculation() {
  unsigned char crc = CRC_INIT;
  crc = do_crc8(FEND, crc);
  if (_rxAddressIsPresent) crc = do_crc8(this->getAddress(), crc);
  crc = do_crc8(this->getCommand(), crc);
  unsigned char i = this->getDataSize();
  crc = do_crc8(i, crc);
  const char *data = this->getData();
  while (i--) {
    crc = do_crc8(*data++, crc);
  }
  return crc;
}

WakePacketTx::WakePacketTx(const WakeUpSerial &wakeUpSerial, unsigned char address,
  unsigned char command, const char *data, unsigned char dataSize):
    WakePacket(wakeUpSerial), _sendAddress(true) {
  
  _address = address;
  _command = command;
  _data = new char[dataSize+1];
  memcpy((char*)_data, (char*)data, (size_t)dataSize);
  _dataSize = dataSize;
}

WakePacketTx::WakePacketTx(const WakeUpSerial &wakeUpSerial, unsigned char command, 
  const unsigned char *data, unsigned char dataSize):
    WakePacket(wakeUpSerial), _sendAddress(false) {

  _command = command;
  _data = new char[dataSize+1];
  memcpy((char*)_data, (char*)data, (size_t)dataSize);
  _dataSize = dataSize;
}

unsigned char  WakePacketTx::performCRCcalculation() {
  unsigned char crc = CRC_INIT;
  crc = do_crc8(FEND, crc);
  if (_sendAddress) crc = do_crc8(this->getAddress(), crc);
  crc = do_crc8(this->getCommand(), crc);
  unsigned char i = this->getDataSize();
  crc = do_crc8(i, crc);
  const char *data = this->getData();
  while (i--) {
    crc = do_crc8(*data++, crc);
  }
  return crc;
}

void WakePacketTx::sendStartSLIP() {
  _wakeUpSerial.write(FEND);
}

void WakePacketTx::sendCharSLIP(unsigned char val) {
  if(val == FEND) {
    _wakeUpSerial.write(FESC);
    _wakeUpSerial.write(TFEND);
  } 
  else if (val == FESC) {
    _wakeUpSerial.write(FESC);
    _wakeUpSerial.write(TFESC);
  }
  else {
    _wakeUpSerial.write(val);
  }
}

void  WakePacketTx::send() {
  sendStartSLIP(); // start packet
  if(_sendAddress) sendCharSLIP(this->getAddress() | 0x80); // send address
  sendCharSLIP(this->getCommand()); // send command
  unsigned char i = this->getDataSize();
  sendCharSLIP(i); // send dataSize
  const char *data = this->getData();
  while(i--) {
    sendCharSLIP(*data++); // send data byte
  }
  sendCharSLIP(performCRCcalculation()); // send crc
}

WakePacketRx* WakeUpSerial::createRxPacket() const {
  return new WakePacketRx(*this);
}


WakePacketTx* WakeUpSerial::createTxPacket(unsigned char address, unsigned char command, const char *data, unsigned char dataSize) const {
  return new WakePacketTx(*this, address, command, data, dataSize);
}

void WakeUpSerial::processing() {
  if(_wakePacketReceived) {
    wakePacketReceived(_rxWakePacket); // processing RX wakePacket
    delete _rxWakePacket;
    _rxWakePacket = this->createRxPacket();
    _wakePacketReceived = false;
  } 
}

void  WakeUpSerial::keepRxOn() {
  while (_serial.available() && !_wakePacketReceived) {
    _wakePacketReceived = _rxWakePacket->readByteFromSerial();
  }
}

void WakeUpSerial::wakePacketReceived(const WakePacketRx* rxp) {
  if (rxp->getCodeErr() == ERR_NO && rxp->getAddress() == GetWakeAddress()) {
    switch (rxp->getCommand()) {
      case CMD_NOP : 
        break;
      case CMD_ECHO :
        sendAnswer(rxp,  rxp->getData(), rxp->getDataSize());
        break;
      default:
        if(_listener) {
          _listener(this, rxp);
        } 
        break;
    }
  }
}

void WakeUpSerial::sendAnswer(const WakePacketRx* rxp, const char* data,  size_t dataSize) const {
  WakePacketTx* txp = this->createTxPacket(rxp->getAddress(), rxp->getCommand() | MOD_ANSW, data, (unsigned char)dataSize);
  txp->send();
  delete txp; 
}

void WakeUpSerial::sendErr(unsigned char address, const char* data,  size_t dataSize) const {
  WakePacketTx* txp = this->createTxPacket(address, CMD_ERR, data, (unsigned char)dataSize);
  txp->send();
  delete txp; 
}

