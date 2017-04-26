using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Threading;


namespace tune
{
    public enum enRWmode : byte
    {
        read = 0,
        write = 1
    }

    public class TuneSerialPort
    {
        SerialPort port;

        public TuneSerialPort(string PortName)
        {
            port = new System.IO.Ports.SerialPort();
            port.PortName = PortName;
            port.WriteTimeout = 500;
            port.ReadTimeout = 500;
            port.BaudRate = 9600;
            port.Parity = Parity.None;
            port.DataBits = 8;
            port.StopBits = StopBits.One;
            //port.Handshake = Handshake.RequestToSend;
            port.DtrEnable = false;
            port.RtsEnable = false;
            //port.NewLine = System.Environment.NewLine; 
            //port.Open();
        }

        ~TuneSerialPort()
        {
            if( port.IsOpen ) port.Close();
        }

        public bool uart_putchar(byte ch)
        {
            byte[] buf = { ch };
            port.Write(buf, 0, 1);
            return true;
        }

        public bool uart_putbuf(List<byte> buf)
        {
            //byte[] buf = { ch };
            //port.Write(buf, 0, 1);
            byte[] txbuf = buf.ToArray();
            port.Write(txbuf, 0, txbuf.Length);
            return true;
        }

        public int uart_getchar()
        {
            return (port.BytesToRead != 0) ? port.ReadByte() : -1;
        }

        public void uart_flash()
        {
            port.DiscardInBuffer();
        }

        public virtual void Close()
        {
            port.Close();
        }

        public void Open()
        {
            port.Open();
        }

    }



    public class WakePacket 
    {
      public int Address = 0;
      public int Command = 0;
      public int CntData = 0;
      public List <byte> data = new List<byte>();
      public int CodeErr = 0;
      //
      //public void SetDataStr(string s)
      //{
      //    data.Clear();
      //    char [] buf = s.ToCharArray();
      //    foreach (char ch in buf) data.Add((byte)ch);
      //    data.Add(0);
      //    CntData = data.Count;
      //}

      public string GetDataStr()
      {
          if(data.Count != CntData) return "<err />";
          char[] buf = new char[CntData];
          int i = 0;
          foreach (byte bt in data) 
          { 
              buf[i++] = (char)bt;
          }
          return new string(buf);
      }

      public void SetDataStr(string s)
      {
          data.Clear();
          foreach (char ch in s)
          {
              data.Add((byte)ch);
          }
          data.Add(0);
          CntData = data.Count;
      }

    }

  

    public class CWake : TuneSerialPort
    {
        const int FRAME = 255;
        const int CRC_INIT  =  0xDE;  //Innitial CRC value
        //SLIP protocol
        const int FEND = 0xC0; // Frame End
        const int FESC = 0xDB; // Frame Escape
        const int TFEND = 0xDC; // Transposed Frame End
        const int TFESC = 0xDD; // Transposed Frame Escape
        //
        //Коды ошибок:

        public const int ERR_NO = 0x00;   //no error
        public const int ERR_TX = 0x01;   //Rx/Tx error
        public const int ERR_BU = 0x02;   //device busy error
        public const int ERR_RE = 0x03;   //device not ready error
        public const int ERR_PA = 0x04;   //parameters value error
        public const int ERR_NR = 0x05;   //no replay
        public const int ERR_NC = 0x06;   //no carrier

        FileStream debug_fslog;
        StreamWriter debug_swlog;

        public CWake(string PortName): base(PortName)
        {
            debug_fslog = new FileStream("logTune.txt", FileMode.OpenOrCreate, FileAccess.Write);
            debug_swlog = new StreamWriter(debug_fslog);
        }

        public override void Close()
        {
            base.Close();
            debug_swlog.Close();
            debug_fslog.Dispose();
        }

        //--------------------- CRC ------------------------
        private void Do_Crc8(int b, ref int crc)
        {
            for (int i = 0; i < 8; b = b >> 1, i++)
            {
                if (((b ^ crc) & 1) == 1)
                {
                    crc = ((crc ^ 0x18) >> 1) | 0x80;
                }
                else
                {
                    crc = (crc >> 1) & 0x7F;//~0x80;
                }
            }
        }

