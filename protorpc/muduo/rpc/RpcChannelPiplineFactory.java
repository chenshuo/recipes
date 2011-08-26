package muduo.rpc;

import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.handler.codec.frame.LengthFieldBasedFrameDecoder;
import org.jboss.netty.handler.codec.frame.LengthFieldPrepender;

public class RpcChannelPiplineFactory implements ChannelPipelineFactory {

    private RpcDecoder rpcDecoder = new RpcDecoder();
    private RpcEncoder rpcEncoder = new RpcEncoder();
    private LengthFieldPrepender frameEncoder = new LengthFieldPrepender(4);
    private RpcPeer rpcPeer;

    public RpcChannelPiplineFactory(RpcPeer peer) {
        this.rpcPeer = peer;
    }

    @Override
    public ChannelPipeline getPipeline() throws Exception {
        ChannelPipeline p = Channels.pipeline();
        p.addLast("frameDecoder", new LengthFieldBasedFrameDecoder(16 * 1024 * 1024, 0, 4, 0, 4));
        p.addLast("rpcDecoder", rpcDecoder);

        p.addLast("frameEncoder", frameEncoder);
        p.addLast("rpcEncoder", rpcEncoder);

        p.addLast("handler", new RpcMessageHandler(rpcPeer));
        return p;
    }

}
