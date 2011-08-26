package muduo.rpc;

public interface NewChannelCallback {

    public abstract void run(RpcChannel channel);
}
