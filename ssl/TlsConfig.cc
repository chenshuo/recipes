
#include "TlsConfig.h"
#include "TlsStream.h"

int TlsConfig::initialized = tls_init() + 1;
