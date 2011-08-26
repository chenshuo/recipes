package sudoku;

import sudoku.Sudoku.SudokuRequest;
import sudoku.Sudoku.SudokuResponse;
import sudoku.Sudoku.SudokuService.Interface;

import com.google.protobuf.RpcCallback;
import com.google.protobuf.RpcController;

public class SudokuImpl implements Interface {

    @Override
    public void solve(RpcController controller, SudokuRequest request,
            RpcCallback<SudokuResponse> done) {
        SudokuResponse resp = SudokuResponse.newBuilder()
                .setSolved(true).setCheckerboard("12345").build();
        done.run(resp);
    }
}
