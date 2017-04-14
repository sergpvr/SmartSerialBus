package com.epam.communication.wakeUp;

/**
 * states for receiving of Wake packet
 * 
 * @author Serhiy_Povoroznyuk
 *
 */
public enum RxState {
    BEGIN,
    STARTPACKET,
    ADDRESS,
    COMMAND,
    DATA,
    CRC
}
