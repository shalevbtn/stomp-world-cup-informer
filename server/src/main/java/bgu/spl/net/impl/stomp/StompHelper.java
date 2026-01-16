package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

public class StompHelper {

    public static StompMessage getConnectedFrame(String version) {
        Map<String, String> headers = new HashMap<>();
        headers.put("version", version);

        return new StompMessage("CONNECTED", headers, "");
    }

    public static StompMessage getReceiptFrame(String receiptId) {
        Map<String, String> headers = new HashMap<>();
        headers.put("receipt-id", receiptId);

        return new StompMessage("RECEIPT", headers, "");
    }

    public static StompMessage getMessageFrame(String subscriptionId, String messageId, String destination, String body) {
        Map<String, String> headers = new HashMap<>();
        headers.put("subscription", subscriptionId);
        headers.put("message-id", messageId);
        headers.put("destination", destination);

        return new StompMessage("MESSAGE", headers, body);
    }
    
    public static StompMessage getErrorFrame(String messageHeader, String description, String receiptId) {
        Map<String, String> headers = new HashMap<>();

        if (receiptId != null) {
            headers.put("receipt-id", receiptId);
        }

        headers.put("message", messageHeader);

        return new StompMessage("ERROR", headers, description);
    }
}
