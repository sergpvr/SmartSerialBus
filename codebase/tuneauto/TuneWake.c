#include "wake.h"
#include "TuneWake.h"
#include "TuneTables.h"
#include <string.h>
#include  "hard.h"

#ifdef MultiDevice
extern unsigned char GetWakeAddress();
#else
static inline unsigned char GetWakeAddress() {return 0;}
#endif


// оды ошибок:
//#define ERR_NC 0x10   //no such command 
const char dev_info[] = "TuneAuto ver:1.0 SN:";

static void hex2s(u8 num, char *s)
{
	u8 i,num1;
	for(i=2;i>0;i--)
	 {
		num1=(num>>(4*(i-1))) & 0xF;
		if (num1<0xA) *s++ = 0x30 + num1;
			else *s++ = 0x37 + num1;
	 }
}

char* Get_U_ID(char *s)
{
  char *start = s;
  hex2s(U_ID11, s); s+=2;
  hex2s(U_ID10, s); s+=2;
  hex2s(U_ID9, s); s+=2;
  hex2s(U_ID8, s); s+=2;
  hex2s(U_ID7, s); s+=2;
  hex2s(U_ID6, s); s+=2;
  hex2s(U_ID5, s); s+=2;
  hex2s(U_ID4, s); s+=2;
  hex2s(U_ID3, s); s+=2;
  hex2s(U_ID2, s); s+=2;
  hex2s(U_ID1, s); s+=2;
  hex2s(U_ID0, s); s+=2;
  *s=0;
  
  return start;
}

void TableTransceiver(WakePacket *wp);
void TableAdjPoint(WakePacket *wp);

inline void CreateParametersValueError(WakePacket *wp)
{
   wp->CntData = 1;
   wp->data[0] = ERR_PA;
}
 
void WakeTuneProc() 
{
  WakePacket wp;
  if( WakeRXnoBlock(&wp) == TRUE && wp.CodeErr == ERR_NO && wp.Address == GetWakeAddress() )
  {  
     if (GDeviceState.BusyDeviceForWake == 1) {
       wp.CntData = 1;
       wp.data[0] = ERR_BU;  
     }
     else switch(wp.Command)
     { 
     case CMD_INFO :
       strcpy((char*)wp.data, dev_info);
       char buf[25];
       strcat((char*)wp.data, Get_U_ID(buf));
       wp.CntData = strlen((char*)wp.data); //+ 1;
       break;
     case CMD_TABLE_TRANSFER :
       TableTransceiver(&wp);
       break; 
//     case CMD_TABLE_INIT :
//       if(wp.CntData > 0) {
//         TableUtilitiesInitReq(&tt_baton, wp.data[0]);
//         wp.CntData = 1;
//         wp.data[0] = ERR_NO;
//       }
//       else CreateParametersValueError(&wp);
//       break;
     case CMD_TABLE_ADJPOINT:
       TableAdjPoint(&wp);
       break;
     case CMD_EMOTOR:
       if (wp.CntData == 2) {
         if (wp.data[0] == 0 ) { // get pwm
           wp.data[0] = 100; // todo read pwm
         }
         else if (wp.data[0] == 1 ) { // set pwm
           EMotorSetPWM(wp.data[1]); 
         }
         else wp.data[0] = ERR_PA;
       }
       else wp.data[0] = ERR_PA;
       wp.CntData = 1;      
       break;
     case CMD_GETADC:
       if (wp.CntData == 1) {
         unsigned char ADCSet = wp.data[0];
         char n = 0;
         for(char i=0;i<8;++i) {
           if ( ADCSet & (1<<i) ) {
             wp.data[n++] = Get_Adc_Channel(i);
           }
         }
         wp.CntData = n;
       }
       else {
         wp.data[0] = ERR_PA;
         wp.CntData = 1;
       }
       break;
     default:
       wp.CntData = 1;
       wp.data[0] = ERR_NC;
       break;
     }
     WakeTX(&wp);
  }
}

struct _TDU {
  unsigned char RW; 
  unsigned char NumberTable;
  unsigned char Offset;
  unsigned char DataLength;
  unsigned char* data;
};
typedef struct _TDU TDU;

