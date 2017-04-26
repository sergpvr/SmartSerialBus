#ifndef WAKE_H
#define WAKE_H

#include "stm8s_type.h"

//#define MultiDevice

#define FRAME     100   //packet max long

//SLIP protocol

  #define 	FEND		0xC0 // Frame End
  #define 	FESC 		0xDB // Frame Escape
  #define 	TFEND		0xDC // Transposed Frame End
  #define 	TFESC		0xDD // Transposed Frame Escape

// protocol WAKE 

//Коды универсальных команд:

#define CMD_NOP  0    //нет операции
#define CMD_ERR  1    //ошибка приема пакета
#define CMD_ECHO 2    //передать эхо
#define CMD_INFO 3    //передать информацию об устройстве

//Коды ошибок:

#define ERR_NO 0x00   //no error
#define ERR_TX 0x01   //Rx/Tx error
#define ERR_BU 0x02   //device busy error
#define ERR_RE 0x03   //device not ready error
#define ERR_PA 0x04   //parameters value error
#define ERR_NR 0x05   //no replay
#define ERR_NC 0x06   //no carrier

//

//#ifdef MultiDevice
//extern unsigned char GetWakeAddress();
//#else
//static inline unsigned char GetWakeAddress() {return 0;}
//#endif

struct _WakePacket {
  unsigned char Address; 
  unsigned char Command;
  unsigned char CntData;
  unsigned char data[FRAME];
  unsigned char CodeErr;
};

typedef struct _WakePacket WakePacket;

bool WakeTX(const WakePacket *wp);
bool WakeRX(WakePacket *wp);
bool WakeRXnoBlock(WakePacket *wp);

#endif // WAKE_H
