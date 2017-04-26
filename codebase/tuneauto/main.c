
#include "config.h"
#include "stm8s.h"
#include "stm8s_type.h"
#include "hard.h"
#include "RegisterFn.h"
#include "TuneWake.h"
//#include "i2c.h"
#include <string.h>
//#include "SwitchToOverlay.h"
#include "TuneTables.h"
//============= функции обслуживания светодиодов ===============================
void flash_led(void)
{
    //PE_DDR_DDR0 = 1;
    //PE_ODR_ODR0 ^= 1;
}
//=============
void alarm_on(void)
{
  static unsigned char flag=0;
  if(flag==0){
    RegisterFn(flash_led,mS(500));
    flag=1;
  }else{
    UnRegisterFnByName(flash_led);
    flag=0;
  }
}
//=============
void testPWM1(void)
{
  static unsigned char i=10;
  static unsigned char flag=0;
    TIM1_CCR2L = i;
    TIM1_CCR4L = TIM1_MAX-i;
    if(flag == 0){
      if(i++ == (TIM1_MAX/2-2)){
        flag = 1;
      }
    }else{
      if(i-- == 1){
        flag = 0;
      }
    }
}
//============= внешние структуры, переменные ==================================
//extern STRUCT_I2C i2c;
//extern const unsigned int sin[];
//static unsigned char buf [100];
//============= функциия main ==================================================
#define PSTR  (const char*)
#define pgm_read_byte(Address)            *(((unsigned char*)Address))


unsigned int fill_tcp_data_p(unsigned char *p,const char *progmem_s)
{
  char pos = 0;
        char c;
        // fill in tcp data at position pos
        //
        // with no options the data starts after the checksum + 2 more bytes (urgent ptr)
        while ((c = pgm_read_byte(progmem_s++))) {
                p[pos]=c;
                pos++;
        }
        return(pos);
}

/* Declare a section for temporary storage. */
/*#pragma section = "TEMPSTORAGE" 
char *GetTempStorageStartAddress()
{
  return __section_begin("TEMPSTORAGE");
}
*/
//#pragma section = "MYOVERLAY1INROM"
/* Function that switches in image 1 into the overlay. */
/*void SwitchToOverlay1()
{
  static char *targetAddr;
  static char *sourceAddr;
  static char *sourceAddrEnd;
  static int size;
  targetAddr = __section_begin("MYOVERLAYINRAM");
  sourceAddr = __section_begin("MYOVERLAY1INROM");
  sourceAddrEnd = __section_end("MYOVERLAY1INROM");
  size = sourceAddrEnd - sourceAddr;
  memcpy(targetAddr, sourceAddr, size);
}*/

unsigned char SelectValFromTable(unsigned char val, const unsigned char *table)
{
  const char xxx[] = {
       0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,
      80,  85,  90,  95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155,
     160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235,
     240, 245, 250
  };   
  unsigned char i;
  unsigned char x0;
  unsigned char x1 = 0;
  unsigned char y0;
  unsigned char y1;
    
  for(i=1; i<51; i++)
  {
    if(val == xxx[i]) return *(table + i); 
    if(val < xxx[i])
    {
      x1 = xxx[i];
      x0 = xxx[i-1];
      y1 = *(table + i);
      y0 = *(table + i - 1);
      break;
    }
  } 
  if(x1 == 0) return *(table + 50);//x is over 250

  return y0 + (unsigned char)( ((int)(val - x0) * (int)(y1 - y0)) / (x1 - x0) );
}

void ADCtoDigRezistor() {
   unsigned char i = 0;
   unsigned char adc_val;
   for(i=0; i<4; i++) {
     if( tt_baton.AdjustPoint.State != LOSInit &&
       i == tt_baton.AdjustPoint.TableNumber ) {
       continue;
     }
     adc_val = Get_Adc_Channel(i);
     SetRezistorCAT5172( i, SelectValFromTable(adc_val, GetTuneTable(i)) );
   }
}

int main( void )
{
  /*static char * TempStorage;
  TempStorage = GetTempStorageStartAddress();
  SwitchToOverlay1();
  FnPointer FnEeprom = (FnPointer)__section_begin("MYOVERLAYINRAM");//__section_begin("MYOVERLAY1");
  FnEeprom();*/
  //CopyFnToRam(BufFunc, wait_eeprom);              //Копирование функции wait_eeprom в RAM
  //t_FnPointer FnPointer = (t_FnPointer)BufFunc;   //создание указателя на функцию
  //FnPointer();                                    //Вызов функции из RAM
  
  INIT_CLK;             //установка тактовой частоты = 16МГц
  BEEP_CSR = 0x6E;

  //TIM1_CR1 = 1;//ARPE CMS[1:0] DIR OPM URS UDIS CEN
  
//static unsigned int gPlen;
  
//gPlen=fill_tcp_data_p(buf,PSTR("<h1>200 OK</h1>"));

//  static unsigned int delay_7 = (unsigned int)(F_CLK/100000/7+1);// 10us
//  while(--delay_7 != 0);
  
//  i2c_init();
  TableUtilitiesInit();
  hard_init();
  
  
  
  /*pool_temperature();//получить температуру с датчика по i2c. Данные в структуре i2c
  while( I2C_ITR == 0x06 );//условие получения данных с термодатчика*/
  /*TIM1_CNTRH = 0;
  TIM1_CNTRL = 0;
  TIM1_SR1_CC1IF = 0;
  while(TIM1_SR1_CC1IF == 0);//пауза 4,096мс*/

  enableInterrupts();
  
  //RegisterFn(alarm_on,mS(5000));
  //RegisterFn(pool_temperature,mS(100));
  //RegisterFn(testPWM1,mS(10));
  //uart_putstr("\r\nuart is ready\r\n");
  // s16 c;
  //SetRezistorAD5204(1, 184);
  //SetRezistorCAT5172(0, 1);
  //SetRezistorCAT5172(1, 251);
  //SetRezistorCAT5172(2, 255);
  //volatile int i;
  while(1){
   //SetRezistorCAT5172(0, 0x00);
   //for (i=1000;i>0;) i--;
   //SetRezistorCAT5172(1, 255);
   //SetRezistorCAT5172(2, 0xAA);
   //SetRezistorCAT5172(3, 0xF0);
   WakeTuneProc();
   if( tt_baton.AdjustPoint.State == LOSInit ) ADCtoDigRezistor();
   TableUtilities();
   //if(CheckButton()) LedState(1);
   //else LedState(0);
   //    c = uart_getchar();
   //    if( c != buffer_is_empty) uart_putchar((u8)c); 
  }
  //TIM1_IER = 
  //TIM1_CR1 = MASK_TIM1_CR1_CEN | MASK_TIM1_CR1_UDIS | MASK_TIM1_CR1_URS | MASK_TIM1_CR1_OPM | MASK_TIM1_CR1_DIR | MASK_TIM1_CR1_CMS | MASK_TIM1_CR1_ARPE;
  //TIM1_CR1_CEN  = 1;
  /*
  TIM1_IER = 0;
  TIM1_IER = TIM1_CR1_CEN;
  TIM1_OISR = 0xff;
  TIM1_CCMR1_bit.CC1S = 1;
  TIM1_IER = 1;
    PD_DDR = 0xFF;            // all PD pins as output
    PD_CR1 = 0xFF;            // all PD pins output push pull
    PD_CR2 = 0;                    // all PD pins speed 2MHz
    

    while(1){
      a++;
      if(a==10000){
        PD_ODR = 0;                    // all set to 0
      }
    if(a==20000){
      PD_ODR = 0xFF;                    // all set to 0
    }
    
    }
  */
}