        private List<byte> TranslCharSLIP(int ch)
        {
          List<byte> result = new List<byte>();
          if(ch == FEND)
          {
            result.Add(FESC);
            result.Add(TFEND);
            //uart_putchar(FESC);
            //uart_putchar(TFEND);
          }
          else if (ch == FESC)
          {
            result.Add(FESC);
            result.Add(TFESC);
            //uart_putchar(FESC);
            //uart_putchar(TFESC);    
          }
          else
          {
            result.Add((byte)ch);
            //uart_putchar((byte)ch);
          }
          return result;
        }

        public bool WakeTX(WakePacket wp)
        {
          List<byte> bufTX = new List<byte>();
          int crc = CRC_INIT;
          Do_Crc8(FEND, ref crc);
          //uart_putchar(FEND); // start packet
          bufTX.Add(FEND);
          //#ifdef MultiDevice 
          if(wp.Address != 0)
          {
            Do_Crc8(wp.Address, ref  crc);
            //SendCharSLIP(wp.Address | 0x80); // send address
            bufTX.AddRange(TranslCharSLIP(wp.Address | 0x80));
          }
          //#endif
          Do_Crc8(wp.Command, ref crc);
          //SendCharSLIP(wp.Command); // send command
          bufTX.AddRange(TranslCharSLIP(wp.Command));
          Do_Crc8(wp.CntData, ref  crc);
          //SendCharSLIP(wp.CntData); // send N
          bufTX.AddRange(TranslCharSLIP(wp.CntData));
          foreach (byte dt in wp.data)
          {
            Do_Crc8((int)dt, ref crc);
            //SendCharSLIP(dt); // send data byte 
            bufTX.AddRange(TranslCharSLIP((int)dt));
          }
          //SendCharSLIP(crc); // send crc
          bufTX.AddRange(TranslCharSLIP(crc));
          //write in log
          debug_swlog.WriteLine("->WakeTX_START");
          foreach (byte bt in bufTX) debug_swlog.Write("| {0}", bt);
          debug_swlog.WriteLine("\n->WakeTX_END");
          debug_swlog.Flush();
          //
          uart_putbuf(bufTX);
          return true;
        }

        enum STATE_RX
        {
          rxBEGIN=0,
          rxSTARTPACKET,
          rxADDRESS,
          rxCOMMAND,
          rxDATA,
          rxCRC
        };

        public bool WakeRX(int timeout, out WakePacket wp)
        {
            debug_swlog.WriteLine("<-WakeRX_START");
            //List<int> btest = new List<int>();
            int times = timeout / 10;
            bool result = false; 
            wp = new WakePacket();
            // for byte stuffing
            bool flagFESC = false;
            //
            int crc = CRC_INIT;
            int rcv;
            STATE_RX state = STATE_RX.rxBEGIN;
            do
            {
                rcv = uart_getchar();
                //btest.Add(rcv);
                if (times-- == 0)
                {
                    wp.CodeErr = CWake.ERR_TX;
                    break;
                }
                if (rcv == -1)
                {
                    if (times-- == 0) break;
                    System.Threading.Thread.Sleep(10);
                    continue;
                }
                //write in log
                debug_swlog.Write("| {0}",rcv);

                if (rcv == FEND) state = STATE_RX.rxBEGIN;

                // byte stuffing
                if(rcv == FESC && flagFESC == false)
                {
                    flagFESC = true;
                    continue;
                }
                if (flagFESC == true) 
                {
                flagFESC = false;
                if (rcv == TFEND) rcv = FEND;
                else if (rcv == TFESC) rcv = FESC;
                }
                // end byte stuffing
              
                switch(state)
                {
                case  STATE_RX.rxBEGIN:
                    if(rcv == FEND)
                    {
                        state = STATE_RX.rxSTARTPACKET;
                        crc = CRC_INIT;
                        Do_Crc8(FEND, ref crc);
                    }
                    break;
                case STATE_RX.rxSTARTPACKET:
                    if((rcv & 0x80) != 0) 
                    {
                        state = STATE_RX.rxADDRESS;
                        wp.Address = rcv & 0x7F;
                        Do_Crc8(wp.Address, ref crc);
                    }
                    else
                    {
                        state = STATE_RX.rxCOMMAND;
                        wp.Address = 0;
                        wp.Command = rcv;
                        Do_Crc8(rcv, ref crc);
                    }
                    break;
                case STATE_RX.rxADDRESS:
                    state = STATE_RX.rxCOMMAND;
                    wp.Command = rcv;
                    Do_Crc8(rcv, ref crc);     
                    break;
                case STATE_RX.rxCOMMAND: // receive CntData
                    wp.data.Clear();
                    state = (rcv != 0) ? STATE_RX.rxDATA : STATE_RX.rxCRC;
                    wp.CntData = rcv;
                    if(rcv > FRAME) // err: packet is very long
                    {
                        wp.CodeErr = ERR_TX;
                        //state = rxBEGIN;
                        result = true;
                    }
                    Do_Crc8(rcv, ref crc);     
                    break; 
                case STATE_RX.rxDATA:
                    wp.data.Add((byte)rcv);
                    Do_Crc8(rcv, ref crc);
                    if (wp.data.Count == wp.CntData) 
                    {
                        state = STATE_RX.rxCRC;
                    }
                    break;
                case STATE_RX.rxCRC:
                    wp.CodeErr = (rcv == crc) ? ERR_NO : ERR_TX;
                    //state = rxBEGIN;
                    result = true;
                    break;       
                }
             } while (result == false);
            debug_swlog.WriteLine("\n<-WakeRX_END: paket={0}", result);
            debug_swlog.Flush();
             return result;
        }
    }

