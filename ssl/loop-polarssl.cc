/*
 * Remember to turn off CPU frequency scaling before testing.
 */


#include <polarssl/ctr_drbg.h>
#include <polarssl/error.h>
#include <polarssl/entropy.h>
#include <polarssl/ssl.h>

#include <polarssl/certs.h>

#include <muduo/base/Thread.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>

bool useRSA = false;
bool useECDHE = false;
const int N = 500;

double now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// FIXME: net_recv with buffer

void clientThread(entropy_context* entropy, int* clientFd)
{
  ctr_drbg_context ctr_drbg;
  ctr_drbg_init(&ctr_drbg, entropy_func, entropy, NULL, 0);

  ssl_context ssl;
  bzero(&ssl, sizeof ssl);
  ssl_init(&ssl);
  ssl_set_rng(&ssl, ctr_drbg_random, &ctr_drbg);
  ssl_set_bio(&ssl, &net_recv, clientFd, &net_send, clientFd);
  ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
  ssl_set_authmode(&ssl, SSL_VERIFY_NONE);

  for (int i = 0; i < N; ++i)
  {
    ssl_session_reset( &ssl );
    int ret = 0;
    while ( (ret = ssl_handshake(&ssl)) != 0)
    {
      if (ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE)
      {
        printf("client handshake failed %d\n", ret);
        break;
      }
    }
    if (i == 0)
      printf("client done %s %s\n", ssl_get_version(&ssl), ssl_get_ciphersuite(&ssl));
  }

  ssl_free(&ssl);
}

void serverThread(entropy_context* entropy, int* serverFd)
{
  const char* srv_cert = test_srv_crt_ec;
  const char* srv_key = test_srv_key_ec;
  if (useRSA)
  {
    srv_cert = test_srv_crt;
    srv_key = test_srv_key;
  }
  x509_crt cert;
  x509_crt_init(&cert);
  x509_crt_parse(&cert, reinterpret_cast<const unsigned char*>(srv_cert), strlen(srv_cert));
  x509_crt_parse(&cert, reinterpret_cast<const unsigned char*>(test_ca_list), strlen(test_ca_list));

  pk_context pkey;
  pk_init(&pkey);
  pk_parse_key(&pkey, reinterpret_cast<const unsigned char*>(srv_key), strlen(srv_key), NULL, 0);

  ctr_drbg_context ctr_drbg;
  ctr_drbg_init(&ctr_drbg, entropy_func, entropy, NULL, 0);

  ssl_context ssl_server;
  bzero(&ssl_server, sizeof ssl_server);
  ssl_init(&ssl_server);
  ssl_set_rng(&ssl_server, ctr_drbg_random, &ctr_drbg);
  ssl_set_bio(&ssl_server, &net_recv, serverFd, &net_send, serverFd);
  ssl_set_endpoint(&ssl_server, SSL_IS_SERVER);
  ssl_set_authmode(&ssl_server, SSL_VERIFY_NONE);
  ssl_set_ca_chain(&ssl_server, cert.next, NULL, NULL);
  ssl_set_own_cert(&ssl_server, &cert, &pkey);
  // ssl_set_dbg(&ssl_server, my_debug, (void*)"server");
  ecp_group_id curves[] = { POLARSSL_ECP_DP_SECP256R1, POLARSSL_ECP_DP_NONE };
  ssl_set_curves(&ssl_server, curves);
  int ciphersuites[] = { TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA, TLS_RSA_WITH_AES_256_CBC_SHA, 0 };
  if (!useECDHE)
    ssl_set_ciphersuites(&ssl_server, ciphersuites);

  for (int i = 0; i < N; ++i)
  {
    ssl_session_reset(&ssl_server);
    int ret = 0;
    while ( (ret = ssl_handshake(&ssl_server)) != 0)
    {
      if (ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE)
      {
        printf("server handshake failed %d\n", ret);
        break;
      }
    }
    if (i == 0)
      printf("server done %s %s\n", ssl_get_version(&ssl_server), ssl_get_ciphersuite(&ssl_server));
  }

  ssl_free(&ssl_server);
  pk_free(&pkey);
  x509_crt_free(&cert);
}

int main(int argc, char* argv[])
{
  unsigned char buf[16384] = { 0 };
  entropy_context entropy;
  entropy_init(&entropy);

  if (argc > 1)
    useRSA = true;

  useECDHE = argc > 2;

  int fds[2];
  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fds))
    abort();

  double start = now();
  muduo::Thread client(boost::bind(&clientThread, &entropy, &fds[0]), "ssl client");
  muduo::Thread server(boost::bind(&serverThread, &entropy, &fds[1]), "ssl server");
  client.start();
  server.start();

  client.join();
  server.join();
  double elapsed = now() - start;
  printf("%.2fs %.1f handshakes/s\n", elapsed, N / elapsed);
  entropy_free(&entropy);
}
