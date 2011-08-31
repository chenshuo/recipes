package sudoku;

import java.net.InetSocketAddress;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import muduo.rpc.NewChannelCallback;
import muduo.rpc.RpcChannel;
import muduo.rpc.RpcClient;
import sudoku.Sudoku.SudokuRequest;
import sudoku.Sudoku.SudokuResponse;
import sudoku.Sudoku.SudokuService;
import sudoku.Sudoku.SudokuService.BlockingInterface;

import com.google.protobuf.RpcCallback;

public class Client {

    private static void blockingConnect(InetSocketAddress addr) throws Exception {
        RpcClient client = new RpcClient();
        RpcChannel channel = client.blockingConnect(addr);
        //sendRequest(channel, client);
        BlockingInterface remoteService = Sudoku.SudokuService.newBlockingStub(channel);
        SudokuRequest request = SudokuRequest.newBuilder().setCheckerboard("001010").build();
        SudokuResponse response = remoteService.solve(null, request);
        System.out.println(response);
        channel.disconnect();
        client.stop();
    }

    @SuppressWarnings("unused")
    private static void asyncConnect(InetSocketAddress addr) {
        final RpcClient client = new RpcClient();
        client.registerService(Sudoku.SudokuService.newReflectiveService(new SudokuImpl()));
        client.startConnect(addr, new NewChannelCallback() {
            @Override
            public void run(RpcChannel channel) {
                sendAsyncRequest(channel, client);
            }
        });
    }

    private static void sendAsyncRequest(final RpcChannel channel, RpcClient client) {
        final CountDownLatch latch = new CountDownLatch(1);
        System.err.println("sendRequest " + channel);
        SudokuService remoteService = Sudoku.SudokuService.newStub(channel);
        SudokuRequest request = SudokuRequest.newBuilder().setCheckerboard("001010").build();
        remoteService.solve(null, request, new RpcCallback<Sudoku.SudokuResponse>() {
            @Override
            public void run(SudokuResponse parameter) {
                System.out.println(parameter);
                channel.disconnect();
                latch.countDown();
            }
        });
        try {
            latch.await(5, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        client.stop();
    }

    public static void main(String[] args) throws Exception {
        InetSocketAddress addr = new InetSocketAddress("localhost", 9981);
        // asyncConnect(addr);
        blockingConnect(addr);
    }
}