    public class ProgressEventArgs : EventArgs
    {
        public int ProgressPercentage; // 0 .. 100
        public ProgressEventArgs(int ProgressPercentage)
        {
            this.ProgressPercentage = ProgressPercentage;
        }
    }	//end of class ProgressEventArgs

    public delegate void ProgressEventHandler(object sender, ProgressEventArgs e);

    public class FinishAdjTableEventArgs : EventArgs
    {
        public List<string> log;
        public byte[] table;
        public int TblNumber;
        public FinishAdjTableEventArgs(List<string> log, byte[] table, int tableNumber)
        {
            this.log = log;
            this.table = table;
            this.TblNumber = tableNumber;
        }
    }	//end of class FinishAdjTableEventArgs

    public delegate void FinishAdjTableEventHandler(object sender, FinishAdjTableEventArgs e);


    public enum WakeCommand
    {
        //Коды универсальных команд:
        NOP = 0,    //немає операції
        ERR = 1,    //помилка прийому пакета
        ECHO = 2,    //передати ехо
        INFO = 3,    //передати информацію про пристрій
        //
        TABLE = 4,    //передача таблиці
        TABLE_INIT = 5, // init table
        EMOTOR = 6, //передача команд електромотора
        ADCGET = 7, // запит ADC
        TABLE_ADJPOINT = 8    //запит на ініціалізацію точки таблиці
    }

    public class TuneDeviceController
    {
        CWake wake;

        public TuneDeviceController(CWake _wake)
        {
            wake = _wake;
            wake.uart_flash();
        }

        public void SetEMotorPWM (byte btValue) 
        {
            WakePacket wp = new WakePacket();
            wp.Command = (int)WakeCommand.EMOTOR;
            wp.data.Add((byte)1); //Set PWM (0 - Get PWM) 
            wp.data.Add(btValue);
            wp.CntData = wp.data.Count;
            wake.WakeTX(wp);
            WakePacket wp_rx;
            bool rp = wake.WakeRX(1000, out wp_rx);
            return;           
        }

        private int OnesCounterInByte(byte Value)
        {
            int result = 0;
            int tempVal = (int)Value;
            for (int i = 0; i < 8; ++i)
            {
                if ( ( tempVal & (1 << i) ) != 0 ) ++result;
            }
            return result;
        }

        public byte[] GetADC(byte ADCset)
        {
            WakePacket wp = new WakePacket();
            wp.Command = (int)WakeCommand.ADCGET;
            wp.data.Add(ADCset); 
            wp.CntData = wp.data.Count;
            wake.WakeTX(wp);
            WakePacket wp_rx;
            bool rp = wake.WakeRX(1000, out wp_rx);
            byte [] result = new byte[8];
            bool goodRXPacket = false;
            if (rp == true && wp.CodeErr == 0 && wp.Command == (int)WakeCommand.ADCGET) {
                if (OnesCounterInByte(ADCset) == wp_rx.CntData)
                {
                    goodRXPacket = true;
                    int n = 0;
                    for (int i = 0; i < 8; ++i)
                    {
                        if (((int)ADCset & (1 << i)) != 0)
                        {
                            result[i] = wp_rx.data[n++];
                        }
                    }

                }
            }
            if (!goodRXPacket) throw new Exception("error ADC RXPacket");
            return result;
        }
    }

