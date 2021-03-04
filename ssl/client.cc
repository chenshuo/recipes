#include "timer.h"

#include "TlsConfig.h"
#include "TlsStream.h"

int main(int argc, char* argv[])
{
  TlsConfig config;
  config.setCaFile("ca.pem");
  const char* hostport = "localhost:4433";
  if (argc > 1)
    hostport = argv[1];
  TlsStreamPtr stream = TlsStream::connect(&config, hostport, "Test Server Cert");
  if (stream)
  {
    LOG_INFO << "OK";
    char buf[16384] = { 0 };
    int64_t total = 0;
    Timer t;
    t.start();
    while (total < 1e10)
    {
      int nw = stream->sendSome(buf, sizeof buf);
      total += nw;
    }
    t.stop();
    LOG_INFO << t.seconds();
    // FIXME: getrusage()
  }
}
