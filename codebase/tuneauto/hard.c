
#include "hard.h"
#include "config.h"
#include "stm8s.h"
#include "stm8s_type.h"
#include "RegisterFn.h"
#include "F_CLK.h"

//======================= переменные ===========================================
#define ADC_Continuous_scan_mode
//#define ADC_Single_scan_mode
#define ADC_DEBUG

/* device globals */

#if defined ADC_Single_scan_mode
  unsigned int* Adc_Buffer = (unsigned int*)&ADC_DB0RH;// Указатель на буфер 10-ти каналов
#elif defined ADC_Continuous_scan_mode
  unsigned int Adc_Channel[10];// Буфер для сохранения 10-ти каналов
#else
  #error  не определён режим работы АЦП
#endif
  
 unsigned char Adc8_Channel[8];// Буфер для збереження 8-ми каналів (8бітних) 
 
GDeviceStateBaton GDeviceState;
  
//======================= функции инициализации ================================
  
static inline void timer1_init(void)
{
  //TIM1_IER
  //TIM1_EGR
  //TIM1_RCR
  //TIM1_CR1 = (1<<7)|(1<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(1<<0);//ARPE CMS[1:0] DIR OPM URS UDIS CEN
  
  //TIM1_CR2 = (0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(1<<2)|(0<<1)|(1<<0);//Reserved MMS[2:0] Reserved COMS Reserved CCPC
  TIM1_CR2 = (0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0);//Reserved MMS[2:0] Reserved COMS Reserved CCPC
  TIM1_SMCR = 0;//MSM TS[2:0] Reserved SMS[2:0]
  TIM1_ETR = (0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0);//ETP ECE ETPS[1:0] ETF[3:0]
  TIM1_IER = (0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0);//BIE TIE COMIE CC4IE CC3IE CC2IE CC1IE UIE
  //TIM1_CCMR1 = (0<<7)|(1<<6)|(1<<5)|(0<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//OC1CE OC1M[2:0] OC1PE OC1FE CC1S[1:0]
  TIM1_CCMR1 = (0<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//OC1CE OC1M[2:0] OC1PE OC1FE CC1S[1:0]
  //TIM1_CCMR2 = (0<<7)|(1<<6)|(1<<5)|(0<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//OC2CE OC2M[2:0] OC2PE OC2FE CC2S[1:0]
  //TIM1_CCMR3 = (0<<7)|(1<<6)|(1<<5)|(0<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//OC3CE OC3M[2:0] OC3PE OC3FE CC3S[1:0]
  //TIM1_CCMR4 = (0<<7)|(1<<6)|(1<<5)|(0<<4)|(1<<3)|(1<<2)|(0<<1)|(0<<0);//OC4CE OC4M[2:0] OC4PE OC4FE CC4S[1:0]
  //TIM1_CCER1 = (0<<7)|(0<<6)|(1<<5)|(1<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0);//CC2NP CC2NE CC2P CC2E CC1NP CC1NE CC1P CC1E
  TIM1_CCER1 = (0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(1<<0);//CC2NP CC2NE CC2P CC2E CC1NP CC1NE CC1P CC1E
  //TIM1_CCER2 = (0<<7)|(0<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0);//Reserved Reserved CC4P CC4E CC3NP CC3NE CC3P CC3E
  //TIM1_CNTRH = 0;
  //TIM1_CNTRL = 0;
  TIM1_PSCRH = 0;//(unsigned char)(_PRESCAL1>>8);
  //TIM1_PSCRL = 0;//(unsigned char)_PRESCAL1;
  TIM1_PSCRL = 3;
  //TIM1_ARRH = (unsigned char)(TIM1_MAX>>8);
  //TIM1_ARRL = (unsigned char)(TIM1_MAX);
  TIM1_ARRH = 0x9C;
  TIM1_ARRL = 0x40;
  //TIM1_CCR1H = 0;//(unsigned char)(30000>>8);
  //TIM1_CCR1L = 0;//(unsigned char)(30000);
  TIM1_CCR1H = 0x96;
  TIM1_CCR1L = 0x00;
  TIM1_CCR2H = 0;//(unsigned char)(20000>>8);
  TIM1_CCR2L = 10;//(unsigned char)(20000);
  TIM1_CCR3H = 0;//(unsigned char)(10000>>8);
  TIM1_CCR3L = 0;//(unsigned char)(10000);
  //TIM1_CCR4H = (unsigned char)((TIM1_MAX-10)>>8);//(unsigned char)(5000>>8);
  //TIM1_CCR4L = (unsigned char)(TIM1_MAX-10);//(unsigned char)(5000);
  TIM1_CCR4H = 0;
  TIM1_CCR4L = 0;
  TIM1_BKR = (1<<7)|(1<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0);//MOE AOE BKP BKE OSSR OSSI LOCK[1:0]
  //TIM1_DTR = 0x66;
  TIM1_DTR = 0;
  //TIM1_EGR = (0<<7)|(0<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0);//BG TG COMG CC4G CC3G CC2G CC1G UG
  TIM1_EGR = (0<<7)|(0<<6)|(1<<5)|(0<<4)|(0<<3)|(0<<2)|(1<<1)|(1<<0);//BG TG COMG CC4G CC3G CC2G CC1G UG
  
  TIM1_CR1 = (1<<7)|(1<<6)|(1<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(1<<0);//ARPE CMS[1:0] DIR OPM URS UDIS CEN

}

void EMotorSetPWM(unsigned char value) {
  int compareLevel = 40000 - value * 20;
  TIM1_CCR1H = (unsigned char)(compareLevel >> 8);
  TIM1_CCR1L = (unsigned char)compareLevel ; 
}

//static inline void timer3_init(void)
//{
//  TIM3_CR1 = 0x01;
//  TIM3_IER = (1<<0);
//  //TIM3_CCMR1 = (1<<6)|(1<<5)|(1<<4);
//  //TIM3_CCMR2 = (1<<6)|(1<<5)|(1<<4);
//  TIM3_CCER1 = (1<<5)|(1<<4)|(0<<1)|(1<<0);
//  TIM3_ARRH = (unsigned char)(TIM3_MAX>>8);
//  TIM3_ARRL = (unsigned char)TIM3_MAX;
//
//}

//static inline void INT_priority_init(void)
//{//15 TIM3 Update/ overflow - - 0x00 8044 Level 3, остальные Level 1
//  ITC_SPR1 = (0<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(1<<2)|(0<<1)|(1<<0);//VECT3SPR[1:0] VECT2SPR[1:0] VECT1SPR[1:0] VECT0SPR[1:0]
//  ITC_SPR2 = (0<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(1<<2)|(0<<1)|(1<<0);//VECT7SPR[1:0] VECT6SPR[1:0] VECT5SPR[1:0] VECT4SPR[1:0]
//  ITC_SPR3 = (0<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(1<<2)|(0<<1)|(1<<0);//VECT11SPR[1:0] VECT10SPR[1:0] VECT9SPR[1:0] VECT8SPR[1:0]
//  ITC_SPR4 = (1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(1<<2)|(0<<1)|(1<<0);//VECT15SPR[1:0] VECT14SPR[1:0] VECT13SPR[1:0] VECT12SPR[1:0]
//  ITC_SPR5 = (0<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(1<<2)|(0<<1)|(1<<0);//VECT19SPR[1:0] VECT18SPR[1:0] VECT17SPR[1:0] VECT16SPR[1:0]
//  ITC_SPR6 = (0<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(1<<2)|(0<<1)|(1<<0);//VECT23SPR[1:0] VECT22SPR[1:0] VECT21SPR[1:0] VECT20SPR[1:0]
//  ITC_SPR7 = (0<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(1<<2)|(0<<1)|(1<<0);//VECT27SPR[1:0] VECT26SPR[1:0] VECT25SPR[1:0] VECT24SPR[1:0]
//  ITC_SPR8 = (0<<3)|(1<<2)|(0<<1)|(1<<0);//Reserved VECT29SPR[1:0] VECT28SPR[1:0]
//}

static inline void adc_init(void)
{
//установка ADC clock frequency 1-4MHz(3.3V), 1-6MHz(5V), т.е 2MHz, в то время, когда ADC is in power down.
//000: fADC = fMASTER/2
//001: fADC = fMASTER/3
//010: fADC = fMASTER/4
//011: fADC = fMASTER/6
//100: fADC = fMASTER/8
//101: fADC = fMASTER/10
//110: fADC = fMASTER/12
//111: fADC = fMASTER/18
#if (F_CLK == 16000000) //110: fADC = fMASTER/12 = 1.333MHz
  #define PRESCAL_ADC (1<<6)|(1<<5)|(0<<4)/* Reserved SPSEL[2:0] Reserved(2) CONT ADON */
#elif (F_CLK == 2000000) //000: fADC = fMASTER/2 = 1MHz
  #define PRESCAL_ADC (0<<6)|(0<<5)|(0<<4)/* Reserved SPSEL[2:0] Reserved(2) CONT ADON */
#endif
#if defined ADC_Single_scan_mode
  ADC_CSR = (1<<5)|0x09;//EOC AWD EOCIE AWDIE CH[3:0]
  //ADC_CR2 = (0<<7)|(0<<6)|(0<<5)|(0<<4)|(1<<3)|(0<<2)|(1<<1)|(0<<0);//Reserved EXTTRIG EXTSEL[1:0] ALIGN Reserved SCAN Reserved
  ADC_CR2 = MASK_ADC_CR2_SCAN;
  ADC_CR1 = (0<<7)|PRESCAL_ADC|(0<<3)|(0<<2)|(0<<1)|(1<<0);//Reserved SPSEL[2:0] Reserved(2) CONT ADON
  ADC_CR3 = (1<<7);
  ADC_CR1_ADON = 1;
#elif defined ADC_Continuous_scan_mode
  ADC_CSR = (1<<5)|0x09;//EOC AWD EOCIE AWDIE CH[3:0]
  //ADC_CR2 = (0<<7)|(0<<6)|(0<<5)|(0<<4)|(1<<3)|(0<<2)|(1<<1)|(0<<0);//Reserved EXTTRIG EXTSEL[1:0] ALIGN Reserved SCAN Reserved
  ADC_CR2 = MASK_ADC_CR2_SCAN;
  ADC_CR1 = (0<<7)|PRESCAL_ADC|(0<<3)|(0<<2)|(1<<1)|(1<<0);//Reserved SPSEL[2:0] Reserved(2) CONT ADON
  ADC_CR3 = (1<<7);
  /*ADC_TDRH = 0x00;
  ADC_TDRL = 0x00;
  ADC_HTRH = 0xFF;
  ADC_HTRL = 0x03;
  ADC_LTRH = 0x00;
  ADC_LTRL = 0x00;
  ADC_AWSRH = 0x00;
  ADC_AWSRL = 0x00;
  ADC_AWCRH = 0x00;
  ADC_AWCRL = 0x00;*/
  ADC_CR1_ADON = 1;
#else
  #error  не определён режим работы АЦП
#endif
}

static inline void timer4_init(void)
{
  TIM4_CR1 = (0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2)|(0<<1)|(1<<0);//ARPE Reserved(6-4) OPM URS UDIS CEN
  //TIM4_CR1_CEN = 1;
  //TIM4_IER = 0x01;//Reserved TIE Reserved(5-1) UIE
  TIM4_IER_UIE = 1;
  TIM4_PSCR = 0x07;
  TIM4_ARR = 249; //значение по умолчанию. Прерывание 1 раз в 32000 машинных тактов
  //TIM4_EGR = 0x01; //нужно при UDIS=1
}

void spi_init() {
  /*
  //cs disable - pin pc4 to 1
  PC_CR1_bit.C14 = 1; //PC_CR1 |= MASK_PC_CR1_C14;
  PC_DDR_bit.DDR4 = 1;//PC_DDR |= MASK_PC_DDR_DDR4;
  PC_ODR_bit.ODR4 = 1;//PC_ODR |= MASK_PC_ODR_ODR4; 
  //
  */
  SPI_CR2 =  MASK_SPI_CR2_SSM | MASK_SPI_CR2_SSI;//MASK_SPI_CR2_BDOE |
  SPI_CR1 = MASK_SPI_CR1_SPE | MASK_SPI_CR1_MSTR | 0x38;
}


//==============================================================================
void hard_init(void)
{
  //pins
   PE_DDR_DDR0 = 1; // to output (led)
   PE_ODR_ODR0 = 1; // led OFF
   PE_DDR_DDR7 = 0; // to input (button)
   PE_CR1_C17  = 0;//1; // pull-up
   PE_CR2_C27  = 0; // non-interrupt
   //PD is output for all pins
   PD_CR1 = 0x0F;
   PD_ODR = 0xFF;
   PD_DDR = 0xFF;
   
   
  //end pins 
  GDeviceState.BusyDeviceForWake = 0;
  GDeviceState.mode = 0;
  //INT_priority_init();
  timer1_init();
  //timer3_init();
  init_FnPointer();
  uart_init();
  timer4_init();
  adc_init();
  spi_init();
}

void ButtonFn() {// every 2 ms
  static char btnstate = 0;
  volatile char btn = CheckButton();
  static char tmp = 0;
  if ( btnstate == 0 && btn == 1 ) { //btnPress
    //NextMode();
    LedState(tmp++ & 1);
  }
  btnstate = btn;
  
  /*
  static int i = 1000;
  static char tmp = 0;
  
  volatile char btn = CheckButton();
  if ( --i == 0 ) {
    LedState(tmp++ & 1);
    i = 1000;
  }
  */
  
}

void LedOn() {
  LedState(1);
  UnRegisterFnByName(LedOn);
  RegisterFn(LedOff,mS(300));
}

void LedOff() {
  LedState(0);
  UnRegisterFnByName(LedOff);
  if (GDeviceState.flash) {
    --(GDeviceState.flash);
    RegisterFn(LedOn,mS(300));
  }
}

//======================= прерывания ===========================================
// interrupt from timer even 2ms если тактовая 16МГц
#pragma vector = TIM4_OVR_UIF_vector
__interrupt void Timer4_Overflow(void)
{
  TIM4_SR_UIF = 0;//TIM4_SR = 0x00;//обнуление флага прерывания, иначе постоянно будет сюда заходить, однако
  ProcessFn();
  ButtonFn();
}
//==============================================================================
#pragma vector = TIM1_CAPCOM_CC1IF_vector
__interrupt void Timer_Capture1(void)
{
  TIM1_SR1_CC1IF = 0;
  /*PA_CR1_C15 = 1;
  PA_DDR_DDR5 = 1;
  PA_ODR_ODR5 ^= 1;*/
}
//==============================================================================
//#pragma vector = TIM3_OVR_UIF_vector
//__interrupt void Timer3_UIF(void)
//{
//  static unsigned char i = 0;
//  //static unsigned char flag = 0;
//  union{
//    unsigned int temp16;
//    unsigned char temp08[2];
//  };
//  
//  TIM3_SR1_UIF = 0;
//  i++;
//  if(i == (TIM1_MAX)){
//    i = 0;
//    PA_CR1_C15 = 1;
//    PA_DDR_DDR5 = 1;
//    PA_ODR_ODR5 ^= 1;
//    
//    if((TIM3_CCMR2&(1<<6)) == 0){
//      TIM3_CCMR1 = 0x00;
//      TIM3_CCMR2 = (1<<6)|(1<<5)|(0<<4);
//      //flag = 1;
//    }else{
//      TIM3_CCMR2 = 0x00;
//      TIM3_CCMR1 = (1<<6)|(1<<5)|(0<<4);
//      //flag = 0;
//    }
//  }
//  temp16 = sin[i];
//  TIM3_CCR1H = temp08[0];//(unsigned char)(temp>>8);
//  TIM3_CCR1L = temp08[1];//(unsigned char)temp;
//  temp16 = TIM3_MAX - temp16;
//  TIM3_CCR2H = temp08[0];//(unsigned char)(temp>>8);
//  TIM3_CCR2L = temp08[1];//(unsigned char)temp;
//  
//}
//==============================================================================
#pragma vector = ADC1_EOC_vector
__interrupt void ADC1_EOC(void)
{
#ifdef ADC_DEBUG
  //UART2_DR = ((ADC_CSR & 0x0f)+'0');
#endif
#if defined ADC_Single_scan_mode
  ADC_CSR_EOC = 0;
// * обработка данных АЦП *
  
// * End of обработка данных АЦП *
  ADC_CR1_ADON = 1;//старт АЦП
#elif defined ADC_Continuous_scan_mode
  ADC_CSR = (1<<5)|0x09;//EOC AWD EOCIE AWDIE CH[3:0]
/*
  unsigned char* a = (unsigned char*)&ADC_DB0RH;
  unsigned char* b = (unsigned char*)&Adc_Channel[0];
  unsigned char i;
  for(i = 0; i < 20; i++){
    *(b+i) = *(a+i);
  }
  */
  Adc8_Channel[0] = ADC_DB0RH;
  Adc8_Channel[1] = ADC_DB1RH;
  Adc8_Channel[2] = ADC_DB2RH;
  Adc8_Channel[3] = ADC_DB3RH;
  Adc8_Channel[4] = ADC_DB4RH;
  Adc8_Channel[5] = ADC_DB5RH;
  Adc8_Channel[6] = ADC_DB6RH;
  Adc8_Channel[7] = ADC_DB7RH;
  GDeviceState.ADCRefresh = 255;
  
// * обработка данных АЦП *
  
// * End of обработка данных АЦП *
#else
#error  не определён режим работы АЦП
#endif
  /*unsigned char i = 20;
  while(i-- > 0){
    *(b+i) = *(a+i);
  }*/
#ifdef ADC_DEBUG
  static unsigned char temp = 0;
  temp++;
#endif
}
//==============================================================================

unsigned char Get_Adc_Channel(unsigned char i) // 0..7
{
  return Adc8_Channel[i & 0x07];
}

unsigned char Get_Adc_ChannelW(unsigned char i) // wait fresh value
{
  i &= 0x07;
  while( (GDeviceState.ADCRefresh & (1 << i)) == 0 ) asm("NOP"); 
  GDeviceState.ADCRefresh &= ~(1<<i);
  return Adc8_Channel[i];
}
//----------------------------
  /*switch(ADC_AWSRL){
  default:
  case 6:
    PD_DDR_DDR0 = 1;
    PD_ODR_ODR0 ^= 1;
    break;
  case 7:
    PD_DDR_DDR0 = 1;
    PD_ODR_ODR0 ^= 1;
    break;
  case 8:
    PD_DDR_DDR0 = 1;
    PD_ODR_ODR0 ^= 1;
    break;
  case 9:
    PD_DDR_DDR0 = 1;
    PD_ODR_ODR0 ^= 1;
    break;
  }*/
//----------------------------
void FLASH_ProgramWord(const unsigned char *Address, unsigned char *Data)
{
  //FLASH_CR2_WPRG = 1; //Word program operation enabled
  //FLASH_NCR2_NWPRG = 0;//Word programming enabled
    //FLASH_CR2 |= (1<<6);
    //FLASH_NCR2 &= (~(1<<6));
    *((u8*)Address)       = *((u8*)(Data)); /* Write one byte - from lowest address*/
    *(((u8*)Address) + 1) = *((u8*)(Data)+1); /* Write one byte*/
    *(((u8*)Address) + 2) = *((u8*)(Data)+2); /* Write one byte*/
    *(((u8*)Address) + 3) = *((u8*)(Data)+3); /* Write one byte - from higher address*/
    while(FLASH_IAPSR & 4 == 0);
    //volatile int i = 10000;
    //while(--i);
}

void WriteInProgramMemory(const unsigned char *address, unsigned char *data, unsigned char count) {
  FLASH_PUKR = 0x56; FLASH_PUKR = 0xAE;///Unlock program memory
  while(count >= 4) {
    FLASH_ProgramWord(address, data);
    count -= 4;
    data +=4;
    address += 4;
  }
  while(count > 0) {
   *((u8*)address) = *((u8*)(data)); //Write one byte - from lowest address;
    count--;
    data++;
    address++;
  }
  FLASH_IAPSR = ~MASK_FLASH_IAPSR_PUL;///Lock program memory
}

void SetRezistorAD5204(unsigned char Address, unsigned char value) {
  PC_ODR_bit.ODR4 = 0;// cs enable
  SPI_DR = Address; // set address byte
  while((SPI_SR & MASK_SPI_SR_TXE) == 0);// white TXE =1
  SPI_DR = value; // set data byte
  while((SPI_SR & MASK_SPI_SR_BSY) != 0);// white BSY =0;
  PC_ODR_bit.ODR4 = 1;//cs disable 
}

void SetRezistorCAT5172(unsigned char Address, unsigned char value) { 
  PD_ODR = ~(1<<Address); // cs enable
  //SPI_DR = Address; // set address byte
  //while((SPI_SR & MASK_SPI_SR_TXE) == 0);// white TXE =1;
  SPI_DR = value; // set data byte
  while((SPI_SR & MASK_SPI_SR_TXE) == 0);// white TXE =1;
  while((SPI_SR & MASK_SPI_SR_BSY) != 0);// white BSY =0;
  PD_ODR = 0xFF; // cs enable 
}

char CheckButton(){
// return 1 - button is press , 0 - unpress
  volatile char isPress = PE_IDR_IDR7 ? 0 : 1;
  return isPress;  //isPress;
}

void LedState(char state) {
  // state == 0 -> ledOff else ledOn
  PE_ODR_ODR0 = state ?  0 : 1;
}
