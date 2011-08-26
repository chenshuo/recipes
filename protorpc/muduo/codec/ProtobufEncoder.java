package muduo.codec;

import java.util.zip.Adler32;

import org.jboss.netty.buffer.BigEndianHeapChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelHandler.Sharable;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.handler.codec.oneone.OneToOneEncoder;

import com.google.protobuf.Message;

@Sharable
public class ProtobufEncoder extends OneToOneEncoder {

    public ProtobufEncoder() {
        super();
    }

    @Override
    public Object encode(ChannelHandlerContext ctx, Channel channel, Object obj)
            throws Exception {
        if (!(obj instanceof Message)) {
            return obj;
        }
        Message message = (Message) obj;
        String name = message.getDescriptorForType().getFullName();
        int size = message.getSerializedSize();
        ChannelBuffer buffer = new BigEndianHeapChannelBuffer(4 + name.length() + 1 + size + 4);
        buffer.writeInt(name.length() + 1);
        buffer.writeBytes(name.getBytes());
        buffer.writeZero(1);
        buffer.writeBytes(message.toByteArray());

        Adler32 checksum = new Adler32();
        checksum.update(buffer.array(), buffer.arrayOffset(), buffer.readableBytes());
        buffer.writeInt((int) checksum.getValue());

        return buffer;
    }
}
