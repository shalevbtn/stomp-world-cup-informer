package bgu.spl.net.impl.stomp;

import java.util.HashMap;

public class StompMessage {
    String command;
    HashMap<String,String> parameters;
    String context;


    public StompMessage(String msg) {
        String[] lines = msg.split("\n");
        command = lines[0];
        context = "";
        parseMessage(lines);
    }

    public String getParameter(String key) {
        try {
            return parameters.get(key);
        } catch (Exception ex) {
            System.out.print("Invalid Parameters of this command.");
            return null;
        }
    }


    private void parseMessage(String[] lines) {
        boolean isParams = true;
        //TO CHECK
        for (String line : lines) {
            if(line == "\n") {
                isParams = false;
            }
            else if(isParams){
                String[] lineSplit = line.split(":");
                parameters.put(lineSplit[0], lineSplit[1]);
            }
            else {
                context += line + "\n";
            }
        }
    }
}
