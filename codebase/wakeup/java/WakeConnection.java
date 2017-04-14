package com.greenhouse.communication.wakeUp;

/**
 * WakeConnection - generic interface for synchronous requests by Wake protocol
 * 
 * @author Serhiy_Povoroznyuk
 *
 */
public interface WakeConnection {
   
    /**
     * 
     * @param request {@link WakePacket} with request data
     * @return {@link WakePacket} with answer or null if timeout occurred
     */
    WakePacket poll(WakePacket request);
    
}
