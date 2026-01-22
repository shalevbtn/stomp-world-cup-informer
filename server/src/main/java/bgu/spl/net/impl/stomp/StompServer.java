package bgu.spl.net.impl.stomp;

import bgu.spl.net.impl.data.Database;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("Usage: port server_type(tpc/reactor)");
            return;
        }

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("Server stopping... generating report.");
            Database.getInstance().printReport(); 
        }));

        int port = Integer.parseInt(args[0]);
        String serverType = args[1];

        if (serverType.equals("tpc")) {
            Server.threadPerClient(
                    port,
                    () -> new StompProtocolImpl(), 
                    StompMessageEncoderDecoder::new).serve();
        } else if (serverType.equals("reactor")) {
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    port,
                    () -> new StompProtocolImpl(), 
                    StompMessageEncoderDecoder::new).serve();
        } else {
            System.out.println("Unknown server type. Use 'tpc' or 'reactor'");
        }
    }
}
