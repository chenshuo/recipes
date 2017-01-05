#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <tls.h>

struct tls* client(int sockfd)
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

  ret = tls_connect_socket(ctx, sockfd, "Test Server Cert");
  assert(ret == 0);

  return ctx;
}

struct tls* server(int sockfd)
{
  struct tls_config* cfg = tls_config_new();
  assert(cfg != NULL);

  int ret = tls_config_set_cert_file(cfg, "server.pem");
  assert(ret == 0);

  ret = tls_config_set_key_file(cfg, "server.pem");
  assert(ret == 0);

  tls_config_verify_client_optional(cfg);
  struct tls* ctx = tls_server();
  assert(ctx != NULL);

  ret = tls_configure(ctx, cfg);
  assert(ret == 0);

  struct tls* sctx = NULL;
  ret = tls_accept_socket(ctx, &sctx, sockfd);
  assert(ret == 0 && sctx != NULL);

  return sctx;
}

bool handshake(struct tls* cctx, struct tls* sctx)
{
  int client_done = false, server_done = false;

  while (!(client_done && server_done))
  {
    if (!client_done)
    {
      int ret = tls_handshake(cctx);
      printf("c %d\n", ret);
      if (ret == 0)
        client_done = true;
      else if (ret == -1)
      {
        printf("client handshake failed: %s\n", tls_error(cctx));
        break;
      }
    }

    if (!server_done)
    {
      int ret = tls_handshake(sctx);
      printf("s %d\n", ret);
      if (ret == 0)
        server_done = true;
      else if (ret == -1)
      {
        printf("server handshake failed: %s\n", tls_error(cctx));
        break;
      }
    }
  }

  return client_done && server_done;
}

int main(int argc, char* argv[])
{
  int ret = tls_init();
  assert(ret == 0);

  int fds[2];
  socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);

  struct tls* cctx = client(fds[0]);
  struct tls* sctx = server(fds[1]);

  if (handshake(cctx, sctx))
    printf("cipher %s\n", tls_conn_cipher(cctx));
}
