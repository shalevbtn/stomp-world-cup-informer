package bgu.spl.net.srv;

import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ConnectionsImpl <T> implements Connections <T> {
    private final Map<Integer, ConnectionHandler<T>> clients = new ConcurrentHashMap<>();
    private final Map<String, ConcurrentLinkedQueue<Integer>> subscriptions = new ConcurrentHashMap<>();
    private final Map<Integer,ConcurrentLinkedQueue<String>> clientChannels = new ConcurrentHashMap<>();

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = clients.get(connectionId);

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
        ConcurrentLinkedQueue<String> channels = clientChannels.remove(connectionId);
        
        if (channels != null) {
            for (String channel : channels) {
                ConcurrentLinkedQueue<Integer> subs = subscriptions.get(channel);
                if (subs != null) {
                    subs.remove(connectionId);
                }
            }
        }

        ConnectionHandler<T> handler = clients.remove(connectionId);
        if (handler != null) {
            try {
                handler.close();
            } catch (IOException e) {
                // Log error or ignore
            }
        }
    }

    public void connect(int connectionId, ConnectionHandler<T> ch) {
        clients.put(connectionId, ch);
    }

    public void subscribe(int connectionId, String channel) {
        subscriptions.computeIfAbsent(channel, k -> new ConcurrentLinkedQueue<>()).add(connectionId); // Adds the user to the channel's users list
        clientChannels.computeIfAbsent(connectionId, k -> new ConcurrentLinkedQueue<>()).add(channel); // Adds the channel to the user's channel list
    }

    public void unsubscribe(int connectionId, String channel) {
        // Deletes the user from the channel's users list
        ConcurrentLinkedQueue<Integer> subs = subscriptions.get(channel);
        if (subs != null) {
            subs.remove(connectionId);
        }

        // Deletes the channel from the user's channel list
        ConcurrentLinkedQueue<String> userChans = clientChannels.get(connectionId);
        if (userChans != null) {
            userChans.remove(channel);
        }
    }

    // TODO: NEED TO VERIFY: if a client is not subscribed to a topic it is not allowed to send messages to it, and the server should send back an ERROR frame
    public Boolean isClientSubed() {return false;}
}