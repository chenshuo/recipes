#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <stdio.h>

#include "timer.h"

int main(int argc, char* argv[])
{
  printf("Compiled with " OPENSSL_VERSION_TEXT "\n");
  SSL_load_error_strings();
  // ERR_load_BIO_strings();
  SSL_library_init();
  OPENSSL_config(NULL);

  SSL_CTX* ctx = SSL_CTX_new(TLSv1_2_server_method());
  SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);

  EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
  SSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE);
  // if (argc > 3)
  SSL_CTX_set_tmp_ecdh(ctx, ecdh);
  EC_KEY_free(ecdh);

  const char* CertFile = "server.pem";  // argv[1];
  const char* KeyFile = "server.pem";  // argv[2];
  SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM);
  SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM);
  if (!SSL_CTX_check_private_key(ctx))
    abort();

  SSL_CTX* ctx_client = SSL_CTX_new(TLSv1_2_client_method());

  double start = now();
  const int N = 1000;
  SSL *ssl, *ssl_client;
  Timer tc, ts;
  for (int i = 0; i < N; ++i)
  {
    BIO *client, *server;
    BIO_new_bio_pair(&client, 0, &server, 0);

    ssl = SSL_new (ctx);
    ssl_client = SSL_new (ctx_client);
    SSL_set_bio(ssl, server, server);
    SSL_set_bio(ssl_client, client, client);

    tc.start();
    int ret = SSL_connect(ssl_client);
    tc.stop();
    //printf("%d %d\n", ret, BIO_retry_type(&server));
    ts.start();
    int ret2 = SSL_accept(ssl);
    ts.stop();
    //printf("%d %d\n", ret2, BIO_retry_type(&client));

    while (true)
    {
      tc.start();
      ret = SSL_do_handshake(ssl_client);
      tc.stop();
      //printf("client handshake %d %d\n", ret, BIO_retry_type(&server));
      ts.start();
      ret2 = SSL_do_handshake(ssl);
      ts.stop();
      //printf("server handshake %d %d\n", ret2, BIO_retry_type(&client));
      //if (ret == -1 && BIO_retry_type(&server) == 0)
      //  break;
      //if (ret2 == -1 && BIO_retry_type(&client) == 0)
      //  break;
      if (ret == 1 && ret2 == 1)
        break;
    }

    if (i == 0)
    {
      printf("SSL connection using %s %s\n", SSL_get_version(ssl_client), SSL_get_cipher (ssl_client));
#ifdef OPENSSL_IS_BORINGSSL
      printf("Curve: %s\n", SSL_get_curve_name(SSL_get_curve_id(ssl_client)));
#elif OPENSSL_VERSION_NUMBER >= 0x10002000L
      EVP_PKEY *key;
      if (SSL_get_server_tmp_key(ssl_client, &key))
      {
        if (EVP_PKEY_id(key) == EVP_PKEY_EC)
        {
          EC_KEY *ec = EVP_PKEY_get1_EC_KEY(key);
          int nid = EC_GROUP_get_curve_name(EC_KEY_get0_group(ec));
          EC_KEY_free(ec);
          const char *cname = EC_curve_nid2nist(nid);
          if (!cname)
            cname = OBJ_nid2sn(nid);
          printf("Curve: %s, %d bits\n", cname, EVP_PKEY_bits(key));
        }
      }
#endif
    }
    if (i != N-1)
    {
      SSL_free (ssl);
      SSL_free (ssl_client);
    }
  }
  double elapsed = now() - start;
  printf("%.2fs %.1f handshakes/s\n", elapsed, N / elapsed);
  printf("client %.3f %.1f\n", tc.seconds(), N / tc.seconds());
  printf("server %.3f %.1f\n", ts.seconds(), N / ts.seconds());
  printf("server/client %.2f\n", ts.seconds() / tc.seconds());


  double start2 = now();
  const int M = 300;
  char buf[1024] = { 0 };
  for (int i = 0; i < M*1024; ++i)
  {
    int nw = SSL_write(ssl_client, buf, sizeof buf);
    if (nw != sizeof buf)
    {
      printf("nw = %d\n", nw);
    }
    int nr = SSL_read(ssl, buf, sizeof buf);
    if (nr != sizeof buf)
    {
      printf("nr = %d\n", nr);
    }
  }
  elapsed = now() - start2;
  printf("%.2f %.1f MiB/s\n", elapsed, M / elapsed);
  SSL_free (ssl);
  SSL_free (ssl_client);

  SSL_CTX_free (ctx);
  SSL_CTX_free (ctx_client);
}
