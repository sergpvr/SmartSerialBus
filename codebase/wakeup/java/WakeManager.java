package com.epam.communication.wakeUp;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;

import com.pi4j.io.serial.Serial;
import com.pi4j.io.serial.SerialFactory;

/**
 * WakeManager synchronizes threads for synchronous interaction request-answer
 * by Wake protocol
 * 
 * @author Serhiy_Povoroznyuk
 *
 */
public class WakeManager implements AutoCloseable {

    private final BlockingQueue<WakeTask> requests = new LinkedBlockingQueue<WakeTask>();

    private final String port;
    private final int baudrate;
    private final long timeout;
    //
    private final Serial serial = SerialFactory.createInstance();
    private final WakeUpSerial wakeUpSerial = new WakeUpSerial();
    private final WakeRunnable wakeRunnable;
    private Thread walk;

    /**
     * 
     * @param port
     *            : String like Serial.DEFAULT_COM_PORT
     * @param baudrate
     *            - 9600, 19200, .. , 115200
     * @param timeout
     *            in TimeUnit.MILLISECONDS
     */
    public WakeManager(String port, int baudrate, long timeout) {
        this.port = port;
        this.baudrate = baudrate;
        this.timeout = timeout;
        wakeRunnable = new WakeRunnable(wakeUpSerial, requests, timeout);
        walk = new Thread(wakeRunnable);
        wakeUpSerial.addWakeUpListener(wakeRunnable);
        wakeUpSerial.setSerial(serial);
    }

    /**
     * opens {@link Serial} connection and starts Thread for consecutive
     * requests to line from asynchronous clients
     * 
     * @throws Exception
     */
    public void open() throws Exception {
        serial.open(port, baudrate);
        walk.start();
    }

    /**
     * close {@link Serial} , interrupt inner thread, remove
     * {@link WakeUpListener}
     */
    public void close() throws Exception {
        wakeUpSerial.removeWakeUpListener(wakeRunnable);
        wakeUpSerial.unsetSerial();
        walk.interrupt();
        serial.close();
    }

    /**
     * synchronous requests by Wake protocol
     * 
     * @param wakePacket
     *            - {@link WakePacket} with request
     * @return {@link WakePacket} with answer or null after timeout
     * @throws InterruptedException
     */
    public WakePacket askLine(WakePacket wakePacket)
            throws InterruptedException {
        WakeTask wakeTask = new WakeTask(wakePacket, timeout);
        requests.add(wakeTask);
        return wakeTask.getAnswer();
    }

    /**
     * 
     * @param request
     *            - {@link WakePacket}
     * @param answer
     *            - {@link WakePacket}
     * @return true if answer correspond of request otherwise false
     */
    public static boolean checkAnswer(WakePacket request, WakePacket answer) {
        boolean result = true;

        if (request == null || answer == null) {
            result = false;
        } else if ((int) request.getAddress() != (int) answer.getAddress()) {
            result = false;
        } else if (((int) request.getCommand() | 0x40) != (int) answer
                .getCommand()) {
            result = false;
        } else if (answer.getCodeErr() != Constants.ERR_NO) {
            result = false;
        }
        return result;
    }

}

/**
 * WakeRunnable - inner thread for consecutive requests to line from
 * asynchronous clients
 * 
 * @author Serhiy_Povoroznyuk
 *
 */
class WakeRunnable implements Runnable, WakeUpListener {

    private final BlockingQueue<WakeTask> requests;
    private final WakeUpSerial wakeUpSerial;
    private final long timeout; // TimeUnit.MILLISECONDS
    private SynchronousQueue<WakePacket> answerSyncQ = new SynchronousQueue<WakePacket>();

    /**
     * 
     * @param wakeUpSerial - {@link wakeUpSerial} for interaction with remote devices 
     * @param requests - BlockingQueue for {@link WakeTask}
     * @param timeout in TimeUnit.MILLISECONDS
     */
    public WakeRunnable(WakeUpSerial wakeUpSerial,
            BlockingQueue<WakeTask> requests, long timeout) {
        this.wakeUpSerial = wakeUpSerial;
        this.requests = requests;
        this.timeout = timeout;
    }

    @Override
    public void run() {
        do {
            if (!Thread.interrupted()) {
                try {
                    WakeTask wakeTask = requests.take();
                    WakePacket request = wakeTask.getRequest();
                    wakeUpSerial.wakeTX(request); // send to serial
                    WakePacket answerWakePacket = wakeRead(); // read with
                                                              // timeout
                    if (answerWakePacket != null) {
                        wakeTask.setAnswer(answerWakePacket);
                    }
                } catch (InterruptedException e) {
                    return;
                }
            } else {
                return;
            }
        } while (true);

    }

    @Override
    public void wakeUpReceived(WakePacket wp) {
        try {
            answerSyncQ.offer(wp, 5, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            return;
        }

    }

    /**
     * 
     * @return received {@link WakePacket} or null if timeout
     * @throws InterruptedException
     */
    private WakePacket wakeRead() throws InterruptedException {
        return answerSyncQ.poll(timeout, TimeUnit.MILLISECONDS);
    }

}