    public class TableTransceiver
    {
        const int BlockSize = 4;  
        public event ProgressEventHandler Progress;
        protected void OnProgress(ProgressEventArgs e)
        {
            if (Progress != null)
                Progress(this, e);
        }

        CWake wake;

        public TableTransceiver(CWake _wake)
        {
            wake = _wake;
        }

        public class TableDataUnit
        {
            public enRWmode RWmode;
            public int TableNumber; // 0..255
            public int Offset;      // 0..255
            public int CountByteReq;   // 0..255
            public byte[] Data;
        }


        public static WakePacket Translate(TableDataUnit tdu)
        {
            WakePacket wp = new WakePacket();
            wp.Command = (int)WakeCommand.TABLE;
            wp.data.Add((byte)tdu.RWmode);
            wp.data.Add((byte)tdu.TableNumber);
            wp.data.Add((byte)tdu.Offset);
            if (tdu.RWmode == enRWmode.write)
            {
                wp.data.Add((byte)tdu.Data.Length);
                foreach (byte b in tdu.Data)
                {
                    wp.data.Add(b);
                }
            }
            else // read
            {
                wp.data.Add((byte)tdu.CountByteReq);
            }
            wp.CntData = wp.data.Count;
            return wp;
        }

        public static TableDataUnit Translate(WakePacket wp)
        {
            TableDataUnit tdu = new TableDataUnit();
            tdu.RWmode = ( wp.data[0] == (byte)enRWmode.read ) ? enRWmode.read : enRWmode.write;
            tdu.TableNumber = wp.data[1];
            tdu.Offset = wp.data[2];
            if (tdu.RWmode == enRWmode.write)
            {
                tdu.Data = new byte[wp.data[3]];
                for (int i = 4; i < wp.data.Count; i++)
                {
                    tdu.Data[i - 4] = wp.data[i];
                }
            }
            else //read
            {
                tdu.CountByteReq = wp.data[3];
            }
            return tdu;
        }

        public void SendTable(int TableNumber, byte[] table)
        {
            int cnt = table.Length / BlockSize;
            int leftover = table.Length % BlockSize;
            TableDataUnit tdu = new TableDataUnit();
            tdu.RWmode = enRWmode.write;
            tdu.TableNumber = TableNumber;
            tdu.Data = new byte[BlockSize];
            for (int i = 0; i < cnt; i++)
            {
                tdu.Offset = i * BlockSize;
                for (int j = 0; j < BlockSize; j++)
                {
                    tdu.Data[j] = table[tdu.Offset + j];
                }
                //send
                SendTableBlock(tdu);
                OnProgress(new ProgressEventArgs((i * 100)/cnt));
            }
            if (leftover != 0)
            {
                tdu.Data = new byte[leftover];
                tdu.Offset = cnt * BlockSize;
                for (int i = 0; i < leftover; i++)
                {
                    tdu.Data[i] = table[table.Length - leftover + i];
                }
                //send
                SendTableBlock(tdu);
            }
            OnProgress(new ProgressEventArgs(100));
        }

        void SendTableBlock(TableDataUnit tdu)
        {
            wake.uart_flash();
            WakePacket wp = Translate(tdu);
            WakePacket wp_rx;
            bool dataTransferSuccess;
            int i = 3; // number of attempts
            do
            {
                wake.WakeTX(wp);
                bool rp = wake.WakeRX(1000, out wp_rx);
                dataTransferSuccess = rp == true && wp_rx.CodeErr == 0 &&
                                     wp_rx.Command == (int)WakeCommand.TABLE &&
                                     wp_rx.CntData == 1 && wp_rx.data[0] == 0;
            } while (!dataTransferSuccess && (--i > 0));
            if (i == 0)
            {
                throw new Exception("error data transfer");
            }
        }

