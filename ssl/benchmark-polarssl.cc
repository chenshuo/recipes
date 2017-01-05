#include <polarssl/ctr_drbg.h>
#include <polarssl/error.h>
#include <polarssl/entropy.h>
#include <polarssl/ssl.h>

#include <polarssl/certs.h>

#include <muduo/net/Buffer.h>
#include <string>
#include <stdio.h>
#include <sys/time.h>
#include "timer.h"

muduo::net::Buffer clientOut, serverOut;

int net_recv(void* ctx, unsigned char* buf, size_t len)
{
  muduo::net::Buffer* in = static_cast<muduo::net::Buffer*>(ctx);
  //printf("%s recv %zd\n", in == &clientOut ? "server" : "client", len);
  if (in->readableBytes() > 0)
  {
    size_t n = std::min(in->readableBytes(), len);
    memcpy(buf, in->peek(), n);
    in->retrieve(n);

    /*
    if (n < len)
      printf("got %zd\n", n);
    else
      printf("\n");
      */
    return n;
  }
  else
  {
    //printf("got 0\n");
    return POLARSSL_ERR_NET_WANT_READ;
  }
}

int net_send(void* ctx, const unsigned char* buf, size_t len)
{
  muduo::net::Buffer* out = static_cast<muduo::net::Buffer*>(ctx);
  // printf("%s send %zd\n", out == &clientOut ? "client" : "server", len);
  out->append(buf, len);
  return len;
}

int main(int argc, char* argv[])
{
  entropy_context entropy;
  entropy_init(&entropy);
  ctr_drbg_context ctr_drbg;
  ctr_drbg_init(&ctr_drbg, entropy_func, &entropy, NULL, 0);

  ssl_context ssl;
  bzero(&ssl, sizeof ssl);
  ssl_init(&ssl);
  ssl_set_rng(&ssl, ctr_drbg_random, &ctr_drbg);
  ssl_set_bio(&ssl, &net_recv, &serverOut, &net_send, &clientOut);
  ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
  ssl_set_authmode(&ssl, SSL_VERIFY_NONE);

  const char* srv_cert = test_srv_crt_ec;
  const char* srv_key = test_srv_key_ec;
  std::string arg = argc > 1 ? argv[1] : "r";
  bool useRSA = arg == "r" || arg == "er";
  bool useECDHE = arg == "er" || arg == "ee";
  if (useRSA)
  {
    srv_cert = test_srv_crt;
    srv_key = test_srv_key;
  }
  x509_crt cert;
  x509_crt_init(&cert);
  // int ret = x509_crt_parse_file(&cert, argv[1]);
  // printf("cert parse %d\n", ret);
  x509_crt_parse(&cert, reinterpret_cast<const unsigned char*>(srv_cert), strlen(srv_cert));
  x509_crt_parse(&cert, reinterpret_cast<const unsigned char*>(test_ca_list), strlen(test_ca_list));

  pk_context pkey;
  pk_init(&pkey);
  pk_parse_key(&pkey, reinterpret_cast<const unsigned char*>(srv_key), strlen(srv_key), NULL, 0);
  // ret = pk_parse_keyfile(&pkey, argv[2], NULL);
  // printf("key parse %d\n", ret);

  ssl_context ssl_server;
  bzero(&ssl_server, sizeof ssl_server);
  ssl_init(&ssl_server);
  ssl_set_rng(&ssl_server, ctr_drbg_random, &ctr_drbg);
  ssl_set_bio(&ssl_server, &net_recv, &clientOut, &net_send, &serverOut);
  ssl_set_endpoint(&ssl_server, SSL_IS_SERVER);
  ssl_set_authmode(&ssl_server, SSL_VERIFY_NONE);
  //ssl_set_ca_chain(&ssl_server, cert.next, NULL, NULL);
  ssl_set_own_cert(&ssl_server, &cert, &pkey);
  ecp_group_id curves[] = { POLARSSL_ECP_DP_SECP256R1, POLARSSL_ECP_DP_SECP224K1, POLARSSL_ECP_DP_NONE };
  ssl_set_curves(&ssl_server, curves);
  if (useECDHE)
  {
    int ciphersuites[] = { TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA, TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA, 0 };
    ssl_set_ciphersuites(&ssl_server, ciphersuites);
  }
  else
  {
    int ciphersuites[] = { TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA, TLS_RSA_WITH_AES_256_CBC_SHA, 0 };
    ssl_set_ciphersuites(&ssl_server, ciphersuites);
  }

  double start = now();
  Timer tc, ts;
  const int N = 500;
  for (int i = 0; i < N; ++i)
  {
    ssl_session_reset(&ssl);
    ssl_session_reset(&ssl_server);
    while (true)
    {
      tc.start();
      int ret = ssl_handshake(&ssl);
      tc.stop();
      //printf("ssl %d\n", ret);
      if (ret < 0)
      {
        if (ret != POLARSSL_ERR_NET_WANT_READ)
        {
          char errbuf[512];
          polarssl_strerror(ret, errbuf, sizeof errbuf);
          printf("client error %d %s\n", ret, errbuf);
          break;
        }
      }
      else if (ret == 0 && i == 0)
      {
        printf("client done %s %s\n", ssl_get_version(&ssl), ssl_get_ciphersuite(&ssl));
      }

      ts.start();
      int ret2 = ssl_handshake(&ssl_server);
      ts.stop();
      // printf("srv %d\n", ret2);
      if (ret2 < 0)
      {
        if (ret != POLARSSL_ERR_NET_WANT_READ)
        {
          char errbuf[512];
          polarssl_strerror(ret2, errbuf, sizeof errbuf);
          printf("server error %d %s\n", ret2, errbuf);
          break;
        }
      }
      else if (ret2 == 0)
      {
        // printf("server done %s %s\n", ssl_get_version(&ssl_server), ssl_get_ciphersuite(&ssl_server));
      }

      if (ret == 0 && ret2 == 0)
        break;
    }
  }
  double elapsed = now() - start;
  printf("%.2fs %.1f handshakes/s\n", elapsed, N / elapsed);
  printf("client %.3f %.1f\n", tc.seconds(), N / tc.seconds());
  printf("server %.3f %.1f\n", ts.seconds(), N / ts.seconds());
  printf("server/client %.2f\n", ts.seconds() / tc.seconds());

  double start2 = now();
  const int M = 200;
  unsigned char buf[16384] = { 0 };
  for (int i = 0; i < M*1024; ++i)
  {
    int n = ssl_write(&ssl, buf, 1024);
    if (n < 0)
    {
      char errbuf[512];
      polarssl_strerror(n, errbuf, sizeof errbuf);
      printf("%s\n", errbuf);
    }
    /*
    n = ssl_read(&ssl_server, buf, 8192);
    if (n != 1024)
      break;
    if (n < 0)
    {
      char errbuf[512];
      polarssl_strerror(n, errbuf, sizeof errbuf);
      printf("%s\n", errbuf);
    }
    */
    clientOut.retrieveAll();
  }
  elapsed = now() - start2;
  printf("%.2f %.1f MiB/s\n", elapsed, M / elapsed);

  ssl_free(&ssl);
  ssl_free(&ssl_server);
  pk_free(&pkey);
  x509_crt_free(&cert);
  entropy_free(&entropy);
}
