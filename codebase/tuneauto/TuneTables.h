#ifndef TUNE_TABLES_H
#define TUNE_TABLES_H

enum LongOpState {
  LOSInit    = 0,
  LOSStart   = 1,
  LOSFinish  = 2,
};

typedef struct RequireAdjustPoint {
  unsigned char TableNumber;
  unsigned char Point;
  enum LongOpState State;
  unsigned char Rdata; 
  char limit;
}RequireAdjustPoint;

typedef struct TuneTablesBaton {
  //unsigned char is_init_table_req; //bool value
  //unsigned char TableNumber_for_req;
  //unsigned char cur_point;
  RequireAdjustPoint AdjustPoint;
}TuneTablesBaton;

extern TuneTablesBaton tt_baton;

extern const unsigned char tunetable0[];
extern const unsigned char tunetable1[];
extern const unsigned char tunetable2[];
extern const unsigned char tunetable3[];
const unsigned char * GetTuneTable(unsigned char i);
//void TableUtilitiesInitReq(TuneTablesBaton *tt_baton, unsigned char TableNumber);
void TableUtilitiesInit(void); 
void TableUtilities(void); // cyclical call 

#endif //TUNE_TABLES_H