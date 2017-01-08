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
  }
}

