package bgu.spl.net.impl.stomp;

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
        StompMessage msg = new StompMessage(message);

        switch (msg.command) {
            case "CONNECT":
                handleConnect(msg);
                break;

            case "SEND":
                handleSend(msg);
                break;

            case "SUBSCRIBE":
                handleSubscribe(msg);
                break;

            case "UNSUBSCRIBE":
                handleUnsubscribe(msg);
                break;

            case "DISCONNECT":
                handleDisconnect(msg);
                break;

            default:
                throw new IllegalArgumentException("Illigal Stomp command.");
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private void handleConnect(StompMessage msg){
        StringBuilder sb = new StringBuilder();

        try {
            String host = msg.getParameter("host");
            String login = msg.getParameter("login");
            String pass = msg.getParameter("passcode");

            //Now validate the user & pass with the DB and connected.

            sb.append("Connected").append("/n");
            sb.append("version:").append(msg.getParameter("accept-version")).append("/n");
        } catch(Exception ex) {
            // Wrong parameters probablly
        }
        finally {
            sb.append("\n");
        }    
    }

    private void handleSend(StompMessage msg){
        
    }

    private void handleSubscribe(StompMessage msg){
        
    }

    private void handleUnsubscribe(StompMessage msg){
        
    }

    private void handleDisconnect(StompMessage msg){
        shouldTerminate = true;
    }

}
