package bgu.spl.net.impl.stomp;

import java.util.ArrayList;
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

    List<String>

    @Override
    public void start(int connectionId, Connections<String> connections) {
        conId = connectionId;
        con = (ConnectionsImpl)connections;
    }

    @Override
    public void process(String message) {
        String[] lines = message.split("\n");
        String command = lines[0];
        List<String> params = new ArrayList<>();
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
                handleConnect(params);
                break;

            case "SEND":
                handleSend(params, context);
                break;

            case "SUBSCRIBE":
                handleSubscribe(params, context);
                break;

            case "UNSUBSCRIBE":
                handleUnSubscribe(params, context);
                break;

            case "DISCONNECT":
                handleDisconnect(params, context);
                break;

            default:
                throw new IllegalArgumentException("Illigal Stomp command.");
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private void handleConnect(List<String> params){
        StringBuilder sb = new StringBuilder();

        String host = params.get(1);
        String login = params.get(2);
        String pass = params.get(3);

        sb.append("Connected").append("/n");
        sb.append(params.get(0).substring(7)).append("/n");
    }

    private void handleSend(List<String> params, String context){
        
    }

    private void handleSubscribe(List<String> params, String context){
        
    }

    private void handleUnSubscribe(List<String> params, String context){
        
    }

    private void handleDisconnect(List<String> params, String context){
        shouldTerminate = true;
    }

}
