package bgu.spl.net.impl.stomp;

import java.util.HashMap;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.data.*;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompProtocolImpl implements StompMessagingProtocol<StompMessage> {
    private int connectionId;
    private ConnectionsImpl<StompMessage> con;
    private boolean shouldTerminate = false;
    private boolean isLoggedIn = false;
    
    private HashMap<String,String> requestIdMap = new HashMap<>(); //subscriptionID -> Channel
    private HashMap<String,String> ChannelSubIdMap = new HashMap<>(); //Channel -> subscriptionID

    @Override
    public void start(int connectionId, Connections<StompMessage> connections) {
        this.connectionId = connectionId;
        this.con = (ConnectionsImpl<StompMessage>)connections;
    }

    @Override
    public void process(StompMessage msg) {
        if(msg == null) {
            StompMessage error = StompHelper.getErrorFrame("Malformed Frame", "Could not parse message", null);
            finishConnection(error);
            return; 
        }

        if (!isLoggedIn && !msg.command.equals("CONNECT")) {
            StompMessage error = StompHelper.getErrorFrame("Not Logged In", "You must log in first", null);
            finishConnection(error);
            return;
        }

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
                StompMessage err = StompHelper.getErrorFrame("Unknown Command", "Command not recognized", null);
                con.send(connectionId, err);
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private void handleConnect(StompMessage msg) {
        String login = msg.getHeader("login");
        String passcode = msg.getHeader("passcode");
        String version = msg.getHeader("accept-version");
        String host = msg.getHeader("host"); 

        if (login == null || passcode == null || version == null || host == null) {
            handleMalFrame(null, "login, passcode, accept-version, or host");
            return;
        }

        LoginStatus status = Database.getInstance().login(connectionId, login, passcode);

        if (status == LoginStatus.LOGGED_IN_SUCCESSFULLY || status == LoginStatus.ADDED_NEW_USER) {
            this.isLoggedIn = true;
            //TODO: TO CHECK this.currentUser = login;
            StompMessage response = StompHelper.getConnectedFrame(version);
            con.send(connectionId, response);
        }
        else {
            handleFailedLogin(status);
        }
    }

    private void handleSubscribe(StompMessage msg){
        String channel = msg.getHeader("destination");
        String id = msg.getHeader("id");
        String receipt = msg.getHeader("receipt");

        if(channel == null || id == null) {
            handleMalFrame(receipt, "destination or id");
            return;
        }
        requestIdMap.put(id, channel);
        ChannelSubIdMap.put(channel, id);
        con.subscribe(connectionId, id, channel);
        
        if (receipt != null) {
            StompMessage response = StompHelper.getReceiptFrame(receipt);
            con.send(connectionId, response);
        }
    }

    private void handleUnsubscribe(StompMessage msg){
        String id = msg.getHeader("id");
        String receipt = msg.getHeader("receipt");

        if(id == null) {
            handleMalFrame(receipt, "id");
            return;
        }

        String channel = requestIdMap.remove(id);
        ChannelSubIdMap.remove(channel);
        if (channel != null) {
            con.unsubscribe(connectionId, channel);
        }
        
        if (receipt != null) {
            StompMessage response = StompHelper.getReceiptFrame(receipt);
            con.send(connectionId, response);
        }
    }

    private void handleDisconnect(StompMessage msg){
        String receipt = msg.getHeader("receipt");
        StompMessage response = null;

        if (receipt != null) 
            response = StompHelper.getReceiptFrame(receipt);
        
        finishConnection(response);
    }

    private void handleSend(StompMessage msg){
        String channel = msg.getHeader("destination");
        String receipt = msg.getHeader("receipt");

        if(channel == null || msg.body == null) {  //TODO: verify that msg.body != "" is a requirement
            handleMalFrame(receipt, "destination");
            return;
        }

        if (!requestIdMap.containsValue(channel)) {
             StompMessage error = StompHelper.getErrorFrame("Usage Error", "Not subscribed to " + channel, receipt);
             finishConnection(error);
             return;
        }

        StompMessage response = StompHelper.getMessageFrame(ChannelSubIdMap.get(channel), receipt, channel, msg.body);

        con.send(channel,response);

        if (receipt != null) {
            con.send(connectionId, StompHelper.getReceiptFrame(receipt));
        }

    }

    private void handleFailedLogin(LoginStatus status) {
        String errorHeader = "Login failed";
        String errorDescription = "";

        switch (status) {
            case WRONG_PASSWORD:
                errorHeader = "Wrong password";
                errorDescription = "Password does not match the username.";
                break;
            case ALREADY_LOGGED_IN:
                errorHeader = "User already logged in";
                errorDescription = "User is already active in another session.";
                break;
            case CLIENT_ALREADY_CONNECTED:
                errorHeader = "User already logged in";
                errorDescription = "Client session already active.";
                break;
            default:
                break;
        }
        StompMessage response = StompHelper.getErrorFrame(errorHeader, errorDescription, null);
        finishConnection(response);
    }

    private void finishConnection(StompMessage response) {
        if(response != null)
            con.send(connectionId, response);
        shouldTerminate = true;
        con.disconnect(connectionId);
    }

    private void handleMalFrame(String receipt, String missingHeaders) {
        StompMessage errorResponse = StompHelper.getErrorFrame(
                "Malformed Frame", 
                "Missing required headers: " + missingHeaders, 
                receipt
        );
        finishConnection(errorResponse);
    }
}
