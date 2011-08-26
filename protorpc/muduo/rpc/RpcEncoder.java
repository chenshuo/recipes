package muduo.rpc;

import java.util.zip.Adler32;

import muduo.rpc.proto.RpcProto.RpcMessage;

import org.jboss.netty.buffer.BigEndianHeapChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelHandler.Sharable;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.handler.codec.oneone.OneToOneEncoder;

import com.google.protobuf.CodedOutputStream;

@Sharable
public class RpcEncoder extends OneToOneEncoder {

    public RpcEncoder() {
        super();
    }

    @Override
    public Object encode(ChannelHandlerContext ctx, Channel channel, Object obj)
            throws Exception {
        if (!(obj instanceof RpcMessage)) {
            return obj;
        }
        RpcMessage message = (RpcMessage) obj;
        int size = message.getSerializedSize();
        ChannelBuffer buffer = new BigEndianHeapChannelBuffer(4 + size + 4);
        buffer.writeBytes("RPC0".getBytes());
        int writerIndex = buffer.writerIndex();
        CodedOutputStream output = CodedOutputStream.newInstance(
                buffer.array(), buffer.writerIndex(), buffer.writableBytes() - 4);
        message.writeTo(output);
        output.checkNoSpaceLeft();

        buffer.writerIndex(writerIndex + size);
        Adler32 checksum = new Adler32();
        checksum.update(buffer.array(), buffer.arrayOffset(), buffer.readableBytes());
        buffer.writeInt((int) checksum.getValue());
        return buffer;
    }
}