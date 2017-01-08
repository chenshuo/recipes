#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <thread>
#include <unistd.h>

void sender(const char* filename, TcpStreamPtr stream)
{
  FILE* fp = fopen(filename, "rb");
  if (!fp)
    return;

  printf("Sleeping 10 seconds.\n");
  sleep(10);

  printf("Start sending file %s\n", filename);
  char buf[8192];
  size_t nr = 0;
  while ( (nr = fread(buf, 1, sizeof buf, fp)) > 0)
  {
    stream->sendAll(buf, nr);
  }
  fclose(fp);
  printf("Finish sending file %s\n", filename);

  // Safe close connection
  printf("Shutdown write and read until EOF\n");
  stream->shutdownWrite();
  while ( (nr = stream->receiveSome(buf, sizeof buf)) > 0)
  {
    // do nothing
  }
  printf("All done.\n");

  // TcpStream destructs here, close the TCP socket.
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Usage:\n  %s filename port\n", argv[0]);
    return 0;
  }

  int port = atoi(argv[2]);
  Acceptor acceptor((InetAddress(port)));
  printf("Accepting... Ctrl-C to exit\n");
  int count = 0;
  while (true)
  {
    TcpStreamPtr tcpStream = acceptor.accept();
    printf("accepted no. %d client\n", ++count);

    std::thread thr(sender, argv[1], std::move(tcpStream));
    thr.detach();
  }
}
