package echo;

import java.net.InetSocketAddress;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executors;

import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;

import com.google.protobuf.ServiceException;

import muduo.rpc.RpcChannel;
import muduo.rpc.RpcClient;
import echo.EchoProto.EchoRequest;
import echo.EchoProto.EchoResponse;
import echo.EchoProto.EchoService;
import echo.EchoProto.EchoService.BlockingInterface;

public class EchoClient {
    static final int kRequests = 20000;

    public static class Client implements Runnable {
        private ChannelFactory channelFactory;
        private InetSocketAddress serverAddr;
        private CountDownLatch latch;

        public Client(ChannelFactory channelFactory, InetSocketAddress server, CountDownLatch latch) {
            this.channelFactory = channelFactory;
            this.serverAddr = server;
            this.latch = latch;
        }

        @Override
        public void run() {
            System.out.println(Thread.currentThread());
            RpcClient client = new RpcClient(channelFactory);
            RpcChannel channel = client.blockingConnect(serverAddr);
            BlockingInterface remoteService = EchoService.newBlockingStub(channel);
            String payload = new String(new byte[100]);
            payload = "Hello";
            EchoRequest request = EchoRequest.newBuilder().setPayload(payload).build();

            for (int i = 0; i < kRequests; ++i) {
                EchoResponse response;
                try {
                    response = remoteService.echo(null, request);
                    assert response.getPayload().equals(payload);
                } catch (ServiceException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                // System.out.println(response);
            }
            latch.countDown();
            System.out.println(Thread.currentThread());
            // System.out.println(response);
            channel.disconnect();
            // client.stop();
        }
    }

    public static void main(String[] args) throws Exception {
        ChannelFactory channelFactory = new NioClientSocketChannelFactory(
                Executors.newCachedThreadPool(),
                Executors.newCachedThreadPool());
        InetSocketAddress server = new InetSocketAddress(args[0], 8888);
        int N = 4;
        CountDownLatch latch = new CountDownLatch(N);
        long start = System.currentTimeMillis();
        Thread[] threads = new Thread[N];
        for (int i = 0; i < N; ++i) {
            threads[i] = new Thread(new Client(channelFactory, server, latch));
            threads[i].start();
        }
        latch.await();
        long end = System.currentTimeMillis();
        System.err.println(end - start);
        System.err.println(N * kRequests * 1000L / (end - start));
    }

}
