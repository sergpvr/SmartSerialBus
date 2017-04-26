
#include "wake.h"
#include "uart.h"


/*---------------------------------------------------
 * is data=FEND then FESC,TFEND;
 * is data=FESC then FESC,TFESC;
 * --------------------------------------------------*/

//end SLIP protocol

#define CRC_INIT  0xDE  //Innitial CRC value

//--------------------- CRC ------------------------
void Do_Crc8(char b, char *crc)
{
  for(char i = 0; i < 8; b = b >> 1, i++)
    if((b ^ *crc) & 1) *crc = ((*crc ^ 0x18) >> 1) | 0x80;
     else *crc = (*crc >> 1) & ~0x80;
}

static inline void SendStartSLIP()
{
  while(!uart_putchar(FEND));
}

void SendCharSLIP(unsigned char ch)
{
  if(ch == FEND)
  {
    while(!uart_putchar(FESC));
    while(!uart_putchar(TFEND));
  }
  else if (ch == FESC)
  {
    while(!uart_putchar(FESC));
    while(!uart_putchar(TFESC));    
  }
  else
  {
    while(!uart_putchar(ch));
  }
}

bool WakeTX(const WakePacket *wp)
{
  char crc = CRC_INIT;
  Do_Crc8(FEND,&crc);
  SendStartSLIP(); // start packet
  if(wp->Address)
  {
    Do_Crc8(wp->Address,&crc);
    SendCharSLIP(wp->Address | 0x80); // send address
  }
  Do_Crc8(wp->Command,&crc);
  SendCharSLIP(wp->Command); // send command
  Do_Crc8(wp->CntData,&crc);
  SendCharSLIP(wp->CntData); // send N 
  unsigned char i = wp->CntData;
  const unsigned char *data = wp->data;
  while(i--)
  {
    Do_Crc8(*data,&crc);
    SendCharSLIP(*data++); // send data byte 
  }
  SendCharSLIP(crc); // send crc
  return TRUE;
}

enum STATE_RX
{
  rxBEGIN=0,
  rxSTARTPACKET,
  rxADDRESS,
  rxCOMMAND,
  rxDATA,
  rxCRC
};

bool WakeRXnoBlock(WakePacket *wp)
{
  // for byte stuffing
  static bool flagFESC = FALSE;
  //
  static char crc;
  static enum STATE_RX state = rxBEGIN;
  static unsigned char *data;
  
  s16 rcv = uart_getchar();
  if(rcv == buffer_is_empty) return FALSE;
  register unsigned char rb = (unsigned char)rcv;
  
  if (rb == FEND)  state = rxBEGIN;
  
  if(rb == FESC && flagFESC == FALSE)
  {
    flagFESC = TRUE;
    return FALSE;
  }
  if (flagFESC == TRUE) //byte stuffing
  {
    flagFESC = FALSE;
    if (rb == TFEND) rb = FEND;
    else if (rb == TFESC) rb = FESC;
  }
  switch(state)
  {
    case  rxBEGIN:
      if(rb == FEND)
      {
        state = rxSTARTPACKET;
        crc = CRC_INIT;
        Do_Crc8(FEND,&crc);
      }
      break;
    case rxSTARTPACKET:
      if(rb & 0x80) 
      {
        state = rxADDRESS;
        wp->Address = rb & 0x7F;
        Do_Crc8(wp->Address,&crc);
      }
      else
      {
        state = rxCOMMAND;
        wp->Address = 0;
        wp->Command = rb;
        Do_Crc8(rb,&crc);
      }
      break;
    case rxADDRESS:
      state = rxCOMMAND;
      wp->Command = rb;
      Do_Crc8(rb,&crc);     
      break;
    case rxCOMMAND: // receive CntData
      data = wp->data; 
      state = (rb != 0) ? rxDATA : rxCRC;
      wp->CntData = rb;
      if(rb > FRAME) // err: packet is very long
      {
        wp->CodeErr = ERR_TX;
        //state = rxBEGIN;
        return TRUE;
      }
      Do_Crc8(rb,&crc);     
      break; 
    case rxDATA:
      *data++ = rb;
      Do_Crc8(rb,&crc);
      if((data - wp->data) == wp->CntData) 
      {
        state = rxCRC;
      }
      break;
    case rxCRC:
      wp->CodeErr = (rb == crc) ? ERR_NO : ERR_TX;
      //state = rxBEGIN;
      return TRUE;
      //break;       
  }
  return FALSE; 
}

bool WakeRX(WakePacket *wp)
{
  while(WakeRXnoBlock(wp) != TRUE)
  {
    //if()  // timeout
    //{
    //  return FALSE;
    //}
  }
  return TRUE;
}
