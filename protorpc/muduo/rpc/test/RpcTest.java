package muduo.rpc.test;

import static org.junit.Assert.assertEquals;

import java.util.HashMap;
import java.util.Map;

import muduo.rpc.RpcChannel;
import muduo.rpc.RpcDecoder;
import muduo.rpc.RpcEncoder;
import muduo.rpc.proto.RpcProto.MessageType;
import muduo.rpc.proto.RpcProto.RpcMessage;

import org.jboss.netty.buffer.BigEndianHeapChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.UpstreamMessageEvent;
import org.junit.Test;

import sudoku.Sudoku;
import sudoku.Sudoku.SudokuRequest;
import sudoku.Sudoku.SudokuResponse;
import sudoku.Sudoku.SudokuService;
import sudoku.Sudoku.SudokuService.Interface;

import com.google.protobuf.RpcCallback;
import com.google.protobuf.RpcController;
import com.google.protobuf.Service;

import echo.EchoProto.EchoService;
import echo.EchoServer;
import echo.EchoProto.EchoRequest;

public class RpcTest {

    @Test
    public void testEncoder() throws Exception {
        RpcEncoder encoder = new RpcEncoder();
        RpcMessage message = RpcMessage.newBuilder().setType(MessageType.REQUEST).setId(1).build();
        encoder.encode(null, null, message);
    }

    @Test
    public void testEncoder2() throws Exception {
        RpcEncoder encoder = new RpcEncoder();
        EchoRequest request = EchoRequest.newBuilder().setPayload("Hello").build();
        RpcMessage message = RpcMessage.newBuilder().setType(MessageType.REQUEST).setId(1)
                .setService(EchoService.getDescriptor().getFullName())
                .setMethod(EchoService.getDescriptor().getMethods().get(0).getName())
                .setRequest(request.toByteString()).build();
        encoder.encode(null, null, message);
    }

    @Test
    public void testDecoder() throws Exception {
        RpcEncoder encoder = new RpcEncoder();
        SudokuRequest request = SudokuRequest.newBuilder().setCheckerboard("001010").build();
        RpcMessage message = RpcMessage.newBuilder().setType(MessageType.REQUEST).setId(1)
                .setService(SudokuService.getDescriptor().getFullName())
                .setMethod(SudokuService.getDescriptor().getMethods().get(0).getName())
                .setRequest(request.toByteString()).build();
        ChannelBuffer buffer = (ChannelBuffer) encoder.encode(null, null, message);

        RpcDecoder decoder = new RpcDecoder();
        RpcMessage decodedMessage = (RpcMessage) decoder.decode(null, null, buffer);
        assertEquals(1, decodedMessage.getId());
    }

    @Test
    public void testDecoder2() throws Exception {
        RpcEncoder encoder = new RpcEncoder();
        RpcMessage message = RpcMessage.newBuilder().setType(MessageType.REQUEST).setId(2).build();
        ChannelBuffer buffer = (ChannelBuffer) encoder.encode(null, null, message);
        ChannelBuffer buf2 = new BigEndianHeapChannelBuffer(buffer.readableBytes() + 8);
        buf2.writeInt(123);
        buf2.writeBytes(buffer);

        buf2.readInt();
        RpcDecoder decoder = new RpcDecoder();
        RpcMessage decodedMessage = (RpcMessage) decoder.decode(null, null, buf2);
        assertEquals(2, decodedMessage.getId());
    }

    SudokuResponse gotResponse;

    @Test
    public void testClient() throws Exception {
        MockChannel mockChannel = new MockChannel();
        RpcChannel channel = new RpcChannel(mockChannel);
        SudokuService remoteService = Sudoku.SudokuService.newStub(channel);
        SudokuRequest request = SudokuRequest.newBuilder().setCheckerboard("001010").build();
        remoteService.solve(null, request, new RpcCallback<Sudoku.SudokuResponse>() {
            @Override
            public void run(SudokuResponse response) {
                // System.out.println(parameter);
                gotResponse = response;
            }
        });
        RpcMessage message = (RpcMessage) mockChannel.message;
        assertEquals(1, message.getId());
        assertEquals(MessageType.REQUEST, message.getType());
        assertEquals(remoteService.getDescriptorForType().getFullName(), message.getService());
        assertEquals("Solve", message.getMethod());

        SudokuResponse sudokuResponse = SudokuResponse.newBuilder()
                .setSolved(true)
                .setCheckerboard("123456")
                .build();
        RpcMessage response = RpcMessage.newBuilder()
                .setType(MessageType.RESPONSE)
                .setId(1)
                .setResponse(sudokuResponse.toByteString())
                .build();
        channel.messageReceived(null, new UpstreamMessageEvent(mockChannel, response, null));
        assertEquals(sudokuResponse, gotResponse);
    }

    SudokuRequest gotRequest;

    @Test
    public void testServer() throws Exception {

        MockChannel mockChannel = new MockChannel();
        Map<String, Service> services = new HashMap<String, Service>();
        final SudokuResponse sudokuResponse = SudokuResponse.newBuilder()
                .setSolved(true)
                .setCheckerboard("98765")
                .build();
        Interface mockImpl = new Interface() {

            @Override
            public void solve(RpcController controller, SudokuRequest request,
                    RpcCallback<SudokuResponse> done) {
                gotRequest = request;

                done.run(sudokuResponse);
            }
        };
        services.put(SudokuService.getDescriptor().getFullName(),
                SudokuService.newReflectiveService(mockImpl));
        RpcChannel channel = new RpcChannel(mockChannel);
        channel.setServiceMap(services);

        SudokuRequest sudokuRequest = SudokuRequest.newBuilder().setCheckerboard("001010").build();
        RpcMessage request = RpcMessage.newBuilder()
                .setType(MessageType.REQUEST)
                .setId(2)
                .setService(SudokuService.getDescriptor().getFullName())
                .setMethod("Solve")
                .setRequest(sudokuRequest.toByteString())
                .build();

        channel.messageReceived(null, new UpstreamMessageEvent(mockChannel, request, null));
        assertEquals(sudokuRequest, gotRequest);

        RpcMessage response = RpcMessage.newBuilder()
                .setType(MessageType.RESPONSE)
                .setId(2)
                .setResponse(sudokuResponse.toByteString())
                .build();
        assertEquals(response, mockChannel.message);
    }
}
