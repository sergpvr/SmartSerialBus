package com.epam.communication.wakeUp;

/**
 * Listener for Wake protocol
 * 
 * @author Serhiy_Povoroznyuk
 *
 */
public interface WakeUpListener {
    /**
     * 
     * @param wp - received WakePacket
     */
    void wakeUpReceived(WakePacket wp);
}
