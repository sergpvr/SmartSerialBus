package com.epam.communication.wakeUp;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * WakePacket encapsulates received data or transferred data and methods for
 * serial remote interaction
 * 
 * @author Serhiy_Povoroznyuk
 *
 */
public class WakePacket {
    private Byte address = 0;
    private Byte command = 0;
    private List<Byte> data = new ArrayList<Byte>();
    private Byte CodeErr = 0;
    // for RX logic
    private boolean flagFESC = false; // for byte stuffing
    private RxState state = RxState.BEGIN;
    private byte rxdataCnt = 0;
    private boolean sendAddress = true;

    /**
     * convert method for logging
     */
    @Override
    public String toString() {
        return String
                .format("\nWakePacket %s:\n\taddress=0x%02X,\n\tcommand=0x%02X,\n\tdata:%s,\n\tCodeErr=0x%02X\n",
                        super.toString(), address, command,
                        data != null ? Arrays.toString(data.toArray()) : "",
                        CodeErr);

    }

    /**
     * 
     * @return address of remote devices
     */
    public Byte getAddress() {
        return address;
    }

    /**
     * 
     * @param address
     *            of remote devices
     */
    public void setAddress(Byte address) {
        this.address = address;
    }

    /**
     * 
     * @return command from remote devices
     */
    public Byte getCommand() {
        return command;
    }

    /**
     * 
     * @param command
     *            for remote devices
     */
    public void setCommand(Byte command) {
        this.command = command;
    }

    /**
     * 
     * @return received data (or data for transferring) as List<Byte>
     */
    public List<Byte> getData() {
        return data;
    }

    /**
     * 
     * @return received data (or data for transferring) as String
     */
    public String getDataAsString() {
        StringBuilder sb = new StringBuilder();
        for (byte bt : getData()) {
            sb.append((char) bt);
        }
        return sb.toString();
    }

    /**
     * 
     * @return data size
     */
    public int getDataCount() {
        return data != null ? data.size() : 0;
    }

    /**
     * 
     * @param value
     *            - data for transferring to remote device
     * @throws IllegalArgumentException
     *             if data size bigger then SLIPFRAME
     */
    public void setData(List<Byte> value) throws IllegalArgumentException {
        if (value.size() > Constants.SLIPFRAME)
            throw new IllegalArgumentException(
                    "data size bigger then SLIPFRAME");
        this.data = value;
    }

    /**
     * 
     * @param value
     *            - data for transferring to remote device
     * @throws IllegalArgumentException
     *             if data size bigger then SLIPFRAME
     */
    public void setData(byte[] value) throws IllegalArgumentException {
        List<Byte> bytes = new ArrayList<Byte>();
        for (byte bt : value) {
            bytes.add(bt);
        }
        this.setData(bytes);
    }

    /**
     * 
     * @return received code error
     */
    public Byte getCodeErr() {
        return CodeErr;
    }

    // --------------------- CRC ------------------------
    private int do_crc8(int b, int crc) {
        for (int i = 0; i < 8; b = b >> 1, i++) {
            if (((b ^ crc) & 1) == 1) {
                crc = ((crc ^ 0x18) >> 1) | 0x80;
            } else {
                crc = (crc >> 1) & 0x7F;
            }
        }
        return crc;
    }

    private List<Byte> translateCharSLIP(Byte ch) {
        List<Byte> result = new ArrayList<Byte>();
        if (ch == Constants.FEND) {
            result.add(Constants.FESC);
            result.add(Constants.TFEND);
        } else if (ch == Constants.FESC) {
            result.add(Constants.FESC);
            result.add(Constants.TFESC);
        } else {
            result.add(ch);
        }
        return result;
    }

    private int performCRCcalculation() {
        int crc = Constants.CRC_INIT;
        crc = do_crc8(Constants.FEND, crc);
        if (sendAddress)
            crc = do_crc8(this.getAddress(), crc);
        crc = do_crc8(this.getCommand(), crc);
        crc = do_crc8(this.getDataCount(), crc);
        for (Byte bt : this.getData()) {
            crc = do_crc8(bt, crc);
        }
        return crc;
    }

    /**
     * 
     * @return buffer for transferring to remote device
     */
    public List<Byte> getTXbuf() {
        List<Byte> bufTX = new ArrayList<Byte>();

        bufTX.add(Constants.FEND); // start packet
        bufTX.addAll(translateCharSLIP((byte) (this.getAddress() | 0x80))); // set
                                                                            // address
        bufTX.addAll(translateCharSLIP(this.getCommand())); // set command
        bufTX.addAll(translateCharSLIP((byte) this.getDataCount()));
        for (Byte bt : this.getData()) {
            bufTX.addAll(translateCharSLIP(bt));
        }
        bufTX.addAll(translateCharSLIP((byte) performCRCcalculation()));

        return bufTX;
    }

    /**
     * 
     * @param rcv
     *            - received byte from remote device
     * @return true if wakePacket receiving is completed otherwise - false
     * @throws IllegalArgumentException
     */
    public boolean setRXbyte(byte rcv) throws IllegalArgumentException {

        boolean wakePacketIsReceived = false;

        if (rcv == Constants.FEND)
            state = RxState.BEGIN;

        // byte stuffing
        if (rcv == Constants.FESC && flagFESC == false) {
            flagFESC = true;
            return false;
        }
        if (flagFESC == true) {
            flagFESC = false;
            if (rcv == Constants.TFEND)
                rcv = Constants.FEND;
            else if (rcv == Constants.TFESC)
                rcv = Constants.FESC;
        }
        // end byte stuffing
        switch (state) {
        case BEGIN:
            if (rcv == Constants.FEND) {
                state = RxState.STARTPACKET;
            }
            break;
        case STARTPACKET:
            if ((rcv & 0x80) != 0) {
                sendAddress = true;
                state = RxState.ADDRESS;
                this.setAddress((byte) (rcv & 0x7F));
            } else {
                sendAddress = false;
                state = RxState.COMMAND;
                this.setAddress((byte) 0);
                this.setCommand(rcv);
            }
            break;
        case ADDRESS:
            state = RxState.COMMAND;
            this.setCommand(rcv);
            break;
        case COMMAND: // receive CntData
            data.clear();
            state = (rcv != 0) ? RxState.DATA : RxState.CRC;
            rxdataCnt = rcv;
            if (rxdataCnt > Constants.SLIPFRAME) { // err: packet is very long
                throw new IllegalArgumentException(
                        "Received WakeUp packet is very long");
            }
            break;
        case DATA:
            data.add(rcv);
            if (data.size() == rxdataCnt) {
                state = RxState.CRC;
            }
            break;
        case CRC:
            this.CodeErr = (rcv == (byte) performCRCcalculation()) ? Constants.ERR_NO
                    : Constants.ERR_TX;
            state = RxState.BEGIN;
            wakePacketIsReceived = true;
            break;
        }
        return wakePacketIsReceived;
    }

}
