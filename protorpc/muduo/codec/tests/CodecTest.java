package muduo.codec.tests;

import static org.junit.Assert.assertEquals;
import muduo.codec.ProtobufDecoder;
import muduo.codec.ProtobufEncoder;
import muduo.codec.tests.QueryProtos.Empty;
import muduo.codec.tests.QueryProtos.Query;

import org.jboss.netty.buffer.BigEndianHeapChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffer;
import org.junit.Test;

import com.google.protobuf.Message;

public class CodecTest {

    @Test
    public void testEncoderEmpty() throws Exception {
        ProtobufEncoder encoder = new ProtobufEncoder();
        Empty empty = Empty.getDefaultInstance();
        encoder.encode(null, null, empty);
    }

    @Test
    public void testDecodeEmpty() throws Exception {
        ProtobufEncoder encoder = new ProtobufEncoder();
        Empty empty = Empty.getDefaultInstance();
        ChannelBuffer buf = (ChannelBuffer) encoder.encode(null, null, empty);

        ProtobufDecoder decoder = new ProtobufDecoder();
        decoder.addMessageType(Empty.getDefaultInstance());
        Message message = (Message) decoder.decode(null, null, buf);
        assertEquals(empty, message);
    }

    @Test
    public void testQuery() throws Exception {
        ProtobufEncoder encoder = new ProtobufEncoder();
        Query query = Query.newBuilder()
                .setId(1)
                .setQuestioner("Chen Shuo")
                .addQuestion("Running?")
                .build();
        ChannelBuffer buf = (ChannelBuffer) encoder.encode(null, null, query);

        ProtobufDecoder decoder = new ProtobufDecoder();
        decoder.addMessageType(Query.getDefaultInstance());
        Message message = (Message) decoder.decode(null, null, buf);
        assertEquals(query, message);
    }

    @Test
    public void testQuery2() throws Exception {
        ProtobufEncoder encoder = new ProtobufEncoder();
        Query query = Query.newBuilder()
                .setId(1)
                .setQuestioner("Chen Shuo")
                .addQuestion("Running?")
                .build();
        ChannelBuffer buf = (ChannelBuffer) encoder.encode(null, null, query);
        ChannelBuffer buf2 = new BigEndianHeapChannelBuffer(buf.readableBytes() + 8);
        buf2.writeInt(123);
        buf2.writeBytes(buf);

        buf2.readInt();
        ProtobufDecoder decoder = new ProtobufDecoder();
        decoder.addMessageType(Query.getDefaultInstance());
        Message message = (Message) decoder.decode(null, null, buf2);
        assertEquals(query, message);
    }
}
