package muduo.rpc;

import java.net.InetSocketAddress;
import java.util.concurrent.Executors;

import org.jboss.netty.bootstrap.ServerBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.socket.nio.NioServerSocketChannelFactory;

public class RpcServer extends RpcPeer {

    ServerBootstrap bootstrap;

    public RpcServer() {
        ChannelFactory channelFactory = new NioServerSocketChannelFactory(
                Executors.newCachedThreadPool(),
                Executors.newCachedThreadPool());
        bootstrap = new ServerBootstrap(channelFactory);
        bootstrap.setPipelineFactory(new RpcChannelPiplineFactory(this));
    }

    public void start(int port) {
        bootstrap.bind(new InetSocketAddress(port));
    }

    @Override
    public void channelConnected(Channel channel) {
        RpcChannel rpcChannel = new RpcChannel(channel);
        setupNewChannel(rpcChannel);
    }
}