void TableTransceiver(WakePacket *wp)
{
// Command TABLE
// data TDU (table data unit):
//  ---------------------------------------------------------------  
//  |  0  |   1     |   2    |    3        |          data        |  
//  ---------------------------------------------------------------
//  | R/W | N table | Offset | Data Length | 0 | 1  | 2  | .. | n |
//  --------------------------------------------------------------- 
  
   if(wp->CntData < 4  || wp->data[3] >= FRAME) // помилка неправильний формат TDU
                                //wp->data[3] >= FRAME - device hasn`t enuf memory
   {
     CreateParametersValueError(wp);
     return;
   }
  
   TDU tdu;
   tdu.RW = wp->data[0];
   tdu.NumberTable = wp->data[1];
   tdu.Offset = wp->data[2]; 
   tdu.DataLength = wp->data[3];
   tdu.data = wp->data + 4;
   const unsigned char *flashTable = GetTuneTable(tdu.NumberTable);
   if(flashTable == 0 || ((int)tdu.Offset + (int)tdu.DataLength) > 256)
   {
     CreateParametersValueError(wp);
     return;
   }
   flashTable += tdu.Offset;
   if (tdu.RW == 0) // read table
   {
     while (tdu.DataLength--)
     {
       *tdu.data++ = *flashTable++;
     }
     wp->data[0] = 1; // write as answer on read request.
     wp->CntData = wp->data[3] + 4;
   }
   else // write table
   {
     if (wp->CntData != (tdu.DataLength + 4))
     {
       CreateParametersValueError(wp);
       return;      
     }
     WriteInProgramMemory(flashTable, tdu.data, tdu.DataLength);// real write to flash
     wp->CntData = 1;
     wp->data[0] = ERR_NO;     
   }      
}

void TableAdjPoint(WakePacket *wp) {
//  Set Point (to device)
//  ----------------------------------------------
//  |  0  |   1     |   2    |   3   |    4      | 
//  ----------------------------------------------
//  |  1  | N table | Point  | Rdata | HalfRange |
//  ----------------------------------------------
// Answer on Set (from device)
//  -------
//  |  0  |
//  -------
//  |State|       // State = ERR_NO or ERR_BU
//  -------
//  Get Point  (to device)
//  -------
//  |  0  |
//  -------
//  |  0  |
//  -------
// Answer on Get (from device)
//  ----------------------------------------------
//  |  0  |   1     |   2    |   3   |    4      | 
//  ----------------------------------------------
//  |State| N table | Point  | Rdata | Limit     |
//  ---------------------------------------------- 
// State = ERR_NO (count data = 5) or ERR_BU or ERR_PA (count data = 1)

   if( wp->CntData == 1 && wp->data[0] == 0 ) { // host asks result operation
     switch (tt_baton.AdjustPoint.State) {
       case LOSStart: // data is not ready
         wp->CntData = 1;
         wp->data[0] = ERR_BU;
         break;
       case LOSFinish: // data is ready
         wp->CntData = 5;
         wp->data[0] = ERR_NO;
         wp->data[1] = tt_baton.AdjustPoint.TableNumber;
         wp->data[2] = tt_baton.AdjustPoint.Point;
         wp->data[3] = tt_baton.AdjustPoint.Rdata;
         wp->data[4] = tt_baton.AdjustPoint.limit;
         tt_baton.AdjustPoint.State = LOSInit;
         break;
       default://LOSInit
         CreateParametersValueError(wp);
         break;
     }
   }
   else if (wp->CntData == 5 && wp->data[0] == 1 ) { //host sets adjust point
     if (tt_baton.AdjustPoint.State == LOSInit) {
       tt_baton.AdjustPoint.TableNumber = wp->data[1];
       tt_baton.AdjustPoint.Point = wp->data[2];
       tt_baton.AdjustPoint.State = LOSStart;
       tt_baton.AdjustPoint.Rdata = wp->data[3];
       wp->data[0] = ERR_NO;
     }
     else {
       wp->data[0] = ERR_BU;
     }
     wp->CntData = 1;
   }
   else {
     CreateParametersValueError(wp); 
   }
 }