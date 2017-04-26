
#include "RegisterFn.h"

struct fc
{
  FnPointer fp;     //function pointer
  unsigned int tc; //time call
  unsigned int cnt; //counter
};

static struct fc fn[REGFNCNT];


void init_FnPointer(void)
{
  unsigned char i;
  for( i = 0; i < REGFNCNT; i++){
    fn[i].fp = 0;
  }
}

signed char RegisterFn(FnPointer fp, unsigned int tc)
{
  unsigned char i;
  for( i = 0; i < REGFNCNT; i++){
    if( fn[i].fp == 0 ){
      fn[i].tc = tc;
      fn[i].cnt = tc;
      fn[i].fp = fp; 
      return (signed char)i;
    }
  }
  return -1;
}

void UnRegisterFn(signed char i)
{
  if (i < REGFNCNT) fn[i].fp = 0;
}

void UnRegisterFnByName(FnPointer fp)
{
  unsigned char i;
  for( i = 0; i < REGFNCNT; i++){
    if( fn[i].fp == fp ){
      fn[i].fp = 0;
      break;
    }
  }
}

void ProcessFn(void)
{
  unsigned char i;
  for( i = 0; i < REGFNCNT; i++ ){
    if( fn[i].fp != 0 ){
      if((fn[i].cnt)-- == 0)
      {
        fn[i].cnt = fn[i].tc;
        fn[i].fp();
      }
    }
  }
}