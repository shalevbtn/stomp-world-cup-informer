package bgu.spl.net.impl.stomp;

import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompProtocolImpl implements StompMessagingProtocol<String> {


    private int conId;
    private ConnectionsImpl con;
    private boolean shouldTerminate = false;
    private boolean isLoggedIn = false;

    @Override
    public void start(int connectionId, Connections<String> connections) {
        conId = connectionId;
        con = (ConnectionsImpl)connections;
    }

    @Override
    public void process(String message) {
        String[] lines = message.split("\n");
        String command = lines[0];
        List<String> params = new LinkedList<>();
        String context = "";
        
        boolean isParams = true;
        for(int i = 0; i < lines.length; i++) {
            if(lines[i] == "\n") isParams = false;

            else if(isParams) {
                params.add(lines[i]);
            }
            else {
                context += lines[i];
            }
        }

        switch (command) {
            case "CONNECT":
                
                break;

            case "SEND":
                
                break;

            case "SUBSCRIBE":
                
                break;
            case "UNSUBSCRIBE":
                
                break;

            case "DISCONNECT":
                con.
                shouldTerminate = true;
                break;

            default:
                throw new IllegalArgumentException("Illigal Stomp command.");
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

}