        public byte[] ReceiveTable(int TableNumber, int TableSize)
        {
            byte[] result = new byte[TableSize];
            int cnt = TableSize / BlockSize;
            int leftover = TableSize % BlockSize;
            TableDataUnit tdu = new TableDataUnit();
            tdu.RWmode = enRWmode.read;
            tdu.TableNumber = TableNumber;
            tdu.Data = new byte[0];
            tdu.CountByteReq = BlockSize;
            TableDataUnit tdu_rx;
            for (int i = 0; i < cnt; i++)
            {
                tdu.Offset = i * BlockSize;
                //receive
                tdu_rx = ReceiveTableBlock(tdu);
                for (int j = 0; j < tdu_rx.Data.Length; j++)
                {
                    result[tdu.Offset + j] = tdu_rx.Data[j];     
                }
                OnProgress(new ProgressEventArgs((i * 100) / cnt));
            }
            if (leftover != 0)
            {
                tdu.CountByteReq = leftover;
                tdu.Offset = cnt * BlockSize;
                //receive
                tdu_rx = ReceiveTableBlock(tdu);
                for (int j = 0; j < tdu_rx.Data.Length; j++)
                {
                    result[tdu.Offset + j] = tdu_rx.Data[j];
                }
            }
            OnProgress(new ProgressEventArgs(100));
            return result;
        }

        TableDataUnit ReceiveTableBlock(TableDataUnit tdu)
        {
            wake.uart_flash();
            WakePacket wp = Translate(tdu);
            WakePacket wp_rx;
            bool dataTransferSuccess;
            int i = 1; // number of attempts 3
            do
            {
                wake.WakeTX(wp);
                bool rp = wake.WakeRX(1000, out wp_rx);
                dataTransferSuccess = rp == true && wp_rx.CodeErr == 0 &&
                                     wp_rx.Command == (int)WakeCommand.TABLE &&
                                     (wp_rx.CntData - 4) == tdu.CountByteReq;
            } while (!dataTransferSuccess && (--i > 0));
            if (i == 0)
            {
                throw new Exception(
                    string.Format("error data transfer(read table block): table={0}, offset={1}, datareq={2}",
                        tdu.TableNumber,tdu.Offset,tdu.CountByteReq));
            }
            TableDataUnit tdu_rx = Translate(wp_rx);
            if (tdu_rx.RWmode != enRWmode.write) throw new Exception("error read data from device");
            return tdu_rx;
        }
    }

    public class TableAdjustment
    {
        const byte HALFRANGE = 2;
        bool Cancel = false;
        public byte[] ProbabilityTable;
        public byte[] ResultAdjTable;

        List<string> log = new List<string>();

        public event ProgressEventHandler Progress;
        protected void OnProgress(ProgressEventArgs e)
        {
            if (Progress != null)
                Progress(this, e);
        }

        public event FinishAdjTableEventHandler FinishAdjTable;
        protected void OnFinishAdjTable(FinishAdjTableEventArgs e)
        {
            if (FinishAdjTable != null)
                FinishAdjTable(this, e);
        }

        CWake wake;
        int TableNumber;
        byte[] PredictedData;

        public TableAdjustment(CWake _wake, int _TableNumber, byte[] _PredictedData)
        {
            wake = _wake;
            TableNumber = _TableNumber;
            PredictedData = _PredictedData;
        }

        private bool InitAdj()
        {
            WakePacket wp = new WakePacket();
            wp.Command = (int)WakeCommand.TABLE_ADJPOINT;
            wp.data.Add((byte)enRWmode.read);
            wp.CntData = wp.data.Count;
            int atempt = 2;
            WakePacket wp_rx;
            byte state = 0xff;
            do
            {
                wake.WakeTX(wp);
                bool rp = wake.WakeRX(500, out wp_rx);
                if (rp && wp_rx.CntData == 1)
                {
                    state = wp_rx.data[0];
                    if (state == CWake.ERR_PA)
                    {
                        log.Add(String.Format("пристрій готовий до ініціалізації таблиці, модуль  в стані {0}", wp_rx.data[0]));
                        return true;
                    }
                }
            } while (--atempt != 0);
            log.Add(String.Format("помилка ініціалізації таблиці, модуль  в стані {0}", state));
            return false;
        }

