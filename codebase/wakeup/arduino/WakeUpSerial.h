#ifndef WAKEUPSERIAL_H
#define WAKEUPSERIAL_H

#include "Arduino.h"
//#include "tests\wakeUpTest\Arduino.h"

//#define MultiDevice

#ifdef MultiDevice
extern unsigned char GetWakeAddress();
#else
static inline unsigned char GetWakeAddress() {return 1;}
#endif

#define SLIPFRAME     100   //packet max long

//SLIP protocol

#define 	FEND		(unsigned char)0xC0 // Frame End
#define 	FESC 		(unsigned char)0xDB // Frame Escape
#define 	TFEND		(unsigned char)0xDC // Transposed Frame End
#define 	TFESC		(unsigned char)0xDD // Transposed Frame Escape

#define 	CRC_INIT  0xDE  //initial CRC value

// protocol WAKE

//Universal command code:

#define CMD_NOP  0    //nop
#define CMD_ERR  1    //rx packet error
#define CMD_ECHO 2    //send echo
#define CMD_INFO 3    //send device info

#define MOD_ANSW  0x40 // command modifier for answer

//Error code:

#define ERR_NO 0x00   //no error
#define ERR_TX 0x01   //Rx/Tx error
#define ERR_BU 0x02   //device busy error
#define ERR_RE 0x03   //device not ready error
#define ERR_PA 0x04   //parameters value error
#define ERR_NR 0x05   //no replay
#define ERR_NC 0x06   //no carrier

//

class WakeUpSerial;

class WakePacket  {
  protected:
  const WakeUpSerial& _wakeUpSerial;
  unsigned char _address = 0;
  unsigned char _command = 0;
  unsigned char _dataSize = 0;
  char *_data = NULL;
  unsigned char do_crc8(unsigned char b, unsigned char crc);
  virtual unsigned char  performCRCcalculation() = 0;
  public:
  WakePacket(const WakeUpSerial &wakeUpSerial):_wakeUpSerial(wakeUpSerial) {}
  virtual ~WakePacket() {}
  unsigned char getAddress() const {return _address;}
  unsigned char getCommand() const {return _command;}
  unsigned char getDataSize() const {return _dataSize;}
  const char* getData() const {return _data;}
};

class WakePacketRx: public WakePacket {
  private:
  unsigned char  performCRCcalculation();
  // for receive logic :
  enum STATE_RX {
    rxBEGIN=0,
    rxSTARTPACKET,
    rxADDRESS,
    rxCOMMAND,
    rxDATA,
    rxCRC
  };
  unsigned char _codeErr = ERR_NO;
  bool _flagFESC = false; // for byte stuffing
  STATE_RX _state = rxBEGIN; // for receive state
  boolean _rxAddressIsPresent = false;
  char* _dataptr;
  public:
  WakePacketRx(const WakeUpSerial &wakeUpSerial):WakePacket(wakeUpSerial) {
    _data = new char [SLIPFRAME];
  }
  ~WakePacketRx() {delete [] _data;}
  boolean readByteFromSerial();
  unsigned char getCodeErr() const {return _codeErr;}
};

class WakePacketTx: public WakePacket {
  private:
  unsigned char  performCRCcalculation();
  boolean _sendAddress;
  void sendStartSLIP();
  void sendCharSLIP(unsigned char val);
  public:
  WakePacketTx(const WakeUpSerial &wakeUpSerial, unsigned char command,
      const unsigned char *data, unsigned char dataSize);
  WakePacketTx(const WakeUpSerial &wakeUpSerial, unsigned char adress,
      unsigned char command, const char *data, unsigned char dataSize);
  ~WakePacketTx() {delete [] _data;}
  void send();
};


/*
class WakePacket {
private:
  WakeUpSerial& _wakeUpSerial;
  unsigned char _address = 0;
  unsigned char _command = 0;
  unsigned char _dataSize = 0;
  unsigned char _data[SLIPFRAME];
  unsigned char do_crc8(unsigned char b, unsigned char crc);
  unsigned char  performCRCcalculation();
  // for receive logic :
  enum STATE_RX {
    rxBEGIN=0,
    rxSTARTPACKET,
    rxADDRESS,
    rxCOMMAND,
    rxDATA,
    rxCRC
  };
  unsigned char _codeErr = ERR_NO;
  bool _flagFESC = false; // for byte stuffing
  STATE_RX _state = rxBEGIN; // for receive state
  boolean _rxAddressIsPresent = false;
  unsigned char* _dataptr;
public:
  WakePacket(WakeUpSerial &wakeUpSerial):_wakeUpSerial(wakeUpSerial) {}
  WakePacket(WakeUpSerial &wakeUpSerial, unsigned char address,
      unsigned char command, const unsigned char *data, unsigned char dataSize);
  boolean readByteFromSerial();
  unsigned char getAddress() {return _address;}
  unsigned char getCommand() {return _command;}
  unsigned char getDataSize() {return _dataSize;}
  const unsigned char* getData() const {return _data;}
  unsigned char getCodeErr() {return _codeErr;}
};
*/

typedef void (*RxPacketListener)(const WakeUpSerial* wserial, const WakePacketRx* rxp);

class WakeUpSerial {
private:
  HardwareSerial& _serial;	
  boolean _wakePacketReceived = false;
  WakePacketRx* _rxWakePacket = NULL;
  void wakePacketReceived(const WakePacketRx* rxp);
  RxPacketListener _listener = NULL;
public:
	WakeUpSerial(HardwareSerial &serial, unsigned long baud):_serial(serial) {_serial.begin(baud); _serial.println("starting WakeUp ..."); _rxWakePacket = createRxPacket();}
  ~WakeUpSerial() { if(_rxWakePacket) delete _rxWakePacket; }
  WakePacketRx* createRxPacket() const;
  WakePacketTx* createTxPacket(unsigned char address, unsigned char command, const char *data, unsigned char dataSize) const;
  void sendAnswer(const WakePacketRx* rxp, const char* data,  size_t dataSize) const;
  void sendErr(unsigned char address, const char* data,  size_t dataSize) const;
  int read() const {return _serial.read();}
  void write(unsigned char ch) const { _serial.write(ch); }
  void processing(); // for calling in loop()
  void keepRxOn(); // for calling in serialEvent()
  void addRxPacketListener(RxPacketListener listener) { _listener = listener; }
  void removeRxPacketListener() { _listener = NULL; }
};


#endif
