#include "timer.h"
#include "thread/Thread.h"
#include <boost/bind.hpp>

#include <assert.h>
#include <fcntl.h>
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

  ret = tls_config_set_ecdhecurve(cfg, "prime256v1");
  assert(ret == 0);

  // tls_config_verify_client_optional(cfg);
  struct tls* ctx = tls_server();
  assert(ctx != NULL);

  ret = tls_configure(ctx, cfg);
  assert(ret == 0);

  struct tls* sctx = NULL;
  ret = tls_accept_socket(ctx, &sctx, sockfd);
  assert(ret == 0 && sctx != NULL);

  return sctx;
}

// only works for non-blocking sockets
bool handshake(struct tls* cctx, struct tls* sctx)
{
  int client_done = false, server_done = false;

  while (!(client_done && server_done))
  {
    if (!client_done)
    {
      int ret = tls_handshake(cctx);
      // printf("c %d\n", ret);
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
      // printf("s %d\n", ret);
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

void setBlockingIO(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags > 0)
  {
    printf("set blocking IO for %d\n", fd);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
  }
}

const int N = 500;

struct Trial
{
  int blocks, block_size;
};

void client_thread(struct tls* ctx)
{
  Timer t;
  t.start();
  for (int i = 0; i < N; ++i)
  {
    int ret = tls_handshake(ctx);
    if (ret != 0)
      printf("client err = %d\n", ret);
  }
  t.stop();
  printf("client %f secs, %f handshakes/sec\n", t.seconds(), N / t.seconds());
  while (true)
  {
    Trial trial = { 0, 0 };

    int nr = tls_read(ctx, &trial, sizeof trial);
    if (nr == 0)
      break;
    assert(nr == sizeof trial);
    // printf("client read bs %d nb %d\n", trial.block_size, trial.blocks);
    if (trial.block_size == 0)
      break;
    char* buf = new char[trial.block_size];
    for (int i = 0; i < trial.blocks; ++i)
    {
      nr = tls_read(ctx, buf, trial.block_size);
      assert(nr == trial.block_size);
    }
    int64_t ack = static_cast<int64_t>(trial.blocks) * trial.block_size;
    int nw = tls_write(ctx, &ack, sizeof ack);
    assert(nw == sizeof ack);
    delete[] buf;
  }
  printf("client done\n");
  tls_close(ctx);
  tls_free(ctx);
}

void send(int block_size, struct tls* ctx)
{
  double start = now();
  int total = 0;
  int blocks = 1024;
  char* message = new char[block_size];
  bzero(message, block_size);
  Timer t;
  while (now() - start < 10)
  {
    Trial trial = { blocks, block_size };
    int nw = tls_write(ctx, &trial, sizeof trial);
    assert(nw == sizeof trial);
    t.start();
    for (int i = 0; i < blocks; ++i)
    {
      nw = tls_write(ctx, message, block_size);
      if (nw != block_size)
        printf("bs %d nw %d\n", block_size, nw);
      assert(nw == block_size);
    }
    t.stop();
    int64_t ack = 0;
    int nr = tls_read(ctx, &ack, sizeof ack);
    assert(nr == sizeof ack);
    assert(ack == static_cast<int64_t>(blocks) * block_size);
    total += blocks;
    blocks *= 2;
  }
  double secs = now() - start;
  printf("bs %5d sec %.3f tot %d thr %.1fKB/s wr cpu %.3f\n", block_size, secs, total,
         block_size / secs * total / 1024, t.seconds());
  delete[] message;
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

  setBlockingIO(fds[0]);
  setBlockingIO(fds[1]);
  muduo::Thread thr(boost::bind(client_thread, cctx), "clientThread");
  thr.start();

  {
  Timer t;
  t.start();
  for (int i = 0; i < N; ++i)
  {
    int ret = tls_handshake(sctx);
    if (ret != 0)
      printf("server err = %d\n", ret);
  }
  t.stop();
  printf("server %f secs, %f handshakes/sec\n", t.seconds(), N / t.seconds());
  }

  for (int i = 1; i <= 1024 * 16; i *= 2)
  {
    send(i, sctx);
  }
  tls_close(sctx);
  shutdown(fds[1], SHUT_RDWR);
  tls_free(sctx);

  thr.join();
}
