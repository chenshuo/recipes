package muduo.rpc;

import java.net.SocketAddress;
import java.util.concurrent.Executors;

import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.ChannelFutureListener;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;

public class RpcClient extends RpcPeer {

    ClientBootstrap bootstrap;
    private RpcChannel rpcChannel;

    public RpcClient() {
        ChannelFactory channelFactory = new NioClientSocketChannelFactory(
                Executors.newCachedThreadPool(),
                Executors.newCachedThreadPool());

        bootstrap = new ClientBootstrap(channelFactory);
        bootstrap.setPipelineFactory(new RpcChannelPiplineFactory(this));
    }

    public RpcChannel blockingConnect(SocketAddress addr) {
        Channel channel = bootstrap.connect(addr).awaitUninterruptibly().getChannel();
        rpcChannel = new RpcChannel(channel);
        RpcMessageHandler handler = (RpcMessageHandler) channel.getPipeline().get("handler");
        handler.setChannel(rpcChannel);
        return rpcChannel;
    }

    public void startConnect(SocketAddress addr, NewChannelCallback newChannelCallback) {
        ChannelFuture future = bootstrap.connect(addr);
        future.addListener(new ChannelFutureListener() {
            @Override
            public void operationComplete(ChannelFuture future) throws Exception {
                System.err.println("operationComplete");
                // channelConnected(future.getChannel());
            }
        });
        this.newChannelCallback = newChannelCallback;
    }

    @Override
    public void channelConnected(Channel channel) {
        rpcChannel = new RpcChannel(channel);
        setupNewChannel(rpcChannel);
    }

    public RpcChannel getChannel() {
        return rpcChannel;
    }

}
