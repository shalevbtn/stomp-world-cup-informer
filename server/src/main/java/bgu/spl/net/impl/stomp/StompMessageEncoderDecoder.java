package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;
import java.util.Arrays;

public class StompMessageEncoderDecoder implements MessageEncoderDecoder<StompMessage> {

    private byte[] bytes = new byte[1 << 10]; // 1k
    private int len = 0;

    @Override
    public StompMessage decodeNextByte(byte nextByte) { 
        if (nextByte == '\u0000')
            try {
                return new StompMessage(popString());
            } catch (Exception e) {
                return null;
            }
            
        pushByte(nextByte);
        return null; 
    }

    @Override
    public byte[] encode(StompMessage message) {

        return (message.toString() + "\u0000").getBytes();
    }

    private String popString(){
        String result = new String(bytes, 0, len); 
        len = 0;
        return result;
    }

    private void pushByte(byte nextByte){
        if (len >= bytes.length)
            bytes = Arrays.copyOf(bytes, len << 1);
        bytes[len++] = nextByte;
    }
    
}
