#ifndef REMOTE_CLIMATE_DEVICE_H
#define REMOTE_CLIMATE_DEVICE_H

#include "WakeUpSerial.h"
#include "ClimateDevice.h"
#include <DHT.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

//command code:

#define CMD_GET_CDI     10    //request ClimateDeviceInfo info
//  0   1   2   3 .. N  - climate devices
//  2   2   1   2 .. 5  -  type of climate devices

#define CMD_GET_CD_VAL 11    //getClimateDevice value
#define CMD_SET_CD_VAL 12    //set ClimateDevice value


class RemoteClimateDevice {
private:
  static boolean _instanceFlag;
  static RemoteClimateDevice* _rcd;
  DHT _dht;
  unsigned long _time;
  float _temperature;
  float _humidity;
  static const char _deviceInfo[];
  static const char _climateDevices[];
  void getValue(char *buffer, size_t maxsize, unsigned char cdNumber) const;
  RemoteClimateDevice(unsigned char dhtPin, unsigned char dhtType);  
public:
  ~RemoteClimateDevice() {_instanceFlag = false;}
  static RemoteClimateDevice* getInstance();
  static void wakeUpRxHandler(const WakeUpSerial* wserial, const WakePacketRx* rxp);
  void updateData();
};


#endif