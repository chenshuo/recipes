package muduo.rpc;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

import muduo.rpc.proto.RpcProto.ErrorCode;
import muduo.rpc.proto.RpcProto.MessageType;
import muduo.rpc.proto.RpcProto.RpcMessage;
import muduo.rpc.proto.RpcProto.RpcMessage.Builder;

import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.MessageEvent;

import com.google.protobuf.BlockingRpcChannel;
import com.google.protobuf.ByteString;
import com.google.protobuf.Descriptors.MethodDescriptor;
import com.google.protobuf.Message;
import com.google.protobuf.RpcCallback;
import com.google.protobuf.RpcController;
import com.google.protobuf.Service;
import com.google.protobuf.ServiceException;

public class RpcChannel implements com.google.protobuf.RpcChannel, BlockingRpcChannel {

    private final static class BlockingRpcCallback implements RpcCallback<Message> {
        public Message response;

        @Override
        public void run(Message response) {
            synchronized (this) {
                this.response = response;
                notify();
            }
        }
    }

    private final static class Outstanding {

        public Message responsePrototype;
        public RpcCallback<Message> done;

        public Outstanding(Message responsePrototype, RpcCallback<Message> done) {
            this.responsePrototype = responsePrototype;
            this.done = done;
        }
    }

    private Channel channel;
    private AtomicLong id = new AtomicLong(1);
    private Map<Long, Outstanding> outstandings = new ConcurrentHashMap<Long, Outstanding>();
    private Map<String, Service> services;

    public RpcChannel(Channel channel) {
        this.channel = channel;
    }

    public void setServiceMap(Map<String, Service> services) {
        this.services = services;
    }

    public Channel getChannel() {
        return channel;
    }

    public void disconnect() {
        channel.disconnect();
    }

    public void messageReceived(ChannelHandlerContext ctx, final MessageEvent e) {
        RpcMessage message = (RpcMessage) e.getMessage();
        assert e.getChannel() == channel;
        // System.out.println(message);
        if (message.getType() == MessageType.REQUEST) {
            doRequest(message);
        } else if (message.getType() == MessageType.RESPONSE) {
            Outstanding o = outstandings.remove(message.getId());
            // System.err.println("messageReceived " + this);
            if (o != null) {
                Message resp = fromByteString(o.responsePrototype, message.getResponse());
                o.done.run(resp);
            } else {
                System.err.println("Unknown id " + message.getId());
            }
        }
    }

    private void doRequest(RpcMessage message) {
        Service service = services.get(message.getService());
        Builder errorBuilder = RpcMessage.newBuilder().setType(MessageType.ERROR);
        boolean succeed = false;
        if (service != null) {
            MethodDescriptor method = service.getDescriptorForType()
                    .findMethodByName(message.getMethod());
            if (method != null) {
                Message request = fromByteString(service.getRequestPrototype(method),
                        message.getRequest());
                if (request != null) {
                    final long id = message.getId();
                    RpcCallback<Message> done = new RpcCallback<Message>() {
                        @Override
                        public void run(Message response) {
                            done(response, id);
                        }
                    };
                    succeed = doCall(request, service, method, done);
                } else {
                    errorBuilder.setError(ErrorCode.INVALID_REQUEST);
                }
            } else {
                errorBuilder.setError(ErrorCode.NO_METHOD);
            }
        } else {
            errorBuilder.setError(ErrorCode.NO_SERVICE);
        }
        if (!succeed) {
            RpcMessage resp = errorBuilder.build();
            channel.write(resp);
        }
    }

    private Message fromByteString(Message prototype, ByteString bytes) {
        Message message = null;
        try {
            message = prototype.toBuilder().mergeFrom(bytes).build();
        } catch (Exception e) {
        }
        return message;
    }

    private boolean doCall(Message request, Service service, MethodDescriptor method,
            RpcCallback<Message> done) {
        service.callMethod(method, null, request, done);
        return true;
    }

    protected void done(Message response, long id) {
        if (response != null) {
            RpcMessage resp = RpcMessage.newBuilder()
                    .setType(MessageType.RESPONSE)
                    .setId(id)
                    .setResponse(response.toByteString())
                    .build();
            channel.write(resp);
        } else {
            RpcMessage resp = RpcMessage.newBuilder()
                    .setType(MessageType.ERROR)
                    .setId(id)
                    .setError(ErrorCode.INVALID_RESPONSE)
                    .build();
            channel.write(resp);
        }
    }

    @Override
    public void callMethod(MethodDescriptor method, RpcController controller, Message request,
            Message responsePrototype, RpcCallback<Message> done) {
        long callId = id.getAndIncrement();
        RpcMessage message = RpcMessage.newBuilder()
                .setType(MessageType.REQUEST)
                .setId(callId)
                .setService(method.getService().getFullName())
                .setMethod(method.getName())
                .setRequest(request.toByteString())
                .build();
        outstandings.put(callId, new Outstanding(responsePrototype, done));
        channel.write(message);
    }

    @Override
    public Message callBlockingMethod(MethodDescriptor method, RpcController controller,
            Message request, Message responsePrototype) throws ServiceException {
        BlockingRpcCallback done = new BlockingRpcCallback();
        callMethod(method, controller, request, responsePrototype, done);
        // if (channel instanceof NioClientSocketChannel)
        // channel.get
        // assert
        synchronized (done) {
            while (done.response == null) {
                try {
                    done.wait();
                } catch (InterruptedException e) {
                }
            }
        }
        return done.response;
    }
}