        private KeyValuePair<bool, byte> PointAdj(int TableNumber, byte Point)
        {
            KeyValuePair<bool, byte> result = new KeyValuePair<bool, byte>(false, 0);
            byte Rdata;
            WakePacket wp = GetReqForAdjust(TableNumber, Point, Point, HALFRANGE);
            wake.WakeTX(wp);
            WakePacket wp_rx;
            bool rp = wake.WakeRX(500, out wp_rx);
            if (rp && wp_rx.CntData == 1)
            {
                if (wp_rx.data[0] == CWake.ERR_NO)
                {
                    // запит прийнятий, точка регулюється
                    wp.data.Clear();
                    wp.data.Add((byte)enRWmode.read);
                    wp.CntData = wp.data.Count;
                    int times = 20;
                    do
                    {
                        Thread.Sleep(TimeSpan.FromMilliseconds(50));
                        wake.WakeTX(wp);
                        rp = wake.WakeRX(500, out wp_rx);
                        if (rp)
                        {
                            if (wp_rx.CntData == 5 && wp_rx.data[0] == CWake.ERR_NO) // отримали відповідь
                            {
                                //  |State| N table | Point  | Rdata | Limit     |
                                if (wp_rx.data[1] == TableNumber && wp_rx.data[2] == Point)
                                {
                                    Rdata = wp_rx.data[3];
                                    result = new KeyValuePair<bool, byte>(true, Rdata);
                                    int Limit = wp_rx.data[4];
                                    log.Add(String.Format("результат налаштування точки {1} таблиці {0}: Rdata={2}, Limit={3}", TableNumber, Point, Rdata, Limit));
                                }
                                else
                                {
                                    log.Add(String.Format("прилад віддав невірні дані на запит результату при ініціалізації таблиці {0} в точці {1}", TableNumber, Point));
                                }
                                break;
                            }
                        }
                        else if (wp_rx.CodeErr == CWake.ERR_TX)
                        {
                            log.Add(String.Format("прилад не відповів на запит результату при ініціалізації таблиці {0} в точці {1}", TableNumber, Point));
                            break;
                        }

                    } while (--times != 0);
                    if (times == 0)
                    {
                        log.Add(String.Format("час вичерпано при ініціалізаціїї таблиці {0} в точці {1}, модуль в стані {2}",TableNumber, Point, wp_rx.data[0]));
                    }
                }
                else
                {
                    log.Add(String.Format("помилка {2} при ініціалізації таблиці {0} в точці {1}", TableNumber, Point, wp_rx.data[0]));
                }
            }
            else
            {
                string s;
                if (!rp) s = "прилад не відповів";
                else s = "невірний формат відповіді";
                s += " на запит при ініціалізації таблиці {0} в точці {1}";
                log.Add(String.Format(s, TableNumber, Point));
            }
            return result;
        }


        private byte[] AdjustTable(int TableNumber, byte[] table)
        {
            //byte[] table - таблиця ймовірних точок, з яких потрібно починати шукати результат
            byte[] result = new byte[table.Length];
            // підготовчі дії
            bool Cancel = false;
            log.Clear();
            wake.uart_flash();
            if (InitAdj())
            {
                for (int i = 0; i < table.Length; ++i)
                {
                    // регулюємо для кожної точки
                    KeyValuePair<bool, byte> pointResult = PointAdj(TableNumber, table[i]);
                    if (pointResult.Key == false) break;
                    else result[i] = pointResult.Value;
                    OnProgress(new ProgressEventArgs(((i + 1) * 100) / table.Length));
                    if (Cancel) break;
                }
            }
            else
            {
                log.Add("Неможливо підготовити пристрій для ініціалізації таблиці");
            }
            OnFinishAdjTable(new FinishAdjTableEventArgs(log, result, TableNumber));
            return result;
        }

        public void AdjustTable()
        {
            AdjustTable(TableNumber, PredictedData);
            //TODO - викоритсати дані
            /*for (int i = 0; i < 10; i++)
            {
                OnProgress(new ProgressEventArgs((i + 1) * 10));
            }*/
        }

        private WakePacket GetReqForAdjust(int TableNumber, byte Point, byte Rdata, byte HalfRange)
        {
            WakePacket wp = new WakePacket();
            wp.Command = (int)WakeCommand.TABLE_ADJPOINT;
            wp.data.Add((byte)enRWmode.write);
            wp.data.Add((byte)TableNumber);
            wp.data.Add(Point);
            wp.data.Add(Rdata);
            wp.data.Add(HalfRange);
            wp.CntData = wp.data.Count;
            return wp;
        }

    }

}
