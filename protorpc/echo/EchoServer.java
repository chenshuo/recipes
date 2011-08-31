package echo;

import com.google.protobuf.RpcCallback;
import com.google.protobuf.RpcController;

import echo.EchoProto.EchoRequest;
import echo.EchoProto.EchoResponse;
import echo.EchoProto.EchoService.Interface;
import muduo.rpc.RpcServer;

public class EchoServer {

    public static void main(String[] args) {
        RpcServer server = new RpcServer();
        server.registerService(EchoProto.EchoService.newReflectiveService(new Interface() {
            @Override
            public void echo(RpcController controller, EchoRequest request, RpcCallback<EchoResponse> done) {
                done.run(EchoResponse.newBuilder().setPayload(request.getPayload()).build());
            }
        }));
        server.start(8888);
    }
}
