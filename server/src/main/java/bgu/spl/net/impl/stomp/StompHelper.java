package bgu.spl.net.impl.stomp;

public class StompHelper {

    public static String getConnectedFrame(String version) {
        return  "CONNECTED\n" +
                "version: " + version + "\n";
    }

    public static String getErrorFrame(String messageHeader, String description, String receiptId) {
        StringBuilder sb = new StringBuilder();
        sb.append("ERROR\n");
        
        if (receiptId != null) {
            sb.append("receipt-id:").append(receiptId).append("\n");
        }
        
        sb.append("message:").append(messageHeader).append("\n")
          .append("\n")
          .append(description).append("\n");
          
        return sb.toString();
    }

    public static String getReceiptFrame(String receiptId) {
        return "RECEIPT\n" +
               "receipt-id:" + receiptId + "\n" +
               "\n";
    }

    public static String getMessageFrame(String subscriptionId, String messageId, String destination, String body) {
        return "MESSAGE\n" +
               "subscription:" + subscriptionId + "\n" +
               "message-id:" + messageId + "\n" +
               "destination:" + destination + "\n" +
               "\n" +
               body + "\n";
    }
}
