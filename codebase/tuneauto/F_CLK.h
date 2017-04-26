#ifndef F_CLK_H
#define F_CLK_H
//===============================
//#define   CLK_2MHZ_RC
//#define   CLK_16MHZ_RC
//#define   CLK_2MHZ_ZQ
#define   CLK_16MHZ_ZQ
//===============================
#ifdef CLK_16MHZ_ZQ
  #define INIT_CLK    CLK_CKDIVR = 0; CLK_ECKR = 1
  #define F_CLK 16000000UL
#endif
#ifdef CLK_2MHZ_ZQ
  #define INIT_CLK    CLK_CKDIVR = 0x18; CLK_ECKR = 1
  #define F_CLK 2000000UL
#endif
#ifdef CLK_16MHZ_RC
  #define INIT_CLK    CLK_CKDIVR = 0; CLK_ECKR = 0
  #define F_CLK 16000000UL
#endif
#ifdef CLK_2MHZ_RC
  #define INIT_CLK    CLK_CKDIVR = 0x18; CLK_ECKR = 0
  #define F_CLK 2000000UL
#endif

#endif  //F_CLK_H
