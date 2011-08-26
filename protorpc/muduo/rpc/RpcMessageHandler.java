package muduo.rpc;

import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ChannelStateEvent;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelUpstreamHandler;

public class RpcMessageHandler extends SimpleChannelUpstreamHandler {

    private volatile RpcChannel channel;
    private RpcPeer rpcPeer;

    public RpcMessageHandler(RpcPeer peer) {
        this.rpcPeer = peer;
    }

    public RpcChannel getChannel() {
        return channel;
    }

    public void setChannel(RpcChannel ch) {
        channel = ch;
    }

    @Override
    public void channelConnected(ChannelHandlerContext ctx, ChannelStateEvent e) throws Exception {
        System.err.println("channelConnected");
        rpcPeer.channelConnected(e.getChannel());
    }

    @Override
    public void messageReceived(ChannelHandlerContext ctx, MessageEvent e) throws Exception {
        assert e.getChannel() == channel.getChannel();
        channel.messageReceived(ctx, e);
    }
}
