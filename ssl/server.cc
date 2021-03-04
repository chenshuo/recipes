#include "timer.h"

#include "InetAddress.h"
#include "TlsAcceptor.h"
#include "TlsConfig.h"
#include "TlsStream.h"

int main(int argc, char* argv[])
{
  TlsConfig config;
  // config.setCaFile("ca.pem");
  config.setCertFile("server.pem");
  config.setKeyFile("server.pem");

  InetAddress listenAddr(4433);
  TlsAcceptor acceptor(&config, listenAddr);

  TlsStreamPtr stream = acceptor.accept();
  if (stream)
  {
    LOG_INFO << "OK";
    int64_t total = 0;
    char buf[20 * 1024];
    int nr = 0;
    Timer t;
    t.start();
    while ( (nr = stream->receiveSome(buf, sizeof buf)) > 0) {
      // LOG_INFO << "nr = " << nr;
      total += nr;
    }
    // LOG_INFO << "nr = " << nr;
    t.stop();
    LOG_INFO << "DONE " << total
        << " " << (total / t.seconds() / 1e6) << " MB/s";
  }
}

