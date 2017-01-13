#include <muduo/net/Buffer.h>

#include <stdio.h>

#include <tls.h>

#include "timer.h"

muduo::net::Buffer clientOut, serverOut;
int64_t clientWrite, serverWrite;

ssize_t net_read(struct tls *ctx, void *buf, size_t len, void *arg)
{
  muduo::net::Buffer* in = ((arg == &clientOut) ? &serverOut : &clientOut);
  if (in->readableBytes() > 0)
  {
    size_t n = std::min(in->readableBytes(), len);
    memcpy(buf, in->peek(), n);
    in->retrieve(n);
    return n;
  }
  else
  {
    return TLS_WANT_POLLIN;
  }
}

ssize_t net_write(struct tls *ctx, const void *buf, size_t len, void *arg)
{
  muduo::net::Buffer* out = static_cast<muduo::net::Buffer*>(arg);
  int64_t& wr = (out == &clientOut ? clientWrite : serverWrite);
  wr += len;
  out->append(buf, len);
  return len;
}

struct tls* client()
{
  struct tls_config* cfg = tls_config_new();
  assert(cfg != NULL);

  tls_config_set_ca_file(cfg, "ca.pem");
  // tls_config_insecure_noverifycert(cfg);
  // tls_config_insecure_noverifyname(cfg);

  struct tls* ctx = tls_client();
  assert(ctx != NULL);

  int ret = tls_configure(ctx, cfg);
  assert(ret == 0);

  tls_connect_cbs(ctx, net_read, net_write, &clientOut, "Test Server Cert");

  return ctx;
}

struct tls* server()
{
  struct tls_config* cfg = tls_config_new();
  assert(cfg != NULL);

  int ret = tls_config_set_cert_file(cfg, "server.pem");
  assert(ret == 0);

  ret = tls_config_set_key_file(cfg, "server.pem");
  assert(ret == 0);

  ret = tls_config_set_ecdhecurve(cfg, "prime256v1");
  assert(ret == 0);

  // tls_config_verify_client_optional(cfg);
  struct tls* ctx = tls_server();
  assert(ctx != NULL);

  ret = tls_configure(ctx, cfg);
  assert(ret == 0);

  struct tls* conn_ctx = NULL;
  tls_accept_cbs(ctx, &conn_ctx, net_read, net_write, &serverOut);
  return conn_ctx;
}

Timer tclient, tserver;

void handshake(struct tls* cctx, struct tls* sctx)
{
  int client_done = false, server_done = false;
  while (!(client_done && server_done))
  {
    if (!client_done)
    {
      tclient.start();
      int ret = tls_handshake(cctx);
      tclient.stop();
      // printf("c %d\n", ret);
      if (ret == 0)
        client_done = true;
      else if (ret == -1)
        break;
    }

    if (!server_done)
    {
      tserver.start();
      int ret = tls_handshake(sctx);
      tserver.stop();
      // printf("s %d\n", ret);
      if (ret == 0)
        server_done = true;
      else if (ret == -1)
        break;
    }
  }
}

void throughput(int block_size, struct tls* cctx, struct tls* sctx)
{
  double start = now();
  int total = 0;
  int batch = 1024;
  char* message = new char[block_size];
  bzero(message, block_size);
  clientWrite = 0;
  tclient.reset();
  tserver.reset();
  while (now() - start < 10)
  {
    for (int i = 0; i < batch; ++i)
    {
      tclient.start();
      int nw = tls_write(cctx, message, block_size);
      tclient.stop();
      assert(nw == block_size);
      tserver.start();
      int nr = tls_read(sctx, message, block_size);
      tserver.stop();
      assert(nr == block_size);
    }
    total += batch;
    batch *= 2;
  }
  double secs = now() - start;
  // throughput is half of real value, because client and server share one core.
  printf("bs %5d sec %.3f tot %d thr %.1fKB/s wr %.2fB client %.3f server %.3f\n", block_size, secs, total,
         block_size / secs * total / 1024, clientWrite * 1.0 / total, tclient.seconds(), tserver.seconds());
  delete[] message;
}

int main(int argc, char* argv[])
{
  int ret = tls_init();
  assert(ret == 0);

  struct tls* cctx = client();
  struct tls* sctx = server();

  const int N = 500;
  Timer all, client, server;
  all.start();
  for (int i = 0; i < N; ++i)
  {
    handshake(cctx, sctx);

    if (i == 0)
      printf("cipher %s\n", tls_conn_cipher(cctx));
  }
  all.stop();
  printf("%f secs, %f handshakes/sec\n", all.seconds(), N / all.seconds());
  printf("client %f secs, server %f secs\n", tclient.seconds(), tserver.seconds());

  for (int i = 1; i <= 1024 * 16; i *= 2)
  {
    throughput(i, cctx, sctx);
  }
}

