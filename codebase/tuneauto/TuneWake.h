#ifndef TUNE_WAKE_H
#define TUNE_WAKE_H

// this is difined in wake.h
// protocol WAKE 

//Коды универсальных команд:

//#define CMD_NOP  0    //нет операции
//#define CMD_ERR  1    //ошибка приема пакета
//#define CMD_ECHO 2    //передать эхо
//#define CMD_INFO 3    //передать информацию об устройстве

#define CMD_TABLE_TRANSFER  4    //передача даних таблиці
#define CMD_TABLE_INIT      5    //ініціалізація таблиці
#define CMD_EMOTOR          6    //передача команд електромотора
#define CMD_GETADC          7    //ADC request
#define CMD_TABLE_ADJPOINT  8    //запит на ініціалізацію точки таблиці

void WakeTuneProc();


#endif //TUNE_WAKE_H