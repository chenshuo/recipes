package sudoku;

import muduo.rpc.NewChannelCallback;
import muduo.rpc.RpcChannel;
import muduo.rpc.RpcServer;

public class Server {

    public static void main(String[] args) {
        RpcServer server = new RpcServer();
        server.registerService(Sudoku.SudokuService.newReflectiveService(new SudokuImpl()));
        server.setNewChannelCallback(new NewChannelCallback() {

            @Override
            public void run(RpcChannel channel) {
                // TODO call client

            }
        });
        server.start(9981);
    }
}
