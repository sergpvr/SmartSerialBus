#ifndef TUNE_WAKE_H
#define TUNE_WAKE_H

// this is difined in wake.h
// protocol WAKE 

//���� ������������� ������:

//#define CMD_NOP  0    //��� ��������
//#define CMD_ERR  1    //������ ������ ������
//#define CMD_ECHO 2    //�������� ���
//#define CMD_INFO 3    //�������� ���������� �� ����������

#define CMD_TABLE_TRANSFER  4    //�������� ����� �������
#define CMD_TABLE_INIT      5    //����������� �������
#define CMD_EMOTOR          6    //�������� ������ �������������
#define CMD_GETADC          7    //ADC request
#define CMD_TABLE_ADJPOINT  8    //����� �� ����������� ����� �������

void WakeTuneProc();


#endif //TUNE_WAKE_H