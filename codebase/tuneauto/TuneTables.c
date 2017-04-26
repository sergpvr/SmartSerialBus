#include "TuneTables.h"
#include "hard.h"

#define HALF_RANGE 2

/* tune tables globals */



const unsigned char tunetable0[] = {
     0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,
    80,  85,  90,  95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155,
   160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235,
   240, 245, 250
}; 

const unsigned char tunetable1[] = {
     0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,
    80,  85,  90,  95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155,
   160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235,
   240, 245, 250
}; 

const unsigned char tunetable2[] = {
     0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,
    80,  85,  90,  95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155,
   160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235,
   240, 245, 250
}; 

const unsigned char tunetable3[] = {
     0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,
    80,  85,  90,  95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155,
   160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235,
   240, 245, 250
}; 

/*  
const char tunetable0[] = {
     0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
    32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
    48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
    96,  97,  98,  99,  50, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
   112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
   128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
   144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
   160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
   176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
   192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
   208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
   224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
   240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};
*/

const unsigned char * GetTuneTable(unsigned char  i)
{
   switch(i)
   {
   case 0: return tunetable0;
   case 1: return tunetable1;
   case 2: return tunetable2;
   case 3: return tunetable3;
   default: return 0;   
   } 
}
/*
// counting average value
#define MAX_CNT_PROBE 6

typedef struct TAverageVal {
  unsigned char adc_val[MAX_CNT_PROBE];
  signed char state;
} TAverageVal;

inline void AverageVal_Init(TAverageVal *av) {
  av->state = MAX_CNT_PROBE - 1;
}

unsigned char AverageVal_SetData(TAverageVal *av, unsigned char data) {
  if(av->state >= 0) av->adc_val[(av->state)--] = data;
  return av->state < 0 ? 1 : 0; // return 1 if array is full
}

unsigned char AverageVal_GetPosMin(const TAverageVal *av) {
  unsigned char pos = 0;
  unsigned char data =  av->adc_val[0];
  for(unsigned char i = 1; i < MAX_CNT_PROBE; i++) 
    if ( av->adc_val[i] < data) 
       pos = i, data =  av->adc_val[i];
  return pos; 
}

unsigned char AverageVal_GetPosMax(const TAverageVal *av) {
  unsigned char pos = 0;
  unsigned char data =  av->adc_val[0];
  for(unsigned char i = 1; i < MAX_CNT_PROBE; i++) 
    if ( av->adc_val[i] > data) 
       pos = i, data =  av->adc_val[i];
  return pos; 
}

unsigned char AverageVal_GetData(const TAverageVal *av) {
  unsigned char it_min = AverageVal_GetPosMin(av);
  unsigned char it_max = AverageVal_GetPosMax(av);
  int sum = 0;
  for(unsigned char i = 0; i < MAX_CNT_PROBE; i++) {
    if ( (i != it_min) && (i != it_max) ) sum += av->adc_val[i];
  }
  unsigned char adata = (unsigned char)(sum  / (MAX_CNT_PROBE - 2));
  return adata;
}

//
char IsPointSet(unsigned char ResistorAddress, unsigned char point) {
// return -1 if measured_value < point
// return 1 if measured_value > point
// return 0 if measured_value == point (or in range)
  signed char result = 0;
  TAverageVal av_ob;
  unsigned char ADC_Val;  // value from ADC
  unsigned char measured_value; // average value
  
  AverageVal_Init(&av_ob); 
  do {
    ADC_Val = Get_Adc_ChannelW(ResistorAddress);// отримати значення з ADC
  }while ( AverageVal_SetData(&av_ob, ADC_Val) );
  measured_value =  AverageVal_GetData(&av_ob); 
  if ( point >= HALF_RANGE && measured_value < (point-HALF_RANGE) ) result = -1;
  else if ( point < (255-HALF_RANGE) && measured_value > (point+HALF_RANGE) ) result = 1;
  
  return result;  
}

char AdjustPoint(unsigned char ResistorAddress,unsigned char point, unsigned char* Rdata) {
  *Rdata = point;
  signed char compare;
  do {
    SetDigRezistor(ResistorAddress, *Rdata);
    compare = IsPointSet(ResistorAddress, point);
    if ( compare > 0 ) {
      if(*Rdata == 0) return -1;
      --(*Rdata);
    }
    else if ( compare < 0 ) {
      if(*Rdata == 255) return -1;
      ++(*Rdata);
    }
  } while(compare != 0 );
  return 0;
}

void TableUtilitiesInitReq(TuneTablesBaton *itt_baton, unsigned char TableNumber)
{
    itt_baton->TableNumber_for_req = TableNumber;
    itt_baton->cur_point = 0;
    itt_baton->is_init_table_req = 1;
    GDeviceState.BusyDeviceForWake = 1; // device is busy until the end initialization of table   
}

void InitBeginTable (TuneTablesBaton *itt_baton) { //переробити щоб виходило не пыдвисало до кынця роботи буде викликатись циклічно
  const unsigned char *TuneTable = GetTuneTable(itt_baton->TableNumber_for_req);
  if (TuneTable != 0) 
  {
      unsigned char Rdata;
      unsigned char point;
      if(itt_baton->cur_point <51)
      {
        point = *(TuneTable + itt_baton->cur_point); 
        AdjustPoint(itt_baton->TableNumber_for_req, point, &Rdata);
        WriteInProgramMemory(TuneTable + itt_baton->cur_point, &Rdata, 1);
        ++(itt_baton->cur_point);
        return;
      } 
  }
  itt_baton->is_init_table_req = 0; // it  removes request to the initialization of table 
  GDeviceState.BusyDeviceForWake = 0; // this allows you to receive new wake-commands
}
*/
TuneTablesBaton tt_baton;

void TableUtilitiesInit(void) {
  //tt_baton.is_init_table_req = 0;
  tt_baton.AdjustPoint.State = LOSInit;
}

void NoBl_AdjustPoint(RequireAdjustPoint* pAdjustPoint) { 
  SetDigRezistor(pAdjustPoint->TableNumber, pAdjustPoint->Rdata);
  Get_Adc_ChannelW( pAdjustPoint->TableNumber + 4 );
  unsigned char measured_value = Get_Adc_ChannelW( pAdjustPoint->TableNumber + 4 );
  signed char compare = 0;
  unsigned char point = pAdjustPoint->Point;
  
  if ( point >= HALF_RANGE && measured_value < (point-HALF_RANGE) ) compare = -1;
  else if ( point < (255-HALF_RANGE) && measured_value > (point+HALF_RANGE) ) compare = 1;
  pAdjustPoint->limit = 0;
  if ( compare > 0 ) {
    if(pAdjustPoint->Rdata == 0) pAdjustPoint->limit = -1;
    else { --(pAdjustPoint->Rdata); }
  }
  else if ( compare < 0 ) {
    if(pAdjustPoint->Rdata == 255) pAdjustPoint->limit = +1;
    else { ++(pAdjustPoint->Rdata); }
  }
  else {
    pAdjustPoint->State = LOSFinish;
  }
  if (pAdjustPoint->limit) {
    pAdjustPoint->State = LOSFinish;
  } 
  
}

void TableUtilities(void) { //cyclical call
  if ( tt_baton.AdjustPoint.State == LOSStart ) {
    NoBl_AdjustPoint(&(tt_baton.AdjustPoint));
  }
}

