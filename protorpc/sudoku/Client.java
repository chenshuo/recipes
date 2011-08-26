package sudoku;

import java.net.InetSocketAddress;

import muduo.rpc.NewChannelCallback;
import muduo.rpc.RpcChannel;
import muduo.rpc.RpcClient;
import sudoku.Sudoku.SudokuRequest;
import sudoku.Sudoku.SudokuResponse;
import sudoku.Sudoku.SudokuService;

import com.google.protobuf.RpcCallback;

public class Client {

    private static void blockingConnect(InetSocketAddress addr) {
        RpcClient client = new RpcClient();
        RpcChannel channel = client.blockingConnect(addr);
        sendRequest(channel);
    }

    private static void asyncConnect(InetSocketAddress addr) {
        RpcClient client = new RpcClient();
        client.registerService(Sudoku.SudokuService.newReflectiveService(new SudokuImpl()));
        client.startConnect(addr, new NewChannelCallback() {
            @Override
            public void run(RpcChannel channel) {
                sendRequest(channel);
            }
        });
    }

    private static void sendRequest(RpcChannel channel) {
        SudokuService remoteService = Sudoku.SudokuService.newStub(channel);
        SudokuRequest request = SudokuRequest.newBuilder().setCheckerboard("001010").build();
        remoteService.solve(null, request, new RpcCallback<Sudoku.SudokuResponse>() {
            @Override
            public void run(SudokuResponse parameter) {
                System.out.println(parameter);
            }
        });
    }

    public static void main(String[] args) throws Exception {
        InetSocketAddress addr = new InetSocketAddress("localhost", 8888);
        // asyncConnect(addr);
        blockingConnect(addr);
    }
}
