package bgu.spl.net.impl.stomp;

import java.util.HashMap;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.data.*;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompProtocolImpl implements StompMessagingProtocol<String> {
    private int connectionId;
    private ConnectionsImpl<String> con;
    private boolean shouldTerminate = false;
    private boolean isLoggedIn = false;
    private HashMap<String,String> requestIdMap = new HashMap<>(); //subscriptionID -> Channel

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.con = (ConnectionsImpl<String>)connections;
    }

    @Override
    public void process(String message) {
        StompMessage msg;

        try {
            msg = new StompMessage(message);
        } catch (Exception e) {
            String error = StompHelper.getErrorFrame("Malformed Frame", "Could not parse message", null);
            finishConnection(error);
            return; 
        }

        if (!isLoggedIn && !msg.command.equals("CONNECT")) {
            String error = StompHelper.getErrorFrame("Not Logged In", "You must log in first", null);
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
                String err = StompHelper.getErrorFrame("Unknown Command", "Command not recognized", null);
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
            // TO CHECK this.currentUser = login;
            String response = StompHelper.getConnectedFrame(version);
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
        con.subscribe(connectionId, id, channel);
        
        if (receipt != null) {
            String response = StompHelper.getReceiptFrame(receipt);
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
        if (channel != null) {
            con.unsubscribe(connectionId, channel);
        }


        
        if (receipt != null) {
            String response = StompHelper.getReceiptFrame(receipt);
            con.send(connectionId, response);
        }
    }

    private void handleDisconnect(StompMessage msg){
        String receipt = msg.getHeader("receipt");
        String response = null;

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
             String error = StompHelper.getErrorFrame("Usage Error", "Not subscribed to " + channel, receipt);
             finishConnection(error);
             return;
        }

        con.send(channel,msg.body);

        if (receipt != null) {
            con.send(connectionId, StompHelper.getReceiptFrame(receipt));
        }

    }

    private void handleFailedLogin(LoginStatus s) {
        String errorHeader = "Login failed";
        String errorDescription = "";

        switch (s) {
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
        }
        String response = StompHelper.getErrorFrame(errorHeader, errorDescription, null);
        finishConnection(response);
    }

    private void finishConnection(String response) {
        if(response != null)
            con.send(connectionId, response);
        shouldTerminate = true;
        con.disconnect(connectionId);
    }

    private void handleMalFrame(String receipt, String missingHeaders) {
         String errorResponse = StompHelper.getErrorFrame(
                "Malformed Frame", 
                "Missing required headers: " + missingHeaders, 
                receipt
            );
            finishConnection(errorResponse);
    }
}
