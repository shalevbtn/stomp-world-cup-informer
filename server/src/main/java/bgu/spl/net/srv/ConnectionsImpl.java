package bgu.spl.net.srv;

import java.io.IOException;
import java.nio.channels.Channel;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ConnectionsImpl <T> implements Connections <T> {
    private final Map<Integer, ConnectionHandler<T>> chList = new ConcurrentHashMap<>();
    private final Map<String, ConcurrentLinkedQueue<Integer>> subscriptions = new ConcurrentHashMap<>();
    private final Map<Integer,ConcurrentLinkedQueue<String>> clientChannels = new ConcurrentHashMap<>();

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = chList.get(connectionId);

        if (handler == null) return false;

        handler.send(msg);
        return true;
    }

    @Override
    public void send(String channel, T msg) {
        ConcurrentLinkedQueue<Integer> subs = subscriptions.get(channel);

        if(subs != null) {
            for (Integer cID : subs) {
                send(cID, msg);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        for (String channel : clientChannels.get(connectionId)) 
            unsubscribe(connectionId, channel);
        
        clientChannels.remove(connectionId); // Deletes the client channel list.
        ConnectionHandler<T> ch = chList.remove(connectionId);

        try {
            ch.close();
        } catch(IOException e) {
            // TODO: handle exception
        }
    }

    public void connect(int connectionId, ConnectionHandler<T> ch) {
        chList.put(connectionId, ch);
    }

    public void subscribe(int connectionId, String channel) {
        clientChannels.get(connectionId).add(channel); // Adds the channel to the user's channel list
        subscriptions.get(channel).add(connectionId); // Adds the user to the channel's users list
    }

    public void unsubscribe(int connectionId, String channel) {
        clientChannels.get(connectionId).remove(channel); // Deletes the channel from the user's channel list
        subscriptions.get(channel).remove(connectionId); // Deletes the user from the channel's users list
    }



}

