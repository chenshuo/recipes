package muduo.rpc;

import java.net.SocketAddress;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.ChannelFutureListener;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;

public class RpcClient extends RpcPeer {

    ClientBootstrap bootstrap;
    private volatile RpcChannel rpcChannel;

    public RpcClient() {
        ChannelFactory channelFactory = new NioClientSocketChannelFactory(
                Executors.newCachedThreadPool(),
                Executors.newCachedThreadPool());

        bootstrap = new ClientBootstrap(channelFactory);
        bootstrap.setPipelineFactory(new RpcChannelPiplineFactory(this));
    }

    public RpcClient(ChannelFactory channelFactory) {
        bootstrap = new ClientBootstrap(channelFactory);
        bootstrap.setPipelineFactory(new RpcChannelPiplineFactory(this));
    }

    public RpcChannel blockingConnect(SocketAddress addr) {
        final CountDownLatch latch = new CountDownLatch(1);
        startConnect(addr, new NewChannelCallback() {
            @Override
            public void run(RpcChannel channel) {
                assert channel == RpcClient.this.rpcChannel;
                latch.countDown();
            }
        });
        try {
            latch.await(5, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return rpcChannel;
    }

    public void startConnect(SocketAddress addr, NewChannelCallback newChannelCallback) {
        ChannelFuture future = bootstrap.connect(addr);
        future.addListener(new ChannelFutureListener() {
            @Override
            public void operationComplete(ChannelFuture future) throws Exception {
                //System.err.println("operationComplete");
            }
        });
        this.newChannelCallback = newChannelCallback;
    }

    public void stop() {
        bootstrap.releaseExternalResources();
    }

    @Override
    public void channelConnected(Channel channel) {
        if (rpcChannel == null) {
            rpcChannel = new RpcChannel(channel);
            setupNewChannel(rpcChannel);
        }
    }

    public RpcChannel getChannel() {
        return rpcChannel;
    }

}
