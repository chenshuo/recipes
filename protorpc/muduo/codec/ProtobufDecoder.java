package muduo.codec;

import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.Map;
import java.util.zip.Adler32;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelHandler.Sharable;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.handler.codec.oneone.OneToOneDecoder;

import com.google.protobuf.Message;

@Sharable
public class ProtobufDecoder extends OneToOneDecoder {

    private Map<String, Message> knownTypes = new HashMap<String, Message>();

    @Override
    public Object decode(ChannelHandlerContext ctx, Channel channel, Object obj)
            throws Exception {
        if (!(obj instanceof ChannelBuffer)) {
            return obj;
        }
        ChannelBuffer buffer = (ChannelBuffer) obj;
        if (buffer.readableBytes() >= 10 && checksum(buffer)) {
            int nameLen = buffer.readInt();
            String typeName = buffer.toString(buffer.readerIndex(), nameLen - 1,
                    Charset.defaultCharset());
            buffer.readerIndex(buffer.readerIndex() + nameLen);
            Message prototype = knownTypes.get(typeName);
            if (prototype != null) {
                return prototype.newBuilderForType().mergeFrom(buffer.array(),
                        buffer.arrayOffset() + buffer.readerIndex(),
                        buffer.readableBytes() - 4).build();
            }
        }
        return obj;
    }

    private boolean checksum(ChannelBuffer buffer) {
        Adler32 adler32 = new Adler32();
        adler32.update(buffer.array(),
                buffer.arrayOffset() + buffer.readerIndex(),
                buffer.readableBytes() - 4);
        buffer.markReaderIndex();
        buffer.readerIndex(buffer.writerIndex() - 4);
        int checksum = buffer.readInt();
        buffer.resetReaderIndex();
        return checksum == (int) adler32.getValue();
    }

    public void addMessageType(Message message) {
        knownTypes.put(message.getDescriptorForType().getFullName(), message);
    }
}
