
#ifndef HARD_H
#define HARD_H

#include "uart.h"
#include "F_CLK.h"
//======================= константы ============================================
#define TIM1_MAX  200
#define TIM3_MAX  799
//======================= константы end ========================================
// @ interrupt from timer even 
//если F_CLK=16 ћ√ц, то шаг = 2мс;  если F_CLK=2 ћ√ц, то шаг = 16мс;  
#define mS(value) (unsigned int)(value * (F_CLK/32000) / 1000)  

//****************************************************************
#include "io_macros.h"
#ifdef __IAR_SYSTEMS_ICC__
#pragma system_include
#pragma language=save
#pragma language=extended
#endif

/* Unique ID stm8s105xx */
__IO_REG8    (U_ID0, 0x48CD, __READ);
__IO_REG8    (U_ID1, 0x48CE, __READ);
__IO_REG8    (U_ID2, 0x48CF, __READ);
__IO_REG8    (U_ID3, 0x48D0, __READ);
__IO_REG8    (U_ID4, 0x48D1, __READ);
__IO_REG8    (U_ID5, 0x48D2, __READ);
__IO_REG8    (U_ID6, 0x48D3, __READ);
__IO_REG8    (U_ID7, 0x48D4, __READ);
__IO_REG8    (U_ID8, 0x48D5, __READ);
__IO_REG8    (U_ID9, 0x48D6, __READ);
__IO_REG8    (U_ID10,0x48D7, __READ);
__IO_REG8    (U_ID11,0x48D8, __READ);

#ifdef __IAR_SYSTEMS_ICC__
#pragma language=restore
#endif
//****************************************************************

typedef struct GDeviceStateBaton {
  unsigned char BusyDeviceForWake;
  volatile unsigned char ADCRefresh; // 1 - event refresh
  volatile unsigned char mode;
  volatile unsigned char flash; // number of flash
} GDeviceStateBaton;

extern GDeviceStateBaton GDeviceState;

void hard_init(void);
void LedOn();
void LedOff();
void WriteInProgramMemory(const unsigned char *address, unsigned char *data, unsigned char count);
void SetRezistorAD5204(unsigned char Address, unsigned char value);
void SetRezistorCAT5172(unsigned char Address, unsigned char value);
static inline void SetDigRezistor(unsigned char Address, unsigned char value) { SetRezistorCAT5172(Address & 0x03,value); }
unsigned char Get_Adc_Channel(unsigned char i); // 0..7
unsigned char Get_Adc_ChannelW(unsigned char i); // wait fresh value
inline char CheckButton();
static inline void NextMode() {
  ++(GDeviceState.mode);
  if (GDeviceState.mode == 4) GDeviceState.mode = 0;
  GDeviceState.flash = GDeviceState.mode + 1;
  LedOn();
}
inline void LedState(char state);
void EMotorSetPWM(unsigned char value);
#endif // HARD_H
