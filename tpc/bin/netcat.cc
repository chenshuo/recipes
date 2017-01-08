#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <thread>

#include <string.h>
#include <unistd.h>

int write_n(int fd, const void* buf, int length)
{
  int written = 0;
  while (written < length)
  {
    int nw = ::write(fd, static_cast<const char*>(buf) + written, length - written);
    if (nw > 0)
    {
      written += nw;
    }
    else if (nw == 0)
    {
      break;  // EOF
    }
    else if (errno != EINTR)
    {
      perror("write");
      break;
    }
  }
  return written;
}

void run(TcpStreamPtr stream)
{
  // Caution: a bad example for closing connection
  std::thread thr([&stream] () {
    char buf[8192];
    int nr = 0;
    while ( (nr = stream->receiveSome(buf, sizeof(buf))) > 0)
    {
      int nw = write_n(STDOUT_FILENO, buf, nr);
      if (nw < nr)
      {
        break;
      }
    }
    ::exit(0);  // should somehow notify main thread instead
  });

  char buf[8192];
  int nr = 0;
  while ( (nr = ::read(STDIN_FILENO, buf, sizeof(buf))) > 0)
  {
    int nw = stream->sendAll(buf, nr);
    if (nw < nr)
    {
      break;
    }
  }
  stream->shutdownWrite();
  thr.join();
}

int main(int argc, const char* argv[])
{
  if (argc < 3)
  {
    printf("Usage:\n  %s hostname port\n  %s -l port\n", argv[0], argv[0]);
    return 0;
  }

  int port = atoi(argv[2]);
  if (strcmp(argv[1], "-l") == 0)
  {
    std::unique_ptr<Acceptor> acceptor(new Acceptor(InetAddress(port)));
    TcpStreamPtr stream(acceptor->accept());
    if (stream)
    {
      acceptor.reset();  // stop listening
      run(std::move(stream));
    }
    else
    {
      perror("accept");
    }
  }
  else
  {
    InetAddress addr(port);
    const char* hostname = argv[1];
    if (InetAddress::resolve(hostname, &addr))
    {
      TcpStreamPtr stream(TcpStream::connect(addr));
      if (stream)
      {
        run(std::move(stream));
      }
      else
      {
        printf("Unable to connect %s\n", addr.toIpPort().c_str());
        perror("");
      }
    }
    else
    {
      printf("Unable to resolve %s\n", hostname);
    }
  }
}
