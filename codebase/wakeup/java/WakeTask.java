package org.smarthouse.communication.wakeUp;

import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;

/**
 * 
 * class WakeTask is job for multithread manager for Wake protocol
 * 
 * @author Serhiy_Povoroznyuk
 *
 */
public class WakeTask {
    private final long timeout; // TimeUnit.MILLISECONDS
    private WakePacket request;
    private SynchronousQueue<WakePacket> answerSyncQ = new SynchronousQueue<WakePacket>();
    
    /**
     * 
     * @param wakePacket - request
     * @param timeout in  TimeUnit.MILLISECONDS
     */
    public WakeTask(WakePacket wakePacket, long timeout) {
        this.request = wakePacket;
        this.timeout = timeout;
    }
    
    /**
     * 
     * @return request for remote device
     */
    public WakePacket getRequest() {return request;}
    
    /**
     * 
     * @return answer or null if timeout
     * @throws InterruptedException
     */
    public WakePacket getAnswer() throws InterruptedException  {
        return answerSyncQ.poll(timeout, TimeUnit.MILLISECONDS);
    }
    
    /**
     * 
     * @param value - received {@link WakePacket}
     * @return true if the answer was added to SynchronousQueue, else false
     */
    public boolean setAnswer(WakePacket value) {
        return answerSyncQ.offer(value);
    }
}
