package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompProtocolImpl<T> implements StompMessagingProtocol<T> {

    private int conId;
    Connections<T> con;
    private boolean shouldTerminate = false;
    private boolean isLoggedIn = false;

    @Override
    public void start(int connectionId, Connections<T> connections) {
        conId = connectionId;
        con = connections;
    }

    @Override
    public void process(T message) {
        
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

}
