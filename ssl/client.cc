#include "TlsConfig.h"
#include "TlsStream.h"

int main(int argc, char* argv[])
{
  TlsConfig config;
  config.setCaFile("ca.pem");
  TlsStreamPtr stream = TlsStream::connect(&config, "localhost:4433", "Test Server Cert");
  if (stream)
  {
    LOG_INFO << "OK";
  }
}
