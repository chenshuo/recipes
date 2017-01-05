#include <muduo/net/Buffer.h>

#include <stdio.h>

#include <tls.h>

#include "timer.h"

muduo::net::Buffer clientOut, serverOut;

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
}

