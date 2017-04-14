#include "RemoteClimateDevice.h"

const char RemoteClimateDevice::_deviceInfo[] = "Arduino with DHT 22";
const char RemoteClimateDevice::_climateDevices[] = {TEMPERATURE, HUMIDITY};
  
boolean RemoteClimateDevice::_instanceFlag = false;
RemoteClimateDevice* RemoteClimateDevice::_rcd = NULL;

RemoteClimateDevice* RemoteClimateDevice::getInstance()
{
  if(! _instanceFlag) {
    _rcd = new RemoteClimateDevice(DHTPIN, DHTTYPE);
    _instanceFlag = true;
    return _rcd;
  }  else  {
    return _rcd;
  }
}
  
RemoteClimateDevice::RemoteClimateDevice(unsigned char dhtPin, unsigned char dhtType): _dht(dhtPin, dhtType){
  _dht.begin();
  _time = millis();
}

void RemoteClimateDevice::updateData() {
  if (millis() - _time > 2500) {
    _time = millis();
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    // Read temperature as Celsius
    float t = _dht.readTemperature();
    float h = _dht.readHumidity();
    if ( !isnan(t) && !isnan(h)) {
      _temperature = t;
      _humidity = h;
    }
  }
}

void RemoteClimateDevice::getValue(char *buffer, size_t maxsize, unsigned char cdNumber) const{
  float data = NAN;
  if(cdNumber < 2) {
    switch(_climateDevices[cdNumber]) {
      case TEMPERATURE :
        data = _temperature;
      break;
      case HUMIDITY :
      data = _humidity;
      break;
    }
  }
  String temp(data);
  strncpy(buffer, temp.c_str(), maxsize);
}

void RemoteClimateDevice::wakeUpRxHandler(const WakeUpSerial* wserial, const WakePacketRx* rxp) {
  char err = ERR_PA;
  unsigned char cdNumber;
  
  switch (rxp->getCommand()) {
    case CMD_INFO :
      wserial->sendAnswer(rxp, _deviceInfo, strlen(_deviceInfo));
    break;
    case CMD_GET_CDI :
      wserial->sendAnswer(rxp, _climateDevices, 2);
    break;
    case CMD_GET_CD_VAL :
      char tempbuf[20];
      cdNumber = *rxp->getData();
      if (cdNumber < 2) {
        RemoteClimateDevice::getInstance()->getValue(tempbuf, 20, cdNumber);
        wserial->sendAnswer(rxp, tempbuf, strlen(tempbuf));
      }
      else {
        wserial->sendErr(rxp->getAddress(), &err, 1);
      }
    break;
    case CMD_SET_CD_VAL :
      //setClimateDevice
      wserial->sendErr(rxp->getAddress(), &err, 1);
    break;
    default:
      wserial->sendErr(rxp->getAddress(), &err, 1);
    break;    
  }    
  
}



