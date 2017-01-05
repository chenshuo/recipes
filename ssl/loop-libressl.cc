#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <tls.h>

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

  return ctx;
}

int main(int argc, char* argv[])
{
  int ret = tls_init();
  assert(ret == 0);

  struct tls_config* cfg = tls_config_new();
  assert(cfg != NULL);

  ret = tls_config_set_cert_file(cfg, "cert.crt");
  assert(ret == 0);

  ret = tls_config_set_key_file(cfg, "keyfile.key");
  assert(ret == 0);

  tls_config_verify_client_optional(cfg);
  struct tls* ctx = tls_server();
  assert(ctx != NULL);

  ret = tls_configure(ctx, cfg);
  // printf("%s\n", tls_error(ctx));
  assert(ret == 0);

  int fds[2];
  socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);

  struct tls* sctx = NULL;
  ret = tls_accept_socket(ctx, &sctx, fds[0]);
  assert(ret == 0);

  struct tls* cctx = client();
  ret = tls_connect_socket(cctx, fds[1], "Test Server Cert");
  assert(ret == 0);

  do {
    printf("ctx ");
    ret = tls_handshake(sctx);
    printf("%d\n", ret);
    ret = tls_handshake(cctx);
    printf("%d\n", ret);
  } while (ret == TLS_WANT_POLLIN || ret == TLS_WANT_POLLOUT);

  printf("%s\n", tls_error(cctx));
  // printf("%s\n", tls_error(sctx));
}
