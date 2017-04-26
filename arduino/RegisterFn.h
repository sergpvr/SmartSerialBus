
#ifndef RegisterFn_H
#define RegisterFn_H

#define REGFNCNT 8 // max count function for register

typedef void (*FnPointer)(void);
//
void init_FnPointer(void);
//
signed char RegisterFn(FnPointer fp, unsigned int tc);
void UnRegisterFn(signed char i);
void UnRegisterFnByName(FnPointer fp);
//
void ProcessFn(void); // call registered function

#endif //RegisterFn_H
