#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <malloc.h>
#include <mcheck.h>
#include <stdio.h>

#include "timer.h"

#include <string>
#include <vector>

void (*old_free_hook) (void *__ptr, const void *);
void *(*old_malloc_hook)(size_t __size, const void *);

void my_free_hook (void*, const void *);

void* my_malloc_hook(size_t size, const void* caller)
{
  void *result;
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  /* Call recursively */
  result = malloc (size);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  /* printf might call malloc, so protect it too. */
  printf ("%p malloc (%u) returns %p\n", caller, (unsigned int) size, result);
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
  return result;
}

void my_free_hook (void *ptr, const void *caller)
{
  if (!ptr) return;
  /* Restore all old hooks */
  __malloc_hook = old_malloc_hook;
  __free_hook = old_free_hook;
  /* Call recursively */
  free (ptr);
  /* Save underlying hooks */
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  /* printf might call free, so protect it too. */
  printf ("freed %p\n", ptr);
  /* Restore our own hooks */
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
}

void init_hook()
{
  old_malloc_hook = __malloc_hook;
  old_free_hook = __free_hook;
  __malloc_hook = my_malloc_hook;
  __free_hook = my_free_hook;
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

  init_hook();

  const int N = 10;
  SSL *ssl, *ssl_client;
  std::vector<SSL*> ssls;
  for (int i = 0; i < N; ++i)
  {
    printf("=============================================== BIO_new_bio_pair %d\n", i);
    BIO *client, *server;
    BIO_new_bio_pair(&client, 0, &server, 0);

    printf("=============================================== SSL_new server %d\n", i);
    ssl = SSL_new (ctx);
    printf("=============================================== SSL_new client %d\n", i);
    ssl_client = SSL_new (ctx_client);
    SSL_set_bio(ssl, server, server);
    SSL_set_bio(ssl_client, client, client);

    printf("=============================================== SSL_connect client %d\n", i);
    int ret = SSL_connect(ssl_client);
    printf("=============================================== SSL_accept server %d\n", i);
    int ret2 = SSL_accept(ssl);

    while (true)
    {
      printf("=============================================== SSL_handshake client %d\n", i);
      ret = SSL_do_handshake(ssl_client);
      printf("=============================================== SSL_handshake server %d\n", i);
      ret2 = SSL_do_handshake(ssl);
      if (ret == 1 && ret2 == 1)
        break;
    }

    if (i == 0)
      printf ("SSL connection using %s %s\n", SSL_get_version(ssl_client), SSL_get_cipher (ssl_client));
    /*
    if (i != N-1)
    {
      printf("=============================================== SSL_free server %d\n", i);
      SSL_free (ssl);
      printf("=============================================== SSL_free client %d\n", i);
      SSL_free (ssl_client);
    }
    else
    */
    {
      ssls.push_back(ssl);
      ssls.push_back(ssl_client);
    }
  }

  printf("=============================================== data \n");

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
  double elapsed = now() - start2;
  printf("%.2f %.1f MiB/s\n", elapsed, M / elapsed);
  printf("=============================================== SSL_free\n");
  for (int i = 0; i < ssls.size(); ++i)
    SSL_free(ssls[i]);

  printf("=============================================== SSL_CTX_free\n");
  SSL_CTX_free (ctx);
  SSL_CTX_free (ctx_client);
  // OPENSSL_cleanup();  // only in 1.1.0
  printf("=============================================== end\n");
}
