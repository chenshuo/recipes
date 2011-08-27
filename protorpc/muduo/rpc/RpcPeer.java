package muduo.rpc;

import java.util.Collections;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.jboss.netty.channel.Channel;

import com.google.protobuf.Service;

public abstract class RpcPeer {

    protected NewChannelCallback newChannelCallback;
    protected Map<String, Service> services = new ConcurrentHashMap<String, Service>();

    public void registerService(Service service) {
        services.put(service.getDescriptorForType().getFullName(), service);
    }

    public void setNewChannelCallback(NewChannelCallback newChannelCallback) {
        this.newChannelCallback = newChannelCallback;
    }

    protected void setupNewChannel(RpcChannel rpcChannel) {
        Channel channel = rpcChannel.getChannel();
        RpcMessageHandler handler = (RpcMessageHandler) channel.getPipeline().get("handler");
        handler.setChannel(rpcChannel);
        rpcChannel.setServiceMap(Collections.unmodifiableMap(services));
        if (newChannelCallback != null) {
            newChannelCallback.run(rpcChannel);
        }
    }

    public abstract void channelConnected(Channel channel);
}
