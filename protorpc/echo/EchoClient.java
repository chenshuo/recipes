package echo;

import java.net.InetSocketAddress;

import muduo.rpc.RpcChannel;
import muduo.rpc.RpcClient;
import echo.EchoProto.EchoRequest;
import echo.EchoProto.EchoResponse;
import echo.EchoProto.EchoService;
import echo.EchoProto.EchoService.BlockingInterface;

public class EchoClient {

    public static void main(String[] args) throws Exception {
        RpcClient client = new RpcClient();
        RpcChannel channel = client.blockingConnect(new InetSocketAddress(args[0], 8888));
        BlockingInterface remoteService = EchoService.newBlockingStub(channel);
        String payload = new String(new byte[100]);
        payload = "Hello";
        EchoRequest request = EchoRequest.newBuilder().setPayload(payload).build();
        long start = System.currentTimeMillis();
        int N = 20000;
        for (int i = 0; i < N; ++i) {
            EchoResponse response = remoteService.echo(null, request);
            assert response.getPayload().equals(payload);
            //System.out.println(response);
        }
        long end = System.currentTimeMillis();
        System.err.println(end-start);
        System.err.println(N*1000L/(end-start));
        // System.out.println(response);
        channel.disconnect();
        client.stop();
    }

}
