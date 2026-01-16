package bgu.spl.net.srv;

import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.impl.stomp.StompHelper;
import bgu.spl.net.impl.stomp.StompMessage;

public class ConnectionsImpl <T> implements Connections <T> {
    private AtomicInteger messageIdCounter = new AtomicInteger(0);
    private final Map<Integer, ConnectionHandler<T>> clients = new ConcurrentHashMap<>(); //ConnectionID -> ConnectionHandler (Which is socket)
    private final Map<String, ConcurrentHashMap<Integer, String>> subscriptions = new ConcurrentHashMap<>(); //Channel -> every client subscribed and the subscriptionID
    private final Map<Integer,ConcurrentLinkedQueue<String>> clientChannels = new ConcurrentHashMap<>(); //Client -> All his channels

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = clients.get(connectionId);

        if (handler == null) return false;
        handler.send(msg);
        return true;
    }

    @Override
    public void send(String channel, T msg) {
        ConcurrentHashMap<Integer, String> subs = subscriptions.get(channel);
        
        if (subs != null) {
            String msgBody = (String) msg;
            
            for (Map.Entry<Integer, String> entry : subs.entrySet()) {
                Integer connId = entry.getKey();
                String subId = entry.getValue();
                
                String messageId = Integer.toString(messageIdCounter.incrementAndGet());
                
                StompMessage frame = StompHelper.getMessageFrame(subId, messageId, channel, msgBody);
                
                send(connId, (T) frame);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        ConcurrentLinkedQueue<String> channels = clientChannels.remove(connectionId);
        
        if (channels != null) {
            for (String channel : channels) {
                ConcurrentHashMap<Integer, String> subs = subscriptions.get(channel);
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

    public void subscribe(int connectionId, String subscriptionId ,String channel) {
        subscriptions.computeIfAbsent(channel, k -> new ConcurrentHashMap<Integer, String>()).put(connectionId,subscriptionId); // Adds the user to the channel's users list
        clientChannels.computeIfAbsent(connectionId, k -> new ConcurrentLinkedQueue<>()).add(channel); // Adds the channel to the user's channel list
    }

    public void unsubscribe(int connectionId, String channel) {
        // Deletes the user from the channel's users list
        ConcurrentHashMap<Integer, String> subs = subscriptions.get(channel);
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
    //public Boolean isClientSubbed() {return false;}
}