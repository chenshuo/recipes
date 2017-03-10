#include "TlsAcceptor.h"

#include "TlsStream.h"

#include <stdio.h>
#include <sys/socket.h>

TlsAcceptor::TlsAcceptor(TlsConfig* config, const InetAddress& listenAddr)
  : context_(TlsContext::kServer, config),
    listenSock_(Socket::createTCP())
{
  listenSock_.setReuseAddr(true);
  listenSock_.bindOrDie(listenAddr);
  listenSock_.listenOrDie();
}

TlsStreamPtr TlsAcceptor::accept()
{
  // FIXME: use accept4
  int sockfd = ::accept(listenSock_.fd(), NULL, NULL);
  if (sockfd >= 0)
  {
    TlsContext context = context_.accept(sockfd);
    if (context.handshake())
    {
      LOG_ERROR << context.error();
      return TlsStreamPtr();
    }
    return TlsStreamPtr(new TlsStream(std::move(context)));
  }
  else
  {
    perror("TlsAcceptor::accept");
    return TlsStreamPtr();
  }
}

