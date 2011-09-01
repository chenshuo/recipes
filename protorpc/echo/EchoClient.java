package echo;

import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executors;

import muduo.rpc.NewChannelCallback;
import muduo.rpc.RpcChannel;
import muduo.rpc.RpcClient;

import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;

import com.google.protobuf.RpcCallback;
import com.google.protobuf.ServiceException;

import echo.EchoProto.EchoRequest;
import echo.EchoProto.EchoResponse;
import echo.EchoProto.EchoService;
import echo.EchoProto.EchoService.BlockingInterface;

public class EchoClient {
    static final int kRequests = 50000;
    static CountDownLatch allConnected, startLatch, allFinished;

    public static class SyncClient implements Runnable {
        private ChannelFactory channelFactory;
        private SocketAddress serverAddr;

        public SyncClient(ChannelFactory channelFactory, SocketAddress server) {
            this.channelFactory = channelFactory;
            this.serverAddr = server;
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

            allConnected.countDown();
            try {
                startLatch.await();
            } catch (InterruptedException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
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
            allFinished.countDown();
            System.out.println(Thread.currentThread());
            // System.out.println(response);
            channel.disconnect();
            // client.stop();
        }
    }

    @SuppressWarnings("unused")
    private static void runSyncClients(InetSocketAddress server, int nClients, int nSelectors)
            throws InterruptedException {
        ChannelFactory channelFactory = new NioClientSocketChannelFactory(
                Executors.newCachedThreadPool(),
                Executors.newCachedThreadPool(),
                nSelectors);
        allConnected = new CountDownLatch(nClients);
        startLatch = new CountDownLatch(1);
        allFinished = new CountDownLatch(nClients);
        Thread[] threads = new Thread[nClients];
        for (int i = 0; i < nClients; ++i) {
            threads[i] = new Thread(new SyncClient(channelFactory, server));
            threads[i].start();
        }
        allConnected.await();
        long start = System.currentTimeMillis();
        startLatch.countDown();
        allFinished.await();
        long end = System.currentTimeMillis();
        System.err.println(end - start);
        System.err.println(nClients * kRequests * 1000L / (end - start));
    }

    public static class AsyncClient {

        private RpcClient client;
        private SocketAddress serverAddr;
        protected EchoService.Stub remoteService;
        private EchoRequest request;
        private String payload;
        private int nPipelines = 1;
        private int count = 0;

        public AsyncClient(ChannelFactory channelFactory, SocketAddress server) {
            this.client = new RpcClient(channelFactory);
            this.serverAddr = server;

            payload = new String(new byte[100]);
            // payload = "Hello";
            request = EchoRequest.newBuilder().setPayload(payload).build();
        }

        public void connect() {
            client.startConnect(serverAddr, new NewChannelCallback() {
                @Override
                public void run(RpcChannel channel) {
                    remoteService = EchoService.newStub(channel);
                    allConnected.countDown();
                }
            });
        }

        public void start() {
            for (int i = 0; i < nPipelines; ++i) {
                sendAsyncRequest();
            }
        }

        private void sendAsyncRequest() {
            RpcCallback<EchoProto.EchoResponse> done = new RpcCallback<EchoProto.EchoResponse>() {
                @Override
                public void run(EchoResponse response) {
                    assert response.getPayload().equals(payload);
                    ++count;
                    if (count < kRequests) {
                        sendAsyncRequest();
                    } else {
                        allFinished.countDown();
                    }
                }
            };
            remoteService.echo(null, request, done);
        }
    }

    private static void runAsyncClients(SocketAddress server, int nClients, int nSelectors)
            throws Exception {
        ChannelFactory channelFactory = new NioClientSocketChannelFactory(
                Executors.newCachedThreadPool(),
                Executors.newCachedThreadPool(),
                nSelectors);
        allConnected = new CountDownLatch(nClients);
        allFinished = new CountDownLatch(nClients);

        AsyncClient[] clients = new AsyncClient[nClients];
        for (int i = 0; i < nClients; ++i) {
            clients[i] = new AsyncClient(channelFactory, server);
            clients[i].connect();
        }
        allConnected.await();
        long start = System.currentTimeMillis();
        for (AsyncClient client : clients) {
            client.start();
        }
        allFinished.await();
        long end = System.currentTimeMillis();
        System.err.println(end - start);
        System.err.println(nClients * kRequests * 1000L / (end - start));
        System.exit(0);
    }

    public static void main(String[] args) throws Exception {
        InetSocketAddress server = new InetSocketAddress(args[0], 8888);
        int nClients = 2;
        int nSelectors = 2;

        // runSyncClients(server, nClients, nSelectors);
        runAsyncClients(server, nClients, nSelectors);
    }
}
