#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <malloc.h>
#include <mcheck.h>
#include <stdio.h>
#include <sys/socket.h>

#include "timer.h"

#include <string>
#include <vector>

using std::string;
string readStatus()
{
  string result;
  FILE* fp = fopen("/proc/self/status", "r");
  char buf[8192];
  int nr = fread(buf, 1, sizeof buf, fp);
  result.append(buf, nr);
  return result;
}

int main(int argc, char* argv[])
{
  SSL_load_error_strings();
  ERR_load_BIO_strings();
  SSL_library_init();
  OPENSSL_config(NULL);

  SSL_CTX* ctx = SSL_CTX_new(TLSv1_2_server_method());
  SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);

  EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
  SSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE);
  SSL_CTX_set_tmp_ecdh(ctx, ecdh);
  EC_KEY_free(ecdh);

  const char* CertFile = "server.pem";  // argv[1];
  const char* KeyFile = "server.pem";  // argv[2];
  SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM);
  SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM);
  if (!SSL_CTX_check_private_key(ctx))
    abort();

  SSL_CTX* ctx_client = SSL_CTX_new(TLSv1_2_client_method());

  string st0 = readStatus();
  double start = now();
  const int N = 1000;
  struct mallinfo mi0 = mallinfo();
  std::vector<SSL*> ssls;

  for (int i = 0; i < N; ++i)
  {
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds) != 0)
      abort();

    SSL* ssl = SSL_new (ctx);
    SSL* ssl_client = SSL_new (ctx_client);
    SSL_set_fd(ssl, fds[0]);
    SSL_set_fd(ssl_client, fds[1]);

    int ret = SSL_connect(ssl_client);
    int ret2 = SSL_accept(ssl);

    while (true)
    {
      ret = SSL_do_handshake(ssl_client);
      ret2 = SSL_do_handshake(ssl);
      if (ret == 1 && ret2 == 1)
        break;
    }

    if (i == 0)
      printf ("SSL connection using %s %s\n", SSL_get_version(ssl_client), SSL_get_cipher (ssl_client));
    ssls.push_back(ssl);
    ssls.push_back(ssl_client);
  }
  double elapsed = now() - start;
  printf("%.2fs %.1f handshakes/s\n", elapsed, N / elapsed);
  struct mallinfo mi1 = mallinfo();
  string st1 = readStatus();
  printf("%s\n", st0.c_str());
  printf("%s\n", st1.c_str());
  for (int i = 0; i < ssls.size(); ++i)
    SSL_free(ssls[i]);

  // string st2 = readStatus();
  struct mallinfo mi2 = mallinfo();
  SSL_CTX_free (ctx);
  SSL_CTX_free (ctx_client);
  // string st3 = readStatus();
  struct mallinfo mi3 = mallinfo();
  // printf("after SSL_free\n%s\n", st2.c_str());
  // printf("after SSL_CTX_free\n%s\n", st3.c_str());
  // OPENSSL_cleanup();  // only in 1.1.0
  printf("\n");
}
