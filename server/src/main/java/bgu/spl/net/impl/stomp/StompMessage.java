package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class StompMessage {
    public String command;
    public Map<String, String> headers = new HashMap<>();
    public String body = "";

    public StompMessage(String rawMessage) {
        parse(rawMessage);
    }

    private void parse(String msg) {
        String[] parts = msg.split("\n\n", 2);

        String[] lines = parts[0].split("\n");
        this.command = lines[0].trim();
        
        for (int i = 1; i < lines.length; i++) {
            String[] pair = lines[i].split(":", 2);
            if (pair.length == 2) {
                headers.put(pair[0], pair[1]);
            }
        }

        if (parts.length > 1) {
            this.body = parts[1];
        }
    }

    public String getHeader(String key) {
        return headers.get(key);
    }
}