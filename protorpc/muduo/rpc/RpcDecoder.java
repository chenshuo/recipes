package muduo.rpc;

import java.nio.charset.Charset;
import java.util.zip.Adler32;

import muduo.rpc.proto.RpcProto.RpcMessage;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.handler.codec.oneone.OneToOneDecoder;
import org.jboss.netty.channel.ChannelHandler.Sharable;

@Sharable
public class RpcDecoder extends OneToOneDecoder {

    @Override
    public Object decode(ChannelHandlerContext ctx, Channel channel, Object obj)
            throws Exception {
        if (obj instanceof ChannelBuffer) {
            ChannelBuffer buffer = (ChannelBuffer) obj;
            if (buffer.readableBytes() > 8) {
                String version = buffer.toString(buffer.readerIndex(), 4, Charset.defaultCharset());
                if (version.equals("RPC0")) {
                    Adler32 adler32 = new Adler32();
                    adler32.update(buffer.array(),
                            buffer.arrayOffset() + buffer.readerIndex(),
                            buffer.readableBytes() - 4);
                    buffer.markReaderIndex();
                    buffer.readerIndex(buffer.writerIndex() - 4);
                    int checksum = buffer.readInt();
                    if (checksum == (int)adler32.getValue()) {
                        buffer.resetReaderIndex();
                        RpcMessage message = RpcMessage.newBuilder().mergeFrom(
                                buffer.array(),
                                buffer.arrayOffset() + buffer.readerIndex() + 4,
                                buffer.readableBytes() - 8).build();
                        return message;
                    }
                }
            }
        }
        return obj;
    }
}
